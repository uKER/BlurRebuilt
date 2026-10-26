#ifndef __MERCURY_SYSTEM_MUTEX_H__
#define __MERCURY_SYSTEM_MUTEX_H__

#ifdef _MSC_VER
#pragma once
#endif

#if defined( PLATFORM_PS3 ) || defined( PLATFORM_SPU )
#include "core\threads\WaitForObjects.h"
#if defined( PLATFORM_PS3 ) 
#include <sys/ppu_thread.h>
#endif
#endif


namespace MT
{

// types which represent platform specific synchronisation objects
#ifndef HAVE_PLATFORM_MUTEX_TYPES
class PlatformMutex;
#if defined( PLATFORM_PS3 ) || defined( PLATFORM_SPU )
class SPUPlatformMutex;
#endif
class PlatformEvent;
class PlatformLock;
#endif


// Times in milliseconds
#define WAIT_TIME_INFINITE		(0xffffffff)

/// Global system mutex ("critical section")
class Lock
{
public:
						/// constructor
						Lock();
						/// Copy constructor
						Lock( const Lock& copy );
						/// destructor
						~Lock();
					    
						///
    void                operator = ( const Lock& copy );
    
						/// Enter critical section
    void                Enter();
						/// Same as above, return false if the critical section was already locked
    bool                TryEnter();
						/// Exit critical section
    void                Exit();
    
private:
	PlatformLock		*m_lock;
};

/// Auto release class for the global system mutex
class AutoLock
{
private:
	Lock& m_lock;

public:
	AutoLock(Lock& lock) : m_lock(lock)
	{
		m_lock.Enter();
	}
	
	~AutoLock()
	{
		m_lock.Exit();
	}
};

#define CRITICAL_SECTION_SPIN_COUNT_DEFAULT 4000

#ifdef PLATFORM_PS3
class Mutex : public C_EventBase
#else
/// Platform independent Mutex Lock object
class Mutex 
#endif
{
public:
						/// constructor
						 Mutex(const char *pszName = NULL);
						/// destructor
						 ~Mutex();
    
						/// Lock the mutex, optionally wait a specified time before giving up
    bool                Wait( U32 timeMs = WAIT_TIME_INFINITE );
						/// Release the lock
    bool                Release();
						/// Return the platform-specific mutex object
	PlatformMutex		*Handle() { return m_Handle; }
    
private:
    bool				CreatedOK;
	//bool				m_bAlreadyLocked;
	PlatformMutex		*m_Handle;
};

class C_MutexLock
{
	Mutex* m_pMutex;
public:
	C_MutexLock(Mutex* mutex) : m_pMutex(mutex)
	{
		m_pMutex->Wait();
	}

	~C_MutexLock()
	{
		m_pMutex->Release();
	}
};


}; // namespace MT



#endif
