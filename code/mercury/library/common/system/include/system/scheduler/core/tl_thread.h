/** \file tl_thread.h
	\brief Operating system multithreading abstractions and utilities.
*/
#ifndef TL_THREAD_H
#define TL_THREAD_H

#include <core/threads/atomic.h>
#include <core/threads/misc.h>

//#undef DEPRECATED
//#define DEPRECATED

// TODO: some of the functions in this file are deprecated and have moved to core/thread/atomic.h

#if TL_PLAT_PC + 0 + TL_PLAT_PS3 + TL_PLAT_PS3_SPU + TL_PLAT_REV != 1
#error This module is only implemented for PC, Xenon and Playstation3 platforms.
#endif

#define UTILS

#if TL_PLAT_PC
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if TL_PLAT_PS3
#include <cell/atomic.h>
#include <sys/ppu_thread.h>
#include <sys/timer.h>
#include <sys/synchronization.h>
#include <sys/spinlock.h>
#endif
#if TL_PLAT_PS3_SPU
#include <cell/atomic.h>
#include <cell/spurs/policy_module.h>
#endif
#if TL_PLAT_REV
#include <revolution/base/PPCArch.h>
#include <revolution/os.h>
#endif

/** 
	\defgroup TL_THREAD_FUNCTIONS Thread functions
	@{
*/

/** \var tlThreadHandle 
	\brief Cross platform thread handle.
	The actual data type is a U64, to keep the size the same on all platforms.  
	It is interpreted as a sys_ppu_thread_t on Playstation3, and a HANDLE on PC/Xenon.
	On SPU, it is the SPU number (not the same as the SPU thread handle).
    On Revolution, it is a zero-extended OSThread pointer. 64 bit it is not.
*/
#if TL_PLAT_PC
typedef U64 /*HANDLE*/ tlThreadHandle;
#endif
#if TL_PLAT_PS3
typedef U64 /*sys_ppu_thread_t*/ tlThreadHandle;
#endif
#if TL_PLAT_PS3_SPU
typedef U64 tlThreadHandle;
#endif
#if TL_PLAT_REV
typedef U32 tlThreadHandle;
//#define RVL_MAX_THREADS 512
#define RVL_MAX_THREADS 4
extern OSThread _rvl_thread_table[RVL_MAX_THREADS];
extern U32 _rvl_thread_walker;
#endif

/** tlThreadId Cross platform thread ID.
	\note Thread IDs are the same as thread handles on Playstation3, but are different on Xenon (see the Xenon SDK documentation).
    On the Revolution, the thread ID is the index into the thread table.
*/
#if TL_PLAT_REV
typedef U32 tlThreadId;
#else
typedef U64 tlThreadId;
#endif

//! Default stack size for new threads.
#define TL_THREAD_DEFAULT_STACK_SIZE (32*1024)

//! Cross platform thread priority.
enum tlThreadPriority
{
#if TL_PLAT_PC
	TL_THREAD_PRIORITY_IDLE = THREAD_PRIORITY_LOWEST,
	TL_THREAD_PRIORITY_LOWEST = THREAD_PRIORITY_LOWEST,
	TL_THREAD_PRIORITY_BELOW_NORMAL = THREAD_PRIORITY_BELOW_NORMAL,
	TL_THREAD_PRIORITY_NORMAL = THREAD_PRIORITY_NORMAL,
	TL_THREAD_PRIORITY_ABOVE_NORMAL = THREAD_PRIORITY_ABOVE_NORMAL,
	TL_THREAD_PRIORITY_HIGHEST = THREAD_PRIORITY_HIGHEST,
	TL_THREAD_PRIORITY_TIME_CRITICAL = THREAD_PRIORITY_TIME_CRITICAL
#endif
#if TL_PLAT_PS3
	TL_THREAD_PRIORITY_IDLE          = 3071,  // 3071 is the lowest possible priority
	TL_THREAD_PRIORITY_LOWEST        = 3000,
	TL_THREAD_PRIORITY_BELOW_NORMAL  = 1500,
	TL_THREAD_PRIORITY_NORMAL        = 1000,  // 1001 is the default priority for the game process.
	TL_THREAD_PRIORITY_ABOVE_NORMAL  = 500,
	TL_THREAD_PRIORITY_HIGHEST       = 100,
	TL_THREAD_PRIORITY_TIME_CRITICAL = 1,     // 1 is the highest possible priority
#endif
#if TL_PLAT_PS3_SPU || TL_PLAT_REV
	TL_THREAD_PRIORITY_NORMAL        = 0,
#endif
};

//! Cross platform thread flags.
enum tlThreadFlags
{
#if TL_PLAT_PC
	TL_THREAD_JOINABLE         = 0,                //!< Pass this flag if you expect to call tlWaitForThread on the thread.
	TL_THREAD_CREATE_SUSPENDED = CREATE_SUSPENDED, //!< Creates the thread in suspended mode.  Not supported on PS3 or Revolution.
#endif
#if TL_PLAT_PS3
	TL_THREAD_JOINABLE         = SYS_PPU_THREAD_CREATE_JOINABLE,
	TL_THREAD_CREATE_SUSPENDED = 0,
#endif
#if TL_PLAT_PS3_SPU || TL_PLAT_REV
	TL_THREAD_JOINABLE         = 0,
	TL_THREAD_CREATE_SUSPENDED = 0,
#endif
	TL_THREAD_DEFAULT_FLAGS    = TL_THREAD_JOINABLE,  //!< Flags for normal threads.
};

/// Returns a handle to the currently active thread.
inline tlThreadHandle tlGetCurrentThread()
{
#if TL_PLAT_PC
	return (tlThreadHandle)GetCurrentThread();
#endif
#if TL_PLAT_PS3
	tlThreadHandle id;
	sys_ppu_thread_get_id(&id);
	return id;
#endif
#if TL_PLAT_PS3_SPU
	return (tlThreadHandle)cellSpursGetCurrentSpuId()|0x80000000;
#endif
#if TL_PLAT_REV
    return (tlThreadHandle)OSGetCurrentThread();
#endif
}

/// Returns the ID of the currently active thread.
inline tlThreadId tlGetCurrentThreadId()
{
#if TL_PLAT_PC
	return (tlThreadId)GetCurrentThreadId();
#endif
#if TL_PLAT_PS3
	tlThreadId id;
	sys_ppu_thread_get_id(&id);
	return id;
#endif
#if TL_PLAT_PS3_SPU
	return (tlThreadId)cellSpursGetCurrentSpuId()|0x80000000;
#endif
#if TL_PLAT_REV
    return (tlThreadId)(OSGetCurrentThread()-_rvl_thread_table);
#endif
}

/// Causes the current thread to sleep for the given number of milliseconds.
inline void tlSleep(U32 milliseconds)
{
#if TL_PLAT_PC
	Sleep(milliseconds);
#endif
#if TL_PLAT_PS3
	sys_timer_usleep(milliseconds*1000);
#endif
#if TL_PLAT_REV && (RVL_SDK > 1)
    OSSleepTicks(OSMillisecondsToTicks(milliseconds));
#endif
}

#if !TL_PLAT_PS3_SPU

// See SetThreadName in the Xenon SDK documentation.  Used by tlCreateThread.
#if TL_PLAT_PC
typedef struct tagTHREADNAME_INFO 
{
    DWORD dwType;     // Must be 0x1000
    LPCSTR szName;    // Pointer to name (in user address space)
    DWORD dwThreadID; // Thread ID (-1 for caller thread)
    DWORD dwFlags;    // Reserved for future use; must be zero
} THREADNAME_INFO;

inline void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName )
{
	// this is not supported on 64-bit windows
#if WORD_SIZE == 32
    THREADNAME_INFO info;
	
    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
	
    __try
    {
        RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD *)&info );
    }
    __except( EXCEPTION_CONTINUE_EXECUTION )
    {
    }
#endif
}
#endif

/** \typedef tlThreadFunction
	\brief Callback function declaration for thread entry functions.
*/
#if TL_PLAT_PC
typedef DWORD (__stdcall *tlThreadFunction)(LPVOID lpThreadParameter);
#endif
#if TL_PLAT_PS3
typedef void (*tlThreadFunction)(uint64_t);
#endif
#if TL_PLAT_REV
typedef U32 (*tlThreadFunction)(void*);
#endif

/** \def TL_THREAD_FUNCTION(name)
	\brief Macro to allow thread entry functions to be declared cross-platform.

	Since the thread entrty function declarations differ between platforms, this macro allows programmers
	to declare a single entry point.
	
	An example of TL_THREAD_FUNCTION and TL_THREAD_RETURN is provided below.

	\code
	TL_THREAD_FUNCTION(MyThreadEntry)
	{
		U32 data = (U32)ThreadParam;
		tlPrintf("Thread entry! data=%d\n", data);
		TL_THREAD_RETURN(0);
	}
	\endcode

	\warning When using ThreadParam, always cast to a U32 since the type is different between platforms. 
*/
#if TL_PLAT_PC
#define TL_THREAD_FUNCTION(name) DWORD __stdcall name(void* ThreadParam)
#endif
#if TL_PLAT_PS3
#define TL_THREAD_FUNCTION(name) void name(uint64_t ThreadParam)
#endif
#if TL_PLAT_REV
#define TL_THREAD_FUNCTION(name) U32 name(void* ThreadParam)
#endif

/** \def TL_THREAD_RETURN(exitcode)
	\brief Macro to return from thread entry functions.
	\param exitcode - Thread exit code.
*/
#if TL_PLAT_PC
#define TL_THREAD_RETURN(exitcode) return exitcode
#endif
#if TL_PLAT_PS3
#define TL_THREAD_RETURN(exitcode) do { tlExitThread(exitcode); return; } while (false)
#endif
#if TL_PLAT_REV
#define TL_THREAD_RETURN(exitcode) do { return exitcode; } while (false)
#endif

/** Creates a new application thread.
	\param entry - Function that will be run in the new thread context.
	\param param - Arbitrary data parameter passed to the entry function.
	\param name - Descriptive name for the thread - displayed in debugger and profilers.
	\param priority - Platform specific priority level.  See tlThreadPriority.
	\param stacksize - Size of the new thread's stack.  
	\param flags - Platform specific thread creation flags.  See tlThreadFlags.
*/
inline tlThreadHandle tlCreateThread(
	tlThreadFunction entry, INTPTR param, // [Blur2PC][x64] carries a pointer
	const char* name, 
	int priority = TL_THREAD_PRIORITY_NORMAL, 
	U32 stacksize = TL_THREAD_DEFAULT_STACK_SIZE, 
	U32 flags = TL_THREAD_DEFAULT_FLAGS)
{
#if TL_PLAT_PC
	name; priority;
	DWORD id;
	HANDLE thr = CreateThread(NULL, stacksize, entry, (LPVOID)param, flags, &id);
	assert(thr != 0); // tlCreateThread failed to create thread.
	return (tlThreadHandle)thr;
#endif
#if TL_PLAT_PS3
	tlThreadHandle thr;
	int ret = sys_ppu_thread_create(&thr, entry, param, priority, stacksize, flags, name);
	assert(ret == CELL_OK); // tlCreateThread failed to create thread.
	return thr;
#endif
#if TL_PLAT_REV
    assert(0) ; // threads are not supported yet due to bad include setups.
    return false;
#if 0
    U32 oldwalk = _rvl_thread_walker;
    bool ret;
    do
    {
        if (OSIsThreadTerminated(&_rvl_thread_table[_rvl_thread_walker]))
        {
            oldwalk = _rvl_thread_walker++;
            break;
        }
        else 
            _rvl_thread_walker++;
        if (_rvl_thread_walker >= RVL_MAX_THREADS)
            _rvl_thread_walker = 0;
    } while (oldwalk != _rvl_thread_walker);
    
    assert(oldwalk != _rvl_thread_walker); // tlCreateThread could not find a free thread.
    OSThread &t = _rvl_thread_table[oldwalk];

    if (t.stackEnd)
        BizAlignedFree(t.stackEnd);
    U8 *stackend = (U8*)BizAlignedMalloc(stacksize, 32);
    ret = OSCreateThread(&t, entry, param, stackend+stacksize, stacksize, priority, flags);
    assert(ret); // tlCreateThread could not create thread.
    return &t;
#endif
#endif
}

//! Wrapper for tlCreateThread to support old calling convention.
inline tlThreadHandle tlCreateThread(tlThreadFunction entry, INTPTR param, int priority, U32 stacksize, U32 flags, const char* name) // [Blur2PC][x64] INTPTR param
{
	return tlCreateThread(entry, param, name, priority, stacksize, flags);
}

/** Exits the current thread.
	\param exitcode Thread exit code. 
*/
inline void tlExitThread(U32 exitcode)
{
#if TL_PLAT_PC
	ExitThread(exitcode);
#endif
#if TL_PLAT_PS3
	sys_ppu_thread_exit(exitcode);
#endif
#if TL_PLAT_REV
    OSExitThread((void*)exitcode);
#endif
}

/** Waits for the given thread to exit, and closes the thread object.
	\param thr Thread handle. 
	\return Thread exit code. 
*/
inline U32 tlWaitForThread(tlThreadHandle thr)
{
#if TL_PLAT_PC
	WaitForSingleObject((HANDLE)thr, INFINITE);
	DWORD ret;
	GetExitCodeThread((HANDLE)thr, &ret);
	CloseHandle((HANDLE)thr);
	return ret;
#endif
#if TL_PLAT_PS3
	U64 ret;
	int err = sys_ppu_thread_join(thr, &ret);
	assert(err == CELL_OK); // tlWaitForThread failed to join thread.  Was the thread created with TL_THREAD_JOINABLE?
	return ret;
#endif
#if TL_PLAT_REV
    void* ret;
    bool err = OSJoinThread((OSThread*)thr, &ret);
    assert(err); // tlWaitForThread failed to join thread.
    return (U32)ret;
#endif
}

/** Sets the name of the current thread.
	\param name New name for the current thread.
*/
inline void tlSetCurrentThreadName(const char* name)
{
#if TL_PLAT_PC
	SetThreadName((DWORD)-1, name);
#endif
}

#endif //!TL_PLAT_PS3_SPU

/** \def tl_threadlocal
	\brief Storage class keyword to declare variables as thread local storage.
	
	Variables declared as tl_threadlocal are unique for each thread.  The thread will automatically receive the correct value,
	no additional code is needed.

	TLS variables must be global variables, static class members, or static function locals.  They can be initialized, but only
	to constant values.

	The tl_threadlocal storage class can be used with extern and static keywords only.
	\code
	extern tl_threadlocal int OneOfThesePerThread;
	tl_threadlocal int OneOfThesePerThread;
	\endcode
*/
#if TL_PLAT_PC
#define tl_threadlocal __declspec(thread)
#endif
#if TL_PLAT_PS3
#define tl_threadlocal __thread
#endif
#if TL_PLAT_PS3_SPU || defined(TL_PLAT_REV)
#define tl_threadlocal
#endif

//@}

#endif
