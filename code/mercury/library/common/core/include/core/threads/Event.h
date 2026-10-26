#ifndef __MERCURY_SYSTEM_EVENT_H__
#define __MERCURY_SYSTEM_EVENT_H__

#ifdef _MSC_VER
#pragma once
#endif

#if defined( PLATFORM_PS3 ) || defined( PLATFORM_SPU )
#include "core\threads\WaitForObjects.h"
#endif

// Times in milliseconds
#define WAIT_TIME_INFINITE		(0xffffffff)

namespace MT
{

// types which represent platform specific synchronisation objects
#ifndef HAVE_PLATFORM_EVENT_TYPES
class PlatformEvent;
#endif

class Event
#ifdef PLATFORM_PS3
	: public C_EventBase
#endif
{
public:
					/// constructor
					Event(const char *pszName = NULL);
					/// destructor
					~Event();
	
					/// signal the event
    void            Signal();
					/// wait for the event to be signaled
    bool            Wait( const U32 timeMs = WAIT_TIME_INFINITE );

					/// retrieve platform-dependent handle
	PlatformEvent*  GetHandle() const __restrict { return m_Handle; }
    
private:
	PlatformEvent*	m_Handle;
	
	#ifdef PLATFORM_PS3
	sys_lwmutex_t	m_Mutex;
	bool			m_bSignalled;
	#elif !defined( COMPILER_VC )
	Mutex			*m_pMutex;
	#endif
};

}; // namespace MT

#endif
