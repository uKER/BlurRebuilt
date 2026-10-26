// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformThread/bdPlatformSignal.h>


bdSignalHandle BD_CALL bdPlatformSignal::create()
{
	return CreateEvent( 
						NULL,         // no security attributes
						FALSE,         // automatic-reset event
						FALSE,         // initial state is not signaled
						NULL  // object name
						);
}

void BD_CALL bdPlatformSignal::destroy(bdSignalHandle &handle)
{
	CloseHandle(handle);
}

void BD_CALL bdPlatformSignal::signal(bdSignalHandle &handle)
{
	SetEvent(handle);
}

bdBool BD_CALL bdPlatformSignal::wait(bdSignalHandle &handle)
{
	return WaitForSingleObject(handle, INFINITE) != WAIT_FAILED;
}


