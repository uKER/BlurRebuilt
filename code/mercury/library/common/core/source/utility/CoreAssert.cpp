#include "CorePch.h"
#include <core/utility/coredebug.h>

#ifdef COMPILER_VC
#include <crtdbg.h>
#ifdef PLATFORM_PC
#include <windows.h>
#endif
#endif
#ifdef PLATFORM_PS3
#include <libsn.h>
#endif

// core's default assert function
bool CoreAssert(const char *pszFunction, const char *pszExpression, const char *pszFilename, int iLineNumber)
{
	CORE_PRINT("Assert(%s) failed in %s(%d) [%s]\n",
		pszExpression,
		pszFilename,
		iLineNumber,
		pszFunction);

#ifndef CFG_SPEED

#ifdef PLATFORM_PC
#if defined(COMPILER_VC) && defined(BUILD_CONFIG_DEBUG)
	// on the PC, in DEBUG mode, bring up the assert dialog box
	// (in RELEASE mode CrtDbgReport doesn't exist)
       if (_CrtDbgReport(_CRT_ASSERT, pszFilename, iLineNumber, NULL, pszExpression))
		   _CrtDbgBreak();
#else
	if (IsDebuggerPresent())
		DebugBreak();
#endif
#endif

#if (defined(PLATFORM_PS3))
	if (snIsDebuggerPresent())
		snPause();
#endif
#endif
	
	return true;
}


#ifdef CFG_GOLD

bool EmptyAssertFunction(const char*,const char*,const char*,int)
{
	return true;
}

AssertFunction *g_pCoreAssertFunction = EmptyAssertFunction;

#else

AssertFunction *g_pCoreAssertFunction = CoreAssert;

namespace CoreDebug
{

bool IsDebuggerAttached()
{
#ifdef PLATFORM_PC
	return (IsDebuggerPresent() == TRUE);
#endif


#ifdef PLATFORM_PS3
	return (snIsDebuggerPresent() == TRUE);
#endif

	return false;
}

}

#endif
