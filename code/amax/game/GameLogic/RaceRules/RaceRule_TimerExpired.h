#ifndef RACE_RULE_TIMER_EXPIRED_H
#define RACE_RULE_TIMER_EXPIRED_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>
#include <GameLogic\Layers\RaceLogicLayer.h>

namespace Game
{

	class RaceModeRule_TimerExpired : public RaceModeRuleBase
	{
	public:
		RaceModeRule_TimerExpired();

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_TimerExpired; };

		void SetTimer(float t)
		{
			for( int i = 0; i < MAX_RACELOGIC_VEHICLES; i++ )
				mTimer[i] = t;
		}

		float GetTimer(U32 vehicleIndex) { return mTimer[vehicleIndex]; }

		void AddTimeAwarded(float t) { mTimerAdditions.PushBack(t); };
		void SetNumTriggerZones(int n) { mNumTriggerZones = n; };

		void GetRuleString(U32 vehicleIndex, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);

	private:
		Btl::Vector<float, Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES> >	mTimer;
		Btl::Vector<float, Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES> >	mLastTime;
		Btl::Vector<float>															mTimerAdditions;
		Btl::Vector<int, Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES> >		mLastRaceRuleArea;
		Btl::Vector<int, Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES> >		mZonesCrossed;

		int mNumTriggerZones;
		int mCurrentZoneIdx;
		float mStartTimer;
	};

} // namespace Game

#endif // RACE_RULE_TIMER_EXPIRED_H