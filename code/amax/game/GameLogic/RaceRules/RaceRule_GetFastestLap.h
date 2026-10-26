#ifndef RACE_RULE_GET_FASTEST_LAP_H
#define RACE_RULE_GET_FASTEST_LAP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_DriverGetFastestLap : public RaceModeRuleBase
	{
	public:
		RaceModeRule_DriverGetFastestLap() : mDriverID(0) {};

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_GetFastestLap; };

		void SetDriverRule(XtId id) { mDriverID = id; }

		const UI::C_WideString512 GetRuleString(RACE_MODE_RULE_STRING_TYPE result, bool pass);

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId	mDriverID;
	};

} // namespace Game

#endif // RACE_RULE_GET_FASTEST_LAP_H
