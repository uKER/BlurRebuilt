#include "SystemPch.h"

#if defined( COMPILER_VC )
#if defined( PLATFORM_PC )
// windows version - needs windows.h unfortunately
// anyway this .h file is never included externally, so it's ok
#include <windows.h>
#include <time.h>
#endif
#elif defined( PLATFORM_PS3 )
#include <time.h>
#include <sys/sys_time.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

#include <system/time/time.h>

/** number of microseconds per second */
#ifndef SYS_USEC_PER_SEC 
#define SYS_USEC_PER_SEC (1000000UL)
#endif

namespace Time
{

#ifdef COMPILER_VC

static void FileTimeToAprTime(U64 *result, FILETIME *input)
{
    /* Convert FILETIME one 64 bit number so we can work with it. */
    *result = input->dwHighDateTime;
    *result = (*result) << 32;
    *result |= input->dwLowDateTime;
    *result /= 10;    /* Convert from 100 nano-sec periods to micro-seconds. */
    *result -= SYS_DELTA_EPOCH_IN_USEC;  /* Convert from Windows epoch to Unix epoch */
    return;
}

/// returns the current time and date
C_Time GetCurrentTime()
{
    U64 aprtime = 0;
    FILETIME time;

    GetSystemTimeAsFileTime(&time);

    FileTimeToAprTime(&aprtime, &time);
    return aprtime; 
}

/// returns the system performance counter. This has a higher resolution that GetCurrentTime, but can't be used to determine the actual time and date.
C_Time GetPerformanceCounter()
{
	C_Time result, freq;
	QueryPerformanceCounter((LARGE_INTEGER*)&result);

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	freq = freq / 10000L;

	return result*100L/freq;
}

#else

// linux version

/// returns the current time and date
C_Time GetCurrentTime()
{
#if defined( PLATFORM_PS3 )
	sys_time_sec_t sec;
	sys_time_nsec_t nsec;
	sys_time_get_current_time(&sec, &nsec);
	U64 result = (U64)sec * 1000000L + ((U64)nsec/1000L);
	return result;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * SYS_USEC_PER_SEC + tv.tv_usec;
#endif
}

/// returns the system performance counter. This has a higher resolution that GetCurrentTime, but can't be used to determine the actual time and date.
C_Time GetPerformanceCounter()
{
#ifdef PLATFORM_PS3
	return sys_time_get_system_time();
#else
	return GetCurrentTime();
#endif
}

#endif

/// A time and date, decomposed into minutes, hours, years, etc.
C_DecomposedTime::C_DecomposedTime(C_Time time, bool forceToGMT)
{
	time = time / SYS_USEC_PER_SEC;

	struct tm *p = (!forceToGMT) ? localtime((const time_t*)&time) : gmtime((const time_t*)&time);
	if (p)
	{
		m_sec = p->tm_sec;
		m_min = p->tm_min;
		m_hour = p->tm_hour;
		m_mday = p->tm_mday;
		m_mon = p->tm_mon;
		m_year = p->tm_year;
		m_wday = p->tm_wday;
		m_yday = p->tm_yday;
		m_isdst = p->tm_isdst;		
	}
	else
		assert(0);
}

/// Format a time value as "3-12-06"
Util::C_String FormatDateString(C_DecomposedTime time)
{
	Util::C_String out;
	out.Printf("%02d-%02d-%02d", time.m_mday, time.m_mon + 1, time.m_year%100);
	return out;
}

/// Format a time value as "Mon 3 Dec 2006"
Util::C_String FormatLongDateString(C_DecomposedTime time)
{
	char *pszMonths[]= { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	char *pszDays[]= { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	Util::C_String out;
	out.Printf("%s %d %s %4d", pszDays[time.m_wday], time.m_mday, pszMonths[time.m_mon], time.m_year+1900);
	return out;
}

/// Format a time value as "11:12:55"
Util::C_String FormatTimeString(C_DecomposedTime time)
{
	Util::C_String out;
	out.Printf("%02d:%02d:%02d", time.m_hour, time.m_min, time.m_sec);
	return out;
}

/// DG : Format a time value as "11-12-55" (valid for filenames)
Util::C_String FormatTimeString2(C_DecomposedTime time)
{
	Util::C_String out;
	out.Printf("%02d-%02d-%02d", time.m_hour, time.m_min, time.m_sec);
	return out;
}

///	Format a time value as "3-12-06_11-12-55" (valid for filenames)
Util::C_String FormatDate_TimeString(C_DecomposedTime time)
{
	Util::C_String out;
	out.Printf("%02d-%02d-%02d_%02d-%02d-%02d", time.m_mday, time.m_mon + 1, time.m_year%100, time.m_hour, time.m_min, time.m_sec);
	return out;
}


/// Format a duration value as "4 days 11 hours 12 mins 55 secs"
Util::C_String FormatDurationString(C_Time duration)
{
	Util::C_String out;

	duration = duration / 1000000;
	
	int secs = (int)(duration % 60);
	int mins = (int)((duration / 60) % 60);
	int hours = (int)((duration / (60*60)) % 24);
	int days = (int)(duration / (60*60*24));

	if (days>0)
		out.Printf("%d days %d hours %d mins", days, hours, mins);
	else if (hours>0)
		out.Printf("%d hours %d mins", hours, mins);
	else if (mins>0)
		out.Printf("%d mins %d secs", mins, secs);
	else
		out.Printf("%d secs", secs);
	return out;
}

Util::C_String FormatProfileDurationString( C_Time duration )
{
	Util::C_String out;
	
	int microsecs = (int)((duration) % 1000);
	int msecs = (int)((duration/1000) % 1000);
	int secs = (int)((duration/1000000) % 60);
	int mins = (int)(duration / 60000000);
	
	if (mins>0)
		out.Printf("%dm %ds %dms %d탎", mins, secs, msecs,microsecs);
	else if (secs>0)
		out.Printf("%ds %dms %d탎", secs, msecs,microsecs);
	else if (msecs>0)
		out.Printf("%dms %d탎", msecs,microsecs);
	else
		out.Printf("%d탎", microsecs);
	return out;
}
}
