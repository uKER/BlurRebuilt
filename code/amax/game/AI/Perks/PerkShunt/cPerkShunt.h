#ifndef _PERK_SHUNT_C_H
#define _PERK_SHUNT_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <perks/perkshunt/iperkshunt.h>
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
	class IPerkSkillUsage;
	class ISystemAI;


	BEGIN_COMPONENT_DEC(CPerkShunt, IPerkShunt)


	void				Evaluate();
	TargetEvaluation*	GetVehicleEvaluation(int idx) { return (idx >= 0 && idx < m_VehicleEvaluationValues.Size() ? &m_VehicleEvaluationValues[idx] : NULL); };
	float				GetProjectileEvaluation(int idx) { return (idx >= 0 && idx < m_ProjectileEvaluationValues.Size() ? m_ProjectileEvaluationValues[idx] : 0.f); };
	int					GetNumProjectiles(){return m_ProjectileEvaluationValues.Size();}

	private:

		void	CachedInterfaces();
		void	Reset();
		
		void	EvaluateVehiclesAhead();
		void	EvaluateVehiclesBehind();
		void	EvaluateProjectiles();

		IData				*m_pIData;
		IGameServerTime		*m_pIGameServerTime;
		IAvoidance			*m_pIAvoidance;
		IPerkSkillUsage		*m_pIPerkSkillUsage;
		ISystemAI			*m_pISystem;

		Game::C_RaceLogicLayer*				m_pRaceLogicLayer;
		Game::Actors::C_VehicleLogicActor*	m_pLogicActor;
		Game::Actors::C_VehicleActor*		m_pVehActor;

		Btl::Vector< TargetEvaluation, Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES> >	m_VehicleEvaluationValues;
		Btl::Vector< float >																		m_ProjectileEvaluationValues;

	END_COMPONENT_DEC
}




#endif //_PERK_SHUNT_C_H
