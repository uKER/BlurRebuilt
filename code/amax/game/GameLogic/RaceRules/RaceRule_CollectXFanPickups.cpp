#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <GameLogic\RaceRules\RaceRule_CollectXFanPickups.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_CollectXFanPickups::IsTriggered(int vehicleIndex)
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

		const Game::Actors::C_VehicleLogicActor* logicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);
		
		return (logicActor->GetStatistics().mFanPickupsCollected >= mNumPickups);
	}

	void RaceModeRule_CollectXFanPickups::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		// PR - not used!
		/*
		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			{
				if( isPassRule )
				{
					static UI::C_TextStringID string("RR_CollectXFanPickups");
					str.Format(string.String(),mNumPickups);
				}
				else
				{
					static UI::C_TextStringID string("RR_CollectXFanPickups_Not");
					str.Format(string.String(),mNumPickups);
				}			
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				static UI::C_TextStringID string("RR_CollectXFanPickups_HUD_Target");
				str.Format(string.String(), mNumPickups );
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

				Game::Actors::C_VehicleLogicActor* logicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);

				static UI::C_TextStringID string("RR_CollectXFanPickups_HUD_Current");
				str.Format(string.String(), logicActor->mStatistics.mFanPickupsCollected);
			}
			break;
		}
		*/
	}

	void RaceModeRule_CollectXFanPickups::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverID << mNumPickups;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_CollectXFanPickups::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverID >> mNumPickups;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace Game
