#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic\RaceRules\RaceAction_ManageGhosts.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

namespace Game
{

	void RaceAction_ManageGhosts::PerformAction(int carIdx)
	{
		Actors::RaceLogicCore* pRLC								= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		const Game::Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(carIdx);
		
		if( pRLC->GetPlayerVehicleIndex(0) == carIdx)
		{
			float currentLap = pLogicActor->GetLapInfo().mCurrentLap;
			if( pLogicActor->GetLapInfo().mCurrentLapTime <= pLogicActor->GetLapInfo().mBestLapTime || pLogicActor->GetLapInfo().mBestLapTime == -1)
			{
				Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(E_ReplayMessage_UpdateSessionGhost);
			}
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(E_ReplayMessage_TimeAttackNewLap);
		}
	}

} // namespace Game