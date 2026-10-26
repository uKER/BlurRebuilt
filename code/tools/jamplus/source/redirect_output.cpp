// redirect output from gnu-c and convert the error message output to VC format
#define _CRT_SECURE_NO_DEPRECATE

#include "jam.h"
#include<windows.h>
#pragma comment(lib, "User32.lib")

#include <string>
#include <iostream>
#include "redirect_output.h"
#include "gnu_vc_errorconv.h"

RedirectedProcess::RedirectedProcess()
{
	m_hChildProcess = NULL;
}

// returns the handle to this process
HANDLE RedirectedProcess::GetHandle()
{
	return m_hChildProcess;
}

void RedirectedProcess::RunCommandlineRedirect(const std::string &commandline, bool bConvertErrors)
{
	m_hChildProcess = NULL;
	m_iBufPos = 0;
	m_bConvertErrors = bConvertErrors;

	DWORD iExitCode = 0;

	PROCESS_INFORMATION	Pi;
	STARTUPINFO			Si;

	ZeroMemory(&Pi, sizeof(Pi));
	ZeroMemory(&Si, sizeof(Si));

	m_iLine = 0;

	Si.cb = sizeof(Si);

	char buffer[32000];
	strcpy(buffer, commandline.c_str());

    HANDLE hOutputReadTmp;
    HANDLE hInputWriteTmp;
    SECURITY_ATTRIBUTES sa;

    // Set up the security attributes struct.
    sa.nLength= sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;


    // Create the child output pipe.
    if (!CreatePipe(&hOutputReadTmp,&m_hOutputWrite,&sa,0))
        DisplayError("CreatePipe");


    // Create a duplicate of the output write handle for the std error
    // write handle. This is necessary in case the child application
    // closes one of its std output handles.
    if (!DuplicateHandle(GetCurrentProcess(),m_hOutputWrite,
                        GetCurrentProcess(),&m_hErrorWrite,0,
                        TRUE,DUPLICATE_SAME_ACCESS))
        DisplayError("DuplicateHandle");


    // Create the child input pipe.
	m_hInputRead = CreateFile("nul", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, 0);


    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if (!DuplicateHandle(GetCurrentProcess(),hOutputReadTmp,
                        GetCurrentProcess(),
                        &m_hOutputRead, // Address of new handle.
                        0,FALSE, // Make it uninheritable.
                        DUPLICATE_SAME_ACCESS))
        DisplayError("DuplicateHandle");

    // Close inheritable copies of the handles you do not want to be
    // inherited.
    if (!CloseHandle(hOutputReadTmp)) DisplayError("CloseHandle");

	m_iStartTime = GetTickCount();

    PrepAndLaunchRedirectedChild(m_hOutputWrite,m_hInputRead,m_hErrorWrite,buffer);

    // Close pipe handles (do not continue to modify the parent).
    // You need to make sure that no handles to the write end of the
    // output pipe are maintained in this process or else the pipe will
    // not close when the child process exits and the ReadFile will hang.
    if (!CloseHandle(m_hOutputWrite)) DisplayError("CloseHandle");
    if (!CloseHandle(m_hErrorWrite)) DisplayError("CloseHandle");
}

DWORD RedirectedProcess::Close()
{
    // Redirection is complete
    if (!CloseHandle(m_hOutputRead)) DisplayError("CloseHandle");

	// propagate bad exit codes
	DWORD dwExitCode = -1;
	BOOL bOk = GetExitCodeProcess( m_hChildProcess, &dwExitCode );

	m_hChildProcess = NULL;

	return dwExitCode;
}


///////////////////////////////////////////////////////////////////////
// PrepAndLaunchRedirectedChild
// Sets up STARTUPINFO structure, and launches redirected child.
///////////////////////////////////////////////////////////////////////
void RedirectedProcess::PrepAndLaunchRedirectedChild(HANDLE hChildStdOut,
                                    HANDLE hChildStdIn,
                                    HANDLE hChildStdErr,
									char *buffer)
{
    PROCESS_INFORMATION pi;
    STARTUPINFO si;

    // Set up the start up info struct.
    ZeroMemory(&si,sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hChildStdOut;
	si.hStdInput  = hChildStdIn;
	si.hStdError  = hChildStdErr;
    // Use this if you want to hide the child:
    //     si.wShowWindow = SW_HIDE;
    // Note that dwFlags must include STARTF_USESHOWWINDOW if you want to
    // use the wShowWindow flags.


    // Launch the process that you want to redirect (in this case,
    // Child.exe). Make sure Child.exe is in the same directory as
    // redirect.c launch redirect from a command line to prevent location
    // confusion.



	HRESULT hr;
	if (hr = CreateProcess(0, buffer, 0, 0, TRUE, 0, 0, 0, &si, &pi) != FALSE)
	{
		// Set global child process handle to cause threads to exit.
		m_hChildProcess = pi.hProcess;
	}
}


std::string RedirectedProcess::GetOutput()
{
	m_output.flush();
	std::string tmp = m_output.str();
	m_output.str("");
	return tmp;
}

void RedirectedProcess::StopTiming()
{
	U64 iDuration = GetTickCount() - m_iStartTime;

	if (globs.showdurations)
	{
		m_output << "Duration: " << iDuration << "\n";
	}
}

///////////////////////////////////////////////////////////////////////
// ReadAndHandleOutput
// Monitors handle for input. Exits when child exits or pipe breaks.
///////////////////////////////////////////////////////////////////////
bool RedirectedProcess::ReadAndHandleOutput()
{
	HANDLE hPipeRead = m_hOutputRead;
	HANDLE hChildProcess = m_hChildProcess;
    DWORD nBytesRead;

	if (!hChildProcess)
		return false;

    while(TRUE)
    {
		CHAR lpBuffer[256];

		DWORD nBytesRead, nBytesAvail, nBytesLeft;
		if (!PeekNamedPipe(hPipeRead, lpBuffer, 0, &nBytesRead, &nBytesAvail, &nBytesLeft))
			return false;
		if (nBytesAvail == 0)
			return false;

        if (!ReadFile(hPipeRead,lpBuffer,sizeof(lpBuffer),
                                        &nBytesRead,NULL) || !nBytesRead)
        {
			return false; // the process exited
        }

		if (nBytesRead == 0)
			return false; // end of pipe

		// convert gcc errors to vc
		for (unsigned int i=0; i<nBytesRead; i++)
		{
			if (lpBuffer[i] == 0x0a || lpBuffer[i] == 0x0d && m_iBufPos>0)
			{
				// flush line of text to converter
				m_line_buf[m_iBufPos] = 0;
				if (m_line_buf[0] != 0)
				{
					// suppress annoying 'terminate batch job' messages
					if (!strstr(m_line_buf, "Terminate batch job (Y/N)? "))
					{
						if (m_bConvertErrors)
						{
							std::string outstr = gnu_vc_errorconv( m_line_buf );
							m_output << outstr << "\n";
						}
						else
						{
							if (m_iLine == 0)
							{
								m_sFirstLine = m_line_buf;
								m_output << m_line_buf << std::endl;
							}
							else if (m_iLine == 1 && strstr(m_sFirstLine.c_str(), m_line_buf))
							{
								// if the 2nd line of output was the same as the first, don't display it
							}
							else
								m_output << m_line_buf << std::endl;
							m_iLine++;
						}
					}
				}

				m_iBufPos = 0;
				continue;
			}

			// copy characters
			m_line_buf[m_iBufPos] = lpBuffer[i];
			if (m_iBufPos<sizeof(m_line_buf)-3)
				m_iBufPos++;
		}
    }

	return true;
}


///////////////////////////////////////////////////////////////////////
// DisplayError
// Displays the error number and corresponding message.
///////////////////////////////////////////////////////////////////////
void RedirectedProcess::DisplayError(char *pszAPI)
{
    LPVOID lpvMessageBuffer;
    CHAR szPrintBuffer[512];
    DWORD nCharsWritten;

    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpvMessageBuffer, 0, NULL);

    wsprintf(szPrintBuffer,
        "ERROR: API    = %s.\n   error code = %d.\n   message    = %s.\n",
            pszAPI, GetLastError(), (char *)lpvMessageBuffer);

    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE),szPrintBuffer,
                    lstrlen(szPrintBuffer),&nCharsWritten,NULL);

    LocalFree(lpvMessageBuffer);
    ExitProcess(GetLastError());
}
