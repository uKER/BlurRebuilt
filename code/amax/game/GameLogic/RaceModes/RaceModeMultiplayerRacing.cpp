#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic\RaceModes\RaceModeMultiplayerRacing.h>
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

#include <GameLogic\RaceRules\RaceAction_EliminateLastDriver.h>
#include <Game\GameActors\Components\Messages\AudioMessages.h>

#include <GameLogic\RaceRules\RaceRulesManager.h>

#include <ai/AI_Amax.h>

#include <ui\Game\Gadgets\ButtonMessages.h>
#include <ui\UiInterface.h>
#include <ui\SoundEffects.h>
#include "Game/GameActors/Components/VehicleLogic/Multiplayer/VLC_MP_IdleKickController.h"

#include "Ui/Game/Multiplayer/MultiplayerCache.h"

extern Debug::Logger &g_MultiplayerLog;

namespace Game
{

void C_RaceModeMultiplayer_Racing::Initialise(Game::C_LevelState *pLevelState)
{
	m_TimeoutSettings=0;

	C_RaceModeMultiplayer::Initialise(pLevelState);

	SetInternalNameAndID("Multiplayer_Racing", E_RaceMode_Multiplayer_Racing);
	AddSubscriber( E_GameLogicMessage_Passed_All_Rule );
	AddSubscriber( E_GameflowMessage_EliminationTimeoutStart );
	AddSubscriber( E_GameflowMessage_EliminationTimeoutEnded );
}

//	Race Mode-specific updating...
void C_RaceModeMultiplayer_Racing::MpMode_Update()
{
	if(UpdateRacingTimers())
		return;

	UpdateLogicPositions();

	switch(m_FinishType)
	{
	case UI::E_MpFinishType_Laps:
		{
			CheckLaps();
			CheckRaceTimoutTimer();
		}
		break;

	case UI::E_MpFinishType_Time:
		{
			CheckTime();
		}
		break;
	}

	if(m_GameMode == E_GameMode_SplitScreen)
	{
		CheckForSplitscreenElimination(); 

		if(m_EliminationTimeOutTimer>=0.0f)
		{
			m_EliminationTimeOutTimer-= (F32)C_GameApp::GetInstance()->GetTimeStepRaw();
			m_EliminationTimeOutTimer = (m_EliminationTimeOutTimer < 0.0f) ? 0.0f : m_EliminationTimeOutTimer;
		}
	}
}

bool C_RaceModeMultiplayer_Racing::AddModeSpecificNetMessageBasicVehicleData( RacerNet::C_NetMessage& netMsg, U8 p_VehicleIndex )
{
	bool ok = netMsg.WriteF32(VehicleData(p_VehicleIndex).m_Statistics[UI::E_MultiplayerStat_LogicPositionCombined]);
	return ok;
}

bool C_RaceModeMultiplayer_Racing::HandleModeSpecificNetMessageBasicVehicleData( RacerNet::C_NetMessage& netMsg, S8 p_VehicleIndex )
{
	bool ok = netMsg.ReadF32(VehicleData(p_VehicleIndex).m_Statistics[UI::E_MultiplayerStat_LogicPositionCombined]);
	return ok;
}

bool C_RaceModeMultiplayer_Racing::UpdateRacingTimers( void )
{
	//	Return true if the racemode should skip any other updates/checks after this.
	if(mFinalDelayTimerActive)
	{
		DecrementFinalDelayTimer();
		return true;
	}
	if (m_TimeOutTimerActive)
	{
		const F32 ot = floorf( m_TimeOutTimer );

		//	Timeout timer we start once a vehicle achieves the lap target
		m_TimeOutTimer -= (F32)C_GameApp::GetInstance()->GetTimeStepRaw();
		if (m_TimeOutTimer <= 0)	// RACE_TIMEOUT_TIMER_LENGTH seconds
		{
			m_TimeOutTimer = 0.0f;

			ForceTimeOutLocalVehicles();

			mFinalDelayTimerActive = true;
			mFinalDelayTimer = FINAL_DELAY_FOR_RESULTS;
			return true;
		}
		else // Tell the UI when we get to the lower timer time
		{
			if (m_TimeoutType != MULTIPLAYER_TIMEOUT_TYPE_MAX_RACE_TIME && AllNotIdleVehiclesFinished())
			{
				ForceTimeOutLocalVehicles();
			}

			if(m_TimeoutType == MULTIPLAYER_TIMEOUT_TYPE_HIDDEN)
			{
				if(m_TimeOutTimer <= m_TimeoutSettings->mVisibleTimerTime)
				{
					_UI_MESSAGE(E_GameflowMessage_RaceTimeoutStart);
					m_TimeoutType = MULTIPLAYER_TIMEOUT_TYPE_VISIBLE;
				}
			}
		}
	}

	return false;
}

bool C_RaceModeMultiplayer_Racing::AllNotIdleVehiclesFinished() const
{
	if (!GetLevelState())
		return false;

	U32 numCars = GetLevelState()->mNumCars;
	for(U32 i = 0; i < numCars; i++)
	{
		NetVehicleData& vehicleData=VehicleData(i);
		if(vehicleData.m_State == E_VEHICLE_STATE_RACING&&!vehicleData.m_Idle)
			return false;
	}

	return true;
}

void C_RaceModeMultiplayer_Racing::CheckRaceTimoutTimer( void )
{
	Game::C_LevelState *pLevelState = GetLevelState();
	if (!pLevelState || !GetRLC() || m_TimeoutType == MULTIPLAYER_TIMEOUT_TYPE_NONE)
		return;


	U32 numCars = pLevelState->mNumCars;
	U8 finished = 0;
	for(U32 i = 0; i < numCars; i++)
	{
		//	[GavinY] Bug #38445 - Remote AI vehicles are still just network vehicles on every other kit, so use the mHuman flag to find real player vehicles.
		if(VehicleData(i).m_State == E_VEHICLE_STATE_FINISHED && pLevelState->mCars[i].mHuman)
			finished++;
	}

	if(finished == 0)
	{
		return;
	}

	//Only go in here if we havent turned or we are on the hidden timer (As we could potentially be switching to the visible one)
	if(!m_TimeOutTimerActive || m_TimeoutType == MULTIPLAYER_TIMEOUT_TYPE_HIDDEN || m_TimeoutType == MULTIPLAYER_TIMEOUT_TYPE_MAX_RACE_TIME)
	{
		//Check for failsafe only first (Including failsafe as a result of no timer settings)
		if(!m_TimeoutSettings || !m_AllowTimeouts || m_TimeoutSettings->mOnlyUseFailsafeTimer)
		{
			m_TimeOutTimerActive = true;
			if(m_TimeoutSettings)
			{
				m_TimeOutTimer = m_TimeoutSettings->mFailsafeTimerTime;
			}
			else
			{
				m_TimeOutTimer = 60.f;
			}
			m_TimeoutType = MULTIPLAYER_TIMEOUT_TYPE_FAILSAFE;
			m_InitialTimeoutTime = m_TimeOutTimer;
			return;
		}

		//If we are using the hidden or max race time timer, check if we should jump to the visible one early
		if(m_TimeoutType == MULTIPLAYER_TIMEOUT_TYPE_HIDDEN || m_TimeoutType == MULTIPLAYER_TIMEOUT_TYPE_MAX_RACE_TIME)
		{
			if(finished >= m_TimeoutSettings->mPlayersFinishedToActivateVisibleTimer)
			{
				m_TimeOutTimer = m_TimeoutSettings->mVisibleTimerTime;
				m_TimeoutType = MULTIPLAYER_TIMEOUT_TYPE_VISIBLE;
				m_TimeOutTimerActive = true;
				return;
			}
		}
		//Else if we are using the max race timer check for the invisible one
		if(m_TimeoutType == MULTIPLAYER_TIMEOUT_TYPE_MAX_RACE_TIME)
		{
			if(finished >= m_TimeoutSettings->mPlayersFinishedToActivateInsivibleTimer)
			{
				m_TimeOutTimer = m_TimeoutSettings->mInvisibleTimerTime;
				m_TimeoutType = MULTIPLAYER_TIMEOUT_TYPE_HIDDEN;
				m_TimeOutTimerActive = true;
				return;
			}
		}

		//Can stop here if the timer is already active
		if(m_TimeOutTimerActive)
		{
			return;
		}

		//Otherwise we need to see if we should set the hidden timer
		if(finished >= m_TimeoutSettings->mPlayersFinishedToActivateInsivibleTimer)
		{
			m_TimeOutTimer = m_TimeoutSettings->mInvisibleTimerTime;
			m_TimeoutType = MULTIPLAYER_TIMEOUT_TYPE_HIDDEN;
			m_TimeOutTimerActive = true;
			m_InitialTimeoutTime = m_TimeOutTimer;
			return;
		}
	}
}

void C_RaceModeMultiplayer_Racing::StartLevel()
{
	C_RaceModeMultiplayer::StartLevel();

	Game::C_LevelState *pLevelState = GetLevelState();
	if(!pLevelState)
		return;

	if(pLevelState->mLaps > 0)
		m_FinishType = UI::E_MpFinishType_Laps;
	else if(pLevelState->mTimeLimit > 0)
		m_FinishType = UI::E_MpFinishType_Time;


	//Find the timeout settings to use(Per event for playlists or defined in the MPConfig for custom races)
	E_PlayMode playMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetPlayMode();
	const XtArray< MultiplayerTimeoutSettings >* timeoutSettings = 0;
	if(playMode == E_PlayMode_Playlist)
	{
		const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
		if(pEvent)
		{
			timeoutSettings = &pEvent->timeout_settings;
		}
	}
	else if(playMode == E_PlayMode_CustomRace)
	{
		MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
		if(pConfig)
		{
			timeoutSettings = &pConfig->m_CustomRaceTimeout;
		}
	}
	//And in those settings find the one to apply based on the number of players
	if(timeoutSettings)
	{
		for(S8 settings = timeoutSettings->count-1; settings >= 0; settings--)
		{
			if(pLevelState->mNumCars >= (*timeoutSettings)[0].mMinPlayers)
			{
				m_TimeoutSettings = &(*timeoutSettings)[settings];
				break;
			}
		}
	}

	//Oh noes, nobody set any timeout settings for the playlist we are on.
	if(!m_TimeoutSettings)
	{
		LOGDEBUG(g_MultiplayerLog, "C_RaceModeMultiplayer_Racing::StartLevel - No timeout settings set for playlist. Using failsafe of 60 seconds only.");
	}

	m_AllowTimeouts = pLevelState->GetMultiplayerSettings().mAlowTimeout;

	if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsRanked())
	{
		m_TimeoutType = MULTIPLAYER_TIMEOUT_TYPE_MAX_RACE_TIME;
		m_TimeOutTimerActive = true;

		if(m_TimeoutSettings && m_TimeoutSettings->mMaxRaceTimerTime)
		{
			m_TimeOutTimer = m_TimeoutSettings->mMaxRaceTimerTime;
		}
		else
		{
			m_TimeOutTimer = 720; //12 minutes if no sorcery data
		}
	}
	else
	{
		m_TimeoutType = (m_AllowTimeouts) ? MULTIPLAYER_TIMEOUT_TYPE_HIDDEN : MULTIPLAYER_TIMEOUT_TYPE_NONE;
		m_TimeOutTimerActive = false;
	}

	m_GameMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode();

	m_EliminationTimeOutTimer = -1.0f;
}

//	Check all local vehicles to see if they have completed the required number of laps
void C_RaceModeMultiplayer_Racing::CheckLaps()
{
	Game::C_LevelState *pLevelState = GetLevelState();

	NetVehicleIndexStack localRacingList;
	GetLocalRacingList(localRacingList);

	const Game::Actors::C_VehicleLogicActor* pLogicActor = NULL;
	for (U8 i=0; i<localRacingList.Size(); i++)
	{
		pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localRacingList[i]);
		if( pLevelState->mLaps > 0 && (U32)pLogicActor->GetLapInfo().mCurrentLap > pLevelState->mLaps )
		{
			VehicleFinished(localRacingList[i], Game::E_VEHICLE_STATE_FINISHED);
		}
	}	
}

void C_RaceModeMultiplayer_Racing::CheckForSplitscreenElimination()
{
	Game::C_LevelState *pLevelState = GetLevelState();

	NetVehicleIndexStack localRacingList;
	GetLocalRacingList(localRacingList);

	const Game::Actors::C_VehicleLogicActor* pLogicActor = NULL;
	for (U8 i=0; i<localRacingList.Size(); i++)
	{
		pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localRacingList[i]);
		if( pLevelState->mCars[pLogicActor->GetVehicleIndex()].mState == E_VEHICLE_STATE_ELIMINATED)
		{
			VehicleFinished(localRacingList[i], Game::E_VEHICLE_STATE_ELIMINATED);
		}
	}	
}


//	Check race time against time limit. If time's up, any local vehicles still racing finish.
void C_RaceModeMultiplayer_Racing::CheckTime()
{
	Game::C_LevelState *pLevelState = GetLevelState();

	if(pLevelState->mTimeLimit > 0.0f && GetRLC()->GetRaceTime() >= (pLevelState->mTimeLimit))
	{
		NetVehicleIndexStack localRacingList;
		GetLocalVehicleList(localRacingList);
		for(U8 i = 0; i < localRacingList.Size(); i++)
		{
			VehicleFinished(localRacingList[i], Game::E_VEHICLE_STATE_FINISHED);
		}
	}
}

F32	C_RaceModeMultiplayer_Racing::GetRaceTimeout() const 
{
	if(m_EliminationTimeOutTimer >= 0.0f)
	{
		return m_EliminationTimeOutTimer;
	}
	else if(m_TimeoutType == MULTIPLAYER_TIMEOUT_TYPE_VISIBLE)
	{
		return m_TimeOutTimer;
	}

	return 0.f;
}

const F32 C_RaceModeMultiplayer_Racing::GetInitialTimeoutTime() const
{
	return m_InitialTimeoutTime;
}

void C_RaceModeMultiplayer_Racing::MpMode_SortResults()
{
	Game::C_LevelState *pLevelState = GetLevelState();

	NetVehicleIndexStack baseStack;
	SortAllNetJoinIds(baseStack);

	// Dont need to do any of this in Split screen (have out own results handlers)

	NetVehicleIndexStack vehiclesRacing;
	NetVehicleIndexStack vehiclesFinished;
	NetVehicleIndexStack vehiclesTimedOut;
	NetVehicleIndexStack vehiclesEliminated;
	NetVehicleIndexStack vehiclesWrecked;
	NetVehicleIndexStack vehiclesLeftRace;

	if(mIsNetworkGame)
	{
		for(S32 i = 0; i < baseStack.Size(); i++)
		{
			switch(VehicleData(baseStack[i]).m_State)
			{
			case E_VEHICLE_STATE_RACING:
				vehiclesRacing.PushBack(baseStack[i]);
				break;
			case E_VEHICLE_STATE_FINISHED:
				vehiclesFinished.PushBack(baseStack[i]);
				break;
			case E_VEHICLE_STATE_TIMED_OUT:
				vehiclesTimedOut.PushBack(baseStack[i]);
				break;
			case E_VEHICLE_STATE_FATALLY_DAMAGED:
				vehiclesWrecked.PushBack(baseStack[i]);
				break;
			case E_VEHICLE_STATE_ELIMINATED:
				vehiclesEliminated.PushBack(baseStack[i]);
				break;
			case E_VEHICLE_STATE_LEFT_RACE:
				vehiclesLeftRace.PushBack(baseStack[i]);
				break;
			}
		}
	}
	else
	{
		
		// Time to sort our local vehicles
		/*for(unsigned int i = 0; i < m_LevelState->mNumCars ; i++)
		{	
			// Need to first ensure all the sorting data needed is up to date! (this should surely be done on an event basis - e.g. vehicle finished) rather than every frame!
			VehicleFinished(i,m_LevelState->mCars[i].mState); 
			//FillVehicleData(i);
		}*/

		// Split screen gets its data directly from the level state
		for(unsigned int i = 0; i < pLevelState->mNumCars ; i++)
		{
			// Need to first ensure all the sorting data needed is up to date! (this should surely be done on an event basis - e.g. vehicle finished) rather than every frame!
			//FillVehicleData(i);

			switch(pLevelState->mCars[i].mState)
			{
			case E_VEHICLE_STATE_RACING:
				vehiclesRacing.PushBack(baseStack[i]);
				break;
			case E_VEHICLE_STATE_FINISHED:
				vehiclesFinished.PushBack(baseStack[i]); // Still use baseStack[i] so can use existing sorting
				break;
			case E_VEHICLE_STATE_TIMED_OUT:
				vehiclesTimedOut.PushBack(baseStack[i]);
				break;
			case E_VEHICLE_STATE_ELIMINATED:
				vehiclesEliminated.PushBack(baseStack[i]);
				break;
			case E_VEHICLE_STATE_FATALLY_DAMAGED:
				vehiclesWrecked.PushBack(baseStack[i]);
				break;
			case E_VEHICLE_STATE_LEFT_RACE:
				vehiclesLeftRace.PushBack(baseStack[i]);
				break;
			default:
				vehiclesTimedOut.PushBack(baseStack[i]);
				break;
			}
		}
	}


	//	Sort by race time for laps, otherwise distance travelled (for timed races)
	if(m_FinishType == UI::E_MpFinishType_Laps)
		SortByStatistic(vehiclesFinished, UI::E_MultiplayerStat_RaceTime, false);
	else
		SortByStatistic(vehiclesFinished, UI::E_MultiplayerStat_DistanceTravelled, true);

	SortByStatistic(vehiclesTimedOut,	UI::E_MultiplayerStat_DistanceTravelled, true);
	SortByStatistic(vehiclesRacing,		UI::E_MultiplayerStat_LogicPositionCombined, true);
	SortByStatistic(vehiclesEliminated, UI::E_MultiplayerStat_DistanceTravelled, true);
	SortByStatistic(vehiclesLeftRace,	UI::E_MultiplayerStat_DistanceTravelled, true);

	if(mIsNetworkGame)
	{
		SortByStatistic(vehiclesWrecked,	UI::E_MultiplayerStat_DistanceTravelled, true);
	}
	else
	{

		// If we have AI data (a track) then use the distance
		// travelled to determine position of cars that are 
		// wrecked. For any level which has no ai data use
		// racetime
		if(!AIAmax::AmaxAIR::Instance().IsDataInitialised())
		{
			SortByStatistic(vehiclesWrecked, UI::E_MultiplayerStat_RaceTime, true);
		}
		else
		{
			SortByStatistic(vehiclesWrecked, UI::E_MultiplayerStat_DistanceTravelled, true);
		}
	}
	
	//	Now we've sorted the vehicles, we put them into the results structure and give them final positions
	S8 position = 0;

	CopyFromStackIntoResults(vehiclesFinished, position);
	CopyFromStackIntoResults(vehiclesTimedOut, position);
	CopyFromStackIntoResults(vehiclesRacing, position);
	CopyFromStackIntoResults(vehiclesEliminated, position);
	CopyFromStackIntoResults(vehiclesWrecked, position);
	CopyFromStackIntoResults(vehiclesLeftRace, position);

	// If split screen Mode need to ensure level result positions are set to match (we use these)
	if(!mIsNetworkGame)
	{
		MpVehiclesData& vehicleResults = C_MpResults::Instance().m_SortedVehicleResults;
		Game::C_LevelResult& levelResults = C_GameApp::GetInstance()->GetGame()->GetLevelResult();
		levelResults.mNumCars = pLevelState->mNumCars; 
		for(unsigned int i = 0; i < pLevelState->mNumCars; i++)
		{
			if(vehicleResults[i].m_LocalIndex > -1)
			{
				levelResults.mCars[vehicleResults[i].m_LocalIndex].mPosition = vehicleResults[i].m_Position; 
			}
		}
	}
}

void C_RaceModeMultiplayer_Racing::CheckAllHumansFinished(int vehIdx)
{
	Game::C_LevelState *pLevelState = GetLevelState();

	BIZ_ASSERT(vehIdx < MAX_LEVELSTATE_CARS);
	pLevelState->mCars[vehIdx].mState = E_VEHICLE_STATE_FINISHED;

	int playerFinishedCount = 0;
	int playerCount = 0;
	for(unsigned int i = 0; i < pLevelState->mNumCars; i++)
	{
		if( !GetRLC()->IsVehicleAIControlled(i) )
		{
			playerCount++;
			if(pLevelState->mCars[i].mState != E_VEHICLE_STATE_RACING)
			{
				playerFinishedCount++;
			}
		}
	}

	if( playerCount == playerFinishedCount )
	{
		_UI_MESSAGE(E_GameflowMessage_RaceFinished);
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
	}
}

bool C_RaceModeMultiplayer_Racing::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if(!ShouldHandleMessages())
		return false;
	
	bool handledMessage = true;

	if(!mIsNetworkGame)
	{
		switch( pMessage->Type() )
		{
		case Game::E_GameLogicMessage_Failed_Rule:
			{
				const Game::C_FailedRuleMsg* pMsg = Messaging::MessageCast<Game::C_FailedRuleMsg>(pMessage);
				CheckAllHumansFinished(pMsg->mVehicleIndex);
			}
			break;

		case Game::E_GameLogicMessage_Passed_All_Rule:
			{
				const Game::C_PassedAllRuleMsg* pMsg = Messaging::MessageCast<Game::C_PassedAllRuleMsg>(pMessage);
				CheckAllHumansFinished(pMsg->mVehicleIndex);	
			}
			break;

		case Game::E_GameflowMessage_EliminationTimeoutStart:
			{
				const Game::C_EliminationTimerStartedMessage* pMsg = Messaging::MessageCast<Game::C_EliminationTimerStartedMessage>(pMessage);
				m_EliminationTimeOutTimer = pMsg->mTimeout;	
				break;
			}		
		case Game::E_GameflowMessage_EliminationTimeoutEnded:
			{
				m_EliminationTimeOutTimer = -1.0f;
				break;
			}
		}
	}

	return C_RaceModeMultiplayer::MessageHandler(pMessage);
}

void C_RaceModeMultiplayer_Racing::UpdateLogicPositions( void )
{
	NetVehicleIndexStack localRacingList;
	if(GetLocalRacingList(localRacingList) == 0)
		return;

	for(int i = 0; i < localRacingList.Size(); i++)
	{
		const Game::Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localRacingList[i]);
		if(pLogicActor)
			VehicleData(localRacingList[i]).m_Statistics[UI::E_MultiplayerStat_LogicPositionCombined] = pLogicActor->GetPositionInfo().mLogicPositionCombined;
	}
}

F32 C_RaceModeMultiplayer_Racing::MpMode_GetPrimaryStatValue( U8 vehicleIndex )
{
	if ( VehicleData(vehicleIndex).m_State == E_VEHICLE_STATE_FINISHED )
	{
		// So that finished vehicles dont have the same values, we add on a number between
		const F32 delta = (F32)(GetLevelState()->mNumCars - (C_MpResults::Instance().GetPlayerPosition( vehicleIndex )-1));

		return VehicleData(vehicleIndex).m_Statistics[UI::E_MultiplayerStat_LogicPositionCombined] + delta;
	}
	else
		return VehicleData(vehicleIndex).m_Statistics[UI::E_MultiplayerStat_LogicPositionCombined];
}

S32 C_RaceModeMultiplayer_Racing::GetVehicleTeamScoreContribution( NetVehicleData& vehicleData )
{
	return GetDriverRacePoints(vehicleData.m_Position, GetLevelState()->mNumCars);
}

} // namespace Game
