#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic/RaceModes/RaceModeStreetRace.h>

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

#include <GameLogic\ContentServer\KeyArchiveManager.h>

#include <GameLogic\profile\GameProfileManager.h>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

extern Debug::Float g_debugEliminatedHideTimer;

namespace Game
{

void C_RaceModeStreetRace::Initialise(Game::C_LevelState *pLevelState)
{
	SetInternalNameAndID("StreetRace",E_RaceMode_StreetRace);
	mRaceInProgress=false;

	SetLevelState(pLevelState);

	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	AddSubscriber( E_GameLogicMessage_Scoring_Event );
	AddSubscriber( E_GameLogicMessage_Failed_Rule );
	AddSubscriber( E_GameLogicMessage_Passed_All_Rule );

	AddSubscriber( E_GameflowMessage_IntroStarted );
	AddSubscriber( E_GameflowMessage_OutroStarted );
	AddSubscriber( E_GameflowMessage_RaceStarted );
	AddSubscriber( E_GameflowMessage_RaceFinished );

	//AddSubscriber( E_VehicleLogicMsg_FiredPerk );
	//AddSubscriber( E_VehicleMessage_FatallyDamaged );

	mEventState			= kSP_Incomplete;
}

void C_RaceModeStreetRace::Update()
{
	if(!mRaceInProgress)
		return;

	if (const Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0))
	{
		int position = pLogicActor->GetPositionInfo().mRacePosition;

		if (position <= 1)
			mEventState = kSP_Gold;
		else if (position <= 2)
			mEventState = kSP_Silver;
		else if (position <= 3)
			mEventState = kSP_Bronze;
		else
			mEventState = kSP_Incomplete;
	}

	C_RaceRulesManager::Instance().Update();

	// ---------------------------------------------------------
	// IMPORTANT!
	// The RaceRules should deal with all this now, rather than
	// this multiplayer hack.
	// ---------------------------------------------------------

	//NetVehicleIndexStack localList;
	//GetLocalPlayerList(localList);

	//if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetID() != Game::E_GameMode_Debug)
	//{
	//	if(RaceBook::C_RaceBookManager::Instance().GetLookingAtGroup())
	//	{
	//		RaceBookGroup *pGroup = C_RaceBookData::Instance().mGroups[(GAMEDATA_ID)RaceBook::C_RaceBookManager::Instance().GetLookingAtGroup()->GetID()];
	//		switch (pGroup->multiplayer.win_by)
	//		{
	//		case E_RACEBOOK_MULTIPLAYER_WIN_BY_POSITION:
	//			UpdateForPosition(localList);
	//			break;

	//		case E_RACEBOOK_MULTIPLAYER_WIN_BY_LAST_MAN:
	//			UpdateForLastManStanding(localList);
	//			break;
	//		};
	//	}
	//	else
	//	{
	//		//must be through debug
	//		UpdateForPosition(localList);
	//	}
	//}
}

//void C_RaceModeStreetRace::UpdateForPosition(NetVehicleIndexStack &localList)
//{
//	// -----------------------------------------------------------
//	// IMPORTANT!
//	// The RaceRulesManager deals with this now, so network games
//	// will have to handle the finished message accordingly rather
//	// than use this.
//	// -----------------------------------------------------------
//
//	//Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
//	//if( !pRLC )
//	//	assert(!"Race Logic Core does not exist");
//
//	//Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
//
//	/*for (U8 i=0; i<localList.Size(); i++)
//	{
//		// Completed the required number of laps?
//		if(pRLC->GetVehicleCurrentLap(localList[i]) > levelState.mLaps)
//		{
//			levelState.mCars[localList[i]].mState = E_VEHICLE_STATE_FINISHED;
//
//			Game::Actors::C_VehicleForceAIControlMsg forceAIControlMsg(localList[i]);
//			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&forceAIControlMsg);
//
//			if (levelState.mNetworkRace)
//			{
//				// Any network players need to be informed this player has finished?
//				// If so, build message and send
//				NetVehicleIndexStack networkList;
//				if (GetNetworkPlayerList(networkList))
//				{
//					RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
//					RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();
//					Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
//
//					RacerNet::C_NetMessage& netMsg = services.Router().GetOutgoingReliable();
//					netMsg.CreateReliable();
//					BuildNetResultsForVehicle(netMsg, localList[i]);
//
//					for (U8 j=0; j<networkList.Size(); j++)
//					{
//						RacerNet::C_NetPlayer *player = manager.FindFromJoinId(levelState.mCars[networkList[j]].mNetJoinId, false);
//						if (player && player->PrimaryPlayer() && player->m_RaceData.m_InRaceVehicleIndex == networkList[j])
//							player->SendReliable(netMsg, RacerNet::eNetMessage_VehicleFinished, false, manager.LocalPrimaryJoinId(false));
//					}
//				}
//
//				CheckForEndOfRace();
//			}
//			else
//			{
//				_UI_MESSAGE(E_GameflowMessage_RaceFinished);
//				C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished );
//				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
//			}
//		}
//	}*/
//}
//
//void C_RaceModeStreetRace::UpdateForLastManStanding(NetVehicleIndexStack &localList)
//{
//	// Check all vehicles for fatally damaged.
//	// If only one vehicle remains, the race should end.
//
//	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
//
//	BIZ_ASSERT_MSG(pRLC, "Race Logic Core does not exist");
//
//	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
//
//	int leftInRace = 0;
//	for (U8 i=0; i<levelState.mNumCars; i++)
//	{
//		const Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(i);
//
//		if (pLogicActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED)
//			levelState.mCars[i].mState = E_VEHICLE_STATE_FATALLY_DAMAGED;
//		else if (levelState.mCars[i].mState == E_VEHICLE_STATE_RACING || levelState.mCars[i].mState == E_VEHICLE_STATE_WAITING_TO_START)
//			leftInRace++;
//	}
//
//	if (leftInRace <= 1)
//	{
//		_UI_MESSAGE(E_GameflowMessage_RaceFinished);
//		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished );
//		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
//	}
//}

void C_RaceModeStreetRace::StartLevel()
{
	C_RaceModeBase::StartLevel();
}

void C_RaceModeStreetRace::StartRace()
{
	C_RaceModeBase::StartRace();
}

void C_RaceModeStreetRace::UpdateHUD()
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

const UI::C_WideString512 C_RaceModeStreetRace::GetScoreString()
{
	UI::C_WideString512 str;
	UI::C_WideString64 str2;

	str.Clear();

	return str;
}

const UI::C_WideString512 C_RaceModeStreetRace::GetRulePassedString(U32 idx)
{
	UI::C_WideString512 str;

	if(mRulePassedTimer)
		C_RaceRulesManager::Instance().GetPassRuleString(idx, str, mRuleIndex, Game::RACE_MODE_RULE_STRING_TYPE_RESULTS);
	else
		str.Clear();

	return str;
}

const UI::C_WideString512 C_RaceModeStreetRace::GetRuleFailedString(U32 idx)
{
	UI::C_WideString512 str;

	if(mRulePassedTimer)
		C_RaceRulesManager::Instance().GetFailRuleString(idx, str, mRuleIndex, Game::RACE_MODE_RULE_STRING_TYPE_RESULTS);
	else
		str.Clear();

	return str;
}

SP_RaceEvent* C_RaceModeStreetRace::GetEvent()
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	SP_Event* pBaseEvent = SP_GetEventById( SP_GetCurrentSaveGame(), levelState.mSpEventId );

	BIZ_ASSERT(pBaseEvent);

	if (!pBaseEvent)		
		return NULL;

	if (pBaseEvent->kind != kSP_RaceEvent)
		return NULL;

	return (SP_RaceEvent*)pBaseEvent;
}

bool C_RaceModeStreetRace::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	if(!ShouldHandleMessages())
		return false;

	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		//case E_VehicleLogicMsg_FiredPerk:
		//	{
		//		const C_VehicleLogicMsg_FiredPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_FiredPerkMsg >(pMessage);
		//		
		//		

		//	}
		//	break;

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
				else
				{
					// Should hopefully never be hit, but just in case, we remove them as the player cannot target them.
					//PK - Hoping this is just safety thing, will check with SteveB tomorrrow
					//Game::C_RaceLogicLayer* pLayer				= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
					//pLayer->GetLogicVehicleActor(pMsg->mVehicleIndex)->PreRemoveFromRace();
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

				if (pLayer->GetNumVehicles() > 0)
					if (pLayer->GetLogicVehicleActor(0)->GetCurrentHealth() <= pRLC->GetRLSystem()->mDamageSystem->mParameters->mHealthIncrement)				// Must be greater than 0 to finish
//					if (pLayer->GetLogicVehicleActor(0)->GetCurrentHealth() <= (pRLC->GetRLSystem()->mDamageSystem->mParameters->mHealthIncrement / 2.0f))		// Now needs 1/2 a bar or less - Must be greater than 0 to finish
						SP_BossDemands::Instance().RegisterBossDemandAction(E_BOSSDEMAND_ACTION_WIN_WITH_1_HEALTH);

			}
			break;
	}

	return handledMessage;
}

// For UI testing only
void C_RaceModeStreetRace::SetupFakeResults()
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

void C_RaceModeStreetRace::SetupQuitAndWinResults(lua_State *pState)
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

	SP_RaceEvent* pEvent = GetEvent();

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

void C_RaceModeStreetRace::SetupResults()
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

		// This should all get done by the race rules manager, constantly updated - if it's not, something has gone wrong.
		//BIZ_ASSERT(results.mCars[a].mPassedRule.Size() == C_RaceRulesManager::Instance().GetNumPassRules());

		//for( int b = 0; b < C_RaceRulesManager::Instance().GetNumPassRules(); b++ )
		//{
		//	if(pLogicActor->GetCurrentHealthLevel() != E_HEALTH_LEVEL_WRECKED)
		//		results.mCars[a].mPassedRule.PushBack(C_RaceRulesManager::Instance().HasPassed(a,b,true));
		//	else
		//		results.mCars[a].mPassedRule.PushBack(false);
		//}

		//for( int b = 0; b < C_RaceRulesManager::Instance().GetNumFailRules(); b++ )
		//	results.mCars[a].mFailedRule.PushBack(C_RaceRulesManager::Instance().HasPassed(a,b,false));
	}

	SP_RaceEvent* pEvent = GetEvent();

	if (pEvent && pEvent->event)
	{
		if (const Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0))
		{
			int position = pLogicActor->GetPositionInfo().mRacePosition;

			if (position <= 1)
				results.mRaceStatistics.mMedal = kSP_Gold;
			else if (position <= 2)
				results.mRaceStatistics.mMedal = kSP_Silver;
			else if (position <= 3)
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

bool C_RaceModeStreetRace::ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel)
{
	/*if (C_RaceModeBase::ReceiveReliableNetMessage(msg, msgType, senderId, partyChannel))
		return true;

	Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	if (!netGameVars.m_InRace)
		return false;

	switch (msgType)
	{
	case RacerNet::eNetMessage_VehicleFinished:
		HandleNetResultsForVehicle(msg, senderId);
		return true;
	}*/

	return false;
}

bool C_RaceModeStreetRace::ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel)
{
	/*if (C_RaceModeBase::ReceiveUnReliableNetMessage(msg, msgType, numUnEncryptedBytes, senderId, partyChannel))
		return true;

	Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	if (!netGameVars.m_InRace)
		return false;*/

	return false;
}

} // namespace Game
