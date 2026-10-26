// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#define BD_LOG_LEVEL "bdPlatformThread"

bdBool BD_CALL bdPlatformThread::createThread(bdThreadProc threadProc,
											  void *args,
											  bdThreadHandle &handle,
											  bdInt priority,
											  bdUInt stackSize,
											  void * stackPointer)
{
	BD_ASSERT(priority >= 0 && priority <= 32, "Priority outside valid range.");

	bdUByte8 *stack = reinterpret_cast<bdUByte8*>(stackPointer) + stackSize;
	bdBool ok = OSCreateThread(&handle,
							   threadProc,
							   args,
							   stack,
							   stackSize,
							   priority,
							   0); //OS_THREAD_ATTR_DETACH);
							   
	return ok;
}

bdBool BD_CALL bdPlatformThread::startThread(bdThreadHandle &handle,
											 void *, bdUInt)
{

	const bdInt32 previousSuspendCount = OSResumeThread(&handle);

	// thread is suspended if the previousSuspendCount is > 0
	const bdBool ok = ( previousSuspendCount > 0 );

	return ok;
}

void BD_CALL bdPlatformThread::joinThread(bdThreadHandle &handle)
{
	const bdBool ok = OSJoinThread(&handle, BD_NULL);
	if(!ok)
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to join thread, thread may have terminated abnormally.");
	}
}

void BD_CALL bdPlatformThread::deleteThread(bdThreadHandle &handle)
{
	if(!OSIsThreadTerminated(&handle))
	{
		OSCancelThread(&handle);
	}
}

void BD_CALL bdPlatformThread::setPriority(bdThreadHandle &handle,
										   const bdInt priority)
{
	BD_ASSERT(priority >= 0 && priority <= 32, "Priority outside valid range.");
	OSSetThreadPriority(&handle, priority);
}

bdInt BD_CALL bdPlatformThread::getPriority(bdThreadHandle &handle)
{
	const OSPriority priority = OSGetThreadPriority(&handle);
	return priority;
}

bdInt BD_CALL bdPlatformThread::getPriority()
{
	const OSPriority priority = OSGetThreadPriority(OSGetCurrentThread());
	return priority;
}

