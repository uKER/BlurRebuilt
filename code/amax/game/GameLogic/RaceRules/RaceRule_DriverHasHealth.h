#ifndef RACE_RULE_TAKE_OUT_DRIVER_H
#define RACE_RULE_TAKE_OUT_DRIVER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_DriverHasHealth : public RaceModeRuleBase
	{
	public:
		RaceModeRule_DriverHasHealth() : mDriverID(0), mHealth(0) {};

		bool IsTriggered(int vehicleIndex);
		bool TrackVehicle(XtId driverId) { return (driverId == mDriverID); };
		RaceModeRuleType GetType() { return E_RaceRule_DriverHasHealth; };

		void SetDriverRule(XtId id)					{ mDriverID = id; };
		void SetDriverLocId(XtLocId id)				{ mDriverLocId = id; };
		void SetHealthRule(float h)					{ mHealth = h; };

		void SetQuantifier(RaceBookQuantifier q)	{ mQuantifier = q; };
		bool CheckQuantifier(int vehIdx);

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);
		void CacheStrings();


		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId				mDriverID;
		XtLocId				mDriverLocId;
		float				mHealth;
		RaceBookQuantifier	mQuantifier;

		float				mDriverToBeatHealth;
		UI::C_WideString32	mDriverLocString;
		UI::C_WideString32	mQuantifierString;
	};

} // namespace Game

#endif // RACE_RULE_TAKE_OUT_DRIVER_H