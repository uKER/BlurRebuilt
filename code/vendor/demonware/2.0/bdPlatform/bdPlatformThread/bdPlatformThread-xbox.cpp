// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS



#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>

#include <xtl.h>

bdBool BD_CALL bdPlatformThread::createThread(bdThreadProc threadProc, 
											  void *args,
											  bdThreadHandle &handle,
											  bdInt priority,
											  bdUInt stackSize,
											  void *)
{
	handle = CreateThread(0, stackSize, threadProc, args, CREATE_SUSPENDED , 0);
	if (handle > 0)
	{
		setPriority( handle, priority );
		return true;
	}
	return false;
}

bdBool BD_CALL bdPlatformThread::startThread(bdThreadHandle &handle, 
											 void *, bdUInt priority)
{
	bdInt32 res = ResumeThread(handle);
	if (res == -1)
	{
		setPriority( handle, priority );
		return false;
	}
	return true;
}

void BD_CALL bdPlatformThread::joinThread(bdThreadHandle &handle)
{
	WaitForSingleObject(handle, INFINITE);
}


void BD_CALL bdPlatformThread::deleteThread(bdThreadHandle &handle)
{
	CloseHandle(handle);
}

void BD_CALL bdPlatformThread::setPriority(bdThreadHandle &handle, 
										   bdInt priority)
{
	SetThreadPriority(handle, priority);
}

bdInt BD_CALL bdPlatformThread::getPriority(bdThreadHandle &handle)
{
	return GetThreadPriority(handle);
}

bdInt32 BD_CALL bdPlatformThread::getPriority()
{
	return GetThreadPriority(GetCurrentThread());
}
