#include "StdAfx.h"
#include "Timer.h"

#if PLATFORM_PS3
#include <sys/sys_time.h>
#endif

U64 GetPerformanceFrequency()
{
#if PLATFORM_PS3
	return 1000000;
#else
	LARGE_INTEGER time;
	QueryPerformanceFrequency(&time);
	return time.QuadPart;
#endif
}

U64 GetPerformanceCounter()
{
#if PLATFORM_PS3
	return sys_time_get_system_time();
#else
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return time.QuadPart;
#endif
}

C_Timer::C_Timer()
{
	m_Paused			= false;
	m_FrameTime 		= 0.0f;
	m_TotalTime 		= 0.0f;
	m_GameFrameTime		= 0.0f;
	m_GameTotalTime		= 0.0f;
}

void C_Timer::Update()
{
	static U64 oldTime = GetPerformanceCounter();
	U64 newTime = GetPerformanceCounter();

	m_FrameTime = (F32)(newTime-oldTime) / (F32)GetPerformanceFrequency();
	m_TotalTime += m_FrameTime;

	if(m_Paused)
	{
		m_GameFrameTime = 0.0f;
	}
	else
	{
		m_GameFrameTime = m_FrameTime;
		m_GameTotalTime += m_GameFrameTime;
	}

	oldTime = newTime;
}
