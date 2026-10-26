// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

#include <sys/time.h>
#include <unistd.h>

bdUInt64 BD_CALL bdPlatformTiming::getHiResTimeStamp()
{
	struct timeval tv;
	// returns time in seconds and microseconds
	gettimeofday(&tv, 0);

	bdUInt64 time;

	// multiply seconds by 10^6
	time = static_cast<bdUInt64>(tv.tv_sec) * 1000000;
	// add on microseconds
	time += tv.tv_usec;

	return time;
}

bdFloat32 BD_CALL bdPlatformTiming::getElapsedTime(const bdUInt64 time1, const bdUInt64 time2)
{
	const bdUInt64 elapsedTime = time2 - time1;

	return static_cast<bdFloat32>(elapsedTime) / 1000000;
}

void BD_CALL bdPlatformTiming::sleep(bdUInt32 timeInMs)
{
	usleep(timeInMs * 1000);
}

bdUInt32 BD_CALL bdPlatformTiming::getLoResTimeStamp()
{
	struct timeval tv;
	gettimeofday(&tv, 0);

	const bdUInt32 msTimestamp = ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
	return msTimestamp ;
}

bdUInt32 BD_CALL bdPlatformTiming::getLoResElapsedTime(const bdUInt32 time1,
													   const bdUInt32 time2)
{
	return ((time2 - time1) / 1000);
}
