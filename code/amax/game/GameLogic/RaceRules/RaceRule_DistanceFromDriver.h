#ifndef RACE_RULE_DISTANCE_FROM_DRIVER_H
#define RACE_RULE_DISTANCE_FROM_DRIVER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>
#include <GameLogic\Layers\RaceLogicLayer.h>

namespace Game
{

	class RaceModeRule_DriverDistanceFromDriver : public RaceModeRuleBase
	{
	public:
		RaceModeRule_DriverDistanceFromDriver() : mDriverID(0), mTime(0), mDistance(0), mDriverDistanceID(0)
		{
			for( int i = 0; i < Game::MAX_RACELOGIC_VEHICLES; i++ )
				mCurrentTimer[i] = 0.0f;
		};

		bool IsTriggered(int vehicleIndex);
		bool TrackVehicle(XtId driverId) { return (driverId == mDriverDistanceID); };

		RaceModeRuleType GetType() { return E_RaceRule_DistanceFromDriver; };
	
		void SetDistanceRule(float d)				{ mDistance = d; };
		void SetDriverRule(XtId id)					{ mDriverID = id; };
		void SetTimeRule(float t)					{ mTime = t; };
		void SetDriverDistanceRule(XtId id)			{ mDriverDistanceID = id; };
		void SetDriverDistanceLocId(XtLocId id)		{ mDriverDistanceLocID = id; };
		void SetQuantifier(RaceBookQuantifier q)	{ mQuantifier = q; };

		bool IsFailing(int vehicleIndex)			{ return (mCurrentTimer[vehicleIndex] != 0.0f); }
		float GetCountDownTimer(int vehicleIndex)	{ return (mTime - mCurrentTimer[vehicleIndex]); }

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);
		void CacheStrings();

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	private:
		XtId																			mDriverID;
		XtId																			mDriverDistanceID;
		XtLocId																			mDriverDistanceLocID;
		float																			mDistance;
		float																			mTime;
		Btl::Vector<float, Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES > >	mCurrentTimer;
		RaceBookQuantifier																mQuantifier;

		UI::C_WideString32																mQuantifierString;
		UI::C_WideString32																mDriverLocString;
	};

} // namespace Game

#endif // RACE_RULE_DISTANCE_FROM_DRIVER_H
