#ifdef PLATFORM_PS3
#include "stdio.h"
#include <core/core.h>
#include "core\threads\WaitForObjects.h"
#include "sys/timer.h"

namespace MT
{


void C_WaitObject::Set( HANDLE *handles, DWORD Count, bool waitForAll, DWORD milliseconds )
{
	mHandles = new HANDLE[ Count ];

	mWaitForAll = waitForAll;

	if (milliseconds == INFINITE) milliseconds = 0;
	mMilliseconds = milliseconds;
	mHandleCount = Count;

	for (DWORD i = 0 ; i < Count ; i++)
	{
		mHandles[i] = handles[i];
		C_EventBase* obj = (C_EventBase*)handles[i];
		obj->SetWaitObject( this );
	}
}


DWORD C_WaitObject::Wait()
{
	while(1)
	{
		DWORD retValue = 0;
		DWORD matchCount = 0;
		// Before we stop the thread on an event queue, 
		// Check to see if the objects are not already set
		for (DWORD i = 0 ; i < mHandleCount ; i++)
		{
			C_EventBase* obj = (C_EventBase*)mHandles[i];
			if (obj)
			{
				if (obj->GetFlag())
				{
					if (obj->GetId() == mSysEvent.data1)
					{
						retValue = i;
						matchCount++;
					}
				}
			}
		}	
		if ((mWaitForAll) && (matchCount == mHandleCount)) return retValue;
		if ((!mWaitForAll) && (matchCount > 0)) return retValue;

		// Now Wait on Que
#if	1
		int ret = sys_event_queue_receive(mEventQueue, &mSysEvent, mMilliseconds * 1000 );
		if (ret != 0)
		{
#ifndef CFG_GOLD
			printf("sys_event_queue_receive failed %8x\n",ret);
			DebugBreak();
#endif
		}
#else
		int ret = 0;
		L_10:
		ret = sys_event_queue_receive(mEventQueue, &mSysEvent, 5000 * 1000 );
		if (ret == ETIMEDOUT)
		{
			printf("time out waiting for object(s)\n");
			goto L_10;
		}
#endif
		if (ret != CELL_OK) 
		{
			if (ret == ETIMEDOUT)
			{
				return WAIT_TIMEOUT;
			}
			printf("Event Receive Failed (%#x)\n", ret);
			return WAIT_FAILED;
		}
		else
		{
			// See what we have off the queue
			for (DWORD i = 0 ; i < mHandleCount ; i++)
			{
				C_EventBase* obj = (C_EventBase*)mHandles[i];
				if (obj)
				{
					if ((obj->GetId() == mSysEvent.data1) || (obj->GetFlag()))
					{
						retValue = i;
						matchCount++;
					}
				}
			}
			if ((mWaitForAll) && (matchCount == mHandleCount)) return retValue;
			if ((!mWaitForAll) && (matchCount > 0)) return retValue;
			printf("ERR: Shouldn't get here\n");
		}
	}
}

void C_WaitObject::Post( uint64_t id )
{
	// Post the event
	sys_event_port_send( mEventPort, (std::uint64_t)id, 0, 0 );
}


C_WaitObject::C_WaitObject()
{
	int ret;
	int queueSize = MAX_NR_EVENTS;
	// Initialise The Queue Attributes
	sys_event_queue_attribute_t queueAttr;
	sys_event_queue_attribute_initialize(queueAttr);
	// Create the Queue
	ret = sys_event_queue_create(&mEventQueue, &queueAttr, SYS_EVENT_QUEUE_LOCAL, queueSize);
	if (ret != CELL_OK) 
	{
		printf("Event queue create failure (%#x)\n", ret);
		mEventQueue = 0;
		assert(0);
	}
	// Create the port
	ret = sys_event_port_create(&mEventPort, SYS_EVENT_PORT_LOCAL, SYS_EVENT_PORT_NO_NAME);
	if (ret != CELL_OK) 
	{
		printf("Event port create failure (%#x)\n", ret);
		mEventPort = 0;
		assert(0);
	}
	// Connect Port to Queue
	ret = sys_event_port_connect_local(mEventPort, mEventQueue);
	if (ret != CELL_OK) 
	{
		printf("Event port connect failure (%#x)\n", ret);
		printf("sender(%#x): Destroy the event port: ", mEventPort);
		ret = sys_event_port_destroy(mEventPort);
		if (ret != CELL_OK) 
		{
			printf("Failed to connect port to queue\n");
		}
		assert(0);
	}
}

C_WaitObject::~C_WaitObject()
{
	if (mEventPort)
	{
		int ret = sys_event_port_disconnect(mEventPort);
		if (ret != CELL_OK) 
		{
			printf("Failed to destroy Event Port\n");
		}

		ret = sys_event_port_destroy(mEventPort);
		if (ret != CELL_OK) 
		{
			printf("Failed to destroy Event Port\n");
		}
	}

	if ( mEventQueue )
	{
		int ret = sys_event_queue_destroy( mEventQueue, SYS_EVENT_QUEUE_DESTROY_FORCE );
		if (ret != CELL_OK) 
		{
			printf("Failed to destroy Event Queue\n");
		}
	}
	delete [] mHandles;
}

uint64_t	C_EventBase::mIdCounter = 0;


C_EventBase::C_EventBase()
{
	mWaitObject = 0;
	mId = mIdCounter++;
	mFlag = false;
}

C_EventBase::~C_EventBase()
{
	// Object is destroyed
}


void C_EventBase::Post()
{
	if (mWaitObject)
	{
//		printf("Post To Queue\n");
		mWaitObject->Post( mId );
	}
}

void C_EventBase::SetFlag(bool value)
{
	if (this) mFlag = value;
}


void C_EventBase::SetWaitObject( C_WaitObject *waitObj )
{
#ifndef CFG_GOLD
	if (mWaitObject) 
	{ 
		printf("Trying to assign WaitObject More than once\n"); 
		DebugBreak(); 
	}
#endif
	mWaitObject = waitObj;
}

uint64_t C_EventBase::GetId()
{
	return mId;
}



}
#endif
