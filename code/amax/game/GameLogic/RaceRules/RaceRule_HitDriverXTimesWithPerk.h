#ifndef RACE_RULE_HIT_DRIVER_X_TIMES_WITH_PERK
#define RACE_RULE_HIT_DRIVER_X_TIMES_WITH_PERK

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_DriverHitDriverXTimesWithPerk : public RaceModeRuleBase
	{
	public:
		RaceModeRule_DriverHitDriverXTimesWithPerk();

		bool IsTriggered(int vehicleIndex);
		bool TrackVehicle(XtId driverId) { return (driverId == mDriverID); };
		RaceModeRuleType GetType() { return E_RaceRule_HitDriverWithPerk; };

		void SetNumHitsRule(int n) { mNumHits = n; };
		void SetDriverOverrideRule(XtId id) { mDriverOverrideID = id; }
		void SetDriverRule(XtId id) { mDriverID = id; }
		void SetDriverLoc(XtLocId id) { mDriverLocId = id; }
		void SetRequireDoubleRule(bool r) { mRequireDoubleHits = r; }
		void AddPickupType(E_PICKUP_TYPE t) { mPickupTypes.PushBack(t); }

		void CacheStrings();
		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId																				mDriverOverrideID;
		XtId																				mDriverID;
		XtLocId																				mDriverLocId;
		int																					mNumHits;
		Btl::Vector<E_PICKUP_TYPE>															mPickupTypes;
		Btl::Vector<int, Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES> >		mDriverHits;
		bool																				mRequireDoubleHits;
		bool																				mIsNitro;
		F32																					mPulseWaitTime;
		
		/*
		UI::C_WideString64																	mRuleStr;
		UI::C_WideString64																	mRuleStrNot;
		UI::C_WideString64																	mRuleStrCurrent;
		UI::C_WideString64																	mRuleStrTarget;
		*/

		UI::C_WideString32																	mDriverLocString;
		//UI::C_WideString32																	mPickupStr;
		//UI::C_WideString32																	mDoubleStr;

		UI::C_TextStringID																	mPickupId;
	};

} // namespace Game

#endif // RACE_RULE_HIT_DRIVER_X_TIMES_WITH_PERK
