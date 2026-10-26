#include "CorePch.h"
#include <core/core.h>
#include <core/Maths.h>

#if defined( PLATFORM_PC )
#include <windows.h>
#elif defined( PLATFORM_PS3 )
#if CELL_SDK_VERSION >= 0x240001
#include <sys/ppu_thread.h>
#else
#include <ppu_thread.h>
#endif
#include <sys/timer.h>
#include <unistd.h>
#include <core\threads\ThreadLocalStorage.h>
#include <netex/net.h>
#else
#include <pthread.h>
#include <sys/types.h> 
#include <sys/unistd.h>
#endif

namespace MT
{

#define HAVE_PLATFORM_THREAD_HANDLE_TYPE
#if defined(COMPILER_VC)
typedef void		ThreadHandle;
#else
#ifndef PLATFORM_PS3
typedef pthread_t   ThreadHandle;
#else
typedef sys_ppu_thread_t   ThreadHandle;
#endif
#endif

}

#include <core/threads/ThreadLocalStorage.h>
#include <core/threads/Thread.h>

namespace MT
{

// JASOND: I've added this to give fast access to the thread's handle for situations where
// you don't have a pointer to the MT::Thread object for the currently running thread!
// The handle returned by GetCurrentThread() [pc/360] is a pseudo-handle
// and DuplicateHandle() needs to be called to convert it into a true handle
// that's useable by other threads to reference this thread. This TLS method
// for obtaining the handle is substantially more efficient.
class TLSWrapper : public ThreadLocalStorage
{
public:
	static TLSWrapper& Instance()
	{
		static TLSWrapper s_instance;

		return s_instance;
	}
};

#ifdef PLATFORM_PC

class ThreadIDTLS : public ThreadLocalStorage
{
public:
	static ThreadIDTLS& Instance()
	{
		static ThreadIDTLS s_instance;

		return s_instance;
	}
};

#endif

/// pthreads implementation



/// Get handle of current thread from TLS [for when you don't have a pointer to the MT::Thread object for the current thread!]
ThreadHandle* Thread::GetCurrentThreadHandle()
{
    return (ThreadHandle*)TLSWrapper::Instance().GetData();
}

/// Set handle of current thread in TLS
void Thread::SetCurrentThreadHandle( ThreadHandle* handle )
{
    TLSWrapper::Instance().SetData( (void*)handle );
}

#ifdef PLATFORM_PC
/// Get handle of current thread from TLS [for when you don't have a pointer to the MT::Thread object for the current thread!]
U32 Thread::GetCurrentThreadID()
{
	if (ThreadIDTLS::Instance().GetData())
		return *(U32*)ThreadIDTLS::Instance().GetData();
	else
		return 0; //Main thread
}

/// Set handle of current thread in TLS
void Thread::SetCurrentThreadID( U32 threadID )
{
    ThreadIDTLS::Instance().SetData( (void*)&threadID );
}
#endif



#if !defined( COMPILER_VC )

#ifndef PLATFORM_PS3
void *ThreadRunProc(void *arg)
{
    Thread* thread = (Thread*)arg;
    
//	LOGDEBUG(g_Log,  DBSTR "Id = %d\n", thread->Id() );
    
#ifdef PLATFORM_PS3
    return (void *)(U32)thread->_ThreadRunProc();
#else
    return (void *)(U64)thread->_ThreadRunProc();
#endif
}
#else
void ThreadRunProc(uint64_t arg)
{
    Thread* thread = (Thread*)arg;
    thread->_ThreadRunProc();
	sys_net_free_thread_context(0, SYS_NET_THREAD_SELF);
	sys_ppu_thread_exit (0);
}
#endif

// return PID
int GetProcessID()
{
#if defined( PLATFORM_PS3 )
	return 0;	// won't need process functionality on PS3, since there is only one process
#else
	return (int)getpid();
#endif
}


Thread::Thread() 
{
    m_Handle = NULL;
    m_Priority = 0;

}

Thread::~Thread()
{
    if ( m_Handle )
    {
//		LOGWARN(g_Log,  DBSTR "Thread not terminated! Id = %d\n", m_Id );
    }
}


U32 Thread::_ThreadRunProc()
{
    Maths::FP_Init();
    
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //Maths::FP_UnmaskExceptions();
    //#endif

#ifdef PLATFORM_PC
    // ensure this thread gets a message queue setup
    MSG msg;
    PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );
#endif

#ifdef PLATFORM_PS3 //TODO: [IW] do we need all these various conditional compilation
 	SetFlag(false);	//flags, can this not be simpler somehow?
#endif
  
    ThreadMain();

#ifdef PLATFORM_PS3
	Post();
#endif

	m_bIsRunning = false;

    return 0;
}

// called from outside the thread
void Thread::Start( const U32 stackSize, const S32 priority, char const * threadName )
{
    #if !defined( PLATFORM_PS3 )
	
	// Create a PS3 thread:
    void* pvParam = (void *)this;
 
	pthread_attr_t attr;
	size_t stacksize;
	pthread_attr_init( &attr );
	pthread_attr_getstacksize( &attr, &stacksize );
	if (!stackSize)
	{
		stacksize = 786432; 
	}
	else
	{
		stacksize = stackSize;
	}
	pthread_attr_setstacksize( &attr, stacksize );

    // Create Thread
	m_Handle = new ThreadHandle;
	
    int result = pthread_create( m_Handle, &attr, MT::ThreadRunProc, pvParam );

	m_bIsRunning = true;
	
    #else // #if !defined( PLATFORM_PS3 )
    
    uint64_t pvParam = (uint64_t)this;
	int stacksize = stackSize;
	if (!stackSize) stacksize = 0x20000;
	// NOTE :: when using flags = SYS_PPU_THREAD_CREATE_JOINABLE
	// if the thread is not exited by using join, there will be memory leaks (according to sony documentation)
	sys_ppu_thread_t handle;
	int ret = sys_ppu_thread_create( &handle, MT::ThreadRunProc, pvParam, 1001 - priority, stacksize, SYS_PPU_THREAD_CREATE_JOINABLE, threadName ? threadName : "MT::Thread" );
	m_Handle = (ThreadHandle*)handle;

#ifndef CFG_GOLD
	if (ret != CELL_OK)
	{
		DebugBreak();
	}
#endif
	m_bIsRunning = true;
    
    #endif // #else // #if !defined( PLATFORM_PS3 )
}

//// Put current thread to sleep for msec
void Thread::Sleep( const U32 msec )
{
#ifdef PLATFORM_PS3
	sys_timer_usleep( msec * 1000 );
#else
	usleep(msec * 1000);
#endif
}

void Thread::Close()
{
	m_Handle = NULL;
}

void Thread::Join()
{
#ifndef PLATFORM_PS3
	if (m_Handle)
		pthread_join(*m_Handle, NULL);
#else
	if (m_Handle)
	{
		uint64_t v;
		sys_ppu_thread_join((sys_ppu_thread_t)m_Handle, &v);
	}
#endif
}

#else // not defined( COMPILER_VC )
/// microsoft implementation



static DWORD WINAPI ThreadRunProc( void* info )
{
    Thread* thread = (Thread*)info;
    //LOGDEBUG(g_Log,  DBSTR "Id = %08X\n", thread->Id() );
    return thread->_ThreadRunProc();
}


Thread::Thread() 
{
    m_Handle = NULL;
    m_Priority = 0;
    
    //SynchroniseMemoryBetweenThreads();
}

Thread::~Thread()
{
    //SynchroniseMemoryBetweenThreads();

	Close();
}

void Thread::Close()
{
    if ( m_Handle )
    {
        CloseHandle( m_Handle );
        m_Handle = NULL;
    }
}

U32 Thread::_ThreadRunProc()
{
	SetCurrentThreadHandle( m_Handle );
#ifdef PLATFORM_PC
	SetCurrentThreadID( m_Id);
#endif
   
    Maths::FP_Init();
    
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //Maths::FP_UnmaskExceptions();
    //#endif

    ThreadMain();

	m_bIsRunning = false;

    return 0;
}

// See SetThreadName in the Xenon SDK documentation.  Used by tlCreateThread.

// called from outside the thread
void Thread::Start( const U32 stackSize, const S32 priority, char const * threadName )
{
	// Create a Win32 thread:
    const LPSECURITY_ATTRIBUTES lpsa = NULL;
    
    U32 dwStackSize = stackSize;
    
    
    const LPVOID  pvParam = (LPVOID)this;
    const DWORD   dwCreationFlags = CREATE_SUSPENDED;
    DWORD threadID = 0;
    
    // Create Thread
	assert(m_Handle == NULL); // Make sure this handle has been released properly
    
    m_Handle = ::CreateThread( lpsa, dwStackSize, MT::ThreadRunProc, pvParam, dwCreationFlags, &threadID );
    
    #if defined(PLATFORM_PC)
	m_Id = threadID;
    #endif


    if ( !m_Handle )
    {
//		LOGFATAL(g_Log,  DBSTR "Failed to create thread!\n" );
    }
	m_bIsRunning = true;

	m_Priority = priority;
    
    // Change the thread priority:
    SetThreadPriority( m_Handle, THREAD_PRIORITY_NORMAL + m_Priority );
    
    ResumeThread( m_Handle );
}

//// Put current thread to sleep for msec
void Thread::Sleep( const U32 msec )
{
    ::Sleep( msec );
}

void Thread::Join()
{
	if (m_Handle)
		WaitForSingleObject(m_Handle, INFINITE);
}

// return PID
int GetProcessID()
{
	return (int)GetCurrentProcessId();
}

#endif

const bool Thread::IsRunning()
{
	return m_bIsRunning;
}

}; // namespace MT


