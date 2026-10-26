#ifndef RACE_RULE_FINISH_IN_X_POS_H
#define RACE_RULE_FINISH_IN_X_POS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

class RaceModeRule_DriverFinishInXPosition : public RaceModeRuleBase
{
public:
	RaceModeRule_DriverFinishInXPosition() : mDriverID(0), mPositionNeeded(1) {};

	bool IsValid();
	bool IsTriggered(int vehicleIndex);
	bool TrackVehicle(XtId driverId) { return (mDriverID != 0 && driverId == mDriverID); }

	RaceModeRuleType GetType()		{ return E_RaceRule_FinishInPosition; };

	void SetPositionRule(int r)			{ mPositionNeeded = r; };
	void SetDriverRule(XtId id)			{ mDriverID = id; }
	void SetDriverToBeatLoc(XtLocId id) { mDriverToBeatLocId = id; };

	int  GetPositionRule()				{ return mPositionNeeded; }

	void CacheStrings();
	void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);

	void Serialise( Sys::BinarySerialiserAutoEndian& stream );
	void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

private:
	XtId				mDriverID;
	XtLocId				mDriverToBeatLocId;
	UI::C_WideString16	mDriverToBeatLocString;
	int					mPositionNeeded;
};

} // namespace Game

#endif // RACE_RULE_FINISH_IN_X_POS_H
