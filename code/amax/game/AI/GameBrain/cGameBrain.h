#ifndef _PERK_GAMEBRAIN_C_H
#define _PERK_GAMEBRAIN_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <driverai/igamebrain.h>

namespace airacingtech
{
	class IData;
	class IPerkSkillUsage;
	class IPerkLogicAI;
	class IPerkSystem;

	class BrainState
	{
	public:

		enum BRAIN_STATE
		{
			BRAIN_STATE_AVOIDANCE = 0,
			BRAIN_STATE_GO_FOR_PERK,
			BRAIN_STATE_BLOCKING,
			BRAIN_STATE_WANDER,
			BRAIN_STATE_NONE
		};

		BrainState() : m_currentBrainState(BRAIN_STATE_AVOIDANCE) {};
		~BrainState(){};

		BRAIN_STATE		m_currentBrainState;

	};




	BEGIN_COMPONENT_DEC(CGameBrain, IGameBrain)

		void Update();
		void PreUpdate();
		void PostUpdate();


	private:

		void UpdateGame();
		void PreUpdateGame();

		void CalculateSteerState();
		void CalculateCommonVehicleData();
		void CachedInterfaces();

		IData				*m_pIData;
		BrainState			m_BrainState;
		IPerkSkillUsage		*m_pIPerkSkillUsage;
		IPerkLogicAI		*m_pIPerkLogic;
		IPerkSystem			*m_pIPerkSystem;


	END_COMPONENT_DEC
}




#endif //_PERK_GAMEBRAIN_C_H
