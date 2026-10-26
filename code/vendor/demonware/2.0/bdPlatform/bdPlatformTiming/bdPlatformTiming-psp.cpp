// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

#include <powersvc.h>
#include <kernel.h>

bdUInt64 BD_CALL bdPlatformTiming::getHiResTimeStamp()
{
    return sceKernelGetSystemTimeWide();
}

bdFloat32 BD_CALL bdPlatformTiming::getElapsedTime(const bdUInt64 time1, const bdUInt64 time2)
{
	const bdUInt64 elapsedTime = time2 - time1;
	if (time2 < time1)
	{
		return 0.0f;
	}
	
	if (elapsedTime < BD_UINT32_MAX)
	{
		bdUInt32 tmp = static_cast<bdUInt32>(elapsedTime);
		return static_cast<bdFloat32>(tmp) / 1000000.0;
	}
	return static_cast<bdFloat32>(elapsedTime / 1000000.0);
}

void BD_CALL bdPlatformTiming::sleep(bdUInt32 timeInMs)
{
	sceKernelDelayThread(timeInMs * 1000);
}

bdUInt32 BD_CALL bdPlatformTiming::getLoResTimeStamp()
{
	return sceKernelGetSystemTimeLow();
}

bdUInt32 BD_CALL bdPlatformTiming::getLoResElapsedTime(const bdUInt32 time1,
													const bdUInt32 time2)
{
	bdUInt64 time2adjusted = time2;
	if (time2adjusted < time1)
	{
		// sceKernelGetSystemTimeLow() wrapped
		time2adjusted += BD_UINT32_MAX;
	}

	return static_cast<bdUInt32>((time2adjusted - time1) / 1000000);
}
