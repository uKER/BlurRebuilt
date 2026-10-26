// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines platform specific settings for Wii.

#ifndef BD_PLATFORM_CONFIG_WII_H
#define BD_PLATFORM_CONFIG_WII_H

#define BD_PLATFORM_WII

//
// Detect the compiler.
//
#if defined(__CWCC__)
#    define BD_COMPILER_CODEWARRIOR
#else
#    error Could not detect compiler
#endif

// Detect build configuration
#if defined(NDEBUG)
#	define BD_RELEASE
#else
#	define BD_DEBUG
#endif

#define BD_CALL

#include <size_t.h>

typedef	long				BD_LONG;
typedef unsigned long		BD_ULONG;

typedef int		 			BD_WORD;
typedef std::size_t			BD_UWORD;

typedef char			BD_CHAR8;
typedef unsigned char	BD_UCHAR8;

typedef long long			BD_INT_64;
typedef unsigned long long	BD_UINT_64;

typedef long long			BD_LONG_64;
typedef unsigned long long	BD_ULONG_64;

#define BD_INT_64_FORMAT "%lli"
#define BD_UINT_64_FORMAT "%llu"

#define BD_PLATFORM_CHAR	char

#define BD_ALIGN_VARIABLE(name, align) name __attribute__ ((aligned (num)))

#define BD_BIG_ENDIAN

// XXX: what's the correct values here?
#define BD_DEFAULT_STACKSIZE (4096)
#define BD_DEFAULT_THREAD_PRIORITY (16)

#include <revolution/os.h>

typedef OSMutex bdMutexHandle;

typedef OSThread bdThreadHandle;
typedef void *(*bdThreadProc)(void*);

typedef int bdDNSHandle;
#define bdSignalHandle OSCond
#define bdSemaphoreHandle OSSemaphore

#define BD_BREAKPOINT() OSHalt("")

#endif // BD_PLATFORM_CONFIG_WII_H

