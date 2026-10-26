#ifndef DISPLAY_GPU_TIMING_TIMING_H
#define DISPLAY_GPU_TIMING_TIMING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/Perf/Timing.h>
#include <Core/Containers/Genesis_Containers/List.h>
#include <Core/Threads/ThreadLocalStorage.h>
#include <Display2/Utils/Utils.h>


#ifdef PLATFORM_PC
struct IDirect3DQuery9;	// [Blur2PC] PC GPU timing queries
#endif

namespace Display
{

//! C_GPUTimingStats

class C_GPUTimingStats : public Perf::C_TimingStats, public Util::C_Item<C_GPUTimingStats>
{
public:

	C_GPUTimingStats(const S32 level, const Maths::RGBA colour, const Util::C_String80& name);
	~C_GPUTimingStats();

	const bool Show() const;

	static void UpdateStats();
	static void ResetPeaks();
	static void BeginFrame();
	static void EndFrame();

	#ifdef PLATFORM_PC
	static void ReleaseQueries();	// [Blur2PC] before the D3D9 device is destroyed
	#endif

	#ifdef PLATFORM_PS3
	void AdvanceTimingRead();
	void AdvanceTimingWrite();
	#endif
	
	static const C_GPUTimingStats *FindStats(const char *psz);

public:

	#ifdef PLATFORM_PS3
	U32											m_TimingRead;
	U32											m_TimingWrite;
	static const U32							m_TimingCount = 4;
	U32											m_TimingIndex[m_TimingCount];
	#endif
	

	#ifdef PLATFORM_PC
	// [Blur2PC] PC backend (the snapshot has none): D3D9 timestamp query pairs per event, read back
	// m_FrameCount frames later like the PS3 path; m_DisplayTime is in nanoseconds (the unit MiniPerfMonitor
	// assumes off Xenon)
	static const U32							m_FrameCount = 4;
	static const U32							m_PairCount = 8;
	ID3D11Query*							m_pQueries[m_FrameCount][m_PairCount][2]; // [Blur1PCDX11] Native timestamp queries.
	U32											m_PairsUsed[m_FrameCount];
	U64											m_DisplayTime;
	#endif

	static Util::C_List<C_GPUTimingStats>		m_StatsList;
};

//! C_GPUTimingEvent

class C_GPUTimingEvent
{
public:

	C_GPUTimingEvent(C_GPUTimingStats& stats);
	~C_GPUTimingEvent();

public:

	S32											m_Depth;
	
	#ifdef PLATFORM_PS3
	U32											m_Index;
	#endif

	#ifdef PLATFORM_PC
	S32											m_Pair;	// [Blur2PC] query pair issued by this event, -1 if none
	#endif

private:

	C_GPUTimingEvent* __restrict				m_pParent;
	C_GPUTimingStats* __restrict				m_pStats;

	static MT::ThreadLocalStorage				m_Tls;
};

} // namespace Display

#include "Display/Timing/GPUTiming.inl"

#if !defined(CFG_GOLD)
#define USE_GPU_TIMINGS
#endif

#ifdef USE_GPU_TIMINGS

#if USE_GPU_TIMINGS2

	#define GPU_TIMING_BEGIN_FRAME() \
		Display2::C_TimingManager::GetInstance().BeginFrame()

	#define GPU_TIMING_END_FRAME() \
		Display2::C_TimingManager::GetInstance().EndFrame()

	#define GPU_TIMING_UPDATE() \
		Display2::C_TimingManager::GetInstance().Update()

	#define GPU_TIMING(_level, _colour, _name) \
		static Display2::C_TimingString gpuTimingString2(_name, _level); \
		Display2::C_TimingEvent gpuTimingEvent2(&gpuTimingString2)

#else

	#define GPU_TIMING_BEGIN_FRAME() \
		Display::C_GPUTimingStats::BeginFrame()

	#define GPU_TIMING_END_FRAME() \
		Display::C_GPUTimingStats::EndFrame()

	#define GPU_TIMING_UPDATE() \
		Display::C_GPUTimingStats::UpdateStats()

	#define GPU_TIMING(_level, _colour, _name) \
		static Display::C_GPUTimingStats gpuTimingStats(_level, _colour, _name); \
		Display::C_GPUTimingEvent gpuTimingEvent(gpuTimingStats)

#endif

#else

	#define GPU_TIMING_BEGIN_FRAME()
	#define GPU_TIMING_END_FRAME()
	#define GPU_TIMING_UPDATE()
	#define GPU_TIMING(_level, _colour, _name)

#endif

#endif // DISPLAY_GPU_TIMING_TIMING_H
