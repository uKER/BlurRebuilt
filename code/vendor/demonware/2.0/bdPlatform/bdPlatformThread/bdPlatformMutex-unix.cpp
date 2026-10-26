// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformMutex.h>

bdMutexHandle BD_CALL bdPlatformMutex::createMutex()
{
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, 0);
	return mutex;
}

void BD_CALL bdPlatformMutex::lock(bdMutexHandle &handle)
{
	pthread_mutex_lock(&handle);
}

void BD_CALL bdPlatformMutex::unlock(bdMutexHandle &handle)
{
	pthread_mutex_unlock(&handle);
}

void BD_CALL bdPlatformMutex::destroy(bdMutexHandle &handle)
{
	pthread_mutex_destroy(&handle);
}

