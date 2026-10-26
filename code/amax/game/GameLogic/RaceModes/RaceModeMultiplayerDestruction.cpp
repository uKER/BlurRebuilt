#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic\RaceModes\RaceModeMultiplayerDestruction.h>
#include <GameLogic\MultiplayerUtilities.h>

#include <GameLogic\Messaging\GameLogicMessages.h>

#include <GameLogic\Structures\MpResults.h>

#include <Game\Messaging\GameflowMessages.h>
#include <Game\Messaging\GameMessages.h>
#include <Game\Messaging\PickupActorMessages.h>

#include <Game\GameWorld\GameWorld.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\GameComponents.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <GameLogic\Layers\RaceLogicLayer.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Layers\AmaxWorldLayer.h>

#include <Network/Misc/NetDefines.h>
#include <Network/Core/NetServices.h>
#include <Network/player/NetPlayerManager.h>
#include <Network\Core.h>

#include <Game/GameActors/Components/Messages/AudioMessages.h>
#include <ui/UiMessages.h>

#include <ui/UiInterface.h>
#include <ui\Game\Gadgets\ButtonMessages.h>
#include "Ui/SoundEffects.h"

Debug::Bool g_InfiniteTimeOut("Multiplayer.MotorMash.InfiniteTimeout", false);
Debug::Bool g_InfiniteScoreLimit("Multiplayer.MotorMash.InfiniteScoreLimit", false);
Debug::Bool g_ForceTimeOutNow("Multiplayer.MotorMash.ForceTimeOutNow", false);

#ifndef CFG_GOLD
Debug::Int g_GiftLowDamages("Multiplayer.MotorMash.GiftLowDamages", 0, Debug::Range(0, 100));
Debug::Int g_GiftHighDamages("Multiplayer.MotorMash.GiftHighDamages", 0, Debug::Range(0, 100));
Debug::Int g_GiftRivalHighDamages("Multiplayer.MotorMash.GiftRivalHighDamages", 0, Debug::Range(0, 100));
Debug::Int g_GiftRivalLowDamages("Multiplayer.MotorMash.GiftRivalLowDamages", 0, Debug::Range(0, 100));
Debug::Int g_GiftRivalWrecks("Multiplayer.MotorMash.GiftRivalWrecks", 0, Debug::Range(0, 100));

Debug::Bool g_GiftNow("Multiplayer.MotorMash.GiftNow", false);
#endif //CFG_GOLD


namespace Game
{
void C_RaceModeMultiplayer_Destruction::Initialise(Game::C_LevelState *pLevelState)
{
	C_RaceModeMultiplayer::Initialise(pLevelState);

	AddSubscriber( E_VehicleLogicMsg_HitByPerk );

	SetInternalNameAndID("Multiplayer_Destruction", E_RaceMode_Multiplayer_Destruction);

	m_UiTimeOutTimerNext = -1.0f;

	// Init delay timer form sorcery
	mRivalsCommencementDelayTimer = C_MultiplayerData::Instance().m_pConfig->m_RivalsCommencementDelayTimer;   // Time after race starts, and before Countdown starts.  
	mRivalsCountdownDelayTimer	  = C_MultiplayerData::Instance().m_pConfig->m_RivalsCountdownTimer;		   // Time before rivals selected
	mInitialRivalsAssigned		  = false;
	
	mBlockScoringMessageSent   = false;
	mRivalsCountdownDelayActive = true;
}

void C_RaceModeMultiplayer_Destruction::StartLevel()
{
	PARENTCLASS::StartLevel();

	m_UiTimeOutTimerNext = -1.0f;

	// Flag based on team mode
	mRivalsModeEnabled = !Game::C_Game::GetInstance()->GetLevelState().mTeams;

	if(mRivalsModeEnabled)
	{
		// Reset flags
		mRivalsCommencementDelayTimer = C_MultiplayerData::Instance().m_pConfig->m_RivalsCommencementDelayTimer;   // Time after race starts, and before Countdown starts.  
		mRivalsCountdownDelayTimer	  = C_MultiplayerData::Instance().m_pConfig->m_RivalsCountdownTimer;		   // Time before rivals selected
		mInitialRivalsAssigned		  = false;
		mBlockScoringMessageSent = false;
		mRivalsCountdownDelayActive = true;
	}
}

void C_RaceModeMultiplayer_Destruction::MpMode_Update()
{
#ifndef CFG_GOLD
	if(g_GiftNow)
	{
		g_GiftNow = false;
		Game::Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(0);
		VehicleRaceStatistics& statistics = pLogicActor->GetStatistics();
		statistics.mNumLowDamageAttacks += g_GiftLowDamages;
		statistics.mNumHighDamageAttacks += g_GiftHighDamages;
		statistics.mNumRivalLowDamageAttacks += g_GiftRivalLowDamages;
		statistics.mNumRivalHighDamageAttacks += g_GiftRivalHighDamages;
		statistics.mNumRivalVehiclesWrecked += g_GiftRivalWrecks;
	}
#endif //CFG_GOLD

	UpdateLocalData();

	if(mFinalDelayTimerActive)
	{
		DecrementFinalDelayTimer();
		return;
	}
	else
	{
		CheckTimeOut();
		CheckScoreLimit();
	}

	// If reach here race has started
	if(mRivalsModeEnabled && !mInitialRivalsAssigned)
	{
		HandleInitialRivalConfig((F32)C_GameApp::GetInstance()->GetTimeStepRaw());
	}
}

F32 C_RaceModeMultiplayer_Destruction::GetRaceTimeout() const
{
	Game::C_LevelState* pLevelState = GetLevelState();
	return pLevelState->mTimeLimit - GetRLC()->GetRaceTime();
}

void C_RaceModeMultiplayer_Destruction::UpdateLocalData()
{
	NetVehicleIndexStack localRacingList;
	if(GetLocalRacingList(localRacingList) == 0)
		return;
	for(int i = 0; i < localRacingList.Size(); i++)
	{
		Game::Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localRacingList[i]);
		if(pLogicActor)
		{
			VehicleData(localRacingList[i]).m_Statistics[UI::E_MultiplayerStat_OnTrackPoints] = CalculateVehicleOnTrackPoints(pLogicActor);
			VehicleData(localRacingList[i]).m_Statistics[UI::E_MultiplayerStat_Fans] = pLogicActor->GetFansAwarded();
		}
	}
}

//Perform the incredibly complicated calculation of the score, using all of my 25 years of advanced mathematical studying 
const S32 C_RaceModeMultiplayer_Destruction::CalculateVehicleOnTrackPoints( Game::Actors::C_VehicleLogicActor* pLogicActor ) const
{
	//...
	VehicleRaceStatistics& statistics = pLogicActor->GetStatistics();
		
	if (!C_MultiplayerData::Instance().m_pConfig)
		return -1;

	MotorMashScoringValues* pScoringValues =	&C_MultiplayerData::Instance().m_pConfig->m_MotormashScoringValues;

	float rivalDamageScoreMultiplier = pScoringValues->m_RivalDamageMultiplier;
	int rivalWreckScore = (int)((float)(pScoringValues->m_WreckScore) * rivalDamageScoreMultiplier);
	int rivalLowDamageScore = (int)((float)(pScoringValues->m_LowDamageScore) * rivalDamageScoreMultiplier);
	int rivalHighDamageScore = (int)((float)(pScoringValues->m_HighDamageScore) * rivalDamageScoreMultiplier);

	return (statistics.mNumVehiclesWrecked * pScoringValues->m_WreckScore) 
		+ (statistics.mNumLowDamageAttacks * pScoringValues->m_LowDamageScore) 
		+ (statistics.mNumHighDamageAttacks * pScoringValues->m_HighDamageScore) 
		- (statistics.mNumTimesSuicided * pScoringValues->m_SuicideScore)
		+ (statistics.mNumRivalVehiclesWrecked * rivalWreckScore) 
		+ (statistics.mNumRivalLowDamageAttacks * rivalLowDamageScore) 
		+ (statistics.mNumRivalHighDamageAttacks * rivalHighDamageScore);
}

void C_RaceModeMultiplayer_Destruction::MpMode_SortResults()
{
	NetVehicleIndexStack baseStack;
	SortAllNetJoinIds(baseStack);

	NetVehicleIndexStack vehiclesRacing;
	NetVehicleIndexStack vehiclesLeftRace;

	for(S32 i = 0; i < baseStack.Size(); i++)
	{
		switch(VehicleData(baseStack[i]).m_State)
		{
		case E_VEHICLE_STATE_RACING:
			vehiclesRacing.PushBack(baseStack[i]);
			break;
		case E_VEHICLE_STATE_FINISHED:
			vehiclesRacing.PushBack(baseStack[i]);
			break;
		case E_VEHICLE_STATE_TIMED_OUT:
			vehiclesRacing.PushBack(baseStack[i]);
			break;
		case E_VEHICLE_STATE_FATALLY_DAMAGED:
			vehiclesRacing.PushBack(baseStack[i]);
			break;
		case E_VEHICLE_STATE_ELIMINATED:
			vehiclesRacing.PushBack(baseStack[i]);
			break;
		case E_VEHICLE_STATE_LEFT_RACE:
			vehiclesLeftRace.PushBack(baseStack[i]);
			break;
		}
	}

	SortByStatistic(vehiclesRacing, UI::E_MultiplayerStat_OnTrackPoints, true);
	SortByStatistic(vehiclesLeftRace, UI::E_MultiplayerStat_OnTrackPoints, true);

	//We only need to worry about ties if there are more than one people racing...to 
	//be honest there nearly always will be or the game will end!
	if(vehiclesRacing.Size() > 1)
	{
		//Now we'll do a second ordering based upon
		CheckForTiedScores(vehiclesRacing);
	}

	//	Now we've sorted the vehicles, we put them into the results structure and give them final positions
	S8 position = 0;
	
	CopyFromStackIntoResults(vehiclesRacing, position);
	CopyFromStackIntoResults(vehiclesLeftRace, position);

	UpdateTeamScores();
}

void C_RaceModeMultiplayer_Destruction::CheckForTiedScores(NetVehicleIndexStack& vehiclesRacingSortedByOnTrackPoints) const
{
	//We need to go through the list of racing vehicles checking for ties (two or more vehicles with the same number of kills),
	//when we find two vehicles with the same number of kills we'll compare the amount of fans they have. If they are still tied
	//its the luck of the draw
	NetVehicleIndexStack vehiclesTied;

	int killTotal = -99999999;	//They'll be doing well to get this many suicides
	S8 firstTiedArrayIndex = -1;
	for(S32 i = 0; i < vehiclesRacingSortedByOnTrackPoints.Size(); i++)
	{
		int currentVehicleIndex = vehiclesRacingSortedByOnTrackPoints[i];

		//Check that the current vehicle doesn't have a kill total that is the same as the previous vehicle
		if( killTotal == VehicleData( currentVehicleIndex ).m_Statistics[UI::E_MultiplayerStat_OnTrackPoints] )
		{
			//We've found a tied vehicle, push it into our tied vehicle array
			vehiclesTied.PushBack( currentVehicleIndex );
		}
		else
		{
			//We've found a vehicle that's not tied, so we'll sort out the tied vehicles we've stored, if any
			if(vehiclesTied.Size() > 1)
			{
				ResolveTies(vehiclesRacingSortedByOnTrackPoints, vehiclesTied, firstTiedArrayIndex);
			}
			

			//Store the current number of kills to check the succeeding vehicles against
			killTotal = VehicleData( currentVehicleIndex ).m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];

			vehiclesTied.Clear();
			vehiclesTied.PushBack( currentVehicleIndex );

			firstTiedArrayIndex = i;
		}
	}

	//The last vehicle might be tied with the second last so just clear up any odds and sods
	if(vehiclesTied.Size() > 1)
	{
		ResolveTies(vehiclesRacingSortedByOnTrackPoints, vehiclesTied, firstTiedArrayIndex);
	}

}

void C_RaceModeMultiplayer_Destruction::ResolveTies(NetVehicleIndexStack& vehiclesRacingSortedByOnTrackPoints, NetVehicleIndexStack& vehiclesTied, const int firstTiedArrayIndex) const
{
	BIZ_ASSERT_MSG((firstTiedArrayIndex >= 0), "C_RaceModeMultiplayer_Destruction::ResolveTies() > Invalid firstTiedArrayIndex");

	//Sort the tied vehicles by fans
	SortByStatistic(vehiclesTied, UI::E_MultiplayerStat_Fans, true);

	//Now repopulate the vehiclesRacingSortedByOnTrackPoints array
	S8 destinationIndex = 0;
	for(S32 j = 0; j < vehiclesTied.Size(); j++)
	{
		//Copy the vehicle index stored in the vehicles array, now sorted by fans back into the source array
		destinationIndex = firstTiedArrayIndex + j;
		
		vehiclesRacingSortedByOnTrackPoints[destinationIndex] = vehiclesTied[j];
	}
}

bool C_RaceModeMultiplayer_Destruction::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if(!ShouldHandleMessages())
		return false;
	
	switch(pMessage->Type())
	{
		case E_VehicleLogicMsg_HitByPerk:
		{
			// Motor mash scoring message for power-up impact in split screen (done on receiving net message in online games)
			if(!mIsNetworkGame)
			{
				const C_VehicleLogicMsg_HitByPerkMsg* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >( pMessage );
				if(pMsg)
				{
					U8 firingVehicleIndex = pMsg->mFiringVehicleIndex;
					U8 victimVehicleIndex = pMsg->mVehicleIndex;
					const Actors::C_VehicleLogicActor* pFiringLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(firingVehicleIndex);
					if(pFiringLogicActor && pFiringLogicActor->pNewScoringComponent)
					{
						pFiringLogicActor->pNewScoringComponent->HandleMotorMashMessagePickupHit(victimVehicleIndex, firingVehicleIndex, pMsg->mPickupType, pMsg->mPickupSubType);
					}
				}
			}
			return true;
		}
	}

	return C_RaceModeMultiplayer::MessageHandler(pMessage);
}

bool C_RaceModeMultiplayer_Destruction::AddModeSpecificNetMessageBasicVehicleData( RacerNet::C_NetMessage& netMsg, U8 p_VehicleIndex )
{
	bool ok = netMsg.WriteF32(VehicleData(p_VehicleIndex).m_Statistics[UI::E_MultiplayerStat_OnTrackPoints]);
	if(!ok)
	{
		return ok;
	}
	
	ok = netMsg.WriteF32(VehicleData(p_VehicleIndex).m_Statistics[UI::E_MultiplayerStat_Fans]);
	return ok;
}

bool C_RaceModeMultiplayer_Destruction::HandleModeSpecificNetMessageBasicVehicleData( RacerNet::C_NetMessage& netMsg, S8 p_VehicleIndex )
{
	bool ok = netMsg.ReadF32(VehicleData(p_VehicleIndex).m_Statistics[UI::E_MultiplayerStat_OnTrackPoints]);
	if(!ok)
	{
		return ok;
	}

	ok = netMsg.ReadF32(VehicleData(p_VehicleIndex).m_Statistics[UI::E_MultiplayerStat_Fans]);
	return ok;
}

void C_RaceModeMultiplayer_Destruction::CheckTimeOut()
{
	// PR - UI hooks
	if ( GetLevelState()->mTimeLimit > 0.0f )
	{
		const F32 fTimeoutStart = 60.0f;
		const F32 fTimeoutSoundStart = 30.0f;

		F32 fDiff = GetLevelState()->mTimeLimit - GetRLC()->GetRaceTime();

		if ( ( fDiff >= 0.0f ) && ( fDiff <= fTimeoutStart ) )
		{
			if ( m_UiTimeOutTimerNext == -1.0f )
			{
				m_UiTimeOutTimerNext = fTimeoutSoundStart;
				_UI_MESSAGE( E_GameflowMessage_RaceTimeoutStart );
			}

			if (fDiff <= fTimeoutSoundStart)
			{
				if ( fDiff <= m_UiTimeOutTimerNext )
				{
					/*
					if ( m_UiTimeOutTimerNext <= 3.0f )
						UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_CountDown03ActionIn );
					else
						UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_CountDownActionIn );
					*/

					m_UiTimeOutTimerNext -= 1.0f;
				}
			}
		}
	}

	//	Check the global timeout (should be fairly generous) so that the event eventually ends.
	if( (GetLevelState()->mTimeLimit > 0.0f && GetRLC()->GetRaceTime() >= (GetLevelState()->mTimeLimit) && !g_InfiniteTimeOut) || g_ForceTimeOutNow)
	{
		ForceTimeOutLocalVehicles(E_VEHICLE_STATE_FINISHED);
	}
}

void C_RaceModeMultiplayer_Destruction::CheckScoreLimit()
{
	S32 scoreLimit = GetLevelState()->GetMultiplayerSettings().mScoreLimit;
	if(scoreLimit <= 0 || g_InfiniteScoreLimit)
		return;
	NetVehicleIndexStack localRacingList;
	if(GetLocalRacingList(localRacingList) == 0)
		return;

	if(AnyVehiclesFinished())
	{
		//	A vehicle has finished, and since this is a there is a score limit race, all other vehicles now finish too
		for(int i = 0; i < localRacingList.Size(); i++)
		{
			VehicleFinished(localRacingList[i], Game::E_VEHICLE_STATE_FINISHED);
		}
	}
	else
	{
		for(int i = 0; i < localRacingList.Size(); i++)
		{
			if(TeamData(VehicleData(localRacingList[i]).m_Team).m_TeamScore >= scoreLimit)
			{
				VehicleFinished(localRacingList[i], Game::E_VEHICLE_STATE_FINISHED);
			}
		}
	}
}

void C_RaceModeMultiplayer_Destruction::VehicleFinished( const U8 p_Index, Game::E_VEHICLE_STATE p_FinishType )
{
	C_RaceModeMultiplayer::VehicleFinished(p_Index, p_FinishType);

	if(p_FinishType != E_VEHICLE_STATE_LEFT_RACE)
	{
		Game::Actors::C_VehicleActor* vehicleActor = GetRLC()->GetVehicleActor(p_Index);
		vehicleActor->GetVehiclePhysics()->Controls().Clear();
		vehicleActor->GetVehiclePhysics()->Controls().Hold();
		Actors::C_VehicleControlsDisableMsg disableControlsMsg(p_Index);
		vehicleActor->SendMessageToActor(vehicleActor->GetHandle(), &disableControlsMsg);
	}
}

void C_RaceModeMultiplayer_Destruction::HandleInitialRivalConfig(float frameTime)
{
	// Turn off all scoring messages until rivals logic starts up
	if(!mBlockScoringMessageSent)
	{
		// PR - no longer supported
		//UI::C_UiEnableDisableGenericMessage genericMsg( false );
		//_UI_MESSAGE_FULL( genericMsg );

		// PR - no longer supported
		//UI::C_UiEnableDisableScoringMessage scoringMsg( false );
		//_UI_MESSAGE_FULL( scoringMsg );

		mBlockScoringMessageSent = true;
	}

	// Step 1 - Delay before showing countdown
	if((mRivalsCommencementDelayTimer-frameTime) > 0.0f)
	{
		mRivalsCommencementDelayTimer -= frameTime;
		return;
	}
	else
	{
		mRivalsCommencementDelayTimer = 0.0f; 
		frameTime -= mRivalsCommencementDelayTimer;
	}

	// Step 2 - Countdown before rivals chosen
	if((mRivalsCountdownDelayTimer-frameTime) > 0.0f)
	{
		mRivalsCountdownDelayTimer -= frameTime;
		return;
	}
	else
	{
		mRivalsCountdownDelayActive = false;
		mRivalsCountdownDelayTimer = 0.0f; 
	}

	// Turn on the motormash scoring messages

	// PR - no longer supported
	//UI::C_UiEnableDisableGenericMessage genericMsg( true );
	//_UI_MESSAGE_FULL( genericMsg );

	// PR - no longer supported
	//UI::C_UiEnableDisableScoringMessage scoringMsg( true );
	//_UI_MESSAGE_FULL( scoringMsg );

	// Tell each local player to choose a rival
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	for(U8 i = 0; i < levelState.mNumCars; i++)
	{
		if(VehicleIsLocal(i))
		{
			// Get the player logic actor and make them choose a new rival 
			Game::Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(i);
			BIZ_ASSERT_MSG(pLogicActor,"C_RaceModeMultiplayer_Destruction::HandleInitialRivalConfig(float frameTime) < couldnt get logic actor pointer..");
			if(pLogicActor)
			{
				Components::C_VehicleComponent_NewScoring* pScoring = (Components::C_VehicleComponent_NewScoring*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_NewScoring);
				BIZ_ASSERT_MSG(pScoring,"C_RaceModeMultiplayer_Destruction::HandleInitialRivalConfig(float frameTime) < couldnt access logic actor's scoring component..");
				if(pScoring)
				{
					// Choose rivals
					pScoring->SelectRivalIndex_Random();
				}
			}
		}
		else
		{
			// No more local players from here on in
			break;
		}
	}

	// Indicate we are done
	mInitialRivalsAssigned = true;
}

F32 C_RaceModeMultiplayer_Destruction::MpMode_GetPrimaryStatValue( U8 vehicleIndex )
{
	return VehicleData(vehicleIndex).m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];
}

S32 C_RaceModeMultiplayer_Destruction::GetVehicleTeamScoreContribution( NetVehicleData& vehicleData )
{
	return vehicleData.m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];
}

} // namespace Game
