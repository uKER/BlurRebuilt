// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines platform specific settings for PS3.

#ifndef BD_PLATFORM_CONFIG_PS3_H
#define BD_PLATFORM_CONFIG_PS3_H

#if !defined BD_PLATFORM_PS3
#define BD_PLATFORM_PS3
#endif


//
// Detect the compiler.
//
#if defined(__GNUC__)
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


#if !defined __LP32__
// 64-bit ABI platform
#define BD_CHAR8	signed char
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

// Ordinary long is 8 bytes on ppu but 4 bytes on spu.
#define BD_LONG		long long
#define BD_ULONG	unsigned long long
#define BD_PLATFORM_CHAR	char

/// xxx: make sure the following is correct:
#define BD_WORD		long int
//long long
#define BD_UWORD	long unsigned int
//unsigned long long

#else // 64-bit ABI
// 32-bit ABI

#define BD_CHAR8	char
#define BD_UCHAR8	unsigned char

#define BD_INT_64	long long
#define BD_UINT_64	unsigned long long

#define BD_LONG_64	long long
#define BD_ULONG_64	unsigned long long

#define BD_INT_64_FORMAT "%lli"
#define BD_UINT_64_FORMAT "%llu"

// native types
#define BD_INT		int
#define BD_UINT		unsigned int

#define BD_LONG		long
#define BD_ULONG	unsigned long
#define BD_PLATFORM_CHAR	char

/// xxx: make sure the following is correct:
#define BD_WORD		 int
//long long
#define BD_UWORD	unsigned int
//unsigned long long
#endif

#define BD_ALIGN_VARIABLE(name, align) name __attribute__((aligned(align)))

#define BD_BIG_ENDIAN

// XXX: what's the correct values here?
#define BD_DEFAULT_STACKSIZE (4096)
#define BD_DEFAULT_THREAD_PRIORITY (1)

#include <sys/ppu_thread.h>
#include <pthread_types.h>
#include <semaphore.h>

typedef void (*bdThreadProc)(BD_ULONG_64);
typedef sys_ppu_thread_t bdThreadHandle;
typedef sys_ppu_thread_t bdDNSHandle;
typedef unsigned int bdMutexHandle;
typedef  sem_t bdSemaphoreHandle;
typedef  pthread_cond_t bdSignalHandle;

#define BD_BREAKPOINT() __asm__ volatile("tw 31,1,1")

#endif // BD_PLATFORM_CONFIG_PS3_H

