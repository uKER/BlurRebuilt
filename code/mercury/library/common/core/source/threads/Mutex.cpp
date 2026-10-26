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

#define HAVE_PLATFORM_MUTEX_TYPES

#if defined ( COMPILER_VC )
typedef CRITICAL_SECTION PlatformLock;
typedef void PlatformMutex;
#elif defined ( PLATFORM_PS3 )
typedef sys_lwmutex_t PlatformMutex;
typedef sys_lwmutex_t PlatformLock;
#else
typedef pthread_mutex_t PlatformMutex;
typedef pthread_mutex_t PlatformLock;
#endif

#include <core/Threads/Mutex.h>

namespace MT
{
//
// Lock Class Functions
//
#if !defined( COMPILER_VC )

Lock::Lock()
{
	m_lock = (PlatformLock*)malloc(sizeof(PlatformLock));

	sys_lwmutex_attribute_t lwAttrib;
	sys_lwmutex_attribute_initialize( lwAttrib );
	lwAttrib.attr_recursive = SYS_SYNC_RECURSIVE;
	int result = sys_lwmutex_create( m_lock, &lwAttrib );

	BIZ_ASSERT(result == 0);
}

Lock::Lock( const Lock& copy )
{
//	LOGFATAL(g_Log,  DBSTR "Cannot copy a critical section!\n" );
}

Lock::~Lock()
{
	sys_lwmutex_destroy(m_lock);
	free(m_lock);
}

void Lock::operator = ( const Lock& copy )
{
//	LOGFATAL(g_Log,  DBSTR "Cannot copy a critical section!\n" );
}

void Lock::Enter()
{
	int result = sys_lwmutex_lock(m_lock, 0);
	BIZ_ASSERT(result == CELL_OK);
}

bool Lock::TryEnter()
{
	int result = sys_lwmutex_trylock(m_lock);
	BIZ_ASSERT(result == CELL_OK || result == EBUSY);

	return (result == CELL_OK);
}

void Lock::Exit()
{
	int result = sys_lwmutex_unlock(m_lock);
	BIZ_ASSERT(result == CELL_OK);
}

#else

Lock::Lock()
{
	m_lock = new PlatformLock;
    InitializeCriticalSection( m_lock );
}

Lock::Lock( const Lock& copy )
{
//	LOGFATAL(g_Log,  DBSTR "Cannot copy a critical section!\n" );
}

Lock::~Lock()
{
    DeleteCriticalSection( m_lock );
	delete m_lock;
}

void Lock::operator = ( const Lock& copy )
{
	assert(0);
//	LOGFATAL(g_Log,  DBSTR "Cannot copy a critical section!\n" );
}

void Lock::Enter()
{
    EnterCriticalSection( m_lock );
}

bool Lock::TryEnter()
{
    return TryEnterCriticalSection( m_lock ) != 0;
}

void Lock::Exit()
{
    LeaveCriticalSection( m_lock );
}
#endif


//
// Mutex Class Functions
//

#if defined( PLATFORM_PS3 )

Mutex::Mutex(const char *pszName)
{
	m_Handle = (PlatformMutex*)malloc(sizeof(PlatformMutex));

	sys_lwmutex_attribute_t lwAttrib;
	sys_lwmutex_attribute_initialize( lwAttrib );
	lwAttrib.attr_recursive = SYS_SYNC_RECURSIVE;
#ifndef CFG_GOLD
	if (pszName)
	{
		strncpy(lwAttrib.name, pszName, SYS_SYNC_NAME_SIZE);
		lwAttrib.name[7] = 0;
	}
#endif
	int result = sys_lwmutex_create( m_Handle, &lwAttrib );
    if ( result )
    {
		CreatedOK = false;
#ifndef CFG_GOLD
		DebugBreak();
#endif
    }
    else
    {
		CreatedOK = true;
    }
}

Mutex::~Mutex()
{
	sys_lwmutex_destroy( m_Handle );
	free(m_Handle);
}

bool Mutex::Wait( const U32 timeMs )
{
    if (!CreatedOK)
		return true;

#if defined ( PLATFORM_PS3 )
	SetFlag( false );
#endif
	int result = sys_lwmutex_lock( m_Handle, timeMs * 1000 );
	if (result == 0) 
		return true;
	else
	{
		return false; 
	}
}

bool Mutex::Release()
{
	int result = sys_lwmutex_unlock( m_Handle );
    if (( result != 0 ) && (result != EPERM))
    {
#ifndef CFG_GOLD
		DebugBreak();
#endif
		return false;
    }
#if defined ( PLATFORM_PS3 )
    Post();
#endif
	return true;
}


#elif !defined( COMPILER_VC )

Mutex::Mutex(const char *pszName)
{
	m_Handle = (PlatformMutex*)malloc(sizeof(PlatformMutex));

	int result = pthread_mutex_init(m_Handle,NULL) ;
    if ( result )
    {
		CreatedOK = false;
		DebugBreak();
    }
    else
    {
		CreatedOK = true;
    }
}

Mutex::~Mutex()
{
	int result = pthread_mutex_destroy(m_Handle);
    if ( result )
    {
		DebugBreak();
//		LOGWARN(g_Log,  DBSTR "Mutex not created!\n" );
    }
	free(m_Handle);
}

bool Mutex::Wait( const U32 timeMs )
{
	// Doesn't have a timeout yet
	// TODO: Add Timeout using 'sys_cond_wait'?

    if (!CreatedOK)
		return true;

#if defined ( PLATFORM_PS3 )
	SetFlag( false );
#endif
	int result = pthread_mutex_lock( m_Handle );
	if (result == 0) 
		return true;
	else
	{
		return false; 
	}
}


bool Mutex::Release()
{
	int result = pthread_mutex_unlock( m_Handle );
    if (( result != 0 ) && (result != EPERM))
    {
		DebugBreak();
		return false;
//		LOGFATAL(g_Log,  DBSTR "Mutex is not owned by the current thread!\n" );
    }
#if defined ( PLATFORM_PS3 )
    Post();
#endif
	return true;
}

#else
typedef void PlatformMutex;

Mutex::Mutex(const char *pszName)
{
    const LPSECURITY_ATTRIBUTES securityAttributes = NULL;
    m_Handle = CreateMutex( securityAttributes, false, pszName );
    if ( !m_Handle )
    {
		CreatedOK = false;
    }
	else
	{
		CreatedOK = true;
	}
	//m_bAlreadyLocked = false;
}

Mutex::~Mutex()
{
    if ( m_Handle )
    {
        CloseHandle( m_Handle );
    }
    else
    {
//		LOGWARN(g_Log,  DBSTR "Mutex not created!\n" );
    }
}

bool Mutex::Wait( const U32 timeMs )
{
	if (CreatedOK)
	{
		U32 result = WaitForSingleObject( m_Handle, timeMs );
		if ( ( result == WAIT_OBJECT_0 ) || ( result == WAIT_ABANDONED ))
		{
			// a mutex can't be locked twice by the same thread
//			if (m_bAlreadyLocked)
//			{
////				assert(timeMs != WAIT_TIME_INFINITE); // on pthreads this would cause a deadlock
//				ReleaseMutex(m_Handle);
//				return false;
//			}
//			m_bAlreadyLocked = true;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool Mutex::Release()
{
	if (CreatedOK)
	{
		//assert(m_bAlreadyLocked); // on the Xenon this causes an exception
		//m_bAlreadyLocked = false;
		if ( !ReleaseMutex( m_Handle ) )
		{
			return false;
	//		LOGFATAL(g_Log,  DBSTR "Mutex is not owned by the current thread!\n" );
		}
	}
	return true;
}
#endif

}; // namespace MT
