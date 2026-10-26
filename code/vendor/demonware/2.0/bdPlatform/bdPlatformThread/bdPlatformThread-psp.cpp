// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#include <kernel.h>
#include <assert.h>

bdBool BD_CALL bdPlatformThread::createThread(bdThreadProc threadProc, 
											  void *args,
											  bdThreadHandle &handle,
											  bdInt priority,
											  bdUInt stackSize,
											  void *stackPointer)
{

	static bdNChar8 threadName[10];
	static bdUInt  threadCnt = 0;
	bdSnprintf(threadName, 10, "Thread%u", threadCnt++);
	handle = sceKernelCreateThread(threadName, 
								   threadProc, 
								   priority,
								   stackSize, 
								   0, 
								   NULL);
	if (handle > 0 )
	{	
		return true;
	}
	return false;
}

bdBool BD_CALL bdPlatformThread::startThread(bdThreadHandle &handle, 
											 void *args, 
											 bdUInt argSize)
{
	bdInt32 res = sceKernelStartThread(handle, argSize, args);
	if (res == -1)
	{
		return false;
	}
	return true;
}

void BD_CALL bdPlatformThread::joinThread(bdThreadHandle& handle)
{
	bdInt res = sceKernelWaitThreadEnd(handle, NULL);
	assert(res >= 0);
}

void BD_CALL bdPlatformThread::deleteThread(bdThreadHandle& handle)
{
	bdInt res = sceKernelDeleteThread(handle);
	assert(res >= 0);
}

void BD_CALL bdPlatformThread::setPriority(bdThreadHandle& handle, 
										   const bdInt priority)
{
	bdInt res = sceKernelChangeThreadPriority(handle, priority);
	assert(res >= 0);
}

bdInt32 BD_CALL bdPlatformThread::getPriority(bdThreadHandle& handle)
{
	SceKernelThreadInfo threadInfo;
	if (sceKernelReferThreadStatus(handle, &threadInfo) == SCE_KERNEL_ERROR_OK)
	{
		return threadInfo.currentPriority;
	}
	return -1;
}

bdInt BD_CALL bdPlatformThread::getPriority()
{
	bdThreadHandle threadHandle = getThreadHandle();
	return getPriority(threadHandle);
}


bdThreadHandle BD_CALL bdPlatformThread::getThreadHandle()
{
	bdThreadHandle handle = sceKernelGetThreadId();
	return handle;
}
