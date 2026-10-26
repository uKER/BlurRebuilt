#ifndef RACE_RULE_DRIVER_LAST_IN_RACE_H
#define RACE_RULE_DRIVER_LAST_IN_RACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_DriverLastInRace : public RaceModeRuleBase
	{
	public:
		RaceModeRule_DriverLastInRace() : mDriverID(0) {};

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_DriverLastInRace; };

		void SetDriverRule(XtId id) { mDriverID = id; }

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId	mDriverID;
	};

} // namespace Game

#endif // RACE_RULE_DRIVER_LAST_IN_RACE_H