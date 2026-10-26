#ifndef __MERCURY_TOOLS_MDUMP_H__
#define __MERCURY_TOOLS_MDUMP_H__

#ifdef COMPILER_VC
#include <windows.h>
#include "dbghelp.h"
// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);
#else
struct _EXCEPTION_POINTERS
{
	int iDummy;
};
#endif

namespace Tools
{

/// handle exceptions and write out .dmp files
class MiniDumper
{
public:
	/// constructor
	MiniDumper();

	/// initialise the exceptionhandler
	static void InitialiseExceptionHandler();

	/// write a .dmp file. can be called at any time
	/// returns true if the .dmp file was successfully written
	static bool WriteDumpFile(struct _EXCEPTION_POINTERS *pExceptionInfo = NULL);

	/// destructor
	~MiniDumper();

private:
#ifdef COMPILER_VC
	/// exception handler
	static LONG WINAPI TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo );
#endif
};

/// minidumper global singleton
extern MiniDumper g_MiniDumper;

};


#endif
