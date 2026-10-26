#ifndef DISPLAY_CPU_TIMING_TIMING_H
#define DISPLAY_CPU_TIMING_TIMING_H

#ifdef _MSC_VER
#pragma once
#endif

#include "System/Perf/Timing.h"
#include <Core/Containers/Genesis_Containers/List.h>
#include <Core/Threads/ThreadLocalStorage.h>

namespace Perf
{

//! C_CPUTimingStats

class C_CPUTimingStats : public C_TimingStats, public Util::C_Item<C_CPUTimingStats>
{
public:

	C_CPUTimingStats(const S32 level, const Maths::RGBA colour, const Util::C_String80& name);
	~C_CPUTimingStats();

	const bool Show() const;

	static void UpdateStats();
	static void ResetPeaks();

	static const C_CPUTimingStats *FindStats(const char *psz);

public:

	static Util::C_List<C_CPUTimingStats>		m_StatsList;
};

//! C_CPUTimingEvent

class C_CPUTimingEvent
{
public:

	C_CPUTimingEvent(C_CPUTimingStats& stats);
	~C_CPUTimingEvent();

public:

	S32											m_Depth;
	Time::C_Time								m_Time;
	Time::C_Time								m_ChildTime;

private:

	C_CPUTimingEvent* __restrict				m_pParent;
	C_CPUTimingStats* __restrict				m_pStats;

	static MT::ThreadLocalStorage				m_Tls;
};

} // namespace Perf

#if !defined(CFG_GOLD)
#define USE_CPU_TIMINGS
#endif

#ifdef USE_CPU_TIMINGS
#define CPU_TIMING(_level, _colour, _name) \
	static Perf::C_CPUTimingStats cpuTimingStats(_level, _colour, _name); \
	Perf::C_CPUTimingEvent cpuTimingEvent(cpuTimingStats)

#else
#define CPU_TIMING(_level, _colour, _name)
#endif

#endif // DISPLAY_CPU_TIMING_TIMING_H
