#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

#include <GameLogic\RaceRules\RaceRule_TimerExpired.h>
#include <GameLogic\Layers\RaceBookLayer.h>

#include <ui/Game/UILocaliser.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <Game\GameActors\Components\GameComponents.h>

#include "Automation\GameDataLogging.h"

#include <Game\GameActors\Components\GameComponents.h>

extern Debug::Logger &g_RaceRulesManagerLogger;
Debug::Float g_debugRaceRuleTimerExpiredStartTime("RaceRules.TimerExpiresStartTime", 1.0f);

namespace Game
{
	RaceModeRule_TimerExpired::RaceModeRule_TimerExpired()
	{
		for( int i = 0; i < MAX_RACELOGIC_VEHICLES; i++ )
		{
			mLastRaceRuleArea[i] = -1;
			mTimer[i] = mLastTime[i] = 0.f;
			mZonesCrossed[i] = 0;
		}

		mCurrentZoneIdx = 0;
		mStartTimer = g_debugRaceRuleTimerExpiredStartTime;
	}

	bool RaceModeRule_TimerExpired::IsTriggered(int vehicleIndex)
	{
		float frameTime = C_GameApp::GetInstance()->GetTimeStep();
		Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);

		if(mStartTimer > 0)
		{
			mStartTimer -= frameTime;
			return false;
		}
		

		if(	pLogicActor->GetPositionInfo().mLastRaceRuleArea > mLastRaceRuleArea[vehicleIndex] || 
			( mLastRaceRuleArea[vehicleIndex] == mNumTriggerZones - 1 && pLogicActor->GetPositionInfo().mLastRaceRuleArea == 0 ) )
		{
			int prevZone	= mCurrentZoneIdx;
			float prevTime	= mTimer[vehicleIndex];

			mTimer[vehicleIndex] += mTimerAdditions[mCurrentZoneIdx++];
			
			if(mCurrentZoneIdx >= mTimerAdditions.Size())
				mCurrentZoneIdx = mTimerAdditions.Size() - 1;

			mLastRaceRuleArea[vehicleIndex] = pLogicActor->GetPositionInfo().mLastRaceRuleArea;

			UI::C_UiTimelineMessage msg( "timer_expired_bonus", true, 0.0f );
			UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg );

			float diff = mLastTime[vehicleIndex] - prevTime;
			if(diff < 0)
				diff = 0;

			//AmaxDataLogging::LogCheckpointHit(pLogicActor->GetPositionInfo().mWorldPosition, prevTime, mTimer[vehicleIndex], mTimer[vehicleIndex] - prevTime, diff, prevZone);

			mLastTime[vehicleIndex] = mTimer[vehicleIndex];
			mZonesCrossed[vehicleIndex]++;
		}

		mTimer[vehicleIndex] -= frameTime;

		//if(vehicleIndex == 0)
			//printf("Timer: %f\n", mTimer[vehicleIndex]);

		if(mTimer[vehicleIndex] <= 0.0f)
		{
			int zonesCrossed = mZonesCrossed[vehicleIndex];
			//AmaxDataLogging::LogCheckpointMissed(pLogicActor->GetPositionInfo().mWorldPosition, mZonesCrossed[vehicleIndex]);
			return true;
		}

		return false;
	}

	void RaceModeRule_TimerExpired::GetRuleString(U32 vehicleIndex, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		if ( stringType == RACE_MODE_RULE_STRING_TYPE_HUD_BONUS )
		{
			static UI::C_TextStringID string("RR_TimerExpired_HUD_Bonus");

			if ( mCurrentZoneIdx != 0 && mCurrentZoneIdx - 1 < mTimerAdditions.Size() )
				str.Format(string.String(), (S32)mTimerAdditions[mCurrentZoneIdx - 1]);

			return;
		}

		int triggers = 0;

		switch(stringType)
		{
			/*
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			{		
				static UI::C_TextStringID string("RR_TimerExpired");
				str.Format(string.String());
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				static UI::C_TextStringID string("RR_TimerExpired_HUD_Target");
				str.Format(string.String());
			}
			break;
			*/

		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			triggers = mTimerAdditions.Size();
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			triggers = mTimerAdditions.Size();
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				triggers = mTimerAdditions.Size() - mCurrentZoneIdx;

				if(triggers < 0)
					triggers = 0;
			}
			break;
		}

		static UI::C_TextStringID string("RR_TimerExpired_HUD_Current");
		static UI::C_TextStringID stringS("RR_TimerExpired_HUD_Current_S");
		static UI::C_TextStringID stringM("RR_TimerExpired_HUD_Current_M");
		static UI::C_TextStringID stringMS("RR_TimerExpired_HUD_Current_M_S");

		if(triggers != mTimerAdditions.Size())
		{
			if(triggers == 1)
				str.Format(stringMS.String(), triggers);
			else
				str.Format(stringM.String(), triggers);
		}
		else
		{
			if(triggers == 1)
				str.Format(stringS.String(), triggers);
			else
				str.Format(string.String(), triggers);
		}
	}

} // namespace Game