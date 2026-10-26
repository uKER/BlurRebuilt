// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>
#include <eekernel.h>

#define EE_CLK_FREQ      294912000.0f          // EE clock frequency

static bdUInt32 lastTime =0;
static bdUInt64 offset = 0;

#undef sleep

bdUInt64 BD_CALL bdPlatformTiming::getHiResTimeStamp()
{
	bdUInt32 clock;
	
	//read the clock tick
    asm volatile ("mfc0 %0,$9" :"=r"(clock));

	bdUInt64 clockTick = clock;
	if (clock < lastTime)   
	{
		// add number of clockticks per wrap around interval
		offset += static_cast<bdUInt64>(294912000 * 14.563); 
	}
	lastTime = clock;
	clockTick += offset;
    return clockTick;
}

bdFloat32 BD_CALL bdPlatformTiming::getElapsedTime(const bdUInt64 time1, const bdUInt64 time2)
{
	const bdUInt64 elapsedTime = time2 - time1;
	if (time2 < time1)
	{
		return 0.0f;
	}
	return static_cast<bdFloat32>(elapsedTime) / EE_CLK_FREQ;
}

void BD_CALL bdPlatformTiming::sleep(bdUInt32 timeInMs)
{
	DelayThread(timeInMs * 1000);
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
