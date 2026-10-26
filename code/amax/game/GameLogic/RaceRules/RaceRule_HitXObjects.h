#ifndef RACE_RULE_HIT_X_OBJECTS
#define RACE_RULE_HIT_X_OBJECTS

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_DriverHitXObjects : public RaceModeRuleBase
	{
	public:
		RaceModeRule_DriverHitXObjects() : mDriverID(0), mGroupID(0) {};

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_HitObjects; };

		void SetDriverRule(XtId id)					{ mDriverID = id; };
		void SetGroupID(int id)						{ mGroupID = id; };

		const UI::C_WideString512 GetRuleString(RACE_MODE_RULE_STRING_TYPE result, bool pass);

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId				mDriverID;
		int					mGroupID;
	};

} // namespace Game

#endif // RACE_RULE_HIT_X_OBJECTS