#ifndef SCORINGRULES_H
#define SCORINGRULES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "..\RaceLogicCore\Data\RLEnums.h"
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

// This lot is now not used, as all rules are pre-defined in sorcery.
// This makes it easier for the network, says Will, although we've lost some of our flexibility
// and it's a little slower game-side.

//namespace Game
//{
//	class ScoringRuleBase
//	{
//	public:
//		ScoringRuleBase()				: mPointsAwarded(1.0f) {};
//		ScoringRuleBase(float points)	: mPointsAwarded(points) {};
//
//		virtual void Update() {};
//		virtual float ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex) { return 0; };
//
//		float mPointsAwarded;
//	};
//
//	class ScoringRule_HitByPerk : public ScoringRuleBase
//	{
//	public:
//		virtual float ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex);
//
//		Btl::Vector<E_PICKUP_TYPE> mPickupTypes;
//	};
//
//	class ScoringRule_CausedDamage : public ScoringRuleBase
//	{
//	public:
//		virtual float ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex);
//
//		E_IMPACT_LEVEL mImpactLevel;
//	};
//}

#endif //SCORINGRULES_H
