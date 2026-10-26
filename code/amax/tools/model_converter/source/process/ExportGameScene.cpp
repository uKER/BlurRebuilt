#include "stdafx.h"

#include <tools_shared/shim/ShimOptions.h>

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/ExportHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/PhysicsData.h>
#include <data/RenderData.h>
#include <data/SceneDescription.h>

#include <process/ExportGameScene.h>

C_ProcessExportGameScene::C_ProcessExportGameScene()
	: m_Platform(Tools::E_Platform_Unknown)
{
}

const Util::C_String C_ProcessExportGameScene::GetName() const
{
	return Util::C_String("Export Game Scene");
}

bool C_ProcessExportGameScene::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateSimple progress( pProgress, GetName() );

	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	// Initialise File List
	Tools::Shim::C_FileCopyXmlExport::ClearXmlFileList( 
		pInputConversionOptions->GetSourceAftkFilename(), 
		((const Tools::Shim::C_Options*)pInputConversionOptions)->GetExecuterNodeName());	

	bool success = true;

	// Write out data for our different skus.
	for(m_Platform=Tools::E_Platform_Initial; m_Platform < Tools::E_Platform_Count; m_Platform++)
    {
		success = ExportMainScene( pInputConversionOptions->GetOutputFilename(GetCurrPlatform()) ) && success;
		if(!success)
		{
			return false;
		}

	/*	const C_DataPhysicsData* pInputPhysicsData = (C_DataPhysicsData*)GetInput( E_GraphData_PhysicsData );
		if(pInputPhysicsData)
		{
			success = ExportCollision( pInputConversionOptions->GetOutputFilename(GetCurrPlatform(), "Level.levelphysics") ) && success;
			if(!success)
			{
				return false;
			}
		}

		const C_DataRenderData* pInputRenderData = (C_DataRenderData*)GetInput( E_GraphData_RenderData );
		if(pInputRenderData)
		{
			success = ExportRender( pInputConversionOptions->GetOutputFilename(GetCurrPlatform(), "Level.levelrender") ) && success;
			if(!success)
			{
				return false;
			}
		}*/
	}

	// Write filecopy list
	success = Tools::Shim::C_FileCopyXmlExport::ExportXmlFileList(
		((const Tools::Shim::C_Options*)pInputConversionOptions)->GetRootTempPath(),
		GetFileCopyList(), 
		pInputConversionOptions->GetSourceAftkFilename(), 
		((const Tools::Shim::C_Options*)pInputConversionOptions)->GetCopyLocations(),
		((const Tools::Shim::C_Options*)pInputConversionOptions)->GetLaunchComputerName(),
		((const Tools::Shim::C_Options*)pInputConversionOptions)->GetExecuterNodeName());

	if (!success)
		return false;

	return true;
}

Tools::FileCopy::C_FileCopyList& C_ProcessExportGameScene::GetFileCopyList()
{
	return m_FileCopyList;
}
	
Tools::E_Platform C_ProcessExportGameScene::GetCurrPlatform()
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

	SAFE_DELETE_PTR( pFile );

	return success;
}

//bool C_ProcessExportGameScene::ExportCollision( const Util::C_String& outputFilename )
//{
//	bool success = true;
//
//	Sys::I_File* pFile = ExportHelpers::WriteExportFile( outputFilename, GetCurrPlatform(), &GetFileCopyList()  );
//	if(!pFile)
//	{
//		return false;
//	}
//
//	const C_DataPhysicsData* pInputPhysicsData = (C_DataPhysicsData*)GetInput( E_GraphData_PhysicsData );
//	assert(pInputPhysicsData);
//
//	success = pInputPhysicsData->Export(pFile) && success;
//	if (!success) 
//	{
//		LOGWARN( Tools::g_log, "Failed to export file data for '%s'", outputFilename.Chars());
//	}
//
//	SAFE_DELETE_PTR( pFile );
//
//	return success;
//}
//
//
//bool C_ProcessExportGameScene::ExportRender( const Util::C_String& outputFilename )
//{
//	bool success = true;
//
//	Sys::I_File* pFile = ExportHelpers::WriteExportFile( outputFilename, GetCurrPlatform(), &GetFileCopyList()  );
//	if(!pFile)
//	{
//		return false;
//	}
//
//	const C_DataRenderData* pInputRenderData = (C_DataRenderData*)GetInput( E_GraphData_RenderData );
//	assert(pInputRenderData);
//
//	success = pInputRenderData->Export(pFile, GetCurrPlatform() ) && success;
//	if (!success) 
//	{
//		LOGWARN( Tools::g_log, "Failed to export file data for '%s'", outputFilename.Chars());
//	}
//
//	SAFE_DELETE_PTR( pFile );
//
//	return success;
//}

enum E_ModelFileVersion
{
	E_ModelFileVersion_Initial = 0,

	E_ModelFileVersion_Current = E_ModelFileVersion_Initial
};

bool C_ProcessExportGameScene::ExportMainScene( Sys::I_File* pFile )
{
	bool success = true;

	Sys::C_FileBlock rootBlock(pFile, "Model");

	// Header Block
	{
		Sys::C_FileBlock headerBlock(&rootBlock, "Header");

		Sys::BinarySerialiserAutoEndian ar(headerBlock);
		ar << (S32)E_ModelFileVersion_Current;

	}

	// Model Data Block
	{
		Sys::C_FileBlock elementsBlock(&rootBlock, "MdlDat");
		
		const C_DataSceneDescription* pInputSceneDescription = (C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
		assert(pInputSceneDescription);

		success = pInputSceneDescription->ExportModelData( &elementsBlock ) && success;
	}

	{
		Sys::C_FileBlock sceneBlock(&rootBlock, "Render");

		const C_DataRenderData* pInputRenderData = (C_DataRenderData*)GetInput( E_GraphData_RenderData );
		assert(pInputRenderData);

		success = pInputRenderData->Export(&sceneBlock, GetCurrPlatform()) && success;
	}

	{
		Sys::C_FileBlock collideBlock(&rootBlock, "Coll");

		const C_DataPhysicsData* pInputPhysicsData = (C_DataPhysicsData*)GetInput( E_GraphData_PhysicsData );
		assert(pInputPhysicsData);
	
		success = pInputPhysicsData->Export(&collideBlock) && success;
	}

	return success;
}
