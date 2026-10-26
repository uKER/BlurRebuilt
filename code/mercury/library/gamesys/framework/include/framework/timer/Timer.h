#ifndef FRAMEWORK_TIMER_H
#define FRAMEWORK_TIMER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Framework
{

//! C_Timer

class C_Timer
{
public:

	C_Timer();

	~C_Timer();

	bool IsPaused() const
	{
		return m_Paused;
	}

	void TogglePaused()
	{
		m_Paused = !m_Paused;
	}

	const F32 GetFrameTime() const
	{
		return m_FrameTime * m_TimerScale;
	}

	const F32 GetFrameRate() const
	{
		return m_FrameRate;
	}
	
	const F32 GetElapsedTime() const
	{
		return m_ElapsedTime;
	}

	void Update();

	void ForceFrameTime(F32 tme)
	{
		m_ForceFrameTime = tme;
	}

	void SetTimerScale(F32 scale);

private:

	bool	m_Paused;
	F32		m_frameAveragingCount;
	F32		m_FrameCycle;
	F32		m_FrameTime;
	F32		m_FrameRate;
	F32		m_ElapsedTime;
	Time::C_Time m_PrevTime;
	F32		m_ForceFrameTime;
	F32		m_TimerScale;
};

}; // namespace Framework

#endif // FRAMEWORK_TIMER_H
