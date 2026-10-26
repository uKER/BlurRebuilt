// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>

bdBool BD_CALL bdPlatformThread::createThread(bdThreadProc threadProc, 
								   void *args,
								   bdThreadHandle &handle,
								   bdInt priority,
								   bdUInt stackSize,
								   void *stackPointer)
{
	if (pthread_create(&handle,0,(bdThreadProc)threadProc, args)==0)
	{
		return true;
	}
	return false;
}

bdBool BD_CALL bdPlatformThread::startThread(bdThreadHandle& handle, 
											 void *args, 
											 bdUInt argSize )
{
	return true;
}

void BD_CALL bdPlatformThread::joinThread(bdThreadHandle& handle)
{
	pthread_join(handle, 0);
}

void BD_CALL bdPlatformThread::deleteThread(bdThreadHandle& handle)
{
}

void BD_CALL bdPlatformThread::setPriority(bdThreadHandle& handle, 
										   bdInt32 priority)
{
	// There is no easy way under Linux to change a thread's priority.
	// This functionality is only ever needed for the PS2. 
}

bdInt32 BD_CALL bdPlatformThread::getPriority(bdThreadHandle& handle)
{
	// There is no easy way under Linux to change a thread's priority.
	// This functionality is only ever needed for the PS2. 
	return 0;
}
