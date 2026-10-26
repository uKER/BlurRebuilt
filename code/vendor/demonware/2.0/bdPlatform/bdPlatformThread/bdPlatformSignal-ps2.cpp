// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformThread/bdPlatformSignal.h>
#include <assert.h>

bdSignalHandle BD_CALL bdPlatformSignal::create()
{
	SemaParam param;
	param.initCount = 0;
	param.maxCount = 1000;
	return CreateSema(&param);
}

void BD_CALL bdPlatformSignal::destroy(bdSignalHandle &handle)
{
	DeleteSema(handle);
}

void BD_CALL bdPlatformSignal::signal(bdSignalHandle &handle)
{
	SignalSema(handle);
}

bdBool BD_CALL bdPlatformSignal::wait(bdSignalHandle &handle)
{
	return (WaitSema(handle)!= -1);
}


