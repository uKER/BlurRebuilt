#ifndef RACE_RULE_PERFORM_X_TRICK_SHOTS_H
#define RACE_RULE_PERFORM_X_TRICK_SHOTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_DriverPerformedXTrickShots : public RaceModeRuleBase
	{
	public:
		RaceModeRule_DriverPerformedXTrickShots() : mDriverID(0), mTrickShotsNeeded(0) {};

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_PerformXTrickShots; };

		void SetNumTrickShots(int n)	{ mTrickShotsNeeded = n; };
		void SetDriverRule(XtId id)		{ mDriverID = id; }

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);

	private:
		XtId	mDriverID;
		int		mTrickShotsNeeded;
	};

} // namespace Game

#endif // RACE_RULE_PERFORM_X_TRICK_SHOTS_H