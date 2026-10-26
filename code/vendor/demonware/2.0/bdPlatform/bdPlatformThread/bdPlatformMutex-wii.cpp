// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformMutex.h>

bdMutexHandle BD_CALL bdPlatformMutex::createMutex()
{
	OSMutex handle;
	OSInitMutex(&handle);
	return handle;
}

void BD_CALL bdPlatformMutex::lock(bdMutexHandle &handle)
{
	OSLockMutex(&handle);
}

void BD_CALL bdPlatformMutex::unlock(bdMutexHandle &handle)
{
	OSUnlockMutex(&handle);
}

void BD_CALL bdPlatformMutex::destroy(bdMutexHandle &)
{
  // there seems to be no way to release a mutex
}

