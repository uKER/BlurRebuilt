#include "stdafx.h"

#include <GameLogic\RaceRules\RaceRulesManager.h>
#include <RaceBook\Data\RaceBookData.h>

#include <Game/Game.h>
#include <GameLogic\RaceLogicCore\Core\RaceLogicCore.h>
#include <GameLogic\Layers\AmaxWorldLayer.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Profile/GameProfile.h>
#include <GameLogic/Profile/GameProfileManager.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic\RaceRules\RaceAction_EliminateLastDriver.h>
#include <GameLogic\RaceRules\RaceAction_TimeAttackDisplay.h>
#include <GameLogic\RaceRules\RaceAction_ManageGhosts.h>

Debug::Logger &g_RaceRulesManagerLogger = Debug::GetLogger("GameLogic.RaceMode.RulesManager");

namespace Game
{

C_RaceRulesManager C_RaceRulesManager::s_instance;

void C_RaceRulesManager::Init()
{
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	AddSubscriber( E_GameLogicMessage_LapIncremented );
	AddSubscriber( E_VehicleMessage_FatallyDamaged );

	mNumEliminated = 0;
};

bool C_RaceRulesManager::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
	case E_GameLogicMessage_LapIncremented:
		{
			const C_LapIncrementedMsg* pMsg = Messaging::MessageCast<C_LapIncrementedMsg>(pMessage);

			if(pMsg->mLap > 1)
			{
				Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				for(int a = 0; a < mRaceLapActions.Size(); a++)
				{
					if( !pLayer->GetLogicVehicleActor(pMsg->mVehicleIndex)->GetGraphicalActor(pMsg->mVehicleIndex)->IsEliminated() )
						mRaceLapActions[a]->PerformAction(pMsg->mVehicleIndex);
				}
			}

			break;
		}

	case E_VehicleMessage_FatallyDamaged:
		{
			mNumEliminated++;
		}
		break;


	default:
		handledMessage = false;
		break;
	}

	return handledMessage;
}

void C_RaceRulesManager::ClearAllRules()
{
	for( int i = 0; i < mRaceRulesPass.Size(); i++ )
		delete(mRaceRulesPass[i]);

	for( int i = 0; i < mRaceRulesFail.Size(); i++ )
		delete(mRaceRulesFail[i]);

	for( int i = 0; i < mRaceLapActions.Size(); i++ )
		delete(mRaceLapActions[i]);

	for( int i = 0; i < mRaceTimeActions.Size(); i++ )
		delete(mRaceTimeActions[i]);

	mRaceRulesPass.Clear();
	mRaceRulesFail.Clear();
	mRaceLapActions.Clear();
	mRaceTimeActions.Clear();

	mCurrentRule = -1;
}

void C_RaceRulesManager::InitRulesFromEvent(int numCars, C_LevelResult& levelResult, AmaxEvent* ev)
{
	GAME_DIFFICULTY difficulty = (GAME_DIFFICULTY)C_GameProfileManager::Instance().GetPrimaryProfile()->GetDifficulty();

	DifficultySettings* difficultySettings = NULL;

	switch(difficulty)
	{
	case GAME_DIFFICULTY_VERY_EASY:
	case GAME_DIFFICULTY_EASY:
		difficultySettings = &ev->easyVersion;
		break;

	case GAME_DIFFICULTY_NORMAL:
		difficultySettings = &ev->normalVersion;
		break;

	case GAME_DIFFICULTY_HARD:
	case GAME_DIFFICULTY_HARDCORE:
		difficultySettings = &ev->hardVersion;
		break;


	default:
		BIZ_ASSERT_FAIL("difficulty is INVALID, defaulting to GAME_DIFFICULTY_NORMAL");
		difficultySettings = &ev->normalVersion;
		break;
	}

	BIZ_ASSERT(difficultySettings);

	mRuleStrOverride = difficultySettings->ruleStrOverride.id;

	mRuleSeqStrOverride.Clear();
	for(int a=0;a<difficultySettings->ruleSeqStrOverride.count;a++)
	{
		mRuleSeqStrOverride.PushBack(difficultySettings->ruleSeqStrOverride[a].id);
	}

	for(int i = 0; i < numCars; i++ )
	{
		for(int b = 0; b < difficultySettings->passRules.count; b++)
			levelResult.mCars[i].mPassedRule.PushBack(false);

		for(int b = 0; b < difficultySettings->failRules.count; b++)
			levelResult.mCars[i].mFailedRule.PushBack(false);
	}

	for(int b = 0; b < difficultySettings->passRules.count; b++)
	{
		RaceModeRuleBase* raceRule = difficultySettings->passRules[b]->CreateRule();
		AddPassRule(raceRule, b);
	}

	for(int b = 0; b < difficultySettings->failRules.count; b++)
	{
		RaceModeRuleBase* raceRule = difficultySettings->failRules[b]->CreateRule();
		AddFailRule(raceRule, b);
	}

	for(int b = 0; b < difficultySettings->lapActions.count; b++)
	{
		RaceActionBase* action = difficultySettings->lapActions[b]->CreateAction();
		AddLapAction(action);
	}
	for(int b = 0; b < difficultySettings->timeActions.count; b++)
	{
		RaceActionBase* action = difficultySettings->timeActions[b]->CreateAction();
		AddTimeAction(action);
	}
	
	SetInstaPass(difficultySettings->instaPass);
	SetInstaFail(difficultySettings->instaFail);

	mSequentialRuleSet=false;
	if(difficultySettings->sequentialRules)
	{
		mSequentialRuleSet=true;
		mCurrentRule = 0;
	}
}

RaceModeRuleBase* C_RaceRulesManager::GetRaceRulePass( S32 i )
{
	if( mCurrentRule > -1 && mCurrentRule < mRaceRulesPass.Size() ) 
		return mRaceRulesPass[mCurrentRule];

	if( i >= 0 && i < mRaceRulesPass.Size() )
		return mRaceRulesPass[i];

	return NULL;
}

RaceActionBase* C_RaceRulesManager::GetTimeAction( S32 i )
{ 
	//Check we're asking for a valid time action
	if( i >= 0 && i < mRaceTimeActions.Size() )
	{
		return mRaceTimeActions[i];
	}
	else
	{
		return (RaceActionBase*)NULL;
	}
}

void C_RaceRulesManager::Update()
{
	Game::C_LevelState&		levelState	  			= C_Game::GetInstance()->GetLevelState();
	Game::C_LevelResult&	levelResult   			= C_Game::GetInstance()->GetLevelResult();
	Game::C_RaceLogicLayer* pLayer		  			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	Actors::RaceLogicCore*  pRLC		  			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	Game::C_GameModeBase*	pGameModeBase 			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	Game::Actors::C_VehicleLogicActor* pLogicActor	= NULL;
	Game::Actors::C_VehicleActor* pGraphicalActor	= NULL;

	enum E_VEHICLE_CONTROL_TYPE eControlType		= E_VEHICLE_CONTROL_TYPE_UNKNOWN;
	enum E_VEHICLE_STATE eState						= E_VEHICLE_STATE_UNKNOWN;
	bool eliminated									= false;
	bool wrecked									= false;


	for( int i = 0; i < mRaceRulesPass.Size(); i++ )
	{
		mRaceRulesPass[i]->PreCache();
		mRaceRulesPass[i]->Update();
	}

	// Note: Lap actions are dealt with automatically through messaging.

	bool allPassed;
	for(unsigned int vehIdx = 0; vehIdx < levelState.mNumCars; vehIdx++ )
	{
		//	Don't process any raceRule logic if the vehicle in question has already finished the race
		eControlType = levelState.mCars[vehIdx].mControlType;
		eState = levelState.mCars[vehIdx].mState;

		if(eState == E_VEHICLE_STATE_FINISHED || eControlType == E_VEHICLE_CONTROL_TYPE_NETWORK)
			continue;

		// Don't run race rules on the AI or we get funny behaviour. We don't need this any more anyway ...
		if(pRLC->IsVehicleAIControlled(vehIdx))
			continue;

		pLogicActor = pLayer->GetLogicVehicleActor(vehIdx);
		pGraphicalActor = pLogicActor->GetGraphicalActor(vehIdx);
		VehicleRaceStatistics& statistics = pLogicActor->GetStatistics();

		eliminated = pGraphicalActor->IsEliminated();
		wrecked = statistics.mWrecked;
			
		if(!pLogicActor->GetStillInRace())
			continue;

		if(wrecked)
		{
			SendFailMessage(vehIdx);
			continue;
		}

		allPassed = true;

		if( pLayer->GetNumVehicles() > 1 && pLayer->GetNumLiveVehicles() == 1 && !wrecked && !eliminated )
		{
			LevelResultCar* car = &levelResult.mCars[vehIdx];

			//Split screen logic for players racing with no AIs
			if(pGameModeBase && pGameModeBase->GetID() == Game::E_GameMode_SplitScreen && 
				eControlType == E_VEHICLE_CONTROL_TYPE_PLAYER && levelState.mNumViewports == levelState.mNumCars)
			{
				bool lastPlayerRemaining = true;
				//Make sure there aren't any other living players
				for(unsigned int veh = 0; veh < levelState.mNumCars; veh++ )
				{
					if(veh != vehIdx)
					{
						if(!wrecked && !eliminated)
						{
							lastPlayerRemaining = false;
							break;
						}
						else if(pLogicActor->GetPositionInfo().mLogicPositionCombined 
							< pLayer->GetLogicVehicleActor(veh)->GetPositionInfo().mLogicPositionCombined)
						{
							lastPlayerRemaining = false;
							break;
						}
					}
				}
				if(lastPlayerRemaining)
				{
					//If not wrecked or eliminated
					if(!wrecked && !eliminated)
					{
						for( int a = 0; a < mRaceRulesPass.Size(); a++ )
						{
							levelResult.mCars[vehIdx].mPassedRule[a] = true;
						}
						SendPassMessage(vehIdx);
						continue;
					}
				}
			}
			else
			{
				for( int a = 0; a < mRaceRulesPass.Size(); a++ )
					levelResult.mCars[vehIdx].mPassedRule[a] = true;

				SendPassMessage(vehIdx);
				continue;
			}
		}

		// Check pass rules
		if( levelResult.mCars[vehIdx].mPassedRule.Size() > 0 )
		{
			// Oh god this hurts me.
			if(mCurrentRule > -1 && mCurrentRule < mRaceRulesPass.Size())
			{
				if ( mRaceRulesPass[mCurrentRule]->IsTriggered(vehIdx) )
				{
					pLayer->MidGamePause();

					levelResult.mCars[vehIdx].mPassedRule[mCurrentRule] = true;
					mCurrentRule++;

					if(mCurrentRule >= mRaceRulesPass.Size())
					{
						mCurrentRule=mRaceRulesPass.Size()-1;
					}

					if(mCurrentRule < mRaceRulesPass.Size())
						allPassed = false;
				}
				else
				{
					levelResult.mCars[vehIdx].mPassedRule[mCurrentRule] = false;
					allPassed = false;
				}
			}
			else
			{
				for( int a = 0; a < mRaceRulesPass.Size(); a++ )
				{
					if( !levelResult.mCars[vehIdx].mPassedRule[a] || !mRaceRulesPass[a]->IsPerisistent() )
					{
						if ( mRaceRulesPass[a]->IsTriggered(vehIdx) )
							levelResult.mCars[vehIdx].mPassedRule[a] = true;
						else
						{
							levelResult.mCars[vehIdx].mPassedRule[a] = false;
							allPassed = false;
						}
					}
				}
			}

			// Check triggers that pass instantly on success
			if(allPassed && mInstaPass)
			{
				SendPassMessage(vehIdx);
				continue;
			}
		}

		// Check fail rules
		bool allFailed = true;
		if( levelResult.mCars[vehIdx].mFailedRule.Size() > 0 )
		{
			for( int a = 0; a < mRaceRulesFail.Size(); a++ )
			{
				if( !levelResult.mCars[vehIdx].mFailedRule[a] || !mRaceRulesFail[a]->IsPerisistent() )
				{
					if ( mRaceRulesFail[a]->IsTriggered(vehIdx) )
					{
						if(mInstaFail)
						{
							levelResult.mCars[vehIdx].mFailedRule[a] = true;
							SendFailMessage(vehIdx);
							break;
						}
					}
					else
						allFailed = false;
				}
			}
		}

		// We might have missed the earlier catch, but still made it here. So we check it again for safety.
		if(eState == E_VEHICLE_STATE_FINISHED || eControlType == E_VEHICLE_CONTROL_TYPE_NETWORK)
			continue;

		// Check number of laps + time limit expiry
		if( ( levelState.mLaps > 0      && pLogicActor && pLogicActor->GetLapInfo().mCurrentLap > (S8) levelState.mLaps ) ||
			( levelState.mTimeLimit > 0 && pRLC && pRLC->GetRaceTime() > levelState.mTimeLimit ) )
		{
			if( allFailed && mRaceRulesFail.Size() )
			{
				SendFailMessage(vehIdx);
				continue;
			}
			else if( allPassed || !mRaceRulesPass.Size() )
			{
				SendPassMessage(vehIdx);
				continue;
			}
			else
			{
				SendFailMessage(vehIdx);
				continue;
			}
		}
	}


	// Perform timed actions
	float frameTime = C_GameApp::GetInstance()->GetTimeStep();
	for( int i = 0; i < mRaceTimeActions.Size(); i++ )
	{
		mRaceTimeActions[i]->mCurrentTimer -= frameTime;
		mRaceTimeActions[i]->Update(frameTime);
		if( mRaceTimeActions[i]->mCurrentTimer <= 0 )
		{
			mRaceTimeActions[i]->Init();
			for( U32 vehIdx = 0; vehIdx < levelState.mNumCars; vehIdx++ )
			{
				if( !pLayer->GetLogicVehicleActor(vehIdx)->GetGraphicalActor(vehIdx)->IsEliminated() )
					mRaceTimeActions[i]->PerformAction(vehIdx);
			}

			mRaceTimeActions[i]->mCurrentTimer = mRaceTimeActions[i]->mTimer;
		}
	}
}

void C_RaceRulesManager::SendPassMessage(int vehIdx)
{
	C_PassedAllRuleMsg passedAllRuleMsg( vehIdx );
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&passedAllRuleMsg);
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &passedAllRuleMsg );

	Game::C_RaceLogicLayer* pLayer		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	//pLayer->GetLogicVehicleActor(vehIdx)->mStillInRace = false;
	pLayer->GetLogicVehicleActor(vehIdx)->SetStillInRace(false);
}

void C_RaceRulesManager::SendFailMessage(int vehIdx)
{
	C_FailedRuleMsg failedMsg( vehIdx );
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&failedMsg);
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &failedMsg );

	Game::C_RaceLogicLayer* pLayer		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	//pLayer->GetLogicVehicleActor(vehIdx)->mStillInRace = false;
	pLayer->GetLogicVehicleActor(vehIdx)->SetStillInRace(false);
}

bool C_RaceRulesManager::ValidPassRules()
{
	for( int a = 0; a < mRaceRulesPass.Size(); a++ )
		if( mRaceRulesPass[a]->IsValid() )
			return true;

	for( int a = 0; a < mRaceRulesFail.Size(); a++ )
		if( mRaceRulesFail[a]->IsValid() )
			return true;

	return false;
}

bool C_RaceRulesManager::TrackVehicle(int idx)
{
	C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	for( int a = 0; a < mRaceRulesPass.Size(); a++ )
	{
		if( mRaceRulesPass[a]->TrackVehicle(levelState.mCars[idx].mCharacterID) )
			return true;
	}

	return false;
}

void C_RaceRulesManager::GetRuleStringOverride( UI::C_WideString512& str)
{
	if(mRuleStrOverride.IsValid())
	{
		if(mSequentialRuleSet)
		{
			str.Format(L"%s", mRuleSeqStrOverride[mCurrentRule].String());
		}
		else
		{
			str.Format(L"%s", mRuleStrOverride.String());
		}
	}
}

const void C_RaceRulesManager::GetPassRuleString(U32 idx, UI::C_WideString512& str, int rule, RACE_MODE_RULE_STRING_TYPE strType )
{
	if (rule < 0 || rule >= mRaceRulesPass.Size())
	{
		str.Format(L"<ERROR>");
		return;
	}

	mRaceRulesPass[rule]->GetRuleString(idx, str, strType, true); 
}

const void C_RaceRulesManager::GetFailRuleString(U32 idx, UI::C_WideString512& str, int rule, RACE_MODE_RULE_STRING_TYPE strType )
{ 
	if (rule < 0 || rule >= mRaceRulesFail.Size())
	{
		str.Format(L"<ERROR>");
		return;
	}

	mRaceRulesFail[rule]->GetRuleString(idx, str, strType, false);
}

void C_RaceRulesManager::AddPassRule(RaceModeRuleBase *r,int index)
{
	r->Init(index);
	assert(r);
	mRaceRulesPass.PushBack(r);
}

void C_RaceRulesManager::AddFailRule(RaceModeRuleBase *r,int index)
{
	r->Init(index);
	assert(r);
	mRaceRulesFail.PushBack(r);
}

void C_RaceRulesManager::AddLapAction(RaceActionBase* act)
{
	if(act)
		mRaceLapActions.PushBack(act);
}

void C_RaceRulesManager::AddTimeAction(RaceActionBase* act)
{
	if(act)
		mRaceTimeActions.PushBack(act);
}

bool C_RaceRulesManager::HasPassed(int vehicleIndex, int ruleIdx, bool pass)
{
	if(pass)
		return mRaceRulesPass[ruleIdx]->IsTriggered(vehicleIndex);

	return mRaceRulesFail[ruleIdx]->IsTriggered(vehicleIndex);
}

} // namespace Game
