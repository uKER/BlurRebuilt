#ifndef RACE_RULE_BEAT_DRIVER_H
#define RACE_RULE_BEAT_DRIVER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>
#include <GameLogic\Layers\RaceLogicLayer.h>

namespace Game
{

	class RaceModeRule_BeatDriver : public RaceModeRuleBase
	{
	public:
		RaceModeRule_BeatDriver() : mDriverToBeatID(0), mDriverOverrideID(0) {};

		bool IsTriggered(int vehicleIndex);
		bool TrackVehicle(XtId driverId) { return (driverId == mDriverToBeatID); };
		RaceModeRuleType GetType() { return E_RaceRule_BeatDriver; };

		void SetDriverToBeatRule(XtId id)	{ mDriverToBeatID = id; };
		void SetDriverToBeatLoc(XtLocId id) { mDriverToBeatLocId = id; };
		void SetDriveroverrideRule(XtId id) { mDriverOverrideID = id; }

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);
		void CacheStrings();

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId				mDriverToBeatID;
		XtId				mDriverOverrideID;
		XtLocId				mDriverToBeatLocId;
		
		//Btl::Vector<float,Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES > > mDistanceFromDriver;
		int					mDriverToBeatPos;
		UI::C_WideString32	mDriverLocString;
	};

} // namespace Game

#endif // RACE_RULE_BEAT_DRIVER_H