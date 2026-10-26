#ifndef DISPLAY_TIMING_TIMING_INL
#define DISPLAY_TIMING_TIMING_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Perf
{

INLINE const S32 C_TimingStats::GetCallCount(const E_TimingFrame frame) const
{
	return m_Frames[frame].m_CallCount;
}

INLINE const Time::C_Time C_TimingStats::GetTotalTime(const E_TimingFrame frame) const
{
	return m_Frames[frame].m_TotalTime;
}

INLINE void C_TimingStats::AddCallCount(const E_TimingFrame frame, const S32 callCount)
{
	m_Frames[frame].m_CallCount += callCount;
}

INLINE void C_TimingStats::AddTotalTime(const E_TimingFrame frame, const Time::C_Time totalTime)
{
	m_Frames[frame].m_TotalTime += totalTime;
}

} // namespace Perf

#endif // DISPLAY_TIMING_TIMING_INL
