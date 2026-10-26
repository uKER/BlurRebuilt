// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformThread/bdPlatformSignal.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#include <kernel.h>
#include <assert.h>

bdSignalHandle BD_CALL bdPlatformSignal::create()
{
	static bdNChar8 sigName[10];
	static bdUInt  sigCnt = 0;
	bdSnprintf(sigName, 10, "Signal%u", sigCnt++);
	return sceKernelCreateSema(sigName, SCE_KERNEL_SA_THFIFO, 0, 10, NULL);
	
}

void BD_CALL bdPlatformSignal::destroy(bdSignalHandle &handle)
{
	bdInt res = sceKernelDeleteSema(handle);
	assert(res >= 0);
}

void BD_CALL bdPlatformSignal::signal(bdSignalHandle &handle)
{
	bdInt res = sceKernelSignalSema(handle, 1);
	assert(res >= 0);
}

bdBool BD_CALL bdPlatformSignal::wait(bdSignalHandle &handle)
{
	return (sceKernelWaitSema(handle, 1, NULL)== SCE_KERNEL_ERROR_OK);
}


