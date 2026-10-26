#ifndef RACE_RULE_HIT_X_MPH
#define RACE_RULE_HIT_X_MPH

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_DriverHitXMph : public RaceModeRuleBase
	{
	public:
		RaceModeRule_DriverHitXMph() : mDriverID(0), mAreaRef(0), mSpeedNeeded(0) {};

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_HitMph; };
		
		void SetSpeedRule(int s)					{ mSpeedNeeded = s; };
		void SetDriverRule(XtId id)					{ mDriverID = id; }
		void SetQuantifier(RaceBookQuantifier q)	{ mQuantifier = q; };
		void SetAreaRefRule(S32 areaRef)			{ mAreaRef = areaRef; };
		S32  GetAreaRefRule()						{ return mAreaRef; }

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId				mDriverID;
		int					mSpeedNeeded;
		int					mAreaRef;
		RaceBookQuantifier	mQuantifier;
	};

} // namespace Game

#endif // RACE_RULE_HIT_X_MPH
