#ifndef DISPLAY_GPU_TIMING_TIMING_INL
#define DISPLAY_GPU_TIMING_TIMING_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

// C_GPUTimingStats

#ifdef PLATFORM_PS3

INLINE void C_GPUTimingStats::AdvanceTimingRead()
{
	m_TimingRead++;
	if(m_TimingRead >= m_TimingCount)
		m_TimingRead = 0;
	assert(m_TimingRead != m_TimingWrite);
}

INLINE void C_GPUTimingStats::AdvanceTimingWrite()
{
	m_TimingWrite++;
	if(m_TimingWrite >= m_TimingCount)
		m_TimingWrite = 0;
	assert(m_TimingRead != m_TimingWrite);
}

#endif // PLATFORM_PS3

} // namespace Display

#endif // DISPLAY_GPU_TIMING_TIMING_INL
