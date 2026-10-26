#ifndef _RUBBERBANDINGAI_C_H
#define _RUBBERBANDINGAI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <rubberbanding/irubberbandingai.h>
#include <fuzzylib/fuzzylibtwo/FuzzyPolicy.h>

namespace airacingtech
{
	class IData;
	class IGameServerTime;
	class ISystemAI;
	class IRaceDirector;
	class AIGameplayBalancing;


	BEGIN_COMPONENT_DEC(CRubberbandingAI, IRubberbandingAI)

	public:

		void Update();
		inline float GetFzCatchup(){return m_fzCatchup;}
		inline float GetFzSlowDown(){return m_fzSlowdown;}
	
		inline float GetDirectTargetSpeed() const {return m_DirectTargetSpeed;}
		inline void SetDirectTargetSpeed(float v){m_DirectTargetSpeed = v;}

		inline AIEventBalancingData *GetEventBalancingData(){return m_pAIEventBalancingData;}

		inline AIEventSegmentData *GetEventBalancingSegmentData(){return m_pAIEventBalancingSegmentData;}

		inline void	SetDriverIndexOverride(int index)	{ m_driverIndexOverride = index; }

	private:

		void CachedInterfaces();
		void CatchupRules();
		void SlowdownRules();
		void GetNearestBehindInfront(int &infront, int &behind, bool &infrontOfPlayer);
		
		IData				*m_pIData;
		IGameServerTime		*m_pIGameServerTime;
		ISystemAI			*m_pISystem;
		IRaceDirector		*m_pIRaceDirector;
		AIGameplayBalancing *m_pGameplayBalancing;
		AIEventBalancing	*m_pAIEventBalancing;

		AIEventBalancingData *m_pAIEventBalancingData;
		AIEventSegmentData *m_pAIEventBalancingSegmentData;

		float			m_lastTime;
		float			m_fzCatchup;
		float			m_fzSlowdown;

		class RangeData
		{
		public:
			RangeData() : m_min(10.0f), m_max(40.0f), m_current(20.0f), m_changeTime(5.0f)  {};
			~RangeData(){};

			float m_min;
			float m_max;
			float m_current;
			float m_changeTime;
		};

		RangeData	m_InfrontData;
		RangeData	m_BehindData;


		RubberBandingInitialiseParams	*m_pParameters;


		float							m_DirectTargetSpeed;
		
		Btl::Vector<IData *>			m_AIlist;
		int								m_driverIndexOverride;
	};
}


#endif //_RUBBERBANDINGAI_C_H