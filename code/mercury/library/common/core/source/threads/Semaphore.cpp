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

#define HAVE_PLATFORM_SEMAPHORE_TYPES

#if defined ( COMPILER_VC )
typedef void PlatformSemaphore;
#elif defined ( PLATFORM_PS3 )
typedef sys_semaphore_t PlatformSemaphore;
#else
typedef sem_t PlatformSemaphore;
#endif

#include <core/Threads/Semaphore.h>

namespace MT
{

//
// Semaphore Class Functions
//

#if defined(PLATFORM_PS3)

Semaphore::Semaphore(const char *pszName, const long initialCount, const long maximumCount)
{
	m_Handle = new PlatformSemaphore;
	int rtn;
	sys_semaphore_attribute_t semaattr;
	sys_semaphore_attribute_initialize(semaattr);
#ifndef CFG_GOLD
	if (pszName)
	{
		strncpy(semaattr.name, pszName, SYS_SYNC_NAME_SIZE);
		semaattr.name[7] = 0;
	}
#endif
	int result = sys_semaphore_create( m_Handle, &semaattr, initialCount, maximumCount );
}

Semaphore::~Semaphore()
{
	int result = sys_semaphore_destroy( *m_Handle );
	delete m_Handle;
}

bool Semaphore::Wait( const U32 timeMs )
{
	int result;
	if (timeMs == INFINITE)
	{
		result = sys_semaphore_wait( *m_Handle, 0 );
	}
	else
	{
		result = sys_semaphore_wait( *m_Handle, timeMs*1000 );
	}
	
    if ( result )
    {
        return false;
    }
	SetFlag( false );
	return true;    
}

void Semaphore::Release( const long releaseCount )
{
	int result = sys_semaphore_post( *m_Handle, releaseCount );
	if ( result )
	{
		return;
	}
	Post();
}

#elif !defined( COMPILER_VC )

Semaphore::Semaphore(const char *pszName, const long initialCount, const long maximumCount)
{
	m_Handle = new PlatformSemaphore;
	int result = sem_init( m_Handle, 0, initialCount );
}

Semaphore::~Semaphore()
{
	int result = sem_destroy( m_Handle );
	delete m_Handle;
}

bool Semaphore::Wait( const U32 timeMs )
{
	int result;
	if (timeMs == INFINITE)
	{
		result = sem_wait( m_Handle );
	}
	else
	{
		const struct timespec time = { (timeMs / 1000), (timeMs % 1000) * 1000000 };
		result = sem_timedwait( m_Handle, &time);
	}
	
    if ( result )
    {
        return false;
    }
	return true;    
}

void Semaphore::Release( const long releaseCount )
{
	for (int i=0; i<releaseCount; i++)
	{
		int result = sem_post( m_Handle );
		if ( result )
		{
			return;
		}
	}
}

#else
typedef void PlatformSemaphore;

Semaphore::Semaphore(const char *pszName, const long initialCount, const long maximumCount)
{
    const LPSECURITY_ATTRIBUTES securityAttributes = NULL;
    const LPCTSTR name = NULL;
    m_Handle = CreateSemaphore( securityAttributes, initialCount, maximumCount, name );
}

Semaphore::~Semaphore()
{
    if ( m_Handle )
    {
        CloseHandle( m_Handle );
    }
}

bool Semaphore::Wait( const U32 timeMs )
{
    if ( !m_Handle )
    {
        return false;
    }
    
    U32 result = WaitForSingleObject( m_Handle, timeMs );
	return ( result == WAIT_OBJECT_0 );
}

void Semaphore::Release( const long releaseCount )
{
    if ( !m_Handle )
    {
        return;
    }
    
    long previousCount = 0;
    ReleaseSemaphore( m_Handle, releaseCount, &previousCount );
}
#endif

}; // namespace MT
