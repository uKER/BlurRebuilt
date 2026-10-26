#include "SystemPch.h"

#include "System/Debug/Variables.h"
#include "System/Perf/CPUTiming.h"
#include "system/Perf/ProfilerEnable.h"
#ifdef ENABLE_MERCURY_PROFILER
#include "System/Perf/ProfilerBackend.h"
#include "System/Perf/Profiler.h"
#endif

#if 1 // [LloydW] Was wondering were these had gone. If you need them in "System.Perf.CPUTiming" can we alias or something so they are also in "Graphics.Display.Render.TimingRender".
Debug::Int		g_debugDisplayCPUTimingDepth	("Graphics.Display.Render.TimingRender.CPUDepth", 		9, Debug::Range(0, 9));
Debug::Int		g_debugDisplayCPUTimingLevel	("Graphics.Display.Render.TimingRender.CPULevel", 		2, Debug::Range(0, 9));
Debug::Bool		g_debugDisplayCPUUpdateStats	("Graphics.Display.Render.TimingRender.CPUUpdateStats",	true);
#else
Debug::Int		g_debugDisplayCPUTimingDepth	("System.Perf.CPUTiming.CPUDepth", 		9, Debug::Range(0, 9));
Debug::Int		g_debugDisplayCPUTimingLevel	("System.Perf.CPUTiming.CPULevel", 		2, Debug::Range(0, 9));
Debug::Bool		g_debugDisplayCPUUpdateStats	("System.Perf.CPUTiming.CPUUpdateStats",	true);
#endif

namespace Perf
{

// C_CPUTimingStats

Util::C_List<C_CPUTimingStats> C_CPUTimingStats::m_StatsList;

C_CPUTimingStats::C_CPUTimingStats(const S32 level, const Maths::RGBA colour, const Util::C_String80& name) :
	C_TimingStats(level, colour, name)
{
	m_StatsList.PushLast(this);
}

C_CPUTimingStats::~C_CPUTimingStats()
{
	m_StatsList.Remove(this);
}

const bool C_CPUTimingStats::Show() const
{
	if(m_Depth != -1 && m_Depth <= g_debugDisplayCPUTimingDepth && m_Level <= g_debugDisplayCPUTimingLevel)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void C_CPUTimingStats::UpdateStats()
{
	if(!g_debugDisplayCPUUpdateStats)
		return;

	C_CPUTimingStats* pStats = m_StatsList.First();
	while(pStats)
	{
		pStats->Update();
		
		pStats = pStats->Next();
	}
}

const C_CPUTimingStats *C_CPUTimingStats::FindStats(const char *psz)
{
	for (C_CPUTimingStats* pStats = m_StatsList.First(); pStats; pStats = pStats->Next())
	{
		if (!stricmp(pStats->m_Name.Chars(), psz))
			return pStats;
	}

	return NULL;
}

void C_CPUTimingStats::ResetPeaks()
{
	C_CPUTimingStats* pStats = m_StatsList.First();
	while(pStats)
	{
		pStats->ResetPeak();
		pStats = pStats->Next();
	}
}

// C_CPUTimingEvent

MT::ThreadLocalStorage C_CPUTimingEvent::m_Tls;

C_CPUTimingEvent::C_CPUTimingEvent(C_CPUTimingStats& stats) // __restrict // JASOND: Intel compiler doesn't allow this...
{
	C_CPUTimingEvent* const __restrict pParent = (C_CPUTimingEvent*)m_Tls.GetData();

	m_Tls.SetData(this);

	m_pParent	= pParent;
	m_pStats	= &stats;
	m_Time		= Time::GetPerformanceCounter();
	m_ChildTime	= 0;
	m_Depth		= m_pParent ? m_pParent->m_Depth+1 : m_Depth = 0;

	if(m_Depth <= g_debugDisplayCPUTimingDepth && stats.m_Level <= g_debugDisplayCPUTimingDepth)
	{
		stats.SetDepth(m_Depth);

		stats.m_Enabled = true;
	}
	else
	{
		stats.m_Enabled = false;
	}

#ifdef ENABLE_MERCURY_PROFILER
	Perf::BeginNamedEvent(stats.m_Name.Chars());
#endif
}

C_CPUTimingEvent::~C_CPUTimingEvent() // __restrict  // JASOND: Intel compiler doesn't allow this...
{
	C_CPUTimingStats* const __restrict pTimings(m_pStats);
	C_CPUTimingEvent* const __restrict pParent(m_pParent);
	
	m_Time = Time::GetPerformanceCounter()-m_Time;

	if(pParent)
	{
		pParent->m_ChildTime += m_Time;
	}

	pTimings->AddCallCount(E_TimingFrame_Curr, 1);
	pTimings->AddTotalTime(E_TimingFrame_Curr, m_Time);

	m_Tls.SetData(pParent);
#ifdef ENABLE_MERCURY_PROFILER
	Perf::EndNamedEvent();
#endif
}

} // namespace Perf
