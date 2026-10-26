#ifndef MP_RACE_ACTION_BASE_H
#define MP_RACE_ACTION_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

class C_MpRaceActionBase
{
public:
	
	C_MpRaceActionBase(F32 p_Period): m_TimePeriod(p_Period), m_CurrentTimer(m_TimePeriod) {};
	//	Reduce m_CurrentTimer, perform the race action if timer expires
	void										UpdateTimer(F32 p_Timestep)
	{
		m_CurrentTimer -= p_Timestep;
		if(m_CurrentTimer <= 0.0f)
		{
			PerformAction();
			m_CurrentTimer = m_TimePeriod;
		}
	}

	virtual void								PerformAction() = 0;

protected:
	F32											m_TimePeriod;
	F32											m_CurrentTimer;
};


}	//	namespace Game

#endif // MP_RACE_ACTION_BASE_H