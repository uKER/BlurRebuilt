#ifndef RACE_RULE_COLLECT_X_FAN_PICKUPS_H
#define RACE_RULE_COLLECT_X_FAN_PICKUPS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_CollectXFanPickups : public RaceModeRuleBase
	{
	public:
		RaceModeRule_CollectXFanPickups() : mDriverID(0), mNumPickups(0) {};

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_CollectFanPickups; };

		void SetDriverRule(XtId id)					{ mDriverID = id; };
		void SetNumPickupsRule(int n)				{ mNumPickups = n; };

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId				mDriverID;
		int					mNumPickups;
	};

} // namespace Game

#endif // RACE_RULE_COLLECT_X_FAN_PICKUPS_H