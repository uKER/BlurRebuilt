#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic\RaceRules\RaceAction_EliminateHitByPerks.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <GameLogic\RaceRules\RaceRulesManager.h>
#include "GameLogic/Messaging/GameLogicMessages.h"
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <Game\GameActors\Components\GameComponents.h>

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <Ui/UiHelpers.h>

namespace Game
{
	RaceAction_EliminateHitByPerks::RaceAction_EliminateHitByPerks() : mFirerID(0), mNumLives(0), m_pLogicLayer(0)
	{
		//NB/ The race logic layer's not set up by the time this constructor so we'll need to cache our pointers the first time perform action's called
	}

	void RaceAction_EliminateHitByPerks::CachePointers()
	{
		if(!m_pLogicLayer)
		{
			//Cache a pointer to the race logic layer
			m_pLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			BIZ_ASSERT_MSG(m_pLogicLayer, "RaceAction_EliminateHitByPerks() > Couldn't cache a pointer to the race logic layer");
		}
	}

	//This will be called once per vehicle in the race
	void RaceAction_EliminateHitByPerks::PerformAction(int carIdx)
	{
		Actors::C_VehicleLogicActor* pLogicActor;

		if(!m_pLogicLayer)
		{
			CachePointers();
		}

		bool IsInSplitScreenMode = false;
		if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == E_GameMode_SplitScreen)
		{
			IsInSplitScreenMode = true;
		}

		bool eliminate = false;
		int numVehicles = m_pLogicLayer->GetNumVehicles();
		int numHitsTaken = 0;

		if(mFirerID)
		{
			//In this mode we're only looking for hits taken from a single, specified driver
			C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
			int firerVehicleIndex = -1;
			
			//Go through each vehicle in the race and compare the character ID we're interested in
			for(int a = 0; a < numVehicles; a++)
			{
				const LevelStateCar& car = levelState.mCars[a];
				if(car.mCharacterID == mFirerID)
				{
					firerVehicleIndex = a;
					break;
				}
			}

			//We haven't been able to find the vehicle in which the character we're worried about is in...no point in carrying on
			if(firerVehicleIndex == -1)
				return;

			//Get the baddy's logic actor
			pLogicActor = m_pLogicLayer->GetLogicVehicleActor(firerVehicleIndex);
			if( !mPickups.Size() && !IsInSplitScreenMode )
			{
				VehicleRaceStatistics& statistics = pLogicActor->GetStatistics();

				//One shot kill
				if( statistics.mDualVehicleStats[carIdx].mNumberOfTimesHitWithAggressivePerk || 
					statistics.mDualVehicleStats[carIdx].mNumberOfTimesHitWithDoubleAggressivePerk)
				{
					eliminate = true;
				}
			}
			else
			{
				//Multiple hit mode
				numHitsTaken += UpdateNumberOfHits(carIdx, pLogicActor);
				if(numHitsTaken >= mNumLives)
				{
					eliminate = true;
				}
			}
		}
		else
		{
			//In this mode we're looking for hits from any vehicle (apart from our own...running into your own mine? valid?)
			for( int i = 0; i < m_pLogicLayer->GetNumVehicles(); i++ )
			{
				pLogicActor = m_pLogicLayer->GetLogicVehicleActor(i);
				VehicleRaceStatistics& statistics = pLogicActor->GetStatistics();

				if( !mPickups.Size() && !IsInSplitScreenMode )
				{				
					if( statistics.mDualVehicleStats[carIdx].mNumberOfTimesHitWithAggressivePerk || 
						statistics.mDualVehicleStats[carIdx].mNumberOfTimesHitWithDoubleAggressivePerk)
					{
						eliminate = true;
					}
				}
				else
				{
					//Multiple hit mode
					numHitsTaken += UpdateNumberOfHits(carIdx, pLogicActor);
					if(numHitsTaken >= mNumLives)
					{
						eliminate = true;
						break;
					}
				}
			}
		}

		if( eliminate )
		{
			pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(carIdx);
			Actor::C_ActorHandle graphicalHandle = pLogicActor->GetGraphicalActor(carIdx)->GetHandle();

			// Tell the graphical actor to disable.
			//Actors::C_VehicleEliminationMessage elimMsg(pLogicActor->GetHandle(), carIdx);
			//pLogicActor->SendMessageToActor(pLogicActor->GetHandle(), &elimMsg);

			// Wreck them instead, because the poor yanks get confused.
			C_VehicleLogicForceDamageMsg forceDamageMsg(carIdx, pLogicActor->GetMaxHealth());
			pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Damage, &forceDamageMsg);

			// Tell the world this vehicle has failed.
			C_RaceRulesManager::Instance().SendFailMessage(carIdx);

			Game::C_LevelResult& levelResult = C_Game::GetInstance()->GetLevelResult();
			for( int i = 0; i < levelResult.mCars[carIdx].mPassedRule.Size(); i++ )
				levelResult.mCars[carIdx].mPassedRule[i] = false;

			if ( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == E_GameMode_SplitScreen )
			{
				Game::C_GameModeBase *pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
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

			//C_RaceRulesManager::Instance().IncrementNumEliminated();
		}
	}

	void RaceAction_EliminateHitByPerks::GetActionString(U32 vehicleIndex, UI::C_WideString512& str, RACE_ACTION_STRING_TYPE stringType) const
	{
		if(stringType != RACE_ACTION_STRING_TYPE_HUD_CURRENT)
		{
			BIZ_WARNING_MSG(false, "RaceAction_EliminateHitByPerks::::GetActionString() > NON IMPLEMENTED MESSAGE TYPE, YOU'RE NOT GETTING ANY STRINGY GOODNESS TODAY :(");
			return;
		}

		const int numHitsTaken = UpdateNumberOfHitsFromAllVehicles(vehicleIndex);

		static UI::C_TextStringID string("RA_HitByPerks_D_of_D_LivesLeft");

		int livesLeft = Maths::Clamp(mNumLives - numHitsTaken, 0, mNumLives);

		str.Format(string.String(), livesLeft, mNumLives);
	}

	void RaceAction_EliminateHitByPerks::SetNumberOfLives(const int lives)
	{
		mNumLives = lives;
	}

	void RaceAction_EliminateHitByPerks::AddPickup(const enum E_PICKUP_TYPE ePickupType)
	{
		mPickups.PushBack(ePickupType);
	}	

	void RaceAction_EliminateHitByPerks::SetFirerID(const int firerId)
	{
		mFirerID = firerId;
	}

	const int RaceAction_EliminateHitByPerks::UpdateNumberOfHitsFromAllVehicles(const int receivingVehicleIndex) const 
	{
		//Go through each vehicle
		int numHitsTaken = 0;

		//The a pointer to the logic actor of the vehicle we're counting the number of power up impacts on
		const Actors::C_VehicleLogicActor* pLogicActor = 0;
		for(int vehicleIndex = 0; vehicleIndex < m_pLogicLayer->GetNumVehicles(); vehicleIndex++)
		{
			//Check we're not looking at the receiving vehicle
			if(vehicleIndex == receivingVehicleIndex)
			{
				continue;
			}

			pLogicActor = m_pLogicLayer->GetLogicVehicleActor(vehicleIndex);

			numHitsTaken += UpdateNumberOfHits(receivingVehicleIndex, pLogicActor);
		}

		return numHitsTaken;
	}

	const int RaceAction_EliminateHitByPerks::UpdateNumberOfHits(const int vehicleIndex, const Actors::C_VehicleLogicActor* pLogicActor) const
	{
		//Check
		BIZ_ASSERT_MSG(pLogicActor, "RaceAction_EliminateHitByPerks::UpdateNumberOfHits() > Being passed an invalid logic actor");
		int hitsTaken = 0;

		const DualVehicleRaceStatistics& statistics = pLogicActor->GetStatistics().mDualVehicleStats[vehicleIndex];

		for( int j = 0; j < mPickups.Size(); j++ )
		{
			switch(mPickups[j])
			{
			case E_PICKUP_TYPE_SHOCK:
				hitsTaken += statistics.mNumberOfShocksAgainstDriver;
				hitsTaken += statistics.mNumberOfDoubleShocksAgainstDriver;
				break;

			case E_PICKUP_TYPE_BARGE:
				hitsTaken += statistics.mNumberOfBargesAgainstDriver;
				hitsTaken += statistics.mNumberOfDoubleBargesAgainstDriver;
				break;

			case E_PICKUP_TYPE_SHUNT:
				hitsTaken += statistics.mNumberOfShuntsAgainstDriver;
				hitsTaken += statistics.mNumberOfDoubleShuntsAgainstDriver;
				break;

			case E_PICKUP_TYPE_NITROUS:
				hitsTaken += statistics.mNumTimesOvertookWithNitro;
				hitsTaken += statistics.mNumTimesOvertookWithDoubleNitro;
				break;

			case E_PICKUP_TYPE_BOMB:
				hitsTaken += statistics.mNumberOfBombsAgainstDriver;
				hitsTaken += statistics.mNumberOfDoubleBombsAgainstDriver;
			}
		}

		return hitsTaken;
	}


} // namespace Game
