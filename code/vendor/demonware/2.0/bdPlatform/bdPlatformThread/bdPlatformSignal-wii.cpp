// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformThread/bdPlatformSignal.h>
#include <bdPlatform/bdPlatformThread/bdPlatformMutex.h>
#include <revolution/os.h>

#define BD_LOG_LEVEL "platforrm/signal"

bdSignalHandle BD_CALL bdPlatformSignal::create()
{
	bdSignalHandle handle;
	OSInitCond (&handle);
	return handle;
}

void BD_CALL bdPlatformSignal::destroy(bdSignalHandle &handle)
{
	OSSignalCond (&handle);
}

void BD_CALL bdPlatformSignal::signal(bdSignalHandle &handle)
{
	OSSignalCond (&handle);
}

bdBool BD_CALL bdPlatformSignal::wait(bdSignalHandle &handle)
{
	void OSWaitCond (OSCond* cond, OSMutex* mutex);

	bdMutexHandle mutex;
	mutex = bdPlatformMutex::createMutex();
	bdPlatformMutex::lock(mutex);
	OSWaitCond (&handle, &mutex);

	bdPlatformMutex::unlock(mutex);
	bdPlatformMutex::destroy(mutex);
	return true;
}


