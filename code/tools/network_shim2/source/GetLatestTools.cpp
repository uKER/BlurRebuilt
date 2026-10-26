#include "stdafx.h"
#include <GetLatestTools.h>
#include <tools_shared/shim/ShimErrorHelpers.h>

bool C_GetLatestTools::GetLatestTools(const C_Options& options)
{
	LOGINFO(Tools::g_pMyApplication->GetLogger(), 
		"Getting latest tools from server: %s", options.GetServerRootToolsPath().Chars());
	LOGINFO(Tools::g_pMyApplication->GetLogger(), 
		"Mirror to folder: %s", options.GetRootToolsPath().Chars());

	Util::C_String params = options.GetServerRootToolsPath() + " " + options.GetRootToolsPath() + " /MIR /r:3";

	Util::C_String workingDirectory = "%temp%";
	Tools::C_MacroExpander::Instance().Expand(workingDirectory);

	/*		
	The return code from Robocopy is a bit map, defined as follows:
	Hex Bit Value	Decimal Value	Meaning If Set
	0x10	16	Serious error. Robocopy did not copy any files. This is either a usage error or an error due to insufficient access privileges on the source or destination directories.
	0x08	8	Some files or directories could not be copied (copy errors occurred and the retry limit was exceeded). Check these errors further.
	0x04	4	Some Mismatched files or directories were detected. Examine the output log. Housekeeping is probably necessary.
	0x02	2	Some Extra files or directories were detected. Examine the output log. Some housekeeping may be needed.
	0x01	1	One or more files were copied successfully (that is, new files have arrived).
	0x00	0	No errors occurred, and no copying was done. The source and destination directory trees are completely synchronized.
	*/
	int exitCode = Sys::ShellExec::ExecuteCommandAndWait(
		ExpandToolPath(options,options.GetRobocopyFilePath()), 
		params, 
		workingDirectory,
		options.GetShellExecLaunchState(),
		&Tools::g_pMyApplication->GetLogger());
	BEGIN_CHECK_CUSTOM(exitCode & (0x10 | 0x08))
		ERROR_FATAL("Something went wrong getting the tools for version '%s' (%i)", options.GetServerRootToolsPath().Chars(), exitCode);
		ERROR_FATAL("Params (%s)", params.Chars());
		ERROR_FATAL("Root folder (%s)", options.GetRootToolsPath().Chars());
	END_RETURN_FALSE;

	LOGDEBUG(Tools::g_pMyApplication->GetLogger(), "Got tools to match version - %s", options.GetServerRootToolsPath().Chars());

	exitCode = Sys::ShellExec::ExecuteCommandAndWait(
		options.GetBootstrapFilePath(), 
		"", 
		"",
		options.GetShellExecLaunchState(),
		&Tools::g_pMyApplication->GetLogger());

	return true;
}
