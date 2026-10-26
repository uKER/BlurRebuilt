#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic\RaceRules\RaceRule_PerformActionYTimes.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_PerformActionYTimes::IsTriggered(int vehicleIndex)
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

		const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);

		switch(mAction)
		{
		case E_STUNT_ACTION_DRIFTING:
			return( pLogicActor->GetStatistics().mNumDrifts >= mNumTimes );
		}

		return false;
	}

	void RaceModeRule_PerformActionYTimes::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		// PR - not used!
		/*
		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			{
				if( isPassRule )
				{
					static UI::C_TextStringID string("RR_PerformXActionYTimes");
					str.Format(string.String(), mActionLocId.String(), mNumTimes);
				}
				else
				{
					static UI::C_TextStringID string("RR_PerformXActionYTimes_Not");
					str.Format(string.String(), mActionLocId.String(), mNumTimes);
				}			
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				static UI::C_TextStringID string("RR_PerformXActionYTimes_HUD_Target");
				str.Format(string.String(), mActionLocId.String(), mNumTimes);
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

				int vehIdx = idx;

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

				const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);

				int numTimes = 0;
				switch(mAction)
				{
				case E_STUNT_ACTION_DRIFTING:
					numTimes = pLogicActor->mStatistics.mNumDrifts;
					break;
				}

				static UI::C_TextStringID string("RR_PerformXActionYTimes_HUD_Current");
				str.Format(string.String(), numTimes);
			}
			break;
		}
		*/
	}

	void RaceModeRule_PerformActionYTimes::CacheStrings()
	{
		// PR - not used!
		/*
		switch(mAction)
		{
			case E_STUNT_ACTION_DRIFTING:
				mActionLocId = UI::C_TextStringID("RR_PerformXActionYTimes_Drifting");
				break;
		}
		*/
	}

	void RaceModeRule_PerformActionYTimes::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverID << mNumTimes;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_PerformActionYTimes::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverID >> mNumTimes;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace Game
