#ifndef __MERCURY_SYSTEM_SEMAPHORE_H__
#define __MERCURY_SYSTEM_SEMAPHORE_H__

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PS3
#include "WaitForObjects.h"
#endif

// Times in milliseconds
#define WAIT_TIME_INFINITE		(0xffffffff)

namespace MT
{

// types which represent platform specific synchronisation objects
#ifndef HAVE_PLATFORM_SEMAPHORE_TYPES
class PlatformSemaphore;
#endif

/// Platform independant Semaphore with spin-count
class Semaphore
#ifdef PLATFORM_PS3
	: public C_EventBase
#endif
{
public:
						        /// Constructor
						        Semaphore(const char *pszName = NULL, const long initialCount=0, const long maximumCount=0x7fffffff);
						        /// Destructor
						        ~Semaphore();
    
						        /// Wait for count to be >0 and decrease
    bool						Wait( const U32 timeMs = WAIT_TIME_INFINITE );
						        /// Increase count
    void						Release( const long releaseCount = 1 );
    
						        /// Return the platform-dependant semaphore object
	NOALIAS PlatformSemaphore*  GetHandle() const __restrict { return m_Handle; }

private:
    
	PlatformSemaphore*          m_Handle;
};

}; // namespace MT

#endif
