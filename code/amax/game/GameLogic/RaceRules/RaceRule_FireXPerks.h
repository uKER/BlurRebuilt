#ifndef RACE_RULE_FIRE_X_PERKS
#define RACE_RULE_FIRE_X_PERKS

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_FireXPerks : public RaceModeRuleBase
	{
	public:
		RaceModeRule_FireXPerks() : mNumFired(0) {};

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_FireXPerks; };

		void SetNumFiredRule(int n) { mNumFired = n; };
		void AddPickupType(E_PICKUP_TYPE t) { mPickupTypes.PushBack(t); }

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);

	private:
		int																					mNumFired;
		Btl::Vector<E_PICKUP_TYPE>															mPickupTypes;
	};

} // namespace Game

#endif // RACE_RULE_FIRE_X_PERKS