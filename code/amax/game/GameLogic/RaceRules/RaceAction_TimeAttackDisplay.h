#ifndef RACE_LAP_ACTION_TIME_ATTACK_DISPLAY_H
#define RACE_LAP_ACTION_TIME_ATTACK_DISPLAY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceAction_Base.h>

namespace Game
{

	class RaceAction_TimeAttackDisplay : public RaceActionBase
	{
	public:
		void PerformAction(int carIdx);
		RaceModeActionType GetType() { return E_Action_TimeAttackDisplay; };

		void Serialise( Sys::BinarySerialiserAutoEndian& stream )		{ RaceActionBase::Serialise(stream); };
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )	{ RaceActionBase::Deserialise(stream); };
	};

} // namespace Game

#endif // RACE_LAP_ACTION_TIME_ATTACK_DISPLAY_H