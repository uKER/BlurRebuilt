// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformSemaphore.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#include <kernel.h>
#include <assert.h>

bdMutexHandle BD_CALL bdPlatformSemaphore::createSemaphore(bdUInt32 initialCount,
														   bdUInt32 maxCount)
{
	static bdNChar8 semName[10];
	static bdUInt  semCnt = 0;
	bdSnprintf(semName, 10, "Sem%u", semCnt++);
	return sceKernelCreateSema(semName, SCE_KERNEL_SA_THFIFO, initialCount, maxCount, NULL);
}

void BD_CALL bdPlatformSemaphore::destroy(bdMutexHandle &handle)
{
	bdInt res = sceKernelDeleteSema(handle);
	assert(res >= 0);
}

void BD_CALL bdPlatformSemaphore::release(bdMutexHandle &handle)
{
	bdInt res = sceKernelSignalSema(handle, 1);
	assert(res >= 0);
}

bdBool BD_CALL bdPlatformSemaphore::wait(bdMutexHandle &handle)
{
	return (sceKernelWaitSema(handle, 1, NULL)== SCE_KERNEL_ERROR_OK);
}


