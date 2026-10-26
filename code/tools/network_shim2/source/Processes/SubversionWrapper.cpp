#include "stdafx.h"
#include "Utility.h"
#include <tools_shared/shim/ShimErrorHelpers.h>

#include <processes/SubversionWrapper.h>

bool SubversionWrapper::GetLatest(const C_Options& options, const Tools::Shim::C_SvnFolder& svnFolder)
{
	//Grab head revision.
	Btl::String args = 
		options.GetSvnOptions().user + 
		Btl::String(" ") + 
		options.GetSvnOptions().pass + 
		Btl::String(" \"") + 
		svnFolder.remotePath +
		Btl::String("\" \"") + 
		svnFolder.localPath.Chars() + Btl::String("\"");

	Sys::C_FilePath workingDir = Sys::ExtractDirectory(options.GetSvnOptions().svnGetLatestCmd);

	U32 kOneSecond = 1000;
	U32 kOneMinute = 60*kOneSecond;
	U32 kTenMinutes = 10*kOneMinute;

	Btl::String cmd = options.GetSvnOptions().svnGetLatestCmd.Chars();
	BEGIN_CHECK_CUSTOM(Sys::ShellExec::ExecuteCommandAndWait(
		cmd.Chars(), 
		args.Chars(), 
		workingDir.Chars(),
		options.GetShellExecLaunchState(),
		&Tools::g_pMyApplication->GetLogger(),
		0,
		"",
		kTenMinutes,
		true,
		true) != 0)
		ERROR_ERROR("An error occured while trying to get the latest files from SVN.\n");
	END_RETURN_FALSE;
	return true;
}
