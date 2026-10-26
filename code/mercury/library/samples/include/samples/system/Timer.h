#ifndef SAMPLES_TIMER_H
#define SAMPLES_TIMER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Samples
{

class C_Timer
{
public:

	C_Timer();
	~C_Timer();

	void Reset();

	void	TogglePaused() { m_Paused = !m_Paused; }
	F32		GetFrameTime() const { return m_FrameTime; }
	F32		GetTime() const { return m_ElapsedTime; }
	F32		GetFrameRate() const { return m_FrameRate; }
	bool	IsPaused() const { return m_Paused; }
	F32		GetElapsedTime() const { return m_ElapsedTime; }
	
	void	Update();

private:
	F32		m_FrameCount;
	F32		m_FrameTime;
	bool	m_Paused;
	F32		m_Cycle;
	F32		m_FrameRate;
	F32		m_ElapsedTime;
};

}; // namespace Samples

#endif // SAMPLES_TIMER_H
