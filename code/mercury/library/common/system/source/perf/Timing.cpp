#include "SystemPch.h"
#include "System/Perf/Timing.h"

namespace Perf
{

C_TimingStats::C_TimingStats(const S32 level, const Maths::RGBA colour, const Util::C_String80& name) :
	m_Enabled(false),
	m_Depth(-1),
	m_Level(level),
	m_Colour(colour),
	m_Name(name)
{
}

C_TimingStats::~C_TimingStats()
{
}

void C_TimingStats::Update()
{
	m_Enabled = false;

	// update total
	m_Frames[E_TimingFrame_Total].m_CallCount += m_Frames[E_TimingFrame_Curr].m_CallCount;
	m_Frames[E_TimingFrame_Total].m_TotalTime += m_Frames[E_TimingFrame_Curr].m_TotalTime;

	// update peak
	if(m_Frames[E_TimingFrame_Curr].m_CallCount > m_Frames[E_TimingFrame_Peak].m_CallCount)
		m_Frames[E_TimingFrame_Peak].m_CallCount = m_Frames[E_TimingFrame_Curr].m_CallCount;
	if(m_Frames[E_TimingFrame_Curr].m_TotalTime > m_Frames[E_TimingFrame_Peak].m_TotalTime)
		m_Frames[E_TimingFrame_Peak].m_TotalTime = m_Frames[E_TimingFrame_Curr].m_TotalTime;

	// update prev
	m_Frames[E_TimingFrame_Prev] = m_Frames[E_TimingFrame_Curr];

	// reset curr
	m_Frames[E_TimingFrame_Curr].m_CallCount = 0;
	m_Frames[E_TimingFrame_Curr].m_TotalTime = 0;
}

void C_TimingStats::ResetPeak()
{
	m_Frames[E_TimingFrame_Peak].m_CallCount = 0;
	m_Frames[E_TimingFrame_Peak].m_TotalTime = 0;
}

void C_TimingStats::SetDepth(const S32 depth)
{
	if(m_Depth == -1)
	{
		m_Depth = depth;
	}
	else
	if(depth < m_Depth)
	{
		m_Depth = depth;
	}
}

} // namespace Perf
