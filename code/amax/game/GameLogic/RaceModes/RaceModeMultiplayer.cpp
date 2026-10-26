#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic\RaceModes\RaceModeMultiplayer.h>
#include <GameLogic\MultiplayerUtilities.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>

#include <GameLogic\RaceRules\RaceRulesManager.h>

#include <GameLogic\Messaging\GameLogicMessages.h>
#include <GameLogic/Profile/MultiplayerProfile.h>

#include <GameLogic\Structures\MpResults.h>
#include <GameLogic\Structures\NetEventPackets.h>

#include <Game\Messaging\GameflowMessages.h>
#include <Game\Messaging\GameMessages.h>
#include <Game\Messaging\PickupActorMessages.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\GameComponents.h>

#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>
#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_PointsSystem.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Damage.h>


#include <GameLogic\Layers\RaceLogicLayer.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Layers\AmaxWorldLayer.h>
#include <GameLogic\Pickups\PickupMineBase.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <GameLogic\ActorStudioActors\CutSceneMessages.h>
#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic\RaceRules\Multiplayer\MpRaceActionManager.h>
#include <Game\GameWorld\GameWorld.h>
#include <GameLogic\Profile\GameProfile.h>
#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic\AchievementsManager.h>
#include <GameLogic\StickerManager.h>
#include <GameLogic\Pickups\PickupsManager.h>
#include <GameLogic\Pickups\Normal\PickupShock.h>

#include <Network\Session.h>
#include <Network\ScoreboardWriter.h>
#include <Network\PlayerSkillWriter.h>
#include <Network\PlayerSkill.h>
#include <Network/Core/NetScoreboards.h>
#include <Network/Misc/NetDefines.h>
#include <Network/Core/NetServices.h>
#include <Network/Misc/NetDatagramPacketTypes.h>
#include <Network/Core/NetRace.h>
#include <Network/player/NetPlayerManager.h>
#include <Network/Router/NetRouterBase.h>
#include <Network\Core\NetLogger.h>
#include <Network\Core.h>
#include <Network\CommunicatorManager.h>

#include <GameAudio/GameAudioLayer.h>
#include "Ui/SoundEffects.h"

#include <Game/GameActors/Components/Messages/AudioMessages.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MultiplayerChallenges.h>

#include <Ui/game/Gadgets/ButtonMessages.h>
#include <Ui\UiMessages.h>
#include <Ui/UiHelpers.h>
#include <ui/Game/Profiles/ProfileInterface.h>

#include <AI\AI_Amax.h>

Debug::Logger &g_MultiplayerLog(Debug::GetLogger("Amax.Multiplayer"));
Debug::Logger &g_debugNetMessageNumLog(Debug::GetLogger("Amax.Multiplayer.PickupMpSync.NetMessageNums"));
Debug::Bool g_debugHoGOnInRollingStart("Amax.Multiplayer.RollingStartHandOfGod", false, Debug::Persist());

Debug::Bool g_AllowSingleRacer("Multiplayer.AllowSingleRacer", false, Debug::Persist() );
Debug::Bool g_LocalPlayerWin("Multiplayer.LocalPlayerWin", false );
Debug::Int g_MinRankedPlayers("Multiplayer.Ranked.MinRankedPlayers", -1, Debug::Range( 2, 20 ) );

Debug::Float g_RacePositionUpdateTime("Multiplayer.RacePositionUpdateTime_", 500.0f, Debug::Range(0.0f, 1000.0f));
Debug::Float g_VehicleUpdateTimeOverride("Multiplayer.VehicleUpdateFrequencyInMilliseconds", 100.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(25.0f));
Debug::Bool g_ReportDamageMessageActedUpon("Multiplayer.ReportWhenNetworkDamageMessageIsActedUpon", false);

Debug::Float g_debugNetworkLeftRaceGhostTime("Multiplayer.LeftRaceGhostTime", 1.0f);

#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
	#if defined(CFG_HACKED_MASTER)
Debug::Bool	g_PlayerVehiclesUnderAiControl("Multiplayer.PlayerVehiclesUnderAiControl", true);
Debug::Bool g_debugForceLocalAIToHumanControl("Multiplayer.ForceLocalAIToHumanControl", false);
	#else
Debug::Bool	g_PlayerVehiclesUnderAiControl("Multiplayer.PlayerVehiclesUnderAiControl", false);
Debug::Bool g_debugForceLocalAIToHumanControl("Multiplayer.ForceLocalAIToHumanControl", false);
	#endif
#else
Debug::Bool	g_PlayerVehiclesUnderAiControl("Multiplayer.PlayerVehiclesUnderAiControl", false);
Debug::Bool g_debugForceLocalAIToHumanControl("Multiplayer.ForceLocalAIToHumanControl", false);
#endif

Debug::Bool g_ReportNetworkPickupAutopsyReports("Multiplayer.ReportNetworkPickupAutopsyReports", false);


extern Debug::Enum<UI::E_MultiplayerVehicleAbilities> g_AbilitySlot1;
extern Debug::Int g_DebugFiringIdx;
extern Debug::Int g_DebugTargetIdx;

static void FakeAbilityActivated(void *)
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[g_DebugFiringIdx].mUpgrades.GetMultiplayerAbilities();
	pAbilities->SetAbilityFiring(g_AbilitySlot1, true, Game::VehicleIsLocal(g_DebugFiringIdx), g_DebugFiringIdx, g_DebugTargetIdx);
}

Debug::Function g_DebugFakeAbilityActivatedMessage("Multiplayer.Upgrades.FakeAbilityInSlot1Activated", FakeAbilityActivated, 0);

extern Debug::Logger &g_debugPickupMpSyncLogger;
Debug::Logger &g_debugNetEventLogger = Debug::GetLogger("Multiplayer.NetEventLog");

namespace Game
{

const int MAX_NET_EVENTS_PER_UPDATE = 10;

void C_RaceModeMultiplayer::Initialise(Game::C_LevelState *pLevelState)
{
	C_RaceModeBase::Initialise(pLevelState);

	SetInternalNameAndID("Multiplayer", E_RaceMode_Nothing);

	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	
	AddSubscriber( E_GameLogicMessage_LapIncremented );
	AddSubscriber( E_GameflowMessage_BeginRollingStart );
	AddSubscriber( E_GameflowMessage_RaceStarted );
	AddSubscriber( E_GameflowMessage_QuitRace );
	AddSubscriber( E_GameflowMessage_QuitLevel );
	//AddSubscriber( E_VehicleLogicMsg_Ghosted );
	AddSubscriber( E_VehicleMessage_Eliminated );
	AddSubscriber( GameMessaging::E_GameMsg_FinalUpdate );

	mIsNetworkGame = true;
}

void C_RaceModeMultiplayer::Update()
{
	PERF_EVENT("RMM.Update");

	Game::C_LevelState* pLevelState = GetLevelState();
	const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();

	const Time::C_Time time = Time::GetPerformanceCounter();
	float timeStep = C_GameApp::GetInstance()->GetTimeStep();

	// JB: Need to skip this in split screen mode (send query to racer 2010 team if confusion.. sorry!)
	if(mIsNetworkGame)
	{
		// we only want to start sending network packets once all players have loaded and synchronized at the start of the race.
		if (!netGameVars.m_PacketTransferEnabled)
			return;

		// calculate which remote players we need to send voice to during the race.
		UpdateInRaceVoiceRecipients();

		if ((time - mUpdateTimer) / 1000 > netGameVars.m_NetUpdateTimestep)
		{
			mUpdateTimer = time;

			IncrementUpdateTimestep();

			// Do an update
			SendControlsUpdate();
		}
	}

	if(!mRaceInProgress)
		return;

	//  Check regular end conditions
	if(!mRaceFinished)
	{
		CheckForEndOfEvent();

		if(NoHumanPlayersStillActive() && !mRaceFinished)
		{
			if(mIsNetworkGame)
			{
				ForceTimeOutLocalVehicles();
			}
			else //Splitscreen
			{
				EnsureValidVehicleStatesSet(); 
				ForceSplitScreenRaceFinished();
			}
		}
	}

	if((time - mRacePositionUpdateTime) / 1000 > g_RacePositionUpdateTime)
	{
		UpdateRacePositions();
		mRacePositionUpdateTime = time;
	}

	MpMode_Update();
}


void C_RaceModeMultiplayer::UpdateRacePositions()
{
	MpMode_SortResults();

	PERF_EVENT("RMM.UpdateRacePositions");

	Game::C_LevelState* pLevelState = GetLevelState();

	//	Mark players as such in the MP results so the UI can highlight player vehicles

	C_MpResults::Instance().ClearPlayerIndices();
	MpVehiclesData& vehicleResults = C_MpResults::Instance().m_SortedVehicleResults;
	for(S32 i = 0; i < vehicleResults.Size(); i++)
	{
		if(vehicleResults[i].m_LocalIndex > -1)
		{
			if(pLevelState->mCars[vehicleResults[i].m_LocalIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
			{
				if(pLevelState->mCars[vehicleResults[i].m_LocalIndex].mController >= 0 && pLevelState->mCars[vehicleResults[i].m_LocalIndex].mController < MAX_USERS )
					C_MpResults::Instance().m_PlayerSortedIndices[pLevelState->mCars[vehicleResults[i].m_LocalIndex].mController] = i;
			}
		}
	}
}

void C_RaceModeMultiplayer::UpdateVehicleSpotlights()
{
	if(!mRaceInProgress)
		return;

	F32 frameTime = C_GameApp::GetInstance()->GetTimeStep();
	float desiredIntensity = 0.f;

	for(U32 i = 0; i < GetLevelState()->mNumCars; i++)
	{
		Game::Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(i);
		
		desiredIntensity = CalculateDesiredSpotlightIntensity(pLogicActor);
		
		if(desiredIntensity <= 0.00001f && mPillarEffects[i].active())
		{
			//We're not rendering this sucker so we'll turn it off and not render it
			mPillarEffects[i].stop();
		}
		else if(desiredIntensity > 0.f && !mPillarEffects[i].active())
		{
			mPillarEffects[i].start();
		}
		// Update the distance effect - will safely return if there's no effect currently running
		mPillarEffects[i].update(frameTime, desiredIntensity, pLogicActor?&pLogicActor->GetWorldMatrix():NULL);
	}
}

const float C_RaceModeMultiplayer::CalculateDesiredSpotlightIntensity(const Game::Actors::C_VehicleLogicActor* pLogicActor) const
{
	float desiredIntensity = 1.f;

	BIZ_ASSERT_MSG(pLogicActor, "C_RaceModeMultiplayer::CalculateDesiredSpotlightIntensity() > LogicActor's pointer's null :(");

	//Look, we all know that this
	const S8 localVehicleIndex = GetRLC_AlwaysChecked()->GetViewportFocusVehicleIndex(0);
	const S8 otherVehicleIndex = pLogicActor->GetVehicleIndex();

	const bool teammate = VehiclesAreTeammates(localVehicleIndex, otherVehicleIndex);

	//Check to see if we've got Silent Running enabled, if we have use the vehicle's current alpha
	if(pLogicActor->IsCloaked() && !teammate)
	{
		//Used the vehicle's current alpha
		return pLogicActor->GetGraphicalActor(otherVehicleIndex)->GetAlpha(E_VAC_TYPE_VEHICLE);
	}

	return desiredIntensity;
}

void C_RaceModeMultiplayer::DecrementFinalDelayTimer( void )
{
	//	Final delay before ending the event and moving to the results screen
	mFinalDelayTimer -= (F32)C_GameApp::GetInstance()->GetTimeStepRaw();
	if(mFinalDelayTimer <= 0)
	{
		mFinalDelayTimer = 0.0f;

		//	We want to force the end of the race here - since anyone who we don't have results for has presumably dropped out!
		ForceEndOfEvent();
	}
}

void C_RaceModeMultiplayer::ForceSplitScreenRaceFinished()
{	
	_UI_MESSAGE(E_GameflowMessage_RaceFinished);
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished );
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
	mRaceFinished = true;
}

void C_RaceModeMultiplayer::UpdateHUD()
{
	if(!mRaceInProgress)
		return;

	/*Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

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
	if(mLevelledTimer)
	{
		mLevelledTimer=Maths::Clamped(mLevelledTimer-pRLC->GetFrameTime(),0.f,3.0f);
	}
	if(mPerfectTimer)
	{
		mPerfectTimer=Maths::Clamped(mPerfectTimer-pRLC->GetFrameTime(),0.f,3.0f);
	}*/
}

bool C_RaceModeMultiplayer::ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel)
{
	Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	Game::C_LevelResult& levelResult = C_GameApp::GetInstance()->GetGame()->GetLevelResult();
	NetVehicleData& vehicleData = VehicleData(0);
	if (!netGameVars.m_InRace)
		return false;

	switch (msgType)
	{
	case RacerNet::eNetMessage_VehicleFinished:
		{
			HandleNetResultsForVehicle(msg, senderId);
			return true;
		}
	case RacerNet::eNetMessage_ForceRaceTimeout:
		{
			HandleForceTimeOutMessage(msg, senderId);
			return true;
		}
	}

	return false;
}

bool C_RaceModeMultiplayer::ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel)
{		
	Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	if (!netGameVars.m_InRace)
		return false;

	switch (msgType)
	{
	case RacerNet::eNetMessage_BasicVehicleData:
		HandleNetMessageBasicVehicleData(msg, senderId);
		break;
	}

	return false;
}

void C_RaceModeMultiplayer::StartLevel()
{
	mRaceResultsPosted=false;

	//	Clear any existing results data
	C_MpResults::Instance().Clear();

	Game::C_LevelState* pLevelState = GetLevelState();

	mIsNetworkGame = (C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == E_GameMode_SplitScreen) ? false : true;

	Game::C_LevelResult& results = C_Game::GetInstance()->GetLevelResult();
	results.Clear();
	//results.mFake=true; - [GavinY]: SteveB has no idea why he added this way back in June and since it's breaking MP (thanks to ChadW) I've taken this out...
	results.mNumCars=pLevelState->mNumCars;

	mRaceInProgress=false;
	mRaceFinished = false;
	mFirstPlayerFinished = false;

	mPostRaceDelayTimer = 0;

	mFinalDelayTimer = 0;
	mFinalDelayTimerActive = false;

	mUpdateTimer = Time::GetPerformanceCounter();
	
	DisableVehiclesLostDuringLoad();

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised() && services.Race().IsRunning())
		services.Race().RegisterRaceLogicListener(this);

	////	First pass at some sort of rolling AI start for Multiplayer
	NetVehicleIndexStack localPlayers;
	if (GetLocalPlayerList(localPlayers) != 0)
	{
		for(int i = 0; i < localPlayers.Size(); i++)
		{
			Game::Actors::C_VehicleForceAIControlMsg forceAIControlMsg(localPlayers[i]);
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&forceAIControlMsg);
		}
	}
	
	GetRaceLogicLayer()->ForceRandomisePickups(pLevelState->m_RandomSeed);

	for (U8 i=0; i<pLevelState->mNumCars; i++)
	{
		if (VehicleData(i).m_State != E_VEHICLE_STATE_LEFT_RACE)
			VehicleData(i).m_State = E_VEHICLE_STATE_RACING;

		VehicleData(i).m_NetJoinId = pLevelState->mCars[i].mNetJoinId;
		VehicleData(i).m_Team = pLevelState->mCars[i].mTeam;
	}

	C_RaceModeBase::StartLevel();

	UpdateRacePositions();
}


void C_RaceModeMultiplayer::BeginRollingStart()
{
	// switch voice to vehicle update packet transport.
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised() && services.Race().IsRunning())
	{
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		mgr.SetVoiceControlState(RacerNet::eVoiceControlStateGamePlay);
	}

	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(ActorMsg::E_CutSceneMsg_Start_ScriptedCamera);

	//C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_EnableRaceAI );

	//if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_SplitScreen)
	//{
	//	switch(Game::C_Game::GetInstance()->GetLevelState().mNumViewports)
	//	{
	//	case 4:
	//		{
	//			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_Start321_4_Player );
	//		}//fall through
	//	case 3:
	//		{
	//			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_Start321_3_Player );
	//		}//fall through
	//	case 2:
	//		{
	//			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_Start321_2_Player );
	//		}//fall through
	//	case 1:
	//		{
	//			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_Start321 );		
	//		}
	//		break;
	//	};
	//}
	//else
	//{
	//	//C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_Start321 );	
	//}
}

void C_RaceModeMultiplayer::StartRace()
{
	C_RaceModeBase::StartRace();

	//	Set player vehicle(s) back to PLAYER control

	NetVehicleIndexStack localPlayers;
	if (GetLocalPlayerList(localPlayers) != 0)
	{
		for(int i = 0; i < localPlayers.Size(); i++)
		{
			Game::Actors::C_VehicleForceHumanControlMsg forceHumanControlMsg(localPlayers[i]);
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&forceHumanControlMsg);

#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
			if(g_PlayerVehiclesUnderAiControl)
			{
				Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localPlayers[i]);

				if(pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Rumble))
					pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Rumble, E_VehicleLogicMsg_DisableRumble);

				pLogicActor->m_ToggledToAiControl = true;
				Game::Actors::C_VehicleForceAIControlMsg forceAiControlMsg(localPlayers[i]);
				pLogicActor->SendMessageToActor(pLogicActor->GetGraphicalActor(localPlayers[i])->GetHandle(), &forceAiControlMsg);
			}
#endif // CFG_GOLD
		}
	}

#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
	if (g_debugForceLocalAIToHumanControl)
	{
		Game::Actors::RaceLogicCore* pRLC = GetRLC();
		if (pRLC)
		{
			for (U32 iCar = 0; iCar < pRLC->GetNumVehicles(); iCar++)
			{
				Game::Actors::C_VehicleActor* pActor = pRLC->GetVehicleActor(iCar);
				if (pActor && pActor->GetControlType() == E_VEHICLE_CONTROL_TYPE_AI)
				{
					Game::Actors::C_VehicleForceHumanControlMsg forceHumanControlMsg(iCar);
					pActor->SendMessageToActor(pActor->GetHandle(), &forceHumanControlMsg);
				}
			}
		}
	}
#endif

	//	Record the start time of the race
	mRaceStartTime = Time::GetPerformanceCounter();

	mRacePositionUpdateTime = Time::GetPerformanceCounter();

	UpdateRacePositions();

	if(mIsNetworkGame)
	{
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetDataGathering().FlagPostEvent(DATA_GATHERING_RECORD_TYPE_VEHICLE_INFORMATION);
		if(((C_GameModeNetwork*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr())->GetModsAllowed())
		{
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetDataGathering().FlagPostEvent(DATA_GATHERING_RECORD_TYPE_MOD_SELECTION);
		}

		//	We need to do an emergency check here in case some stuff has screwed up and the network session has actually been terminated (probably itself caused by a separate bug)
		//	- otherwise the ::Update() method will bail out and never end the game.
		if (!C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_PacketTransferEnabled)
			CheckForEndOfEvent();
	}

	if(GetLevelState()->mTeams)
	{
		SetupTeamSpotlights();
	}
	else if(!mIsNetworkGame)
	{
		//	This can go here for now. If we do teams we'll want to prioritise the team spotlights since SS teams probably won't have AI.
		SetUpSplitScreenSpotlights();
	}

	mPostRaceDelayTimer = -1;
}

void C_RaceModeMultiplayer::CheckRaceTimoutTimer( void )
{
	
}

bool C_RaceModeMultiplayer::NoHumanPlayersStillActive()
{
	// Check all vehicles for fatally damaged.
	// If only one vehicle remains, the race should end.

	Game::Actors::RaceLogicCore* pRLC = GetRLC();

	BIZ_ASSERT_MSG(pRLC, "Race Logic Core does not exist");

	Game::C_LevelState* pLevelState = GetLevelState();

	int humansLeftInRace = 0;
	int aiLeftInRace = 0;
	for (U8 i=0; i<pLevelState->mNumCars; i++)
	{
		const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(i);

		//if (pLogicActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED)
		//{
		//	pLevelState->mCars[i].mState = E_VEHICLE_STATE_FATALLY_DAMAGED;
		//}
		//else 
			
		if (pLevelState->mCars[i].mState == E_VEHICLE_STATE_RACING || pLevelState->mCars[i].mState == E_VEHICLE_STATE_WAITING_TO_START)
		{
			// record type
			if(pLevelState->mCars[i].mHuman)
			{
				humansLeftInRace++;
			}
			else
			{
				aiLeftInRace++; 
			}
		}
	}

	bool timeToEnd = false; 

	//if( humansLeftInRace == 0 ||
	//	(humansLeftInRace == 1 && aiLeftInRace == 0) )
	if( humansLeftInRace == 0 )
	{
		timeToEnd = true; 
	}

	return timeToEnd; 
}

void C_RaceModeMultiplayer::EnsureValidVehicleStatesSet()
{
	if (!GetRaceLogicLayer())
		return;

	// This method called when race ends EARLY (e.g. all players die)
	// Iterate through all vehicles, ensure those alive are set to racing state, those wrecked are set to wrecked
	Game::C_LevelState* pLevelState	= GetLevelState();
	for(unsigned int i = 0; i < pLevelState->mNumCars; i++)
	{
		if(!(pLevelState->mCars[i].mState == E_VEHICLE_STATE_FINISHED))
		{
			const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(i);
			Actors::C_VehicleActor* pVehicleActor = pLogicActor->GetGraphicalActor(i);
			if(pLevelState->mCars[i].mState == E_VEHICLE_STATE_ELIMINATED)
			{
				pLevelState->mCars[i].mState = E_VEHICLE_STATE_ELIMINATED;
			}
			else if(pLogicActor->IsFatallyDamaged())
			{
				pLevelState->mCars[i].mState = E_VEHICLE_STATE_FATALLY_DAMAGED;
			}
			else //must still be in the race
			{
				pLevelState->mCars[i].mState = E_VEHICLE_STATE_TIMED_OUT;
			}
		}
	}
}

void C_RaceModeMultiplayer::SetupTeamSpotlights()
{
	BIZ_ASSERT_MSG( C_MultiplayerData::Instance().m_pConfig, "No Multiplayer config" );
	MultiplayerConfig* mpConfig = C_MultiplayerData::Instance().m_pConfig;

	U8 opposingTeam = (GetLevelState()->mCars[0].mTeam == 0) ? 1 : 0;

	//	[GavinY] - Bug #37377: Only create light pillars on vehicles that are still connected.
	NetVehicleIndexStack globalRacingList;
	GetGlobalInRaceList(globalRacingList);
	for(U32 i = 0; i < globalRacingList.Size(); i++)
	{
		U8 vehicleIndex = globalRacingList[i];
		if( GetLevelState()->mCars[vehicleIndex].mController == INVALID_CONTROLLER)
		{
			const SpecialFX::C_SpecialFX* p_pEffect = NULL;
			if( GetLevelState()->mCars[vehicleIndex].mTeam == opposingTeam )
			{
				p_pEffect = (opposingTeam == 0) ? mpConfig->m_TeamAPillarEffect : mpConfig->m_TeamBPillarEffect;
				SetupSpotlight(vehicleIndex, p_pEffect);
			}
		}
	}
}

void C_RaceModeMultiplayer::SetUpSplitScreenSpotlights()
{
	BIZ_ASSERT_MSG( C_MultiplayerData::Instance().m_pConfig, "No Multiplayer config" );
	MultiplayerConfig* mpConfig = C_MultiplayerData::Instance().m_pConfig;

	F32 frameTime = C_GameApp::GetInstance()->GetTimeStep();

	for(U32 i = 0; i < GetLevelState()->mNumCars; i++)
	{
		if( GetLevelState()->mCars[i].mHuman)
		{
			U32 controller = GetLevelState()->mCars[i].mController;
			BIZ_ASSERT_MSG(controller < MAX_VIEWPORTS, "player vehicle has an invalid controller index of %d", controller);
			const SpecialFX::C_SpecialFX* p_pEffect = mpConfig->m_SplitScreenLightPillars[controller];
			SetupSpotlight(i, p_pEffect);
		}
	}
}

//	Checks for the end of the event (all vehicles have ceased racing)
bool C_RaceModeMultiplayer::CheckForEndOfEvent(void)
{
#ifndef CFG_GOLD
	if ( g_AllowSingleRacer && g_LocalPlayerWin )
	{
		NetVehicleIndexStack localRacingList;
		if(GetLocalRacingList(localRacingList))
		{
			for ( S32 i = 0; i < localRacingList.Size(); i++ )
				VehicleFinished(localRacingList[i], E_VEHICLE_STATE_FINISHED);
		}
	}
#endif // CFG_GOLD

	//	If one of our local vehicles is the only vehicle left racing, they finish (no point continuing with one car on track)
	NetVehicleIndexStack globalRacingList;
	if(GetGlobalInRaceList(globalRacingList) == 1 && VehicleIsLocal(globalRacingList[0]) && !g_AllowSingleRacer)
	{
		if(mPostRaceDelayTimer == -1)
			mPostRaceDelayTimer = 30;
		
		if(mPostRaceDelayTimer > 0)
		{
			mPostRaceDelayTimer--;
		}
		else
		{
			VehicleFinished(globalRacingList[0], E_VEHICLE_STATE_FINISHED);
		}
	}

	//	If all vehicles have finished them head to the results screen (since we get their results with the Finished message)
	if (GetNumGlobalRacing() == 0 && mIsNetworkGame)
	{
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished );
		if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() &&
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld())
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );

		EnsureValidVehicleStatesSet(); 
		_UI_MESSAGE(E_GameflowMessage_RaceFinished);
		mRaceFinished = true;

		return true;
	}
	return false;
}


void C_RaceModeMultiplayer::ForceTimeOutLocalVehicles( E_VEHICLE_STATE timedOutState /*= E_VEHICLE_STATE_TIMED_OUT*/ )
{
	NetVehicleIndexStack localRacingList;
	if (GetLocalRacingList(localRacingList) == 0)
		return;				// No local vehicles
	for(int i=0; i<localRacingList.Size();i++)
	{
		VehicleFinished(localRacingList[i], timedOutState);
	}
}

void C_RaceModeMultiplayer::ForceEndOfEvent( void )
{
	//	Any vehicles still racing (which should only be remote vehicles) can be assumed to have disconnected from the race.
	Game::C_LevelState* pLevelState = GetLevelState();
	for (U8 i=0; i<pLevelState->mNumCars; i++)
	{
		if (VehicleData(i).IsRacing())
		{
			VehicleLeftRace(i);
		}
	}
}

void C_RaceModeMultiplayer::GetLocalPoints( void )
{
	NetVehicleIndexStack localRacingList;
	if(GetLocalRacingList(localRacingList) == 0)
		return;
	for(int i = 0; i < localRacingList.Size(); i++)
	{
		const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localRacingList[i]);
		if(pLogicActor && pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Points))
		{
			Components::C_VehicleComponent_PointsSystem* pointsComponent = (Components::C_VehicleComponent_PointsSystem*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Points);
			VehicleData(localRacingList[i]).m_Statistics[UI::E_MultiplayerStat_OnTrackPoints] = pointsComponent->GetCurrentPoints();
		}
	}
}

bool C_RaceModeMultiplayer::BuildLobbyWaitingRaceInfoData( LobbyWaitingRaceInfo& raceInfo )
{
	NetVehicleIndexStack localVehicles;
	const U8 numVehicles = GetLocalVehicleList(localVehicles);
	raceInfo.m_Timeleft = GetRaceTimeout() > .0f ? GetRaceTimeout() : -1.0f;
	raceInfo.m_Vehicles.Clear();

	LobbyRaceVehicleInfo vehicle;

	for(int i = 0; i < numVehicles; i++)
	{
		U8 vehicleIndex = localVehicles[i];
		Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);

		vehicle.m_JoinId = GetLevelState()->mCars[vehicleIndex].mNetJoinId;
		vehicle.m_RaceState = (U8)VehicleData(vehicleIndex).m_State;
		vehicle.m_StatData = MpMode_GetPrimaryStatValue(vehicleIndex);

		if(pLogicActor)
		{
			vehicle.m_Xposition = pLogicActor->GetWorldMatrix().AW().X();
			vehicle.m_Zposition = pLogicActor->GetWorldMatrix().AW().Z();
		}
		else
		{
			vehicle.m_Xposition = 0.f;
			vehicle.m_Zposition = 0.f;
		}
		raceInfo.m_Vehicles.PushBack(vehicle);
	}
	
	return true;
}

bool C_RaceModeMultiplayer::SendControlsUpdate(void)
{
	PERF_EVENT("RMM.SendControlsUpdate");

	Game::C_LevelState *pLevelState = GetLevelState();

	NetVehicleIndexStack localToSend;
	if (GetLocalVehicleList(localToSend) == 0)
		return true;				// No data to send

	// Build packet data

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();

	NetVehicleIndexStack networkList;
	if (GetNetworkVehicleList(networkList))
	{
		for(int i=0; i<networkList.Size();i++)
		{
			RacerNet::C_NetPlayer *player = manager.FindFromJoinId(pLevelState->mCars[networkList[i]].mNetJoinId, false);
			if (player && player->PrimaryPlayer() && player->m_RaceData.m_InRaceVehicleIndex == networkList[i])
			{
				RacerNet::C_NetMessage& netMsg = services.Router().GetOutgoingUnReliable();
				netMsg.CreateUnReliable(RacerNet::eNetMessage_BasicVehicleData, manager.LocalPrimaryJoinId(false), false);

				BuildNetMessageBasicVehicleData(netMsg, networkList[i]);

				netMsg.WriteU16(pLevelState->mCars[networkList[i]].mNetMessageNos[0]); // What this kit believes the Message Number of the recipient to be

				const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
				U8 eventsToSend = pLevelState->mCars[0].mNetMessageNos[0] - pLevelState->mCars[networkList[i]].mNetMessageNos[1];

				if(eventsToSend > MAX_NET_EVENTS_PER_UPDATE)
					eventsToSend = MAX_NET_EVENTS_PER_UPDATE;
				
				netGameVars.m_EventManager->WriteEventsIntoMessage(pLevelState->mCars[networkList[i]].mNetMessageNos[1] + 1, netMsg, eventsToSend);
				
				const S32 remain = RacerNet::NET_MAX_DATAGRAM_SIZE - netMsg.DataSize();
				const S32 voiceSize = player->VoiceSize() + sizeof(RacerNet::NetLocalVoiceSizes); 
				if (remain > voiceSize)
				{
					AppendVoiceDataToControlsUpdate(netMsg, pLevelState->mCars[networkList[i]].mNetJoinId);

					netMsg.TerminateMessage();
					if (player->m_RaceConnection.CanMessage())
						player->SendUnReliable(netMsg, false, services.Router());
				}
				else
				{
					netMsg.TerminateMessage();
					if (player->m_RaceConnection.CanMessage())
						player->SendUnReliable(netMsg, false, services.Router());

					manager.SendBasicVoiceMessageToPlayer(netMsg, services.Router(), pLevelState->mCars[networkList[i]].mNetJoinId, false); 
				}
			}
		}
	}
	return true;
}

void C_RaceModeMultiplayer::AppendVoiceDataToControlsUpdate(RacerNet::C_NetMessage& msg, const U8 joinId)
{
	PERF_EVENT("RMM.AppendVoiceDataToControlsUpdate");

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();
	RacerNet::C_NetPlayer *p = manager.FindFromJoinId(joinId, false);
	if (p)
	{
		bool ok = true;
		RacerNet::NetLocalVoiceSizes localVoiceSize;
		for (S32 i=0; i<localVoiceSize.Size(); i++)
		{
			const U32 size = p->VoiceSize(i);
			assert(size <= 255); // only going to transmit this as U8 so can't go above 255 bytes per local player;
			localVoiceSize[i] = static_cast<U8> (size);
		}

		// write number of voice bytes per local player.
		for (S32 i=0; i<localVoiceSize.Size(); i++)
			ok = ok && msg.WriteU8(localVoiceSize[i]);

		// start unencrypted voice data. only voice data can be transmitted unencrypted.
		ok = ok && msg.StartUnEncryptedData();

		// copy buffered voice to message.
		ok = ok && p->WriteOutgoingVoiceToMessage(msg);

		if (!ok)
		{
			LOGWARN(RacerNet::g_Log, "C_RaceModeMultiplayer::AppendVoiceDataToControlsUpdate() - unable to pack voice data into race data.");
		}
	}
	else
	{
		LOGERROR(RacerNet::g_Log, "C_RaceModeMultiplayer::AppendVoiceDataToControlsUpdate() - trying to send voice to unknown player.");
	}
}

void C_RaceModeMultiplayer::ExtractVoiceDataFromControlsUpdate(RacerNet::C_NetMessage& msg, const U8 senderId)
{
	bool ok = true;

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();
	RacerNet::C_NetPlayer *p = manager.FindFromJoinId(senderId, false);
	if (NULL == p)
		return; // remote player no longer in our session.

	const U64 remoteConsoleId = p->m_ConsoleId;

	//----------------------------------------------------
	// TCR #093 : bug 29766, 29767, 29744, 29745
	//----------------------------------------------------
	// most restrictive 
	if(!manager.CheckLowestVoicePrivileges())
		return;
						
	// read how much voice data we expect for each remote peer.
	RacerNet::NetLocalVoiceSizes localVoiceSize;
	for (S32 i=0; i<localVoiceSize.Size(); i++)
		ok = ok && msg.ReadU8(localVoiceSize[i]);

	if (ok)
	{
		U8 data[RacerNet::NET_VOICE_BUFFER_SIZE];
		for (S32 i=0; i<localVoiceSize.Size() && ok; i++)
		{
			if (localVoiceSize[i] > 0)
			{
				// extract voice from msg
				ok = ok && msg.ReadBuffer(data, localVoiceSize[i]);
				if (ok)
				{
					RacerNet::C_NetPlayer* rt = manager.FindFromConsoleId(remoteConsoleId, i);
					if (rt)
					{
						if (0 == rt->m_QuickMute) // TODO -> this wont work if we have more than 1 local talker. need a different way to handle quick muting in that case.
							Communicator::C_Core::Instance().SubmitRemoteVoiceData(rt->m_User, data, localVoiceSize[i]);
					}
					else
						ok = false;
				}
			}
		}
	}
	else
	{
	}
}

bool C_RaceModeMultiplayer::BuildNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, U32 remoteIndex)
{
	Game::C_LevelState* pLevelState = GetLevelState();

	//	Only send position/controls etc for local vehicles which are actually still racing.
	NetVehicleIndexStack localRacingList;
	GetLocalRacingList(localRacingList);

	netMsg.WriteU8(localRacingList.Size());

	for (U8 i=0; i<localRacingList.Size(); i++)
	{
		Game::Actors::RaceLogicCore *raceLogicCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		Game::Actors::C_VehicleActor *vehicleSource = (Game::Actors::C_VehicleActor *) raceLogicCore->GetVehicleActor(localRacingList[i]);
		Game::Actors::C_VehicleActor *vehicleTarget = (Game::Actors::C_VehicleActor *) raceLogicCore->GetVehicleActor(remoteIndex);

		RacingPhysics::Vehicle::CompressedNetworkControls controls;
		RacingPhysics::Vehicle::CompressedNetworkState state;

		vehicleSource->GetNetworkControlsAndState(&controls, &state, vehicleTarget);

		const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localRacingList[i]);
		VehicleData(localRacingList[i]).m_Statistics[UI::E_MultiplayerStat_HealthRemaining] = pLogicActor->GetCurrentHealth();
		E_PICKUP_TYPE currentPickup = pLogicActor->GetPickupInfo().mCurrentlySelectedType;


		//	Write
		netMsg.WriteU8(pLevelState->mCars[localRacingList[i]].mNetJoinId);
		netMsg.WriteBuffer(&controls, sizeof(controls));
		netMsg.WriteBuffer(&state, sizeof(state));
		netMsg.WriteS32(VehicleData(localRacingList[i]).m_Statistics[UI::E_MultiplayerStat_HealthRemaining]);	//	Write the vehicle's current health
		netMsg.WriteU32((U32)currentPickup);

		AddModeSpecificNetMessageBasicVehicleData(netMsg, localRacingList[i]);	//	Add any mode-specific data we want to send to the end
	}

	return true;
}

bool C_RaceModeMultiplayer::AddModeSpecificNetMessageBasicVehicleData( RacerNet::C_NetMessage& netMsg, U8 p_VehicleIndex )
{
	return true;
}

bool C_RaceModeMultiplayer::HandleNetMessageBasicVehicleData(RacerNet::C_NetMessage& msg, const U8 senderId)
{
	Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	netGameVars.m_ReceivedRemoteRacePackets = true;

	if (!netGameVars.m_PacketTransferEnabled)
		return false; // don't accept vehicle updates if we are not in a state to process them.

	Game::Actors::RaceLogicCore *raceLogicCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	// Because people change there minds, we can actual dump the level on the results screen and can have a backlog of vehicle message updates.
	// Plus checking pointers is always a good idea
	if ( !raceLogicCore )
		return false;

	bool ok = true;
	ok = msg.ReadOpen();
	if (ok)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();

		U8 numCars;

		ok = ok && msg.ReadU8(numCars);

		Game::C_LevelState* pLevelState = GetLevelState();

		for (U8 i=0; i<numCars; i++)
		{
			S8 index = -1;
			U8 joinId;
			S32 health;
			U32 currentPickup;
			RacingPhysics::Vehicle::CompressedNetworkControls controls;
			RacingPhysics::Vehicle::CompressedNetworkState state;

			//	Read

			ok = ok && msg.ReadU8(joinId);

			ok = ok && msg.ReadBuffer(&controls, sizeof(controls));
			ok = ok && msg.ReadBuffer(&state, sizeof(state));
			ok = ok && msg.ReadS32(health);
			ok = ok && msg.ReadU32(currentPickup);

			//	We might still have some mode-specific data to read out of the message,
			//	but we'll get that after we process the standard stuff (and have an index)
			index = GetLocalIndexFromJoinId(joinId);
			if (index != -1)
			{
				if (pLevelState->mOriginalNetworkHost)
				{
				}
				if(VehicleData(index).IsRacing())
				{
					Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(index);
					BIZ_ASSERT_MSG(pLogicActor, "pLogicActor is NULL - see GavinY :)");
					const Actor::C_ActorHandle vehicleHandle = raceLogicCore->GetActorHandleFromVehicleIndex(index);

					Game::Actors::C_VehicleUpdateNetworkControlsMsg vehicleControls;

					vehicleControls.m_CarControls = controls;
					vehicleControls.m_CarState = state;
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->SendMessageToActor(vehicleHandle, &vehicleControls);

					pLogicActor->OverrideCurrentPickup((E_PICKUP_TYPE)currentPickup);

					SetNetworkVehicleHealth(index, health);
					VehicleData(index).m_Statistics[UI::E_MultiplayerStat_HealthRemaining] = health;
				}
			}
			HandleModeSpecificNetMessageBasicVehicleData(msg, index);	//	Read and process any mode-specific data in the message
		}
		U8 primaryPlayerLocalIndex = GetLocalIndexFromJoinId(senderId);
		if(ok)
		{
			//	Record what the sender thinks the Message Number of this kit is
			ok = ok && msg.ReadU16(pLevelState->mCars[primaryPlayerLocalIndex].mNetMessageNos[1]);
		}
		U8	numEventsReceived = 0;
		if(ok)
			ok = ok && msg.ReadU8(numEventsReceived);
		//	For the number of packets, read in the event data and act upon it
		ProcessEvents(primaryPlayerLocalIndex, numEventsReceived, msg);
		if (ok)
			ExtractVoiceDataFromControlsUpdate(msg, senderId);

		ok = ok && msg.ReadClose();
	}

	return true;
}

bool C_RaceModeMultiplayer::HandleModeSpecificNetMessageBasicVehicleData( RacerNet::C_NetMessage& netMsg, S8 p_VehicleIndex )
{
	return true;
}

//	Performs the "tidy up" functions we carry out when we're done with a local MP vehicle - whether it finished/wrecked/timed out/eliminated/WHATEVER
void C_RaceModeMultiplayer::TidyUpVehicleAndSendResults( const U8 p_Index )
{
	Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(p_Index);
	pLogicActor->SendMessageToActor(pLogicActor->GetHandle(), Game::E_GameflowMessage_VehicleFinished);

	//send to audio
	const Messaging::C_MessageBase message(Game::E_GameflowMessage_VehicleFinished, sizeof(Messaging::C_MessageBase));
	C_GameApp::GetInstance()->GetGameAudioLayer()->MessageHandler(&message);

	//	Fill the vehicle's data structure with race statistics and scoring data(results)
	FillVehicleData(p_Index);
	//	Send off the results to all the other players
	SendNetResultsForVehicle(p_Index);
}

//	Handles a local vehicle finishing a multiplayer event
void C_RaceModeMultiplayer::VehicleFinished(const U8 p_Index, Game::E_VEHICLE_STATE p_FinishType)
{
	mVehicleHasLeft = true;

	//	Kill any spotlight effect on this vehicle.
	mPillarEffects[p_Index].reset();

	//	We could potentially come in here if someone disconnected on the race results screen, and since we dump the Amax world layer soon after
	//	reaching that screen, lots of the below calls could explode. We don't need to do any of this stuff by that point anyway!
	if(mRaceFinished)
		return;

	bool forceTimeout = false;
	bool isLocal = VehicleIsLocal(p_Index);

	Game::C_LevelState* pLevelState = GetLevelState();
	pLevelState->mCars[p_Index].mState = p_FinishType;

	if(isLocal)
	{
		VehicleData(p_Index).m_State = p_FinishType;

		TidyUpVehicleAndSendResults(p_Index);

		if(C_Game::GetInstance()->GetLevelState().mCars[p_Index].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
		{
			//	Flag to the UI that it's time to go to the Vehicle Results screen
			mPrimaryVehicleFinished = true;

			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_SplitScreen)
			{
				forceTimeout = true;
				NetVehicleIndexStack localVehicles;
				GetLocalVehicleList(localVehicles);
				for(int vehicleI = 0; vehicleI < localVehicles.Size(); vehicleI++) //For each vehicle
				{
					if(C_Game::GetInstance()->GetLevelState().mCars[vehicleI].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER) //If its a human
					{
						E_VEHICLE_STATE test = VehicleData(vehicleI).m_State;
						if(VehicleData(vehicleI).m_State == E_VEHICLE_STATE_RACING || VehicleData(vehicleI).m_State == E_VEHICLE_STATE_UNKNOWN)
						{
							forceTimeout = false; //Player still going and so shall the Race
							mPrimaryVehicleFinished = false;
							break;
						}
					}
					else
					{
						break;  //Players come first so can break now
					}
				}

				// IF not eliminated..
				if(p_FinishType != E_VEHICLE_STATE_ELIMINATED)
				{
					// PR - send vehicle finished to UI
					_UI_MESSAGE1( E_GameflowMessage_VehicleFinished, UI::GetLocalPlayerIndex( p_Index ) );
				}
			}
			else
			{
				// BUG: 35623 - Spectating silent running vehicles causes problems. If we finish.. turn off silent running on all cars (on this kit.. not over the network). 
				TurnOffSilentRunningOnAllVehicles(); 
			}
		}
	}
	//Bug# 33081 - When vehicles with Silent Running on win you see them still invisible or with a pure black car
	else
	{
		Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(p_Index);
		BIZ_ASSERT_MSG(pLogicActor, "C_RaceModeMultiplayer::VehicleFinished() > Couldn't get a pointer to the logic actor :(");
		pLogicActor->SendMessageToActor(pLogicActor->GetHandle(), Game::E_GameflowMessage_VehicleFinished);

		if(p_FinishType == E_VEHICLE_STATE_LEFT_RACE)
		{
			VehicleData(p_Index).m_State = p_FinishType;
		}
	}

	bool finished = false;
	switch(p_FinishType)
	{
	case E_VEHICLE_STATE_FINISHED:
		{
			finished = true;
			//If the first car has finished then check to see if they are the first one finished
			if(!mFirstPlayerFinished)
			{
				Game::C_LevelResult& levelResults = C_GameApp::GetInstance()->GetGame()->GetLevelResult();
				Game::C_RaceLogicLayer* pRaceLogic = GetRaceLogicLayer();
				Actors::C_VehicleLogicActor* pActor = NULL;
				float finisherDistanceFromStart = pRaceLogic->GetLogicVehicleActor(p_Index)->GetPositionInfo().mDistanceFromStart;
				float myDistanceFromStart = 0;
				for(unsigned int i = 0; i < pLevelState->mNumCars; i++)
				{
					if(VehicleIsLocal(i) && pLevelState->mCars[i].mState == E_VEHICLE_STATE_RACING)
					{
						pActor = pRaceLogic->GetLogicVehicleActor(i);
						myDistanceFromStart = pActor->GetPositionInfo().mDistanceFromStart;

						levelResults.mCars[i].mStatistics.mDistToFirstOnFinish = finisherDistanceFromStart - myDistanceFromStart;
					}
				}
				mFirstPlayerFinished = true;
			}
		}
		break;

	case E_VEHICLE_STATE_FATALLY_DAMAGED:
		{
			finished = false;
			if(!isLocal)
			{
				SetNetworkVehicleHealth(p_Index, -1);
			}
		}
		break;

	case E_VEHICLE_STATE_ELIMINATED:
		{
			finished = false;
			EliminateVehicle(p_Index);
		}
	}

	if(p_FinishType != E_VEHICLE_STATE_LEFT_RACE)
	{
		//	Switch control of the finished vehicle to AI - this SHOULDN'T cause wrecked or eliminated vehicles to start racing around again...
		Game::Actors::C_VehicleForceAIControlMsg forceAIControlMsg(p_Index);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&forceAIControlMsg);
	}

	if(forceTimeout)
	{
		ForceTimeOutLocalVehicles();
	}

	return;
}

void C_RaceModeMultiplayer::TurnOffSilentRunningOnAllVehicles()
{
	Game::C_LevelState* pLevelState = GetLevelState();
	Game::C_RaceLogicLayer* pRaceLogic = GetRaceLogicLayer();
	if(!pLevelState || !pRaceLogic)
	{
		return;
	}

	Actors::C_VehicleLogicActor* pActor = NULL;
	for(unsigned int i = 0; i < pLevelState->mNumCars; i++)
	{
		pActor = pRaceLogic->GetLogicVehicleActor(i);
		if(pActor)
		{
			pActor->ClearUpSilentRunningSFXForEndOfRace();
		}
	}
}

//	Handles vehicle leaving (dropping out) of a Multiplayer event.
void C_RaceModeMultiplayer::VehicleLeftRace(const U8 p_Index)
{
	// Make sure this vehicle is registered as finished
	VehicleFinished(p_Index, Game::E_VEHICLE_STATE_LEFT_RACE);

	float fHideTime = 0.0f;
	if (GetRaceLogicLayer() && 
		GetRaceLogicLayer()->GetRaceState() == Game::E_RaceState_Racing)
		fHideTime = g_debugNetworkLeftRaceGhostTime;

	// Need to inform physics and renderer that car is no longer required.
	Game::Actors::C_VehicleLeftRaceMsg msg(p_Index, fHideTime);
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&msg);

	//	Check here if there are any vehicles in the race that this vehicle was responsible for (i.e. AI vehicles running on its kit)
	const Game::C_LevelState* pLevelState = GetLevelState();
	for(U32 i = 0; i < pLevelState->mNumCars; i++)
	{
		if(pLevelState->mCars[i].mOwnerNetJoinId == pLevelState->mCars[p_Index].mNetJoinId)
		{
			//	We could call VehicleLeftRace on this vehicle too
			//	but we can assume it wasn't responsible for any other vehicles itself, so this is quicker :)

			// Make sure this vehicle is registered as finished
			VehicleFinished(static_cast<U8>(i), Game::E_VEHICLE_STATE_LEFT_RACE);

			// Need to inform physics and renderer that car is no longer required.
			Game::Actors::C_VehicleLeftRaceMsg msg(i, fHideTime);
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&msg);
		}
	}
}

void C_RaceModeMultiplayer::DisableVehiclesLostDuringLoad(void)
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (!services.IsInitialised())
		return;

	RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();

	NetVehicleIndexStack networkList;
	GetNetworkVehicleList(networkList);

	const Game::C_LevelState* pLevelState = GetLevelState();
	for (S32 i=0; i<networkList.Size(); i++)
	{
		const LevelStateCar& car = pLevelState->mCars[networkList[i]];

		//	If this vehicle actually belongs to another one, it's probably an AI vehicle and as such won't have a valid C_NetPlayer structure.
		//	We'll get rid of it if its owner drops out though :)
		if(pLevelState->mCars[networkList[i]].mOwnerNetJoinId != 0)
			continue;

		RacerNet::C_NetPlayer* p = manager.FindFromJoinId(car.mNetJoinId, false);
		if (NULL == p || p->m_RaceData.m_InRaceVehicleIndex != networkList[i]) // player not found or new player joined on the same id but isn't in the race.
			VehicleLeftRace((U8)networkList[i]);
	}
}

void C_RaceModeMultiplayer::EliminateVehicle( U8 p_Index )
{
	Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(p_Index);
	if(pLogicActor)
	{
		// Nasty to broadcast the message but needs to get to any other vehicles too, so they get a scoring event for surviving.
		Actors::C_VehicleEliminationMessage elimMsg(pLogicActor->GetHandle(), p_Index);
		pLogicActor->SendMessageToActor(pLogicActor->GetHandle(), &elimMsg);
	}
}

void C_RaceModeMultiplayer::UpdateInRaceVoiceRecipients(void)
{
	PERF_EVENT("RMM.UpdateInRaceVoiceRecipients");

	const Game::C_LevelState* pLevelState = GetLevelState();
	assert(pLevelState->mNetworkRace);

	RacerNet::C_NetPlayerManager& plrManager = RacerNet::C_NetServices::Instance().PlayerManager();
	RacerNet::C_NetRouterBase& router = RacerNet::C_NetServices::Instance().Router();

	if (RacerNet::eVoiceControlStateGamePlay != plrManager.GetVoiceControlState())
		return; // some other system is currently controlling the voice recipients.

	Communicator::C_Core& voiceMgr = Communicator::C_Core::Instance();

	/*	TODO -> For now we send voice to all players who are racing all the time. race modes may dictate we send voice to only our team or bandwidth
		restrictions may mean we only send voice to a small number of the other racers. This needs to be decided.
	*/

	// build a list of remote consoles, voice data is only ever encoded for the primary remote player 
	// even if their is multiple players on that remote console
	RacerNet::NetConsoleIds remoteConsoles;
	for (S32 i=0; i<plrManager.TotalSize(); i++)
	{
		RacerNet::C_NetPlayer* rp = plrManager.Index(i);
		if (!rp->LocalPlayer())
		{
			// this will be rebuilt below.
			rp->m_VoiceEncodeArray.ClearBits();

			if (rp->PrimaryPlayer() && rp->IsInRace() && RacerNet::eNetPlayerDisconnectionTypeNone == rp->m_RaceConnection.m_DisconnectRequested)
				remoteConsoles.PushBack(rp->m_ConsoleId);
		}
	}

	// which local players should be encoding voice.
	for (S32 localIndex=0; localIndex<plrManager.LocalSize(); localIndex++)
	{
		RacerNet::C_NetPlayer* lp = plrManager.Index(localIndex);
		assert(lp->LocalPlayer());
		if (voiceMgr.IsLocalTalkerRegistered(lp->m_ControllerPort))
		{
			// local player has voice capability, which remote players should we send voice to ?
			for (S32 remote=0; remote<remoteConsoles.Size(); remote++)
			{
				// check all players on this remote console, no point sending voice if they all have this local player muted.  
				bool anyRemoteValid = false;
				for (S32 z=0; z<plrManager.TotalSize(); z++)
				{
					RacerNet::C_NetPlayer* rp = plrManager.Index(z);
					if (remoteConsoles[remote] == rp->m_ConsoleId)
					{
						if (voiceMgr.IsRemoteTalkerRegistered(rp->m_User))
						{
							if (!voiceMgr.LocalMuted(lp->m_ControllerPort, rp->m_User) && !voiceMgr.RemoteMuted(lp->m_ControllerPort, rp->m_User))
							{
								if (PlayersCanVoiceChat(lp->m_RaceConnection.m_JoinId, rp->m_RaceConnection.m_JoinId))
									anyRemoteValid = true; // this local player should send voice to this console.
							}
						}
					}
				}

				if (anyRemoteValid)
				{
					RacerNet::C_NetPlayer* rp = plrManager.FindFromConsoleId(remoteConsoles[remote], 0);
					assert(rp);
					assert(!rp->LocalPlayer());
					rp->m_VoiceEncodeArray.SetBit(localIndex);
				}
			}
		}
	}
}

/*	used in multiplayer races to determine if 2 players are on the same team. voice only gets sent to team members.
	the values passed in are the network join ids for the 2 players to test.
	if teams mode is currently not enabled for this race return true.
*/
bool C_RaceModeMultiplayer::PlayersCanVoiceChat(const U8 joinId1, const U8 joinId2)
{
	Game::C_LevelState *pLevelState = GetLevelState();
	if(pLevelState->mTeams)
	{
		return VehiclesAreTeammates(GetLocalIndexFromJoinId(joinId1), GetLocalIndexFromJoinId(joinId2));
	}
	return true;
}


void C_RaceModeMultiplayer::SetNetworkVehicleHealth(U8 p_Index, S32 p_HealthPoints)
{
	Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(p_Index);
	if(pLogicActor && pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Damage))
	{
		const C_VehicleLogicSetHealthMsg pMsg(p_Index, p_HealthPoints);
		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Damage, &pMsg);
	}
}

void C_RaceModeMultiplayer::LocalEliminationSurvival()
{
	//	Send "Survived Elimination" scoring message to all local vehicles still racing
	//NetVehicleIndexStack localRacing;
	//if(GetLocalRacingList(localRacing) == 0)
	//	return;

	//for(int i = 0; i < localRacing.Size(); i++)
	//{
	//	const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localRacing[i]);
	//	if(pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Scoring))
	//		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Scoring, E_VehicleLogicMsg_SurvivedElimination);
	//}
}

void C_RaceModeMultiplayer::FillVehicleData( const U8 p_Index )
{
	const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(p_Index);
	NetVehicleData& vehicleData = VehicleData(p_Index);

	const VehicleRaceStatistics& statistics = pLogicActor->GetStatistics();
	const VehicleScoringInfo& scoring = pLogicActor->GetScoring();

	F32 totalRaceTime = F32((Time::GetPerformanceCounter() - mRaceStartTime))/ 1000 / 1000; //	Race time in seconds

	vehicleData.m_Statistics[UI::E_MultiplayerStat_RaceTime]		= totalRaceTime;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_FastestLap]		= statistics.mFastestLapTime;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_SlowestLap]		= statistics.mSlowestLapTime;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_Fans]			= scoring.mFinalScore;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_HealthRemaining]	= pLogicActor->GetCurrentHealth();
	vehicleData.m_Statistics[UI::E_MultiplayerStat_KillStreak]		= statistics.mBestWreckStreak;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_TotalCarsWrecked]= statistics.mNumVehiclesWrecked;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_DeathStreak]		= statistics.mWorstLosingStreak;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_TimesHit]		= statistics.mNumTimesHit;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_LongestChain]	= scoring.mLongestChain;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_LongestDrift]	= statistics.mLongestDrift;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_TimesInWater]	= statistics.mNumTimesDrowned;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_PowerupsEvaded]	= statistics.mPickupsEvaded + statistics.mNumTimesShielded;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_Car2CarAggressiveHits] = statistics.mCarsRammed;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_TopSpeed]		= statistics.mFastestSpeedInMPH;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_EnvironmentalCrashes] = statistics.mTimesCrashed;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_TimesOffEdge]	= statistics.mNumTimesOffEdge;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_LongestTimeWithoutBeingWrecked]	= statistics.mLongestTimeWithoutWreck;

	// Mod stats
	vehicleData.m_Statistics[UI::E_MultiplayerStat_MinesFragmentedWithMastermine]	= statistics.mMinesFragmentedWithMastermine;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_PlayersScrambled]				= statistics.mPlayersScrambled;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_CarsBatteringRammed]				= statistics.mCarsBatteringRammed;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_TimesProtectedByStableFrame]		= statistics.mTimesProtectedByStableFrame;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_PlayersHitWithIronFist]			= statistics.mPlayersHitWithIronFist;

	// Specific powerup hit stats
	vehicleData.m_Statistics[UI::E_MultiplayerStat_PlayersHitwithDecoys]			= statistics.mPlayersHitwithDecoys;	
	vehicleData.m_Statistics[UI::E_MultiplayerStat_PowerupsShielded]				= statistics.mNumTimesShielded;	
	vehicleData.m_Statistics[UI::E_MultiplayerStat_BlockedPowerups]					= statistics.mPowerupsBlocked;	
	vehicleData.m_Statistics[UI::E_MultiplayerStat_PowerupsAbsorbedWithSafetyNet]	= statistics.mPowerupsAbsorbedBySafetyNet;	

	// Misc
	vehicleData.m_Statistics[UI::E_MultiplayerStat_NitroOvertakes]			    	= statistics.mNitroOvertakes;	
	vehicleData.m_Statistics[UI::E_MultiplayerStat_PowerupsDropped]			    	= statistics.mNumPowerupsDropped;	
	vehicleData.m_Statistics[UI::E_MultiplayerStat_TimeInFirstPlace]			   	= statistics.mTimeInFirstPlace;	
	vehicleData.m_Statistics[UI::E_MultiplayerStat_TimeInvisibleWithSilentRunning] 	= statistics.mTimeStealthed;	
	vehicleData.m_Statistics[UI::E_MultiplayerStat_LongestTimeNotDamaged] 			= statistics.mLongestTimeNotDamaged;	

	Game::C_MultiplayerProfile* pMultiplayerProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetMultiplayerProfile();
	if(pMultiplayerProfile)
	{
		vehicleData.m_Statistics[UI::E_MultiplayerStat_LongestJump] 				= statistics.mLongestDistanceTraveledInJump;	
	}

	//	Get the total distance travelled by the vehicle (along the track - going in circles doesn't count!)
	F32 logicalDistance = pLogicActor->GetPositionInfo().mLogicPositionCombined;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_LogicPositionCombined] = logicalDistance;
	//	Combined logical distance is 1.0 when you initially cross the start line,
	//	so anything less than 1.0 means the vehicle hasn't started the race yet
	if(logicalDistance < 1)
		logicalDistance = 0;
	else
		logicalDistance = logicalDistance - 1;	//	Gets rid of the extra 1 and gives us the logical distance from the start of the first lap

	//	Find the actual physical gameworld distance travelled in metres
	vehicleData.m_Statistics[UI::E_MultiplayerStat_DistanceTravelled] = (logicalDistance * AIAmax::AmaxAIR::Instance().DistanceFromStartOfSpline(1));

	//	Find the number of laps completed
	S8 lapsCompleted = pLogicActor->GetLapInfo().mCurrentLap - 1;
	if(lapsCompleted < 0)
		lapsCompleted = 0;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_LapsCompleted]	= lapsCompleted;

	for( int i = 0; i < statistics.mNumPerksHit.Size(); i++ )
	{
		if( !CheckPickupTypeAggressive((E_PICKUP_TYPE)i) )
		{
			continue;
		}
		
		vehicleData.m_Statistics[UI::E_MultiplayerStat_PowerupHits] += statistics.mNumPerksHit[i];

		switch((E_PICKUP_TYPE)i)
		{

		case E_PICKUP_TYPE_SHUNT:
			{
				vehicleData.m_Statistics[UI::E_MultiplayerStat_PlayersHitWithShunt] += statistics.mNumPerksHit[i]; 
			}
			break;

		case E_PICKUP_TYPE_BLINDFIRE:
			{
				vehicleData.m_Statistics[UI::E_MultiplayerStat_PlayersHitWithBolt] += statistics.mNumPerksHit[i];
			}
			break;

		case E_PICKUP_TYPE_BARGE:
			{
				vehicleData.m_Statistics[UI::E_MultiplayerStat_PlayersHitWithBarge] += statistics.mNumPerksHit[i];
			}
			break;

		case E_PICKUP_TYPE_BOMB:
			{
				vehicleData.m_Statistics[UI::E_MultiplayerStat_PlayersHitWithMine] += statistics.mNumPerksHit[i];
			}
			break;

		default :
			break;

		}
	}
	
	U32 numAggressivePerksFiredTotal = 0;
	for( int i = 0; i < statistics.mNumPerksFired.Size(); i++ )
	{
		if( !CheckPickupTypeAggressive((E_PICKUP_TYPE)i) )
		{
			continue;
		}

		numAggressivePerksFiredTotal += statistics.mNumPerksFired[i];
	}

	if(numAggressivePerksFiredTotal > 0)
	{
		vehicleData.m_Statistics[UI::E_MultiplayerStat_Accuracy]		= (vehicleData.m_Statistics[UI::E_MultiplayerStat_PowerupHits] / (float)numAggressivePerksFiredTotal ) * 100.0f;
	}

	vehicleData.m_Statistics[UI::E_MultiplayerStat_DamageDealt]			= statistics.mDamageDealt;
	vehicleData.m_Statistics[UI::E_MultiplayerStat_PickupsCollected]	= statistics.mTotalPickupsCollected;

}

bool C_RaceModeMultiplayer::CheckPickupTypeAggressive(E_PICKUP_TYPE currType ) const
{
	if( currType == E_PICKUP_TYPE_SHIELD ||
		currType == E_PICKUP_TYPE_REPAIR ||
		currType == E_PICKUP_TYPE_NITROUS )
	{
		return false;
	}
	else
	{
		return true;
	}

}

void C_RaceModeMultiplayer::FindPreyAndNemesis( NetVehicleIndexStack &localList, Game::C_LevelResult &levelResult )
{
	//	There's no way this check should ever fail, but just to be on the safe side...
	if(localList.Size())
	{
		U8 localVehicleIndex = localList[0];
		int preyHits = 0;
		int nemesisHits = 0;
		S8 preyIndex = -1;
		S8 nemesisIndex = -1;
		U8 preyJoinId = -1;
		U8 nemesisJoinId = -1;

		SysProfile::UserIdentifier		preyUid;
		SysProfile::UserIdentifier		nemesisUid;

		Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

		VehicleRaceStatistics& raceStats = levelResult.mCars[localVehicleIndex].mStatistics;

		for(int i = 0; i < levelResult.mNumCars; i++)
		{
			if(i == localVehicleIndex)
				continue;

			DualVehicleRaceStatistics& dualStats = raceStats.mDualVehicleStats[i];

			if(dualStats.mNumTimesHit > preyHits)
			{
				preyIndex = i;
				preyHits = dualStats.mNumTimesHit;
				preyJoinId = levelState.mCars[i].mNetJoinId;
				preyUid.m_Uid = levelState.mCars[i].mSystemPlayerId.m_Uid;
			}

			if(dualStats.mNumTimesHitBy > nemesisHits)
			{
				nemesisIndex = i;
				nemesisHits = dualStats.mNumTimesHitBy;
				nemesisJoinId = levelState.mCars[i].mNetJoinId;
				nemesisUid.m_Uid = levelState.mCars[i].mSystemPlayerId.m_Uid;
			}
		}
		C_MpResults::Instance().SetPreyAndNemesis(preyIndex, preyHits, preyJoinId, nemesisIndex, nemesisHits, nemesisJoinId ,preyUid ,nemesisUid );
	}
}

void C_RaceModeMultiplayer::SendNetResultsForVehicle( int p_Index )
{
	Game::C_LevelState *pLevelState = GetLevelState();

	//	Send out the results for this vehicle
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();

	RacerNet::C_NetMessage& netMsg = services.Router().GetOutgoingReliable();
	netMsg.CreateReliable();

	BuildNetResultsForVehicle(netMsg, p_Index);

	NetVehicleIndexStack networkList;
	if (GetNetworkVehicleList(networkList))
	{
		for (int i=0; i<networkList.Size(); i++)
		{
			RacerNet::C_NetPlayer *player = manager.FindFromJoinId(pLevelState->mCars[networkList[i]].mNetJoinId, false);
			if (player && player->PrimaryPlayer() && player->m_RaceData.m_InRaceVehicleIndex == networkList[i])
				player->SendReliable(netMsg, RacerNet::eNetMessage_VehicleFinished, false, manager.LocalPrimaryJoinId(false));
		}
	}
}

bool C_RaceModeMultiplayer::BuildNetResultsForVehicle(RacerNet::C_NetMessage& netMsg, int index)
{
	Game::C_LevelState* pLevelState = GetLevelState();

	NetVehicleData& inData = VehicleData(index);

	netMsg.WriteU8(1);											//	Only one car
	netMsg.WriteU8(pLevelState->mCars[index].mNetJoinId);			//	Write JoinId
	netMsg.WriteBuffer(&inData, sizeof(NetVehicleData));

	netMsg.TerminateMessage();

	return true;
}


bool C_RaceModeMultiplayer::HandleNetResultsForVehicle(RacerNet::C_NetMessage& msg, const U8 senderId)
{
	Game::C_LevelState* pLevelState = GetLevelState();
	Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	netGameVars.m_ReceivedRemoteRacePackets = true;

	bool ok = true;
	ok = msg.ReadOpen();
	if (ok)
	{
		U8 numCars, joinId;

		ok = ok && msg.ReadU8(numCars);		// Should only ever be 1

		for(int i=0;i < numCars; i++)
		{
			ok = ok && msg.ReadU8(joinId);

			if (ok)
			{
				S8 localIndex = GetLocalIndexFromJoinId(joinId);
				if(localIndex != -1)
				{
					NetVehicleData temp;
					msg.ReadBuffer(&temp, sizeof(temp));
					VehicleData(localIndex) = temp;

					VehicleFinished(localIndex, temp.m_State);
				}
			}
		}
		ok = ok && msg.ReadClose();
	}

	return true;
}

bool C_RaceModeMultiplayer::HandleForceTimeOutMessage(RacerNet::C_NetMessage& msg, const U8 senderId)
{
	bool ok = true;
	ok = msg.ReadOpen();
	if (ok)
	{
		U8 filler;

		ok = ok && msg.ReadU8(filler); //Empty messages are not sent.
	}
	ok = ok && msg.ReadClose();

	ForceTimeOutLocalVehicles();
	return true;

}

void C_RaceModeMultiplayer::SetupResults()
{
	mRaceInProgress = false;

	if(C_MpResults::Instance().m_ResultsValid)
		return;	//	Don't want to set up the results more than once per race...this protects us from trying to do so whenever we go back to the race results screen

	Game::C_LevelState* pLevelState = GetLevelState();

	if (pLevelState->mNumCars == 0)
		return;

	if(mIsNetworkGame)
	{
		// In network races when we go to the results screen we also open up normal voice transmission again, since game
		// data is no longer being transmitted.

		// stop updating game play data.
		Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		netGameVars.m_PacketTransferEnabled = false;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised() && services.Race().IsRunning())
		{
			RacerNet::C_NetPlayerManager& plrManager = services.PlayerManager();
			plrManager.SetVoiceControlState(RacerNet::eVoiceControlStateRaceConnection);
		}
	}

	UpdateRacePositions();

	C_MpResults::Instance().m_TotalRaceLength = pLevelState->mLaps * AIAmax::AmaxAIR::Instance().DistanceFromStartOfSpline(1);

	C_MpResults::Instance().m_ResultsValid = true;

	//	Need to update the position values in C_LevelResult - even though we don't use this for Multiplayer, the Achievement/Trophy system does.
	Game::C_LevelResult& levelResult = C_GameApp::GetInstance()->GetGame()->GetLevelResult();
	for(int i = 0; i < levelResult.mNumCars; i++)
	{
		levelResult.mCars[i].mPosition = C_MpResults::Instance().GetPlayerPosition(i);
		if(VehicleData(i).m_State == E_VEHICLE_STATE_FINISHED)
			levelResult.mCars[i].mStatistics.mFinished = true;
	}

	//	Only really applies if the event is team-based, but worth calling here to make sure the final results are correct in those cases.
	UpdateTeamScores();

	//CA - Send the ResultsCalculated message to the challenges component on the local vehicles, I was going to put this functionality in a separate 
	//function but that would mean that we'd have to include the Components.h and GameflowMessage.h in the header for this file...longer compile times
	NetVehicleIndexStack localList;
	GetLocalVehicleList(localList);
	for(int i = 0; i < localList.Size(); i++)
	{
		const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localList[i]);
		if(pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
			((Components::C_VehicleComponent_MP_Challenges*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))->CheckPostRaceChallenges();
	}

	FindPreyAndNemesis(localList, levelResult);

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_ResultsCalculated );
}

void C_RaceModeMultiplayer::QuitLevel()
{
	// Un-register this from network notifications as we are leaving the race.
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised() && services.Race().IsRunning())
	{
		RacerNet::C_NetPlayerManager& plrManager = services.PlayerManager();
		plrManager.SetVoiceControlState(RacerNet::eVoiceControlStateRaceConnection);
		services.Race().RegisterRaceLogicListener(NULL);
	}

	C_RaceModeBase::QuitLevel();
}

bool C_RaceModeMultiplayer::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	//	Since RaceModeMultiplayer is an abstract class, it won't actually be subscribed to anything and so will only
	//	handle messages if this MessageHandler is explicitly called (by one of the derived RaceModes).
	//	So no need to check ShouldHandleMessages()!

	switch(pMessage->Type())
	{
		case E_GameflowMessage_BeginRollingStart:
		{
			BeginRollingStart();
			return true;
		}

		case GameMessaging::E_GameMsg_FinalUpdate:
		{
			UpdateVehicleSpotlights();	//	Call this at the end of the frame so the spotlights don't lag behind the vehicle.
			return true;
		}

		case Game::E_VehicleMessage_Eliminated:
		{
			const Actors::C_VehicleEliminationMessage* pMsg = Messaging::MessageCast<Actors::C_VehicleEliminationMessage>(pMessage);
			int vehicleIndex = pMsg->mVehicleIndex;
			if(VehicleIsLocal(vehicleIndex) && VehicleData(vehicleIndex).IsRacing())
			{
				Game::C_LevelState* pLevelState = GetLevelState();
				if(pLevelState->mWreckedBehaviour == Game::E_AMAXWRECKED_ELIMINATED)
				{
					if(C_Game::GetInstance()->GetLevelState().mCars[vehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
						_UI_MESSAGE1(E_GameflowMessage_Wrecked, UI::GetLocalPlayerIndex(vehicleIndex));

					VehicleFinished(vehicleIndex, Game::E_VEHICLE_STATE_ELIMINATED);
					Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
					pLogicActor->PreRemoveFromRace(g_debugNetworkLeftRaceGhostTime);
				}
			}
			return true;
		}
		break;
	}
	return C_RaceModeBase::MessageHandler(pMessage);
}

void C_RaceModeMultiplayer::ProcessEvents(U8 p_LocalIndex, U8 p_NumEvents, RacerNet::C_NetMessage& p_Msg )
{
 	PERF_EVENT("Multiplayer.ProcessEvents");
	Game::C_LevelState* pLevelState = GetLevelState();
	U16 eventNum = pLevelState->mCars[p_LocalIndex].mNetMessageNos[0];
	bool anyProcessed = false;
	for(U8 i = 0; i < p_NumEvents; i++)
	{
		bool process = false;
		p_Msg.ReadU16(eventNum);
		//	Is this a "new" event?
		if(eventNum == pLevelState->mCars[p_LocalIndex].mNetMessageNos[0] + 1)
		{
			//if(!(eventNum == pLevelState->mCars[p_LocalIndex].mNetMessageNos[0] + 1))
			//{
			//	//	[GavinY] - If we drop into here then we've received a newer event than the last one we processed (good), but it's not the next one in sequence (bad).
			//	//	Ergo, the sender has probably spammed the net event manager and filled their buffer, deleting old events which we've yet to be sent.
			//	//	Shouldn't ever get in here, so this is BAD and you need to grab me.
			//	BIZ_ASSERT_MSG_SPEED(eventNum == pLevelState->mCars[p_LocalIndex].mNetMessageNos[0] + 1, "The sender has had to throw away some net events we haven't processed - GavinY would like to know :)");
			//	_UI_MESSAGE( UI::E_UiGameMsg_LobbyConnectionLost );
			//}
			process = true;
			anyProcessed = true;
		}
		U8 eventType = 0;
		p_Msg.ReadU8(eventType);
 		switch((E_NetEventType)eventType)
		{
			//	"Proper" events - things that actually effect the game
		case E_NetEventType_DroppedPickupPickedUp:
			{
				ProcessEvent_DroppedPickupPickedUp(p_Msg, process);
			}
			break;
		case E_NetEventType_RegisterShocks:
			{
				ProcessEvent_RegisterShocks(p_Msg, process);
			}
			break;
		case E_NetEventType_PickupFired:
			{
				ProcessEvent_PerkFired(p_Msg, process);
			}
			break;
		case E_NetEventType_PickupKilled:
			{
				ProcessEvent_PickupKilled(p_Msg, process);
				break;
			}
		case E_NetEventType_PickupPostDelay:
			{
				ProcessEvent_PickupPostDelay(p_Msg, process);
				break;
			}
		case E_NetEventType_PickupDroppedOnTrack:
			{
				ProcessEvent_PickupDroppedOnTrack(p_Msg, process);
			}
			break;
		case E_NetEventType_DecoyDroppedOnTrack:
			{
				ProcessEvent_DecoyDroppedOnTrack(p_Msg, process);
			}
			break;
		case E_NetEventType_HitByPickup:
			{
				ProcessEvent_HitByPerk(p_Msg, process);
			}
			break;
		case E_NetEventType_Ghosted:
			{
				ProcessEvent_Ghosted(p_Msg, process);
			}
			break;
		case E_NetEventType_Idle:
			{
				ProcessEvent_Idle(p_Msg, process);
			}
			break;
		case E_NetEventType_MineMoved:
			{
				ProcessEvent_MineMoved(p_Msg, process);
			}
			break;
		case E_NetEventType_Eliminated:
			{
				ProcessEvent_Eliminated(p_Msg, process);
			}
			break;
		case E_NetEventType_Repaired:
			{
				ProcessEvent_Repaired(p_Msg, process);
			}
			break;
		case E_NetEventType_ShieldedPickup:
			{
				ProcessEvent_ShieldedPerk(p_Msg, process);
			}
			break;
		case E_NetEventType_Score_Aggressive:
			{
				ProcessEvent_AggressiveScore(p_Msg, process);
			}
			break;
		case E_NetEventType_DamageDealt:
			{
				ProcessEvent_DamageDealt(p_Msg, process);
			}
			break;
		case E_NetEventType_SetPowerupType:
			{
				ProcessEvent_SetPowerupType(p_Msg, process);
			}
			break;
		case E_NetEventType_AbilityActivated:
			{
				ProcessEvent_AbilityActivated(p_Msg, process);
			}
			break;
		case E_NetEventType_JudgedPowerupShielded:
			{
				ProcessEvent_JudgedPowerupShielded(p_Msg, process);
			}
			break;
		case E_NetEventType_JudgedPowerupHit:
			{
				ProcessEvent_JudgedPowerupHit(p_Msg, process);
			}
			break;
		case E_NetEventType_MineDestroyedByProjectile:
			{
				ProcessEvent_MineDestroyedByProjectile(p_Msg, process);
			}
			break;
		case E_NetEventType_SetPowerupRespawnDelay:
			{
				ProcessEvent_SetPowerupRespawnDelay(p_Msg, process);
			}
			break;
		case E_NetEventType_EvadedShock:
			{
				ProcessEvent_EvadedShock(p_Msg, process);
			}
			break;
		case E_NetEventType_HitAllShockDomes:
			{
				ProcessEvent_HitEveryShock(p_Msg, process);
			}
			break;
		case E_NetEventType_SilentRunning:
			{
				ProcessEvent_SilentRunning(p_Msg, process);
			}
			break;
		default:
			{
				ProcessEventsModeSpecific(eventType, p_Msg, process);
			}
			break;
		}
		if(process)
			pLevelState->mCars[p_LocalIndex].mNetMessageNos[0] = eventNum;
	}
	return;
}

void C_RaceModeMultiplayer::ProcessEvent_SetPowerupType(RacerNet::C_NetMessage& p_Msg, bool p_Process)
{
	DataRef::C_SorceryUid powerupID;
	U8 perkType, isNext;
	p_Msg.ReadU32(powerupID);
	p_Msg.ReadU8(perkType);
	p_Msg.ReadU8(isNext);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	C_RaceLogicLayer* rl=GetRaceLogicLayer();

	::Actors::PickupPositionActor* actor=rl->GetPickupPositionActor(powerupID);

	if (actor)
	{
		E_PICKUP_TYPE newType=(E_PICKUP_TYPE)perkType;

		if(isNext == 1)
		{
			actor->SetNextPickupType(newType);
		}
		else
		{
			BIZ_ASSERT_MSG(newType!=E_PICKUP_TYPE_NONE, "Fail. Go find Norbert.");
			actor->SetPickupType(newType);
		}

	}

}
void C_RaceModeMultiplayer::ProcessEvent_SetPowerupRespawnDelay(RacerNet::C_NetMessage& p_Msg, bool p_Process)
{
	DataRef::C_SorceryUid powerupID;
	F32 respawnDelayMultiplier;
	U8  collectorID;
	p_Msg.ReadU32(powerupID);
	p_Msg.ReadU8(collectorID);
	p_Msg.ReadF32(respawnDelayMultiplier);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	C_RaceLogicLayer* rl=GetRaceLogicLayer();

	::Actors::PickupPositionActor* actor=rl->GetPickupPositionActor(powerupID);

	if (actor)
	{
		actor->ForceNetRespawnDelay(respawnDelayMultiplier,GetLocalIndexFromJoinId(collectorID)); 
	}

}

void C_RaceModeMultiplayer::ProcessEvent_DroppedPickupPickedUp(RacerNet::C_NetMessage& p_Msg, bool p_Process)
{
	U32 uniqueId;

	p_Msg.ReadU32(uniqueId);

	if(!p_Process)
		return;

	GetRaceLogicLayer()->DestroyDroppedPickupPositionActor(uniqueId);

}

void C_RaceModeMultiplayer::ProcessEvent_PerkFired( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U8 firerId, targetId;
	PickupCreationStruct pickupInfo(E_PICKUP_TYPE_NONE, E_PICKUP_SUB_TYPE_NORMAL, -1);

	C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Read_PickupFired(p_Msg, firerId, targetId, pickupInfo);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	pickupInfo.mFiringIdx = GetLocalIndexFromJoinId(firerId);
	pickupInfo.mTargetIdx = (targetId != 0) ? GetLocalIndexFromJoinId(targetId) : -1;

	//	Create pickup
	Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);

	switch(pickupInfo.mPickupType )
	{
	case E_PICKUP_TYPE_BARGE:
		{
			if(pickupInfo.mPickupSubType == E_PICKUP_SUB_TYPE_MOD)
			{
				Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(pickupInfo.mFiringIdx);
				if(pLogicActor)
				{
					// Has a last gasp been fired off.. if so we need to stop the looping sfx on the remote vehicle
					// Reset timers to 0! (that way update wont keep calling 'fire last gasp' 
					Components::C_VehicleComponent_Damage* damageComponent = (Components::C_VehicleComponent_Damage*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Damage);
					if(damageComponent)
					{
						damageComponent->GetLastGaspModData()->Initialise(GetLevelState(), pickupInfo.mFiringIdx);
						Game::C_LevelStateMultiplayerVehicleAbilities* pMultiplayerAbilities = GetLevelState()->mCars[pickupInfo.mFiringIdx].mUpgrades.GetMultiplayerAbilities();
						if(pMultiplayerAbilities)
						{
							pMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Last_Gasp, false);
						}
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

void C_RaceModeMultiplayer::ProcessEvent_RegisterShocks(RacerNet::C_NetMessage& p_Msg, bool p_Process)
{
	U8 firerId, targetId;
	U32 pickupUid;

	p_Msg.ReadU8(firerId);
	p_Msg.ReadU8(targetId);
	p_Msg.ReadU32(pickupUid);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	int firerIndex = (firerId != 0) ? GetLocalIndexFromJoinId(firerId) : SYSTEM_FIRING_INDEX;
	int targetIndex = (targetId != 0) ? GetLocalIndexFromJoinId(targetId): -1;

	if(HostIsLocal())
	{
		C_PickupsManager::Instance().GetShockManager().RegisterShocks(firerIndex, targetIndex, pickupUid);
	}

	C_PickupsManager::Instance().GetShockManager().PlayFX(firerIndex, targetIndex, pickupUid);
}

void C_RaceModeMultiplayer::ProcessEvent_PickupKilled( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U32 pickupUid;
	Game::PickupAutopsyReport autopsyReport;

	p_Msg.ReadU32(pickupUid);
	p_Msg.ReadBuffer(&autopsyReport, sizeof(autopsyReport));

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	// Convert join ids to local
	autopsyReport.mTargettedVehicleId = GetLocalIndexFromJoinId(autopsyReport.mTargettedVehicleId);
	autopsyReport.mVehicleIdHit = GetLocalIndexFromJoinId(autopsyReport.mVehicleIdHit);

#ifndef CFG_GOLD
	if(g_ReportNetworkPickupAutopsyReports)
	{
		LOGDEBUG(g_MultiplayerLog, "Hello, I am pickup %d and I died due to :", pickupUid );
		LogDebugNetworkPickupKilled(autopsyReport.mCauseOfDeath);
	}
#endif

	// Do the ninja kill here...
	Game::C_PickupsManager::Instance().NinjaKillPickup(pickupUid, autopsyReport);
}

#ifndef CFG_GOLD
void C_RaceModeMultiplayer::LogDebugNetworkPickupKilled(const E_PICKUP_CAUSE_OF_DEATH causeOfDeath) const
{
	switch(causeOfDeath)
	{
		case E_PICKUP_CAUSE_OF_DEATH_NONE :
		{
			LOGDEBUG(g_MultiplayerLog, "Im Not Dead!");
		}
		break;

		case E_PICKUP_CAUSE_OF_DEATH_HIT_VEHICLE:
		{
			LOGDEBUG(g_MultiplayerLog, "Hitting a vehicle");
		}
		break;

		case E_PICKUP_CAUSE_OF_DEATH_HIT_ENVIRONMENT:
		{
			LOGDEBUG(g_MultiplayerLog, "Hitting the environment");
		}
		break;

		case E_PICKUP_CAUSE_OF_DEATH_HIT_STATIC_OBJECT:
		{
			LOGDEBUG(g_MultiplayerLog, "hitting a static object");
		}
		break;

		case E_PICKUP_CAUSE_OF_DEATH_HIT_DYNAMIC_OBJECT:
		{
			LOGDEBUG(g_MultiplayerLog, "hitting a dynamic object");
		}
		break;

		case E_PICKUP_CAUSE_OF_DEATH_HIT_OTHER_PICKUP:
		{
			LOGDEBUG(g_MultiplayerLog, "hitting another pickup");
		}
		break;

		case E_PICKUP_CAUSE_OF_DEATH_OLD_AGE:
		{
			LOGDEBUG(g_MultiplayerLog, "My lifetime expired");
		}
		break;

		case E_PICKUP_CAUSE_OF_DEATH_RESET:		
		{
			LOGDEBUG(g_MultiplayerLog, "I was Reset(e.g. a shield overridden by a new shield)");
		}
		break;

		case E_PICKUP_CAUSE_OF_DEATH_REPLACED:	
		{
			LOGDEBUG(g_MultiplayerLog, "I was replaced by a new pickup");
		}
		break;

		case E_PICKUP_CAUSE_OF_DEATH_REPLAY:	
		{
			LOGDEBUG(g_MultiplayerLog, "REPLAY");
		}
		break;

		case E_PICKUP_CAUSE_OF_DEATH_UNKNOWN:
		{
			LOGDEBUG(g_MultiplayerLog, "UNKNOWN!");
		}
		break;

		
		default:
		{
			LOGDEBUG(g_MultiplayerLog, "OOPS... none of the known enums came through here" );
		}
		break;
	}

	LOGDEBUG(g_MultiplayerLog, ""); // new line
}
#endif // #ifndef CFG_GOLD

void C_RaceModeMultiplayer::ProcessEvent_PickupPostDelay( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U32 pickupUid;
	Maths::HMat43 targetMatrix;
	float fVal;

	p_Msg.ReadU32(pickupUid);
	p_Msg.ReadBuffer(&targetMatrix, sizeof(targetMatrix));
	p_Msg.ReadF32(fVal);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	//Maths::HVec3 pickupPos = targetMatrix.AW();
	//if(pickupPos.X() == .0f && pickupPos.Y() == .0f && pickupPos.Z() == .0f)
	//	BIZ_ASSERT_MSG_SPEED(false, "post delay matrix has translation at the origin");

	C_PickupBase* pPickup = C_PickupsManager::Instance().GetPickup(pickupUid);
	//	This check shouldn't fail but just to be on the safe side
	//BIZ_ASSERT_MSG_SPEED(pPickup, "Couldn't find the pickup that this post delay matrix belongs to");
	if(pPickup)
	{
			pPickup->SetPostDelayMatrix(targetMatrix, fVal);
	}
}

void C_RaceModeMultiplayer::ProcessEvent_PickupDroppedOnTrack( RacerNet::C_NetMessage& p_Msg, bool p_Process)
{
	U32 uniqueId;
	U8 pickupType;
	U8 joinID;
	Maths::HMat43 positionMat;

	p_Msg.ReadU32(uniqueId);
	p_Msg.ReadU8(pickupType);
	p_Msg.ReadU8(joinID);
	p_Msg.ReadBuffer(&positionMat, sizeof(positionMat));

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 localIndex = GetLocalIndexFromJoinId(joinID);

	GetRaceLogicLayer()->DropPickup(uniqueId, localIndex, E_PICKUP_TYPE(pickupType), positionMat);
}

void C_RaceModeMultiplayer::ProcessEvent_DecoyDroppedOnTrack( RacerNet::C_NetMessage& p_Msg, bool p_Process)
{
	U32 uniqueId;
	U8 pickupType;
	U8 joinID;
	Maths::HMat43 positionMat;

	p_Msg.ReadU32(uniqueId);
	p_Msg.ReadU8(pickupType);
	p_Msg.ReadU8(joinID);
	p_Msg.ReadBuffer(&positionMat, sizeof(positionMat));

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 localIndex = GetLocalIndexFromJoinId(joinID);

	GetRaceLogicLayer()->DropDecoyPickup(uniqueId, localIndex, E_PICKUP_TYPE(pickupType), positionMat);
}

void C_RaceModeMultiplayer::ProcessEvent_HitByPerk(RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U8 joinID, fireJoinID;
	E_PICKUP_TYPE perkType;
	E_PICKUP_SUB_TYPE subType;
	E_PICKUP_HIT_TYPE hitType;
	E_TARGETTING_DIRECTION hitDirection;
	U32 perkUID, groupId;
	PickupHitInfo hitInfo;

	C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Read_HitByPickup(p_Msg, joinID, fireJoinID, perkType, subType, hitType, hitDirection, perkUID, groupId, hitInfo);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 hitIndex = GetLocalIndexFromJoinId(joinID);
	U8 firerIndex = (fireJoinID != 0) ? GetLocalIndexFromJoinId(fireJoinID) : SYSTEM_FIRING_INDEX;

	const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(hitIndex);
	if(pLogicActor && pLogicActor->pPickupComponent)
	{
		pLogicActor->pPickupComponent->RegisterHitByPickup(perkType, subType, hitType, firerIndex, perkUID, hitDirection, false, hitInfo, groupId);
	}

	if(VehicleIsLocal(firerIndex))
	{
		const Actors::C_VehicleLogicActor* pFiringLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(firerIndex);
		if(pFiringLogicActor && pFiringLogicActor->pNewScoringComponent)
		{
			pFiringLogicActor->pNewScoringComponent->HandleMotorMashMessagePickupHit(hitIndex, firerIndex, perkType, subType);
		}
	}

	// If this is the host machine, then we need to keep track of people hit by shocks, so we can accurately award shock evader points/challenge progress
	if(HostIsLocal() && (perkType == E_PICKUP_TYPE_SHOCK) )
	{
		C_PickupsManager::Instance().GetShockManager().RegisterNetworkShockHit( hitIndex, perkUID, hitType ); 
	}
}

void C_RaceModeMultiplayer::ProcessEvent_Ghosted(RacerNet::C_NetMessage& p_Msg, bool p_Process)
{
	U8 joinID;
	F32 ghostTime;
	p_Msg.ReadU8(joinID);
	p_Msg.ReadF32(ghostTime);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 localIndex = GetLocalIndexFromJoinId(joinID);
	Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localIndex);
	if(pLogicActor)
		pLogicActor->Ghost(ghostTime);
}

void C_RaceModeMultiplayer::ProcessEvent_Idle(RacerNet::C_NetMessage& p_Msg, bool p_Process)
{
	U8 joinID;
	U8 idle;
	p_Msg.ReadU8(joinID);
	p_Msg.ReadU8(idle);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 localIndex = GetLocalIndexFromJoinId(joinID);
	VehicleData(localIndex).m_Idle=idle==1;
}

void C_RaceModeMultiplayer::ProcessEvent_MineMoved( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U8 ownerID;
	U32 mineID;
	F32 posX, posY, posZ;
	p_Msg.ReadU8(ownerID);
	p_Msg.ReadU32(mineID);
	p_Msg.ReadF32(posX);
	p_Msg.ReadF32(posY);
	p_Msg.ReadF32(posZ);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	//U8 localIndex = GetLocalIndexFromJoinId(ownerID);
	//Game::C_PickupMine* pMine = GetRaceLogicLayer()->GetMine(localIndex, mineID);
	//if(pMine)
	//{
	//	Maths::HVec3 minePos(posX, posY, posZ);
	//	pMine->mpCollisionBody->GetBody().m_Position = minePos;
	//	pMine->mForcedPositionUpdate = true;
	//}
}

void C_RaceModeMultiplayer::ProcessEvent_Eliminated(RacerNet::C_NetMessage& p_Msg, bool p_Process)
{
	U8 numEliminated;
	NetVehicleIndexStack eliminatedNetJoinIds;

	p_Msg.ReadU8(numEliminated);

	for(int i = 0; i < numEliminated; i++)
	{
		U8 joinId;
		p_Msg.ReadU8(joinId);
		eliminatedNetJoinIds.PushBack(joinId);
	}

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	for(int i = 0; i < eliminatedNetJoinIds.Size(); i++)
	{
		U8 localIndex = GetLocalIndexFromJoinId(eliminatedNetJoinIds[i]);

		if(VehicleIsLocal(localIndex))
		{
			VehicleFinished(localIndex, E_VEHICLE_STATE_ELIMINATED);
		}
	}
	//	Give any local players who survived a bonus
	//LocalEliminationSurvival();
}

void C_RaceModeMultiplayer::ProcessEvent_MineDestroyedByProjectile( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	E_PICKUP_TYPE pickupKillerType, pickupVictimType; 
	E_PICKUP_SUB_TYPE pickupKillerSubType, pickupVictimSubType; 
	U8 pickupKillerOwnerJoinID, pickupVictimOwnerJoinID; 
	U32 pickupKillerActorID, pickupVictimActorID; 

	C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Read_MineDestroyedByProjectile(p_Msg, pickupKillerType, pickupKillerSubType, pickupKillerOwnerJoinID, pickupKillerActorID,
		pickupVictimType, pickupVictimSubType, pickupVictimOwnerJoinID, pickupVictimActorID);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	// Get vehicle IDs from join IDs
	U8 pickupKillerOwnerId = GetLocalIndexFromJoinId(pickupKillerOwnerJoinID);
	U8 pickupVictimOwnerId = GetLocalIndexFromJoinId(pickupVictimOwnerJoinID);

	// Build message
	C_VehicleLogicMsg_DestroyedPickupWithPickup mineDestroyedMsg( pickupKillerType, pickupKillerSubType, pickupKillerOwnerId, pickupKillerActorID, 
																  pickupVictimType, pickupVictimSubType, pickupVictimOwnerId, pickupVictimActorID);

	// Send the message to interested parties 
	if(VehicleIsLocal(pickupKillerOwnerId))
	{
		const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(pickupKillerOwnerId );
		if(pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
		{
			//Tell the multiplayer challenges component 
			pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &mineDestroyedMsg);
		}
	}
	if(VehicleIsLocal(pickupVictimOwnerId))
	{
		const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(pickupVictimOwnerId );
		if(pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
		{
			//Tell the multiplayer challenges component 
			pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &mineDestroyedMsg);
		}
	}
}

void C_RaceModeMultiplayer::ProcessEvent_Repaired( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U8 joinID;
	p_Msg.ReadU8(joinID);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 localIndex = GetLocalIndexFromJoinId(joinID);
	const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localIndex);
	if(pLogicActor && pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Pickups))
	{
		C_VehicleLogicRepairMsg repairVehicleMsg(localIndex, 0);
		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Damage, &repairVehicleMsg);
	}
}

void C_RaceModeMultiplayer::ProcessEvent_ShieldedPerk( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U8 shieldedJoinId, firerJoinId, pickupType;
	U32 pickupUid;
	p_Msg.ReadU8(shieldedJoinId);
	p_Msg.ReadU8(firerJoinId);	
	p_Msg.ReadU8(pickupType);
	p_Msg.ReadU32(pickupUid);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 shieldedIndex = GetLocalIndexFromJoinId(shieldedJoinId);
	U8 firerIndex = (firerJoinId != 0) ? GetLocalIndexFromJoinId(firerJoinId) : SYSTEM_FIRING_INDEX;

	const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(shieldedIndex);
	if(pLogicActor && pLogicActor->pPickupComponent)
		pLogicActor->pPickupComponent->RegisterShieldedPickup((E_PICKUP_TYPE)pickupType, firerIndex, pickupUid);
}

void C_RaceModeMultiplayer::ProcessEvent_AggressiveScore( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U8 aggressorJoinId, victimJoinId;
	Components::BlurScoreMoveSummary scoreSummary;

	C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Read_AggressiveAction(p_Msg, aggressorJoinId, victimJoinId, scoreSummary);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 aggressorIndex = GetLocalIndexFromJoinId(aggressorJoinId);
	U8 victimIndex = GetLocalIndexFromJoinId(victimJoinId);

	C_VehicleLogicMsg_AggressiveScore scoreEventMsg(aggressorIndex, victimIndex, scoreSummary);
	//	Sticker manager needs to know about this regardless of whether the  aggressor was local,
	//	as our viral achievement needs to know when anyone was wrecked in order to keep it *NSYNC
	Game::C_StickerManager::Instance().MessageHandler(&scoreEventMsg);

	if(VehicleIsLocal(aggressorIndex))
	{
		const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(aggressorIndex);
		if(pLogicActor)
		{	
			pLogicActor->SendMessageToActor(pLogicActor->GetHandle(), &scoreEventMsg);			
		}
	}
}

void C_RaceModeMultiplayer::ProcessEvent_DamageDealt(RacerNet::C_NetMessage& p_Msg, bool p_Process)
{
	U8 damagerId, damagedId;
	S16 damage;

	p_Msg.ReadU8(damagerId);
	p_Msg.ReadU8(damagedId);
	p_Msg.ReadS16(damage);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 damagerIndex = (damagerId != 0) ? GetLocalIndexFromJoinId(damagerId) : SYSTEM_FIRING_INDEX;
	U8 damagedIndex = GetLocalIndexFromJoinId(damagedId);

	if(Game::VehicleIsLocal(damagerIndex))
	{
		Game::Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(damagerIndex);
		pLogicActor->GetStatistics().mDamageDealt += damage;
		if(damagerIndex != SYSTEM_FIRING_INDEX)
			pLogicActor->GetStatistics().mDualVehicleStats[damagedIndex].mNumTimesHit++;

		if(g_ReportDamageMessageActedUpon)
		{
			LOGDEBUG(g_MultiplayerLog, "Hello, I am vehicle %d and I've been told I've done some damage, that damage was %d.", damagerIndex, damage);
		}
	}
}
void C_RaceModeMultiplayer::ProcessEvent_AbilityActivated(RacerNet::C_NetMessage& p_Msg, bool p_Process)
{
	U8 vehicleId;
	U8 targetId;
	U8 abilityAndAlternateFlag;

	p_Msg.ReadU8(vehicleId);
	p_Msg.ReadU8(targetId);
	p_Msg.ReadU8(abilityAndAlternateFlag);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 vehicleIndex = GetLocalIndexFromJoinId(vehicleId);
	U8 targetIndex = GetLocalIndexFromJoinId(targetId);

	bool alternateFx = false;
	U8 ability = abilityAndAlternateFlag;
	if(abilityAndAlternateFlag & (1 << 7))
	{
		alternateFx = true;
		ability = (abilityAndAlternateFlag & (~(1 << 7)));
	}

	U8 playersAbilitiesIndex = vehicleIndex;
	if(ability == UI::E_MultiplayerVehicleAbilities_Scrambler)
	{
		playersAbilitiesIndex = targetIndex;
	}

	Game::C_LevelState* pLevelState = GetLevelState();

	pLevelState->mCars[playersAbilitiesIndex].mUpgrades.GetMultiplayerAbilities()->SetAbilityFiring((UI::E_MultiplayerVehicleAbilities)ability, true, false, vehicleIndex, 
		targetIndex, alternateFx);

	// Culprit: [J.Bunner] Dirty check (eww) for last gasp firing as we need to loop some external 
	// special effects that require update timers during the last gasp countdown...
	if( (UI::E_MultiplayerVehicleAbilities)ability == UI::E_MultiplayerVehicleAbilities_Last_Gasp &&
         !Game::VehicleIsLocal(vehicleIndex) ) // err dont think i need this check
		
	{	
		// Get pointer to damage component, access LastGaspModData struct and call setupTimers()
		const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
		if(pLogicActor && pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Damage))
		{
			Components::C_VehicleComponent_Damage* damageComponent = (Components::C_VehicleComponent_Damage*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Damage);
			damageComponent->GetLastGaspModData()->SetupTimers();
		}
	}
}

void C_RaceModeMultiplayer::ProcessEvent_JudgedPowerupShielded( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U8 shieldedId, firerId, pickupType;
	U32 pickupId;
	
	p_Msg.ReadU8(shieldedId);
	p_Msg.ReadU8(firerId);
	p_Msg.ReadU8(pickupType);
	p_Msg.ReadU32(pickupId);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 shieldedIndex = GetLocalIndexFromJoinId(shieldedId);
	if(VehicleIsLocal(shieldedIndex))
	{
		U8 firingIndex = (firerId != 0) ? GetLocalIndexFromJoinId(firerId) : SYSTEM_FIRING_INDEX;
		const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(shieldedIndex);
		if(pLogicActor && pLogicActor->pPickupComponent)
		{
			pLogicActor->pPickupComponent->RegisterShieldedPickup((E_PICKUP_TYPE)pickupType, firingIndex, pickupId);
		}
	}
}

void C_RaceModeMultiplayer::ProcessEvent_JudgedPowerupHit( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U8 joinID, fireJoinID;
	E_PICKUP_TYPE perkType;
	E_PICKUP_SUB_TYPE subType;
	E_PICKUP_HIT_TYPE hitType;
	E_TARGETTING_DIRECTION hitDirection;
	U32 perkUID, groupId;
	PickupHitInfo hitInfo;

	C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Read_HitByPickup(p_Msg, joinID, fireJoinID, perkType, subType, hitType, hitDirection, perkUID, groupId, hitInfo);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 victimIndex = GetLocalIndexFromJoinId(joinID);
	if(VehicleIsLocal(victimIndex))
	{
		U8 firingIndex = (fireJoinID != 0) ? GetLocalIndexFromJoinId(fireJoinID) : SYSTEM_FIRING_INDEX;
		const Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(victimIndex);
		if(pLogicActor && pLogicActor->pPickupComponent)
		{
			pLogicActor->pPickupComponent->AttemptHitByPickup(perkType, subType, hitType, firingIndex, hitDirection, perkUID, false, hitInfo, groupId);
		}
	}
}

void C_RaceModeMultiplayer::ProcessEvent_HitEveryShock( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U8 firerId;
	p_Msg.ReadU8(firerId);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 localFirerId = GetLocalIndexFromJoinId(firerId);
	if(VehicleIsLocal(localFirerId))
	{
		C_VehicleLogicMsg_HitWithAllDomes msg(localFirerId);
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&msg);
	}
}

void C_RaceModeMultiplayer::ProcessEvent_EvadedShock( RacerNet::C_NetMessage& p_Msg, bool p_Process )
{
	U8 evaderId, firerId;
	p_Msg.ReadU8(evaderId);
	p_Msg.ReadU8(firerId);

	//	If we don't want to actually act on this data, just return
	if(!p_Process)
		return;

	U8 localEvaderId = GetLocalIndexFromJoinId(evaderId);
	U8 localFirerId = GetLocalIndexFromJoinId(firerId);

	if(VehicleIsLocal(localEvaderId))
	{
		Game::Actors::C_VehicleLogicActor* pLogicActor = GetRaceLogicLayer()->GetLogicVehicleActor(localEvaderId);
		BIZ_ASSERT_MSG(pLogicActor, "C_RaceModeMultiplayer::ProcessEvent_ShockEvaded < Failed to get pLogicactor");

		C_VehicleLogicMsg_EvadedPowerUp evadeMsg(localEvaderId, localFirerId, E_PICKUP_TYPE_SHOCK, 0);
		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_NewScoring, &evadeMsg);	

		C_VehicleLogicMsg_AvoidedAllDomes avoidMsg(localFirerId, localEvaderId);
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&avoidMsg);
	}
}

void C_RaceModeMultiplayer::ProcessEvent_SilentRunning(RacerNet::C_NetMessage& p_Msg, bool process)
{
	U8 joinId, firing;
	
	p_Msg.ReadU8(joinId);
	p_Msg.ReadU8(firing);

	//	If we don't want to actually act on this data, just return
	if(!process)
		return;

	// BUG:35623 Once the primary vehicle is finished, no car shows silent running any more. Dont process any sfx. 
	if(mPrimaryVehicleFinished)
	{
		return;
	}

	Game::C_LevelState *pLevelState = GetLevelState();

	bool engageCloak = firing ? true : false;
	S8 localVehicleIndex = GetLocalIndexFromJoinId(joinId);
	if(localVehicleIndex >= 0 && localVehicleIndex < pLevelState->mCars.Size())
	{
		pLevelState->mCars[localVehicleIndex].mUpgrades.GetMultiplayerAbilities()->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Silent_Running, engageCloak, false, localVehicleIndex);
	}
}

/*----------------------------------------------	RESULTS SORTING	-------------------------------------------*/

void C_RaceModeMultiplayer::SortAllNetJoinIds( NetVehicleIndexStack& out ) const
{
	out.Clear();

	Game::C_LevelState* pLevelState = GetLevelState();
	//Push indices onto stack
	for (U32 i=0; i< pLevelState->mNumCars; i++)
		out.PushBack(i);

	assert(out.Size());  

	// Sort stack in ascending Net Join ID order
	for (S32 i=0; i < out.Size()-1; i++)
	{
		S32 swap = -1;
		U8 lowestJoinRef = pLevelState->mCars[out[i]].mNetJoinId;
		for (S32 j=i+1; j<out.Size(); j++)
		{
			if (lowestJoinRef > pLevelState->mCars[out[j]].mNetJoinId)
			{
				lowestJoinRef = pLevelState->mCars[out[j]].mNetJoinId;
				swap = j;
			}
		}

		if (swap != -1)
			Btl::Swap(out[i], out[swap]);
	}
}

void C_RaceModeMultiplayer::SortByStatistic( NetVehicleIndexStack& out, UI::E_MultiplayerStat stat, bool greatestFirst ) const
{
	for (S32 i=0; i < out.Size()-1; i++)
	{
		S32 swap = -1;
		F32 currentBest = VehicleData(out[i]).m_Statistics[stat];
		for (S32 j=i+1; j<out.Size(); j++)
		{
			if((greatestFirst && currentBest < VehicleData(out[j]).m_Statistics[stat]) ||
				(!greatestFirst && currentBest > VehicleData(out[j]).m_Statistics[stat]))
			{
				currentBest = VehicleData(out[j]).m_Statistics[stat];
				swap = j;
			}
		}

		if (swap != -1)
			Btl::Swap(out[i], out[swap]);
	}
}

void C_RaceModeMultiplayer::CopyFromStackIntoResults( NetVehicleIndexStack& stack, S8& startPos )
{
	Game::C_LevelState *pLevelState = GetLevelState();
	MpVehiclesData& vehicleResults = C_MpResults::Instance().m_SortedVehicleResults;
	for(S32 i=0; i < stack.Size(); i++)
	{
		S32 thisPos = startPos++;
		S32 index = stack[i];

		NetVehicleData& result = vehicleResults[thisPos];
		result = VehicleData(index);
		result.m_Position = thisPos+1;
		result.m_LocalIndex = index;

		// trueskill crap for xbox360.
		result.m_SkillRating = pLevelState->mNumCars - thisPos;		// higher rating means player has better skill than player with lower rating.
		result.m_SkillTeam = thisPos;								// no team modes yet, so each player gets an unique team id.
	}
}

void C_RaceModeMultiplayer::UpdateTeamScores()
{
	if(!GetLevelState()->mTeams)
		return;

	S8 previousTeam = C_MpResults::Instance().GetWinningTeam();
	C_MpResults::Instance().ClearTeamScores();
	TeamsData& teams = C_MpResults::Instance().m_Teams;
	for(U32 i = 0; i < GetLevelState()->mNumCars; i++)
	{
		NetVehicleData& vehicleData = C_MpResults::Instance().m_SortedVehicleResults[i];
		TeamData(vehicleData.m_Team).m_TeamScore += GetVehicleTeamScoreContribution(vehicleData);
		TeamData(vehicleData.m_Team).m_RacerOrder.PushBack(i);
	}
	S8 newTeam = C_MpResults::Instance().GetWinningTeam();
	
	//	Inform the UI if a team has just taken the lead
	if(newTeam >= 0 && newTeam != previousTeam)
	{
		UI::C_UiScoringEventPerk uimsg1( UI::C_TextStringID( newTeam == 0 ? "MP_TEAM_A_LEADING" : "MP_TEAM_B_LEADING").String(), 0, 0, 0, 0 );
		_UI_MESSAGE_FULL( uimsg1 );
	}
}

NetVehicleData& C_RaceModeMultiplayer::VehicleData( U8 p_Index ) const
{
	return C_MpResults::Instance().m_LiveVehicleData[p_Index];
}

NetTeamData& C_RaceModeMultiplayer::TeamData( U8 p_Index ) const
{
	BIZ_ASSERT(p_Index < C_MpResults::Instance().m_Teams.Size());
	return C_MpResults::Instance().m_Teams[p_Index];
}

} // namespace Game
