// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines platform specific settings for Unix.

#ifndef BD_PLATFORM_CONFIG_UNIX_H
#define BD_PLATFORM_CONFIG_UNIX_H

#include <stddef.h>

#define BD_PLATFORM_UNIX

//
// Detect the compiler.
//
#if defined(__GNUC__)
#	define BD_COMPILER_GCC
#else
#	error Could not detect compiler
#endif

// Detect build configuration
#if defined(NDEBUG)
#	define BD_RELEASE
#else
#	define BD_DEBUG
#endif

// Detect the OS.  The hierarchy is
//                UNIX                                                        #
//               /    \                                                       #
//           APPLE    LINUX                                                   #
//          /     \                                                           #
//        OSX     IPHONE                                                      #
//               /      \                                                     #
//        IPHONEOS      IPHONESIMULATOR                                       #
#if defined(__APPLE__)
#	define BD_PLATFORM_APPLE
#	include "TargetConditionals.h"
#	if TARGET_OS_IPHONE
#		define BD_PLATFORM_IPHONE
#		if TARGET_IPHONE_SIMULATOR
#			define BD_PLATFORM_IPHONESIMULATOR
#		else
#			define BD_PLATFORM_IPHONEOS
#		endif
#	else
#		define BD_PLATFORM_OSX
#	endif
#elif defined(__linux)
#	define BD_PLATFORM_LINUX
#else
#	error Could not detect OS
#endif

#if defined(__i386__)
#	define BD_CALL __attribute__((__cdecl__))
#else
#	define BD_CALL // TODO: Investigate why all non-i386 don't like BD_CALL.
#endif

#define BD_ALIGN_VARIABLE(name, align) name __attribute__((aligned(align)))

// This was observed through trial-and-error.  It looks like BD_UWORD must be of the same type as size_t,
// otherwise we get major errors when overloading operator new.
#if defined(__i386__) && defined(BD_PLATFORM_LINUX)
typedef int					BD_WORD;
typedef unsigned int		BD_UWORD;
#else
typedef long int			BD_WORD;
typedef unsigned long int	BD_UWORD;
#endif

typedef	long				BD_LONG;
typedef unsigned long		BD_ULONG;

#define BD_CHAR8			char
#define BD_UCHAR8			unsigned char
#define BD_PLATFORM_CHAR	char

#define BD_INT_64			long long
#define BD_UINT_64			unsigned long long

#define BD_LONG_64			long long
#define BD_ULONG_64			unsigned long long

#define BD_INT_64_FORMAT	"%lli"
#define BD_UINT_64_FORMAT	"%llu"

#include <pthread.h>
#include <semaphore.h>

#define BD_DEFAULT_STACKSIZE 		(0)
#define BD_DEFAULT_THREAD_PRIORITY	(0)

typedef pthread_t bdThreadHandle;
typedef void *(*bdThreadProc)(void*);
typedef pthread_mutex_t bdMutexHandle;
typedef int bdDNSHandle;
typedef sem_t bdSemaphoreHandle;
typedef pthread_cond_t bdSignalHandle;

#if defined(__arm__)
#	define BD_BREAKPOINT() asm("bkpt 0x0001\n\t")
#else
#	define BD_BREAKPOINT() asm("int3")
#endif

#endif // BD_PLATFORM_CONFIG_UNIX_H
