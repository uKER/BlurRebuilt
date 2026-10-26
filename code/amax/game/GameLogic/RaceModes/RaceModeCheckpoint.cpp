#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic/RaceModes/RaceModeCheckpoint.h>

#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameLogic\RaceRules\RaceRulesManager.h>

#include <Game\Messaging\GameflowMessages.h>
#include <ui\Game\Gadgets\ButtonMessages.h>
#include "GameLogic/Messaging/GameLogicMessages.h"

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <RaceBook/RaceBookManager.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include "Game/GameActors/Components/GameComponents.h"

#include <ui\Game\Gadgets\ButtonMessages.h>
#include <ui\UiHelpers.h>
#include <ui\UiMessages.h>

#include <GameLogic\profile\GameProfileManager.h>

#include <SinglePlayer\SinglePlayer.h>

#include <ai/AI_Amax.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include <Game/GameActors/Components/GameComponents.h>

#include "Automation\GameDataLogging.h"
#include "Game/AmaxCam/CameraSystem.h"

//Debug::Float g_debugMaxCheckpointFreezeTime("GameModes.Checkpoint.MaxFreezeTime", 12.0f);
Debug::Bool	gCheckpointUnlimitedTime("Amax.GameMode.Checkpoint.UnlimitedTime", false);


extern Debug::Float g_debugEliminatedHideTimer;

//#pragma optimize("", off)					// TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP


namespace Game
{


//=================================================================================================
//							Initialise Routines
//=================================================================================================

void C_RaceModeCheckpoint::Initialise(Game::C_LevelState *pLevelState)
{
	SetInternalNameAndID("Checkpoint",E_RaceMode_Checkpoint);

	InitialiseVars();

	SetLevelState(pLevelState);

	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	AddSubscriber( E_GameLogicMessage_Scoring_Event );
	AddSubscriber( E_GameLogicMessage_Failed_Rule );
	AddSubscriber( E_GameLogicMessage_Passed_All_Rule );

	AddSubscriber( E_GameflowMessage_RaceStarted );
	AddSubscriber( E_GameflowMessage_RaceFinished );
	AddSubscriber( E_PickupMessage_CollectStopwatch );
}

//=================================================================================================

void C_RaceModeCheckpoint::InitialiseVars()
{

	mRaceInProgress		= false;
	mNumCheckpoints		= 0;
	mCheckpointsPassed	= 0;
	mTimeLeft			= 0.0f;
	mTimeDiffToMedal	= 0.0f;

	mCheckpointsPassed	= 0;
	mCurrentRaceRuleArea= -1;

	mEventState			= kSP_Incomplete;

	mUiTimeAddedCheckpoint = 0.0f;
	mUiTimeAddedStopwatch = 0.0f;
	mUiTimeLast = 0.0f;
	
	mLastValidPlayerPosInitialised = false;

	mNitrosCollected = 0;
	mNitrosUsed = 0;
	mClocksCollected = 0;
	mTimeAtLastCheckpoint = 0.f;
}

//=================================================================================================

void C_RaceModeCheckpoint::SetupRace()
{
}

//=================================================================================================


void C_RaceModeCheckpoint::StartLevel()
{
	SP_CheckpointEvent* pEvent = GetEvent();
	if (!pEvent || !pEvent->event || !pEvent->event->raceMode)
		return;


	InitialiseVars();

	AmaxRaceModeCheckpointVars*	diffvars = GetDifficultyVars();

	mTimeLeft			= diffvars->initialTime;
	mTimeDiffToMedal	= 0.0f;
	mNumCheckpoints		= diffvars->timeAwarded.count;

	C_RaceModeBase::StartLevel();
}



//=================================================================================================

void C_RaceModeCheckpoint::StartRace()
{
	C_RaceModeBase::StartRace();
}

//=================================================================================================

SP_CheckpointEvent* C_RaceModeCheckpoint::GetEvent()
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	SP_Event* pBaseEvent = SP_GetEventById( SP_GetCurrentSaveGame(), levelState.mSpEventId );
	BIZ_ASSERT(pBaseEvent);

	if (!pBaseEvent)		
		return NULL;
	
	if (pBaseEvent->kind != kSP_CheckpointEvent)
		return NULL;
	
	return (SP_CheckpointEvent*)pBaseEvent;
}

//=================================================================================================

AmaxRaceModeCheckpointVars*	C_RaceModeCheckpoint::GetDifficultyVars()
{
	SP_CheckpointEvent* pEvent = GetEvent();
	if (!pEvent || !pEvent->event || !pEvent->event->raceMode)
		return(NULL);

	AmaxRaceModeCheckpoint* pRaceMode = (AmaxRaceModeCheckpoint*)pEvent->event->raceMode;

	switch(C_Game::GetInstance()->GetLevelState().m_difficulty)
	{	
		case Game::GAME_DIFFICULTY_VERY_EASY:
		case Game::GAME_DIFFICULTY_EASY:
			return(&pRaceMode->easy);

		case Game::GAME_DIFFICULTY_NORMAL:
			return(&pRaceMode->medium);

		case Game::GAME_DIFFICULTY_HARD:
		case Game::GAME_DIFFICULTY_HARDCORE:
			return(&pRaceMode->hard);
	}

	return(&pRaceMode->easy);					// Should never reach here
}


//=================================================================================================
//							Update Routines
//=================================================================================================

void C_RaceModeCheckpoint::Update()
{
	if(!mRaceInProgress)
		return;

	SP_CheckpointEvent* pEvent = GetEvent();
	if (!pEvent || !pEvent->event || !pEvent->event->raceMode)
		return;

	Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);
	if (!pLogicActor)
		return;
	VehiclePositionInfo* posinfo = &pLogicActor->GetPositionInfo();

	AmaxRaceModeCheckpoint* pRaceMode = (AmaxRaceModeCheckpoint*)pEvent->event->raceMode;


// Check for player driving backwards
	F32	logicpos = posinfo->mLogicPosition;

	if (!mLastValidPlayerPosInitialised)				// If first time in then just get the current position
	{
		mLastValidPlayerPosLap = logicpos;
		mLastValidPlayerPosWorld = pLogicActor->GetWorldMatrix().AW();
		GetValidPlayerDirection();
		mLastValidPlayerPosInitialised = true;
	}
	else
	{
		if (mCheckpointsPassed == 0)							// Only stop car going backwards if the player hasn't passed the start line
		{
			F32	lapinc = (logicpos - mLastValidPlayerPosLap) + 1.0f;
			lapinc -= (F32)((S32)lapinc);						// Get Mod
			if (lapinc < 0.5f)									// If moving forward
			{
				mLastValidPlayerPosLap = logicpos;
				mLastValidPlayerPosWorld = pLogicActor->GetWorldMatrix().AW();
				GetValidPlayerDirection();
			}
			else
			{
				if (lapinc < 0.98f)								// If travelled backwards more than 0.02 of a lap
				{
					Game::Actors::C_VehicleResetOnTrack resetMsg(pLogicActor->GetHandle());
					resetMsg.mDirection = mLastValidPlayerPosDirection;
					resetMsg.mPosition = mLastValidPlayerPosWorld;
					resetMsg.mVehicleIndex = 0;
					resetMsg.mOverride = true;
					resetMsg.mVelocity = 0.1f;
					
					pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_HandOfGod, &resetMsg);

					AmaxCam::CameraSystem().ResetVehicleFeed(0);			// Reset the chasde cam behind the car
				}
			}
		}
	}


	if (posinfo->mLastRaceRuleArea != mCurrentRaceRuleArea)		// Not checking for only next so just look for a change
	{
		mCurrentRaceRuleArea = posinfo->mLastRaceRuleArea;

		if ((mCheckpointsPassed > 0) && (mCheckpointsPassed < mNumCheckpoints))				// Ignore start and finish lines
		{
			F32 fTimeAdded = GetDifficultyVars()->timeAwarded[mCheckpointsPassed];
			mTimeLeft += fTimeAdded;
			mUiTimeAddedCheckpoint = fTimeAdded;
			mUiTimeLast = fTimeAdded;
			_UI_MESSAGE( UI::E_UiGameMsg_SpCheckpointPassed );

			C_VehicleLogicMsg_CrossedCheckpoint msg(pLogicActor->GetVehicleIndex());
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&msg);

			UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_PassedCheckpointActionIn );

		}

		if (mCheckpointsPassed > 0)
		{
			float timeFromLastPoint = pLogicActor->GetRLC()->GetRaceTime() - mTimeAtLastCheckpoint;

#ifndef CFG_GOLD
			U8 clocksCollected = pLogicActor->GetStatistics().mStopwatchPickupsCollected - mClocksCollected;
			mClocksCollected = pLogicActor->GetStatistics().mStopwatchPickupsCollected;
			U8 nitrosCollected = pLogicActor->GetStatistics().mNitrousPickupsCollected - mNitrosCollected;
			mNitrosCollected = pLogicActor->GetStatistics().mNitrousPickupsCollected;
			U8 nitrosUsed = pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_NITROUS] - mNitrosUsed;
			mNitrosUsed = pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_NITROUS];

			AmaxDataLogging::LogCheckpointHit(pLogicActor->GetPositionInfo().mWorldPosition, mTimeLeft-mUiTimeLast, mTimeLeft, mUiTimeLast, timeFromLastPoint, mCheckpointsPassed, 
					nitrosCollected, nitrosUsed, clocksCollected);
#endif

			mTimeAtLastCheckpoint = pLogicActor->GetRLC()->GetRaceTime();
		}

		mCheckpointsPassed++;
		if (mCheckpointsPassed > mNumCheckpoints)					// Passed finish line
		{
			C_RaceRulesManager::Instance().SendPassMessage(0);
			mRaceInProgress = false;
		}
	}

	if (mCheckpointsPassed > 1)												// Update medal status
	{
		if (AmaxRaceModeCheckpointVars*	diffVars = GetDifficultyVars())
		{
			if (mTimeLeft >= diffVars->goldTimeRemaining)
			{
				mEventState = kSP_Gold;
				mTimeDiffToMedal = 0.0f;
			}
			else if (mTimeLeft >= diffVars->silverTimeRemaining)
			{
				mEventState = kSP_Silver;
				mTimeDiffToMedal = diffVars->goldTimeRemaining;
			}
			else if (mTimeLeft >= 0.0f)
			{
				mEventState = kSP_Bronze;
				mTimeDiffToMedal = diffVars->silverTimeRemaining;
			}
			else
			{
				mEventState = kSP_Incomplete;
				mTimeDiffToMedal = 0.0f;//diffVars->bronzeTimeRemaining;
			}
		}
	}

	if (mCheckpointsPassed > 0)
	{
		if (!gCheckpointUnlimitedTime)
			mTimeLeft -= C_GameApp::GetInstance()->GetTimeStep();				// Reduce timer and check that it hasn't run out

		if (mTimeLeft < 0.0f)
		{
#ifndef CFG_GOLD
			U8 clocksCollected = pLogicActor->GetStatistics().mStopwatchPickupsCollected - mClocksCollected;
			mClocksCollected = pLogicActor->GetStatistics().mStopwatchPickupsCollected;
			U8 nitrosCollected = pLogicActor->GetStatistics().mNitrousPickupsCollected - mNitrosCollected;
			mNitrosCollected = pLogicActor->GetStatistics().mNitrousPickupsCollected;
			U8 nitrosUsed = pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_NITROUS] - mNitrosUsed;

			AmaxDataLogging::LogCheckpointMissed(pLogicActor->GetPositionInfo().mWorldPosition, mCheckpointsPassed, nitrosCollected, nitrosUsed, clocksCollected);
#endif
			mEventState = kSP_Incomplete;
			C_RaceRulesManager::Instance().SendFailMessage(0);
		}
	}

	//store this away so the Friend Demands can use them
	pLogicActor->GetScoring().mCheckpointTimeLeft = mTimeLeft;
}

//=================================================================================================


void C_RaceModeCheckpoint::GetValidPlayerDirection()
{
	airacingtech::ITrackDataAI *pTrackData			= AIAmax::AmaxAIR::Instance().GetTrackDataSystem();
	airacingtech::AITrackDataRoutes* pLogicRoute	= pTrackData->GetTrackData()->m_TrackLogicData.m_pLogicRoute;
	airacingtech::ISplineTracker* pSplineTracker	= AIAmax::AmaxAIR::Instance().GetAISplineTracker(0);
	int idx											= pSplineTracker->GetLogicRoute()->m_index;
	if(idx > 1 && idx < pLogicRoute->GetNumSegments() - 1)
	{
		pLogicRoute->GetDirection(idx, mLastValidPlayerPosDirection);
	}
}

//=================================================================================================

void C_RaceModeCheckpoint::UpdateHUD()
{
	if(!mRaceInProgress)
		return;

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	if(mScoreTimer)
	{
		mScoreTimer=Maths::Clamped(mScoreTimer-pRLC->GetFrameTime(),0.f,3.0f);

		if(mScoreTimer == 0.f)
		{
			UI::C_UiTimelineMessage message( "earning_power", false );
			UI::gButtons.BroadcastMessageNow( 0xffffffff, &message );
		}
	}
	if(mRulePassedTimer)
	{
		mRulePassedTimer=Maths::Clamped(mRulePassedTimer-pRLC->GetFrameTime(),0.f,3.0f);
	}
}

//=================================================================================================

const UI::C_WideString512 C_RaceModeCheckpoint::GetScoreString()
{
	UI::C_WideString512 str;
	UI::C_WideString64 str2;

	str.Clear();

	return str;
}

//=================================================================================================

const UI::C_WideString512 C_RaceModeCheckpoint::GetRulePassedString(U32 idx)
{
	UI::C_WideString512 str;

	if(mRulePassedTimer)
		C_RaceRulesManager::Instance().GetPassRuleString(idx, str, mRuleIndex, Game::RACE_MODE_RULE_STRING_TYPE_RESULTS);
	else
		str.Clear();

	return str;
}

//=================================================================================================

const UI::C_WideString512 C_RaceModeCheckpoint::GetRuleFailedString(U32 idx)
{
	UI::C_WideString512 str;

	if(mRulePassedTimer)
		C_RaceRulesManager::Instance().GetFailRuleString(idx, str, mRuleIndex, Game::RACE_MODE_RULE_STRING_TYPE_RESULTS);
	else
		str.Clear();

	return str;
}

//=================================================================================================

bool C_RaceModeCheckpoint::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	if(!ShouldHandleMessages())
		return false;

	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case Game::E_PickupMessage_CollectStopwatch:
			{
				if (mRaceInProgress)
				{
					SP_CheckpointEvent* pEvent = GetEvent();
					if (pEvent && pEvent->event && pEvent->event->raceMode)
					{
						AmaxRaceModeCheckpoint* pRaceMode = (AmaxRaceModeCheckpoint*)pEvent->event->raceMode;

						F32 fTimeAdded = GetDifficultyVars()->stopwatchIncTime;
						mTimeLeft += fTimeAdded;
						mUiTimeAddedStopwatch = fTimeAdded;
						mUiTimeLast = fTimeAdded;
						_UI_MESSAGE( UI::E_UiGameMsg_SpCollectStopwatch );
						UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_StopwatchCollectedActionIn );

						SP_BossDemands::Instance().RegisterBossDemandAction(E_BOSSDEMAND_ACTION_COLLECT_STOPWATCH);
					}				
				}
			}
			break;

		//temp until we have a proper gadget
		case Game::E_GameLogicMessage_Scoring_Event:
			{
				const Game::C_ScoringEventMsg* pMsg = Messaging::MessageCast<Game::C_ScoringEventMsg>(pMessage);

				if(pMsg->mVehicleIndex == 0)
				{
					UI::C_UiTimelineMessage message( "earning_power", true );
					UI::gButtons.BroadcastMessageNow( 0xffffffff, &message );

					mScoreTimer=3.0f;
					mScore=pMsg->mScore;
					mScoreString=pMsg->mString;
				}
			}
			break;

		case Game::E_GameLogicMessage_Passed_All_Rule:
			{
				const Game::C_PassedAllRuleMsg* pMsg	= Messaging::MessageCast<Game::C_PassedAllRuleMsg>(pMessage);
				Game::Actors::RaceLogicCore* pRLC		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

				if( !pRLC->IsVehicleAIControlled(pMsg->mVehicleIndex) )
				{
					_UI_MESSAGE(E_GameflowMessage_RaceFinished);
					C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished);
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
				}
				else
				{
					// Should hopefully never be hit, but just in case, we remove them as the player cannot target them.
					Game::C_RaceLogicLayer* pLayer				= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
					pLayer->GetLogicVehicleActor(pMsg->mVehicleIndex)->PreRemoveFromRace(g_debugEliminatedHideTimer);
				}
			}
			break;

		case Game::E_GameLogicMessage_Failed_Rule:
			{
				const Game::C_FailedRuleMsg* pMsg = Messaging::MessageCast<Game::C_FailedRuleMsg>(pMessage);
				Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

				if( !pRLC->IsVehicleAIControlled(pMsg->mVehicleIndex) )
				{
					_UI_MESSAGE(E_GameflowMessage_Failed);
					C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished );
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
				}
			}
			break;

		case Game::E_GameflowMessage_RaceStarted:
			{
				Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor *) pRLC->GetVehicleActor(0);
				const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
				
				if (pActor && levelState.mCars[pActor->GetVehicleIndex()].mControlType != E_VEHICLE_CONTROL_TYPE_AI)
				{
					pActor->ForcePlayerControl();
				}
			}
			break;

		case Game::E_GameflowMessage_RaceFinished:
			{
				Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

				for( int i = 0; i < pLayer->GetNumVehicles(); i++)
				{
					Game::Actors::C_VehicleActor* pActor = (Game::Actors::C_VehicleActor*) pLayer->GetLogicVehicleActor(i)->GetGraphicalActor(i);
					if(pActor && !pRLC->IsVehicleAIControlled(i))
						pActor->ForceAIControl();
				}

			}
			break;
	}

	return handledMessage;
}

//=================================================================================================
// For UI testing only

void C_RaceModeCheckpoint::SetupFakeResults()
{
	Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	Game::C_LevelState& state=C_Game::GetInstance()->GetLevelState();

	if(!state.mEvent)
		state.mEvent = C_GameEventData::Instance().mEvents[0]->ID;

	if(!state.mCity)
		state.mCity = C_GameData::Instance().mCities[0]->ID;
	
	if(!state.mNumCars)
		state.mNumCars = C_GameEventData::Instance().mEvents[0]->opponents.count;

	results.Clear();
	results.mFake=true;
	results.mNumCars=state.mNumCars;

	int i = 1;

	for(unsigned int a=0;a<state.mNumCars;a++)
	{
		results.mCars[a].mPosition = i++;
		results.mCars[a].mScore = 1000;
		results.mCars[a].mStatistics.Clear();
		results.mCars[a].mStatistics.mFinished = true;

		for( int b = 0; b < C_RaceRulesManager::Instance().GetNumPassRules(); b++ )
		{
			results.mCars[a].mPassedRule.PushBack(true);
		}

		for( int b = 0; b < C_RaceRulesManager::Instance().GetNumFailRules(); b++ )
		{
			results.mCars[a].mFailedRule.PushBack(false);
		}
	}

	C_RaceModeBase::SetupResults();

	results.Finished();

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_ResultsCalculated );

	mRaceInProgress=false;
}

//=================================================================================================

void C_RaceModeCheckpoint::SetupQuitAndWinResults(lua_State *pState)
{
	Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	Game::C_LevelState& state=C_Game::GetInstance()->GetLevelState();

	results.Clear();
	results.mNumCars=state.mNumCars;

	for(unsigned int a=0;a<state.mNumCars;a++)
	{
		results.mCars[a].mPosition=a+1;
		results.mCars[a].mStatistics.Clear();
		results.mCars[a].mStatistics.mFinished = true;
	}

	SP_CheckpointEvent* pEvent = GetEvent();

	if (pEvent && pEvent->event)
	{
		if (const Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0))
		{
			S8 pos;
			Lua::GetDataMember(pState,"Position",pos,true);

			int score;
			Lua::GetDataMember(pState,"XP",score,true);

			int goldFanDemand;
			Lua::GetDataMember(pState,"GoldFanDemand",goldFanDemand,true);
			int goldFanDemandFans;
			Lua::GetDataMember(pState,"GoldFanDemandFans",goldFanDemandFans,true);

			if (pos <= 1)
				results.mRaceStatistics.mMedal = kSP_Gold;
			else if (pos <= 2)
				results.mRaceStatistics.mMedal = kSP_Silver;
			else if (pos <= 3)
				results.mRaceStatistics.mMedal = kSP_Bronze;
			else
				results.mRaceStatistics.mMedal = kSP_Incomplete;

			results.mRaceStatistics.mFansEarned = score;
			results.mRaceStatistics.mFanDemandsComplete = pLogicActor->GetScoring().mFanDemandsComplete;
			results.mRaceStatistics.mGoldenFanDemandComplete = (goldFanDemand == 1);
			results.mRaceStatistics.mGoldenFanDemandScore = goldFanDemandFans;
		}
	}

	C_RaceModeBase::SetupQuitAndWinResults(pState);

	results.Finished();

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_QuitAndWinRace );
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_ResultsCalculated );

	mRaceInProgress=false;
}

//=================================================================================================

void C_RaceModeCheckpoint::SetupResults()
{
	Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	Game::C_LevelState& state=C_Game::GetInstance()->GetLevelState();
	
	results.Clear();
	results.mNumCars=state.mNumCars;
	results.mRaceStatistics.mTotalRaceTime = mTimeLeft;


	SP_CheckpointEvent* pEvent = GetEvent();
	if (pEvent && pEvent->event)
	{
		AmaxRaceModeCheckpoint* pRaceMode = (AmaxRaceModeCheckpoint*)pEvent->event->raceMode;

		if (mTimeLeft >= GetDifficultyVars()->goldTimeRemaining)
			results.mRaceStatistics.mMedal = kSP_Gold;
		else if (mTimeLeft >= GetDifficultyVars()->silverTimeRemaining)
			results.mRaceStatistics.mMedal = kSP_Silver;
		else if (mTimeLeft >= 0.0f)
			results.mRaceStatistics.mMedal = kSP_Bronze;
		else
			results.mRaceStatistics.mMedal = kSP_Incomplete;

		const Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);
		if (pLogicActor)
		{
			results.mRaceStatistics.mFansEarned = pLogicActor->GetScoring().mFinalScore;
			results.mRaceStatistics.mFanDemandsComplete = pLogicActor->GetScoring().mFanDemandsComplete;
			results.mRaceStatistics.mGoldenFanDemandComplete = pLogicActor->GetScoring().mGoldenFanDemandComplete;
			results.mRaceStatistics.mGoldenFanDemandScore = pLogicActor->GetScoring().mGoldenFanDemandScore;
		}
	}

	C_RaceModeBase::SetupResults();

	results.Finished();

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_ResultsCalculated );
	mRaceInProgress=false;
}

//=================================================================================================

bool C_RaceModeCheckpoint::ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel)
{
	return false;
}

//=================================================================================================

bool C_RaceModeCheckpoint::ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel)
{
	return false;
}

} // namespace Game
