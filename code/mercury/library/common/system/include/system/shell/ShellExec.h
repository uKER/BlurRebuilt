#ifndef MERCURY_SHELL_EXECUTE_H
#define MERCURY_SHELL_EXECUTE_H

#ifdef _MSC_VER_
#pragma once
#endif

//Forward declare debug logger so that we're not forcing people to bring in the
//logging system if they don't wish to do so.
namespace Debug
{
	class Logger;
}
namespace Progress
{
	class I_ProgressBar;
};

namespace Sys
{
namespace  ShellExec
{
	typedef bool (*CheckForCancelCallback)(void* pContext);

	enum E_LaunchState
	{
		E_LaunchState_Show = 0,
		E_LaunchState_Hide,
		E_LaunchState_Minimise
	};

	S32 ExecuteCommandAndWait(
		const char* command, 
		const char* parameters,
		const char* workingDir,
		E_LaunchState launchState,
		Debug::Logger* pLogger = 0,
		Progress::I_ProgressBar *p = 0,
		const char *pszProgressName = "",
		U32 timeoutInverval = 0,
		bool logOutput = false,
		bool stdErrToStdOut = false,
		CheckForCancelCallback cancelFunc = 0,
		void* pContext = 0);
};
}

#endif
