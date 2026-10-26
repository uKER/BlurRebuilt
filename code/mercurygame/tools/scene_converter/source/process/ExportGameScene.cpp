#include "stdafx.h"

#include <tools_shared/shim/ShimOptions.h>

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/ExportHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/PhysicsData.h>
#include <data/RenderData.h>
#include <data/PortalData.h>
#include <data/SceneDescription.h>
#include <data/LightListData.h>
#include <data/LightSamplePointsData.h>
#include <data/CubemapPointsData.h>
#include <data/NavMeshData.h>
#include <data/PvsData.h>
#include <data/CrowdData.h>
#include <data/AdvertisingData.h>
#include <process/ExportGameScene.h>
#include <debug.h>

C_ProcessExportGameScene::C_ProcessExportGameScene()
	: m_Platform(Util::E_Platform_Unknown)
{
}

const Util::C_String C_ProcessExportGameScene::GetName() const
{
	return Util::C_String("Export Game Scene");
}

bool C_ProcessExportGameScene::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateStepped progress( pProgress, Util::E_Platform_Count * 3, GetName() );

	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	// Initialise File List
	Tools::Shim::C_FileCopyXmlExport::ClearXmlFileList(
		pInputConversionOptions->GetSourceAftkFilename(), 
		((const Tools::Shim::C_Options*)pInputConversionOptions)->GetExportName());	

	bool success = true;

	// Write out data for our different skus.
	for(m_Platform=Util::E_Platform_Begin; m_Platform != Util::E_Platform_End; m_Platform++)
	{
		if( !pInputConversionOptions->ExportForPlatform( m_Platform ) )
		{
			LOGINFO( Tools::g_log, "Not exporting for platform %s", Util::GetPlatformString(m_Platform).Chars() );

			progress.Step();
			progress.Step();
			progress.Step();
			progress.Step();

			continue;
		}

		const C_DataPhysicsData* pInputPhysicsData = (C_DataPhysicsData*)GetInput( E_GraphData_PhysicsData );
		if(pInputPhysicsData)
		{
			success = ExportCollision( pInputConversionOptions->GetOutputFilename(GetCurrPlatform(), "Level.levelphysics") ) && success;
			if(!success)
			{
				return false;
			}
		}

		progress.Step();

		const C_DataRenderData* pInputRenderData = (C_DataRenderData*)GetInput( E_GraphData_RenderData );
		if(pInputRenderData)
		{
			const Util::C_String renderFilename = pInputConversionOptions->GetOutputFilename(GetCurrPlatform(), "Level.levelrender" );
			const Util::C_String streamFilename = pInputConversionOptions->GetOutputFilename(GetCurrPlatform(), "Level.levelstream" );
			const bool exportStreamFile = pInputConversionOptions->ExportStreamFile();
			success = ExportRender( renderFilename, streamFilename, exportStreamFile ) && success;
			if(!success)
			{
				return false;
			}
		}
		progress.Step();


		//m_Platform = Tools::E_Platform_360;
		success = ExportMainScene( pInputConversionOptions->GetOutputFilename(GetCurrPlatform(), "Level.level") ) && success;
		if(!success)
		{
			return false;
		}

		progress.Step();
	}

	// Write filecopy list
	success = Tools::Shim::C_FileCopyXmlExport::ExportXmlFileList(
		((const Tools::Shim::C_Options*)pInputConversionOptions)->GetRootTempPath(),
		GetFileCopyList(), 
		pInputConversionOptions->GetSourceAftkFilename(), 
		((const Tools::Shim::C_Options*)pInputConversionOptions)->GetCopyLocations(),
		((const Tools::Shim::C_Options*)pInputConversionOptions)->GetLaunchComputerName(),
		((const Tools::Shim::C_Options*)pInputConversionOptions)->GetExportName());

	if (!success)
		return false;

	return true;
}

Tools::FileCopy::C_FileCopyList& C_ProcessExportGameScene::GetFileCopyList()
{
	return m_FileCopyList;
}
	
Util::E_Platform C_ProcessExportGameScene::GetCurrPlatform()
{
	return m_Platform;
}

bool C_ProcessExportGameScene::ExportMainScene( const Util::C_String& outputFilename )
{
	bool success = true;
	Sys::I_File* pFile = ExportHelpers::WriteExportFile( outputFilename, GetCurrPlatform(), &GetFileCopyList() );
	if(!pFile)
	{
		return false;
	}

	success = ExportMainScene( pFile ) && success;
	if (!success) 
	{
		LOGWARN( Tools::g_log, "Failed to export file data for '%s'", outputFilename.Chars());
	}

	// Write debug block onto end of file (and outside of root block)
	{
		Sys::C_FileBlock metaDebugBlock(pFile, "MtaDbg");
		C_AppMetaDebug::GetInstance()->RemoveLogSubscriber();
		C_AppMetaDebug::GetInstance()->GetRootNode()->Write( &metaDebugBlock );
	}

	SAFE_DELETE_PTR( pFile );

	return success;
}

bool C_ProcessExportGameScene::ExportCollision( const Util::C_String& outputFilename )
{
	bool success = true;

	Sys::I_File* pFile = ExportHelpers::WriteExportFile( outputFilename, GetCurrPlatform(), &GetFileCopyList()  );
	if(!pFile)
	{
		return false;
	}

	const C_DataPhysicsData* pInputPhysicsData = (C_DataPhysicsData*)GetInput( E_GraphData_PhysicsData );
	assert(pInputPhysicsData);

	// write platform id
	{
		U32 platformFourCC = Util::GetPlatformFourCC(GetCurrPlatform());
		Sys::BinarySerialiserAutoEndian ar(*pFile);
		ar << platformFourCC;
	}

	success = pInputPhysicsData->Export(pFile) && success;
	if (!success) 
	{
		LOGWARN( Tools::g_log, "Failed to export file data for '%s'", outputFilename.Chars());
	}

	SAFE_DELETE_PTR( pFile );

	return success;
}


bool C_ProcessExportGameScene::ExportRender( const Util::C_String& outputFilename, const Util::C_String& streamFilename, const bool exportStreamFile )
{
	bool success = true;

	Sys::I_File* pFile = ExportHelpers::WriteExportFile( outputFilename, GetCurrPlatform(), &GetFileCopyList()  );
	if(!pFile)
	{
		return false;
	}

	Sys::I_File* pStreamFile = NULL;
	if( exportStreamFile )
	{
		pStreamFile = ExportHelpers::WriteExportFile( streamFilename, GetCurrPlatform(), &GetFileCopyList()  );
		if(!pStreamFile)
		{
			return false;
		}
	}
	
	success = ExportRender( pFile, pStreamFile ) && success;
	if (!success) 
	{
		LOGWARN( Tools::g_log, "Failed to export file data for '%s'", outputFilename.Chars());
	}


	SAFE_DELETE_PTR( pStreamFile );
	SAFE_DELETE_PTR( pFile );

	return success;
}

bool C_ProcessExportGameScene::ExportRender( Sys::I_File* pFile, Sys::I_File* pStreamFile )
{
	const C_DataRenderData* pInputRenderData = (C_DataRenderData*)GetInput( E_GraphData_RenderData );
	assert(pInputRenderData);

	// write platform id
	{
		U32 platformFourCC = Util::GetPlatformFourCC(GetCurrPlatform());
		Sys::BinarySerialiserAutoEndian ar(*pFile);
		ar << platformFourCC;
	}

	Sys::C_FileBlock rootBlock(pFile, "Render");

	//Export Render Data

	bool success = pInputRenderData->Export(&rootBlock, pStreamFile, GetCurrPlatform() );
	if (!success) 
	{
		LOGWARN( Tools::g_log, "Failed to export file data");
	}


	//Export Light Sample Points Data
	const C_DataLightSamplePoints* pLightSamplePoints = (C_DataLightSamplePoints*)GetInput( E_GraphData_LightSamples );
	assert(pLightSamplePoints);

	success = pLightSamplePoints->Export(&rootBlock, GetCurrPlatform() ) && success;
	if (!success) 
	{
		LOGWARN( Tools::g_log, "Failed to export file data");
	}

	//Export Light List Data
	const C_DataLightList* pLightList = (C_DataLightList*)GetInput( E_GraphData_LightList );
	assert(pLightList);

	success = pLightList->Export(&rootBlock, GetCurrPlatform() ) && success;
	if (!success) 
	{
		LOGWARN( Tools::g_log, "Failed to export file data");
	}

	//Export PVS Data
	const C_DataPvs* pDataPvs = (C_DataPvs*)GetInput( E_GraphData_PvsData );
	//Could be deactivated
	if (pDataPvs)
	{
		success = pDataPvs->Export(&rootBlock, GetCurrPlatform() ) && success;
		if (!success) 
		{
			LOGWARN( Tools::g_log, "Failed to export file data");
		}
	}

	//Export cubemap point data
	const C_DataCubemapPoints* pDataCubemapPoints = (C_DataCubemapPoints*)GetInput( E_GraphData_CubemapPoints );
	assert(pDataCubemapPoints);

	success = pDataCubemapPoints->Export(&rootBlock, GetCurrPlatform() ) && success;
	if (!success)
	{
		LOGWARN( Tools::g_log, "Failed to export cubemap point data");
	}

	//Export Portal data
	const C_DataPortalData* pPortalData = (C_DataPortalData*)GetInput( E_GraphData_PortalRooms );
	assert(pPortalData);

	success = pPortalData->Export(&rootBlock, GetCurrPlatform() ) && success;
	if (!success) 
	{
		LOGWARN( Tools::g_log, "Failed to export file data");
	}

	//Export crowd data
	const C_DataCrowd* pCrowdData = (C_DataCrowd*)GetInput( E_GraphData_CrowdData );
	assert(pCrowdData);

	success = pCrowdData->Export(&rootBlock, GetCurrPlatform() ) && success;
	if (!success)
	{
		LOGWARN( Tools::g_log, "Failed to export file data");
	}

	return success;
}
enum E_SceneFileVersion
{
	E_SceneFileVersion_Initial = 0,

	E_SceneFileVersion_Current = E_SceneFileVersion_Initial
};

bool C_ProcessExportGameScene::ExportMainScene( Sys::I_File* pFile )
{
	bool success = true;

	// write platform id
	{
		U32 platformFourCC = Util::GetPlatformFourCC(GetCurrPlatform());
		Sys::BinarySerialiserAutoEndian ar(*pFile);
		ar << platformFourCC;
	}

	Sys::C_FileBlock rootBlock(pFile, "Level");

	// Header Block
	{
		Sys::C_FileBlock headerBlock(&rootBlock, "Header");

		Sys::BinarySerialiserAutoEndian ar(headerBlock);
		ar << (S32)E_SceneFileVersion_Current;
	}

	// ModelData Block
	{
		Sys::C_FileBlock elementsBlock(&rootBlock, "MdlDat");
		
		const C_DataSceneDescription* pInputSceneDescription = (C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
		assert(pInputSceneDescription);

		success = pInputSceneDescription->ExportModelData( &elementsBlock ) && success;
	}

	// NavMeshData Block
	{
		Sys::C_FileBlock elementsBlock(&rootBlock, "NavDat");

		const C_DataNavMesh* pInputNavMesh = (C_DataNavMesh*)GetInput( E_GraphData_NavMesh );
		assert(pInputNavMesh);

		success = pInputNavMesh->Export( &elementsBlock ) && success;
	}

	// AdvertisingData Block
	{
		Sys::C_FileBlock elementsBlock(&rootBlock, "Advert");

		const C_DataAdvertising* pInputAdData = (C_DataAdvertising*)GetInput( E_GraphData_AdvertisingData );
		assert(pInputAdData);

		success = pInputAdData->Export( &elementsBlock ) && success;
	}
	return success;
}
