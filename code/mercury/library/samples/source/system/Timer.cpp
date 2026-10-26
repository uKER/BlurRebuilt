#include "StdAfx.h"
#include "Samples/System/Timer.h"

namespace Samples
{

C_Timer::C_Timer()
{
	Reset();
}

C_Timer::~C_Timer()
{
}

void C_Timer::Reset()
{
	m_Paused		= false;
	m_FrameTime		= 0.f;
	m_FrameRate		= 0.f;
	m_ElapsedTime	= 0.f;
	m_Cycle			= 0.f;
	m_FrameCount	= 0.f;
}

void C_Timer::Update()
{
	static Time::C_Time oldTime = Time::GetPerformanceCounter();
	const Time::C_Time newTime = Time::GetPerformanceCounter();
	m_FrameTime = (F32)(newTime - oldTime) / (F32)SYS_USEC_PER_SEC;
	oldTime = newTime;
	
	// update frame rate 4 times per second
	m_Cycle += m_FrameTime;
	m_FrameCount += 1.f;
	if(m_Cycle > 0.25f)
	{
		m_FrameRate = m_FrameCount / m_Cycle;
		m_FrameCount = 0.f;
		m_Cycle = 0.f;
	}

	// clamp frame time when debugging etc
	if (m_FrameTime > 1.f/30.f)
	{
		m_FrameTime = 1.f/30.f;
	}

	// increment total elapsed time
	if(!m_Paused)
		m_ElapsedTime += m_FrameTime;
}

}; // namespace Samples
