// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

#if !defined (_INC_WINDOWS)     // Already include windows.h ?
// Must define strict before including windows.h !
# if !defined (STRICT)
#  define STRICT 1
# endif // STRICT
# if !defined (WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN
# endif // !defined (WIN32_LEAN_AND_MEAN)
#endif // !defined (_INC_WINDOWS)

#include <windows.h>

bdUInt64 BD_CALL bdPlatformTiming::getHiResTimeStamp()
{
	LARGE_INTEGER time;
	
	::QueryPerformanceCounter(&time);

	return static_cast<bdUInt64>(time.QuadPart);
}

bdFloat32 BD_CALL bdPlatformTiming::getElapsedTime(const bdUInt64 time1, 
												   const bdUInt64 time2)
{
	LARGE_INTEGER frequency;
	// we have to check the freq every time
	// because of those people with SpeedStep or PowerNow 
	::QueryPerformanceFrequency(&frequency);
	
	LARGE_INTEGER begin;
	begin.QuadPart = time1;

	LARGE_INTEGER end;
	end.QuadPart = time2;

	const bdFloat32 elapedTime = (end.QuadPart - begin.QuadPart) / static_cast<bdFloat32>(frequency.QuadPart);
	return elapedTime;
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
