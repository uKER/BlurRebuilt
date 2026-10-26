#ifndef RACE_RULE_EARN_XP_H
#define RACE_RULE_EARN_XP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>
#include <GameLogic\Layers\RaceLogicLayer.h>

namespace Game
{

	class RaceModeRule_DriverEarnXP : public RaceModeRuleBase
	{
	public:
		RaceModeRule_DriverEarnXP();

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_EarnXP; }

		void SetXpRule(int xp)		{ mXpNeeded = xp; }
		void SetDriverRule(XtId id) { mDriverID = id; }

		int  GetXpRule()			{ return mXpNeeded; }

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId	mDriverID;
		int		mXpNeeded;
		F32		mPulseWaitTime;
		Btl::Vector<int, Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES > >	mCurrentXp;
	};

} // namespace Game

#endif // RACE_RULE_EARN_XP_H
