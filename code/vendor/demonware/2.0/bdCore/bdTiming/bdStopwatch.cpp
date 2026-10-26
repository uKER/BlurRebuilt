// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdCore/bdTiming/bdStopwatch.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

bdStopwatch::bdStopwatch()
{
	reset();
}

void bdStopwatch::start()
{
	m_start = bdPlatformTiming::getHiResTimeStamp();
}

void bdStopwatch::reset()
{
	m_start = 0;
}

bdFloat32 bdStopwatch::getElapsedTimeInSeconds() const
{
	if(m_start != 0)
	{
		// establish length of time between m_start & now
		const bdUInt64 nowTimeStamp = bdPlatformTiming::getHiResTimeStamp();
		return bdPlatformTiming::getElapsedTime(m_start, nowTimeStamp);
	}
	else
	{
		return 0.f;
	}
}
