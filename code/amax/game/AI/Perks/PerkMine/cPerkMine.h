#ifndef _PERK_MINE_C_H
#define _PERK_MINE_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <perks/perkmine/iperkmine.h>
#include <perks/PerkAndSkillUsage/cPerkSkillUsage.h>

namespace Game
{
	class C_RaceLogicLayer;

	namespace Actors
	{
		class C_VehicleLogicActor;
		class C_VehicleActor;
	}
};

namespace airacingtech
{

	class IData;
	class IGameServerTime;
	class IAvoidance;
	class ISplineTracker;
	class ISplineFollower;
	class IPerkSkillUsage;


	BEGIN_COMPONENT_DEC(CPerkMine, IPerkMine)

		void				Evaluate();
		
		float				GetDropEvaluation() { return m_fDropEvaluation; };
		TargetEvaluation*	GetVehicleEvaluation(int idx) { return (idx >= 0 && idx < m_VehicleEvaluationValues.Size() ? &m_VehicleEvaluationValues[idx] : NULL); };
		TargetEvaluation*	GetProjectileEvaluation(int idx) { return (idx >= 0 && idx < m_ProjectileEvaluationValues.Size() ? &m_ProjectileEvaluationValues[idx] : NULL); };
		int					GetNumProjectiles(){return m_ProjectileEvaluationValues.Size();}

	private:

		void CachedInterfaces();
		void Reset();

		void EvaluateDrop();
		void EvaluateOpponents(float speed);
		void EvaluateProjectiles(float speed);

		float GetMineSpeed();

		IData				*m_pIData;
		IGameServerTime		*m_pIGameServerTime;
		IAvoidance			*m_pIAvoidance;
		ISplineTracker		*m_pSplineTracker;
		ISplineFollower		*m_pSplineFollower;
		IPerkSkillUsage		*m_pIPerkSkillUsage;

		Game::C_RaceLogicLayer*				m_pRaceLogicLayer;
		Game::Actors::C_VehicleLogicActor*	m_pLogicActor;
		Game::Actors::C_VehicleActor*		m_pVehActor;

		float								m_TimeStamp;
		float																						m_fDropEvaluation;
		Btl::Vector< TargetEvaluation, Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES> >	m_VehicleEvaluationValues;
		Btl::Vector< TargetEvaluation >																m_ProjectileEvaluationValues;

	END_COMPONENT_DEC
}




#endif //_PERK_MINE_C_H
