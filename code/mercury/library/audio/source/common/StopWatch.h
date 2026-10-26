#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

#if defined (PLATFORM_PS3)
#include <sys/sys_time.h>
#include <sys/time_util.h>
#endif

//--------------------------------------------------------------------------------------
// Name: Stopwatch
// Desc: Simple stopwatch class for timing events
//--------------------------------------------------------------------------------------
class StopWatch
{
public:
	StopWatch()
	{
#if defined(PLATFORM_PC)
		#ifndef CFG_GOLD
		QueryPerformanceFrequency(&m_TicksPerSecond);	
		m_s64StartTime.QuadPart = 0;
		Start();
		#endif
#endif
#if defined (PLATFORM_PS3)
		#ifndef CFG_GOLD
		m_TicksPerSecond = sys_time_get_timebase_frequency();	
		m_s64StartTime = 0;
		Start();
		#endif
#endif
	}
	inline void Start() 
	{ 
#if defined(PLATFORM_PC)
		#ifndef CFG_GOLD
		QueryPerformanceCounter(&m_s64StartTime);	
		#endif
#endif
#if defined (PLATFORM_PS3)
		#ifndef RELCFG_GOLDEASE_TO_MASTER
		#ifndef CFG_GOLD
//		m_s64StartTime = sys_time_get_system_time();	
		SYS_TIMEBASE_GET(m_s64StartTime);
		#endif
		#endif
#endif
	}
	// get current time. time is in seconds.
	// can be called repeatedly.
	inline F32 Time()
	{
#if defined(PLATFORM_PC)
		#ifndef CFG_GOLD
		LARGE_INTEGER s64EndTime;
		QueryPerformanceCounter(&s64EndTime); 
		__int64 s64Ticks = s64EndTime.QuadPart - m_s64StartTime.QuadPart;
		F32 fElapsedTime = s64Ticks / (F32)m_TicksPerSecond.QuadPart;
		return fElapsedTime;
		#else
		return 0.0f;
		#endif
#endif
#if defined (PLATFORM_PS3)
		#ifndef CFG_GOLD
		system_time_t s64EndTime;
//		s64EndTime = sys_time_get_system_time(); 
		SYS_TIMEBASE_GET(s64EndTime);
		system_time_t s64Ticks = s64EndTime - m_s64StartTime;
		F32 fElapsedTime = s64Ticks / (F32)m_TicksPerSecond;
		return fElapsedTime;
		#else
		return 0.0f;
		#endif
#endif
		return 0.0f;
	}

public:
#if defined(PLATFORM_PC)
	#ifndef CFG_GOLD
	LARGE_INTEGER m_s64StartTime;
	LARGE_INTEGER m_TicksPerSecond;
	#endif
#endif
#if defined (PLATFORM_PS3)
	#ifndef CFG_GOLD
//	system_time_t m_s64StartTime;
	uint64_t m_s64StartTime;
	uint64_t m_TicksPerSecond;
	#endif
#endif
};
//uint64_t sys_time_get_timebase_frequency  sys_time_get_system_time
#endif // _STOPWATCH_H_
