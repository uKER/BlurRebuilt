#ifndef _WAIT_FOR_OBJECTS_H
#define _WAIT_FOR_OBJECTS_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined( PLATFORM_PS3 )
#include <pthread.h>
#include <semaphore.h>
#include <sys/event.h>

namespace MT
{

#define MAX_NR_EVENTS						127
#define STATUS_WAIT_0						((DWORD   )0x00000000L)    
//#define STATUS_ABANDONED_WAIT_0				((DWORD   )0x00000080L)    
#define WAIT_FAILED							((DWORD)0xFFFFFFFF)
#define WAIT_OBJECT_0						((STATUS_WAIT_0 ) + 0 )
#define WAIT_TIMEOUT						258L    // dderror
//#define WAIT_ABANDONED						((STATUS_ABANDONED_WAIT_0 ) + 0 )
//#define WAIT_ABANDONED_0					((STATUS_ABANDONED_WAIT_0 ) + 0 )

class C_WaitObject
{
    sys_event_queue_t		mEventQueue;
	sys_event_port_t		mEventPort;	
	sys_event_t				mSysEvent;	
	
	HANDLE*					mHandles;
	DWORD					mHandleCount;
	bool					mWaitForAll;
	DWORD					mMilliseconds;
public:
	C_WaitObject();
	~C_WaitObject();
	void Set( HANDLE *handles, DWORD Count, bool waitForAll, DWORD milliseconds );
	DWORD Wait();
	void Post( uint64_t id );
};


class C_EventBase
{
	static	uint64_t	mIdCounter;
	uint64_t			mId;
	C_WaitObject*		mWaitObject;
	bool				mFlag;
public:
	C_EventBase();
	~C_EventBase();
	void Post();
	void SetFlag(bool value);
	bool GetFlag() { return mFlag; }
	void SetWaitObject( C_WaitObject *waitObj );
	uint64_t GetId();
	HANDLE GetBaseHandle() { return (HANDLE)this; }
	HANDLE GetBaseHandleConst() const { return (HANDLE)this; }
};

}

#elif defined( PLATFORM_SPU)

class C_WaitObject
{
};

class C_EventBase
{
};

#endif

#endif // _WAIT_FOR_OBJECTS_H_
