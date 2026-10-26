#ifndef _PERK_SHIELD_C_H
#define _PERK_SHIELD_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <perks/perkshield/iperkshield.h>

namespace airacingtech
{
	class IData;
	class IGameServerTime;
	class IAvoidance;

	BEGIN_COMPONENT_DEC(CPerkShield, IPerkShield)

		void	Evaluate();
		float	GetVehicleEvaluation(int idx) { return (idx >= 0 && idx < m_VehicleEvaluationValues.Size() ? m_VehicleEvaluationValues[idx] : 2.f); };
		float	GetProjectileEvaluation(int idx) { return (idx >= 0 && idx < m_ProjectileEvaluationValues.Size() ? m_ProjectileEvaluationValues[idx] : 2.f); };
		int				GetNumProjectiles(){return m_ProjectileEvaluationValues.Size();}

	private:

		void CachedInterfaces();
		void Reset();

		void CheckPickups();
		void CheckProjectiles();

		IData				*m_pIData;
		IGameServerTime		*m_pIGameServerTime;
		IAvoidance			*m_pIAvoidance;
		IPerkSkillUsage		*m_pIPerkSkillUsage;

		Game::C_RaceLogicLayer*				m_pRaceLogicLayer;
		Game::Actors::C_VehicleLogicActor*	m_pLogicActor;
		Game::Actors::C_VehicleActor*		m_pVehActor;
		Game::C_LevelState*					m_pLevelState;

		Btl::Vector< float, Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES> >	m_VehicleEvaluationValues;
		Btl::Vector< float >															m_ProjectileEvaluationValues;

	END_COMPONENT_DEC
}




#endif //_PERK_SHIELD_C_H
