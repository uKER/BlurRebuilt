// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformMutex.h>
#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

#include <assert.h>

bdMutexHandle BD_CALL bdPlatformMutex::createMutex()
{
	SemaParam param;
	param.initCount = 1;
	param.maxCount = 1;
	return CreateSema(&param);
}

void BD_CALL bdPlatformMutex::lock(bdMutexHandle &handle)
{
	WaitSema(handle);
}

void BD_CALL bdPlatformMutex::unlock(bdMutexHandle &handle)
{
	SignalSema(handle);
}

void BD_CALL bdPlatformMutex::destroy(bdMutexHandle &handle)
{
	DeleteSema(handle);
}

