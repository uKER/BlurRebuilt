#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic\RaceRules\RaceAction_TimeAttackDisplay.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <RaceBook\RaceBookManager.h>

namespace Game
{
	void RaceAction_TimeAttackDisplay::PerformAction(int carIdx)
	{
		Actors::RaceLogicCore* pRLC								= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		Game::C_LevelState& levelState							= C_Game::GetInstance()->GetLevelState();
		RaceBook::RaceBookTimeAttackStore* timeAttackStore		= RaceBook::C_RaceBookManager::Instance().GetTimeAttackStore(levelState.mRoute);
		const Game::Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(carIdx);

		if( pRLC->GetPlayerVehicleIndex(0) != carIdx || pLogicActor->GetLapInfo().mCurrentLapTime >= pLogicActor->GetLapInfo().mBestLapTime )
			return;

		// Check if we are already in the list
		for( int i = 0; i < timeAttackStore->mEntries.Size(); i++ )
		{
			if( timeAttackStore->mEntries[i].mName == levelState.mCars[carIdx].mCharacterID )
			{
				if( pLogicActor->GetLapInfo().mCurrentLapTime <= timeAttackStore->mEntries[i].mTime )
					// Can't use swap/pop here, or we lose the ordering.
					timeAttackStore->mEntries.Erase(timeAttackStore->mEntries.Begin() + i);
				else
					return;
			}
		}

		for( int i = 0; i < timeAttackStore->mEntries.Size(); i++ )
		{
			if( pLogicActor->GetLapInfo().mCurrentLapTime < timeAttackStore->mEntries[i].mTime )
			{
				RaceBook::RaceBookTimeAttackEntry entry;
				entry.mTime = pLogicActor->GetLapInfo().mCurrentLapTime;
				entry.mName = levelState.mCars[carIdx].mCharacterID;

				if( timeAttackStore->mEntries.Size() == RaceBook::MAX_RACEBOOK_TIME_ATTACK_ENTRIES )
					timeAttackStore->mEntries.PopBack();

				timeAttackStore->mEntries.InsertBefore(entry, timeAttackStore->mEntries.Begin() + i);

				break;
			}
		}
	}

} // namespace Game