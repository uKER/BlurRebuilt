// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <revolution/os.h>

#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

bdUInt64 BD_CALL bdPlatformTiming::getHiResTimeStamp()
{
	const bdInt64 time = OSGetTime();
	return time;
}

bdFloat32 BD_CALL bdPlatformTiming::getElapsedTime(const bdUInt64 time1, 
												   const bdUInt64 time2)
{
	bdFloat32 elapsedTime = 0;

	const bdInt64 t1 = static_cast<bdInt64>(time1);
	const bdInt64 t2 = static_cast<bdInt64>(time2);
	
	if (t2 > t1)
	{
		const bdInt64 elapsedTicks = t2 - t1;
		elapsedTime = elapsedTicks / static_cast<bdFloat32>(OS_TIMER_CLOCK);
	}
	
	return elapsedTime;
}
	

void BD_CALL bdPlatformTiming::sleep(bdUInt32 timeInMs)
{
	// hack for RVL SDK that crashes if any sleep functions are passed zero.
	if(timeInMs == 0)
	{
		OSYieldThread();
	}
	else
	{
		OSSleepMilliseconds(timeInMs);
	}
}

bdUInt32 BD_CALL bdPlatformTiming::getLoResTimeStamp()
{
	const OSTime time = OSGetTime();
	const bdUInt32 timeInSeconds = OSTicksToSeconds(time);
	return timeInSeconds;
}

bdUInt32 BD_CALL bdPlatformTiming::getLoResElapsedTime(const bdUInt32 time1,
													   const bdUInt32 time2)
{
	const bdUInt32 elapsedTime = time2 - time1;
	return elapsedTime;
}
