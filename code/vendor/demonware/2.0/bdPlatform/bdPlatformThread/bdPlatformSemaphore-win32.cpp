// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformSemaphore.h>

bdMutexHandle BD_CALL bdPlatformSemaphore::createSemaphore(bdUInt initialCount,
														   bdUInt maxCount)
{
	return CreateSemaphore(BD_NULL, initialCount, maxCount, BD_NULL);
}

void BD_CALL bdPlatformSemaphore::destroy(bdMutexHandle &handle)
{
	ReleaseSemaphore(handle,1, BD_NULL);
	CloseHandle(handle);
}

void BD_CALL bdPlatformSemaphore::release(bdMutexHandle &handle)
{
	ReleaseSemaphore(handle, 1, BD_NULL);
}

bdBool BD_CALL bdPlatformSemaphore::wait(bdMutexHandle &handle)
{
	return (WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0);
}


