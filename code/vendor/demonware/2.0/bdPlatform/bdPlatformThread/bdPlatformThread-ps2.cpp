// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>
#include <eekernel.h>
#include <assert.h>

bdBool BD_CALL bdPlatformThread::createThread(bdThreadProc threadProc, 
											  void *args,
											  bdThreadHandle &handle,
											  bdInt priority ,
											  bdUInt32 stackSize,
											  void *stackPointer)
{
	ThreadParam param;
	param.gpReg = &_gp;

	param.stack = stackPointer;
	param.stackSize = stackSize;
	param.initPriority = priority;
	param.entry = (bdThreadProc)threadProc;

	handle = CreateThread(&param);
	return handle > 0;
}

bdBool BD_CALL bdPlatformThread::startThread(bdThreadHandle &handle, 
											 void *args, 
											 bdUInt argSize )
{
	const bdInt32 res = StartThread(handle, args);
	return res != -1;
}

void BD_CALL bdPlatformThread::joinThread(bdThreadHandle& )
{
}

void BD_CALL bdPlatformThread::deleteThread(bdThreadHandle &handle)
{
	DeleteThread(handle);
}

void BD_CALL bdPlatformThread::setPriority(bdThreadHandle &handle, 
										   bdInt32 priority)
{
	ChangeThreadPriority(handle, priority);
}

bdInt32 BD_CALL bdPlatformThread::getPriority(bdThreadHandle &handle)
{
	ThreadParam param;
	ReferThreadStatus(handle, &param);
	return param.currentPriority;
}

bdInt32 BD_CALL bdPlatformThread::getPriority()
{
	bdThreadHandle handle = GetThreadId();
	ThreadParam param;
	ReferThreadStatus(handle, &param);
	return param.currentPriority;
}

bdThreadHandle BD_CALL bdPlatformThread::getThreadHandle()
{
	return GetThreadId();
}
