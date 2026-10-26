#include "stdafx.h"
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <stdio.h>
#include "Vis_Core/mdump.h"

#include "Tools_Common\SendEmail.h"

LPCSTR m_szAppName;

bool gbSilentMode = false;

void minidump_Init()
{
	m_szAppName = "VIS";

	::SetUnhandledExceptionFilter( TopLevelFilter );
}

LONG WINAPI TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
	HWND hParent = NULL;						// find a better value for your app

	char buffer[512];
	LOGINFO( g_Log, "exception number: %x\n",pExceptionInfo->ExceptionRecord->ExceptionCode);

	bool bSilent = gbSilentMode;
/*
	// if we're in server mode, don't put up a box, but just quit out
	if (g_useNetworkClients)
	{
		bSilent = true;
	}
*/
	// don't handle CTRL+C
	if (pExceptionInfo->ExceptionRecord->ExceptionCode == DBG_CONTROL_C)
		return EXCEPTION_CONTINUE_SEARCH;

	// catch 'out of memory' error separately
	if (pExceptionInfo->ExceptionRecord->ExceptionCode == -529697949)
	{
		if (!bSilent)
			::MessageBox( NULL, "The PVS tool has run out of memory.\nClick 'OK' to exit this process now.", "Unhandled Exception", MB_OK|MB_ICONERROR );
		else
			LOGERROR( g_Log, "Out of memory. Exiting silently.\n");
		exit(-1);
	}

	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	HMODULE hDll = NULL;
	char szDbgHelpPath[_MAX_PATH];

	if (GetModuleFileName( NULL, szDbgHelpPath, _MAX_PATH ))
	{
		char *pSlash = _tcsrchr( szDbgHelpPath, '\\' );
		if (pSlash)
		{
			_tcscpy( pSlash+1, "DBGHELP.DLL" );
			hDll = ::LoadLibrary( szDbgHelpPath );
		}
	}

	if (hDll==NULL)
	{
		// load any version we can
		hDll = ::LoadLibrary( "DBGHELP.DLL" );
	}

	LPCTSTR szResult = NULL;

	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if (pDump)
		{
			char szDumpPath[_MAX_PATH];
			char szScratch [_MAX_PATH];

			// work out a good place for the dump file
//			if (!GetTempPath( _MAX_PATH, szDumpPath ))
				_tcscpy( szDumpPath, "c:\\" );

			_tcscat( szDumpPath, m_szAppName );
			_tcscat( szDumpPath, ".dmp" );

			// create the file
			HANDLE hFile = ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
										FILE_ATTRIBUTE_NORMAL, NULL );

			if (hFile!=INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

				ExInfo.ThreadId = ::GetCurrentThreadId();
				ExInfo.ExceptionPointers = pExceptionInfo;
				ExInfo.ClientPointers = NULL;

				// write the dump
				BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs /*MiniDumpNormal*/, &ExInfo, NULL, NULL );
				if (bOK)
				{
					sprintf( szScratch, "Saved minidump file to '%s'.", szDumpPath);
					szResult = szScratch;
					retval = EXCEPTION_EXECUTE_HANDLER;
				}
				else
				{
					sprintf( szScratch, "Failed to save dump file to '%s' (error %d)", szDumpPath, GetLastError() );
					szResult = szScratch;
				}
				::CloseHandle(hFile);
			}
			else
			{
				sprintf( szScratch, "Failed to create dump file '%s' (error %d)", szDumpPath, GetLastError() );
				szResult = szScratch;
			}

			szResult = NULL;

			if (!bSilent)
			{
				if (::MessageBox( NULL, "An error occured in the VIS tool, and a diagnostic file has been saved.\n"
									"You will now be asked to send an e-mail with information about the crash.", "Unhandled Exception", MB_OKCANCEL|MB_ICONERROR )==IDOK)
				{
					Tu::Status status;
					Tu::SendEmail mail(status);

					mail.SetBody("<enter details about the crash here>");
					mail.SetSubject("VIS Tool crash report");
					mail.AttachFile(szDumpPath);

					status = mail.Send(true);
					szResult = NULL;
				}
			}
			else
			{
				LOGFATAL( g_Log, "An error occured in the VIS tool, and a diagnostic file has been saved.");
			}
		}
		else
		{
			szResult = "DBGHELP.DLL too old";
		}
	}
	else
	{
		szResult = "DBGHELP.DLL not found";
	}

	if (szResult)
	{
		if (!bSilent)
			::MessageBox( NULL, szResult, "Message", MB_OK );
		else
		{
			LOGINFO( g_Log, szResult);
		}
	}

	return retval;
}

