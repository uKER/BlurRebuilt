#ifndef _PERK_REPAIR_C_H
#define _PERK_REPAIR_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <perks/perkrepair/iperkrepair.h>

namespace Game
{
	class C_RaceLogicLayer;

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

	BEGIN_COMPONENT_DEC(CPerkRepair, IPerkRepair)


		void	Evaluate();
		float	GetEvaluationValue() { return m_LastEvaluationValue; };

	private:

		void CachedInterfaces();
		
		IData				*m_pIData;
		IGameServerTime		*m_pIGameServerTime;
		IAvoidance			*m_pIAvoidance;

		Game::C_RaceLogicLayer*				m_pRaceLogicLayer;
		Game::Actors::C_VehicleLogicActor*	m_pLogicActor;

		float	m_LastEvaluationValue;

	END_COMPONENT_DEC
}




#endif //_PERK_REPAIR_C_H
