#ifndef RACE_RULE_PERFORM_ACTION_Y_TIMES_H
#define RACE_RULE_PERFORM_ACTION_Y_TIMES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_PerformActionYTimes : public RaceModeRuleBase
	{
	public:
		RaceModeRule_PerformActionYTimes() : mDriverID(0) {};

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_PerformActionYTimes; };

		void SetDriverRule(XtId id)			{ mDriverID = id; };
		void SetNumTimes(int n)				{ mNumTimes = n; };
		void SetAction(E_STUNT_ACTION a)	{ mAction = a; };

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);
		void CacheStrings();

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId				mDriverID;
		int					mNumTimes;
		E_STUNT_ACTION		mAction;

		UI::C_TextStringID	mActionLocId;
	};

} // namespace Game

#endif // RACE_RULE_PERFORM_ACTION_Y_TIMES_H