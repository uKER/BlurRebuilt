#ifndef RACE_RULE_CAUSE_X_DAMAGE_TO_DRIVER_H
#define RACE_RULE_CAUSE_X_DAMAGE_TO_DRIVER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\Layers\RaceLogicLayer.h>
#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_CauseXDamageToDriver : public RaceModeRuleBase
	{
	public:
		RaceModeRule_CauseXDamageToDriver() : mDriverID(0)
		{
			for( int i = 0; i < mDriverToBeatDamage.Size(); i++ )
				mDriverToBeatDamage[i] = 0;
		}

		bool IsTriggered(int vehicleIndex);
		bool TrackVehicle(XtId driverId) { return (driverId == mDriverID); };
		RaceModeRuleType GetType() { return E_RaceRule_CauseDamageToDriver; };

		void SetDriverRule(XtId id)					{ mDriverID = id; };
		void SetDriverLocId(XtLocId id)				{ mDriverLocId = id; };
		void SetDamageRule(float d)					{ mDamage = d; };
		void SetQuantifier(RaceBookQuantifier q)	{ mQuantifier = q; };

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);
		void CacheStrings();

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId				mDriverID;
		XtLocId				mDriverLocId;
		float				mDamage;
		RaceBookQuantifier	mQuantifier;

		Btl::Vector< float, Btl::VectorParameters_Array< Game::MAX_RACELOGIC_VEHICLES > >	mDriverToBeatDamage;
		UI::C_WideString32																	mDriverLocString;
		UI::C_WideString32																	mQuantifierString;
	};

} // namespace Game

#endif // RACE_RULE_CAUSE_X_DAMAGE_TO_DRIVER_H