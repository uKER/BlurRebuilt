// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines platform specific settings for PSP.

#ifndef BD_PLATFORM_CONFIG_PSP_H
#define BD_PLATFORM_CONFIG_PSP_H

#define BD_PLATFORM_PSP

//
// Detect the compiler.
//
#if defined(__GNUC__) || defined(__MIPS__)
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

typedef char BD_CHAR8;
typedef unsigned char BD_UCHAR8;

typedef long long BD_INT_64;
typedef unsigned long long BD_UINT_64;

typedef long long BD_LONG_64;
typedef unsigned long long BD_ULONG_64;

#define BD_INT_64_FORMAT "%lli"
#define BD_UINT_64_FORMAT "%llu"

// native types
typedef int BD_INT;
typedef unsigned int BD_UINT;

typedef long BD_LONG;
typedef unsigned long BD_ULONG;

typedef char BD_PLATFORM_CHAR;

/// xxx: make sure the following is correct:
typedef int BD_WORD;
typedef unsigned int BD_UWORD;

#include <kerneltypes.h>
#include <kernel.h>

#define BD_DEFAULT_STACKSIZE (0x2000)
#define BD_DEFAULT_THREAD_PRIORITY SCE_KERNEL_USER_HIGHEST_PRIORITY
typedef SceUID bdThreadHandle;
typedef int (*bdThreadProc)(SceSize, void*);
typedef int bdDNSHandle;
typedef int bdMutexHandle;
typedef int bdSemaphoreHandle;
typedef int bdSignalHandle;

#define BD_BREAKPOINT() ::abort()

#endif // BD_PLATFORM_CONFIG_PSP_H

