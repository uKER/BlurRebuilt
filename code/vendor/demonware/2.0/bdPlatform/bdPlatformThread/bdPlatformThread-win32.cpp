// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#define _WIN32_WINNT    0x0400

#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>

bdBool BD_CALL bdPlatformThread::createThread(bdThreadProc threadProc,
											  void *args,
											  bdThreadHandle &handle,
											  bdInt ,
											  bdUInt stackSize,
											  void *)
{
	LPDWORD threadID = 0;
	handle = CreateThread(0, stackSize, threadProc, args, CREATE_SUSPENDED , threadID);
	if (handle > 0)
	{
		return true;
	}
	return false;
}

bdBool BD_CALL bdPlatformThread::startThread(bdThreadHandle& handle,
											 void *, bdUInt)
{
	bdInt32 res = ResumeThread(handle);
	if (res == -1)
	{
		return false;
	}
	return true;
}

void BD_CALL bdPlatformThread::joinThread(bdThreadHandle& handle)
{
	WaitForSingleObject(handle, INFINITE);
}

void BD_CALL bdPlatformThread::deleteThread(bdThreadHandle& handle)
{
	CloseHandle(handle);
}

void BD_CALL bdPlatformThread::setPriority(bdThreadHandle& handle,
										   const bdInt priority)
{
	SetThreadPriority(handle, priority);
}

bdInt BD_CALL bdPlatformThread::getPriority(bdThreadHandle& handle)
{
	return GetThreadPriority(handle);
}

bdInt32 BD_CALL bdPlatformThread::getPriority()
{
	return GetThreadPriority(GetCurrentThread());
}
