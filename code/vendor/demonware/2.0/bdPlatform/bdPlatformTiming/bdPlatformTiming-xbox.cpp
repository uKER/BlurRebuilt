// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

bdUInt64 BD_CALL bdPlatformTiming::getHiResTimeStamp()
{
	LARGE_INTEGER time;
	
	::QueryPerformanceCounter(&time);
	
	return time.QuadPart;
}

bdFloat32 BD_CALL bdPlatformTiming::getElapsedTime(const bdUInt64 time1, const bdUInt64 time2)
{
	static bdBool gotFreq = false;
	static LARGE_INTEGER frequency;

	// just need to check this once on xbox
	if(!gotFreq)
	{
		::QueryPerformanceFrequency(&frequency);
		gotFreq = true;
	}

	const bdUInt64 elapsedTime = time2 - time1;

	return static_cast<bdFloat32>(elapsedTime) / frequency.QuadPart;
}

void BD_CALL bdPlatformTiming::sleep(bdUInt timeInMs)
{
	Sleep(timeInMs);
}

bdUInt32 BD_CALL bdPlatformTiming::getLoResTimeStamp()
{
	return ::GetTickCount();
}

bdUInt32 BD_CALL bdPlatformTiming::getLoResElapsedTime(const bdUInt32 time1,
													   const bdUInt32 time2)
{
	bdUInt64 time2adjusted = time2;
	if (time2adjusted < time1)
	{
		// GetTickCount() time stamp wrapped.
		time2adjusted += BD_UINT32_MAX;
	}

	return static_cast<bdUInt32>((time2adjusted - time1) / 1000);
}