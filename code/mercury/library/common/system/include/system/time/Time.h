#ifndef __SYSTEM_TIME_H__
#define __SYSTEM_TIME_H__

#include <core/containers/String.h>

namespace Time
{

#if defined( PLATFORM_PC )
#undef GetCurrentTime
#endif

// number of microseconds per second
#define SYS_USEC_PER_SEC (1000000UL)

/* Number of micro-seconds between the beginning of the Windows epoch
 * (Jan. 1, 1601) and the Unix epoch (Jan. 1, 1970) 
 */
#define SYS_DELTA_EPOCH_IN_USEC   (11644473600000000UL)

/// A time and date, measured in microseconds since the Unix epoch (UMT)
typedef U64 C_Time;

/// A time and date, decomposed into minutes, hours, years, etc.
class C_DecomposedTime
{
public:
	C_DecomposedTime(C_Time time, bool forceToGMT=false);

	S32 m_sec;     // seconds after the minute - [0,59]
	S32 m_min;     // minutes after the hour - [0,59]
	S32 m_hour;    // hours since midnight - [0,23]
	S32 m_mday;    // day of the month - [1,31]
	S32 m_mon;     // months since January - [0,11]
	S32 m_year;    // years since 1900
	S32 m_wday;    // days since Sunday - [0,6]
	S32 m_yday;    // days since January 1 - [0,365]
	S32 m_isdst;   // daylight savings time flag
};

/// returns the current time and date
C_Time GetCurrentTime();

/// returns the system performance counter. This has a higher resolution that GetCurrentTime, but can't be used to determine the actual time and date.
C_Time GetPerformanceCounter();

/// returns the C_Time frequency. This is always a time in uSec so we just return the number of uSec in a second
__forceinline const U64 GetPerformanceFrequency() { return SYS_USEC_PER_SEC; }

/// Format a time value as "3-12-06"
Util::C_String FormatDateString(C_DecomposedTime time);

/// Format a time value as "Mon 3 Dec 2006"
Util::C_String FormatLongDateString(C_DecomposedTime time);

/// Format a time value as "11:12:55"
Util::C_String FormatTimeString(C_DecomposedTime time);

/// Format a time value as "11-12-55" (valid for filenames)
Util::C_String FormatTimeString2(C_DecomposedTime time);

///	Format a time value as "3-12-06_11-12-55" (valid for filenames)
Util::C_String FormatDate_TimeString(C_DecomposedTime time);

/// Format a duration value as "4 days 11 hours 12 mins 55 secs"
Util::C_String FormatDurationString(C_Time duration);

/// Format a profile duration value as "12m 55s 33ms 44µs"
Util::C_String FormatProfileDurationString(C_Time duration);
}

#endif

