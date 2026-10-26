#ifndef RACE_RULE_DRIFTING_IN_TRIGGER_REGION
#define RACE_RULE_DRIFTING_IN_TRIGGER_REGION

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_DriverDriftingInTriggerRegion : public RaceModeRuleBase
	{
	public:
		RaceModeRule_DriverDriftingInTriggerRegion() : mDriverID(0), mAreaRef(0) {};

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_InTriggerRegion; };

		void SetDriverRule(XtId id)			{ mDriverID = id; }
		void SetAreaRefRule(S32 areaRef)	{ mAreaRef = areaRef; };

		const UI::C_WideString512 GetRuleString(RACE_MODE_RULE_STRING_TYPE result, bool pass);

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId	mDriverID;
		S32		mAreaRef;
	};

} // namespace Game

#endif // RACE_RULE_DRIFTING_IN_TRIGGER_REGION