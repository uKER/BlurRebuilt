#include "stdafx.h"
#include "GameLogic/GameModes/GameModeNetwork.h"

#include <GameLogic/GameModes/GameModeDefines.h>
#include <GameLogic/GameData/GameData.h>
#include <GameLogic/GameData/MultiplayerData.h>
#include <GameLogic\RaceModes\RaceModeMultiplayer.h>
#include <GameLogic\RaceRules\RaceRulesManager.h>
#include <GameLogic\Structures\MpResults.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Structures\LevelState.h>


#include <Network/Core/NetServices.h>
#include <Network/Core/NetRace.h>
#include <Network/Core/NetJoinHelper.h>
#include <Network/Core/NetLogger.h>
#include <Network/Player/NetPlayer.h>
#include <Network/Router/NetRouterBase.h>
#include <Network/Player/NetPlayerManager.h>
#include <Network/Misc/NetRaceSettings.h>
#include <Network/Misc/NetRaceSettingsQos.h>
#include <Network/Message/NetMessage.h>
#include <Network/Misc/NetDatagramPacketTypes.h>

#include <GameApp.h>
#include <Game/Game.h>

#include <System/Misc/Random.h>
#include <Ui\UiManager.h>
#include <Ui/Game/Multiplayer/MultiplayerCache.h>
#include <Ui\game\Multiplayer\MultiplayerHelpers.h>

#include <GameLogic/AchievementsManager.h>
#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic\Profile\GameProfile.h>
#include <GameLogic/Profile/MultiplayerProfile.h>
#include <GameLogic/RaceRules/Multiplayer/MpRaceActionManager.h>

#include <Ui\game\Multiplayer\MultiplayerCache.h>

#include "Automation\GameDataLogging.h"
#include <RaceBook/Data/RaceBookData.h>
#include <GameLogic\GameLoggingManager.h>

#include <Gamelogic\SocialNetworking\SocialNetworkManager.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game/Messaging/GameflowMessages.h>

#include <GameLogic/StickerManager.h>

#include <utilityai/utilityai.h>

#include "UI/UIMessages.h"

#include <GameLogic/SinglePlayer/SinglePlayer.h> // For single player BOSSES!

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

Debug::Int g_debugMPLocalCarTextureDetail("Vehicle.LOD.MultiplayerLocalDetail", 10, Debug::Range(1, 10), Debug::Step(1));
Debug::Int g_debugMPRemoteCarTextureDetail("Vehicle.LOD.MultiplayerRemoteDetail", 7, Debug::Range(1, 10), Debug::Step(1));
Debug::Bool gDecreaseFanMultiplier("Multiplayer.DecreaseFanMultiplier", false);
Debug::Float gRacePreviewSortTime("Multiplayer.RacePreviewSortTime", 0.5f );

namespace Game
{
	C_GameModeNetwork::C_GameModeNetwork()
		: C_GameModeMultiPlayer()
	{
		SetInternalNameAndID("Network", E_GameMode_Network);	
	}

	void C_GameModeNetwork::Initialise()
	{
		C_GameModeMultiPlayer::Initialise();

		Default();
	}

	void C_GameModeNetwork::Update()
	{
		// enter critical section with this logic as it runs when loading. 
		// must stop it conflicting with the network loading thread.
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		services.EnterLock();
		if (NetValid())
		{
			// game mode specific net logic
			MonitorSelectedVehicles();
			UpdateLobbyRaceInfo();

			// Take the timeout down locally. It is set to -1.0f when not active.
			LobbyWaitingRaceInfo& infoForLobby = C_GameApp::GetInstance()->GetGame()->GetNetGameVars().GetLobbyWaitingRaceInfo();
			if ( infoForLobby.m_Timeleft >= 0.0f )
			{
				infoForLobby.m_Timeleft -= (F32)C_GameApp::GetInstance()->GetTimer().GetFrameTime();

				// Clamp it at 0.0f
				if ( infoForLobby.m_Timeleft < 0.0f )
					infoForLobby.m_Timeleft = 0.0f;
			}

			m_RaceInfoSortTimer -= (F32)C_GameApp::GetInstance()->GetTimer().GetFrameTime(); 
			if ( m_RaceInfoSortTimer <= 0 ) 
			{
				m_RaceInfoSortTimer = gRacePreviewSortTime;
				SortLobbyRaceInfo(); // Sort Lobby data
			}
		}
		services.Exitlock();
	}

	void C_GameModeNetwork::Default(void)
	{
		m_Settings.Clear();
		m_RaceInfoSortTimer = gRacePreviewSortTime;
	}

	bool C_GameModeNetwork::IsHost()
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		RacerNet::C_NetPlayer* hostPlayer = services.PlayerManager().HostPlayer(false);

		if ( hostPlayer && hostPlayer->LocalPlayer() )
			return true;

		return false;
	}

	void C_GameModeNetwork::Start()
	{
		Default();

		C_RaceRulesManager::Instance().ClearAllRules();
	}

	void C_GameModeNetwork::StartRace()
	{
		EnableChangeViewOnAllViewports();

		if(IsOnline())
		{
			if(IsRanked())
			{
				C_GameProfile* pProf = C_GameProfileManager::Instance().GetPrimaryProfile();
				if(pProf)
				{
					pProf->mMultiplayerProfile.PreRaceUpdate();
				}
			}
			//C_AchievementsManager::Instance().PreRace();
			C_SocialNetworkManager::Instance().StoreDataPreRace();
		}
		m_RaceAwards.ResetSelection();

		UpdateLobbyState( UI::E_LobbyState_Waiting, 0.0f );
		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().GetLobbyWaitingRaceInfo().Reset();
	}

	void C_GameModeNetwork::InterpretRaceResults()
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		//	We want awards in any network MP, including LAN play.
		CalculateAwards();
		//	Update the temp stats we only hold for this session (winning streak and stuff for session scoreboard)
		C_GameProfileManager::Instance().GetPrimaryProfile()->mMultiplayerProfile.GetMultiplayerStatistics().SessionStatsUpdate();

		if(IsOnline())
		{
			if(IsRanked())
			{//	Only update the MP profile if we've finished a ranked game
				C_GameProfileManager::Instance().GetPrimaryProfile()->mMultiplayerProfile.PostRaceUpdate();
				C_StickerManager::Instance().CheckMpRaceFinishedStickers();
			}

			Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
			Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();
			Game::C_GameProfile* pGameProfile = 0;
			Game::C_MultiplayerProfile* pProfile = 0;
			Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();

			if(pLayer && IsHost())
			{
				pLayer->GetDataGathering().FlagPostEvent(DATA_GATHERING_RECORD_TYPE_VOTE_RESULTS);
			}

			int a=0;
			{
				if(levelState.mCars[a].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
				{
					NetVehicleData result;
					C_MpResults::Instance().GetPlayerResults(a,result);
	
					pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
					pProfile = pGameProfile->GetMultiplayerProfile();
					Game::C_MultiplayerChallengeTracker::TChallengeList challengesCompleted = pProfile->GetChallengeSystem().GetTracker().GetChallengesComplete();

					if(pLayer && challengesCompleted.Size() > 0)
					{
						pLayer->GetDataGathering().FlagPostEvent(DATA_GATHERING_RECORD_TYPE_CHALLENGE_COMPLETED);
					}
#ifndef CFG_GOLD
					for( Game::C_MultiplayerChallengeTracker::TChallengeList::Iterator it = challengesCompleted.Begin(); it != challengesCompleted.End(); it++ )
					{
						AmaxDataLogging::LogMpChallengeCompleted(pProfile->GetCurrentLevel()+1, pProfile->GetMultiplayerStatistics().mNumEventsCompleted, 
								pProfile->GetMultiplayerStatistics().mTotalPlaytime, (*it).type, (*it).tier, levelState.mCars[a].mCarID);

						//Have we just completed a whole pack with this challenge?
						S8 packCompleted = pProfile->GetChallengeSystem().IsPackComplete((*it).pack) ? (*it).pack : -1;

						if(packCompleted != -1)
						{
							AmaxDataLogging::LogMpChallengePackCompleted(pProfile->GetCurrentLevel()+1, pProfile->GetMultiplayerStatistics().mNumEventsCompleted, 
								pProfile->GetMultiplayerStatistics().mTotalPlaytime, packCompleted, levelState.mCars[a].mCarID);
						}
					}
#endif
					if(pLayer)
					{
						pLayer->GetDataGathering().FlagPostEvent(DATA_GATHERING_RECORD_TYPE_CAREER_INFORMATION);
					}

					PostRaceLogging(pProfile, levelState);

					//store away the last car used for the UI
					pProfile->SetLastCarUsed(levelState.mCars[a].mCarID);
				}
			}

			C_GameProfileManager::Instance().GetPrimaryProfile()->UploadPlayerStats();

			RacerNet::C_NetPlayerManager& manager = services.PlayerManager();
			RacerNet::C_NetPlayer* pHost = manager.HostPlayer(false);
			if (pHost && pHost->LocalPlayer())
			{
				// MP_TODO_CUSTOM_RACE
				if ( m_PlayMode == E_PlayMode_Playlist )
				{
					C_GameLoggingManager::Instance().UploadGameLog();
				}
			}

			if(IsOnline())
			{
				C_SocialNetworkManager::Instance().PostRace(true,true);
			}
		}

		// if network game then update players profile data and send to peers
		//	we do this even in LAN and online private games so the session scoreboard is kept up to date.
		if (services.IsInitialised())
		{
			services.EnterLock();
			if (services.Race().IsRunning())
			{
				services.Race().PostRace();
			}
			services.Exitlock();
		}
	}

	void C_GameModeNetwork::PostRaceLogging(C_MultiplayerProfile* pProfile, C_LevelState& levelState)
	{
		if(pProfile)
		{
			Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

			MultiplayerStatistics mpStats = pProfile->GetMultiplayerStatistics();
		
			F32 powerupHitRatio = 0.f;
			if(mpStats.mTotalPickupsFired > 0)
			{
				powerupHitRatio = (F32)mpStats.mTotalPickupsHit / (F32)mpStats.mTotalPickupsFired * 100.f;
			}
			Game::C_RaceBookLayer* rbl=C_GameApp::GetInstance()->GetRaceBookLayer();
#ifndef CFG_GOLD
			AmaxDataLogging::LogPlayerCareerInformation(levelState.mCars[0].mNameAboveCar.AsString().Chars(), pProfile->GetCurrentLevel()+1,mpStats.mNumRacePoints, powerupHitRatio, mpStats.mTotalPlaytime,
				mpStats.mNumLifetimeFans, mpStats.mAveragePosition, mpStats.mNumEventsPlayed, mpStats.mNumEventsWon);

			U8 rankUps = pProfile->GetCurrentLevel() - pProfile->GetPreviousLevel();
			if(rankUps)
			{
				AmaxDataLogging::LogMpRankUp(pProfile->GetCurrentLevel()+1,pProfile->GetMultiplayerStatistics().mTotalPlaytime,
					pProfile->GetMultiplayerStatistics().mNumEventsPlayed);
			}

			Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
			NetVehicleData vehicleResultData;
			C_MpResults::Instance().GetPlayerResults(0,vehicleResultData);

			bool timedOut = levelState.mCars[0].mState == Game::E_VEHICLE_STATE_TIMED_OUT;
			F32 timeoutTime = 0.f;

			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceMode() == E_RaceMode_Multiplayer_Racing)
			{
				Game::C_RaceModeMultiplayer_Racing* pRaceMode = (Game::C_RaceModeMultiplayer_Racing*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();
				if(pRaceMode)
				{
					timeoutTime = pRaceMode->GetInitialTimeoutTime();
				}
			}

			//And local logging
			RacerNet::NetPlayerLoadOut& loadout = netGameVars.m_StartRaceData.mPlayer[0].mLoadOut;
			S8 slot1 = loadout.slots[UI::E_MULTIPLAYER_LOADOUT_SLOT1];
			S8 slot2 = loadout.slots[UI::E_MULTIPLAYER_LOADOUT_SLOT2];
			S8 slot3 = loadout.slots[UI::E_MULTIPLAYER_LOADOUT_SLOT3];
			F32 averageLapTime = vehicleResultData.m_Statistics[UI::E_MultiplayerStat_RaceTime] / (F32)levelState.mLaps;
			U8 timesWrecked = results.mCars[0].mStatistics.mNumTimesWrecked;
			F32 distanceToFinish = results.mCars[0].mStatistics.mDistToFirstOnFinish;

			AmaxDataLogging::LogMpRaceInformation(m_Settings.m_Race.m_Type, timeoutTime, averageLapTime, &pProfile->GetRaceSummary(), 
				timesWrecked, distanceToFinish, timedOut);

			U32 activationSlot1 = 0; 
			U32 activationSlot2 = 0; 
			U32 activationSlot3 = 0;
			Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[0].mUpgrades.GetMultiplayerAbilities();
			if(pAbilities)
			{
				if(slot1 != -1)
					activationSlot1 = pAbilities->GetActivationTimes(UI::E_MULTIPLAYER_LOADOUT_SLOT1);
				if(slot2 != -1)
					activationSlot2 = pAbilities->GetActivationTimes(UI::E_MULTIPLAYER_LOADOUT_SLOT2);
				if(slot3 != -1)
					activationSlot3 = pAbilities->GetActivationTimes(UI::E_MULTIPLAYER_LOADOUT_SLOT3);
			}


			AmaxDataLogging::LogMpModUsage(slot1, slot2, slot3, activationSlot1, activationSlot2, activationSlot3);
#endif
			//Flag for Demonware
			rbl->GetDataGathering().FlagPostEvent(DATA_GATHERING_RECORD_TYPE_POST_RACE);
			rbl->GetDataGathering().StoreTrackPickupInformation();
		}
	}

	bool C_GameModeNetwork::GetNetRaceSettings(RacerNet::NetRaceSettings& out)
	{
		out = m_Settings;

		out.m_LobbyState = C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyState;
		out.m_LobbyTimer = C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyTimer;

		return true;
	}

	bool C_GameModeNetwork::GetNetRaceSettingsQos(RacerNet::NetRaceSettingsQos& out)
	{
		out.m_EventID		= m_Settings.m_EventId;
		out.m_CustomRace	= m_Settings.m_CustomRace;
		out.m_PackVersion	= m_Settings.m_PackVersion;
		out.m_Ranked		= m_Settings.m_Ranked;

		return true;
	}

	void C_GameModeNetwork::SetNetRaceSettings(RacerNet::NetRaceSettings& in)
	{
		m_Settings = in;

		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyState = (UI::E_LobbyState)m_Settings.m_LobbyState;
		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyTimer = m_Settings.m_LobbyTimer;

		// Need to shuffle the playlist with the hosts seed.
		if ( m_PlayMode == E_PlayMode_Playlist )
		{
			m_Playlist.Shuffle( m_Settings.m_PlaylistSeed );
			m_Playlist.SetRaceIndex( m_Settings.m_PlaylistIndex );

			// Read vote data
			m_VoteInfo.Reset();
			m_VoteInfo.countA = m_Settings.m_VoteCountA;
			m_VoteInfo.countB = m_Settings.m_VoteCountB;
			m_VoteInfo.indexA = m_Settings.m_VoteIndexA;
			m_VoteInfo.indexB = m_Settings.m_VoteIndexB;
		}
		else
		if ( m_PlayMode == E_PlayMode_CustomRace )
		{
			m_CustomRace.SetupRace( m_Settings );

			// Inform the lobby to refresh
			_UI_MESSAGE1(UI::E_UiGameMsg_RefreshLobby, false);
		}
		else
			BIZ_ASSERT_FAIL_SPEED( "Play mode is invalid" );
	}

	bool C_GameModeNetwork::ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel)
	{
		// only receive messages when we are fully joined into session
		if (!NetValid())
			return false;

		switch (msgType)
		{
		case RacerNet::eNetMessage_VehicleData:
			ProcessVehicleData(msg);
			return true;
			break;

		case RacerNet::eNetMessage_StartRace:
			ProcessStartRace(msg);
			return true;
			break;

		case RacerNet::eNetMessage_BeginVoting:
			ReceiveBeginVotingMessage(msg);
			return true;
			break;

		case RacerNet::eNetMessage_VoteToSkipNextRace:
			ProcessVoteToSkipNextRace(msg);
			return true;
			break;

		case RacerNet::eNetMessage_VoteOnNextRace:
			ProcessVoteOnNextRace(msg);
			return true;
			break;

		case RacerNet::eNetMessage_VoteResults:
			ProcessVoteResults(msg);
			return true;
			break;

		case RacerNet::eNetMessage_PostRace:
			ReceivePostRaceMessage(msg, senderId);
			return true;
		};

		return false;
	}

	bool C_GameModeNetwork::ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel)
	{
		// only receive messages when we are fully joined into session
		if (!NetValid())
			return false;

		switch (msgType)
		{
		case RacerNet::eNetMessage_LobbyUpdate:
			ReceiveLobbyUpdateMessage(msg, senderId);
			return true;
			break;

		case RacerNet::eNetMessage_LobbyWaitingRaceInfo:
			ProcessLobbyRaceInfo(msg);
			return true;
			break;
		}

		return false;
	}

	bool C_GameModeNetwork::NetValid(void)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				if (RacerNet::eJoinStateComplete == race.JoinGetState())
				{
					// fully created or joined the race session. safe to send messages
					return true;
				}
			}
		}

		return false;
	}

	void C_GameModeNetwork::ProcessStartRace(RacerNet::C_NetMessage& msg)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();

		bool ok = msg.ReadOpen();

		Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();

		// some other player will be hosting.
		netGameVars.m_RaceHost = false;

		RacerNet::NetStartRacePacket& startRacePacket = netGameVars.m_StartRaceData;

		ok = ok && msg.ReadBuffer(&startRacePacket, sizeof(startRacePacket));
		if (ok)
		{
			RacerNet::C_NetPlayerManager& playerMgr = services.PlayerManager();

			netGameVars.m_StartRaceDataValid = true;

			// update our current settings with then index of the current event. if we migrate to the host it means
			// the events will continue in order instead of resetting to first event again.
			m_Settings.m_PlaylistIndex = startRacePacket.mEvntIndex; // TODO_MP_REFACTOR

			// assume we are going to start a race.
			m_Settings.m_HasAnyRacesStarted = 1;

			// make sure local players appear in the first vehicle slots, on all kits..
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
			for (U8 i=0; i<mgr.LocalSize(); i++)
			{
				const RacerNet::C_NetPlayer* p = mgr.Index(i);
				assert(p->LocalPlayer());
				assert(p->m_RaceConnection.IsActive());

				// find this local player in the start race vehicle list.
				U8 localStarting = 0;
				for (U8 z=0; z<startRacePacket.mNumPlayerVehicles; z++)
				{
					if (p->m_RaceConnection.m_JoinId == startRacePacket.mPlayer[z].mJoinRef)
					{
						// swap
						Btl::Swap(startRacePacket.mPlayer[z], startRacePacket.mPlayer[localStarting]);
						localStarting += 1;
					}
				}
			}

			//First championship race should store all the players in the championship structure
			InitChampionshipStandingsStructure();
		}

		msg.ReadClose();

		if (!ok)
		{
			assert(0 && "C_GameModeNetwork::ProcessStartRace data error");
		}
	}

	void C_GameModeNetwork::ProcessVoteToSkipNextRace(RacerNet::C_NetMessage& msg)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		bool ok = msg.ReadOpen();
		if (ok)
		{
			U64 consoleId = 0;
			ok = ok && msg.ReadU64(consoleId);
			if (ok)
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (consoleId == p->m_ConsoleId && p->m_RaceConnection.IsActive())
						p->m_RaceData.m_BitArray.SetBit(RacerNet::NetPlayerRaceGameData::eBitArrayData_WantsToSkipNextRace);
				}
			}
		}

		msg.ReadClose();

		BIZ_ASSERT_MSG(ok, "C_GameModeNetwork::ProcessVoteToSkipNextRace data error");
	}

	void C_GameModeNetwork::ProcessVoteOnNextRace(RacerNet::C_NetMessage& msg)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		bool ok = msg.ReadOpen();
		if (ok)
		{
			U64 consoleId = 0;
			S8 vote = 0;
			ok = ok && msg.ReadU64(consoleId);
			ok = ok && msg.ReadS8(vote);
			if (ok)
			{
				VoteOnRace( vote );
			}
		}

		msg.ReadClose();

		BIZ_ASSERT_MSG(ok, "C_GameModeNetwork::ProcessVoteOnNextRace data error");
	}

	void C_GameModeNetwork::ProcessVoteResults(RacerNet::C_NetMessage& msg)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		bool ok = msg.ReadOpen();
		if (ok)
		{
			S8 vote = 0;
			S16 raceIndex = 0;
			ok = ok && msg.ReadS8(vote);
			ok = ok && msg.ReadS16(raceIndex);
			if (ok)
			{
				m_VoteInfo.result = vote;

				// just incase the clients arnt in sync with the host
				m_Playlist.SetRaceIndex( raceIndex );
				// Rewrite back into NetRaceSettings
				m_Playlist.SetupNetRaceSettings(m_Settings);
			}
		}

		msg.ReadClose();

		BIZ_ASSERT_MSG(ok, "C_GameModeNetwork::ProcessVoteResults data error");
	}

	void C_GameModeNetwork::ProcessVehicleData(RacerNet::C_NetMessage& msg)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();

		bool ok = msg.ReadOpen();
		if (ok)
		{
			U8 num = 0;
			ok = ok && msg.ReadU8(num);

			for (U8 i=0; i<num; i++)
			{
				U8 joinId = 0;

				RacerNet::NetPlayerVehicle vehicle;
				RacerNet::NetPlayerLoadOut loadOut;

				ok = ok && msg.ReadU8(joinId);
				ok = ok && msg.ReadBuffer(&vehicle, sizeof(vehicle));
				ok = ok && msg.ReadBuffer(&loadOut, sizeof(loadOut));

				if (ok)
				{
					//LOGINFO(RacerNet::g_Log, "received vehicle data from join Id %d", joinId);
					RacerNet::C_NetPlayer* p = services.PlayerManager().FindFromJoinId(joinId, false);
					if (p && p->m_RaceConnection.IsActive())
					{
						p->m_RaceData.m_VehicleData	= vehicle;
						BIZ_ASSERT_MSG_SPEED(p->m_RaceData.m_VehicleData.m_xtVehicleId != 0, "player has a vehicle ID of zero - tell GavinY");
						p->m_RaceData.m_LoadOut		= loadOut;

						//If we are locking to hosts car and this is not the host then change our local vehicles
						if(m_CustomRace.GetLockToHostCar() && !IsHost())
						{
							RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

							for (U8 j = 0; j < mgr.LocalSize(); j++)
							{
								RacerNet::C_NetPlayer* player = mgr.Index(j);
								if(player)
								{
									//Force new vehicle selection
									player->m_RaceData.m_VehicleData.m_xtVehicleId = 0;
								}
							}

							//Then refresh the lobby (And skip the game settings have changed message)
							_UI_MESSAGE1(UI::E_UiGameMsg_RefreshLobby, true);
						}
					}
				}
			}
		}

		if (!ok)
		{
			LOGERROR(RacerNet::g_Log, "C_GameModeNetwork::ProcessVehicleData() - invalid data received.");
		}
	}

	bool C_GameModeNetwork::HasVehicleUnlocked(const int currentLevel, const AmaxVehicle* pVeh) const
	{
		// sanity check
		if(!pVeh)
		{
			return false;
		}

		Btl::Map<U32, int>::Iterator it = C_MultiplayerData::Instance().m_MultiplayerVehicleMap.Find(pVeh->ID);

		if(it != C_MultiplayerData::Instance().m_MultiplayerVehicleMap.End())
		{
			// Some cars (not legend) require a rank check against the regular profile level
			bool exceedsRequiredRank = false;
			int reqLevel = it->GetValue();
			if(currentLevel >= reqLevel)
			{
				exceedsRequiredRank =  true;
			}

			// Default cars require a simple level requirement check. However, boss and legend cars require a parent vehicle check 
			bool specialCaseVehicle = (pVeh->m_ParentVehicle != NULL);

			if(specialCaseVehicle) 	
			{
				if( HasLegendVehicleUnlocked(pVeh->ID) ||							// If a legend car.. we do a special legend rank check
					( HasBossVehicleUnlocked(pVeh->ID) && exceedsRequiredRank )	 )  // If a boss car we check the parent + boss vehicle and if the rank is exceeded
				{
					return true;
				}
			}
			else
			{
				// If regular car, simply check its level requirement aginst the mpProfile level
				if(exceedsRequiredRank)
				{
					return true; 
				}
			}
		}

		// we do not have access to this vehicle
		return false;
	}


	bool C_GameModeNetwork::HasLegendVehicleUnlocked(const XtId vehID) const
	{
		// 2) Push back any unlocked legend cars this player may select 
		TLevelData& legendLevelData = C_MultiplayerData::Instance().m_pConfig->m_LegendLevelingData;
		Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
		if(!pGameProfile)
		{
			return false;
		}
		S32 currentLegendLevel = pGameProfile->GetMultiplayerProfile()->GetLegendLevel();
		for ( S32 i = 0; i < currentLegendLevel; i++ )
		{
			for ( S32 j = 0; j < legendLevelData[ i ].mVehicleUnlocks.count; j++ )
			{
				AmaxVehicle* pVehicleAM = legendLevelData[ i ].mVehicleUnlocks[ j ].Object();

				if ( !pVehicleAM )
					continue;

				if(pVehicleAM->ID == vehID)
				{
					// This vehicle is unlocked!
					return true;
				}
			}
		}

		return false;
	}

	bool C_GameModeNetwork::HasBossVehicleUnlocked(const XtId vehID) const
	{
		SP_SaveGame* pSave = SP_GetCurrentSaveGame();
		if(pSave)
		{
			return SP_IsVehicleUnlocked(pSave, vehID);
		}
		else
		{
			// oops
			return false;
		}
	}

	void C_GameModeNetwork::MonitorSelectedVehicles(void)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

		bool forcePeerUpdate = false;
		for (U8 i=0; i<mgr.LocalSize(); i++)
		{
			RacerNet::C_NetPlayer* p = mgr.Index(i);
			assert(p->LocalPlayer()); // local players should always be at the start of out player list.
			if (!p->m_RaceConnection.IsActive())
				continue;

			if (0 == p->m_RaceData.m_VehicleData.m_xtVehicleId)	//	We should only hit this once, when we join the game
			{
				// need to choose a default vehicle since we don't have one yet. probably just joined or created the session.
				ChooseNewVehicle(p->m_ControllerPort, p->m_RaceData.m_VehicleData);
				ChooseNewLoadOut(p->m_ControllerPort, p->m_RaceData.m_LoadOut);

				// peers need updating with our new vehicle.
				forcePeerUpdate = true;
			}
			else if(m_ChooseRandoms)	//	Runs after we enter the lobby between each race.
			{
				if(m_Settings.m_Misc & ( 1 << UI::E_MiscSettings_RandomCars ))
				{
					ChooseNewVehicle(p->m_ControllerPort, p->m_RaceData.m_VehicleData, true);
				}

				ChooseNewLoadOut(p->m_ControllerPort, p->m_RaceData.m_LoadOut);
				m_ChooseRandoms = false;
				forcePeerUpdate = true;
			}
			else
			{
				// validate current vehicle against race settings. 
				// its up to us to choose a new vehicle if the host changes the race settings.
				// he runs logic that will stop him starting the race until our all vehicles are valid.
				if (!services.Race().ValidateVehicle(p->m_RaceData.m_VehicleData.m_xtVehicleId))	//	Class restrictions etc may well be different from the previous race, so we might need to choose a new car
				{
					ChooseNewVehicle(p->m_ControllerPort, p->m_RaceData.m_VehicleData);
					ChooseNewLoadOut(p->m_ControllerPort, p->m_RaceData.m_LoadOut);

					forcePeerUpdate = true;
				}
			}
		}

		// check remote peers, if they have yet to get any vehicle info from me then send it to them now.
		// or send the updated vehicle data if we have forced a change.
		for (U8 i=0; i<mgr.TotalSize(); i++)
		{
			RacerNet::C_NetPlayer* p = mgr.Index(i);
			if (!p->LocalPlayer() && p->PrimaryPlayer() && p->m_RaceConnection.IsActive() && p->m_RaceConnection.CanMessage())
			{
				if (forcePeerUpdate || !p->m_RaceData.m_BitArray.GetBitValue(RacerNet::NetPlayerRaceGameData::eBitArrayData_HasMyLatestVehicleData))
				{
					// wait until remote peer has actually joined the lobby before sending data.
					// this stops us sending the peer this message while he is still in his handshaking phase.
					if (p->m_RaceConnection.m_PlayerState >= RacerNet::eNetPlayerStateIdle)
					{
						// send local vehicle data to this primary remote peer.
						RacerNet::C_NetMessage& msg = services.Router().GetOutgoingReliable();

						bool ok = msg.CreateReliable();
						const U8 numLocal = mgr.LocalSize();
						ok = ok && msg.WriteU8(numLocal);
						for (U8 z=0; z< numLocal; z++)
						{
							RacerNet::C_NetPlayer* pl = mgr.Index(z);
							BIZ_ASSERT(pl->LocalPlayer());

							BIZ_ASSERT_MSG_SPEED(pl->m_RaceData.m_VehicleData.m_CharacterId != 0, "Character ID is zero, PLEASE grab Jon B or Gavin Y.");

							BIZ_ASSERT_MSG_SPEED(pl->m_RaceData.m_VehicleData.m_xtVehicleId != 0, "player has a vehicle ID of zero - tell GavinY");

							ok = ok && msg.WriteU8(pl->m_RaceConnection.m_JoinId);
							ok = ok && msg.WriteBuffer(&(pl->m_RaceData.m_VehicleData), sizeof(pl->m_RaceData.m_VehicleData));
							ok = ok && msg.WriteBuffer(&(pl->m_RaceData.m_LoadOut), sizeof(pl->m_RaceData.m_LoadOut));
						}
						msg.TerminateMessage();

						if (ok)
						{
							LOGINFO(RacerNet::g_Log, "Sending vehicle data to %S", p->m_User.Name());
							ok = p->SendReliable(msg, RacerNet::eNetMessage_VehicleData, false, mgr.LocalPrimaryJoinId(false));
						}

						if (ok) // 
							p->m_RaceData.m_BitArray.SetBit(RacerNet::NetPlayerRaceGameData::eBitArrayData_HasMyLatestVehicleData);
					}
				}
			}
		}
	}


	bool C_GameModeNetwork::ChooseNewVehicle(const U8 userIndex, RacerNet::NetPlayerVehicle& out, const bool bRandom)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		AmaxCarClass validClass = GetRaceCarClass();
		
		// what dlc is availble to all peers plays a role in what vehicles we can actually choose. dlc sucks.
		RacerNet::NetRaceDlcSettings dlcContent;
		services.Race().GenerateDlcSettings(dlcContent);

		Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
		Game::C_MultiplayerProfile* pMultiProf = pGameProfile ? pGameProfile->GetMultiplayerProfile() : NULL;

		AmaxVehicle* pVehicle = NULL;

		if ( !bRandom && pMultiProf )
		{
			U32 vehicleId = 0;
			//If we are not the host car and the lock to host car option has been set then choose the hosts car
			if(m_CustomRace.GetLockToHostCar() && !IsHost())
			{
				RacerNet::C_NetPlayerManager& manager = services.PlayerManager();
				RacerNet::C_NetPlayer* pHost = manager.HostPlayer(false);
				if(pHost)
				{
					vehicleId = pHost->m_RaceData.m_VehicleData.m_xtVehicleId;
				}
			}

			if(vehicleId != 0)
				pVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)vehicleId ];
		}

		// TODO - validate against DLC

		if ( !pVehicle || bRandom )
		{
			//	Give the player a random vehicle from their garage that meets the event restrictions (just class atm).
			S32 mpRank = 0;
			
			if(pMultiProf)
			{
				mpRank = pMultiProf->GetCurrentLevel();
			}

			VehicleList validVehicles;		//	Vehicles which are valid for this race
			GetValidVehicles(validVehicles);
			VehicleList availableVehicles;	//	Valid vehicles which the player has access to

			for(int i = 0; i < validVehicles.Size(); i++)
			{
				//	Only allow vehicles of which are:
				if(!IsOnline() || HasVehicleUnlocked(mpRank, validVehicles[i]))	//	Are we in system link / has this vehicle been unlocked unlocked by the primary MP profile?
				{
					availableVehicles.PushBack(validVehicles[i]);
				}
			}
			if(!availableVehicles.Size())
			{
				BIZ_ASSERT_MSG(availableVehicles.Size(), "Couldn't find ANY valid vehicles - defaulting to the first one in leveling data (assuming it didn't assert there too...)");
			}
			else
			{
				//	First check if the last vehicle we used of this class is available for this race
				U32 lastVehicleUsedId = pMultiProf->GetLastCarUsed(GetRaceCarClass());
				for(int i = 0; i < availableVehicles.Size(); i++)
				{
					if(lastVehicleUsedId == availableVehicles[i]->ID)
						pVehicle = const_cast<AmaxVehicle*>(availableVehicles[i]);
				}
				//	Choose a random vehicle from our valid list
				if(!pVehicle)
					pVehicle = const_cast<AmaxVehicle*>(availableVehicles[m_CarRandomGenerator.Rand(availableVehicles.Size())]);
			}
		}

		if ( !pVehicle )
		{
			pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)C_MultiplayerData::Instance().m_pConfig->m_LevelingData[0].mVehicleUnlocks[0]->ID];
			if (NULL == pVehicle)
			{
				BIZ_ASSERT_MSG(pVehicle, "Multiplayer Leveling Data does not have any cars in first level");
				pVehicle = C_GameVehicleData::Instance().mVehicles[0];
			}
		}

		// update the profile
		if ( pMultiProf )
			pMultiProf->SetLastCarUsed( pVehicle->ID );

		out.m_xtVehicleId			= pVehicle->ID;
		out.m_xtColourId			= pGameProfile ? pGameProfile->ResprayGet( pVehicle->ID, true ) : 0;
		out.m_AutoGears				= true;
		out.m_VehicleContentPack	= pVehicle->contentPack;
		out.m_CharacterId			= GetRandomCharacterID();

		//	If we're playing online, we use our Multiplayer profile to fill out our info
		if(IsOnline())
		{
			//	Now vehicle has been selected, we need to find out how many pickup slots etc the player has (since vehicles can give bonuses)
			Game::C_MultiplayerProfile* pMultiProf = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetMultiplayerProfile();
			if(pMultiProf)
			{
				pMultiProf->BuildNetworkVehicleUpgradesData(out, (UI::E_CarUpgrades)m_Settings.m_Race.m_Upgrades );
			}
		}
		else
		{
			//	We must be in system link, so we just some default values - fair that way.
			out.m_PickupSlots = 3;
			out.m_ShieldSlots = 3;
		}

		_UI_MESSAGE1(UI::E_UiGameMsg_RefreshLobby, false);

		return true;
	}


	bool C_GameModeNetwork::ChooseNewLoadOut( const U8 userIndex, RacerNet::NetPlayerLoadOut& out )
	{
		if(IsNetwork() && GetModsAllowed())
		{
			Game::C_MultiplayerProfile* pMultiProf = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetMultiplayerProfile();
			if(pMultiProf)
			{
				pMultiProf->GetLoadouts(IsRanked()).BuildNetworkLoadOutData(out, IsRanked(), &C_Game::GetInstance()->GetMultiplayerAbilities());
			}
		}
		return true;
	}

	bool C_GameModeNetwork::BuildStartRacePacket(RacerNet::NetStartRacePacket* packet)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (!services.IsInitialised())
			return false;

		// assume we are going to start a race.
		m_Settings.m_HasAnyRacesStarted = 1;

		//	Clear start positions to standard order before attempting to set them based on reversed previous results
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

		bool teamDestruction=m_Settings.m_Teams&&(UI::E_RaceType)m_Settings.m_Race.m_Type == UI::E_RaceType_Destruction;
		InitStartPositions(mgr, teamDestruction);
		if (!teamDestruction)
		{
			ReverseStartPositions();
		}

		packet->mEvntIndex	= m_Playlist.GetRaceIndex(); // TODO_MP_REFACTOR
		packet->mRandomSeed	= (U32)Time::GetCurrentTime();

		bdOnlineUserID hostID;
		SysProfile::C_Manager::Instance().GetDemonwareUserId(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), hostID);

		packet->mHostId=hostID;
		packet->mHostStartTime=(U32)Time::GetCurrentTime();

		// should never fail because the host should always be a local player, as only the host can start the race.
		packet->mOriginalHostJoinId	= mgr.HostPlayer(false)->m_RaceConnection.m_JoinId;
		packet->mNumAIVehicles = 0;
		U8 count = 0;
		for (U8 i=0; i<mgr.TotalSize(); i++)
		{
			RacerNet::C_NetPlayer* player = mgr.Index(i);
			if (player->m_RaceConnection.IsActive() && ( RacerNet::eNetPlayerStateIdle == player->m_RaceConnection.m_PlayerState || RacerNet::eNetPlayerStateChangingCar == player->m_RaceConnection.m_PlayerState ) )
			{
				RacerNet::NetStartRacePacketPlayer& packetPlayer = packet->mPlayer[count];

				packetPlayer.mJoinRef		= player->m_RaceConnection.m_JoinId;
				packetPlayer.mStartPos		= m_StartPositions[count];	// ???
				packetPlayer.mTeam			= -1;	// No team
				packetPlayer.m_User			= player->m_User;
				packetPlayer.mAiAvatarId	= -1;   // We need the avatar index for the texture sheet (only for AI racers)

				packetPlayer.mVehicle		= player->m_RaceData.m_VehicleData;
				packetPlayer.mLoadOut		= player->m_RaceData.m_LoadOut;

				if ( m_Settings.m_Teams )
				{
					U32 colour_id = ( player->m_RaceData.m_Team ) ? C_MultiplayerData::Instance().m_pConfig->m_TeamBPaint.Id() : C_MultiplayerData::Instance().m_pConfig->m_TeamAPaint.Id();
					packetPlayer.mVehicle.m_xtColourId = colour_id;
					packetPlayer.mTeam = player->m_RaceData.m_Team;
				}

				BIZ_ASSERT_MSG_SPEED(packetPlayer.mVehicle.m_xtVehicleId != 0, "player has a vehicle ID of zero - tell GavinY");
				count++;
			}
		}
		packet->mNumPlayerVehicles = count;

		bool addAiVehicles = false;

		if((m_PlayMode == E_PlayMode_Playlist && (m_Playlist.GetRaceData()->aiMode != UI::E_AiOff)) ||
			(m_PlayMode == E_PlayMode_CustomRace && (m_CustomRace.GetAI() != UI::E_AiOff)))
		{

			addAiVehicles = true;
		}

		if(addAiVehicles)
		{
			//	We must be in system link
			AddAiVehiclesToStartRacePacket(packet);
			DistributeAiVehicles(packet);
			NameAiRacers(packet);
			ChooseAiVehicles(packet);
		}

		return true;
	}

	void C_GameModeNetwork::PlayerNotification(const WCHAR* pPlayerName, const bool join, const bool party)
	{
		/** NOTE
			This can be called from a thread while loading.
			pPlayerName string must be copied as it will be deleted when this function exits (if the player is leaving)
			join will be true if this player is joining or false if this player is leaving.
			party will be true if this notification relates to the party session or false if it relates to the race session. 
		*/
	}

	void C_GameModeNetwork::ReverseStartPositions()
	{
		if(!C_MpResults::Instance().m_ResultsValid)
			return;

		for(int i = 0; i < m_StartPositions.Size(); i++)
		{
			m_StartPositions[i] = -1;
		}

		U8 currentStartPos = 0;
		MpVehiclesData& previousResults = C_MpResults::Instance().m_SortedVehicleResults;
		for(int i = 0; i < previousResults.Size(); i++)
		{
			//	Break out if we run over the number of vehicles from the last race
			if(previousResults[i].m_Position == 0)
				break;
			m_StartPositions[previousResults[i].m_LocalIndex] = currentStartPos;
			currentStartPos++;
		}

		//	Go through m_StartPositions and set remaining positions
		for(int i = 0; i < m_StartPositions.Size(); i++)
		{
			if(m_StartPositions[i] == -1)
			{
				m_StartPositions[i] = currentStartPos;
				currentStartPos++;
			}
		}
	}

	void C_GameModeNetwork::AddAiVehiclesToStartRacePacket( RacerNet::NetStartRacePacket* p_pPacket )
	{
		//	Check if p_pPacket->mNumPlayerVehicles is less than the required racers
		if(p_pPacket->mNumPlayerVehicles >= m_Settings.m_MaxDrivers)
			return;

		//	Calculate how many AI vehicles we need to add to the pack.
		const U8 desiredAI = m_Settings.m_MaxDrivers - p_pPacket->mNumPlayerVehicles;
		if(m_PlayMode == E_PlayMode_CustomRace)
		{
			p_pPacket->mNumAIVehicles = ( desiredAI > m_Settings.m_Race.m_AIMaxPadding ) ? m_Settings.m_Race.m_AIMaxPadding : desiredAI;
		}
		else
			p_pPacket->mNumAIVehicles = desiredAI;

		U8 maxSafeAi = p_pPacket->mNumPlayerVehicles * C_MultiplayerData::Instance().m_pConfig->m_MaxAiPerConsole;
		if(p_pPacket->mNumAIVehicles > maxSafeAi)
			p_pPacket->mNumAIVehicles = maxSafeAi;

		const S32 max = p_pPacket->mNumPlayerVehicles + p_pPacket->mNumAIVehicles;

		for(int i = p_pPacket->mNumPlayerVehicles, j = 0; i < max; i++, j++)
		{
			p_pPacket->mPlayer[i].mJoinRef			= RacerNet::NET_AI_STARTING_JOIN_ID + j;	//	Player net join IDs only go from 1 - 32, so 50+ is VERY safe.
			p_pPacket->mPlayer[i].mPrimaryJoinRef	= p_pPacket->mOriginalHostJoinId;	//	Stick the host's join ID in here in case the AI distribution fails for whatever reason...
			p_pPacket->mPlayer[i].mStartPos			= m_StartPositions[i];
			p_pPacket->mPlayer[i].mTeam				= -1;
			UI::WideStrFormat(p_pPacket->mPlayer[i].m_User.m_Name, 16, L"AI_%d", i+1);
		}
	}

	void C_GameModeNetwork::DistributeAiVehicles( RacerNet::NetStartRacePacket* p_pPacket )
	{
		//	Calculate how many AI we need to host in each player's game to give a reasonably even distribution.
		U32 numAiPerPlayer = (U32)Maths::Ceil((F32)p_pPacket->mNumAIVehicles / (F32)p_pPacket->mNumPlayerVehicles);
		U8 aiPrimaryJoinIdSet = 0;
		U8 currentPlayer = 0;
		while (aiPrimaryJoinIdSet < p_pPacket->mNumAIVehicles)
		{
			for(U32 i = 0; i < numAiPerPlayer; i++)
			{
				if(aiPrimaryJoinIdSet >= p_pPacket->mNumAIVehicles)
					break;
				p_pPacket->mPlayer[p_pPacket->mNumPlayerVehicles + aiPrimaryJoinIdSet].mPrimaryJoinRef = p_pPacket->mPlayer[currentPlayer].mJoinRef;
				aiPrimaryJoinIdSet++;
			}
			currentPlayer++;
		}
	}

	void C_GameModeNetwork::NameAiRacers( RacerNet::NetStartRacePacket* p_pPacket )
	{
		if ( !C_MultiplayerData::Instance().m_pConfig || !C_MultiplayerData::Instance().m_pConfig->m_AiCharacters.count )
			return;

		MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;

		UI::E_AiMode aiMode = ( m_PlayMode == E_PlayMode_CustomRace ) ? (UI::E_AiMode)m_CustomRace.GetAI() : UI::E_AiMedium ;

		//	Get MP rank-ish for AI vehicles based on AI mode
		U8 basicAiMpRank = 0;
		if(aiMode == UI::E_AiEasy)
			basicAiMpRank = pConfig->m_AiMpRank_Easy;
		else if(aiMode == UI::E_AiMedium)
			basicAiMpRank = pConfig->m_AiMpRank_Medium;
		else if(aiMode == UI::E_AiHard)
			basicAiMpRank = pConfig->m_AiMpRank_Hard;

		U8 aiMpRankVariance = pConfig->m_AiMpRankVariance;
		Util::C_Random rand;
		rand.Seed((U32)Time::GetCurrentTime());

		for(int i = 0; i < p_pPacket->mNumAIVehicles; i++ )
		{
			if ( i >= C_MultiplayerData::Instance().m_pConfig->m_AiCharacters.count )
			{
				BIZ_ASSERT_MSG( i < C_MultiplayerData::Instance().m_pConfig->m_AiCharacters.count, "Not enough AI characters in the config. Add more" );
				continue;
			}

			const RaceBookCharacter* pChar = C_MultiplayerData::Instance().m_pConfig->m_AiCharacters[i];
			if ( !pChar )
				continue;

			U8 vehicleNum = p_pPacket->mNumPlayerVehicles + i;

			// Copy name into the ai player
			UI::WideStrFormat( p_pPacket->mPlayer[vehicleNum].m_User.m_Name, SYSTEM_PROFILE_NAME_LENGTH, UI::C_TextStringID( pChar->loc_string.id ).String() );
			p_pPacket->mPlayer[vehicleNum].mVehicle.m_CharacterId	= pChar->ID;
			p_pPacket->mPlayer[vehicleNum].mAiAvatarId				= pChar->avatarID;

			//	Give the AI a fake multiplayer rank based on the difficulty
			S8 aiMpRank = ((basicAiMpRank + aiMpRankVariance) - rand.Rand(2 * aiMpRankVariance));
			aiMpRank = (aiMpRank < 0) ? 0 : aiMpRank;
			aiMpRank = (aiMpRank > pConfig->m_LevelingData.count - 1) ? pConfig->m_LevelingData.count - 1 : aiMpRank;
			p_pPacket->mPlayer[vehicleNum].mAiMultiplayerRank = aiMpRank;
		}
	}

	void C_GameModeNetwork::UpdateLobbyRaceInfo( void )
	{
		if(!C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr()->IsRaceInProgress())
			return;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		RacerNet::C_NetPlayerManager& manager = services.PlayerManager();

		static F32 lobbyWaitingTimer = RacerNet::NET_LOBBY_WAITING_INFO_FREQUENCY;
		static U8 lastNumPlayers = manager.Size(false);

		lobbyWaitingTimer -= (F32)C_GameApp::GetInstance()->GetTimer().GetFrameTime();
		//	Do we need to send the race status info to any players waiting in the lobby?
		if(lobbyWaitingTimer < 0 || manager.Size(false) != lastNumPlayers)
		{
			lobbyWaitingTimer = RacerNet::NET_LOBBY_WAITING_INFO_FREQUENCY;
			lastNumPlayers = manager.Size(false);

			LobbyWaitingRaceInfo& infoForLobby = C_GameApp::GetInstance()->GetGame()->GetNetGameVars().GetLobbyWaitingRaceInfo();
			//	Just in case we get a bad ptr or an illegal race mode
			if(!C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr() || !C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr()->BuildLobbyWaitingRaceInfoData(infoForLobby))
				return;

			const U8 numVehicles = infoForLobby.m_Vehicles.Size();

			RacerNet::C_NetMessage& netMsg = services.Router().GetOutgoingUnReliable();
			netMsg.CreateUnReliable(RacerNet::eNetMessage_LobbyWaitingRaceInfo, manager.LocalPrimaryJoinId(false), false);
			
			netMsg.WriteF32(infoForLobby.m_Timeleft);
			netMsg.WriteU8(numVehicles);
			for(int i = 0; i < numVehicles; i++)	//	Write the data for all our local vehicles (99% of the time this is one, but there could be AI)
			{
				netMsg.WriteBuffer(&infoForLobby.m_Vehicles[i], sizeof(LobbyRaceVehicleInfo));
			}

			netMsg.TerminateMessage();

			for (U8 i=0; i < manager.Size(false); i++)
			{					
				RacerNet::C_NetPlayer* player = manager.Index(i);
				if (NULL == player || player->LocalPlayer() || !player->PrimaryPlayer() || !player->m_RaceConnection.IsActive() || !player->m_RaceConnection.CanMessage())
					continue;

				// is the remote player in the lobby ?
				switch (player->m_RaceConnection.m_PlayerState)
				{
				case RacerNet::eNetPlayerStateIdle:
				case RacerNet::eNetPlayerStateChangingCar:
				case RacerNet::eNetPlayerStateChangingSettings:
					player->SendUnReliable(netMsg, false, services.Router());
					break;
				}
			}
		}
	}

	void C_GameModeNetwork::ProcessLobbyRaceInfo( RacerNet::C_NetMessage& msg )
	{
		LobbyWaitingRaceInfo& raceInfo = C_GameApp::GetInstance()->GetGame()->GetNetGameVars().GetLobbyWaitingRaceInfo();
		LobbyRaceVehicleInfo tempVehicle;
	
		F32 timeleft = -1.0f;
		U8 numVehiclesInMsg = 0;

		bool ok = msg.ReadOpen();
		if(ok)
		{
			ok = ok && msg.ReadF32(timeleft);
			ok = ok && msg.ReadU8(numVehiclesInMsg);
			for(int i = 0; i < numVehiclesInMsg; i++)
			{
				ok = ok && msg.ReadBuffer(&tempVehicle, sizeof(tempVehicle));

				bool found = false;
				for(int i = 0; i < raceInfo.m_Vehicles.Size(); i++)
				{
					LobbyRaceVehicleInfo& vehicle = raceInfo.m_Vehicles[i];
					if(tempVehicle.m_JoinId == vehicle.m_JoinId)
					{
						vehicle = tempVehicle;
						found = true;
						break;
					}
				}
				if(!found)
				{
					raceInfo.m_Vehicles.PushBack(tempVehicle);
				}
			}

			msg.ReadClose();
		}

		// Only want to read it once
		if ( raceInfo.m_Timeleft == -1.0f )
			raceInfo.m_Timeleft = timeleft;
	}

	void C_GameModeNetwork::ReceiveLobbyUpdateMessage(RacerNet::C_NetMessage& msg, const U8 senderId)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

		bool ok = msg.ReadOpen();
		if (ok)
		{
			F32 tmpTimer = 0;
			S8 tmpState = 0;
			U32 seed = 0;
			S16 tmpIndex = 0;

			// read data
			ok = ok && msg.ReadF32(tmpTimer);
			ok = ok && msg.ReadS8(tmpState);
			ok = ok && msg.ReadS16(tmpIndex);
			ok = ok && msg.ReadU32(seed);
			ok = ok && msg.ReadS8(m_VoteInfo.countA);
			ok = ok && msg.ReadS8(m_VoteInfo.countB);
			ok = ok && msg.ReadS16(m_VoteInfo.indexA);
			ok = ok && msg.ReadS16(m_VoteInfo.indexB);

			// Read team data
			if ( m_Settings.m_Teams )
			{
				U8 size = 0;
				ok = ok && msg.ReadU8( size );

				for (U8 i=0; i<size; i++)
				{
					U8 teamData = 0;
					ok = ok && msg.ReadU8( teamData );

					// Find player and write team value
					U8 joinID = ( teamData & ~0x80 );
					U8 team = ( teamData >> 7 );

					RacerNet::C_NetPlayer* pPlayer = mgr.FindFromJoinId( joinID, false );
					if ( pPlayer )
						pPlayer->m_RaceData.m_Team = team;
				}
			}
			msg.ReadClose();

			// Only if its a playlist does the host control your race index / vote data
			if ( m_PlayMode == E_PlayMode_Playlist )
			{
				// The seed could have changed?
				if( m_Playlist.GetSeed() != seed)
				{
					m_Playlist.Shuffle( seed );
					m_Playlist.SetupNetRaceSettings( m_Settings );
				}

				// The race index could have changed
				if ( tmpIndex != m_Playlist.GetRaceIndex() || tmpIndex != m_Settings.m_PlaylistIndex )
				{
					m_Playlist.SetRaceIndex( tmpIndex );
					m_Playlist.SetupNetRaceSettings( m_Settings );
				}

				// Everybody needs to maintain their local net race settings data
				m_Settings.m_VoteCountA = m_VoteInfo.countA;
				m_Settings.m_VoteCountB = m_VoteInfo.countB;
				m_Settings.m_VoteIndexA = m_VoteInfo.indexA;
				m_Settings.m_VoteIndexB = m_VoteInfo.indexB;
			}

			// Set net game vars
			C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyTimer = tmpTimer;
			C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyState = (UI::E_LobbyState)tmpState;

			// Still process this
			UpdateLobbyState( C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyState, C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyTimer );
		}
	}

	void C_GameModeNetwork::ReceiveBeginVotingMessage(RacerNet::C_NetMessage& msg)
	{
		bool ok = msg.ReadOpen();
		if (ok)
		{
			// read data
			S8 tmp = 0;
			ok = ok && msg.ReadS8(tmp);
			msg.ReadClose();
		}
		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_VoteStarted = true;
	}

	bool C_GameModeNetwork::ReceivePostRaceMessage(RacerNet::C_NetMessage& msg, const U8 senderId)
	{
		bool ok = msg.ReadOpen();

		if (ok)
		{
			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			RacerNet::C_NetPlayerManager& manager = services.PlayerManager();

			U8 numPlayers = 0; 
			U8 joinId = 0;
			RacerNet::NetPlayerProfileGameData prof;

			ok = ok && msg.ReadU8(numPlayers);
			if (ok)
			{
				for (U8 i=0; i<numPlayers; i++)
				{
					ok = ok && msg.ReadU8(joinId);
					ok = ok && msg.ReadBuffer(&prof, sizeof(prof));
					if (ok)
					{
						RacerNet::C_NetPlayer* p = manager.FindFromJoinId(joinId, false);
						if (p)
							p->m_ProfileData = prof;

						// if vehicle exists in level state then overwrite new rank.
						Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
						bool done = false;
						for (U32 z = 0; z < levelState.mNumCars && !done; z++)
						{
							if (joinId == levelState.mCars[z].mNetJoinId)
							{
								levelState.mCars[z].mMultiplayerRank = prof.m_Game.m_Rank;
								levelState.mCars[z].mMultiplayerWinningStreak = prof.m_Game.m_WinningStreak;
								levelState.mCars[z].mBeatenBizarre = p->m_ProfileData.m_Game.m_HasBeatenBizarre;
								done = true;
							}
						}

						//Update championship standing points
						if(m_Playlist.IsChampionship())
						{
							ChampionshipStanding& standing = m_ChampionshipStandings[GetChampionshipStandingsIndex(joinId)];
							standing.mTotalScore = p->m_ProfileData.m_Game.m_RacePointsThisSession;
							standing.mTotalWins = p->m_ProfileData.m_Game.m_WinsThisSession;
							GenerateSortedStandings();
						}
					}
				}
			}
		}

		if (!ok)
		{
			LOGWARN(g_Log, "C_GameModeNetwork::ReceivePostRaceMessage() - malformed data received.");
		}

		msg.ReadClose();
		return true;
	}

	void C_GameModeNetwork::ResetGameValues()
	{
		m_ChooseRandoms = false;
	}


	//	This stuff basically ensures that the vehicle/ mods will always be randomized before each race (if they should be), even if the class restriction is the same as the previous one			
	void C_GameModeNetwork::PreSelectRandom()
	{
		m_ChooseRandoms = true;
	}

	bool C_GameModeNetwork::ChooseAiVehicles( RacerNet::NetStartRacePacket* p_pPacket )
	{
		BIZ_ASSERT_MSG( C_MultiplayerData::Instance().m_pConfig, "No Multiplayer config" );

		//	Bail out if we don't have a MP config or if we're not in System Link.
		if(!C_MultiplayerData::Instance().m_pConfig)
			return false;

		VehicleList classVehicles;
		GetValidVehicles(classVehicles, false);

		// oh oh we have no vehicles
		if (classVehicles.Size() == 0)
		{
			BIZ_ASSERT_MSG(false, "We've got no vehicles!!!");
			return false;
		}

		//	Give any AI racers a random vehicle (Unless lock to host is on).
		for(int i = p_pPacket->mNumPlayerVehicles; i < p_pPacket->mNumPlayerVehicles + p_pPacket->mNumAIVehicles; i++)
		{
			const AmaxVehicle* newVehicle = 0;
			if(m_CustomRace.GetLockToHostCar())
			{
				RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
				RacerNet::C_NetPlayerManager& manager = services.PlayerManager();
				RacerNet::C_NetPlayer* pHost = manager.HostPlayer(false);
				if(pHost)
				{
					U32 vehicleId = pHost->m_RaceData.m_VehicleData.m_xtVehicleId;
					if(vehicleId)
					{
						newVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)vehicleId ];
					}
				}
			}

			if(!newVehicle)
			{
				newVehicle = classVehicles[m_CarRandomGenerator.Rand(classVehicles.Size())];
			}

			p_pPacket->mPlayer[i].mVehicle.m_xtVehicleId = newVehicle->ID;
		}
		return true;
	}

	void C_GameModeNetwork::OnSetupRace( lua_State* pState )
	{
		m_Settings.Clear(); // Reset to defaults

		// Setup playmode data
		if ( m_PlayMode == E_PlayMode_CustomRace )
		{
			m_CustomRace.SetupNetRaceSettings( m_Settings );
		}
		else
		if ( m_PlayMode == E_PlayMode_Playlist )
		{
			m_Playlist.SetupNetRaceSettings( m_Settings );
		}

		// Fill out other settings
		m_Settings.m_Ranked				= IsRanked();
		m_Settings.m_PublicGame			= IsRanked();
	}

	void C_GameModeNetwork::OnSetupLevelState( lua_State* pState, C_LevelState& levelState )
	{
		C_MpRaceActionManager::Instance().ClearActions();
		Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		
		// Clear this AGAIN just to be certain we've purged any events from any previous event(s)
		netGameVars.ClearNetEventList();

		if (!netGameVars.m_StartRaceDataValid)
		{
			BIZ_ASSERT_FAIL("OnSetupLevelState.() - called when start race data is not valid.");
			return;
		}
		netGameVars.m_StartRaceDataValid = false; // can clear this now as the race is about to load.

		// Specific network settings
		levelState.mNetworkRace			= true;
		levelState.mOriginalNetworkHost = netGameVars.m_RaceHost;
		levelState.m_RandomSeed = netGameVars.m_StartRaceData.mRandomSeed;

		//	If it's a Custom Race, get the Ai difficulty setting, otherwise (playlist) just use the medium one. Won't break anything if there are no AI in the game.
		UI::E_AiMode aiMode = ( m_PlayMode == E_PlayMode_CustomRace ) ? (UI::E_AiMode)m_CustomRace.GetAI() : UI::E_AiMedium ;
		if ( aiMode != UI::E_AiOff)
		{
			switch(aiMode)
			{
			case UI::E_AiEasy:
				levelState.m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\Multiplayer_Easy.csv");
				break;
			case UI::E_AiMedium:
				levelState.m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\Multiplayer_Medium.csv");
				break;
			case UI::E_AiHard:
				levelState.m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\Multiplayer_Hard.csv");
				break;
			}
		}
		if( !levelState.mAIController )
		{
			levelState.mAIController = RLDatabase::Instance().mRLSystems[0]->mAIPickupController->mControllers[0].mController.Object();
		}

		if ( IsRanked() && m_PlayMode == E_PlayMode_Playlist )
			DecreaseFanMultiplier( levelState );

		// Setup players
		RacerNet::C_NetPlayerManager& playerMgr = RacerNet::C_NetServices::Instance().PlayerManager();
		U32 numPlayers = netGameVars.m_StartRaceData.mNumPlayerVehicles;
		levelState.mNumCars	= numPlayers + netGameVars.m_StartRaceData.mNumAIVehicles;
		BIZ_ASSERT( levelState.mNumCars <= RacerNet::NET_MAX_VEHICLES );

		for( U32 i = 0; i < levelState.mNumCars; i++ )
		{
			LevelStateCar& levelStateCar=levelState.mCars[i];
			const RacerNet::NetStartRacePacketPlayer& playerPacket=netGameVars.m_StartRaceData.mPlayer[i];

			levelStateCar.mCharacterID		= playerPacket.mVehicle.m_CharacterId;
			levelStateCar.mSystemPlayerId	= playerPacket.m_User; 
			levelStateCar.mNetJoinId		= playerPacket.mJoinRef;
			levelStateCar.mCarID			= playerPacket.mVehicle.m_xtVehicleId;
			BIZ_ASSERT_MSG_SPEED(levelStateCar.mCarID != 0, "car has a vehicle ID of zero - tell GavinY");
			levelStateCar.mAutoGears		= playerPacket.mVehicle.m_AutoGears ? true : false;
			levelStateCar.mStartPosition	= playerPacket.mStartPos;
			levelStateCar.mHuman			= i < numPlayers;		// Human players always come first
			levelStateCar.mAiAvatarId		= playerPacket.mAiAvatarId;
			levelStateCar.mResprayID		= playerPacket.mVehicle.m_xtColourId;

			if ( m_Settings.m_Teams )
				levelStateCar.mTeam = playerPacket.mTeam;

			// setup name above car.
			levelStateCar.mNameAboveCar.Format(playerPacket.m_User.Name());

			// Human data
			if ( i < numPlayers )
			{
				levelStateCar.mUpgrades.BuildFromNetPlayerVehicle(playerPacket.mVehicle);

				// If we are using an event check the event setting
				bool allowMods = true; //Default to true.. if no event.. e.g. custom race.. this will be true
				if(m_PlayMode == Game::E_PlayMode_Playlist)
				{
					const MultiplayerRace*  pRace = m_Playlist.GetRaceData();
					if(pRace)
					{
						allowMods = (pRace->powerupMods == (S8)(UI::E_PowerupMods_On));
					}
				}
				else
				if ( m_PlayMode == E_PlayMode_CustomRace )
				{
					allowMods = (m_CustomRace.GetMods() == (S8)(UI::E_PowerupMods_On));
				}

				if( allowMods )																												
				{
					C_Game::GetInstance()->GetMultiplayerAbilities().FillOutLevelStateCar(playerPacket.mLoadOut, levelStateCar);
				}
				else
				{
					// Just ensure we have empty loadouts.
					C_Game::GetInstance()->GetMultiplayerAbilities().ResetLoadOuts(levelStateCar);
				}
				

				// Force shield off if its been disabled in the event
				if ( !(m_Settings.m_Race.m_Pickups & (1 << UI::E_Pickup_Shield )) )
					levelStateCar.mUpgrades.NumShieldSlots( 0 );

				RacerNet::C_NetPlayer* p = playerMgr.FindFromJoinId(levelStateCar.mNetJoinId, false);
				if ( p )
				{
					// record vehicle index this player uses so network layer can reference race logic core.
					p->m_RaceData.m_InRaceVehicleIndex		= i; 

					// read players rank. for ui purposes.
					levelStateCar.mMultiplayerRank	= p->m_ProfileData.m_Game.m_Rank;
					levelStateCar.mMultiplayerLegendLevel = p->m_ProfileData.m_Game.m_Legend;
					levelStateCar.mBeatenBizarre = p->m_ProfileData.m_Game.m_HasBeatenBizarre;

					if ( p->LocalPlayer() )
					{
						levelStateCar.mControlType	= E_VEHICLE_CONTROL_TYPE_PLAYER;
						levelStateCar.mController		= p->m_ControllerPort;
						levelStateCar.mTextureDetail	= g_debugMPLocalCarTextureDetail;
					}
					else
					{	
						levelStateCar.mControlType	= E_VEHICLE_CONTROL_TYPE_NETWORK;
						levelStateCar.mController		= Game::INVALID_CONTROLLER;
						levelStateCar.mTextureDetail	= g_debugMPRemoteCarTextureDetail;
					}
				}
				else
				{
					//	Couldn't find the player that this vehicle belongs to, so the player has probably managed to disconnect from the game in between building the start race packet
					//	and setting the level state up here. Setting the vehicle to be under network control will ensure it's disabled and removes from the race after the load :)
					levelStateCar.mControlType		= E_VEHICLE_CONTROL_TYPE_NETWORK;
					levelStateCar.mController		= Game::INVALID_CONTROLLER;
					levelStateCar.mTextureDetail	= g_debugMPRemoteCarTextureDetail;
				}
			}
			else // AI player
			{
				levelStateCar.mUpgrades.OfflineValues();	//	Nice quick way to give the AI vehicles 3 pickup slots and the base abilities for now.
				levelStateCar.mMultiplayerRank = playerPacket.mAiMultiplayerRank;	//	Fake a multiplayer rank.
				levelStateCar.mOwnerNetJoinId	= playerPacket.mPrimaryJoinRef; // We don't wish to do this for players to differentiate network vehicles between player and AI

				//	If the AI's "owner" is this kit, we spawn the vehicles as AI, otherwise they're just more Network vehicles :)
				if(playerPacket.mPrimaryJoinRef == levelState.mCars[0].mNetJoinId)
				{
					levelStateCar.mControlType	= E_VEHICLE_CONTROL_TYPE_AI;
					levelStateCar.mController	= Game::INVALID_CONTROLLER;
				}
				else
				{
					levelStateCar.mControlType	= E_VEHICLE_CONTROL_TYPE_NETWORK;
					levelStateCar.mController	= Game::INVALID_CONTROLLER;
				}
			}
		}
	}

	void C_GameModeNetwork::OnCalculateVoteIndicies()
	{
		BIZ_ASSERT( IsHost() );

		// Rewrite back into NetRaceSettings
		m_Settings.m_PlaylistIndex = m_Playlist.GetRaceIndex();
		m_Settings.m_PlaylistSeed = m_Playlist.GetSeed();

		// Rewrite vote data
		m_Settings.m_VoteCountA = m_VoteInfo.countA;
		m_Settings.m_VoteCountB = m_VoteInfo.countB;
		m_Settings.m_VoteIndexA = m_VoteInfo.indexA;
		m_Settings.m_VoteIndexB = m_VoteInfo.indexB;
	}

	void C_GameModeNetwork::OnCalculateVoteResults()
	{
		BIZ_ASSERT( IsHost() );

		// Rewrite back into NetRaceSettings
		m_Playlist.SetupNetRaceSettings(m_Settings);
	}

	void C_GameModeNetwork::OnVoteOnRace()
	{
		// Rewrite back into NetRaceSettings
		m_Settings.m_VoteCountA = m_VoteInfo.countA;
		m_Settings.m_VoteCountB = m_VoteInfo.countB;
	}

	void C_GameModeNetwork::CalculateAwards()
	{
		Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
		m_RaceAwards.CalculateAwards(levelState.m_RandomSeed);
		m_RaceAwards.ConvertToLocalIndices();
		m_RaceAwards.ResolveTiedAwards();
	}

	//Championship
	void C_GameModeNetwork::InitChampionshipStandingsStructure()
	{
		if(m_Playlist.IsChampionship() && !m_Playlist.IsChampionshipInProgress())
		{
			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

			for(int i = 0; i < m_ChampionshipStandings.Size(); i++)
			{
				m_ChampionshipStandings[i].Reset();
			}
			
			for (U8 i = 0; i < mgr.TotalSize(); i++)
			{
				const RacerNet::C_NetPlayer* p = mgr.Index(i);
				ChampionshipStanding& standing = m_ChampionshipStandings[i];
				standing.mPlayerName = p->m_User.m_Name;
				standing.mJoinId = p->m_RaceConnection.m_JoinId;
			}

			GenerateSortedStandings();

			m_Playlist.StartChampionship();
		}
	}

	const U16 C_GameModeNetwork::GetPlayersCurrentChampionshipScore(U8 joinId) const
	{
		return m_ChampionshipStandings[joinId-1].mTotalScore;
	}

	const U32 C_GameModeNetwork::GetChampionshipFansWon(U8 finishPosition) const
	{
		Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

		U32 penaltyPerPlayerLeft = C_MultiplayerData::Instance().m_pConfig->m_ChampionshipPlayerLeftPenalty;

		U8 noOfPlayersLeft = 0;

		for(U8 i = 0; i < m_ChampionshipStandings.Size(); i++)
		{
			if(m_ChampionshipStandings[i].mTotalScore == -1)
			{
				noOfPlayersLeft++;
			}
		}

		S32 finishBonus = 0;

		const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
		if(pEvent->championship_scores.count > finishPosition-1)
		{
			finishBonus = pEvent->championship_scores[finishPosition-1] - (noOfPlayersLeft * penaltyPerPlayerLeft);
		}

		return finishBonus > 0 ? (U32)finishBonus : 0;
	}

	const U32 C_GameModeNetwork::GetChampionshipFinishBonus() const
	{
		return C_MultiplayerData::Instance().m_pConfig->m_ChampionshipCompletionBonus;
	}

	const F32 C_GameModeNetwork::GetChampionshipLobbyWaitTime() const
	{
		return C_MultiplayerData::Instance().m_pConfig->m_ChampionshipWaitTime;
	}

	const U8 C_GameModeNetwork::GetChampionshipStandingsIndex(U8 joinId) const
	{
		for(U8 i = 0; i < RacerNet::NET_MAX_VEHICLES; i++)
		{
			if(m_ChampionshipStandings[i].mJoinId == joinId)
			{
				return i;
			}
		}

		BIZ_ASSERT_MSG(false, "C_GameModeNetwork::GetChampionshipStandingsIndex - Join ID not found in championship standings");
		return 0;
	}

	S32 C_GameModeNetwork::GetNumberOfPlayers() const
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if ( !services.IsInitialised() )
			return 0;

		RacerNet::C_NetRace& race = services.Race();
		if ( !race.IsInitialised() && !race.IsRunning() )
			return 0;

		return services.PlayerManager().TotalSize();
	}

	void C_GameModeNetwork::UpdateLobbyState( const UI::E_LobbyState eState, const F32 fTime )
	{
		m_Settings.m_LobbyState = eState;
		m_Settings.m_LobbyTimer = fTime;

		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyState = eState;
		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyTimer = fTime;
	}

	void C_GameModeNetwork::GenerateSortedStandings()
	{
		m_SortedChampionshipStandings.Clear();
		for(int i=0; i<m_ChampionshipStandings.Size(); i++)
		{
			if (m_ChampionshipStandings[i].mJoinId!=0)
			{
				m_SortedChampionshipStandings.PushBack(m_ChampionshipStandings[i]);
			}
		}

		for(int i=0; i<m_SortedChampionshipStandings.Size()-1; i++)
		{
			int highest=i;
			for(int j=i+1; j<m_SortedChampionshipStandings.Size(); j++)
			{
				if (m_SortedChampionshipStandings[highest].mTotalScore<m_SortedChampionshipStandings[j].mTotalScore)
				{
					highest=j;
				}
			}

			if (highest!=i)
			{
				Game::C_GameModeNetwork::ChampionshipStanding tmp=m_SortedChampionshipStandings[i];
				m_SortedChampionshipStandings[i]=m_SortedChampionshipStandings[highest];
				m_SortedChampionshipStandings[highest]=tmp;
			}
		}
	}

	const C_GameModeNetwork::SortedChampionshipStandingContainer& C_GameModeNetwork::GetSortedChampionshipStandings() const
	{
		return m_SortedChampionshipStandings;
	}

	void C_GameModeNetwork::DecreaseFanMultiplier( C_LevelState& levelState ) const
	{
		// Should only be called in a playlist
		BIZ_ASSERT( m_PlayMode == E_PlayMode_Playlist );

		Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
		Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

		bool hasDoneALegend = (pMultiplayerProfile->GetLegendLevel() > 0);
		U8 maxRankPossible = pMultiplayerProfile->GetMaxLevellingRank()+1;
		U8 maxRankForPlaylist = m_Playlist.GetPlaylistData()->max_rank;
		U8 playerRank = pMultiplayerProfile->GetCurrentLevel();

		bool shouldPunishAsOverRank = false;
		//Check to see if this is a rank locked playlist, set the playlists max rank to the maximum rank possible 
		if(maxRankForPlaylist < maxRankPossible)
		{
			//See if the player's rank is over the maximum or if they've done a legend
			if(playerRank > maxRankForPlaylist || hasDoneALegend)
			{
				shouldPunishAsOverRank = true;
			}
		}

		if(gDecreaseFanMultiplier || shouldPunishAsOverRank)
		{
			levelState.mPunitiveMeasuresInPlace = true;
			levelState.mPunitiveFanMultiplier = C_MultiplayerData::Instance().m_pConfig->m_OverRankPunitiveMultiplier;
		}
		else
		{
			levelState.mPunitiveMeasuresInPlace = false;
			levelState.mPunitiveFanMultiplier = 1.f;
		}
	}

	void C_GameModeNetwork::ResetChampionship()
	{
		BIZ_ASSERT( m_PlayMode == E_PlayMode_Playlist );

		// Reset this so that games are joinable again
		m_Settings.m_HasAnyRacesStarted = 0;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if ( race.IsInitialised() && race.IsRunning() && race.IsOnline() )
			{
				race.UpdateMatchingServer(true); 
			}
		}
	}

	void C_GameModeNetwork::InitStartPositions(RacerNet::C_NetPlayerManager& playerManager, bool teamDestruction)
	{
		if (!teamDestruction)
 		{
			for(int i = 0; i < m_StartPositions.Size(); i++)
			{
				m_StartPositions[i] = i;
			}
		}
		else
		{
			for(int i = 0; i < m_StartPositions.Size(); i++)
			{
				m_StartPositions[i] = -1;
			}

			int teamA=RacerNet::NET_MAX_VEHICLES-2;
			int teamB=RacerNet::NET_MAX_VEHICLES-1;

			int teamAStep=-2;
			int teamBStep=-2;

			int lastTeamA=0;
			int lastTeamB=1;

			for (U8 i=0; i<playerManager.TotalSize(); i++)
			{
				const RacerNet::C_NetPlayer* player = playerManager.Index(i);

				if (player->m_RaceData.m_Team==0)
				{
					SetTeamStartPosition(i, teamA, teamAStep, lastTeamA, lastTeamB);

				}
				else
				{
					SetTeamStartPosition(i, teamB, teamBStep, lastTeamB, lastTeamA);
				}
			}

			for(int i = 0; i < m_StartPositions.Size(); i++)
			{
				if(m_StartPositions[i] == -1)
				{
					if ((i&1)==0)
					{
						SetTeamStartPosition(i, teamA, teamAStep, lastTeamA, lastTeamB);

					}
					else
					{
						SetTeamStartPosition(i, teamB, teamBStep, lastTeamB, lastTeamA);
					}
				}
			}
		}
	}

	void C_GameModeNetwork::SetTeamStartPosition( U8 carIndex, int &nextTeamPos, int &teamStep, int lastTeamIndex, int lastTeamIndexOther )
	{
		BIZ_ASSERT(carIndex>=0);
		BIZ_ASSERT(carIndex<RacerNet::NET_MAX_VEHICLES);

		m_StartPositions[carIndex] = nextTeamPos;
		nextTeamPos+=teamStep;
		if (nextTeamPos<lastTeamIndex)
		{
			nextTeamPos=lastTeamIndexOther;
			teamStep=-teamStep;
		}
	}
	//Mods are allowed if we have unlocked the mod shop, are in non ranked matches, or in custom races with mods on
	bool C_GameModeNetwork::GetModsAllowed()
	{
		Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
		bool modshopUnlocked = pGameProfile->GetMultiplayerProfile()->GetFeatureUnlocked(UI::E_MultiplayerFeature_ModShop);
		bool modsAllowed = modshopUnlocked;

		if(GetPlayMode() == Game::E_PlayMode_CustomRace)
		{
			modsAllowed = GetCustomRace().GetMods() == UI::E_PowerupMods_On;
		}
		else if(!IsRanked())
		{
			modsAllowed = true;
		}

		return modsAllowed;
	}

	void C_GameModeNetwork::SortLobbyRaceInfo()
	{
		// Dont want to run this if we are in race
		if( C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr()->IsRaceInProgress() )
			return;

		LobbyWaitingRaceInfo& infoForLobby = C_GameApp::GetInstance()->GetGame()->GetNetGameVars().GetLobbyWaitingRaceInfo();
		if ( infoForLobby.m_Vehicles.Size() == 0 )
			return;

		// Set join_id to an unused value for any PLAYERS (not AI) who have disconnected.
		RacerNet::C_NetPlayerManager& mgr = RacerNet::C_NetServices::Instance().PlayerManager();
		for ( U8 i = 0; i < infoForLobby.m_Vehicles.Size(); i++ )
		{
			bool found = false;
			for ( U8 j = 0; j < mgr.TotalSize(); j++ )
			{
				RacerNet::C_NetPlayer* p = mgr.Index(j);
				if ( p && p->m_RaceConnection.m_JoinId == infoForLobby.m_Vehicles[i].m_JoinId )
				{
					found = true;
					break;
				}
			}
			// not found so stick it to a invalid number
			if ( !found && (infoForLobby.m_Vehicles[i].m_JoinId < RacerNet::NET_AI_STARTING_JOIN_ID))
				infoForLobby.m_Vehicles[i].m_JoinId = 0xff;
		}

		// Sort by highest value
		bool finished = false;
		while( !finished )
		{
			finished = true;
			for ( U8 i = 0; i < infoForLobby.m_Vehicles.Size()-1; i++ )
			{
				if ( infoForLobby.m_Vehicles[i].m_StatData < infoForLobby.m_Vehicles[i+1].m_StatData )
				{
					Btl::Swap( infoForLobby.m_Vehicles[i], infoForLobby.m_Vehicles[i+1] );
					finished = false;
				}
			}
		}
	}

	bool C_GameModeNetwork::SendLobbyUpdateMessage( const UI::E_LobbyState eState, const F32 fTime )
	{
		// Need to update net race settings
		UpdateLobbyState( eState, fTime );

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised() && services.Race().IsRunning())
		{
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
			RacerNet::C_NetPlayer* hostPlayer = mgr.HostPlayer(false);
			if (hostPlayer && hostPlayer->LocalPlayer())
			{
				if (hostPlayer->m_RaceConnection.IsActive())
				{
					RacerNet::C_NetMessage& netMsg = services.Router().GetOutgoingUnReliable();
					netMsg.CreateUnReliable(RacerNet::eNetMessage_LobbyUpdate, mgr.LocalPrimaryJoinId(false), false);

					netMsg.WriteF32( fTime );
					netMsg.WriteS8( eState );
					netMsg.WriteU16( m_Playlist.GetRaceIndex() );
					netMsg.WriteU32( m_Playlist.GetSeed() );
					netMsg.WriteS8( m_VoteInfo.countA );
					netMsg.WriteS8( m_VoteInfo.countB );
					netMsg.WriteS16( m_VoteInfo.indexA );
					netMsg.WriteS16( m_VoteInfo.indexB );

					// Add on team data
					if ( m_Settings.m_Teams )
					{
						netMsg.WriteU8( mgr.TotalSize() );

						for (U8 i=0; i<mgr.TotalSize(); i++)
						{
							U8 teamData = 0;
							teamData |= ( mgr.Index(i)->m_RaceData.m_Team << 7 );			// Team is either 0 or 1. Pack this into the top bit
							teamData |= ( mgr.Index(i)->m_RaceConnection.m_JoinId & ~0x80 );// Pack the join id into the other 7 bits.
							netMsg.WriteU8( teamData );
						}
					}
					netMsg.TerminateMessage();

					for (U8 i=0; i<mgr.TotalSize(); i++)
					{
						RacerNet::C_NetPlayer* p = mgr.Index(i);
						if (!p->LocalPlayer() && p->PrimaryPlayer() && p->m_RaceConnection.CanMessage())
						{
							// Only send it to players in the lobby
							if ( p->m_RaceConnection.m_PlayerState >= RacerNet::eNetPlayerStateLoadingRace )
								continue;

							p->SendUnReliable(netMsg, false, services.Router());
						}
					}
				}
			}
			else
			{
				LOGWARN(RacerNet::g_Log, "NetRace.SendLobbyUpdateMessage() - called for non host player !");
			}
		}
		return 0;
	}

	S8 C_GameModeNetwork::GetLocalPlayerTeam() const
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

		for ( U8 i = 0; i < mgr.TotalSize(); i++ )
		{
			RacerNet::C_NetPlayer* p = mgr.Index(i);
			if ( p->LocalPlayer() )
				return p->m_RaceData.m_Team;
		}
		return -1;
	}

} // namespace Game


#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
