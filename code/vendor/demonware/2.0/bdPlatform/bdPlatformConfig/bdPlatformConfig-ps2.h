// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines platform specific settings for PS2.

#ifndef BD_PLATFORM_CONFIG_PS2_H
#define BD_PLATFORM_CONFIG_PS2_H

#define BD_PLATFORM_PS2


//
// Detect the compiler.
//
#if defined(__GNUC__) || defined(__MWERKS__)
#    define BD_COMPILER_GCC
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

#define BD_CHAR8	char
#define BD_UCHAR8	unsigned char

#define BD_INT_64	long
#define BD_UINT_64	unsigned long

#define BD_LONG_64	long
#define BD_ULONG_64	unsigned long

#define BD_INT_64_FORMAT "%lli"
#define BD_UINT_64_FORMAT "%llu"

// native types
#define BD_INT		int
#define BD_UINT		unsigned int

#define BD_LONG		long
#define BD_ULONG	unsigned long

#define BD_PLATFORM_CHAR char

/// xxx: make sure the following is correct:
#define BD_WORD		int
#define BD_UWORD	unsigned int

#define BD_ALIGN_VARIABLE(name, align) name __attribute__((aligned(align)))

#include <eekernel.h>

#define BD_DEFAULT_STACKSIZE (8192)
#define BD_DEFAULT_THREAD_PRIORITY (1)
typedef int bdThreadHandle;
typedef void (*bdThreadProc)(void*);
typedef int bdDNSHandle;
typedef int bdMutexHandle;
typedef int bdSemaphoreHandle;
typedef int bdSignalHandle;

#define BD_BREAKPOINT() __asm( "break 1" )

#endif // BD_PLATFORM_CONFIG_PS2_H

