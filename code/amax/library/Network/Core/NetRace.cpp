#include <stdafx.h>
#include <Core/core.h>
#include <Network/Core.h>
#include <Network/Securekey.h>
#include <Network/Session.h>
#include <Network/CommunicatorManager.h>
#include <Network/SessionCommandNotification.h>
#include <bdNet/bdNet.h>
#include "../Misc/NetVersion.h"
#include "../Misc/NetDebug.h"
#include "../Misc/NetDefines.h"
#include "../Misc/NetMigrationStates.h"
#include "../Misc/NetRaceModeListener.h"
#include "../Core/NetMatchingParameters.h"
#include "../Misc/NetJoinRaceFromPartyData.h"
#include "../Misc/NetRaceSettings.h"
#include "../Misc/NetRaceSettingsQos.h"
#include "NetRace.h"
#include "NetServices.h"
#include "NetSessionKeyTracker.h"
#include "NetCreationHelper.h"
#include "NetTeamBalancer.h"
#include "NetJoinHelper.h"
#include "../Misc/NetDatagramPacketTypes.h"
#include "../Message/NetMessage.h"
#include "../Router/NetRouterBase.h"
#include "../Player/NetPlayerManager.h"
#include "../Lan/NetLanDiscovery.h"
#include "../Player/NetPlayer.h"
#include"../Core/NetRichPresence.h"
#include <GameLogic/GameData/GameData.h>
#include <GameLogic/GameData/GameVehicleData.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameApp.h>
#include <GameLogic/Profile/GameProfileManager.h>
#include <GameLogic\Structures\MpResults.h>
#include <GameLogic/GameData/MultiplayerData.h>
#include <UI/UiMessages.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

Debug::Bool g_Matchmaking_Return_Sessions_In_Race("Network.Race.Matchmaking.ReturnSessionsInRace", false, Debug::Persist());

#if !defined ( CFG_GOLD )
	Debug::Bool g_Join_ForceError_Generic("Network.Race.JoinErrors.ForceGeneric", false);
	Debug::Bool g_Join_ForceError_SlotCount("Network.Race.JoinErrors.ForceNotEnoughSlots", false);
	Debug::Bool g_Join_ForceError_Banned("Network.Race.JoinErrors.ForceBanned", false);
	Debug::Bool g_Join_ForceError_NoComms("Network.Race.JoinErrors.ForceNoCommunications", false);
	Debug::Bool g_Join_ForceError_Teams("Network.Race.JoinErrors.ForceTooManyTeams", false);
	Debug::Bool g_Join_ForceError_TeamMembers("Network.Race.JoinErrors.ForceToomanyTeamMembers", false);
	Debug::Bool g_Join_ForceError_ContentMismatch("Network.Race.JoinErrors.ForceContentMismatch", false);
	Debug::Bool g_Join_ForceError_Started("Network.Race.JoinErrors.ForceChampionshipStarted", false);
#endif

#ifndef CFG_GOLD
	namespace RacerNet
	{
		extern Debug::Enum<eNetBuildVersion> NET_PACKET_VERSION;
	}
#endif


namespace RacerNet
{
	C_NetRace::C_NetRace() :m_Initialised(false), m_OnlineMode(false), m_Running(false), 
							m_DisconnectResendMsgTimer(0.0f), m_Migrating(false), m_SessionIsMigrating(false), m_VoiceEnabled(false), m_InRace(false), m_BigError(false),
							m_LocalMatchingSessionManagerIndex(0), m_ForceMatchingServerModify(false), m_NewDisconnects(false), m_ppPlayerMgr(NULL), m_ppRouter(NULL), m_ppRaceMatchmaking(NULL), 
							m_pLanDiscoveryMgr(NULL), m_pSessionKeyTracker(NULL), m_pLanSecurity(NULL), m_pRaceModeListener(NULL),
							m_pMatchingMigrationNotification(NULL), m_MatchingMigrationNotificationId(0), m_MatchingMigrationState(eMigrationStateNotActive),
							m_MatchingMigrationTimer(0.0f), m_MatchingInitialDelayTimer(0.0f), m_BalanceTeams( true )
	{
	} 

	C_NetRace::~C_NetRace() 
	{
		Shutdown();
	}

	bool C_NetRace::Initialise(C_NetRouterBase** ppRouter, C_NetPlayerManager **ppPlayerManager, NetPlay::C_Session** ppRaceMatchmaking)
	{
		if (m_Initialised)
			return true;  
		
		BIZ_ASSERT(ppRouter != NULL);
		BIZ_ASSERT(*ppRouter != NULL);
		BIZ_ASSERT(ppPlayerManager != NULL);
		BIZ_ASSERT(*ppPlayerManager != NULL);
		BIZ_ASSERT(ppRaceMatchmaking != NULL);
		BIZ_ASSERT(*ppRaceMatchmaking != NULL);
		BIZ_ASSERT(m_MatchingToRemove.Capacity() >= NET_MAX_VEHICLES);

		m_ppRouter = ppRouter;
		m_ppPlayerMgr = ppPlayerManager;
		m_ppRaceMatchmaking = ppRaceMatchmaking;

		m_pLanDiscoveryMgr = new C_LanDiscoveryManager;
		m_pSessionKeyTracker = new C_NetSessionKeyTracker;
		m_pLanSecurity = new NetPlay::SessionSecurity;
		m_pMatchingMigrationNotification = new NetPlay::SessionCommandNotification;

		if (m_pLanDiscoveryMgr && m_pSessionKeyTracker && m_pLanSecurity && m_pMatchingMigrationNotification)
		{
			m_pLanDiscoveryMgr->Initialise(AMAX_LAN_ID);

			m_QoS.m_Version = NET_PACKET_VERSION;
			m_Initialised = true;
		}
		else
			Shutdown();

		return m_Initialised;
	}

	void C_NetRace::Shutdown(void)
	{
		DisconnectAndClear();
		
		if (m_pMatchingMigrationNotification)
			delete m_pMatchingMigrationNotification;
		m_pMatchingMigrationNotification = NULL;

		if (m_pLanSecurity)
			delete m_pLanSecurity;
		m_pLanSecurity = NULL;

		if (m_pSessionKeyTracker)
			delete m_pSessionKeyTracker;
		m_pSessionKeyTracker = NULL;

		if (m_pLanDiscoveryMgr)
			delete m_pLanDiscoveryMgr;
		m_pLanDiscoveryMgr = NULL;

		m_ppRaceMatchmaking = NULL;
		m_ppRouter = NULL;
		m_ppPlayerMgr = NULL;
		m_Initialised = false;
	}

	void C_NetRace::Poll(const F32 frameTimeInSeconds)
	{
		if (!m_Initialised)
			return;

		// track security keys still in use
		PollSecurityKeyTracker();

		// run join logic if required
		PollJoin();

		// track any failed join attempts to allow for message to reach peer before disconnecting.
		JoinPollFailures(*(*m_ppRouter));

		// setup who we send voice to.
		PollVoice();

		// update the teams
		PollTeamBalancing();

		// update matching server with latest player data.
		PollMatchingServer(false, frameTimeInSeconds);

		// pump lan discovery broadcast socket.
		m_pLanDiscoveryMgr->Poll();		

		// if hosting a lan game then this will keep the broadcast data right up to date.
		AdvertiseLanGame();

		// run disconnection logic
		PollDisconnects(frameTimeInSeconds);
	}

	void C_NetRace::DisconnectAndClear(void)
	{
		if (!m_Initialised)
			return;
	
		NetQoSRace blankQos;

		JoinSetPeerJoinFailureNameSetValid(false);

		// send a leaving session unreliable message to all connected peers.
		if (m_Running)
		{
			LOGDEBUG(g_Log, "C_NetRace::DisconnectAndClear()");

			if ((*m_ppPlayerMgr)->TotalSize() > 0)
			{
				const C_NetPlayer* pLocal = (*m_ppPlayerMgr)->Index(0);
				BIZ_ASSERT(pLocal->LocalPlayer());
				if (pLocal->m_RaceConnection.m_JoinId > 0)
				{
					C_NetMessage& msg = (*m_ppRouter)->GetOutgoingUnReliable();
					msg.Flush();
					bool ok = msg.CreateUnReliable(eNetMessage_Leaving, pLocal->m_RaceConnection.m_JoinId, false);
					ok = ok && msg.WriteU64(pLocal->m_ConsoleId);
					msg.TerminateMessage();

					// send to peers who have a connected socket.
					for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
					{
						C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
						if (p->LocalPlayer())
							continue;

						if (p->PrimaryPlayer() && p->m_RaceConnection.IsConnected())
						{
							for (U8 z=0; z<3; z++)
								(*m_ppRouter)->SendMessage(msg, p->m_RaceConnection.m_Address);
						}
					}
				}
			}

			DestroyAllPlayers();

			// flush any join information if we were in the process of joining.
			FlushJoinHelper(false, *(*m_ppRouter));

			if (!m_OnlineMode)
			{
				bdNetImpl *const net = bdNet::getInstance();
				if (net->getStatus() == bdNetImpl::BD_NET_DONE)
					net->getSocketRouter()->getKeyMap()->unregisterKey(m_pLanSecurity->m_Id);
			}
		}

		// matchmaking session
		(*m_ppRaceMatchmaking)->Delete();

		m_pMatchingMigrationNotification->Reset();
		m_MatchingMigrationNotificationId = 0;
		m_MatchingMigrationState = eMigrationStateNotActive;
		m_MatchingMigrationTimer = 0.0f;
		m_MatchingInitialDelayTimer = 0.0f;

		(*m_ppPlayerMgr)->Flush(false);
		m_pLanDiscoveryMgr->UnAdvertise();
		m_pSessionKeyTracker->Flush();
		m_MatchingToRemove.Clear();
		
		m_LocalMatchingSessionManagerIndex = 0;
		m_DisconnectResendMsgTimer = 0.0f;
		m_pRaceModeListener = NULL;
		m_QoS = blankQos;
		m_Migrating = false;
		m_SessionIsMigrating = false;
		m_VoiceEnabled = false;
		m_InRace = false;
		m_BigError = false;
		m_ForceMatchingServerModify = false;
		m_NewDisconnects = false;
		m_Running = false;
	}

	bool C_NetRace::CreateLanSession(C_NetCreationHelper& helper)
	{
		if (!m_Initialised || m_Running)
			return false;

		if (0 == helper.m_ConsoleId)
			return false; // invalid console identifier.

		m_OnlineMode = false;
		FlushJoinHelper(true, *(*m_ppRouter));
		
		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			if (CreateLocalPlayers(helper, true))
			{
				if(bdSecurityKeyMap::initKey(m_pLanSecurity->m_Id, m_pLanSecurity->m_Key))
				{
					if (NULL != net->getSocketRouter() && net->getSocketRouter()->getKeyMap()->registerKey(m_pLanSecurity->m_Id, m_pLanSecurity->m_Key))
					{	
						m_Running = true;
						m_BalanceTeams = true;

						JoinSetState(eJoinStateComplete);
						AdvertiseLanGame();
					}
				}
			}
		}

		if (!m_Running)
			DisconnectAndClear();
		return m_Running;
	}

	bool C_NetRace::CreateOnlineSession(C_NetCreationHelper& helper)
	{
		if (!m_Initialised || m_Running)
			return false;

		if (0 == helper.m_ConsoleId)
			return false; // invalid console identifier.

		m_OnlineMode = true;
		FlushJoinHelper(true, *(*m_ppRouter));

		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE && (*m_ppRaceMatchmaking)->SessionState() == NetPlay::C_Session::eStateRunning)
		{
			// get secure key data from matching session and store in our key tracker..
			NetPlay::SessionSecurity sec;
			if ((*m_ppRaceMatchmaking)->GetSecurityInfo(sec))
			{
				m_pSessionKeyTracker->Add(sec);

				if (CreateLocalPlayers(helper, true))
				{
					m_LocalMatchingSessionManagerIndex = helper.m_LocalSessionManagerIndex; // which local player actually controls the matching session.
					m_Running = true;
					m_BalanceTeams = true;
					JoinSetState(eJoinStateComplete);		
				}
			}
		}

		if (!m_Running)
			DisconnectAndClear();
		return m_Running;
	}

	bool C_NetRace::JoinLanSession(C_NetCreationHelper& helper, NetPlay::SessionSecurity& security, const U8* pHostCommonAddress, const U64 hostConsoleId)
	{
		if (!m_Initialised || m_Running || NULL == pHostCommonAddress)
			return false;

		if (0 == helper.m_ConsoleId)
			return false; // invalid coneole identifier.

		m_OnlineMode = false;
		FlushJoinHelper(true, *(*m_ppRouter));

		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			if (CreateLocalPlayers(helper, false))
			{
				// copy security data.
				(*m_pLanSecurity) = security;

				if (NULL != net->getSocketRouter() && net->getSocketRouter()->getKeyMap()->registerKey(m_pLanSecurity->m_Id, m_pLanSecurity->m_Key))
				{
					if (!JoinStart(*m_pLanSecurity, pHostCommonAddress, hostConsoleId))
					{
						FlushJoinHelper(true, *(*m_ppRouter));
						net->getSocketRouter()->getKeyMap()->unregisterKey(m_pLanSecurity->m_Id);
					}
					else
						m_Running = true;
				}
			}
		}

		if (!m_Running)
			DisconnectAndClear();
		return m_Running;
	}

	bool C_NetRace::JoinOnlineSession(C_NetCreationHelper& helper, NetPlay::SessionSecurity& security, const U8* pHostCommonAddress, const U64 hostConsoleId, const SysProfile::BaseProfile& hostName)
	{
		if (!m_Initialised || m_Running || NULL == pHostCommonAddress)
			return false;

		if (0 == helper.m_ConsoleId)
			return false; // invalid coneole identifier.

		m_OnlineMode = true;
		FlushJoinHelper(true, *(*m_ppRouter));

		// store the host name just incase we fail to connect.
		JoinSetPeerJoinFailureName(hostName);

		// which local player actually controls the matching session.
		m_LocalMatchingSessionManagerIndex = helper.m_LocalSessionManagerIndex; 

		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE && (*m_ppRaceMatchmaking)->SessionState() == NetPlay::C_Session::eStateRunning)
		{
			// store security info from the session we are about to join.
			m_pSessionKeyTracker->Add(security);

			if (CreateLocalPlayers(helper, false))
			{
				if (JoinStart(security, pHostCommonAddress, hostConsoleId))
					m_Running = true;
				else
					FlushJoinHelper(true, *(*m_ppRouter));
			}
		}

		if (!m_Running)
			DisconnectAndClear();
		return m_Running;
	}

	void C_NetRace::EnterLobby(void)
	{
		if (!m_Initialised || !m_Running)
			return;

		// must have joined
		BIZ_ASSERT(eJoinStateComplete == JoinGetState());
		if (eJoinStateComplete != JoinGetState())
			return; 

		bool statesUpdated = false;
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (p->LocalPlayer())
			{
				// voice becomes active once we enter the lobby.
				if (!m_VoiceEnabled)
				{
					(*m_ppPlayerMgr)->SetVoiceControlState(eVoiceControlStateRaceConnection);
					m_VoiceEnabled = true;
					SetupRemoteVoice();

					// update mute info for all remote players based on the data we currently have stored.
					for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
					{
						const C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
						if (p->LocalPlayer())
							continue;

						if (p->m_RaceConnection.IsActive())
							(*m_ppPlayerMgr)->ReceiveMuteList(p->m_RaceConnection.m_JoinId, p->m_RaceConnection.m_VoiceMuteData, false);
					}

					//	Piggy-back on the fact that this section is only run once when the player(s) first join the session, and cache their current race points
					Game::C_GameProfile* gameProf = Game::C_GameProfileManager::Instance().GetProfile(p->m_ControllerPort);	
					if (gameProf)
						gameProf->mMultiplayerProfile.ResetSessionStats();

					//	Clear the MP results (as we've just joined a new session).
					Game::C_MpResults::Instance().Clear();
				}

				if (eNetPlayerStateIdle != p->m_RaceConnection.m_PlayerState)
				{
					// upon entering lobby player goes back to idle state
					// states updated means we will post the local players new states to connected peers.
					statesUpdated = true;
					p->m_RaceConnection.m_PlayerState = eNetPlayerStateIdle;
				}
			}
		}

		if (statesUpdated)
			SendLocalPlayerStatesMessage();
	}

	bool C_NetRace::ReceiveReliable(C_NetMessage& msg, const U8 msgType, const U8 senderId)
	{
		if (!m_Initialised || !m_Running)
			return false;

		switch (msgType)
		{
		case eNetMessage_JoinData:
			ReceiveJoinDataMessage(msg, senderId);
			return true;
			break;

		case eNetMessage_NewRaceData:
			ReceiveNewRaceDataMessage(msg, senderId);
			return true;
			break;

		case eNetMessage_InitialData:
			ReceiveInitialDataMessage(msg, senderId);

			m_BalanceTeams = true; // only rebalance once player fully joins the game.
			return true;
			break;

		case eNetMessage_PlayerStates:
			ReceivePlayerStatesMessage(msg, senderId);
			return true;
			break;

		case eNetMessage_DisconnectConsoles:
			ReceiveDisconnectConsolesMessage(msg, senderId);
			return true;
			break;

		case eNetMessage_Migrate:
			ReceiveMigrateMessage(msg, senderId);
			return true;
			break;

		case eNetMessage_MigrateMatchingSession:
			ReceiveMigrateMatchingSessionMessage(msg, senderId);
			return true;
			break;

		case eNetMessage_MigrateMatchingSessionReply:
			ReceiveMigrateMatchingSessionReplyMessage(msg, senderId);
			return true;
			break;

		case eNetMessage_MuteChange:
			ReceiveMuteChangeMessage(msg, senderId);
			return true;
			break;
		
		case eNetMessage_CommunicatorStatusUpdate:
			ReceiveCommunicatorStatusChangedMessage(msg, senderId);
			return true;
			break;
		}
		
		// try and pass message to game mode and race mode controllers
		Game::C_RaceBookLayer* pRBLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if (pRBLayer && pRBLayer->GetGameModePtr()->IsNetwork())
		{
			if (pRBLayer->GetGameModePtr()->ReceiveReliableNetMessage(msg, msgType, senderId, false))
				return true;

			if (pRBLayer->GetRaceModePtr()->ReceiveReliableNetMessage(msg, msgType, senderId, false))
				return true;
		}

		return false;
	}

	bool C_NetRace::ReceiveUnReliable(C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, bdAddrHandleRef& addr)
	{
		if (!m_Initialised)
			return false;

		// if not running then just disconnect the address
		if (!m_Running)
		{
			(*m_ppRouter)->Disconnect(addr);
			return true;
		}

		switch (msgType)
		{
		case eNetMessage_Leaving:
			ReceiveLeavingMessage(msg, senderId);
			return true;
			break;
		}

		// try and pass message to game mode and race mode controllers.
		Game::C_RaceBookLayer* pRBLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if (pRBLayer && pRBLayer->GetGameModePtr()->IsNetwork())
		{
			if (pRBLayer->GetGameModePtr()->ReceiveUnReliableNetMessage(msg, msgType, numUnEncryptedBytes, senderId, false))
				return true;

			if (pRBLayer->GetRaceModePtr()->ReceiveUnReliableNetMessage(msg, msgType, numUnEncryptedBytes, senderId, false))
				return true;
		}

		return false;
	}

	bool C_NetRace::ReceiveJoinRequest(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, bdAddrHandleRef& addr)
	{
		if (!m_Initialised)
			return false;

		if (addr.isNull())
			return false;

		// if not running then just disconnect the address
		if (!m_Running)
		{
			(*m_ppRouter)->Disconnect(addr);
			return true;
		}

		NetJoinReferencesSingleConsole joined;	// will push back any player who joined into this stack for tracking.
		
		bool ok = msg.ReadOpen();
		
		// read and compare packet version
		U32 packetVersion = 0;
		ok = ok && msg.ReadU32(packetVersion);
		ok = (packetVersion == NET_PACKET_VERSION) ? true : false;
		if (!ok)
		{
			// incorect packet version means quick disconnect.
			msg.ReadClose();
			(*m_ppRouter)->Disconnect(addr);
			return true;
		}
		
		// read my join ID as supplied by the peer. 
		U8 myPrimaryJoinId = 0;
		ok = ok && msg.ReadU8(myPrimaryJoinId);
		if (!ok)
		{
			// unable to read my join id.
			msg.ReadClose();
			(*m_ppRouter)->Disconnect(addr);
			return true;
		}

		// if i actually have a valid join id ovewwrite what was supplied by the joining peer.
		if ((*m_ppPlayerMgr)->LocalPrimaryJoinId(false) != 0)
		{
			myPrimaryJoinId = (*m_ppPlayerMgr)->LocalPrimaryJoinId(false);
		}
		else
		{
			// set local primary join id since we have yet to receive one from the host, its possible multiple
			// peers are joining and we have yet to recive the join reply from the host.
			C_NetPlayer* pLocalPrimary = (*m_ppPlayerMgr)->Index(0);
			BIZ_ASSERT(pLocalPrimary->LocalPlayer());
			pLocalPrimary->m_RaceConnection.m_JoinId = myPrimaryJoinId;
		}

		// read console Ids
		U64 hostConsoleId, joiningConsoleId;
		ok = ok && msg.ReadU64(hostConsoleId);
		ok = ok && msg.ReadU64(joiningConsoleId);

		// if this console is already in our join failure list then we can't allow join since we may have already sent the join failure
		// reply message.
		if (JoinIsConsoleFlaggedToFail(joiningConsoleId))
		{
			msg.ReadClose();
			return true;
		}

#if !defined ( CFG_GOLD )
		// debug testing for join failures..
		if (g_Join_ForceError_Generic)
		{
			JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorNoConnectionToHost, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
			return true;
		}
		else if (g_Join_ForceError_SlotCount)
		{
			JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorNotEnoughOpenSlots, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
			return true;
		}
		else if (g_Join_ForceError_Banned)
		{
			JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorBannedFromSession, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
			return true;
		}
		else if (g_Join_ForceError_NoComms)
		{
			SysProfile::BaseProfile temp;
			UI::WideStrCopy(temp.m_Name, SYSTEM_PROFILE_NAME_LENGTH , L"WWWWWWWWWWWWWWW");
			JoinFailConnectionWithName(eNetMessage_JoinFailure,  eJoinErrorNoCommunications, temp, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
			return true;
		}
		else if (g_Join_ForceError_Teams)
		{
			JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorToManyTeams, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
			return true;
		}
		else if (g_Join_ForceError_TeamMembers)
		{
			JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorToManyTeamMembers, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
			return true;
		}
		else if (g_Join_ForceError_ContentMismatch)
		{
			JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorContentMismatch, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
			return true;
		}
		else if (g_Join_ForceError_Started)
		{
			JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorChampionshipStarted, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
			return true;
		}
#endif


		if (ok)
		{
			// does the host console id match that of the current race host ?
			C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(false);
			if (NULL != pHost && hostConsoleId == pHost->m_ConsoleId && pHost->m_RaceConnection.IsActive() && eNetPlayerDisconnectionTypeNone == pHost->m_RaceConnection.m_DisconnectRequested)
			{
				// check is this player already in our race session.
				bool addPlayers = false;
				C_NetPlayer *pPeer = (*m_ppPlayerMgr)->FindFromConsoleId(joiningConsoleId, 0);
				if (NULL == pPeer)
					addPlayers = true;
				else if (!pPeer->m_RaceConnection.IsActive())
					addPlayers = true;

				if (!addPlayers)
				{
					// players from the joinng console id are already defined in our race session so we ignore this message.
					msg.ReadClose();
					return true;
				}
				else
				{
					// read generic data.
					U8 commonAddr[BD_COMMON_ADDR_SERIALIZED_SIZE];
					U16 bandwidthUp = 0;
					U16 bandwidthDown = 0;
					U32 dlcMask = 0;
					U8 voiceBanned = 0;
					NetPlayerProfileGameData profile;
					U8 numToJoin = 0;

					ok = ok && msg.ReadBuffer(commonAddr, BD_COMMON_ADDR_SERIALIZED_SIZE);
					ok = ok && msg.ReadU16(bandwidthUp);
					ok = ok && msg.ReadU16(bandwidthDown);
					ok = ok && msg.ReadU32(dlcMask);
					ok = ok && msg.ReadU8(numToJoin);

					if (!ok || numToJoin > NET_MAX_LOCAL_PLAYERS || 0 == numToJoin)
					{
						// dodgy data.
						msg.ReadClose();
						(*m_ppRouter)->Disconnect(addr);
					}
					else
					{
						// dlc check
						bool dlcMismatch = false;
						NetRaceDlcSettings dlcSettings;
						GenerateDlcSettings(dlcSettings);

						for (U32 q=0; q<32 && !dlcMismatch; q++)
						{
							if (dlcSettings.m_AdditionalContentPacksInUse&(1<<q))
							{
								if (!(dlcMask&(1<<q)))
									dlcMismatch = true;
							}
						}

						if (dlcMismatch)
						{
							msg.ReadClose();
							JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorContentMismatch, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
							return true;
						}

						if (pHost->LocalPlayer())
						{
							if (!CheckIfChampionshipJoinsAllowed())
							{
								// fail because not championship has started.
								msg.ReadClose();
								JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorChampionshipStarted, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
								return true;
							}
							else
							{
								NetRaceSettings raceSettings;
								GetCurrentRaceSettings(raceSettings);

								// host checks for available player slots.
								const U8 remainingSlots = raceSettings.m_MaxDrivers - (*m_ppPlayerMgr)->Size(false);
								if (remainingSlots < numToJoin)
								{
									// fail because not enough player slots remaining.
									msg.ReadClose();
									JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorNotEnoughOpenSlots, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
									return true;
								}
							}
						}

						// create the joining players.
						for (U8 i=0; i<numToJoin && ok; i++)
						{
							SysProfile::BaseProfile bs;
							U8 joinId;

							ok = ok && msg.ReadBuffer(&bs, sizeof(bs));
							ok = ok && msg.ReadU8(voiceBanned);
							ok = ok && msg.ReadBuffer(&profile, sizeof(profile));
							ok = ok && msg.ReadU8(joinId);

							if (ok)
							{
								if (m_OnlineMode)
								{
									// if peer packet routing is enabled we dont need to fail players because of nat issues.
									if (!(*m_ppRouter)->PeerRoutingEnabled())
									{
										// check against most restrictive nat already in the game.
										SysProfile::BaseProfile natUser;
										if (!(*m_ppPlayerMgr)->CheckAgainstMostRestrictiveNat(false, profile.m_NatType, natUser))
										{
											LOGINFO(g_Log, "'%S' join Denied because of NAT incompatbility with current peers.", bs.Name());
											JoinFailConnectionWithName(eNetMessage_JoinFailure,  eJoinErrorNoCommunications, natUser, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);

											// destroy any players we may have already setup.
											for (S32 i=0; i<joined.Size(); i++)
												DestroyPlayer(joined[i]);

											msg.ReadClose();
											return true;
										}
									}
								}

								C_NetPlayer* pRacePlayer = NULL;

								// does a valid party conenction exist ? if not then a new player structure will be allocated.
								C_NetPlayer * pPartyPlayer = (*m_ppPlayerMgr)->FindFromConsoleId(joiningConsoleId, i);
								if (NULL == pPartyPlayer)
									pRacePlayer = (*m_ppPlayerMgr)->AllocatePlayer(true);
								else
									pRacePlayer = pPartyPlayer; 

								if (pRacePlayer)
								{
									// notify game logic a new player has joined the our session.
									// only called if we have caled EnterLobby()
									if (m_VoiceEnabled)
										C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->PlayerNotification(bs.Name(), true, false);

									// overwrite generic player data. safe to do this even if party conenction exists as the data will be the same (hopefully).
									pRacePlayer->m_User = bs;

									for (U32 z=0; z<BD_COMMON_ADDR_SERIALIZED_SIZE; z++)
										pRacePlayer->m_CommonAddress[z] = commonAddr[z];		

									pRacePlayer->m_ConsoleId		= joiningConsoleId;												
									pRacePlayer->m_UpStreamKbps		= bandwidthUp;												
									pRacePlayer->m_DownStreamKbps	= bandwidthDown;	
									pRacePlayer->m_DlcMask			= dlcMask;
									pRacePlayer->m_VoiceBanned		= voiceBanned != 0 ? true : false;
									pRacePlayer->m_ProfileData		= profile;
									pRacePlayer->m_LocalIndex		= i;		

									// copy race specific data.
									pRacePlayer->m_RaceConnection.m_PlayerState = eNetPlayerStateJoining;
									pRacePlayer->m_RaceConnection.m_HandshakingComplete = true;

									// store remote address for primary player
									if (i == 0)
										pRacePlayer->m_RaceConnection.m_Address = addr;

									// if its an invalid join id then allocate one now, only host console will need to be doing this.
									if (0 == joinId)
									{
										BIZ_ASSERT(pHost->LocalPlayer());
										
										NetRaceSettings raceSettings;
										GetCurrentRaceSettings(raceSettings);

										pRacePlayer->m_RaceConnection.m_JoinId = (*m_ppPlayerMgr)->GenerateJoinReference(false);
									}
									else
									{
										pRacePlayer->m_RaceConnection.m_JoinId = joinId;
									}

									// enable keepalives now
									pRacePlayer->m_RaceConnection.StartKeepAliveTimer();

									// player is now valid so push into joined list.
									BIZ_ASSERT(pRacePlayer->m_RaceConnection.m_JoinId != 0); // 0 is an invalid join id
									joined.PushBack(pRacePlayer->m_RaceConnection.m_JoinId);

									// if online mode then we need to update the matching server data.
									m_ForceMatchingServerModify = m_OnlineMode ? true : false;
								}
								else 
									ok = false;
							}
						}

						if (ok)
						{
							BIZ_ASSERT(joined.Size() > 0); 

							// send back join reply 
							C_NetPlayer* pJoinReplyPlayer = (*m_ppPlayerMgr)->FindFromJoinId(joined[0], false);
							BIZ_ASSERT(pJoinReplyPlayer != NULL);
							C_NetMessage& reply = (*m_ppRouter)->GetOutgoingReliable();
							reply.Flush();

							if (reply.CreateReliable())
							{
								// does this join reply come from the host ? if so it will contain updated data for the recipient.. join Id's etc.
								if (pHost->LocalPlayer())
								{
									// peer joining the host
									//----------------------

									ok = ok && reply.WriteU8(1);					// updated local player data follows
									ok = ok && reply.WriteU8((U8)joined.Size());	// how many updated local players data follows.
									for (S32 z=0; z<joined.Size(); z++)
									{
										ok = ok && reply.WriteU8(joined[z]); // pack updated join id for this new player
									}

									C_NetPlayer* pPrimaryLocalPlayer = (*m_ppPlayerMgr)->Index(0);
									BIZ_ASSERT(pPrimaryLocalPlayer != NULL);
									BIZ_ASSERT(pPrimaryLocalPlayer->LocalPlayer());

									// shared stuff first.
									ok = ok && reply.WriteU64(pPrimaryLocalPlayer->m_ConsoleId);
									ok = ok && reply.WriteBuffer(pPrimaryLocalPlayer->m_CommonAddress, BD_COMMON_ADDR_SERIALIZED_SIZE);
									ok = ok && reply.WriteU16(pPrimaryLocalPlayer->m_UpStreamKbps);
									ok = ok && reply.WriteU16(pPrimaryLocalPlayer->m_DownStreamKbps);
									ok = ok && reply.WriteU32(pPrimaryLocalPlayer->m_DlcMask);
									
									const U8 localPlayerCount = (*m_ppPlayerMgr)->LocalSize();
									ok = ok && reply.WriteU8(localPlayerCount);
									// player specific stuff.
									for (U8 z=0; z<(*m_ppPlayerMgr)->TotalSize(); z++)
									{
										C_NetPlayer* pH = (*m_ppPlayerMgr)->Index(z);
										if (!pH->LocalPlayer())
											continue;

										ok = ok && reply.WriteBuffer(&(pH->m_User), sizeof(pH->m_User));
										ok = ok && reply.WriteU8(pH->m_RaceConnection.m_JoinId);
										ok = ok && reply.WriteU8(pH->m_VoiceBanned ? 1 : 0);
										ok = ok && reply.WriteBuffer(&(pH->m_ProfileData), sizeof(pH->m_ProfileData));
										ok = ok && reply.WriteU8(pH->m_RaceConnection.m_Host);	
										ok = ok && reply.WriteU8(pH->m_RaceConnection.m_PlayerState);
									}
								}
								else
								{
									// peer joining another peer.
									//-----------------------------

									// updated local joining player data does not follow.
									ok = ok && reply.WriteU8(0); 
									
									// console id of the player player who sent this reply.
									ok = ok && reply.WriteU64((*m_ppPlayerMgr)->Index(0)->m_ConsoleId); 

									// basic peer data for all local players
									const U8 localPlayerCount = (*m_ppPlayerMgr)->LocalSize();
									ok = ok && reply.WriteU8(localPlayerCount);

									for (U8 q=0; q<(*m_ppPlayerMgr)->TotalSize(); q++)
									{
										C_NetPlayer* pPeer = (*m_ppPlayerMgr)->Index(q);
										if (!pPeer->LocalPlayer())
											continue;

										ok = ok && reply.WriteU8(pPeer->m_LocalIndex);
										ok = ok && reply.WriteU8(pPeer->m_RaceConnection.m_PlayerState);
									}
								}

								reply.TerminateMessage();

								// add message into reliable message queue of primary remote player.
								// flag as initial connection mesage.
								if (ok)
									pJoinReplyPlayer->SendReliable(reply, eNetMessage_JoinReply, false, myPrimaryJoinId, true);

								// if hosting we need to send back a list of what other peers the joining player needs to make a connection to.
								// this message also contains the latest race settings.
								if (ok && pHost->LocalPlayer())
								{
									// build a list of additional peer data that need to go to the joining peer
									NetJoinReferences additionalPeers;
									for (U8 z=0; z<(*m_ppPlayerMgr)->TotalSize(); z++)
									{
										C_NetPlayer* pAdditional = (*m_ppPlayerMgr)->Index(z);
										if (pAdditional->LocalPlayer() || !pAdditional->m_RaceConnection.IsActive())
											continue;

										bool add = true;
										for (S32 q=0; q<joined.Size(); q++)
										{
											if (joined[q] == pAdditional->m_RaceConnection.m_JoinId)
												add = false;
										}

										if (add)
											additionalPeers.PushBack(pAdditional->m_RaceConnection.m_JoinId);
									}

									NetRaceSettings raceSettings;
									GetCurrentRaceSettings(raceSettings);

									reply.Flush();
									ok = ok && reply.CreateReliable();
									ok = ok && reply.WriteBuffer(&raceSettings, sizeof(raceSettings));
									ok = ok && reply.WriteU8((U8)additionalPeers.Size());

									for (U8 z=0; z<additionalPeers.Size(); z++)
									{
										C_NetPlayer* pAdditional = (*m_ppPlayerMgr)->FindFromJoinId(additionalPeers[z], false);
										BIZ_ASSERT(pAdditional != NULL);

										ok = ok && reply.WriteU64(pAdditional->m_ConsoleId);
										ok = ok && reply.WriteU8(pAdditional->m_LocalIndex);
										ok = ok && reply.WriteBuffer(pAdditional->m_CommonAddress, BD_COMMON_ADDR_SERIALIZED_SIZE);
										ok = ok && reply.WriteU16(pAdditional->m_UpStreamKbps);
										ok = ok && reply.WriteU16(pAdditional->m_DownStreamKbps);
										ok = ok && reply.WriteU32(pAdditional->m_DlcMask);
										ok = ok && reply.WriteU8(pAdditional->m_VoiceBanned ? 1 : 0);
										ok = ok && reply.WriteBuffer(&(pAdditional->m_ProfileData), sizeof(pAdditional->m_ProfileData));
										ok = ok && reply.WriteBuffer(&(pAdditional->m_User), sizeof(pAdditional->m_User));	
										ok = ok && reply.WriteU8(pAdditional->m_RaceConnection.m_JoinId);
										ok = ok && reply.WriteU8(pAdditional->m_RaceConnection.m_PlayerState);
										ok = ok && reply.WriteU8(pAdditional->m_RaceData.m_Team);
									}

									reply.TerminateMessage();

									if (ok)
										pJoinReplyPlayer->SendReliable(reply, eNetMessage_JoinData, false, myPrimaryJoinId);
								}

							}
							else
								ok = false;
						}
					}
				}
			}
			else
			{ 
				// fail the join here because of different hosts defined.
				msg.ReadClose();
				JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorHostDiffersOnPeer, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), false);
				return true;
			}
		}
		else 
		{	
			msg.ReadClose();
			(*m_ppRouter)->Disconnect(addr);
			return true;
		}
		
		if (!ok)
		{
			// some big error occurred that caused us to have to disconnect this connection.
			// destroy any players we may have already added
			for (S32 i=0; i<joined.Size(); i++)
				DestroyPlayer(joined[i]);

			if (addr.notNull())
				(*m_ppRouter)->Disconnect(addr);
		}

		msg.ReadClose();
		return true;
	}

	bool C_NetRace::ReceiveJoinFailure(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, bdAddrHandleRef& addr)
	{
		if (!m_Initialised)
			return false;

		// if not running then just disconnect the address
		if (!m_Running)
		{
			(*m_ppRouter)->Disconnect(addr);
			return true;
		}

		if (IsJoining())
		{
			bool ok = msg.ReadOpen();
			U8 errorCode = eJoinErrorInvalid;
			SysProfile::BaseProfile prof;

			ok = ok && msg.ReadU8(errorCode);
			ok = ok && msg.ReadBuffer(&prof, sizeof(SysProfile::BaseProfile));

			if (ok)
			{
				bool isOnline = m_OnlineMode;

				DisconnectAndClear();
				JoinSetFailed(static_cast<eNetJoinSessionErrors> (errorCode), *(*m_ppRouter));

				// this case is to catch the fact that the host knows we cant join to a peer so why try.
				// the host gives us back the name of the peer who we wwould not beable to connect to.
				if (isOnline && eJoinErrorNoCommunications == errorCode)
				{
					JoinSetPeerJoinFailureName(prof);
					JoinSetPeerJoinFailureNameSetValid(true);
				}
			}
			else
			{
				LOGWARN(g_Log, "C_NetRace::ReceiveJoinFailure() - invalid packet. ");
			}
		}
		else
		{
			// if idle then disconenct this address..
			if (JoinGetState() != eJoinStateComplete)
				(*m_ppRouter)->Disconnect(addr);
		}

		msg.ReadClose();
		return true;
	}

	bool C_NetRace::ReceiveJoinReply(C_NetMessage& msg, const DatagramReliablePacketHeader& hdr, bdAddrHandleRef& addr, const U8 senderId)
	{
		if (!m_Initialised)
			return false;

		if (addr.isNull())
			return false;

		// if not running then just disconnect the address
		if (!m_Running)
		{
			(*m_ppRouter)->Disconnect(addr);
			return true;
		}

		if (!IsJoining())
			return true; // already joined so ignore
	
		bool ok = msg.ReadOpen();
		if (ok)
		{
			U8 hostReply = 0;
			ok = ok && msg.ReadU8(hostReply);
			if (ok)
			{
				if (hostReply)
				{
					// reply from the host to our initial join request.
					if (JoinGetState() == eJoinStateWaitingForHostReply)
					{
						// still waiting for host data so read updated local player data, supplied by the host.
						U8 numLocal = 0;
						ok = ok && msg.ReadU8(numLocal);
						if (ok && numLocal == (*m_ppPlayerMgr)->LocalSize())
						{
							for (U8 i=0; i<numLocal; i++)
							{
								U8 joinId = 0;
								ok = ok && msg.ReadU8(joinId);
								if (ok)
								{
									C_NetPlayer* pLocal = (*m_ppPlayerMgr)->Index(i);
									BIZ_ASSERT(pLocal->LocalPlayer());
									pLocal->m_RaceConnection.m_JoinId = joinId;
								}
							}

							// now read the player config data on the host console and create those network players.
							if (ok)
							{
								// read generic data.
								U64 consoleId;
								U8 commonAddr[BD_COMMON_ADDR_SERIALIZED_SIZE];
								U16 bandwidthUp = 0;
								U16 bandwidthDown = 0;
								U32 dlcMask = 0;
								U8 voiceBanned = 0;
								NetPlayerProfileGameData profile;
								U8 numToCreate = 0;

								ok = ok && msg.ReadU64(consoleId);
								ok = ok && msg.ReadBuffer(commonAddr, BD_COMMON_ADDR_SERIALIZED_SIZE);
								ok = ok && msg.ReadU16(bandwidthUp);
								ok = ok && msg.ReadU16(bandwidthDown);
								ok = ok && msg.ReadU32(dlcMask);
								ok = ok && msg.ReadU8(numToCreate);
									
								if (ok && consoleId == JoinGetHostConsoleId())
								{
									for (U8 i=0; i<numToCreate; i++)
									{
										SysProfile::BaseProfile bs;
										U8 joinid = 0;
										U8 host = 0;
										U8 state = 0;

										ok = ok && msg.ReadBuffer(&bs, sizeof(bs));
										ok = ok && msg.ReadU8(joinid);
										ok = ok && msg.ReadU8(voiceBanned);
										ok = ok && msg.ReadBuffer(&profile, sizeof(profile));
										ok = ok && msg.ReadU8(host);
										ok = ok && msg.ReadU8(state);

										if (ok)
										{
											// does a valid party conenction exist ? if not then a new player structure will be allocated.
											C_NetPlayer* pRacePlayer = NULL;
											
											C_NetPlayer *pPartyPlayer = (*m_ppPlayerMgr)->FindFromConsoleId(consoleId, i);
											if (NULL == pPartyPlayer)
												pRacePlayer = (*m_ppPlayerMgr)->AllocatePlayer(true);
											else
												pRacePlayer = pPartyPlayer; 

											if (pRacePlayer)
											{
												pRacePlayer->m_User				= bs;
												pRacePlayer->m_ConsoleId		= consoleId;												
												pRacePlayer->m_UpStreamKbps		= bandwidthUp;												
												pRacePlayer->m_DownStreamKbps	= bandwidthDown;	
												pRacePlayer->m_DlcMask			= dlcMask;
												pRacePlayer->m_VoiceBanned		= voiceBanned != 0 ? true : false;
												pRacePlayer->m_ProfileData		= profile;
												pRacePlayer->m_LocalIndex		= i;		

												for (U32 z=0; z<BD_COMMON_ADDR_SERIALIZED_SIZE; z++)
													pRacePlayer->m_CommonAddress[z] = commonAddr[z];	

												// copy race specific data.
												pRacePlayer->m_RaceConnection.m_PlayerState = eNetPlayerStateJoining;
												pRacePlayer->m_RaceConnection.m_HandshakingComplete = true;
												pRacePlayer->m_RaceConnection.m_JoinId = joinid;
												pRacePlayer->m_RaceConnection.m_Host = (host == 1) ? true : false;
												pRacePlayer->m_RaceConnection.m_PlayerState = static_cast<eNetPlayerState> (state);

												// store remote address for primary player only
												if (i == 0)
													pRacePlayer->m_RaceConnection.m_Address = addr;

												pRacePlayer->m_RaceConnection.StartKeepAliveTimer();
											}
											else
												ok = false;
										}
									}
								}
								else
									ok = false;
							}
						}
						else
							ok = false;

						// move to next state in join procedure.
						if (ok)
							JoinSetState(eJoinStateWaitingForPeerData);
						else
						{
							DisconnectAndClear();
							JoinSetFailed(eJoinErrorNoConnectionToHost, *(*m_ppRouter));
						}
					}
				}
				else
				{
					// reply from a peer who we attempted to join
					if (JoinGetState() == eJoinStateConnectingPeers)
					{
						U64 consoleId = 0;
						U8 count = 0;
						ok = ok && msg.ReadU64(consoleId); 
						ok = ok && msg.ReadU8(count);
						if (ok && consoleId > 0)
						{
							for (U8 i=0; i<count; i++)
							{
								U8 state =  0;
								U8 index = 0;
								ok = ok && msg.ReadU8(index);
								ok = ok && msg.ReadU8(state);
								if (ok)
								{
									C_NetPlayer* p = (*m_ppPlayerMgr)->FindFromConsoleId(consoleId, index);
									if (p)
									{
										BIZ_ASSERT(!p->LocalPlayer());
										BIZ_ASSERT(p->m_RaceConnection.IsActive());

										p->m_RaceConnection.m_HandshakingComplete = true;
										p->m_RaceConnection.m_PlayerState = static_cast<eNetPlayerState> (state);
										p->m_RaceConnection.StartKeepAliveTimer();
									}
								}
							}
						}
						
						if (!ok)
						{
							DisconnectAndClear();
							JoinSetFailed(eJoinErrorNoCommunications, *(*m_ppRouter));
						}
					}
				}
			}
			msg.ReadClose();
		}

		return true;
	}

	void C_NetRace::VoiceMuteChanged(void)
	{
		if (!m_Initialised || !m_Running)
			return;

		// realtime mute only applicable once we have fully joined a lobbt.
		if (JoinGetState() != eJoinStateComplete)
			return;  

		C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
		msg.Flush();

		const U8 localPlayerCount = (*m_ppPlayerMgr)->LocalSize();

		bool ok = msg.CreateReliable();
		ok = ok && msg.WriteU8(localPlayerCount);
		for (U8 i=0; i<localPlayerCount; i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			BIZ_ASSERT(p->LocalPlayer());

			NetMuteListJoinReferences muteList;
			(*m_ppPlayerMgr)->BuildMuteList(i, muteList, false);

			ok = ok && msg.WriteU8(p->m_RaceConnection.m_JoinId);
			ok = ok && msg.WriteBuffer(&muteList, sizeof(muteList));
		}
		msg.TerminateMessage();

		if (ok)
			SendReliableMessage(msg, eNetMessage_MuteChange);
		else
		{
			LOGWARN(g_Log, "C_NetRace::MuteInfoModified() - unable to create mute message.");
		}
	}

	void C_NetRace::SendLocalPlayerStatesMessage(void)
	{
		if (!m_Initialised || !m_Running && JoinGetState() > eJoinStateWaitingForHostReply)
			return;

		const U8 numLocal = (*m_ppPlayerMgr)->LocalSize();
		if (0 == numLocal) 
			return;

		C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
		msg.CreateReliable();

		U8 added = 0;
		bool ok = false;

		ok = msg.WriteU8(numLocal); // write number of state we will be sending.

		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			const C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (!p->LocalPlayer())
				continue;

			BIZ_ASSERT(p->m_RaceConnection.m_JoinId != 0);
			added += 1;
			
			ok = ok && msg.WriteU8(p->m_RaceConnection.m_JoinId);				// write player identifier
			ok = ok && msg.WriteU8(p->m_RaceConnection.m_PlayerState);			// write actual player state

			if (!ok)
			{
				LOGERROR(g_Log, "C_NetRace::SendLocalPlayerStatesMessage() - failed to build network message");
				return;
			}
		}
		
		msg.TerminateMessage();
		if (0 == added)
			return;

		// attempt tos end message to connected peers
		SendReliableMessage(msg, eNetMessage_PlayerStates);
	}

	void C_NetRace::GenerateDlcSettings(NetRaceDlcSettings& content)
	{
		content.Clear();
		if (!m_Initialised || !m_Running)
			return;

		U32 availableToAll = 0;
		U32 inUse = 0;

		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p= (*m_ppPlayerMgr)->Index(i);
			if (!p->m_RaceConnection.IsActive())
				continue;
			if (p->m_RaceData.m_VehicleData.m_VehicleContentPack > 0)
				inUse |= (1 << p->m_RaceData.m_VehicleData.m_VehicleContentPack);
		}

		for (U32 z=0; z<32; z++)
		{
			// make sure all current peers have access to this content pack.
			bool addPack = true;
			for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
			{
				C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
				if (!p->m_RaceConnection.IsActive())
					continue;

				if (!(p->m_DlcMask&(1<<z)))
					addPack = false;
			}

			if (addPack)
				availableToAll |= (1 << z);
		}

		// everybody should have bit 0 set since this defines shipped content.
		BIZ_ASSERT(availableToAll&(1<<0));

		content.m_AdditionalContentPacksAvailable = availableToAll;
		content.m_AdditionalContentPacksInUse = inUse;
	}

	bool C_NetRace::ValidateVehicle(const U32 xtVehicleId)
	{
		if (!m_Initialised || !m_Running)
			return true;
	
		NetRaceSettings raceSettings;
		GetCurrentRaceSettings(raceSettings);

		NetRaceDlcSettings dlcSettings;
		GenerateDlcSettings(dlcSettings);

		AmaxVehicle * pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)xtVehicleId];
		if (NULL == pVehicle)
			return false;

		// make sure vehicle is from a content pack that all peers can reference.
		if (!(dlcSettings.m_AdditionalContentPacksAvailable&(1<<pVehicle->contentPack)))
			return false;

		// check vehicle against class restrictions
		// [GavinY] - this code assumes that the UI and game car class enums match up, so if they change this will need to too.
		if( !((Game::C_GameModeMultiPlayer*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr())->IsCarClassValid(pVehicle->carClass) )
			return false;

		return true;
 	}

	void C_NetRace::DisconnectIdleLoadingPlayers( void )
	{
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			RacerNet::C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (p->m_RaceConnection.IsActive() && p->m_RaceData.m_InRaceVehicleIndex != -1)
			{
				if (RacerNet::eNetPlayerStateWaitingForOthers > p->m_RaceConnection.m_PlayerState)
				{
					LOGINFO(g_Log, "C_NetRace::DisconnectIdleLoadingPlayers() - gave up waiting for %S to Ready after load", p->m_User.Name());
					m_NewDisconnects = true;
					p->m_RaceConnection.m_DisconnectRequested = eNetPlayerDisconnectionTypeLostConnection;

					// if i also have a party connection to this player then i need to disconnect from that also.
					if (p->m_PartyConnection.IsActive())
						p->m_PartyConnection.m_DisconnectRequested = eNetPlayerDisconnectionTypeLostConnection;
				}
			}
		}
	}

	void C_NetRace::RegisterRaceLogicListener(C_NetRaceModeListener* pListener)
	{
		if (!m_Initialised || !m_Running)
			return;
		m_pRaceModeListener = pListener;
	}

	void C_NetRace::SendReliableMessage(C_NetMessage& msg, const U8 messageId)
	{
		if (!m_Initialised || !m_Running)
			return;

		// put message into the peers message queues.
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (p->LocalPlayer())
				continue;
			if (p->PrimaryPlayer() && p->m_RaceConnection.CanMessage() && eNetPlayerDisconnectionTypeNone == p->m_RaceConnection.m_DisconnectRequested)
				p->SendReliable(msg, messageId, false, (*m_ppPlayerMgr)->LocalPrimaryJoinId(false), false);
		}
	}

	bool C_NetRace::JoinInProgressAvailable(bdSecurityID& sessionId)
	{
		if (!m_Initialised || !m_Running || !m_OnlineMode || m_Migrating)
			return false;

		if (JoinGetState() != eJoinStateComplete)
			return false;

		if (!CheckIfChampionshipJoinsAllowed())
			return false;
		
		NetRaceSettings settings;
		GetCurrentRaceSettings(settings);

		if ((*m_ppPlayerMgr)->Size(false) < settings.m_MaxDrivers)
		{
			// still room for more players in the session.
			NetPlay::SessionSecurity sec;
			if (m_pSessionKeyTracker->Current(sec))
			{
				sessionId = sec.m_Id;
				return true;
			}
		}
		
		return false;
	}

	bool C_NetRace::CanInvite(NetPlay::SessionSecurity& sessionSec)
	{
		bdSecurityID id;
		if (JoinInProgressAvailable(id))
		{
			if (m_pSessionKeyTracker->Current(sessionSec))
				return true;
		}
		return false;
	}

	bool C_NetRace::InSession(const bdSecurityID& sessionId)
	{
		if (!m_Initialised || !m_Running)
			return false;
		return m_pSessionKeyTracker->IsRegistered(sessionId);
	}

	bool C_NetRace::BuildPartyPeerJoinSessionData(NetJoinRaceFromPartyData& out, const U64& partyHostConsoleId)
	{
		out.Clear();
		if (!m_Initialised || !m_Running || !m_OnlineMode)
			return false;

		C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(false);
		if (NULL == pHost)
			return false;
		
		NetPlay::SessionSecurity sec;
		if (!GetSecurity(sec))
			return false;
	
		if (JoinGetState() != eJoinStateComplete)
			return false;
		NetRaceSettings settings;
		GetCurrentRaceSettings(settings);

		out.m_Active = true;
		out.m_MaxPlayers			=	settings.m_MaxDrivers;
		out.m_PublicGame			=	settings.m_PublicGame;
		out.m_Ranked				=	settings.m_Ranked;
		out.m_PartyHostConsoleId	=	partyHostConsoleId;
		out.m_RaceHostConsoleId		=	pHost->m_ConsoleId;
		out.m_Security				=	sec;
		out.m_HostName				=	pHost->m_User;
		out.m_EventID				=	settings.m_EventId;
		out.m_CustomRace			=	settings.m_CustomRace;

		for (S32 i=0; i<BD_COMMON_ADDR_SERIALIZED_SIZE; i++)
			out.m_HostTextAddress[i] = pHost->m_CommonAddress[i];

		return true;
	}

	void C_NetRace::PostRace(void)
	{
		if (!m_Initialised || !m_Running)
			return;

		BIZ_ASSERT(eJoinStateComplete == JoinGetState());
		if (eJoinStateComplete != JoinGetState())
			return; 

		U8 numLocalToSend = 0;

		// for all local players update their game profile data
		for (U8 i=0; i<(*m_ppPlayerMgr)->LocalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			BIZ_ASSERT(p->LocalPlayer());
			if (p->LocalPlayer() && p->m_RaceConnection.IsActive())
			{
				BIZ_ASSERT(p->m_RaceConnection.m_JoinId > 0);
				numLocalToSend += 1;		
				Game::C_GameProfileManager::Instance().BuildNetworkSharedProfileData(p->m_ControllerPort, p->m_ProfileData.m_Game);
			}
		}

		if (numLocalToSend)
		{
			C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
			bool ok = msg.CreateReliable();
			if (ok)
			{
				ok = ok && msg.WriteU8(numLocalToSend); 
				
				for (U8 i=0; i<(*m_ppPlayerMgr)->LocalSize(); i++)
				{
					C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
					if (p->LocalPlayer() && p->m_RaceConnection.IsActive())
					{
						ok = ok && msg.WriteU8(p->m_RaceConnection.m_JoinId);
						ok = ok && msg.WriteBuffer(&(p->m_ProfileData), sizeof(p->m_ProfileData));
					}
				}

				msg.TerminateMessage();
			}

			if (ok)
				SendReliableMessage(msg, eNetMessage_PostRace); 
			else
			{
				LOGERROR(g_Log, "C_NetRace::PostRace() - unable to create message.");
			}
		}
	}

	void C_NetRace::SendCommunicatorStatusChangedMessage(void)
	{
		if (!m_Initialised || !m_Running)
			return;

		if (JoinGetState() != eJoinStateComplete)
			return;  

		C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
		msg.Flush();

		const U8 localPlayerCount = (*m_ppPlayerMgr)->LocalSize();

		bool ok = msg.CreateReliable();
		ok = ok && msg.WriteU8(localPlayerCount);
		for (U8 i=0; i<localPlayerCount; i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			BIZ_ASSERT(p->LocalPlayer());

			ok = ok && msg.WriteU8(p->m_RaceConnection.m_JoinId);
			ok = ok && msg.WriteU8(p->m_HasCommunicator == true ? 1 : 0);
		}
		msg.TerminateMessage();

		if (ok)
			SendReliableMessage(msg, eNetMessage_CommunicatorStatusUpdate);
		else
		{
			LOGWARN(g_Log, "C_NetRace::SendCommunicatorStatusChangedMessage() - unable to create communicator status update message.");
		}
	}

	bool C_NetRace::ReceiveCommunicatorStatusChangedMessage(C_NetMessage& msg, const U8 senderId)
	{
		LOGINFO(g_Log, "C_NetRace::ReceiveCommunicatorStatusChangedMessage()");

		bool ok = msg.ReadOpen();
		if (ok)
		{
			U8 plrCount = 0;
			ok = ok && msg.ReadU8(plrCount);
			for (U8 i=0; i<plrCount; i++)
			{
				U8 joinId = 0;
				U8 hasCommunicator = 0;
				ok = ok && msg.ReadU8(joinId);
				ok = ok && msg.ReadU8(hasCommunicator);
				if (ok)
				{
					// store just incase we have yet to enter the lobby. (voice is only enabled once we enter the lobby)
					C_NetPlayer* plr = (*m_ppPlayerMgr)->FindFromJoinId(joinId, false);
					if (plr)
						plr->m_HasCommunicator = (hasCommunicator == 1) ? true : false;
				}
			}
			msg.ReadClose();
		}

		if (!ok)
		{
			LOGWARN(g_Log, "C_NetRace::ReceiveCommunicatorStatusChangedMessage() - malformed data received.");
		}

		return true;
	}

	void C_NetRace::PollDisconnects(const F32 frameTimeInSeconds)
	{
		if (!m_Initialised || !m_Running)
			return;

		C_NetPlayerManager& mgr = *(*m_ppPlayerMgr);
	
		if (mgr.LocalPrimaryJoinId(false) == 0)
			return; // no disconnect logic until contacted host at least.

		// host should never be null since we migrate
		C_NetPlayer* pHost = mgr.HostPlayer(false);

		if (eJoinStateComplete != JoinGetState())
		{
			if (NULL == pHost || pHost->m_ConsoleId != JoinGetHostConsoleId() || eNetPlayerDisconnectionTypeNone != pHost->m_RaceConnection.m_DisconnectRequested)
			{
				// host has left before we have fully joined.
				LOGINFO(g_Log, "C_NetRace::PollDisconnects() - host left before fully connected to all peers.");
				DisconnectAndClear();
				JoinSetFailed(eJoinErrorNoConnectionToHost, *(*m_ppRouter));			
			}
			return;	
		}

		// [WillB] #37608 Clear this to the default. If we are not becoming the host, then CheckForNewHost() will be set m_SessionIsMigrating to true
		if ( !m_Migrating ) // Only the next host will have this flagged. They will reset the variables on succession.
			m_SessionIsMigrating = false;

		// do we need to migrate the host ?
		if (NULL == pHost || !pHost->m_RaceConnection.IsActive() || eNetPlayerDisconnectionTypeNone != pHost->m_RaceConnection.m_DisconnectRequested)
		{
			CheckForNewHost();
			pHost = mgr.HostPlayer(false); // get new host player
		}

		// Need to work out how many players we have timed out to
		S32 playersLostCount = 0;
		S32 originalPlayersCount = mgr.TotalSize();
		
		if (m_Migrating)
		{
			if (PollMatchingServerMigration(frameTimeInSeconds))
				return; // no disconnection logic while matching server is being migrated

			// for offline sessions migration is complete after a new host is chosen.
			m_Migrating = false;
			m_SessionIsMigrating = false;
			m_MatchingMigrationState = eMigrationStateNotActive;

			// Check to see if we have disconnected from everybody in the lobby and its more than one peer as this is quite normal.
			for (U8 i=0; i<mgr.TotalSize(); i++)
			{
				C_NetPlayer* p = mgr.Index(i);
				if (p->LocalPlayer() || !p->PrimaryPlayer())
					continue;		

				if (p->m_RaceConnection.IsActive() && eNetPlayerDisconnectionTypeNone != p->m_RaceConnection.m_DisconnectRequested)
					playersLostCount++;
			}
			LOGINFO(g_Log, "C_NetRace::PollDisconnects() - migration finished, %d players lost", playersLostCount );

			// Inform UI that we are about to become billy nomates
			if ( mgr.TotalSize() - playersLostCount == 1 )
			{
				LOGINFO(g_Log, "C_NetRace::PollDisconnects() - lost connection to everybody in the lobby after migration");
				_UI_MESSAGE( UI::E_UiGameMsg_LobbyConnectionLost );
			}
		}

		// connection timed out ?
		for (U8 i=0; i<mgr.TotalSize(); i++)
		{
			C_NetPlayer* p = mgr.Index(i);
			if (p->LocalPlayer() || !p->m_RaceConnection.IsActive() || eNetPlayerDisconnectionTypeNone != p->m_RaceConnection.m_DisconnectRequested)
				continue;
	
			F32 timeOut = NET_PEER_DROPOUT_TIMER;
			if(pHost && pHost->LocalPlayer())
				timeOut = NET_HOST_DROPOUT_TIMER;
			if (p->m_RaceConnection.KeepAliveTimedOut(timeOut))
			{
				LOGINFO(g_Log, "C_NetRace::PollDisconnects() - lost connection to %S", p->m_User.Name());
				m_NewDisconnects = true;
				p->m_RaceConnection.m_DisconnectRequested = eNetPlayerDisconnectionTypeLostConnection;
				
				playersLostCount++;

				// if i also have a party connection to this player then i need to disconnect from that also.
				if (p->m_PartyConnection.IsActive())
					p->m_PartyConnection.m_DisconnectRequested = eNetPlayerDisconnectionTypeLostConnection;
			}
		}
	
		NetConsoleIds toDrop;
		for (U8 i=0; i<mgr.TotalSize(); i++)
		{
			C_NetPlayer* p = mgr.Index(i);
			if (p->LocalPlayer() || !p->PrimaryPlayer())
				continue;		

			if (p->m_RaceConnection.IsActive() && eNetPlayerDisconnectionTypeNone != p->m_RaceConnection.m_DisconnectRequested)
				toDrop.PushBack(p->m_ConsoleId);
		}

		if (toDrop.Size() == 0)
			m_DisconnectResendMsgTimer = 0.0f;
		else
		{
			if (pHost && pHost->LocalPlayer())
			{
				m_DisconnectResendMsgTimer = 0.0f;

				C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
				bool ok = msg.CreateReliable();
				ok = ok && msg.WriteU32(toDrop.Size());
				for (S32 i=0; i<toDrop.Size(); i++)
				{
					C_NetPlayer* pd = (*m_ppPlayerMgr)->FindFromConsoleId(toDrop[i], 0);
					BIZ_ASSERT(pd != NULL);
					ok = ok && msg.WriteU64(toDrop[i]); // console
					ok = ok && msg.WriteU8(pd->m_RaceConnection.m_DisconnectRequested); // disconnect reason
				}
				msg.TerminateMessage();

				if (ok)
				{
					// host sends mesasge to all peers who are connetced and not flagged to disconnect
					SendReliableMessage(msg, eNetMessage_DisconnectConsoles);

					// actually destroy players connections.
					for (S32 i=0; i<toDrop.Size(); i++)
						DestroyConsole(toDrop[i]);
				}
				else
				{
					LOGERROR(g_Log, "C_NetRace::PollDisconnects() - unable to create disconnect consoles message. Hosting.");
				}
			}
			else
			{
				m_DisconnectResendMsgTimer += frameTimeInSeconds;
				if (m_DisconnectResendMsgTimer >= NET_DISCONNECT_RESEND_TIMER || m_NewDisconnects)
				{
					m_DisconnectResendMsgTimer = (m_DisconnectResendMsgTimer - NET_DISCONNECT_RESEND_TIMER);

					C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
					bool ok = msg.CreateReliable();
					ok = ok && msg.WriteU32(toDrop.Size());
					for (S32 i=0; i<toDrop.Size(); i++)
					{
						C_NetPlayer* pd = (*m_ppPlayerMgr)->FindFromConsoleId(toDrop[i], 0);
						BIZ_ASSERT(pd != NULL);
						ok = ok && msg.WriteU64(toDrop[i]);		// console 
						ok = ok && msg.WriteU8(pd->m_RaceConnection.m_DisconnectRequested); // disconnect reason
					}
					msg.TerminateMessage();

					if (ok)
					{
						// peer sends a request to the current host.
						if (pHost && eNetPlayerDisconnectionTypeNone == pHost->m_RaceConnection.m_DisconnectRequested)
							pHost->SendReliable(msg, eNetMessage_DisconnectConsoles, false, (*m_ppPlayerMgr)->LocalPrimaryJoinId(false));
					}
					else
					{
						LOGERROR(g_Log, "C_NetRace::PollDisconnects() - unable to create disconnect consoles message. Peer.");
					}
				}
			}
		}

		// Check to see if we have disconnected from everybody in the lobby and its more than one peer as this is quite normal.
		if ( toDrop.Size() && ( toDrop.Size() == playersLostCount ) && ( originalPlayersCount - playersLostCount == 1 ) )
		{
			LOGINFO(g_Log, "C_NetRace::PollDisconnects() - lost connection to everybody in the lobby. %d players dropped", playersLostCount );
			_UI_MESSAGE( UI::E_UiGameMsg_LobbyConnectionLost );
		}
		m_NewDisconnects = false;
	}

	void C_NetRace::PollJoin(void)
	{
		if (!m_Initialised || !m_Running)
			return;

		if (!IsJoining())
			return;

		// time join out if it looks like we can't connect to either the host or peers..
		float join_timeOut = ((*m_ppRouter)->PeerRoutingEnabled() == true) ? NET_MAX_JOIN_TIME_WITH_PEER_ROUTING : NET_MAX_JOIN_TIME;
		if (JoinTimedOut(join_timeOut))
		{
			LOGINFO(g_Log, "C_NetRace::PollJoin() - unable to connect to game.");
			const eNetJoinSessionErrors errorCode = (JoinGetState() == eJoinStateWaitingForHostReply) ? eJoinErrorNoConnectionToHost : eJoinErrorNoCommunications;
		
			bool isOnline = m_OnlineMode;

			// if we failed to connect to a peer then try and find the first one..
			bool peerJoinFailuireFound = false;
			SysProfile::BaseProfile peerName;
			if (eJoinErrorNoCommunications == errorCode)
			{
				for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize() && !peerJoinFailuireFound; i++)
				{
					C_NetPlayer* plr = (*m_ppPlayerMgr)->Index(i);
					if (NULL == plr || plr->LocalPlayer() || plr->m_RaceConnection.IsActive())
						continue;

					// connection failed tio this player..
					peerJoinFailuireFound = true;
					peerName = plr->m_User;
				}
			}
			
			DisconnectAndClear();
			JoinSetFailed(errorCode, *(*m_ppRouter));

			if (isOnline)
			{
				if (eJoinErrorNoConnectionToHost == errorCode)
				{
					// host name wil lalready be valid in the join failure because we set it when we originally joined
					JoinSetPeerJoinFailureNameSetValid(true);
				}
				else if (eJoinErrorNoCommunications == errorCode && peerJoinFailuireFound)
				{
					// set the name of the peer who we failed to connect to and flag it so the ui renders it.
					JoinSetPeerJoinFailureName(peerName);
					JoinSetPeerJoinFailureNameSetValid(true);
				}
			}
			return;
		}

		if (JoinGetState() == eJoinStateWaitingForHostReply)
		{
			if (JoinHostConnectionComplete())
			{
				if (JoinReSendRequired(1.5f))
				{
					bdAddrHandleRef hostAddress = JoinGetHostAddress();
					if (hostAddress.notNull())
					{
						C_NetMessage& msg = (*m_ppRouter)->GetOutgoingUnReliable();
						msg.Flush();
						if (BuildJoinRequestMessage(msg, NULL)) // no host player is defined yet.
							(*m_ppRouter)->SendMessage(msg, hostAddress);
					}
				}
			}
		}
		else if (JoinGetState() == eJoinStateWaitingForPeerData)
		{
		}
		else if (JoinGetState() == eJoinStateConnectingPeers)
		{
			 // once all handshaking is complete we are done..
			if (!(*m_ppPlayerMgr)->AnyPlayersHandshaking(false))
			{
				// put local players into new join complete state..
				for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
				{
					C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
					if (!p->LocalPlayer() || !p->m_RaceConnection.IsActive())
						continue;
					p->m_RaceConnection.m_PlayerState = eNetPlayerStateJoinComplete;
				}
				JoinSetFinished();
				SendInitialDataMessage(NULL);
			}
			else
			{
				C_NetMessage& msg = (*m_ppRouter)->GetOutgoingUnReliable();
				if (JoinReSendRequired(1.5f))
				{
					// need to send join request to peers again.. 
					for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
					{
						C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
						if (p->LocalPlayer())
							continue;
						if (p->PrimaryPlayer() && p->m_RaceConnection.IsActive())
						{
							if (p->m_RaceConnection.IsConnected() && !p->m_RaceConnection.m_HandshakingComplete)
							{
								msg.Flush();
								if (BuildJoinRequestMessage(msg, p)) 
									(*m_ppRouter)->SendMessage(msg, p->m_RaceConnection.m_Address);
							}
						}
					}
				}
			}
		}
	}

	void C_NetRace::PollSecurityKeyTracker(void)
	{
		if (!m_Initialised || !m_Running)
			return;
		SecurityKeyIds activeKeyIds;
		C_NetPlayerManager& mgr = **m_ppPlayerMgr;

		for (U8 i=0; i<mgr.TotalSize(); i++)
		{
			const C_NetPlayer* pPlayer = mgr.Index(i);
			if (pPlayer->LocalPlayer() || !pPlayer->PrimaryPlayer())
				continue; // dont check against local players or non remote primary players.

			if (pPlayer->m_RaceConnection.m_Address.isNull())
				continue; // player is not connected on the race channel.

			bool alreadyAdded = false;
			for (S32 z=0; !alreadyAdded && z<activeKeyIds.Size(); z++)
			{
				if (activeKeyIds[z] == pPlayer->m_RaceConnection.m_Address->m_endpoint.getSecID())
					alreadyAdded = true;
			}

			if (!alreadyAdded)
				activeKeyIds.PushBack(pPlayer->m_RaceConnection.m_Address->m_endpoint.getSecID());
		}

		m_pSessionKeyTracker->Poll(activeKeyIds);
	}

	void C_NetRace::PollVoice(void)
	{
		if (!m_Initialised || !m_Running || !m_VoiceEnabled)
			return;

		if (eVoiceControlStateRaceConnection != (*m_ppPlayerMgr)->GetVoiceControlState())
			return; // some other system is currently controlling the voice.

		Communicator::C_Core& voiceMgr = Communicator::C_Core::Instance();

		// build a list of remote consoles, voice data is only ever encoded for the primary remote player 
		// even if their is multiple players on that remote console
		NetConsoleIds remoteConsoles;
		for (S32 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* rp = (*m_ppPlayerMgr)->Index(i);
			if (!rp->LocalPlayer())
			{
				// this will be rebuilt below.
				rp->m_VoiceEncodeArray.ClearBits();

				if (rp->PrimaryPlayer() && rp->m_RaceConnection.IsActive() && eNetPlayerDisconnectionTypeNone == rp->m_RaceConnection.m_DisconnectRequested)
				{
					if(rp->m_RaceConnection.m_PlayerState >= eNetPlayerStateIdle && !rp->IsInRace())
						remoteConsoles.PushBack(rp->m_ConsoleId);
				}
			}
		}

		// which local players should be encoding voice.
		for (S32 localIndex=0; localIndex<(*m_ppPlayerMgr)->LocalSize(); localIndex++)
		{
			C_NetPlayer* lp = (*m_ppPlayerMgr)->Index(localIndex);
			BIZ_ASSERT(lp->LocalPlayer());
			if (voiceMgr.IsLocalTalkerRegistered(lp->m_ControllerPort))
			{
				// local player has voice capability, which remote players should we send voice to ?
				for (S32 remote=0; remote<remoteConsoles.Size(); remote++)
				{
					// check all players on this remote console, no point sending voice if thet all this local player muted.  
					bool anyRemoteValid = false;
					for (S32 z=0; z<(*m_ppPlayerMgr)->TotalSize(); z++)
					{
						C_NetPlayer* rp = (*m_ppPlayerMgr)->Index(z);
						if (remoteConsoles[remote] == rp->m_ConsoleId)
						{
							if (voiceMgr.IsRemoteTalkerRegistered(rp->m_User))
							{
								if (!voiceMgr.LocalMuted(lp->m_ControllerPort, rp->m_User) && !voiceMgr.RemoteMuted(lp->m_ControllerPort, rp->m_User))
									anyRemoteValid = true; // this local player should send voice to this console.
							}
						}
					}
		
					if (anyRemoteValid)
					{
						C_NetPlayer* rp = (*m_ppPlayerMgr)->FindFromConsoleId(remoteConsoles[remote], 0);
						BIZ_ASSERT(rp != NULL);
						BIZ_ASSERT(!rp->LocalPlayer());
						rp->m_VoiceEncodeArray.SetBit(localIndex);
					}
				}
			}
		}

		if ((*m_ppPlayerMgr)->VoiceTransferRequired(NET_DEFAULT_VOICE_SEND_TIME_IN_SECONDS))
			(*m_ppPlayerMgr)->SendBasicVoiceMessage((*m_ppRouter)->GetOutgoingUnReliable(), *(*m_ppRouter));
	}

	void C_NetRace::PollTeamBalancing()
	{
		// TODO_MP_TEAM

		// Check we are the host
		C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer( false );
		if ( pHost == NULL || !pHost->LocalPlayer())
			return;

		NetRaceSettings settings;
		GetCurrentRaceSettings(settings);

		// Do we need to rebalance the teams and are we in the lobby
		if ( !m_BalanceTeams || !settings.m_Teams || !PlayersInLobby() )
			return;

		LOGINFO( g_Log, "NetPlay::PollTeamBalancing() - Balancing teams" );

		// Build a list of all the unique ids
		Btl::Vector< NetPlay::Nonce, Btl::VectorParameters_Static<NET_MAX_VEHICLES> > ids;
		for ( S32 i = 0; i < (*m_ppPlayerMgr)->TotalSize(); i++ )
		{
			C_NetPlayer* pPlayer = (*m_ppPlayerMgr)->Index( i );

			// Ignore remote players are not fully connected
			if ( !pPlayer->LocalPlayer() && ( !pPlayer->m_RaceConnection.IsActive() || !pPlayer->m_RaceConnection.IsConnected() ) )
				continue;

			LOGINFO( g_Log, "NetPlay::PollTeamBalancing() - Player %S party id %d %d %d %d %d %d %d %d", pPlayer->m_User.m_Name, pPlayer->m_PlayerNonce.m_Nonce[0], pPlayer->m_PlayerNonce.m_Nonce[1],
																										 pPlayer->m_PlayerNonce.m_Nonce[2], pPlayer->m_PlayerNonce.m_Nonce[3],
																										 pPlayer->m_PlayerNonce.m_Nonce[4], pPlayer->m_PlayerNonce.m_Nonce[5],
																										 pPlayer->m_PlayerNonce.m_Nonce[6], pPlayer->m_PlayerNonce.m_Nonce[7] );

			// Have we already added this id
			bool found = false;
			for ( S32 j = 0; j < ids.Size(); j++ )
			{
				if ( pPlayer->m_PlayerNonce == ids[j] )
				{
					found = true;
					break;
				}
			}
			if ( !found )
				ids.PushBack( pPlayer->m_PlayerNonce );
		}

		// Bucket together players with the same ids
		C_NetTeamBalancer::NetBucketList teamBuckets;
		for ( S32 i = 0; i < ids.Size(); i++ )
		{
			C_NetTeamBalancer::NetBucket bucket;
			for ( S32 j = 0; j < (*m_ppPlayerMgr)->TotalSize(); j++ )
			{
				C_NetPlayer* pPlayer = (*m_ppPlayerMgr)->Index( j );

				// Ignore remote players are not fully connected
				if ( !pPlayer->LocalPlayer() && ( !pPlayer->m_RaceConnection.IsActive() || !pPlayer->m_RaceConnection.IsConnected() ) )
					continue;

				if ( pPlayer->m_PlayerNonce == ids[i] )
					bucket.PushBack( pPlayer );
			}
			BIZ_ASSERT( bucket.Size() );
			teamBuckets.PushBack( bucket );
		}
		BIZ_ASSERT( teamBuckets.Size() );

		// Assign party groups to teams
		m_TeamBalancer.AssignTeams( teamBuckets, settings.m_MaxDrivers / 2, C_MultiplayerData::Instance().m_pConfig->m_BalancingRange );
		m_BalanceTeams = false;
	}

	bool C_NetRace::PlayersInLobby(void)
	{
		// Check that every valid player is in the lobby
		for ( S32 i = 0; i < (*m_ppPlayerMgr)->TotalSize(); i++ )
		{
			C_NetPlayer* pPlayer = (*m_ppPlayerMgr)->Index( i );

			// Ignore remote players are not fully connected
			if ( !pPlayer->LocalPlayer() && ( !pPlayer->m_RaceConnection.IsActive() || !pPlayer->m_RaceConnection.IsConnected() ) )
				continue;

			// Is anybody not in the lobby? Excluding players connecting
			if ( pPlayer->m_RaceConnection.m_PlayerState >= RacerNet::eNetPlayerStateLoadingRace )
				return false;
		}
		return true;
	}

	// creates local players as network playres
	bool C_NetRace::CreateLocalPlayers(C_NetCreationHelper& helper, const bool hostingSession)
	{
		const bool createFromParty = (*m_ppPlayerMgr)->Size(true) > 0 ? true : false;

		if (helper.m_LocalPlayerData.Size() == 0)
			return false; // no local players defined so can't create.

		// must have already checked for complete connection.
		bdNetImpl *const net = bdNet::getInstance();
		bdCommonAddrRef commonAddress = net->getLocalCommonAddr();
		if (commonAddress.isNull())
			return false;

		for (S32 i=0; i<helper.m_LocalPlayerData.Size(); i++)
		{
			C_NetPlayer* newPlayer = NULL; 
			
			// Perserve data in player manager
			if ( createFromParty )
			{
				newPlayer = (*m_ppPlayerMgr)->Index(i);
			}
			else
				newPlayer = (*m_ppPlayerMgr)->AllocatePlayer(false);

			if (NULL == newPlayer)
				return false;

			// generic  (safe to overwrite data since the creation helper SHOULD contain the same data as when we created the party (if it's running)
			newPlayer->m_User										= helper.m_LocalPlayerData[i].m_SystemProfile;
			newPlayer->m_ControllerPort								= helper.m_LocalPlayerData[i].m_SystemProfile.m_UserIndex;
			newPlayer->m_VoiceBanned								= helper.m_LocalPlayerData[i].m_VoiceBanned;
			newPlayer->m_ConsoleId									= helper.m_ConsoleId;
			newPlayer->m_LocalIndex									= i;
			newPlayer->m_UpStreamKbps								= helper.m_UpStreamBandwidthKbps;
			newPlayer->m_DownStreamKbps								= helper.m_DownStreamBandwidthKbps;
			newPlayer->m_DlcMask									= helper.m_DlcMask;
			newPlayer->m_ProfileData								= helper.m_LocalPlayerData[i].m_ProfileData;

			// serialise common address.
			commonAddress->serialize(newPlayer->m_CommonAddress);

			// race connection specific.
			if (hostingSession)
			{
				newPlayer->m_RaceConnection.m_PlayerState = eNetPlayerStateJoinComplete;
				newPlayer->m_RaceConnection.m_Host = (i == 0) ? true : false;
				newPlayer->m_RaceConnection.m_JoinId = (*m_ppPlayerMgr)->GenerateJoinReference(false);
			}
			else
			{
				newPlayer->m_RaceConnection.m_PlayerState = eNetPlayerStateJoining;
			}

			LOGINFO(g_Log, "%S : Nat:%d", newPlayer->m_User.Name(), newPlayer->m_ProfileData.m_NatType);

			// register local voice talkers.
			if (!newPlayer->m_VoiceBanned)
			{
				if (!Communicator::C_Core::Instance().IsLocalTalkerRegistered(newPlayer->m_ControllerPort))
				{
					if (Communicator::C_Core::Instance().RegisterLocalTalker(helper.m_LocalPlayerData[i].m_SystemProfile))
						Communicator::C_Core::Instance().SetLocalMode(newPlayer->m_ControllerPort, Communicator::C_Core::eModeNormal);
				}

				newPlayer->m_HasCommunicator = Communicator::C_Core::Instance().CommunicatorConnected(newPlayer->m_ControllerPort);
			}
			else
				newPlayer->m_HasCommunicator = false;
		}

		// if online then force an modify on the matching session to set the parameters to the latest.
		// will only occur when fully joined.
		m_ForceMatchingServerModify = m_OnlineMode ? true : false;

		return true;
	}

	// advertises the game on the lan.
	void C_NetRace::AdvertiseLanGame(void)
	{
		if (!m_Initialised || !m_Running || m_OnlineMode || m_Migrating)
			return;

		C_NetPlayer* hPlayer = (*m_ppPlayerMgr)->HostPlayer(false);
		if (NULL == hPlayer || !hPlayer->LocalPlayer() || hPlayer->m_ConsoleId == 0)
			return; // only host should advertise game.	

		// stop advetising sesion if we are racing in a championship..
		if (!CheckIfChampionshipJoinsAllowed())
		{
			m_pLanDiscoveryMgr->UnAdvertise();
			return;
		}

		NetRaceSettings settings;
		GetCurrentRaceSettings(settings);
		U32 openSlotCount = settings.m_MaxDrivers - (*m_ppPlayerMgr)->Size(false);
		BIZ_ASSERT(openSlotCount < NET_MAX_VEHICLES);
		BIZ_ASSERT(openSlotCount < settings.m_MaxDrivers);

		NetRaceSettingsQos raceQos;
		if (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettingsQos(raceQos))
		{
			BIZ_ASSERT(false);
			return;
		}

		m_pLanDiscoveryMgr->Advertise(hPlayer->m_User, *m_pLanSecurity, hPlayer->m_CommonAddress, settings.m_MaxDrivers, openSlotCount, hPlayer->m_ConsoleId, raceQos); // TODO_MP_SYSLINK
	}

	bool C_NetRace::GetSecurity(NetPlay::SessionSecurity& security)
	{
		BIZ_ASSERT(m_Running);
		if (!m_OnlineMode)
		{
			security = *m_pLanSecurity;
			return true;
		}
		else
		{
			return m_pSessionKeyTracker->Current(security);
		}

		return false;
	}

	void C_NetRace::GetCurrentRaceSettings(NetRaceSettings& current)
	{
		BIZ_ASSERT(JoinGetState() == eJoinStateComplete);
		if (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings(current))
		{
			LOGWARN(g_Log, "C_NetRace:GetCurrentRaceSettings() - unable to get race settings from game mode controller.");
		}
	}

	void C_NetRace::SetCurrentRaceSettings(NetRaceSettings& current) 
	{
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	
		pLayer->GetGameModePtr()->SetNetRaceSettings(current);
	}

	void C_NetRace::DestroyConsole(const U64& consoleId)
	{
		NetJoinReferencesSingleConsole toDestroy;
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (p->m_RaceConnection.IsActive() && consoleId == p->m_ConsoleId)
				toDestroy.PushBack(p->m_RaceConnection.m_JoinId);
		}

		for (S32 i=0; i<toDestroy.Size(); i++)
			DestroyPlayer(toDestroy[i], true);
	}

	void C_NetRace::DestroyAllPlayers(void)
	{
		NetJoinReferences toDestroy;
	
		// destroy remote players first
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (!p->LocalPlayer() && p->m_RaceConnection.IsActive())
				toDestroy.PushBack(p->m_RaceConnection.m_JoinId);
		}

		// destroy local players last
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (p->LocalPlayer() && p->m_RaceConnection.IsActive())
				toDestroy.PushBack(p->m_RaceConnection.m_JoinId);
		}
	
		for (S32 i=0; i<toDestroy.Size(); i++)
			DestroyPlayer(toDestroy[i]);
	}

	void C_NetRace::DestroyPlayer(const U8 joinId, const bool showNotification)
	{
		C_NetPlayer* p = (*m_ppPlayerMgr)->FindFromJoinId(joinId, false);
		if (NULL == p)
			return;

		// if online mode then we will need to update the matching session data.
		m_ForceMatchingServerModify = m_OnlineMode ? true : false;
		m_BalanceTeams = true;

		// do we need to inform race logic that player has left ?
		if (NULL != m_pRaceModeListener && !p->LocalPlayer() && p->m_RaceData.m_InRaceVehicleIndex != -1)
			m_pRaceModeListener->VehicleLeftRace((U8)p->m_RaceData.m_InRaceVehicleIndex);

		// if applicable remove player from the matching session.
		if (p->m_RaceConnection.m_AddedToOnlineSession)
			RemoveMatchingPlayer(p->m_User);

		// only give a notification if remote player and we have called EnterLobby()
		if (!p->LocalPlayer() && m_VoiceEnabled)
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->PlayerNotification(p->m_User.Name(), false, false);

		// disconenct from race session.
		// only remote primary players need their address unregistering.
		p->m_RaceConnection.Disconnect(!p->LocalPlayer() && p->PrimaryPlayer());

		// clear race data
		p->m_RaceData.Flush();

		// if no party connection active totally destroy this network player.
		// otherise leave the players party connection active for the party manager to delete.
		if (!p->m_PartyConnection.IsActive())
		{
			// unregister voice since we have no party..
			if (p->LocalPlayer())
			{
				Communicator::C_Core::Instance().UnRegisterLocalTalker(p->m_ControllerPort);
				
				// leaving race with no party means voice mode goes to default.
				if (p->PrimaryPlayer())
					(*m_ppPlayerMgr)->SetVoiceControlState(eVoiceControlStateNone);
			}
			else
			{
				LOGINFO(g_Log, "C_NetRace::DestroyPlayer : '%S' ", p->m_User.Name());
				Communicator::C_Core::Instance().UnRegisterRemoteTalker(p->m_User);
			}

			(*m_ppPlayerMgr)->DeallocatePlayer(p);
		}
		else
		{
			// put voice mode back to party.
			if (p->LocalPlayer() && p->PrimaryPlayer())
				(*m_ppPlayerMgr)->SetVoiceControlState(eVoiceControlStatePartyConnection);

			// remove any race channel reliable messages from this mesasge queue since they can no longer be sent or acknowledged.
			// this will allow any messages queued up on the party channel to be sent correctly.
			p->RemovePendingReliableMessages(false);

			// clear quick mute info for this player since we are going back to the party lobby, and you can't quick mute from there.
			p->m_QuickMute = 0;
		}
	}

	bool C_NetRace::BuildJoinRequestMessage(C_NetMessage& msg, C_NetPlayer* pRecipient)
	{
		if (!m_Running)
			return false;

		const U8 localPlayerCount = (*m_ppPlayerMgr)->LocalSize();
		if (0 == localPlayerCount)
			return false;

		C_NetPlayer* pPrimaryLocalPlayer = (*m_ppPlayerMgr)->Index(0);
		BIZ_ASSERT(pPrimaryLocalPlayer != NULL);
		BIZ_ASSERT(pPrimaryLocalPlayer->LocalPlayer());

		// create unreliable join request
		msg.CreateUnReliable(eNetMessage_JoinRequest, (*m_ppPlayerMgr)->LocalPrimaryJoinId(false), false);

		// pack network transport version.
		bool ok = msg.WriteU32(NET_PACKET_VERSION);

		// pack join id of the recipient who we will be sending this join request to.
		// we need this for the following fringe case. 
		// 2 players join at the same time, because of latency player 2 starts peer negotiations
		// before player 1 has even received the initial join acknowledgement from the host. 
		// for player 1 to accept the join request of player 2 he will need his own join id 
		// (which would normaly come in the join reply from the host).
		ok = ok && msg.WriteU8(pRecipient != NULL ? pRecipient->m_RaceConnection.m_JoinId : 0);

		ok = ok && msg.WriteU64(JoinGetHostConsoleId());
		ok = ok && msg.WriteU64(pPrimaryLocalPlayer->m_ConsoleId);
		ok = ok && msg.WriteBuffer(pPrimaryLocalPlayer->m_CommonAddress, BD_COMMON_ADDR_SERIALIZED_SIZE);
		ok = ok && msg.WriteU16(pPrimaryLocalPlayer->m_UpStreamKbps);
		ok = ok && msg.WriteU16(pPrimaryLocalPlayer->m_DownStreamKbps);
		ok = ok && msg.WriteU32(pPrimaryLocalPlayer->m_DlcMask);
		ok = ok && msg.WriteU8(localPlayerCount);

		// for each local player pack the required data.
		U8 added = 0;
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (!p->LocalPlayer())
				continue;

			added += 1; 

			// generic player data
			ok = ok && msg.WriteBuffer(&(p->m_User), sizeof(p->m_User));	// pack user data
			ok = ok && msg.WriteU8(p->m_VoiceBanned ? 1 : 0);
			ok = ok && msg.WriteBuffer(&(p->m_ProfileData), sizeof(p->m_ProfileData));

			// specific race connection data.
			ok = ok && msg.WriteU8(p->m_RaceConnection.m_JoinId); // pack join id.
		}

		// sanity check.
		if( ok)
		{
			BIZ_ASSERT(added == localPlayerCount);
			if (added != localPlayerCount)
				ok = false;
		}

		msg.TerminateMessage();
		return ok;
	}

	bool C_NetRace::ReceiveJoinDataMessage(C_NetMessage& msg, const U8 senderId)
	{
		if (!m_Running)
			return false;
		BIZ_ASSERT(JoinGetState() == eJoinStateWaitingForPeerData);

		bool ok = msg.ReadOpen();
		
		NetRaceSettings raceSettings;
		ok == ok && msg.ReadBuffer(&raceSettings, sizeof(raceSettings));
		if (ok)
			SetCurrentRaceSettings(raceSettings);

		U8 numPeersToJoin = 0;
		ok = ok && msg.ReadU8(numPeersToJoin);

		for (U8 i=0; i<numPeersToJoin; i++)
		{
			// create peers who we need to connect to before we can be allowed into the session.
			U64 consoleId = 0;
			U8 localIndex = 0;
			U8 commonAddr[BD_COMMON_ADDR_SERIALIZED_SIZE];
			U16 upBandwidth = 0;
			U16 downBandwidth = 0;
			U32 dlcMask = 0;
			U8 voiceBanned = 0;
			NetPlayerProfileGameData profile;
			SysProfile::BaseProfile bs;
			U8 joinId = 0;
			U8 state = 0;
			U8 team = 0;

			ok = ok && msg.ReadU64(consoleId);
			ok = ok && msg.ReadU8(localIndex);
			ok = ok && msg.ReadBuffer(commonAddr, BD_COMMON_ADDR_SERIALIZED_SIZE);
			ok = ok && msg.ReadU16(upBandwidth);
			ok = ok && msg.ReadU16(downBandwidth);
			ok = ok && msg.ReadU32(dlcMask);
			ok = ok && msg.ReadU8(voiceBanned);
			ok = ok && msg.ReadBuffer(&profile, sizeof(profile));
			ok = ok && msg.ReadBuffer(&(bs), sizeof(bs));	
			ok = ok && msg.ReadU8(joinId);	
			ok = ok && msg.ReadU8(state);	
			ok = ok && msg.ReadU8(team);

			bdNetImpl *const net = bdNet::getInstance();
			bdCommonAddrRef myCommonAddress = net->getLocalCommonAddr();
			if (myCommonAddress.isNull())
				ok = false;

			if (ok)
			{
				C_NetPlayer *p = (*m_ppPlayerMgr)->FindFromConsoleId(consoleId, localIndex);
				if (NULL == p)
					p = (*m_ppPlayerMgr)->AllocatePlayer(true);

				// even if player exists in party, they should not have a race connection active yet.
				if (p && !p->m_RaceConnection.IsActive())
				{
					p->m_User				= bs;
					p->m_ConsoleId			= consoleId;
					p->m_LocalIndex			= localIndex;
					p->m_DlcMask			= dlcMask;
					p->m_VoiceBanned		= voiceBanned != 0 ? true : false;
					p->m_ProfileData		= profile;
					p->m_UpStreamKbps		= upBandwidth;
					p->m_DownStreamKbps 	= downBandwidth;

					p->m_RaceConnection.m_PlayerState = static_cast <eNetPlayerState> (state);
					p->m_RaceConnection.m_JoinId = joinId;
					p->m_RaceData.m_Team = team;

					for (U32 z=0; z<BD_COMMON_ADDR_SERIALIZED_SIZE; z++)
						p->m_CommonAddress[z] = commonAddr[z];	

					// if primary remote player then attempt to start connection process..
					if (p->PrimaryPlayer())
					{
						BIZ_ASSERT(p->m_RaceConnection.m_Address.isNull());
						bdCommonAddrRef addr = new bdCommonAddr();
						if (addr->deserialize(myCommonAddress, p->m_CommonAddress))
						{
							NetPlay::SessionSecurity security;
							ok = GetSecurity(security);

							if (ok)
							{
								if (NULL != net->getSocketRouter() && net->getSocketRouter()->getAddressMap()->getAddrHandle(addr, security.m_Id, p->m_RaceConnection.m_Address))
									ok = net->getSocketRouter()->connect(p->m_RaceConnection.m_Address);
								else 
								{
									ok = false;
									addr = BD_NULL;
								}
							}
						}
						else
						{
							ok = false;
							addr = BD_NULL;
						}
					}

					p->m_RaceConnection.StartKeepAliveTimer();
				}
				else
					ok = false;
			}
		}

		if (!ok)
		{
			DisconnectAndClear();
			JoinSetFailed(eJoinErrorInvalid, *(*m_ppRouter));
		}
		else
		{
			// move join procedure to connecting to any additional peers.
			JoinEnterPeerConnectionPhase();
		}

		msg.ReadClose();
		return true;
	}

	void C_NetRace::SendNewRaceDataMessage( void )
	{
		C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
		msg.CreateReliable();

		NetRaceSettings raceSettings;
		GetCurrentRaceSettings(raceSettings);

		msg.WriteBuffer(&raceSettings, sizeof(raceSettings)); // write the new race settings
		msg.TerminateMessage();

		// attempt to send message to connected peers
		SendReliableMessage(msg, eNetMessage_NewRaceData);
	}

	bool C_NetRace::ReceiveNewRaceDataMessage( C_NetMessage& msg, const U8 senderId )
	{
		if (!m_Running)
			return false;

		// TODO_MP_CUSTOM_RACE
		//BIZ_ASSERT(JoinGetState() == eJoinStateWaitingForPeerData);

		bool ok = msg.ReadOpen();

		NetRaceSettings raceSettings;
		ok == ok && msg.ReadBuffer(&raceSettings, sizeof(raceSettings));
		if (ok)
			SetCurrentRaceSettings(raceSettings);

		//	TODO_MP_CUSTOM_RACE
		// probably need to send a message back to the host that we've received the new settings, or change our state from some "waiting for data" state back to Idle.
		return ok;
	}


	void C_NetRace::SendInitialDataMessage(C_NetPlayer* pRecipient)
	{
		BIZ_ASSERT(JoinGetState() == eJoinStateComplete);

		C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
		bool ok = msg.CreateReliable();
		
		// is this message a reply or a request ?.
		if (pRecipient)
			ok = ok && msg.WriteU8(0); // reply
		else
			ok = ok && msg.WriteU8(1); // request.

		const U8 localSize = (*m_ppPlayerMgr)->LocalSize();
		BIZ_ASSERT(localSize > 0);
		ok = ok && msg.WriteU8(localSize);
		U8 added = 0;
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (!p->LocalPlayer())
				continue;
			BIZ_ASSERT(p->m_RaceConnection.m_JoinId > 0);

			// re check communicator status
			if (!p->m_VoiceBanned)
				p->m_HasCommunicator = Communicator::C_Core::Instance().CommunicatorConnected(p->m_ControllerPort);
		
			ok = ok && msg.WriteU8(p->m_RaceConnection.m_JoinId);	
			ok = ok && msg.WriteU8(p->m_RaceConnection.m_PlayerState);
			ok = ok && msg.WriteBuffer(&(p->m_ProfileData), sizeof(p->m_ProfileData));
			
			NetMuteListJoinReferences muteList;
			(*m_ppPlayerMgr)->BuildMuteList(p->m_LocalIndex, muteList, false);
			ok = ok && msg.WriteBuffer(&muteList, sizeof(muteList));

			ok = ok && msg.WriteU8(p->m_HasCommunicator == true ? 1 : 0);
			
			// Send nonce 
			for ( U16 j = 0; j < p->m_PlayerNonce.Size(); j++ )
				ok = ok && msg.WriteU8( p->m_PlayerNonce.m_Nonce[j] );

			added += 1;
		}

		// sanity check.
		BIZ_ASSERT(added == localSize);

		msg.TerminateMessage();

		if (ok)
		{
			if (NULL == pRecipient)
				SendReliableMessage(msg, eNetMessage_InitialData);
			else
			{
				BIZ_ASSERT(pRecipient->PrimaryPlayer());
				BIZ_ASSERT(pRecipient->m_LocalIndex == 0);
				if (pRecipient->PrimaryPlayer() && pRecipient->m_RaceConnection.CanMessage())
					pRecipient->SendReliable(msg, eNetMessage_InitialData, false, (*m_ppPlayerMgr)->LocalPrimaryJoinId(false));
			}
		}
		else
		{
			LOGERROR(g_Log, "C_NetRace::SendInitialDataMessage() - failed to send initial data message.");
		}
	}

	bool C_NetRace::ReceiveInitialDataMessage(C_NetMessage& msg, const U8 senderId)
	{
		if (!m_Running)
			return false;

		bool ok = msg.ReadOpen();
		if (ok)
		{
			U8 request = 0;
			ok = ok && msg.ReadU8(request);
			if (ok)
			{	
				U8 numPeers = 0;
				ok = ok && msg.ReadU8(numPeers);
				if (ok)
				{
					// register voice talkers if we are in a state where voice is active (ie : we have entered the lobby)
					SetupRemoteVoice();

					for (U8 i=0; i<numPeers; i++)
					{
						U8 joinId, playerState, hasCommunicator;
						NetPlayerProfileGameData profile;
						NetMuteListJoinReferences muteList;
						NetPlay::Nonce nonce;

						ok = ok && msg.ReadU8(joinId);
						ok = ok && msg.ReadU8(playerState);
						ok = ok && msg.ReadBuffer(&profile, sizeof(profile));
						ok = ok && msg.ReadBuffer(&muteList, sizeof(muteList));
						ok = ok && msg.ReadU8(hasCommunicator);

						// Read nonce 
						for ( U16 j = 0; j < nonce.Size(); j++ )
							ok = ok && msg.ReadU8( nonce.m_Nonce[j] );

						if (ok)
						{
							C_NetPlayer* p = (*m_ppPlayerMgr)->FindFromJoinId(joinId, false);
							if (p)
							{
								p->m_RaceConnection.m_PlayerState = static_cast<eNetPlayerState> (playerState);
								p->m_RaceConnection.m_VoiceMuteData = muteList;
								p->m_ProfileData = profile;
								p->m_HasCommunicator = (hasCommunicator == 1) ? true : false;
								(*m_ppPlayerMgr)->ReceiveMuteList(joinId, muteList, false);
								p->m_PlayerNonce = nonce;
							}
						}
					}
				}
			}

			if (ok && request > 0)
			{
				if (JoinGetState() == eJoinStateComplete)
				{
					C_NetPlayer* p = (*m_ppPlayerMgr)->FindFromJoinId(senderId, false);
					if (p)
						SendInitialDataMessage(p); // reply straight back to the sender with our initial data. 
				}
			}
		}

		if (!ok)
		{
			LOGERROR(g_Log, "C_NetRace::ReceiveInitialDataMessage() - failed to read initial data message.");
		}

		msg.ReadClose();
		return true;
	}

	bool C_NetRace::ReceivePlayerStatesMessage(C_NetMessage& msg, const U8 senderId)
	{
		// this simple message can be parsed via the player manager since all the data is going in there anyway.
		(*m_ppPlayerMgr)->ReceivePlayerStatesMessage(msg, senderId, false);

		// check if this player has left the race . if so message race logic to drop the vehicle.
		C_NetPlayer* pSender = (*m_ppPlayerMgr)->FindFromJoinId(senderId, false);
		if (pSender)
		{
			// check all players at the same console id.
			for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
			{
				C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
				if (pSender->m_ConsoleId == p->m_ConsoleId && !p->LocalPlayer() && p->m_RaceConnection.IsActive())
				{
					if (p->m_RaceData.m_InRaceVehicleIndex != -1)
					{
						switch (p->m_RaceConnection.m_PlayerState)
						{
						case eNetPlayerStateLoadingUi:		// player has left the race and gone back to ui.
							if (m_pRaceModeListener)
								m_pRaceModeListener->VehicleLeftRace((U8)p->m_RaceData.m_InRaceVehicleIndex);
							p->m_RaceData.m_InRaceVehicleIndex = -1;
							break;
						}
					}
				}
			}
		}

		return true;
	}

	bool C_NetRace::ReceiveLeavingMessage(C_NetMessage& msg, const U8 senderId)
	{
		U64 consoleId = 0;
		bool ok = msg.ReadOpen();
		ok = ok && msg.ReadU64(consoleId);
		msg.ReadClose();

		if (ok)
		{
			for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
			{
				C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
				if (consoleId == p->m_ConsoleId && p->m_RaceConnection.IsActive())
				{
					if (eNetPlayerDisconnectionTypeNone == p->m_RaceConnection.m_DisconnectRequested)
						p->m_RaceConnection.m_DisconnectRequested = eNetPlayerDisconnectionTypeNormal;	// players are leaving the race the normal way.
				}
			}
		}

		return true;
	}

	bool C_NetRace::ReceiveDisconnectConsolesMessage(C_NetMessage& msg, const U8 senderId)
	{
		C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(false);

		LOGDEBUG(g_Log, "C_NetRace::ReceiveDisconnectConsolesMessage()");

		bool ok = msg.ReadOpen();
		if (ok)
		{
			NetConsoleIds unknownConsoleIds;
			U32 numConsoles = 0;
			ok = ok && msg.ReadU32(numConsoles);
			for (U32 i=0; i<numConsoles; i++)
			{
				U64 console = 0;
				U8 reason = eNetPlayerDisconnectionTypeNone;
				ok = ok && msg.ReadU64(console);
				ok = ok && msg.ReadU8(reason);
				if (ok)
				{
					if (pHost && pHost->LocalPlayer())
					{
						bool found = false;
						// host flags all players at the specified console to disconnect asap. handled from within PollDisconnects()
						for (U8 z=0; z<(*m_ppPlayerMgr)->TotalSize(); z++)
						{
							C_NetPlayer* p = (*m_ppPlayerMgr)->Index(z);
							if (console == p->m_ConsoleId)
							{
								found = true;
								if (p->m_RaceConnection.IsActive())
								{
									p->m_RaceConnection.m_DisconnectRequested = static_cast<eNetPlayerDisconnectionType> (reason);

									// if disconnect reason is lost connection the we can also remove this console from our party session (is applicable)
									// this should maintain sesion consistency.
									if (eNetPlayerDisconnectionTypeLostConnection == p->m_RaceConnection.m_DisconnectRequested)
									{
										if (p->m_PartyConnection.IsActive())
											p->m_PartyConnection.m_DisconnectRequested = eNetPlayerDisconnectionTypeLostConnection;
									}
								}
							}
						}

						if (!found)
							unknownConsoleIds.PushBack(console);
					}
					else
					{
						const eNetPlayerDisconnectionType t = static_cast<eNetPlayerDisconnectionType> (reason);
						
						// if disconnect reason is lost connection the we can also remove this console from our party session (is applicable)
						// this should maintain sesion consistency.
						if (eNetPlayerDisconnectionTypeLostConnection == t)
						{
							for (U8 z=0; z<(*m_ppPlayerMgr)->TotalSize(); z++)
							{
								C_NetPlayer* p = (*m_ppPlayerMgr)->Index(z);
								if (console == p->m_ConsoleId)
								{
									if (p->m_PartyConnection.IsActive())
										p->m_PartyConnection.m_DisconnectRequested = eNetPlayerDisconnectionTypeLostConnection;
								}
							}
						}

						// peers just do as requested and destroy the specified console players.
						DestroyConsole(console);	
					}
				}
			}

			// host has the option of replying to the peer if the peer has requested we disconnect a console that we know nothing about.
			// when the peer receives this reply they will disconnect from that console. maintains session consistency.
			if (unknownConsoleIds.Size() > 0)
			{
				C_NetPlayer* pSender = (*m_ppPlayerMgr)->FindFromJoinId(senderId, false);
				if (pSender)
				{
					C_NetMessage& reply = (*m_ppRouter)->GetOutgoingReliable();
					bool okeydokey = reply.CreateReliable();
					okeydokey = okeydokey && reply.WriteU32(unknownConsoleIds.Size());
					for (S32 i=0; i<unknownConsoleIds.Size(); i++)
					{
						okeydokey = okeydokey && reply.WriteU64(unknownConsoleIds[i]); // console
						okeydokey = okeydokey && reply.WriteU8(eNetPlayerDisconnectionTypeLostConnection); // disconnect reason
					}
					reply.TerminateMessage();

					if (okeydokey)
						pSender->SendReliable(reply, eNetMessage_DisconnectConsoles, false, (*m_ppPlayerMgr)->LocalPrimaryJoinId(false));
					else
					{
						LOGWARN(g_Log, "C_NetRace::ReceiveDisconnectConsolesMessage() - unable to reply to sender as unknown consoles exist.");
					}
				}
			}
		}

		if (!ok)
		{
			LOGWARN(g_Log, "C_NetRace::ReceiveDisconnectConsolesMessage() - malformed data received.");
		}
		return true;
	}

	bool C_NetRace::ReceiveMuteChangeMessage(C_NetMessage& msg, const U8 senderId)
	{
		bool ok = msg.ReadOpen();
		if (ok)
		{
			U8 plrCount = 0;
			ok = ok && msg.ReadU8(plrCount);
			for (U8 i=0; i<plrCount; i++)
			{
				U8 joinId = 0;
				NetMuteListJoinReferences muteList;
				ok = ok && msg.ReadU8(joinId);
				ok = ok && msg.ReadBuffer(&muteList, sizeof(muteList));
				if (ok)
				{
					// store just incase we have yet to enter the lobby. (voice is only enabled once we enter the lobby)
					C_NetPlayer* plr = (*m_ppPlayerMgr)->FindFromJoinId(joinId, false);
					if (plr)
						plr->m_RaceConnection.m_VoiceMuteData = muteList; 

					(*m_ppPlayerMgr)->ReceiveMuteList(joinId, muteList, false);
				}
			}
			msg.ReadClose();
		}

		if (!ok)
		{
			LOGWARN(g_Log, "C_NetRace::ReceiveMuteChangeMessage() - malformed data received.");
		}
		return true;
	}

	void C_NetRace::CheckForNewHost(void)
	{
		NetConsoleIds consoleIds;
		U8 lowestJoinId = 255;	// not possible for a player to get this id
		C_NetPlayerManager& mgr = *(*m_ppPlayerMgr);
		for (U8 i=0; i<mgr.TotalSize(); i++)
		{
			C_NetPlayer* p = mgr.Index(i);
			if (p->m_RaceConnection.IsActive())
			{
				if (p->PrimaryPlayer())
					consoleIds.PushBack(p->m_ConsoleId);

				if (eNetPlayerDisconnectionTypeNone == p->m_RaceConnection.m_DisconnectRequested)
				{
					if (255 == lowestJoinId || p->m_RaceConnection.m_JoinId < lowestJoinId)
						lowestJoinId = p->m_RaceConnection.m_JoinId;
				}
			}
		}

		C_NetPlayer* pNewHost = mgr.FindFromJoinId(lowestJoinId, false);
		if (NULL == pNewHost)
			return;

		m_SessionIsMigrating = true;

		if (!pNewHost->LocalPlayer())
			return; // a remote console will migrate to the new host.
		
		m_MatchingInitialDelayTimer = 0.0f;
		m_Migrating = true;
		m_MatchingMigrationState = eMigrationStateHostInitialDelay;
		
		mgr.PromoteToHost(pNewHost->m_RaceConnection.m_JoinId, false);

		LOGINFO(g_Log, "C_NetRace::CheckForNewHost() - local player is new host");
	}

	void C_NetRace::StartHostMigration(void)
	{
		BIZ_ASSERT(m_Migrating);

		C_NetPlayer* pNewHost = (*m_ppPlayerMgr)->HostPlayer(false);
		BIZ_ASSERT(pNewHost != NULL);
		
		if (pNewHost && pNewHost->LocalPlayer())
		{
			if (eNetPlayerStateDisconnecting == pNewHost->m_RaceConnection.m_PlayerState || pNewHost->m_RaceConnection.m_IgnoreMigration)
				return; // we are disconnecting so no point us becoming the new host. once we leave another peer will get the host privileges.

			LOGINFO(g_Log, "C_NetRace::StartHostMigration()");

			NetConsoleIds consoleIds;
			U8 lowestJoinId = 0;
			C_NetPlayerManager& mgr = *(*m_ppPlayerMgr);
			for (U8 i=0; i<mgr.TotalSize(); i++)
			{
				C_NetPlayer* p = mgr.Index(i);
				if (p->m_RaceConnection.IsActive())
				{
					if (p->PrimaryPlayer())
						consoleIds.PushBack(p->m_ConsoleId);
				}
			}

			// send message to peers so they can promote me to host.
			C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
			bool ok = msg.CreateReliable();

			// new host data
			ok = ok && msg.WriteU64(pNewHost->m_ConsoleId);
			ok = ok && msg.WriteU8(pNewHost->m_LocalIndex);

			// current race settings.
			NetRaceSettings settings;
			GetCurrentRaceSettings(settings);
			ok = ok && msg.WriteBuffer(&settings, sizeof(settings));

			// list of known console ids on the race channel.
			ok = ok && msg.WriteU8((U8)consoleIds.Size());
			for (S32 i=0; i<consoleIds.Size(); i++)
				ok = ok && msg.WriteU64(consoleIds[i]);

			msg.TerminateMessage();

			if (ok)
				SendReliableMessage(msg, eNetMessage_Migrate);
			else
			{
				LOGERROR(g_Log, "C_NetRace::StartHostMigration() - unable to create migrate message.");
			}

			if (m_OnlineMode)
				m_MatchingMigrationState = eMigrationStateHostStart;
			else
			{
				m_Migrating = false;
				m_SessionIsMigrating = false;
			}
		}
		else
		{
			m_Migrating = false;
			m_SessionIsMigrating = false;
			m_MatchingMigrationState = eMigrationStateNotActive;
		}
	}

	bool C_NetRace::ReceiveMigrateMessage(C_NetMessage& msg, const U8 senderId)
	{
		LOGINFO(g_Log, "C_NetRace::ReceiveMigrateMessage()");

		NetConsoleIds activeConsoles;
		NetConsoleIds toDestroy;

		bool ok = msg.ReadOpen();

		U64 newHostConsole = 0;
		U8 newHostIndex = 0;
		ok = ok && msg.ReadU64(newHostConsole);
		ok = ok && msg.ReadU8(newHostIndex);
		if (ok)
		{
			C_NetPlayer *p =(*m_ppPlayerMgr)->FindFromConsoleId(newHostConsole, newHostIndex);
			if (p && p->m_RaceConnection.IsActive())
			{
				(*m_ppPlayerMgr)->PromoteToHost(p->m_RaceConnection.m_JoinId, false);

				NetRaceSettings settings;
				ok = ok && msg.ReadBuffer(&settings, sizeof(settings));
				if (ok)
				{
					SetCurrentRaceSettings(settings);

					U8 consoleCount = 0;
					ok = ok && msg.ReadU8(consoleCount);
					for (U8 i=0; i<consoleCount; i++)
					{
						U64 consoleId = 0;
						ok = ok && msg.ReadU64(consoleId);
						if (ok)
							activeConsoles.PushBack(consoleId);
					}
				}
			}

			// disconnect any consoles that we know about that are not in the list supplied by the new host
			if (ok)
			{
				for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
				{
					C_NetPlayer* pc = (*m_ppPlayerMgr)->Index(i);
					if (pc->LocalPlayer() || !pc->PrimaryPlayer() || !pc->m_RaceConnection.IsActive())
						continue;

					bool found = false;
					for (S32 z=0; z<activeConsoles.Size() && !found; z++)
					{
						if (activeConsoles[z] == pc->m_ConsoleId)
							found = true;
					}

					if (!found)
						toDestroy.PushBack(pc->m_ConsoleId);
				}

				for (S32 z=0; z<toDestroy.Size(); z++)
				{
					// destroy console from race connection.
					DestroyConsole(toDestroy[z]);
				}
			}
		}

		// need to force an update so players may need adding / removing from the matching session if it's running.
		UpdateMatchingServer(false);

		msg.ReadClose();

		if (ok)
		{
			if (m_OnlineMode)
			{
				m_Migrating = true;
				m_SessionIsMigrating = true;
				m_MatchingMigrationTimer = 0.0f;
				m_MatchingMigrationState = eMigrationStatePeerWaitingForHost; // peer waits until host tells him to start matching session migration
			}
		}
		else
		{
			LOGWARN(g_Log, "C_NetRace::ReceiveMigrateMessage() - malformed data received.");
		}

		return true;
	}

	void C_NetRace::SetupRemoteVoice(void)
	{
		//BIZ_ASSERT(m_Running);
		//BIZ_ASSERT(m_VoiceEnabled);

		if (!m_VoiceEnabled || !m_Running)
			return;

		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (p->LocalPlayer() || p->m_VoiceBanned)
				continue;

			if (!Communicator::C_Core::Instance().IsRemoteTalkerRegistered(p->m_User))
			{
				if (Communicator::C_Core::Instance().RegisterRemoteTalker(p->m_User))
				{
					// for all registered local talkers set decoding priority
					for (U8 z=0; z<(*m_ppPlayerMgr)->TotalSize(); z++)
					{
						C_NetPlayer* pl = (*m_ppPlayerMgr)->Index(z);
						if (pl->LocalPlayer() && Communicator::C_Core::Instance().IsLocalTalkerRegistered(pl->m_ControllerPort))
							Communicator::C_Core::Instance().SetDecodingPriority(pl->m_ControllerPort, p->m_User, 0, false);
					}
				}
			}
		}

		Communicator::C_Core::Instance().MuteUpdate();
	}

	void C_NetRace::PollMatchingQoS(void)
	{
		if (!m_Initialised || !m_Running || !m_OnlineMode || m_Migrating)
			return;

		C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(false);
		if (NULL == pHost || !pHost->LocalPlayer())
			return;

		if (JoinGetState() != eJoinStateComplete)
			return;

		NetRaceSettings settings;
		GetCurrentRaceSettings(settings);

		bool forceServerUpdate = false;
		NetQoSRace	tempQos;

		// base settings
		tempQos.m_Version			= NET_PACKET_VERSION;
		tempQos.m_Type				= NetQoSBase::QoSTypeRace;
		tempQos.m_Host				= pHost->m_User;
		tempQos.m_HostConsoleId		= pHost->m_ConsoleId;
		tempQos.m_MaxPlayers		= settings.m_MaxDrivers;
		tempQos.m_NumSLotsStillOpen	= settings.m_MaxDrivers - (*m_ppPlayerMgr)->Size(false);
		tempQos.m_PublicGame		= settings.m_PublicGame; 
		tempQos.m_PackVersion		= S32(		((C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion << 16) & 0xffff0000) 
											|	((C_MultiplayerData::Instance().m_pConfig->m_PackVersion << 0) & 0x0000ffff));

		// generate Qos race settings
		if (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettingsQos(tempQos.m_Race))
		{
			LOGERROR(g_Log, "C_NetRace::PollMatchingQoS() - unable to generate qos race settings");
			return;
		}

		// race only settings.
		tempQos.m_SessionState = (m_InRace == true) ? NetQoSBase::QoSStateInRace : NetQoSBase::QoSStateInLobby;
		tempQos.m_MatchingRegion = (*m_ppRouter)->GetMatchingRegion();

		GenerateDlcSettings(tempQos.m_DlcSettings);

		// has qos data changed in any way.
		if (tempQos != m_QoS)
			forceServerUpdate = true;

		if (forceServerUpdate)
		{
			m_QoS = tempQos;

			LOGINFO(g_Log, "C_NetRace::UpdateMatchingQoS() - updating server with new qos data.");
			const bool ok = (*m_ppRaceMatchmaking)->EnableQos(16384, (U8*)&m_QoS, sizeof(NetQoSRace));
			if (!ok)
			{
				LOGWARN(g_Log, "C_NetRace::UpdateMatchingQoS() - failed to set the QoS data to the matching session");
			}
		}
	}

	void C_NetRace::PollMatchingServer(const bool forceParamChange, const F32 frameTimeinSeconds)
	{
		if (!m_Initialised || !m_Running || !m_OnlineMode)
			return;

		// check matching session for going into an invalid state.
		if (!m_BigError)
		{
			switch ((*m_ppRaceMatchmaking)->SessionState())
			{
			case NetPlay::C_Session::eStateRunning:
			case NetPlay::C_Session::eStatePlaying:
				break;
			default:
				LOGWARN(g_Log, "C_NetRace::PollMatchingServer() - matching session is in invalid state!.");
				m_BigError = true;
				break;
			}
		}

		// we should only update the session once we have fully joined the session if the host leaves before we fully join then the
		// disconnect logic will fail the join attempt.
		if (JoinGetState() == eJoinStateComplete)
		{
			NetRaceSettings settings;
			GetCurrentRaceSettings(settings);

			// do we need to update the server with the latest matching parameters ?
			// this should only happen if we need to change the parameters that are used in the matching queries.
			if (m_ForceMatchingServerModify || forceParamChange)
			{
				m_ForceMatchingServerModify = false;

				// Fudge number of players to so that large parties that cant fit on one team cannot be split
				const U8 numPlayers = ( settings.m_Teams ) ? settings.m_MaxDrivers - m_TeamBalancer.FreeSpace(): (*m_ppPlayerMgr)->Size(false);
				
				// use the game settings to generate matching data
				NetMatchingServerData matchingServerCreationData;
				BuildMatchingServerData(settings, matchingServerCreationData, numPlayers);

				// convert the game matching data into platform specific properties
				NetMatchingParameterList matchingServerParamList;
				BuildMatchingParameters(matchingServerCreationData, matchingServerParamList, true, true);
				BIZ_ASSERT(matchingServerParamList.Size() > 0);

				NetPlay::C_Session::SlotConfig matchSlotConfig;
				matchSlotConfig.m_MaxPublic		= (settings.m_PublicGame != 0) ? settings.m_MaxDrivers: 0;
				matchSlotConfig.m_PublicInUse	= (settings.m_PublicGame != 0) ? numPlayers : 0;
				matchSlotConfig.m_MaxPrivate	= (settings.m_PublicGame == 0) ? settings.m_MaxDrivers: 0;
				matchSlotConfig.m_PrivateInUse	= (settings.m_PublicGame == 0) ? numPlayers : 0;

				U32 modifyFlags = NetPlay::SESSION_CREATE_ONLINE_MULTIPLAYER_STANDARD;
				const U32 currentFlags = (*m_ppRaceMatchmaking)->GetSessionParameters();
				
				if (currentFlags & NetPlay::SESSION_CREATE_HOST)
					modifyFlags |= NetPlay::SESSION_CREATE_HOST;

				if (!CheckIfChampionshipJoinsAllowed())
				{
					// add these settings flags when we wish the guide to stop working.. only usefull on xbox360.
					modifyFlags |= NetPlay::SESSION_CREATE_INVITES_DISABLED;
					modifyFlags |= NetPlay::SESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED;
					modifyFlags |= NetPlay::SESSION_CREATE_JOIN_IN_PROGRESS_DISABLED;
				}

				bool ok = (*m_ppRaceMatchmaking)->Modify(modifyFlags, matchSlotConfig, &matchingServerParamList[0], matchingServerParamList.Size());
				if (ok)
				{
					LOGINFO(g_Log, "C_NetRace::PollMatchingServer() - matching server parameters modified.");
				}
				else
				{
					LOGWARN(g_Log, "C_NetRace::PollMatchingServer() - unable to modify matching server parameters.");
				}
			}

			NetPlay::C_Session::SessionUserList serverList;

			// remove player first.
			for (S32 i=0; i<m_MatchingToRemove.Size(); i++)
			{
				NetPlay::C_Session::SessionUser user;
				user.m_User = m_MatchingToRemove[i];
				user.m_PublicSlot = settings.m_PublicGame != 0 ? true : false;
				serverList.PushBack(user);

				if (serverList.Capacity() == serverList.Size() || serverList.Size() == m_MatchingToRemove.Size())
				{
					bool ok = (*m_ppRaceMatchmaking)->RemovePlayers(serverList);
					if (ok)
					{
						LOGINFO(g_Log, "C_NetRace::PollMatchingServer() - removed %d player(s) from matching session.", serverList.Size());
					}
					else
					{
						m_BigError = true;
						LOGWARN(g_Log, "C_NetRace::PollMatchingServer() - unable to remove %d player(s) from matching session.", serverList.Size());
					}
					serverList.Clear();
				}
			}
			m_MatchingToRemove.Clear();
			serverList.Clear();

			// fully joined so see what players need adding to the session. 
			// only players who have completed the join process will be added.
			MatchingPlayersRaceList addList;
			for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
			{
				C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
				if (p->m_RaceConnection.IsActive() && p->m_RaceConnection.m_PlayerState >= eNetPlayerStateJoinComplete && !p->m_RaceConnection.m_AddedToOnlineSession)
				{
					p->m_RaceConnection.m_AddedToOnlineSession = true;
					addList.PushBack(p->m_User);
				}
			}

			for (S32 i=0; i<addList.Size(); i++)
			{
				NetPlay::C_Session::SessionUser user;
				user.m_User = addList[i];
				user.m_PublicSlot = settings.m_PublicGame != 0 ? true : false;
				serverList.PushBack(user);

				if (serverList.Capacity() == serverList.Size() || serverList.Size() == addList.Size())
				{
					bool ok = (*m_ppRaceMatchmaking)->AddPlayers(serverList);
					if (ok)
					{
						LOGINFO(g_Log, "C_NetRace::PollMatchingPlayers() - added %d player(s) to the matching session.", serverList.Size());
					}
					else
					{
						m_BigError = true;
						LOGWARN(g_Log, "C_NetRace::PollMatchingPlayers() - unable to add %d player(s) to the matching session.", serverList.Size());
					}
					serverList.Clear();
				}
			}
			serverList.Clear();
		}

		// update QoS data on matching session if required.
		PollMatchingQoS();
	}

	void C_NetRace::RemoveMatchingPlayer(const SysProfile::UserIdentifier& plr)
	{
		if (!m_Initialised || !m_Running || !m_OnlineMode)
			return;

		m_MatchingToRemove.PushBack(plr);
	}

	bool C_NetRace::PollMatchingServerMigration(const F32 frameTimeInSeconds)
	{
		if (!m_Initialised || !m_Running || !m_Migrating)
			return false;

		if (eMigrationStateHostInitialDelay == m_MatchingMigrationState)
		{
			m_MatchingInitialDelayTimer += frameTimeInSeconds;
			if (m_MatchingInitialDelayTimer >= 3.0f)
				StartHostMigration();
		}
		else if (eMigrationStateHostStart == m_MatchingMigrationState)
		{
			if (++m_MatchingMigrationNotificationId == 0)
				m_MatchingMigrationNotificationId = 1; // needs to be anything but 0.

			// setup our notification with unique id.
			m_pMatchingMigrationNotification->Reset(m_MatchingMigrationNotificationId);

			const U8 numPlayers = (*m_ppPlayerMgr)->Size(false);

			NetRaceSettings settings;
			GetCurrentRaceSettings(settings);

			// generate game matching data
			NetMatchingServerData matchingServerCreationData;
			BuildMatchingServerData(settings, matchingServerCreationData, numPlayers);

			// convert into platform specific matching properties.
			NetMatchingParameterList matchingServerParamList;
			BuildMatchingParameters(matchingServerCreationData, matchingServerParamList, true, true);
			BIZ_ASSERT(matchingServerParamList.Size() > 0);

			NetPlay::C_Session::SlotConfig matchSlotConfig;
			matchSlotConfig.m_MaxPublic		= (settings.m_PublicGame != 0) ? settings.m_MaxDrivers : 0;
			matchSlotConfig.m_PublicInUse	= (settings.m_PublicGame != 0) ? numPlayers : 0;
			matchSlotConfig.m_MaxPrivate	= (settings.m_PublicGame == 0) ? settings.m_MaxDrivers : 0;
			matchSlotConfig.m_PrivateInUse	= (settings.m_PublicGame == 0) ? numPlayers : 0;

			const bool ok = (*m_ppRaceMatchmaking)->Migrate(m_pMatchingMigrationNotification, &matchingServerParamList[0], matchingServerParamList.Size(), matchSlotConfig);
			if (ok)
			{
				LOGINFO(g_Log, "C_NetRace::PollMatchingServerMigration() - Host : started matching migration.");
				m_MatchingMigrationState = eMigrationStateHostContinue;
			}
			else
			{
				LOGWARN(g_Log, "C_NetRace::PollMatchingServerMigration() - unable to start matching server migration.");
				m_Migrating = false;
				m_SessionIsMigrating = false;
				m_BigError = true; // we should delete the session asap.
			}
		}
		else if (eMigrationStateHostContinue == m_MatchingMigrationState)
		{
			if (m_pMatchingMigrationNotification->m_Finished)
			{
				if (m_MatchingMigrationNotificationId == m_pMatchingMigrationNotification->m_Id)
				{
					LOGINFO(g_Log, "C_NetRace::PollMatchingServerMigration() - Host : matching migration complete, informing peers.");

					// get the new security keys from the migrated session.
					NetPlay::SessionSecurity newSec;
					if ((*m_ppRaceMatchmaking)->GetSecurityInfo(newSec))
					{
						// update our key tracker so it knows about the new new session key.
						m_pSessionKeyTracker->Add(newSec);
				
						// put peers in migration state.
						(*m_ppPlayerMgr)->SetPeersToMigrationPhase(false);

						// Send msg to peers so they start their matching migration process.. 
						SendMigrateMatchingSessionMessage();
						
						m_MatchingMigrationState = eMigrationStateHostWaitForPeers;
						m_MatchingMigrationTimer = 0.0f;
					}
					else
					{
						LOGWARN(g_Log, "C_NetRace::PollMatchingServerMigration() - Host : unable to get security data from migrated session");
						m_Migrating = false;
						m_SessionIsMigrating = false;
						m_BigError = true;
					}
				}
				else
					m_pMatchingMigrationNotification->Reset(0); // old notification so reset and wait for correct one. 
			}
		}
		else if (eMigrationStateHostWaitForPeers == m_MatchingMigrationState)
		{
			m_MatchingMigrationTimer += frameTimeInSeconds;
			const bool timeOut = m_MatchingMigrationTimer >= 10.0f ? true : false;
			if (!(*m_ppPlayerMgr)->AnyPeersStillMigrating(false) || timeOut)
			{
				// if timed out then flag any peers that didn't respond to be dropped.
				if (timeOut)
					(*m_ppPlayerMgr)->PeerMigrationTimeout(false);

				// when using demonware we need to add players into the session again since they won't be migrated over like on xbox360
				for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
				{
					C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
					if (p->m_RaceConnection.IsActive())
						p->m_RaceConnection.m_AddedToOnlineSession = false;
				}
				m_MatchingToRemove.Clear(); // sanity.. no point removing players now as we have just created a new session
				LOGINFO(g_Log, "C_NetRace::PollMatchingServerMigration() - Host : process complete.");

				m_Migrating = false;
				m_SessionIsMigrating = false;
				m_ForceMatchingServerModify = true;
			}
		}
		else if (eMigrationStatePeerWaitingForHost == m_MatchingMigrationState)
		{
			m_MatchingMigrationTimer += frameTimeInSeconds;
			const bool timeOut = m_MatchingMigrationTimer >= 20.0f ? true : false;
			if (timeOut)
			{
				LOGWARN(g_Log, "C_NetRace::PollMatchingServerMigration() - Peer waited too long for host to start matching migration.");
				m_Migrating = false;
				m_SessionIsMigrating = false;
				m_ForceMatchingServerModify = true;
			}
		}
		else if (eMigrationStatePeerContinue == m_MatchingMigrationState)
		{
			if (m_pMatchingMigrationNotification->m_Finished)
			{
				if (m_MatchingMigrationNotificationId == m_pMatchingMigrationNotification->m_Id)
				{
					// when using demonware we need to add players into the session again since they won't be migrated over like on xbox360
					for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
					{
						C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
						if (p->m_RaceConnection.IsActive())
							p->m_RaceConnection.m_AddedToOnlineSession = false;
					}
					m_MatchingToRemove.Clear(); // sanity.. no point removing players now as we have just created a new session
					// get the new security keys from the migrated session.
					NetPlay::SessionSecurity newSec;
					if ((*m_ppRaceMatchmaking)->GetSecurityInfo(newSec))
					{
						LOGINFO(g_Log, "C_NetRace::PollMatchingServerMigration() - Peer : matching migration complete, informing host.");

						// update our key tracker so it knows about the new new session key.
						m_pSessionKeyTracker->Add(newSec);

						// msg host to say our migration is complete.
						SendMigrateMatchingSessionReplyMessage();
					}
					else
					{
						LOGWARN(g_Log, "C_NetRace::PollMatchingServerMigration() - Peer : unable to get security data from migrated session.");
						m_BigError = true;
					}

					m_Migrating = false;
					m_SessionIsMigrating = false;
					m_ForceMatchingServerModify = true;
				}
				else
					m_pMatchingMigrationNotification->Reset(0); // old notification so reset and wait for correct one.
			}
		}
		else
		{
			BIZ_ASSERT_FAIL("C_NetRace::PollMatchingServerMigration() - unknown migration state.");
		}

		return m_Migrating; 
	}

	void C_NetRace::SendMigrateMatchingSessionMessage(void)
	{
		if (!m_Running || !m_OnlineMode)
			return;

		BIZ_ASSERT(eMigrationStateHostContinue == m_MatchingMigrationState);
		BIZ_ASSERT(m_Migrating);

		// msg peers with new security info so they can migrate their session also.
		NetPlay::MigrationInfo migrationInfo;
		if ((*m_ppRaceMatchmaking)->GetMigrationInfo(migrationInfo))
		{
			C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
			bool ok = msg.CreateReliable();
			
			ok = ok && msg.WriteBuffer(&migrationInfo, sizeof(migrationInfo));
			msg.TerminateMessage();
			if (ok)
				SendReliableMessage(msg, eNetMessage_MigrateMatchingSession);
			else
			{
				LOGWARN(g_Log, "C_NetRace::SendMigrateMatchingSessionMessage() - unable to create message.");
			}
		}
		else
		{
			LOGWARN(g_Log, "C_NetRace::SendMigrateMatchingSesionMessage() - unable to get migration data from matching session.");
		}
	}

	bool C_NetRace::ReceiveMigrateMatchingSessionMessage(C_NetMessage& msg, const U8 senderId)
	{
		if (!m_Running || !m_OnlineMode)
			return false;

		if (eMigrationStatePeerWaitingForHost == m_MatchingMigrationState && m_Migrating)
		{
			const C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(false);
			if (pHost && senderId == pHost->m_RaceConnection.m_JoinId)
			{
				NetPlay::MigrationInfo migrationInfo;
				bool ok = msg.ReadOpen();
				ok = ok && msg.ReadBuffer(&migrationInfo, sizeof(migrationInfo));
				if (ok)
				{
					if (++m_MatchingMigrationNotificationId == 0)
						m_MatchingMigrationNotificationId = 1; // needs to be anything but 0.

					// setup our notification with unique id.
					m_pMatchingMigrationNotification->Reset(m_MatchingMigrationNotificationId);

					const U8 numPlayers = (*m_ppPlayerMgr)->Size(false);
					NetRaceSettings settings;
					GetCurrentRaceSettings(settings);

					NetPlay::C_Session::SlotConfig matchSlotConfig;
					matchSlotConfig.m_MaxPublic		= (settings.m_PublicGame != 0) ? settings.m_MaxDrivers : 0;
					matchSlotConfig.m_PublicInUse	= (settings.m_PublicGame != 0) ? numPlayers : 0;
					matchSlotConfig.m_MaxPrivate	= (settings.m_PublicGame == 0) ? settings.m_MaxDrivers : 0;
					matchSlotConfig.m_PrivateInUse	= (settings.m_PublicGame == 0) ? numPlayers : 0;

					const bool ok = (*m_ppRaceMatchmaking)->MigratePeer(migrationInfo, m_pMatchingMigrationNotification, matchSlotConfig);
					if (ok)
					{
						LOGINFO(g_Log, "C_NetRace::ReceiveMigrateMatchingSessionMessage() - Peer : started matching migration.");
						m_MatchingMigrationState = eMigrationStatePeerContinue;
					}
					else
					{
						LOGWARN(g_Log, "C_NetRace::ReceiveMigrateMatchingSessionMessage() - unable to start matching server migration.");
						m_Migrating = false;
						m_SessionIsMigrating = false;
						m_BigError = true; // we should delete the session asap.
						m_MatchingMigrationState = eMigrationStateNotActive;
					}
				}
				else
				{
					LOGWARN(g_Log, "C_NetRace::ReceiveMigrateMatchingSessionMessage() - malformed msg received.");
				}
			}
			else
			{
				LOGWARN(g_Log, "C_NetRace::ReceiveMigrateMatchingSessionMessage() - received from not the current host, ignoring.");
			}
		}

		return true;
	}

	void C_NetRace::SendMigrateMatchingSessionReplyMessage(void)
	{
		if (!m_Running || !m_OnlineMode)
			return;

		if (eMigrationStatePeerContinue == m_MatchingMigrationState && m_Migrating)
		{
			C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
			bool ok = msg.CreateReliable();
			ok = ok && msg.WriteU8(0); // not really required.
			msg.TerminateMessage();
			if (ok)
				SendReliableMessage(msg, eNetMessage_MigrateMatchingSessionReply);
			else
			{
				LOGWARN(g_Log, "C_NetRace::SendMigrateMatchingSessionReplyMessage() - unable to create message.");
			}
		}
	}

	bool C_NetRace::ReceiveMigrateMatchingSessionReplyMessage(C_NetMessage& msg, const U8 senderId)
	{
		if (!m_Running || !m_OnlineMode)
			return false;

		if (eMigrationStateHostWaitForPeers == m_MatchingMigrationState && m_Migrating)
			(*m_ppPlayerMgr)->SetPeerMigrationComplete(senderId, false);

		return true;
	}

	void C_NetRace::BuildMatchingServerData(const NetRaceSettings& settings, NetMatchingServerData& outData, const U8 numPlayersInSession)
	{
		if (!m_Initialised || !m_Running || !m_OnlineMode)
			return;

		// which local player controls the matching session ?
		C_NetPlayer* plr = NULL;
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize() && NULL == plr; i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (!p->LocalPlayer())
				continue;
			if (p->m_ControllerPort == m_LocalMatchingSessionManagerIndex)
				plr = (*m_ppPlayerMgr)->Index(i);
		}

		BIZ_ASSERT(plr);
		BIZ_ASSERT(plr->LocalPlayer());

		outData.m_MatchingMode			= eNetMatchingModeNormal;
		outData.m_RankRestriction		= plr->m_ProfileData.m_Game.m_Rank;
		outData.m_MatchingRegion		= (*m_ppRouter)->GetMatchingRegion();
		outData.m_EventId				= (S64)settings.m_EventId;
		outData.m_CustomRace			= settings.m_CustomRace;
		outData.m_OpenSlotCount			= settings.m_MaxDrivers - numPlayersInSession;
		outData.m_Nat					= (*m_ppRouter)->PeerRoutingEnabled() ? (*m_ppRouter)->GetNatType() : (*m_ppPlayerMgr)->GetMostRestrictiveNatType(false);
		outData.m_InRace				= (m_InRace == true) ? 1 : 0;

		outData.m_PackVersion			= S32(	((C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion << 16) & 0xffff0000) 
												|	((C_MultiplayerData::Instance().m_pConfig->m_PackVersion << 0) & 0x0000ffff));

		if (settings.m_PublicGame) 
		{
			// Stops the matchmaking server advertising the session if the race is in progress
			// this is controlled by the UI calling NetRace.FlagInRace() from lua.
			// usability requested this to stop player having to wait in lobbies for large amounts of time.
			outData.m_Searchable = (CheckIfChampionshipJoinsAllowed() == true) ? 1 : 0; 
		}
		else
			outData.m_Searchable = 0; // private games can't be found by matching queries. 

		BIZ_ASSERT((((S32)settings.m_MaxDrivers) - ((S32)numPlayersInSession)) >= 0);
	}

	bool C_NetRace::CheckIfChampionshipJoinsAllowed(void)
	{
		if (!m_Initialised ||  !m_Running)
			return false;

		NetRaceSettings raceSettings;
		GetCurrentRaceSettings(raceSettings);

		// Only if its the first race
		return ( ( raceSettings.m_Championship ) ? !raceSettings.m_HasAnyRacesStarted : true );
	}

	void C_NetRace::ForceRaceTimeout()
	{
		if (!m_Initialised || !m_Running)
			return;

		C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
		msg.CreateReliable();
		msg.WriteU8(0);	//	Can't send an empty message
		msg.TerminateMessage();

		SendReliableMessage(msg, eNetMessage_ForceRaceTimeout); 
	}
} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
