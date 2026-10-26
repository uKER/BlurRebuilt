#include "SystemPch.h"
#include <system/shell/ShellExec.h>
#include <system/debuglog.h>
#include <core/threads.h>
#include <system/network.h>
#include <system/misc/progress.h>

#ifdef PLATFORM_WINDOWS
#include "shellapi.h"
#endif

namespace Sys
{
namespace ShellExec
{

#ifdef PLATFORM_WINDOWS

// listen for progress updates via the network and pass them on
const char* kTimoutMessage = "Timeout period has elapsed. Failed to execute command.\n";

void UpdateProgress(void *handle, Progress::I_ProgressBar *pProgress, const char *psz)
{
	if (pProgress)
	{
		Network::TCPSocket socket(8004, 6);
		// wait a second for an incoming connection
		if (socket.IsInputPending(10000))
		{
			// create a socket containing the new connection
			Network::Socket *pConnection = new Network::Socket(accept(socket.GetSocket(), NULL, NULL));
			
			// while the process is active
			while (WaitForSingleObject( handle, 10 ))
			{
				// poll for progress updates
				U8 x = 255;
				if (pConnection->Receive(&x, 1) != 1)
					break;

				// send this as a progress update
				pProgress->ProgressNotify(0, float(x)*(100.f/255.f), psz);
			}
			
			delete pConnection;
		}
	}
}

S32 ExecuteCommandAndWait(
	const char* command, 
	const char* parameters, 
	const char* workingDir,
	E_LaunchState launchState,
	Debug::Logger* pLogger,
	Progress::I_ProgressBar *pProgress,
	const char *pszProgressName,
	U32 timeoutInterval,
	bool logOutput,
	bool stdErrToStdOut,
	CheckForCancelCallback cancelFunc,
	void* pContext)
{
	int exitCode;

	TCHAR* processCommand = 0;
	if (strlen(command) > 0)
	{
		processCommand = new TCHAR[strlen(command) + strlen(parameters) + 2];

		strcpy(processCommand, command);

		if (strlen(parameters) > 0)
		{
			strcat(processCommand, " ");
			strcat(processCommand, parameters);
		}
	}

	TCHAR* processWorkingDir = 0;
	if (strlen(workingDir))
	{
		processWorkingDir = new TCHAR[strlen(workingDir)+1];
		strcpy(processWorkingDir, workingDir);
	}


	HANDLE outputPipeR;
	HANDLE outputPipeW;
	HANDLE errorPipeR;
	HANDLE errorPipeW;

	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;

	switch (launchState)
	{
	default:
	case E_LaunchState_Show:
		si.wShowWindow = SW_SHOW;
		break;
	case E_LaunchState_Hide:
		si.wShowWindow = SW_HIDE;
		break;
	case E_LaunchState_Minimise:
		si.wShowWindow = SW_SHOWMINNOACTIVE;
		break;
	};

	if (pLogger && logOutput)
	{
		SECURITY_ATTRIBUTES saAttr; 
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
		saAttr.lpSecurityDescriptor = NULL;
		saAttr.bInheritHandle = true;

		if (!CreatePipe(&outputPipeR, &outputPipeW, &saAttr, 65535))
		{
			DWORD iReturnVal = GetLastError();
			return 1;
		}

		if (!CreatePipe(&errorPipeR, &errorPipeW, &saAttr, 65535))
		{
			DWORD iReturnVal = GetLastError();
			return 1;
		}

		si.hStdOutput = outputPipeW;
		si.hStdError = errorPipeW;
		si.hStdInput = INVALID_HANDLE_VALUE;
		si.dwFlags |= STARTF_USESTDHANDLES;
	}

	if (pLogger)
	{
		Debug::Logger& logRef(*pLogger);
		LOGDEBUG(logRef, "ExecuteCommandAndWait:\n");
		LOGDEBUG(logRef, "- File(%s)\n", command);
		LOGDEBUG(logRef, "- Params(%s)\n", parameters);
		LOGDEBUG(logRef, "- Entire Command Line(%s)\n", processCommand);
	}

	if (pProgress)
		pProgress->ProgressNotify(0, 0.f, pszProgressName?pszProgressName:command);
	
	if (CreateProcessA(	NULL,
						processCommand,
						NULL,
						NULL, 
						true,
						0,
						NULL,
						processWorkingDir,
						&si,
						&pi ))
	{
		if (pLogger)
		{
			Debug::Logger& logRef(*pLogger);
			LOGDEBUG(logRef, "Command started...\n");
		}

		UpdateProgress(pi.hProcess, pProgress, pszProgressName?pszProgressName:command);

		// If a logger is available, then we need to poll the pipes and make sure they're not full
		// otherwise we can simply wait on the process
		if (pLogger && logOutput)
		{
			Debug::Logger& logRef(*pLogger);

			char* stdOutBuffer = new char[1024];

			U32 timeWaited = 0;

			while (WaitForSingleObject(pi.hProcess, 50) == WAIT_TIMEOUT)
			{
				DWORD bytesAvail, bytesLeft;

				// Pipe stdout
				while (PeekNamedPipe(outputPipeR, NULL, 0, NULL, &bytesAvail, &bytesLeft) && bytesAvail > 0)
				{
					DWORD numBytesRead;
					if (bytesAvail)
					{
						if (bytesAvail > 1023)
						{
							ReadFile(outputPipeR, stdOutBuffer, 1024, &numBytesRead, false);
							stdOutBuffer[1023] = 0;
						}
						else
						{
							ReadFile(outputPipeR, stdOutBuffer, bytesAvail, &numBytesRead, false);
							stdOutBuffer[bytesAvail] = 0;
						}

						LOGINFO(logRef, "%s", stdOutBuffer);
					}
				}

				// Pipe stderr
				while (PeekNamedPipe(errorPipeR, NULL, 0, NULL, &bytesAvail, &bytesLeft) && bytesAvail > 0)
				{
					DWORD numBytesRead;
					if (bytesAvail)
					{
						if (bytesAvail > 1023)
						{
							ReadFile(errorPipeR, stdOutBuffer, 1024, &numBytesRead, false);
							stdOutBuffer[1023] = 0;
						}
						else
						{
							ReadFile(errorPipeR, stdOutBuffer, bytesAvail, &numBytesRead, false);
							stdOutBuffer[bytesAvail] = 0;
						}

						if (stdErrToStdOut)
							LOGINFO(logRef, "%s", stdOutBuffer);
						else
							LOGERROR(logRef, "%s", stdOutBuffer);
					}
				}

				timeWaited += 50;

				if (timeoutInterval && timeWaited > timeoutInterval)
				{
					if (pLogger)
					{
						Debug::Logger& logRef(*pLogger);
						LOGERROR(logRef, kTimoutMessage);
					}
					TerminateProcess(pi.hProcess, 1);
					exitCode = -1;
					break;
				}

				if (cancelFunc && cancelFunc(pContext))
				{
					if (pLogger)
					{
						Debug::Logger& logRef(*pLogger);
						LOGERROR(logRef, "Command cancelled.\n");
					}
					TerminateProcess(pi.hProcess, 1);
					exitCode = -1;
					break;
				}
			}

			delete[] stdOutBuffer;
		}
		else 
		{
			int waitExitCode;
			if (timeoutInterval)
				waitExitCode = WaitForSingleObject(pi.hProcess, timeoutInterval);
			else
				waitExitCode = WaitForSingleObject(pi.hProcess, INFINITE);

			DWORD dwExitCode;
			GetExitCodeProcess(pi.hProcess, &dwExitCode);
			exitCode = (int)dwExitCode;

			if (waitExitCode == WAIT_TIMEOUT)
			{
				if (pLogger)
				{
					Debug::Logger& logRef(*pLogger);
					LOGERROR(logRef, kTimoutMessage);
				}
				TerminateProcess(pi.hProcess, 1);
			}
		}

		DWORD dwExitCode;
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
		exitCode = (int)dwExitCode;		
	} else {
        DWORD iReturnVal = GetLastError();

		if (pLogger)
		{
			char errorBuffer[512];
			memset(errorBuffer, 0, sizeof(errorBuffer));
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, iReturnVal, 0, errorBuffer, 512, 0);
			Debug::Logger& logRef(*pLogger);
			LOGERROR(logRef, "An error occurred executing the command. Error %d : %s", iReturnVal, errorBuffer);
		}

		exitCode = -1;
	}

	if (pProgress)
		pProgress->ProgressNotify(0, 100.f, pszProgressName?pszProgressName:command);

	if (pLogger)
	{
		Debug::Logger& logRef(*pLogger);
		LOGDEBUG(logRef, "Exiting with code %i\n", exitCode);
	}

	if (processCommand)
		delete[] processCommand;

	if (processWorkingDir)
		delete[] processWorkingDir;

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	if (pLogger && logOutput)
	{
		CloseHandle(outputPipeR);
		CloseHandle(outputPipeW);
	}

	return exitCode;
}

#else

S32 ExecuteCommandAndWait(
	const char* command, 
	const char* parameters, 
	const char* workingDir,
	E_LaunchState launchState,
	Debug::Logger* pLogger,
	Progress::I_ProgressBar *pProgress,
	const char *pszProgressName,
	U32 timeoutInterval,
	bool logOutput,
	bool stdErrToStdOut,
	CheckForCancelCallback cancelFunc,
	void* pContext)
{
	BIZ_ASSERT_FAIL("Not yet implemented for this platform.");
	return 1;
}

#endif

}
}
