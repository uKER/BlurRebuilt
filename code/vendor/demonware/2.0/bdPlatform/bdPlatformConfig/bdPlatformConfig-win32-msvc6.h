// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines platform specific settings for Win32 compiled with Microsoft
//			Visual Studio 6.

#ifndef BD_PLATFORM_CONFIG_WIN32_MSVC6_H
#define BD_PLATFORM_CONFIG_WIN32_MSVC6_H

#define BD_COMPILER_MSVC
#define BD_COMPILER_MSVC6

// Detect build configuration
#if defined(_DEBUG)
#	define BD_DEBUG
#else
#	define BD_RELEASE
#endif

#define BD_CALL __cdecl

// native types
typedef int				BD_INT;
typedef unsigned int	BD_UINT;

typedef long			BD_LONG;
typedef unsigned long	BD_ULONG;

typedef int				BD_WORD;
typedef unsigned int	BD_UWORD;

typedef char			BD_PLATFORM_CHAR;

// fixed width types

typedef char			BD_CHAR8;
typedef unsigned char	BD_UCHAR8;

typedef __int64				BD_INT_64;
typedef unsigned __int64	BD_UINT_64;

typedef __int64				BD_LONG_64;
typedef unsigned __int64	BD_ULONG_64;

#define BD_INT_64_FORMAT "%I64i"
#define BD_UINT_64_FORMAT "%I64u"

#define BD_ALIGN_VARIABLE(name, align) __declspec(align(align)) name

# if !defined (_INC_WINDOWS)     // Already include windows.h ?
// Must define strict before including windows.h !
#  if !defined (STRICT)
#   define STRICT 1
#  endif // STRICT
#  if !defined (WIN32_LEAN_AND_MEAN)
#   define WIN32_LEAN_AND_MEAN
#  endif // !defined (WIN32_LEAN_AND_MEAN)
# endif // !defined (_INC_WINDOWS)

#include <windows.h>

#define bdThreadHandle HANDLE
#define bdThreadProc   LPTHREAD_START_ROUTINE
#define bdMutexHandle  HANDLE
#define bdSemaphoreHandle HANDLE
#define bdSignalHandle HANDLE

#endif // BD_PLATFORM_CONFIG_WIN32_MSVC6_H

