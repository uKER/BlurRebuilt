#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic/RaceModes/RaceModeBossBattle.h>

#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameLogic\RaceRules\RaceRulesManager.h>
#include <GameLogic\StickerManager.h>

#include <Game\Messaging\GameflowMessages.h>
#include <ui\Game\Gadgets\ButtonMessages.h>
#include "GameLogic/Messaging/GameLogicMessages.h"

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <RaceBook/RaceBookManager.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <ui\Game\Gadgets\ButtonMessages.h>
#include <ui\UiHelpers.h>

#include <GameLogic\profile\GameProfileManager.h>

#include <SinglePlayer\SinglePlayer.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>

#include <AI/AI_Amax.h>

#include <Automation/GameDataLogging.h>


//#pragma optimize("", off)					// TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP



namespace Game
{


//=================================================================================================
//							Initialise Routines
//=================================================================================================

void C_RaceModeBossBattle::Initialise(Game::C_LevelState *pLevelState)
{
	SetInternalNameAndID("BossBattle", E_RaceMode_BossBattle);

	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );

	SetLevelState(pLevelState);

	AddSubscriber(Game::E_VehicleMessage_FatallyDamaged);
//	AddSubscriber(ActorMsg::E_CutSceneMsg_Finished);
	AddSubscriber(Game::E_GameflowMessage_RaceFinished);
//	AddSubscriber(Game::E_GameLogicMessage_Passed_All_Rule);

	mEventState			= kSP_Incomplete;
}

//=================================================================================================

void C_RaceModeBossBattle::SetupRace()				// 1st - After race is selected
{
}

//=================================================================================================

void C_RaceModeBossBattle::StartLevel()				// 2nd - Before loading
{

	SP_BossEvent* pEvent = GetEvent();
	if (!pEvent || !pEvent->event || !pEvent->event->raceMode)
		return;

	AmaxRaceModeBossBattle* pRaceMode = (AmaxRaceModeBossBattle*)pEvent->event->raceMode;

	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	levelState.mWreckedBehaviour = E_AMAXWRECKED_DO_NOTHING;					// Don't respawn any bosses


/*
	mDistanceFromMinders		= pRaceMode->distanceFromMinders;
	mTimeInFrontOfMinders		= pRaceMode->timeInFrontOfMinders;
	mWreckMindersToWin			= pRaceMode->wreckMindersToWin;

	mDistanceFromBoss			= pRaceMode->distanceFromBoss;
	mTimeInFrontOfBoss			= pRaceMode->timeInFrontOfBoss;
	mWreckBossToWin				= pRaceMode->wreckBossToWin;


	mRaceInProgress = false;
	mFirstStage	= true;							// Boss race specific
	mNumberWrecked = 0;

	mIgnoreDistanceChecks = 0;


	Game::C_RaceLogicLayer* pLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	S32 num_cars = pLayer->GetNumVehicles();

	mPlayerIndex		= 0;						// Indexes should be standard but just in case we'll make them variable
	mFirstMinderIndex	= 1;
	mBossIndex			= num_cars - 1;
	mNumberMinders		= num_cars - 2;
*/

	Game::C_RaceLogicLayer* pLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	mNumBosses = pLayer->GetNumVehicles() - 1;

	C_RaceModeBase::StartLevel();

	mWon = false;
	mHasWonByWreck = false;
}

//=================================================================================================

void C_RaceModeBossBattle::StartRace()				// 3rd - On GO
{
//	S32		a;
	
	C_RaceModeBase::StartRace();
/*
	Game::C_RaceLogicLayer* pLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	if (mWreckMindersToWin)
	{
		for(a=mFirstMinderIndex;a<(mFirstMinderIndex + mNumberMinders);a++)		// Force all enemy vehicles to die once
			pLayer->GetLogicVehicleActor(a)->SetRemoveWhenWrecked(true);
	}
	if (mWreckBossToWin)
	{
		pLayer->GetLogicVehicleActor(mBossIndex)->SetRemoveWhenWrecked(true);
	}
	pLayer->GetLogicVehicleActor(mBossIndex)->SetAlwaysShieldState(true);		// Make Boss always shield
	SetControlType(mPlayerIndex, true);											// Make sure the player has control

	mTimeOutsideRange = 0.0f;
*/
	SetControlType(0, true);													// Make sure the player has control
}


//=================================================================================================
//							Update Routines
//=================================================================================================

void C_RaceModeBossBattle::Update()
{
//	S32		a;

//	SP_BossEvent* pEvent = GetEvent();
//	if (!pEvent)
//		return;

	if(!mRaceInProgress)
		return;


	Game::C_RaceLogicLayer* pLayer	= GetRaceLogicLayer();
	Game::C_LevelState* pLevelState = GetLevelState();
	for(S32 a=0;a<pLayer->GetNumVehicles();a++)
	{
		const Actors::C_VehicleLogicActor* pLogicActor	= pLayer->GetLogicVehicleActor(a);
		if( (pLevelState->mLaps > 0) && (pLogicActor && pLogicActor->GetLapInfo().mCurrentLap > (S8)pLevelState->mLaps) )
		{
			if (a == 0)
				EndEvent(true);					// Player Wins
			else
				EndEvent(false);				// Boss Wins
		}
	}


	if (mWon)
		mEventState = kSP_Gold;
	else
		mEventState = kSP_Incomplete;

/*
	if (mIgnoreDistanceChecks > 0)				// Skip logic if we've just repositioned the cars - STILL DOESN'T WORK BECAUSE OF GAME LOGIC
	{
		mIgnoreDistanceChecks--;
		return;
	}

	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	Game::C_RaceLogicLayer* pLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();


	if (mFirstStage)
	{
		bool ahead = true;

		for(a=mFirstMinderIndex;a<(mFirstMinderIndex + mNumberMinders);a++)					// Loop through minders
			if (levelState.mCars[a].mState == Game::E_VEHICLE_STATE_RACING)					// Have we got ahead of this minder
				if (GetVehicleGap(mPlayerIndex, a) < mDistanceFromMinders)
					ahead = false;

		if (ahead)
		{
			if (IncRangeTimer(mTimeInFrontOfMinders))
			{
				for(a=mFirstMinderIndex;a<(mFirstMinderIndex + mNumberMinders);a++)				// Loop through minders
					if (levelState.mCars[a].mState == Game::E_VEHICLE_STATE_RACING)				// Kill off the minders that haven't been wrecked
						pLayer->GetLogicVehicleActor(a)->RemoveFromRace();
				StartSecondStage();																// and start the next stage
			}
		}
		else
		{
			mTimeOutsideRange = 0.0f;
		}

	}
	else
	{
		F32 gap = GetVehicleGap(mPlayerIndex, mBossIndex);
		if (gap > mDistanceFromBoss)
		{
			if (IncRangeTimer(mTimeInFrontOfBoss))
				EndEvent(true);				// Way ahead of Boss
		}
		else if (gap < -mDistanceFromBoss)
		{
			if (IncRangeTimer(mTimeInFrontOfBoss))
				EndEvent(false);			// Way behind Boss
		}
		else
		{
			mTimeOutsideRange = 0.0f;
		}
	}

//	C_RaceRulesManager::Instance().Update();				// IS THIS REQUIRED?
*/
}


//=================================================================================================

void C_RaceModeBossBattle::UpdateHUD()
{
	/*
	if	(!mRaceInProgress)
		return;


	if (mFirstStage)
	{
		static Util::C_String messageStr;
		messageStr.Printf("Wreck the minions");
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(68.0f, 178.0f), messageStr, Maths::RGBA(0xff,0xff,0xff,0xff), 24.0f);
	}
	else
	{
		static Util::C_String distanceStr;
		F32 dist = GetVehicleGap(mPlayerIndex, mBossIndex);
		if (dist > 0.0f)
			distanceStr.Printf("Distance ahead of Boss : %f", dist);
		else
			distanceStr.Printf("Distance behind of Boss : %f", -dist);

		if (dist > mDistanceFromBoss)
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(68.0f, 178.0f), distanceStr, Maths::RGBA(0x00,0xff,0x00,0xff), 24.0f);
		if (dist < -mDistanceFromBoss)
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(68.0f, 178.0f), distanceStr, Maths::RGBA(0xff,0x00,0x00,0xff), 24.0f);
		else
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(68.0f, 178.0f), distanceStr, Maths::RGBA(0xff,0xff,0xff,0xff), 24.0f);
	}
	*/
}


//=================================================================================================

bool C_RaceModeBossBattle::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	if(!ShouldHandleMessages())
		return false;

	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case E_VehicleMessage_FatallyDamaged:
			{
				const Game::Actors::C_VehicleFatallyDamagedMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleFatallyDamagedMsg>(pMessage);

				if (pMsg->m_VehicleIndex == 0)
				{
					EndEvent(false);
				}
				else
				{
					mNumBosses--;					// Might be more than one boss
					if (mNumBosses == 0)
						EndEvent(true, true);
				}

/*
				if ((pMsg->m_VehicleIndex == mBossIndex) && (mWreckBossToWin))
				{
					EndEvent(true);
				}
				else if (((pMsg->m_VehicleIndex != mBossIndex) && (pMsg->m_VehicleIndex != mPlayerIndex)) && (mWreckMindersToWin))		// Minder killed
				{
					Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
					Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor *) pRLC->GetVehicleActor(pMsg->m_VehicleIndex);
					if(pActor)
					{
						mNumberWrecked++;
						if (mNumberWrecked >= mNumberMinders)
							StartSecondStage();
					}
				}
*/
			}
			break;
/*
		case ActorMsg::E_CutSceneMsg_Finished:
			{
				SetControlType(mPlayerIndex, true);			// Make sure player has control at end of mini cutscene
			}
			break;


		case Game::E_GameLogicMessage_Passed_All_Rule:
			{
				_UI_MESSAGE(E_GameflowMessage_RaceFinished);
				C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished);
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
			}
			break;
*/

		case Game::E_GameflowMessage_RaceFinished:
			{
				Game::Actors::RaceLogicCore* pRLC	= GetRLC();
				C_RaceLogicLayer* pLayer			= GetRaceLogicLayer();

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
/*
void C_RaceModeBossBattle::StartSecondStage()
{
	mFirstStage = false;

	RepositionOnStartPoint(mPlayerIndex, 0, 20.0f);			// Player on player position at 20mph
	RepositionOnStartPoint(mBossIndex, 3, 20.0f);			// Boss on Boss position
	mIgnoreDistanceChecks = 4;								// Disable logic until cars rest themselves
	mTimeOutsideRange = 0.0f;

	SetControlType(mPlayerIndex, false);						// Put player vehicle under AI control

	// Using PassedScriptedCamera because it shouldn't be used anymore by the cutscenes
	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_Start_PassedScriptedCamera );		// Start camera sequence

	Game::C_RaceLogicLayer* pLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	pLayer->GetLogicVehicleActor(mBossIndex)->SetAlwaysShieldState(false);				// Stop Boss using the shield all the time
}

//=================================================================================================

void C_RaceModeBossBattle::RepositionOnStartPoint(S32 vehicle_index, S32 start_pos_index, F32 speed)
{
	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	Game::Actors::C_VehicleActor* pVehicleActor = pLayer->GetLogicVehicleActor(vehicle_index)->GetGraphicalActor( vehicle_index );

	RLStartPosition startPos = pRLC->GetStartPosition(start_pos_index);
	Game::Actors::C_VehicleResetAtWorldPositionMsg resetMsg(pVehicleActor->GetHandle(), vehicle_index, startPos.mWorldMatrix, startPos.mWorldMatrix.AZ() * (speed * 0.44704f));			// mph -> m/s
	pVehicleActor->SendMessageToActor(pVehicleActor->GetHandle(), &resetMsg);
}
*/
//=================================================================================================

void C_RaceModeBossBattle::SetControlType(S32 vehicle_index, bool player_control)
{
	Game::Actors::RaceLogicCore* pRLC = GetRLC();
	Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor *) pRLC->GetVehicleActor(vehicle_index);
	if (player_control)
		pActor->ForcePlayerControl();
	else
		pActor->ForceAIControl();
}


//=================================================================================================
/*
F32 C_RaceModeBossBattle::GetVehicleGap(S32 vehicle_index1, S32 vehicle_index2)
{
	Game::C_RaceLogicLayer* pLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

	F32 pos1 = pLayer->GetLogicVehicleActor(vehicle_index1)->GetPositionInfo().mLogicPosition;
	F32 pos2 = pLayer->GetLogicVehicleActor(vehicle_index2)->GetPositionInfo().mLogicPosition;
	
//	return(AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(pos1, pos2));
	return(AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(pos2, pos1));
}
*/

//=================================================================================================

SP_BossEvent* C_RaceModeBossBattle::GetEvent()
{
	Game::C_LevelState* pLevelState = GetLevelState();

	SP_Event* pBaseEvent = SP_GetEventById( SP_GetCurrentSaveGame(), pLevelState->mSpEventId );
	
	BIZ_ASSERT(pBaseEvent);

	if (!pBaseEvent)		
		return NULL;
	
	if (pBaseEvent->kind != kSP_BossEvent)
		return NULL;
	
	return (SP_BossEvent*)pBaseEvent;
}

//=================================================================================================
/*
bool C_RaceModeBossBattle::IncRangeTimer(F32 maxtime)
{
	mTimeOutsideRange += C_GameApp::GetInstance()->GetTimeStep();
	if (mTimeOutsideRange > maxtime)
		return(true);
	else
		return(false);
}
*/
//=================================================================================================
//							End Level Routines
//=================================================================================================


void C_RaceModeBossBattle::EndEvent(bool won, bool hasWonByWreck)
{
	Game::C_LevelState* pLevelState = GetLevelState();

	mRaceInProgress = false;
	mWon = won;
	mHasWonByWreck = hasWonByWreck;

	if (mWon)
		mEventState = kSP_Gold;
	else
		mEventState = kSP_Incomplete;

	if (won)
	{
		_UI_MESSAGE(E_GameflowMessage_RaceFinished);

#ifndef CFG_GOLD
		//Log which bosses we killed/didn't kill
		for(U8 i = 1; i < pLevelState->mNumCars; i++)
		{
			const Actors::C_VehicleLogicActor* pLogicActor	= GetRaceLogicLayer()->GetLogicVehicleActor(i);
			AmaxDataLogging::LogBossEventFinished(pLevelState->mCars[i].mNameAboveCar.AsString(), pLogicActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED);
		}
#endif
	}
	else
	{
		_UI_MESSAGE(E_GameflowMessage_Failed);
	}

	if(won)
	{
		if( SP_BossEvent* pEvent = GetEvent() )
		{
			if( SP_Tier* pTier = pEvent->tier )
			{
				Game::C_StickerManager::Instance().CheckBossBeaten(	pTier->index );
			}
		}
	}

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished);
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
}

//=================================================================================================

void C_RaceModeBossBattle::SetupResults()
{
	Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	Game::C_LevelState *pState=GetLevelState();
	
	results.Clear();
	results.mNumCars=pState->mNumCars;

	for(unsigned int a=0;a<pState->mNumCars;a++)
	{
		const Actors::C_VehicleLogicActor* pLogicActor	= GetRaceLogicLayer()->GetLogicVehicleActor(a);
		if(a==0)
		{
			results.mRaceStatistics.mTotalRaceTime=pLogicActor->GetStatistics().mTotalRaceTime;
		}
	}

	SP_BossEvent* pEvent = GetEvent();
	if (pEvent && pEvent->event)
	{
		AmaxRaceModeCheckpoint* pRaceMode = (AmaxRaceModeCheckpoint*)pEvent->event->raceMode;

		if (mWon)
			results.mRaceStatistics.mMedal = kSP_Gold;
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

/*
// For UI testing only
void C_RaceModeBossBattle::SetupFakeResults()
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

void C_RaceModeBossBattle::SetupQuitAndWinResults(lua_State *pState)
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

	S8 pos;
	Lua::GetDataMember(pState,"Position",pos,true);
	results.mCars[0].mPosition=pos;

	int score;
	Lua::GetDataMember(pState,"XP",score,true);
	results.mCars[0].mScore=score;

	int size = results.mCars[0].mPassedRule.Size();

	for(int r=0;r<C_RaceRulesManager::Instance().GetNumPassRules();r++)
	{
		int passed;
		char temp[32];

		sprintf(temp,"Rule%d",r+1);

		Lua::GetDataMember(pState,temp,passed,true);
		results.mCars[0].mPassedRule.PushBack(passed ? true : false);
	}

	for(int r=0;r<C_RaceRulesManager::Instance().GetNumFailRules();r++)
	{
		results.mCars[0].mFailedRule.PushBack(false);
	}

	C_RaceModeBase::SetupResults();

	results.Finished();

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_QuitAndWinRace );
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_ResultsCalculated );

	mRaceInProgress=false;
}
*/


} // namespace Game
