#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic\RaceRules\RaceRule_PerformXTrickShots.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_DriverPerformedXTrickShots::IsTriggered(int vehicleIndex)
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

		bool triggered = (pLogicActor->GetScoring().mSkillShots >= mTrickShotsNeeded);

		if(triggered)
			mDontShowOnHUD[vehicleIndex] = true;

		return triggered;
	}

	void RaceModeRule_DriverPerformedXTrickShots::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		int trickShots = 0;

		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			trickShots = mTrickShotsNeeded;
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			trickShots = mTrickShotsNeeded;
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

				trickShots = mTrickShotsNeeded - pLogicActor->GetScoring().mSkillShots;

				if(trickShots < 0)
					trickShots = 0;
			}
			break;
		}

		static UI::C_TextStringID string("RR_PerformXTrickShots_HUD_Current");
		static UI::C_TextStringID stringS("RR_PerformXTrickShots_HUD_Current_S");
		static UI::C_TextStringID stringM("RR_PerformXTrickShots_HUD_Current_M");
		static UI::C_TextStringID stringMS("RR_PerformXTrickShots_HUD_Current_M_S");

		if(trickShots != mTrickShotsNeeded)
		{
			if(trickShots == 1)
				str.Format(stringMS.String(), trickShots);
			else
				str.Format(stringM.String(), trickShots);
		}
		else
		{
			if(trickShots == 1)
				str.Format(stringS.String(), trickShots);
			else
				str.Format(string.String(), trickShots);
		}
	}

} // namespace Game
