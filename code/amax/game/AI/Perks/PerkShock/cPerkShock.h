#ifndef _PERK_SHOCK_C_H
#define _PERK_SHOCK_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <perks/perkshock/iperkshock.h>

namespace airacingtech
{

	class IData;
	class IGameServerTime;
	class IAvoidance;
	class IPerkSkillUsage;
	class ISystemAI;


	BEGIN_COMPONENT_DEC(CPerkShock, IPerkShock)


		void	Evaluate();
		float	GetEvaluationValue() { return m_LastEvaluationValue; };

	private:

		void	CachedInterfaces();

		IData				*m_pIData;
		IGameServerTime		*m_pIGameServerTime;
		IAvoidance			*m_pIAvoidance;
		IPerkSkillUsage		*m_pIPerkSkillUsage;
		ISystemAI			*m_pISystem;

		Game::C_RaceLogicLayer*				m_pRaceLogicLayer;
		Game::Actors::C_VehicleLogicActor*	m_pLogicActor;

		float	m_LastEvaluationValue;

	END_COMPONENT_DEC
}




#endif //_PERK_SHOCK_C_H
