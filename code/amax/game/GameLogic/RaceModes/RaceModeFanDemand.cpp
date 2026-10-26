#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic/RaceModes/RaceModeFanDemand.h>

#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameLogic\RaceRules\RaceRulesManager.h>

#include <Game\Messaging\GameflowMessages.h>
#include <ui\Game\Gadgets\ButtonMessages.h>
#include "GameLogic/Messaging/GameLogicMessages.h"

#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>
#include <Network/Player/NetPlayer.h>
#include <Network/Player/NetPlayerManager.h>
#include <Network/Message/NetMessage.h>
#include <Network/Misc/NetDatagramPacketTypes.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <RaceBook/RaceBookManager.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <ui\Game\Gadgets\ButtonMessages.h>
#include <ui\UiHelpers.h>
#include <Ui/UiInterface.h>
#include <Ui/SoundEffects.h>

#include <GameLogic\ContentServer\KeyArchiveManager.h>

#include <GameLogic\profile\GameProfileManager.h>

#include <SinglePlayer\SinglePlayer.h>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

extern Debug::Float g_debugNetworkLeftRaceGhostTime;

namespace Game
{

void C_RaceModeFanDemand::Initialise(Game::C_LevelState *pLevelState)
{
	SetInternalNameAndID("FanDemand",E_RaceMode_FanDemand);

	SetLevelState(pLevelState);

	mRaceInProgress=false;
	
	mPassed = false;

	mEventState = kSP_Incomplete;
	mTimeLeft = 0.0f;
	mNumFans = 0;
	mDiffToMedal = 0;
	mNumDemandsCompleted = 0;

	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	AddSubscriber( E_GameLogicMessage_Scoring_Event );
	AddSubscriber( E_GameLogicMessage_Failed_Rule );
	AddSubscriber( E_GameLogicMessage_Passed_All_Rule );

	AddSubscriber( E_GameflowMessage_IntroStarted );
	AddSubscriber( E_GameflowMessage_OutroStarted );
	AddSubscriber( E_GameflowMessage_RaceStarted );
	AddSubscriber( E_GameflowMessage_RaceFinished );
}

SP_FanEvent* C_RaceModeFanDemand::GetEvent()
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	SP_Event* pBaseEvent = SP_GetEventById( SP_GetCurrentSaveGame(), levelState.mSpEventId );
	
	BIZ_ASSERT(pBaseEvent);

	if (!pBaseEvent)		
		return NULL;
	
	if (pBaseEvent->kind != kSP_FanEvent)
		return NULL;
		
	return (SP_FanEvent*)pBaseEvent;
}

void C_RaceModeFanDemand::SetupRace()
{
	SP_FanEvent* pEvent = GetEvent();

	if (!pEvent)
		return;

	mRaceInProgress=false;
	
	mPassed = false;

	mEventState = kSP_Incomplete;
	mTimeLeft = Game::C_Game::GetInstance()->GetLevelState().mTimeLimit;
	mNumFans = 0;

	if (AmaxRaceModeFanDemandVars* diffvars = GetDifficultyVars())
		mDiffToMedal = diffvars->BronzeNumToComplete;

	mNumDemandsCompleted = 0;

	C_Game::GetInstance()->GetLevelState().mStartOpponentsDisabled = true;		// Hide all opponents at start of race
}

void C_RaceModeFanDemand::Update()
{
	if(!mRaceInProgress)
		return;

	SP_FanEvent* pEvent = GetEvent();
	if (!pEvent)
		return;

	AmaxRaceModeFanDemandVars*	diffvars = GetDifficultyVars();				// Gets pointer to Gold/Silver/Bronze values based on difficulty 


	const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(0);
	if (pLogicActor)
	{
		mNumFans = pLogicActor->GetScoring().mFinalScoreForUI;
		mNumDemandsCompleted = pLogicActor->GetScoring().mFanDemandsComplete;
	}
	else
	{
		mNumFans = 0;
		mNumDemandsCompleted = 0;
	}
	mPassed = mNumDemandsCompleted >= diffvars->BronzeNumToComplete;

	CheckRaceEnd();


//	if (mNumFans >= pEvent->goldFanPar)
	if (mNumDemandsCompleted >= diffvars->GoldNumToComplete)
	{
		mEventState = kSP_Gold;
		mDiffToMedal = 0;
	}
	else if (mNumDemandsCompleted >= diffvars->SilverNumToComplete)
	{
		mEventState = kSP_Silver;
		mDiffToMedal = diffvars->GoldNumToComplete;
	}
	else if (mNumDemandsCompleted >= diffvars->BronzeNumToComplete)
	{
		mEventState = kSP_Bronze;
		mDiffToMedal = diffvars->SilverNumToComplete;
	}
	else
	{
		mEventState = kSP_Incomplete;
		mDiffToMedal = diffvars->BronzeNumToComplete;
	}

	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() &&
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore())
	{
		mTimeLeft = Game::C_Game::GetInstance()->GetLevelState().mTimeLimit - C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetRaceTime();
	}
}

void C_RaceModeFanDemand::CheckRaceEnd()
{
	C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	bool trigger = false;

	const Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);
	
	if (levelState.mTimeLimit > 0.0f) 
	{
		const Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		if (pRLC && levelState.mTimeLimit - pRLC->GetRaceTime() <= 0.0f)
			trigger = true;
	}
	else if (pLogicActor && (U32)pLogicActor->GetLapInfo().mCurrentLap > levelState.mLaps)
	{
		trigger = true;
	}

	if (trigger)
	{
		if (mPassed)
			C_RaceRulesManager::Instance().SendPassMessage(0);
		else
			C_RaceRulesManager::Instance().SendFailMessage(0);

		mRaceInProgress = false;
	}
}

void C_RaceModeFanDemand::StartLevel()
{
	C_RaceModeBase::StartLevel();
}

void C_RaceModeFanDemand::StartRace()
{
	C_RaceModeBase::StartRace();
}

void C_RaceModeFanDemand::UpdateHUD()
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


const UI::C_WideString512 C_RaceModeFanDemand::GetScoreString()
{
	UI::C_WideString512 str;
	UI::C_WideString64 str2;

	str.Clear();

	return str;
}

const UI::C_WideString512 C_RaceModeFanDemand::GetRulePassedString(U32 idx)
{
	UI::C_WideString512 str;

	if(mRulePassedTimer)
		C_RaceRulesManager::Instance().GetPassRuleString(idx, str, mRuleIndex, Game::RACE_MODE_RULE_STRING_TYPE_RESULTS);
	else
		str.Clear();

	return str;
}

const UI::C_WideString512 C_RaceModeFanDemand::GetRuleFailedString(U32 idx)
{
	UI::C_WideString512 str;

	if(mRulePassedTimer)
		C_RaceRulesManager::Instance().GetFailRuleString(idx, str, mRuleIndex, Game::RACE_MODE_RULE_STRING_TYPE_RESULTS);
	else
		str.Clear();

	return str;
}

bool C_RaceModeFanDemand::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	if(!ShouldHandleMessages())
		return false;

	bool handledMessage = true;

	switch( pMessage->Type() )
	{
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
					pLayer->GetLogicVehicleActor(pMsg->mVehicleIndex)->PreRemoveFromRace(g_debugNetworkLeftRaceGhostTime);
				}
			}
			break;

		case E_VehicleMessage_FatallyDamaged:
			{
				const Game::Actors::C_VehicleFatallyDamagedMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleFatallyDamagedMsg>(pMessage);
				if( pMsg->m_VehicleIndex == 0)
				{
#ifndef CFG_GOLD
					if(C_ReplayDirector::Instance().GetReplayMode() != Game::E_ReplayMode_Play)
#endif
					{
						_UI_MESSAGE1(E_GameflowMessage_Wrecked, UI::GetLocalPlayerIndex(pMsg->m_VehicleIndex));
						C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished );
						C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
					}
				}

				Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
				levelState.mCars[pMsg->m_VehicleIndex].mState = E_VEHICLE_STATE_FATALLY_DAMAGED;
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

		case Game::E_GameflowMessage_IntroStarted:
		case Game::E_GameflowMessage_OutroStarted:
			{
				Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor *) pRLC->GetVehicleActor(0);
				if(pActor)
					pActor->ForceAIControl();
			}
			break;

		case Game::E_GameflowMessage_RaceStarted:
			{
#ifndef CFG_GOLD
				//	Very quick and dirty fix for player control behing forced in replays!
				if(C_ReplayDirector::Instance().GetReplayMode() == E_ReplayMode_Play ||
					C_ReplayDirector::Instance().GetReplayMode() == E_ReplayMode_PrePlay)
					break;
#endif
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

// For UI testing only
void C_RaceModeFanDemand::SetupFakeResults()
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

void C_RaceModeFanDemand::SetupQuitAndWinResults(lua_State *pState)
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

	SP_FanEvent* pEvent = GetEvent();

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

void C_RaceModeFanDemand::SetupResults()
{
	Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	Game::C_LevelState& state=C_Game::GetInstance()->GetLevelState();
	
	results.Clear();
	results.mNumCars=state.mNumCars;

	for(unsigned int a=0;a<state.mNumCars;a++)
	{
		const Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(a);
		if(a==0)
		{
			results.mRaceStatistics.mTotalRaceTime=pLogicActor->GetStatistics().mTotalRaceTime;
		}
	}


	SP_FanEvent* pEvent = GetEvent();
	if (pEvent && pEvent->event)
	{

		AmaxRaceModeFanDemandVars*	diffvars = GetDifficultyVars();				// Gets pointer to Gold/Silver/Bronze values based on difficulty 
		const Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);
		if (pLogicActor)
		{
			mNumDemandsCompleted = pLogicActor->GetScoring().mFanDemandsComplete;
			if (mNumDemandsCompleted >= diffvars->GoldNumToComplete)			// pEvent->goldFanPar
				results.mRaceStatistics.mMedal = kSP_Gold;
			else if (mNumDemandsCompleted >= diffvars->SilverNumToComplete)	// pEvent->silverFanPar
				results.mRaceStatistics.mMedal = kSP_Silver;
			else if (mNumDemandsCompleted >= diffvars->BronzeNumToComplete)	// pEvent->bronzeFanPar
				results.mRaceStatistics.mMedal = kSP_Bronze;
			else
				results.mRaceStatistics.mMedal = kSP_Incomplete;
			
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

bool C_RaceModeFanDemand::ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel)
{
	return false;
}

bool C_RaceModeFanDemand::ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel)
{
	return false;
}

//=================================================================================================

AmaxRaceModeFanDemandVars*	C_RaceModeFanDemand::GetDifficultyVars()
{
	SP_FanEvent* pEvent = GetEvent();
	if (!pEvent || !pEvent->event || !pEvent->event->raceMode)
		return(NULL);

	AmaxRaceModeFanDemand* pRaceMode = (AmaxRaceModeFanDemand*)pEvent->event->raceMode;

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


} // namespace Game
