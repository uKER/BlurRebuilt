// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

#include <sys/sys_time.h>
#include <sys/timer.h>

bdUInt64 BD_CALL bdPlatformTiming::getHiResTimeStamp()
{
	bdInt64 time = sys_time_get_system_time();

	return static_cast<bdUInt64>(time);
}

bdFloat32 BD_CALL bdPlatformTiming::getElapsedTime(const bdUInt64 time1, const bdUInt64 time2)
{
	bdInt64 t1 = static_cast<bdInt64>(time1);
	bdInt64 t2 = static_cast<bdInt64>(time2);

	const bdInt64 elapsedTime = t2 - t1;
	if (t2 < t1)
	{
		return 0.0f;
	}

	return static_cast<bdFloat32>(elapsedTime) / 1000000.0f;
}

void BD_CALL bdPlatformTiming::sleep(bdUInt32 timeInMs)
{
	// TODO: undocumented function.
	sys_timer_usleep(timeInMs * 1000);
}

bdUInt32 BD_CALL bdPlatformTiming::getLoResTimeStamp()
{
	return static_cast<bdUInt32>(bdPlatformTiming::getHiResTimeStamp() / BD_UINT32_MAX);
}

bdUInt32 BD_CALL bdPlatformTiming::getLoResElapsedTime(const bdUInt32 time1,
													   const bdUInt32 time2)
{
	return static_cast<bdUInt32>(
			bdPlatformTiming::getElapsedTime(static_cast<bdUInt64>(time1) * BD_UINT32_MAX,
											 static_cast<bdUInt64>(time2) * BD_UINT32_MAX));
}
