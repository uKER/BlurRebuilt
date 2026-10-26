#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic\RaceRules\RaceRule_EarnXP.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/GameComponents.h>

#include <ui/Game/UILocaliser.h>
#include "ui/UiXtConfig.h"

#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>

namespace Game
{
	RaceModeRule_DriverEarnXP::RaceModeRule_DriverEarnXP() : mDriverID(0), mXpNeeded(0)
	{
		for( int i = 0; i < Game::MAX_RACELOGIC_VEHICLES; i++ )
			mCurrentXp[i] = 0;

		if( UI::C_UiXtConfig::Instance() )
		{
			mPulseFreq		= UI::C_UiXtConfig::Instance()->m_PulseEarnXpFreq;				// Pulse frequency when not earning fans
			mPulseWaitTime	= UI::C_UiXtConfig::Instance()->m_PulseEarnXpWaitTime;			// Time not earning fans until we start pulsing
			mPulseTimer		= mPulseWaitTime;
		}
	}

	bool RaceModeRule_DriverEarnXP::IsTriggered(int vehicleIndex)
	{
		C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();


		int vehIdx = vehicleIndex;

		if( mDriverID )
		{
			for(unsigned int a=0;a<C_Game::GetInstance()->GetLevelState().mNumCars;a++)
			{
				const LevelStateCar& car = levelState.mCars[a];
				if(car.mCharacterID == mDriverID)
				{
					vehIdx = a;
					break;
				}
			}
		}

		const Actors::C_VehicleLogicActor* pLogicActor						= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);
		Components::C_VehicleComponent_NewScoring* vehScoringComponent	= (Components::C_VehicleComponent_NewScoring*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_NewScoring);

		U32 currentXP = 0;

		if(vehScoringComponent)
			currentXP = vehScoringComponent->GetTotalFans();

		// Err whaaat? Only tracking player0 for this hackery
		if( vehIdx == 0 && mCurrentXp[ 0 ] != currentXP )
			mPulseTimer = mPulseWaitTime;

		mCurrentXp[vehIdx] = currentXP;

		bool triggered = (currentXP >= (U32)mXpNeeded);

		if(triggered)
			mDontShowOnHUD[vehicleIndex] = true;

		return triggered;
	}

	void RaceModeRule_DriverEarnXP::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		int xp = 0;

		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			xp = mXpNeeded;
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			xp = mXpNeeded;
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				xp = mXpNeeded - mCurrentXp[idx];

				if(xp < 0)
					xp = 0;
			}
			break;
		}

		static UI::C_TextStringID string("RR_EarnXP_HUD_Current");
		static UI::C_TextStringID stringS("RR_EarnXP_HUD_Current_S");
		static UI::C_TextStringID stringM("RR_EarnXP_HUD_Current_M");
		static UI::C_TextStringID stringMS("RR_EarnXP_HUD_Current_M_S");

		if(xp != mXpNeeded)
		{
			if(mXpNeeded == 1)
				str.Format(stringMS.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas(xp));
			else
				str.Format(stringM.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas(xp));
		}
		else
		{
			if(mXpNeeded == 1)
				str.Format(stringS.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas(xp));
			else
				str.Format(string.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas(xp));
		}
	}

	void RaceModeRule_DriverEarnXP::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverID << mXpNeeded;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_DriverEarnXP::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverID >> mXpNeeded;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace Game