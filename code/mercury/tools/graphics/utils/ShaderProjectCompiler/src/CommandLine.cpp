#include "StdAfx.h"
#include "CommandLine.h"

int LaunchCommandLine(char* pCommandLine, char* pOutputBuffer, int outputBufferSize)
{
	pOutputBuffer[0] = '\0';
	
	HANDLE hOutputReadTmp, hOutputRead, hOutputWrite;
	HANDLE hInputWriteTmp, hInputRead, hInputWrite;
	HANDLE hErrorWrite;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if(!CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 0))
	{
		DBError("Creating output pipe\n");
		return 1;
	}

	if(!DuplicateHandle(GetCurrentProcess(), hOutputWrite, GetCurrentProcess(), &hErrorWrite, 0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		DBError("Duplicating output write handle\n");
		return 1;
	}

	if(!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 0))
	{
		DBError("Creating input pipe\n");
		return 1;
	}

	if(!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(), &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		DBError("Duplicating output read handle\n");
		return 1;
	}

	if(!DuplicateHandle(GetCurrentProcess(), hInputWriteTmp, GetCurrentProcess(), &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		DBError("Duplicating input write handle\n");
		return 1;
	}

	if(!CloseHandle(hOutputReadTmp))
	{
		DBError("Closing output read handle\n");
		return 1;
	}

	if(!CloseHandle(hInputWriteTmp))
	{
		DBError("Closing input write handle\n");
		return 1;
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));

	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hOutputWrite;
	si.hStdInput = hInputRead;
	si.hStdError = hErrorWrite;
	
	PROCESS_INFORMATION pi;
	if(CreateProcess(0, pCommandLine, 0, 0, TRUE, 0, 0, 0, &si, &pi) == 0)
	{
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			0,
			(LPTSTR)&lpMsgBuf,
			0,
			NULL 
		);
		DBError("Creating process %s %s\n", (LPCTSTR)lpMsgBuf, pCommandLine);
		return 1;
		
	}
	
	HANDLE hChildProcess = pi.hProcess;

	WaitForSingleObject(hChildProcess, INFINITE);

	DWORD exitCode = 666;
	if(GetExitCodeProcess(hChildProcess, &exitCode) == 0)
	{
		DBError("Getting exit code from handle\n");
		return 1;
	}

	if(!hChildProcess)
	{
		DBError("Creating process\n");
		return 1;
	}

	if(!CloseHandle(hOutputWrite))
	{
		DBError("Closing output write handle\n");
		return 1;
	}

	if(!CloseHandle(hInputRead))
	{
		DBError("Closing input read handle\n");
		return 1;
	}

	if(!CloseHandle(hErrorWrite))
	{
		DBError("Closing error write handle\n");
		return 1;
	}

	DWORD numberOfBytesToRead = outputBufferSize-4;
	DWORD numberOfBytesRead;
	ReadFile(hOutputRead, pOutputBuffer, numberOfBytesToRead, &numberOfBytesRead, NULL);
	pOutputBuffer[numberOfBytesRead] = '\0';
	
	if(!CloseHandle(hOutputRead))
	{
		DBError("Closing output read handle\n");
		return 1;
	}

	if(!CloseHandle(hInputWrite))
	{
		DBError("Closing input write handle\n");
		return 1;
	}

	return exitCode;
}
