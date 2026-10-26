#include "SystemPch.h"

#include "system/Perf/ProfilerEnable.h"

#ifdef ENABLE_MERCURY_PROFILER
#ifndef PLATFORM_SPU
#include <core/Threads.h>
#include <system/Time/Time.h>
#include "System/Perf/ProfilerBackend.h"
#include "System/Perf/ProfilerNetworkCode.h"
#include "System/Perf/ProfilerUtil.h"
#endif
#ifdef PLATFORM_PS3
#include <sys/ppu_thread.h>
#include <sys/time_util.h>
#include <sys/dbg.h>
#endif
#endif

namespace Perf
{

#ifdef ENABLE_MERCURY_PROFILER



/// emit a perf event
void EmitEvent(const char *pszName, U64 iTime, U32 iThread, U8 iCore, U8 iEnd)
{
#ifdef PLATFORM_PS3
	int iEvent = cellAtomicAdd32( (std::uint32_t *)&g_iProfilerCurrentEvent, 1 );
#elif  defined(COMPILER_VC)
    int iEvent = InterlockedExchangeAdd( (LONG*)&g_iProfilerCurrentEvent, 1 );
#else
	int iEvent = m_iCurrentEvent++;
#endif
	if (iEvent>=g_iProfilerNumEvents)
		return;
	volatile Event *pEvent = &g_pProfilerEvents[iEvent];

	pEvent->name = pszName;
	pEvent->thread = iThread;
	PROFILER_EVENT_SET(*pEvent, iTime, iCore, iEnd);
}

/// mark a perf event
FORCE_INLINE void NamedEvent(const char *pszName, int iEnd)
{
#ifdef PLATFORM_PS3
	int iEvent = cellAtomicAdd32( (std::uint32_t *)&g_iProfilerCurrentEvent, 1 );
#elif  defined(COMPILER_VC)
    int iEvent = InterlockedExchangeAdd( (LONG*)&g_iProfilerCurrentEvent, 1 );
#else
	int iEvent = m_iCurrentEvent++;
#endif
	if (iEvent>=g_iProfilerNumEvents)
		return;
	volatile Event *pEvent = &g_pProfilerEvents[iEvent];

	U8 iCore;
	U32 iThread;
	U64 iTime;
#if defined( PLATFORM_PC )
	iTime = Time::GetPerformanceCounter();
	iThread = MT::Thread::GetCurrentThreadID();
	iCore = 0; // only on windows vista
#elif defined( PLATFORM_PS3 )
	// TODO: implement this on PS3
	sys_ppu_thread_t tid;
	sys_ppu_thread_get_id(&tid);
	iThread = tid;
	iCore = 1 + 0;
	SYS_TIMEBASE_GET(iTime);
#else
	iThread = 0;
	iTime = Time::GetPerformanceCounter();
	iCore = 0;
#endif
	pEvent->name = pszName;
	pEvent->thread = iThread;
	PROFILER_EVENT_SET(*pEvent, iTime, iCore, iEnd);
}

/// mark the start of a perf event
void BeginNamedEvent(const char *pszName)
{
	NamedEvent(pszName, 0);
}

/// mark the end of a perf event
void EndNamedEvent()
{
	NamedEvent(NULL, 1);
}

void AddNamedCounter(float fValue, const char *pszName)
{
#ifdef PLATFORM_PS3
	int iEvent = cellAtomicAdd32( (std::uint32_t *)&g_iProfilerCurrentEvent, 1 );
#elif  defined(COMPILER_VC)
    int iEvent = InterlockedExchangeAdd( (LONG*)&g_iProfilerCurrentEvent, 1 );
#else
	int iEvent = m_iCurrentEvent++;
#endif
	if (iEvent>=g_iProfilerNumEvents)
		return;
	volatile Event *pEvent = &g_pProfilerEvents[iEvent];
	pEvent->name = pszName;
	pEvent->thread = 0;
	pEvent->time = ((int&)fValue) | 0x4000000000000000ULL;

}

void DumpPerfData()
{
	Perf::ProfilerCommon::Instance().RegisterOnlyOnceCallback(Perf::g_DumpPerfCallbackFunction);
}

void CapturePerfData(const char* captureFileName)
{
	Perf::ProfilerCommon::Instance().RegisterOnlyOnceCallback(Perf::g_CapturePerfCallbackFunction, (void*)captureFileName);
}

#endif

}

#if PLATFORM_PC

namespace Perf
{
extern Debug::Logger &g_Log;
}

void PIN_BeginTrace(void *pEventBuf, void *piCurrentEvent, void *iMaxEvent)
{
	LOGINFO( Perf::g_Log, "Buffer: %d\n", (int)(INTPTR)pEventBuf);	// [Blur2PC][x64]
	LOGINFO( Perf::g_Log, "IdxPtr: %d\n", (int)(INTPTR)piCurrentEvent);	// [Blur2PC][x64]
	LOGINFO( Perf::g_Log, "MaxEvt: %d\n", (int)(INTPTR)iMaxEvent);	// [Blur2PC][x64]
}

void PIN_EndTrace()
{
	LOGINFO( Perf::g_Log, "---------------------------------" );
	LOGINFO( Perf::g_Log, "         PIN Trace End ");
	LOGINFO( Perf::g_Log, "---------------------------------" );
}

namespace Perf
{

void BeginTrace()
{
#ifdef ENABLE_MERCURY_PROFILER
	PIN_BeginTrace(g_pProfilerEvents, &g_iProfilerCurrentEvent, (void*)g_iProfilerNumEvents);
#endif
}

void EndTrace()
{
	PIN_EndTrace();
}

} // namespace Perf

#endif
