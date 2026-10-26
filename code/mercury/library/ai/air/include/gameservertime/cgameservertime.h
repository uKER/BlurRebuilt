#ifndef _GAMESERVERTIME_AI_C_H
#define _GAMESERVERTIME_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <gameservertime/igameservertime.h>

namespace airacingtech
{
	BEGIN_COMPONENT_DEC(CGameServerTime, IGameServerTime)

	public:

		inline float GetTime() const {return m_time;}
		inline void SetTime( float t ){m_time = t;}
		inline void SetTimeAccum(float t){m_time += t;}
		
		inline void IncrementTime()
		{
			m_time += m_increment;
		}
		
		inline void SetFrameRate(int rate){m_increment = 1.0f / rate;}

	private:

		float m_time;
		float m_increment;

	
	END_COMPONENT_DEC



}

#endif //_GAMESERVERTIME_AI_C_H
