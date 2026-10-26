// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>
#include <bdPlatform/bdPlatformThread/bdPlatformMutex.h>
#include <bdPlatform/bdPlatformThread/bdPlatformSignal.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#include <pthread.h>

bdSignalHandle BD_CALL bdPlatformSignal::create()
{
	bdSignalHandle handle;
	pthread_cond_init(&handle, BD_NULL);
	return handle;
}

void BD_CALL bdPlatformSignal::destroy(bdSignalHandle &handle)
{
	pthread_cond_destroy(&handle);
}

void BD_CALL bdPlatformSignal::signal(bdSignalHandle &handle)
{
	pthread_cond_signal(&handle);	
}

bdBool BD_CALL bdPlatformSignal::wait(bdSignalHandle &handle)
{
	pthread_mutex_t mutex;
	const bdInt initResult = pthread_mutex_init(&mutex, BD_NULL);
	if (initResult != 0)
	{
		bdFprintf(BD_STDERR, "pthread_mutex_init failed %i\n", initResult);
		return false;
	}
	pthread_mutex_lock(&mutex);
	const bdInt waitResult = pthread_cond_wait(&handle, &mutex);
	if (waitResult != 0)
	{
		bdFprintf(BD_STDERR, "pthread:_cond_wait failed %i\n", waitResult);
		return false;
	}
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);

	return true;
}




