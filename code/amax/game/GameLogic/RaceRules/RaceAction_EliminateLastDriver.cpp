#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic\RaceRules\RaceAction_EliminateLastDriver.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <GameLogic\RaceRules\RaceRulesManager.h>

#include "GameLogic/Messaging/GameLogicMessages.h"

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <Ui/UiHelpers.h>

Debug::Float g_PercentageOfCarsToEliminate("SplitScreenMode.PercentageOfCarsToEliminate", 0.5, Debug::Range(0.0f, 1.0f));

namespace Game
{
	RaceAction_EliminateLastDriver::RaceAction_EliminateLastDriver()
	:mElminationTimerModeActive(false)
	{
	}

	void RaceAction_EliminateLastDriver::Update(float t) 
	{
		float displayTimer=mTimer/2.f;
		if (displayTimer<10.f)
			displayTimer=10.f;

		if(mCurrentTimer<displayTimer)
		{
			if( !mElminationTimerModeActive )
			{
				//Update number of players to be eliminated this time
				Game::C_GameModeBase *pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
				if(pGameModeBase->GetID() == Game::E_GameMode_SplitScreen)
				{
					int numVehicles = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetNumVehicles();
					int numEliminated = C_RaceRulesManager::Instance().GetNumEliminated();

					int playersToEliminate = (int)(float(numVehicles - numEliminated) * g_PercentageOfCarsToEliminate);

					BIZ_ASSERT_MSG(playersToEliminate > 0, "RaceAction_EliminateLastDriver::Update - Trying to eliminate 0 players. Infinite loop.");
				}

				mElminationTimerModeActive = true;
				C_EliminationTimerStartedMessage eliminationTimerStartMessage(displayTimer);
				C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &eliminationTimerStartMessage );
				_UI_MESSAGE(Game::E_GameflowMessage_EliminationTimeoutStart);
			}
		}
		else if( mElminationTimerModeActive )
		{
			mElminationTimerModeActive = false;
			C_EliminationTimerEndedMessage eliminationTimerEndedMessage;
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &eliminationTimerEndedMessage );
			_UI_MESSAGE(Game::E_GameflowMessage_EliminationTimeoutEnded);
		}
	}

	void RaceAction_EliminateLastDriver::Init()
	{
		mEliminationAlreadyHappenedThisFrame = false;
		mEliminatedThisFrame = 0;
	}

	void RaceAction_EliminateLastDriver::PerformAction(int carIdx)
	{
		Game::C_GameModeBase *pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

		if(mEliminationAlreadyHappenedThisFrame)
		{
			return;
		}

		Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(carIdx);

		int numVehicles = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetNumVehicles();
		int numEliminated = C_RaceRulesManager::Instance().GetNumEliminated();
		int numToEliminate;

		//Splitscreen for Last Man Standing - Until a quarter of the vehicles remain we eliminate half
		if(pGameModeBase->GetID() == Game::E_GameMode_SplitScreen 
			&& numVehicles + mEliminatedThisFrame - numEliminated > (numVehicles + mEliminatedThisFrame) * (g_PercentageOfCarsToEliminate / 2.0f))
		{
			numToEliminate = (int)(float(numVehicles + mEliminatedThisFrame - numEliminated) * g_PercentageOfCarsToEliminate);
		}
		else
		{
			numToEliminate = mNumDrivers;
		}

		if( pLogicActor->GetPositionInfo().mRacePosition > numVehicles - numEliminated - numToEliminate )
		{
			Actor::C_ActorHandle graphicalHandle = pLogicActor->GetGraphicalActor(carIdx)->GetHandle();

			// Tell the graphical actor to disable.
			Actors::C_VehicleEliminationMessage elimMsg(pLogicActor->GetHandle(), carIdx);
		//	pLogicActor->SendMessageToActor(graphicalHandle, &elimMsg);
			pLogicActor->SendMessageToActor(pLogicActor->GetHandle(), &elimMsg);

			mEliminatedThisFrame++;

			if(mEliminatedThisFrame == numToEliminate)
			{
				if(pGameModeBase->GetID() == Game::E_GameMode_SplitScreen)
				{
					if (mTimer>20.f)
					{
						mTimer/=2.f;
					}

					//Update players to eliminate
					int playersToEliminate = (int)(float(numVehicles - numEliminated) * g_PercentageOfCarsToEliminate);
				}

				mEliminationAlreadyHappenedThisFrame = true;
			}

			// Tell the world this vehicle has failed.
			C_FailedRuleMsg failedMsg( carIdx );
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &failedMsg );

			C_RaceRulesManager::Instance().IncrementNumEliminated();

			if ( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == E_GameMode_SplitScreen )
			{
				if(pGameModeBase)
				{
					if(pGameModeBase->GetID() == Game::E_GameMode_SplitScreen)
					{
						Game::C_Game::GetInstance()->GetLevelState().mCars[carIdx].mState = E_VEHICLE_STATE_ELIMINATED;
					}
				}

				// PR - tell the UI
				if ( UI::GetLevelState().mCars[ carIdx ].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER )
				{
					_UI_MESSAGE1( Game::E_VehicleMessage_Eliminated, UI::GetLocalPlayerIndex( carIdx ) );
				}
			}
		}
	}

} // namespace Game
