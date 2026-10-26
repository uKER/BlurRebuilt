#include "CorePch.h"
#include <core/core.h>

#if defined( PLATFORM_PC )
#include <windows.h>
#else
#if defined ( PLATFORM_PS3 )
#include <cell/atomic.h>
#include <sys/timer.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/sys_time.h>
#endif
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#if !defined( PLATFORM_PS3 )
#include <sys/unistd.h>
#include <errno.h>
#include <sys\time.h>
#include <time.h>
#endif
#endif

#if defined( PLATFORM_PS3 )
#include <cell/sync/mutex.h>
#endif

#define HAVE_PLATFORM_EVENT_TYPES

#if defined ( COMPILER_VC )
typedef CRITICAL_SECTION PlatformCriticalSection;
typedef void PlatformEvent;
#elif defined ( PLATFORM_PS3 )
typedef sys_lwcond_t PlatformEvent;
#else
typedef pthread_cond_t PlatformEvent;
#endif

#include <core/Threads/Event.h>
//#include <System/DebugLog.h>

namespace MT
{


//
// Event Class Functions
//
#if defined( PLATFORM_PS3 )

Event::Event(const char *pszName)
{
	m_Handle = new PlatformEvent;
	int rtn;
	sys_lwmutex_attribute_t mutexattr;
	sys_lwmutex_attribute_initialize(mutexattr);
	mutexattr.attr_recursive = SYS_SYNC_NOT_RECURSIVE;
#ifndef CFG_GOLD
	if (pszName)
	{
		strncpy(mutexattr.name, pszName, SYS_SYNC_NAME_SIZE);
		mutexattr.name[7] = 0;
	}
#endif
	rtn = sys_lwmutex_create(&m_Mutex, &mutexattr);
	assert(rtn == 0);
	sys_lwcond_attribute_t condattr;
	sys_lwcond_attribute_initialize(condattr);
#ifndef CFG_GOLD
	if (pszName)
	{
		strncpy(condattr.name, pszName, SYS_SYNC_NAME_SIZE);
		condattr.name[7] = 0;
	}
#endif
	rtn = sys_lwcond_create(m_Handle, &m_Mutex, &condattr);
	assert(rtn == 0);
	m_bSignalled = false;
}

Event::~Event()
{
	sys_lwcond_destroy(m_Handle);
	sys_lwmutex_destroy(&m_Mutex);
	delete m_Handle;
}

void Event::Signal()  
{ 
	int rtn = sys_lwmutex_lock(&m_Mutex, 0);
	if (!m_bSignalled)
	{
		m_bSignalled = true;
		sys_lwcond_signal(m_Handle);
	}
	sys_lwmutex_unlock(&m_Mutex);
	Post();
}

bool Event::Wait( const U32 timeMs ) 
{ 
	int rtn = sys_lwmutex_lock(&m_Mutex, 0);
	assert(rtn == 0);
	bool bRtn = true;

	// NOTE:
	// The caller PPU thread temporarily releases the lock of the lightweight mutex while it is
	// waiting on the lightweight condition variable, and then reacquires it when it is woken up.
	if (!m_bSignalled)
	{
		U32 rv = 0;

		if ( timeMs == INFINITE )
		{
			rv = sys_lwcond_wait(m_Handle, 0);
			assert(rv == 0);
		}
		else if (timeMs > 0) // This case seems to be broken (18/9/08)
		{
			// Add 1 to avoid blocking on zero timeout, matching behaviour of Win32 WaitForSingleObject.
			rv = sys_lwcond_wait(m_Handle, 1 + (timeMs * 1000));
			bRtn = (ETIMEDOUT == (int)rv)?false:true;
		}
		else
		{
			bRtn = false;
		}
	}
	if (bRtn)
	{
		m_bSignalled = false;
	}

	sys_lwmutex_unlock(&m_Mutex);

    return bRtn;
}

#elif !defined( COMPILER_VC )

Event::Event( const char *pszName )
{
	m_Handle = new PlatformEvent;
	pthread_cond_init(m_Handle, NULL);
	m_pMutex = new Mutex();
}

Event::~Event()
{
	pthread_cond_destroy(m_Handle);
	delete m_pMutex;
	delete m_Handle;
}

void Event::Signal()  
{ 
	pthread_cond_signal(m_Handle);
}

// copy of the function from system::time, which we can't rely on because this is core
U64 _GetCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

bool Event::Wait( const U32 timeMs ) 
{ 
	m_pMutex->Wait();

	U32 rv = 0;
	if ( timeMs == INFINITE )
	{
		struct timespec timeout;
		U64 t = _GetCurrentTime();
		t = t + 1 * 1000;
		timeout.tv_sec = t/1000000;
		timeout.tv_nsec = (t-(timeout.tv_sec*1000000))*1000;
		do
		{
			U32 rv = pthread_cond_timedwait( (pthread_cond_t*)m_Handle, (pthread_mutex_t*)m_pMutex->Handle(), (const std::timespec*)&timeout );
		} while(ETIMEDOUT == (int)rv);
	}
	else
	{
		struct timespec timeout;

		// calculate now-time + ms offset
		U64 t = _GetCurrentTime();
		t = t + timeMs * 1000;

		// convert to timespec
		timeout.tv_sec = t/1000000;
		timeout.tv_nsec = (t-(timeout.tv_sec*1000000))*1000;

		U32 rv = pthread_cond_timedwait( (pthread_cond_t*)m_Handle, (pthread_mutex_t*)m_pMutex->Handle(), &timeout );
	}
	bool bRtn = (ETIMEDOUT == (int)rv)?false:true;

	m_pMutex->Release();

    return bRtn;
}

#else

Event::Event(const char *pszName)
{
    const LPSECURITY_ATTRIBUTES securityAttributes = NULL;
    m_Handle = CreateEvent( securityAttributes, FALSE, FALSE, pszName );
}

Event::~Event()
{
    if ( m_Handle )
    {
        CloseHandle( m_Handle );
    }
}


void Event::Signal() 
{ 
    SetEvent( m_Handle );
}

bool Event::Wait( const U32 timeMs )
{ 
	return WaitForSingleObject( m_Handle, timeMs ) == WAIT_OBJECT_0;
}

#endif

}; // namespace MT
