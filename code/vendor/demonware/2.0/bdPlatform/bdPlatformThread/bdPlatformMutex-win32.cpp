// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformMutex.h>

bdMutexHandle BD_CALL bdPlatformMutex::createMutex()
{
	return CreateMutex(0, false, 0);
}

void BD_CALL bdPlatformMutex::lock(bdMutexHandle &handle)
{
	WaitForSingleObject(handle,INFINITE);
}

void BD_CALL bdPlatformMutex::unlock(bdMutexHandle &handle)
{
	ReleaseMutex(handle);
}

void BD_CALL bdPlatformMutex::destroy(bdMutexHandle &handle)
{
	ReleaseMutex(handle);
	CloseHandle(handle);
}

