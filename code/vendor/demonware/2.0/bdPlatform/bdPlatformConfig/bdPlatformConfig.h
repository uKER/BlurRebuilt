// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines platform specific settings.

#ifndef BD_PLATFORM_CONFIG_H
#define BD_PLATFORM_CONFIG_H

#include <bdPlatform/bdPlatformConfig/bdSettings.h>

//
// Detect supported platform.
//
#if defined(_WIN32) && !defined(_XBOX)
#	include <bdPlatform/bdPlatformConfig/bdPlatformConfig-win32.h>
#elif defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#	include <bdPlatform/bdPlatformConfig/bdPlatformConfig-unix.h>
#elif defined(__psp__)
#	include <bdPlatform/bdPlatformConfig/bdPlatformConfig-psp.h>
#elif defined(__mips__) || defined(__MIPS__)
#	include <bdPlatform/bdPlatformConfig/bdPlatformConfig-ps2.h>
#elif defined(GEKKO) 
#	include <bdPlatform/bdPlatformConfig/bdPlatformConfig-wii.h>
#elif defined(_XBOX)
#	if ( _XBOX_VER >= 200 )
#		include <bdPlatform/bdPlatformConfig/bdPlatformConfig-xenon.h>
#	else
#		include <bdPlatform/bdPlatformConfig/bdPlatformConfig-xbox.h>
#	endif
#elif defined(__PPU__)
#	include <bdPlatform/bdPlatformConfig/bdPlatformConfig-ps3-ppu.h>
#else
#	error Unknown platform, not supported.
#endif

// define this if running on a little-endian CPU
// little endian will be assumed if BD_BIG_ENDIAN is not defined
#ifndef BD_BIG_ENDIAN
#   define BD_LITTLE_ENDIAN
#endif

// BD_DEBUG or BD_RELEASE must be defined
#if (!defined(BD_DEBUG) && !defined(BD_RELEASE))
#	error Neither BD_DEBUG or BD_RELEASE are defined.
#endif // (!BD_DEBUG && !BD_RELEASE)

//
// BD_DEBUG Settings
//

#ifdef BD_DEBUG

/// Enable throwing of asserts.
#	define BD_THROW_ASSERTS

#endif // BD_DEBUG


//
// BD_RELEASE Settings
//

#ifdef BD_RELEASE

// Uncomment to enable asserts in release
//#	define BD_THROW_ASSERTS

#endif // BD_RELEASE

//
// Listed here are all the possible bdPlatformConfig*.h #defines
//
//
// - Build Configuration -
// BD_DEBUG
// BD_RELEASE
//
// - Platforms -
// BD_PLATFORM_WIN32
// BD_PLATFORM_PS2
// BD_PLATFORM_UNIX
//
// - Compilers -
// BD_COMPILER_MSVC
// BD_COMPILER_MSVC6
// BD_COMPILER_MSVC7
// BD_COMPILER_MSVC8
// BD_COMPILER_GCC
//
// - Endianness -
// BD_BIG_ENDIAN
// BD_LITTLE_ENDIAN
//
// - Calling convention -
// BD_CALL
//
// - Errors and warnings -
// BD_THROW_ASSERTS
// BD_BREAKPOINT
//
// - Types -
// BD_INT_64
// BD_UINT_64
//
// BD_LONG_64
// BD_ULONG_64
//
// - printf formats -
// BD_INT_64_FORMAT
// BD_UINT_64_FORMAT
//
// - alignment -
// BD_ALIGN_VARIABLE
//

#endif // BD_PLATFORM_CONFIG_H

