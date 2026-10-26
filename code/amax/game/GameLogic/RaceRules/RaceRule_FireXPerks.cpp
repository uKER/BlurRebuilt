#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <GameLogic\RaceRules\RaceRule_FireXPerks.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_FireXPerks::IsTriggered(int vehicleIndex)
	{
		C_LevelState& levelState					= C_Game::GetInstance()->GetLevelState();
		C_RaceLogicLayer* pLayer					= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor* pActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
		VehicleRaceStatistics& statistics			= pActor->GetStatistics();

		int numFired = 0;
		if( !mPickupTypes.Size() )
		{
			for( int i = 0; i < statistics.mNumPerksFired.Size(); i++ )
				numFired += statistics.mNumPerksFired[i];
		}
		else
		{
			for( int i = 0; i < mPickupTypes.Size(); i++ )
				 numFired += statistics.mNumPerksFired[mPickupTypes[i]];
		}

		return ( numFired >= mNumFired );
	}

	void RaceModeRule_FireXPerks::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		// PR - not used
		/*
		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			{
				if( isPassRule )
				{
					static UI::C_TextStringID string("RR_FireXPerks");
					str.Format(string.String(), mNumFired);
				}
				else
				{
					static UI::C_TextStringID string("RR_FireXPerks_Not");
					str.Format(string.String(), mNumFired);
				}			
			}
			break;
		}
		*/
	}

} // namespace Game