// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformSemaphore.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>

#include <revolution/os.h>

bdSemaphoreHandle BD_CALL bdPlatformSemaphore::createSemaphore(bdUInt32 initialCount,
															   bdUInt32 /*maxCount*/)
{

	OSSemaphore semaphore;
	OSInitSemaphore(&semaphore, initialCount);
	return semaphore;
}

void BD_CALL bdPlatformSemaphore::destroy(bdSemaphoreHandle &handle)
{
	release(handle);
}

void BD_CALL bdPlatformSemaphore::release(bdSemaphoreHandle &handle)
{
	const s32 result = OSSignalSemaphore ( &handle );
}

bdBool BD_CALL bdPlatformSemaphore::wait(bdSemaphoreHandle &handle)
{
	const s32 result = OSWaitSemaphore(&handle );
	return true;
}
