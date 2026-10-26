// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines platform specific settings for Win32.

#ifndef BD_PLATFORM_CONFIG_WIN32_H
#define BD_PLATFORM_CONFIG_WIN32_H

#define BD_PLATFORM_WIN32

//
// Detect the compiler.
//

#define BD_STRINGIFY(X) #X
#define BD_STRINGIFY_EXP(X) BD_STRINGIFY(X)

#if defined (_MSC_VER)

#	if (_MSC_VER < 1200)
#		pragma message (__FILE__ "(" BD_STRINGIFY_EXP(__LINE__) ") : Build not tested on Microsoft Compiler (_MSC_VER = " BD_STRINGIFY_EXP(_MSC_VER) ")\n")
#	endif

#	if (_MSC_VER < 1300)
#		include <bdPlatform/bdPlatformConfig/bdPlatformConfig-win32-msvc6.h>
#	elif(_MSC_VER >= 1300 && _MSC_VER < 1400)
#		include <bdPlatform/bdPlatformConfig/bdPlatformConfig-win32-msvc7.h>
#	elif(_MSC_VER >= 1400)
#		include <bdPlatform/bdPlatformConfig/bdPlatformConfig-win32-msvc8.h>
#	endif

#else
#	error "Compiler is not supported"
#endif // _MSC_VER

#define BD_DEFAULT_STACKSIZE (0)
#define BD_DEFAULT_THREAD_PRIORITY (0)

#define BD_BREAKPOINT() DebugBreak()

#endif // BD_PLATFORM_CONFIG_WIN32_H

