#ifndef RACE_RULE_GET_LAP_TIME_OF_X_H
#define RACE_RULE_GET_LAP_TIME_OF_X_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_DriverGetLapTimeOfX : public RaceModeRuleBase
	{
	public:
		RaceModeRule_DriverGetLapTimeOfX() : mDriverID(0), mLapTime(0) {};

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_GetLapTime; };

		void SetLapTimeRule(float t)  { mLapTime = t; };
		void SetDriverRule(XtId id) { mDriverID = id; }

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId	mDriverID;
		float	mLapTime;
	};

} // namespace Game

#endif // RACE_RULE_GET_LAP_TIME_OF_X_H
