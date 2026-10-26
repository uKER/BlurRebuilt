#include "stdafx.h"
#include "Maya.h"

#include <maya/MLibrary.h>
#include <maya/MGlobal.h>
#include <maya/MCommandResult.h>
#include <maya/MStringArray.h>

void AtExitMayaCleanup()
{
	atexit(0);
	//Exits process with given exit code.
	MLibrary::cleanup(g_exitCode);
}
	

bool InitialiseMaya()
{
	MStatus mayaStatus = MLibrary::initialize(true, "Nexus", true);
	if (!mayaStatus) 
	{
		LOGERROR(g_log, "Failed in initialise Maya: %s", mayaStatus.errorString().asChar());
		return false;
	}

	MStatus mStatus;
	//Ensure we skip the "get latest from svn" phase
	mStatus = MGlobal::executeCommand("global int $biz_TM_skipGetLatest = 1;");
	if (!mayaStatus)
		return false;

	std::string command(
		"source biz_batchModeSetup.mel;");
	mayaStatus = MGlobal::executeCommand(MString(command.c_str()));
	if (!mayaStatus)
	{
		LOGERROR(g_log, "biz_batchModeSetup was not successful: %s", mayaStatus.errorString().asChar());
		return false;
	}

	atexit(&AtExitMayaCleanup);
	return true;
}
