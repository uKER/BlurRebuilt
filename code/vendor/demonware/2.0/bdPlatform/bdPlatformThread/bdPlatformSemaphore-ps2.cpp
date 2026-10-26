// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformSemaphore.h>
#include <assert.h>

bdMutexHandle BD_CALL bdPlatformSemaphore::createSemaphore(bdUInt32 initialCount,
														   bdUInt32 maxCount)
{
	SemaParam param;
	param.initCount = initialCount;
	param.maxCount = maxCount;
	return CreateSema(&param);
}

void BD_CALL bdPlatformSemaphore::destroy(bdMutexHandle &handle)
{
	DeleteSema(handle);
}

void BD_CALL bdPlatformSemaphore::release(bdMutexHandle &handle)
{
	SignalSema(handle);
}

bdBool BD_CALL bdPlatformSemaphore::wait(bdMutexHandle &handle)
{
	return (WaitSema(handle)!= -1);
}


