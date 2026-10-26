#include "SystemPch.h"

#include <core/threads/Misc.h>
#include <system/time/ScopedTimer.h>
#include <system/time/Time.h>

namespace Time
{

#ifndef CFG_GOLD

Debug::Logger &g_TimeLog = Debug::GetLogger("Mercury.Time");

C_ScopedTimer::C_ScopedTimer(const char* name, F32 logThresholdMs)	:
	m_Name(name),
	m_LogThresholdMs(logThresholdMs),
	m_StartTime( Time::GetPerformanceCounter() )
{		
}

C_ScopedTimer::~C_ScopedTimer()
{
	F32 deltaTime = ((F32)(Time::GetPerformanceCounter() - m_StartTime))/1000.f;
	if (deltaTime > m_LogThresholdMs)
	{
		#ifdef COMPILER_VC
		LOGINFO(g_TimeLog, "TID %I64u %s took %.2fms\n", MT::GetThreadId(), m_Name, deltaTime);
		#else
		LOGINFO(g_TimeLog, "TID %llu %s took %.2fms\n", MT::GetThreadId(), m_Name, deltaTime);
		#endif
	}
}

#endif

}
