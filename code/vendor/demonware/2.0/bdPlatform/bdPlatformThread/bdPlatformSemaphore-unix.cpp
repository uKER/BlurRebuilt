// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformSemaphore.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>

bdSemaphoreHandle BD_CALL bdPlatformSemaphore::createSemaphore(bdUInt32 initialCount,
															   bdUInt32 maxCount)
{
	sem_t semaphore;
	bdMemset(&semaphore, 0, sizeof(sem_t));
	sem_init(&semaphore, 0, initialCount);
	return semaphore;
}

void BD_CALL bdPlatformSemaphore::destroy(bdSemaphoreHandle &handle)
{
	int res;
	res = sem_destroy(&handle);
}

void BD_CALL bdPlatformSemaphore::release(bdSemaphoreHandle &handle)
{
	sem_post(&handle);
}

bdBool BD_CALL bdPlatformSemaphore::wait(bdSemaphoreHandle &handle)
{
	sem_wait(&handle);
	return true;
}
