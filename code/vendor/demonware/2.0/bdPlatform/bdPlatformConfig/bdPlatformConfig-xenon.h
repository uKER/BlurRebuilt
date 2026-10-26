// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines platform specific settings for XBox compiled with Microsoft
//			Visual Studio.

#ifndef BD_PLATFORM_CONFIG_XENON_H
#define BD_PLATFORM_CONFIG_XENON_H

#define BD_PLATFORM_XENON
#define BD_COMPILER_MSVC_XENON

// Detect build configuration
#if defined(_DEBUG)
#	define BD_DEBUG
#else
#	define BD_RELEASE
#endif

#define BD_CALL __cdecl

// native types
#ifdef  _WIN64
typedef	long				BD_LONG;
#else
typedef	__w64 long			BD_LONG;
#endif

#ifdef  _WIN64
typedef unsigned long		BD_ULONG;
#else
typedef __w64 unsigned long	BD_ULONG;
#endif

#ifdef  _WIN64
typedef __int64				BD_WORD;
#else
typedef __w64 int			BD_WORD;
#endif

#ifdef  _WIN64
typedef unsigned __int64	BD_UWORD;
#else
typedef __w64 unsigned int	BD_UWORD;
#endif

typedef char				BD_PLATFORM_CHAR;

// fixed width types

typedef char				BD_CHAR8;
typedef unsigned char		BD_UCHAR8;

typedef __int64				BD_INT_64;
typedef unsigned __int64	BD_UINT_64;

typedef __int64				BD_LONG_64;
typedef unsigned __int64	BD_ULONG_64;

#define BD_INT_64_FORMAT "%I64i"
#define BD_UINT_64_FORMAT "%I64u"

#define BD_ALIGN_VARIABLE(name, align) __declspec(align(align)) name

#define BD_BIG_ENDIAN

#pragma warning( push )
# pragma warning(disable : 4311 4312)
#include <Xtl.h>
#include <winsockx.h>
#pragma warning( pop )

#define BD_DEFAULT_STACKSIZE (0)
#define BD_DEFAULT_THREAD_PRIORITY (0)

typedef HANDLE bdThreadHandle;
typedef LPTHREAD_START_ROUTINE bdThreadProc;
typedef int bdDNSHandle;
typedef HANDLE bdMutexHandle;
typedef HANDLE bdSemaphoreHandle;
typedef HANDLE bdSignalHandle;

#define BD_BREAKPOINT() DebugBreak()

#endif // BD_PLATFORM_CONFIG_XENON_H

