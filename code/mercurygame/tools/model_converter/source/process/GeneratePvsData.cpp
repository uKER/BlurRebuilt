#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <process/GeneratePvsData.h>

bool C_ProcessGeneratePvsData::Process( Progress::I_ProgressBar* pProgress )
{
	// Vis tool has it's own progress bar update

	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	bool success = true;

	const Tools::ProcessGraph::C_Data* pLitAftkFile = GetInput( E_GraphData_LitAftkFile );
	const Tools::ProcessGraph::C_Data* pPreprocessedAftkFile = GetInput( E_GraphData_PreprocessedAftkFile );

	Util::C_String visApp = pInputConversionOptions->GetToolConfig().GetVisFilenameWin32();
	Util::C_String visInputFile = pInputConversionOptions->GetVisInputFilename();
	Util::C_String visOutputFile = pInputConversionOptions->GetVisOutputFilename();

	Util::C_String a3File;
	if (pLitAftkFile)
		a3File = pInputConversionOptions->GetLitAftkFilename();
	else
	if (pPreprocessedAftkFile)
		a3File = pInputConversionOptions->GetPreprocessedAftkFilename();

	//Run VIS tool
	Util::C_String visCommandLine = " --silent ";

	if(pInputConversionOptions->GetVisibilityOptions().GetUseLightingScreensavers())
	{
		visCommandLine += " -s -e \"";
		visCommandLine += visApp;
		visCommandLine += "\"";
	}

	Util::C_String sPVSConfigFilename = pInputConversionOptions->GetModelOptions().GetPVSConfigFilename();
	if(sPVSConfigFilename.Length() > 0)
	{
		visCommandLine += " --config \"" + sPVSConfigFilename + "\"";
	}

	visCommandLine += " -f " + pInputConversionOptions->GetToolConfig().GetFxbPath();
	visCommandLine += " -t " + pInputConversionOptions->GetToolConfig().GetTextureCachePath();

	visCommandLine += " ";
	visCommandLine += a3File;
	visCommandLine += " ";
	visCommandLine += visOutputFile;
	visCommandLine += " ";
	visCommandLine += visInputFile;

	LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "Executing command line :\n");
	LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "%s\n",visCommandLine.Chars()); 

	int result = Sys::ShellExec::ExecuteCommandAndWait(
		visApp, visCommandLine, "", Sys::ShellExec::E_LaunchState_Minimise,
		&Tools::g_pMyApplication->GetLogger(), pProgress, "Process PVS");

	if(result != 0)
	{
		EXECUTER_FATAL("PVS Processing failed\n");
		return false;
	}

	return success;
}

const Util::C_String C_ProcessGeneratePvsData::GetName() const 
{
	return Util::C_String("C_ProcessGeneratePvsData");
}
