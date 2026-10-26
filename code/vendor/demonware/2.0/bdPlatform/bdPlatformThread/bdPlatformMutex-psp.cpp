// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformMutex.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#include <kernel.h>
#include <assert.h>

bdMutexHandle BD_CALL bdPlatformMutex::createMutex()
{
	static bdNChar8 semName[10];
	static bdUInt  semCnt = 0;
	bdSnprintf(semName, 10, "Mutex%u", semCnt++);
	return sceKernelCreateSema(semName, SCE_KERNEL_SA_THFIFO, 1, 1, NULL);
}

void BD_CALL bdPlatformMutex::lock(bdMutexHandle &handle)
{
	bdInt res = sceKernelWaitSema(handle, 1, NULL);
	assert(res >= 0);
}

void BD_CALL bdPlatformMutex::unlock(bdMutexHandle &handle)
{
	bdInt res = sceKernelSignalSema(handle, 1);
	assert(res >= 0);
}

void BD_CALL bdPlatformMutex::destroy(bdMutexHandle &handle)
{
	bdInt res = sceKernelDeleteSema(handle);
	assert(res >= 0);
}

