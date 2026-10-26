#ifndef _PERK_BARGE_C_H
#define _PERK_BARGE_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <perks/perkbarge/iperkbarge.h>
#include <GameLogic\Layers\RaceLogicLayer.h>
#include <fuzzylib/fuzzylibtwo/FuzzyPolicy.h>

namespace Game
{
	class C_LevelState;
	class C_PickupBase;

	namespace Actors
	{
		class C_VehicleLogicActor;
	}
}

namespace airacingtech
{


	class IData;
	class IGameServerTime;
	class IAvoidance;
	class IPerkSkillUsage;



	BEGIN_COMPONENT_DEC(CPerkBarge, IPerkBarge)

		void	Evaluate();
		float	GetVehicleEvaluation(int idx) { return (idx >= 0 && idx < m_VehicleEvaluationValues.Size() ? m_VehicleEvaluationValues[idx] : 2.f); };
		float	GetProjectileEvaluation(int idx) { return (idx >= 0 && idx < m_ProjectileEvaluationValues.Size() ? m_ProjectileEvaluationValues[idx] : 2.f); };
		int		GetNumProjectiles(){return m_ProjectileEvaluationValues.Size();}

	private:

		void CachedInterfaces();
		void Reset();

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



		fuzzy_lib_two::FuzzyPolicy		m_fzPolicyEvaluateSkill;



	END_COMPONENT_DEC



}




#endif //_PERK_NITRO_C_H
