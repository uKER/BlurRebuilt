#ifndef DISPLAY_TIMING_TIMING_H
#define DISPLAY_TIMING_TIMING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/Time/Time.h>
#include <Core/Maths.h>

namespace Perf
{

//! E_TimingFrame

enum E_TimingFrame
{
	E_TimingFrame_Total = 0,
	E_TimingFrame_Peak,
	E_TimingFrame_Prev,
	E_TimingFrame_Curr,
	E_TimingFrame_Count,
};

//! C_TimingStats

class C_TimingStats
{
public:

	C_TimingStats(const S32 level, const Maths::RGBA colour, const Util::C_String80& name);
	~C_TimingStats();

	const S32 GetCallCount(const E_TimingFrame frame) const;
	const Time::C_Time GetTotalTime(const E_TimingFrame frame) const;

	void AddCallCount(const E_TimingFrame frame, const S32 callCount);
	void AddTotalTime(const E_TimingFrame frame, const Time::C_Time totalTime);

	void Update();

	void ResetPeak();

	void SetDepth(const S32 depth);

public:

	bool									m_Enabled;
	S32										m_Depth;
	S32										m_Level;
	Maths::RGBA								m_Colour;
	const Util::C_String80					m_Name;

protected:
	
	struct C_Frame
	{
		S32									m_CallCount;
		Time::C_Time						m_TotalTime;
	};

	C_Frame									m_Frames[E_TimingFrame_Count];
};

} // namespace Perf

#include "System/Perf/Timing.inl"

#endif // DISPLAY_TIMING_TIMING_H
