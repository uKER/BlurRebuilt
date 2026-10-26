#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/SceneDescription.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <process/CreatePvsSamplePointsFile.h>
#include <Polyflags.h>

bool C_ProcessCreatePvsSamplePointsFile::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateSimple progressSimple(pProgress, GetName());

	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataSceneDescription* pInputSceneDescription = (const C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
	assert(pInputSceneDescription);


	bool success = true;

	// build sample data
	Pvs::Build::ModelSamplePointSetup setup;
	m_pvsData.GenerateModelSamplePoints(setup);

	//Create vis directory
	
	Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(pInputConversionOptions->GetVisInputFilename()));

	//Write file to Vis tool
	m_pvsData.WriteDataForPvsTool(pInputConversionOptions->GetVisInputFilename());

	//Write file to ReadPVSData
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(pInputConversionOptions->GetPVSDataFilename(), Sys::E_FileAccess_Write);
	if (pFile)
	{
		Arch::BinaryOutputArchive ar(*pFile, true);
		ar << m_pvsData;
		delete pFile;
	}


	return success;
}

const Util::C_String C_ProcessCreatePvsSamplePointsFile::GetName() const
{
	return "C_ProcessCreatePvsSamplePointsFile";
}