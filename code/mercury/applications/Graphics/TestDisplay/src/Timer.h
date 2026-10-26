#ifndef TIMER_H
#define TIMER_H

#ifdef _MSC_VER
#pragma once
#endif

struct C_Timer
{
	C_Timer();
	void Update();

	bool				m_Paused;
	F32					m_FrameTime;
	F32					m_TotalTime;
	F32					m_GameFrameTime;
	F32					m_GameTotalTime;
};

#endif // TIMER_H
