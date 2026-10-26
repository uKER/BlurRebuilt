#include <stdafx.h>
#include <Core/core.h>
#include <Network/Core.h>
#include <Network/Securekey.h>
#include <Network/Session.h>
#include <Network/CommunicatorManager.h>
#include <Network/SessionCommandNotification.h>
#include <Network/SessionDeletion.h>
#include <bdNet/bdNet.h>
#include "../Misc/NetVersion.h"
#include "../Misc/NetDebug.h"
#include "../Misc/NetDefines.h"
#include "../Misc/NetMigrationStates.h"
#include "../Misc/NetRaceModeListener.h"
#include "../Core/NetMatchingParameters.h"
#include "NetRace.h"
#include "NetParty.h"
#include "NetServices.h"
#include "NetSessionKeyTracker.h"
#include "NetCreationHelper.h"
#include "NetJoinHelper.h"
#include "../Misc/NetDatagramPacketTypes.h"
#include "../Message/NetMessage.h"
#include "../Router/NetRouterBase.h"
#include "../Player/NetPlayerManager.h"
#include "../Player/NetPlayer.h"
#include"../Core/NetRichPresence.h"
#include"../Misc/NetJoinRaceFromPartyData.h"
#include <GameLogic/GameData/MultiplayerData.h>
#include <GameLogic/Profile/GameProfile.h>
#include <GameLogic/Profile/GameProfileManager.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameApp.h>
#include <UI/UiMessages.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

#if !defined ( CFG_GOLD )
Debug::Bool g_Join_ForceError_Generic_Party("Network.Party.JoinErrors.ForceGeneric", false);
Debug::Bool g_Join_ForceError_SlotCount_Party("Network.Party.JoinErrors.ForceNotEnoughSlots", false);
Debug::Bool g_Join_ForceError_NoComms_Party("Network.Party.JoinErrors.ForceNoCommunications", false);
Debug::Bool g_Join_ForceError_Started_Party("Network.Party.JoinErrors.ForcePartyClosed", false);
#endif

#ifndef CFG_GOLD
namespace RacerNet
{
	extern Debug::Enum<eNetBuildVersion> NET_PACKET_VERSION;
}
#endif


namespace RacerNet
{
	C_NetParty::C_NetParty() :	m_Initialised(false), m_Running(false), m_BigError(false), m_VoiceEnabled(false), m_Migrating(false), m_ReturnToLobby(false), m_QoSEnabled(false),
								m_DisconnectResendMsgTimer(0.0f), m_ppPlayerMgr(NULL), m_ppRouter(NULL), m_ppPartyMatchmaking(NULL), m_pSessionKeyTracker(NULL), 
								m_MatchingMigrationState(eMigrationStateNotActive), m_MatchingMigrationTimer(0.0f), m_MatchingMigrationNotificationId(0), 
								m_pMatchingMigrationNotification(NULL), m_MatchingPlayerCommandState(ePartyMatchingStatePlayersRemoved),
								m_pMatchingRemovePlayersNotification(NULL), m_MatchingRemovePlayersNotificationId(0), m_pJoinRaceFromPartyData(NULL)
	{
	}

	C_NetParty::~C_NetParty()
	{
		Shutdown();
	}

	bool C_NetParty::Initialise(C_NetRouterBase** ppRouter, C_NetPlayerManager **ppPlayerManager, NetPlay::C_Session** ppPartyMatchmaking)
	{
		if (m_Initialised)  
			return true;

		BIZ_ASSERT(ppRouter != NULL);
		BIZ_ASSERT(*ppRouter != NULL);
		BIZ_ASSERT(ppPlayerManager != NULL);
		BIZ_ASSERT(*ppPlayerManager != NULL);
		BIZ_ASSERT(ppPartyMatchmaking != NULL);
		BIZ_ASSERT(*ppPartyMatchmaking != NULL);

		m_ppRouter = ppRouter; 
		m_ppPlayerMgr = ppPlayerManager;
		m_ppPartyMatchmaking = ppPartyMatchmaking;
		m_pMatchingMigrationNotification = new NetPlay::SessionCommandNotification;
		m_pMatchingRemovePlayersNotification = new NetPlay::SessionCommandNotification;
		m_pSessionKeyTracker = new C_NetSessionKeyTracker;
		m_pJoinRaceFromPartyData = new NetJoinRaceFromPartyData;

		if (m_pSessionKeyTracker && m_pMatchingMigrationNotification && m_pMatchingRemovePlayersNotification && m_pJoinRaceFromPartyData)
		{
			m_QoS.m_Version = NET_PACKET_VERSION;
			m_Initialised = true;
		}
		else
			Shutdown();

		return m_Initialised;
	}

	void C_NetParty::Shutdown(void)
	{
		DisconnectAndClear();

		if (m_pJoinRaceFromPartyData)
			delete m_pJoinRaceFromPartyData;
		m_pJoinRaceFromPartyData = NULL;

		if (m_pMatchingRemovePlayersNotification)
			delete m_pMatchingRemovePlayersNotification;
		m_pMatchingRemovePlayersNotification = NULL;

		if (m_pMatchingMigrationNotification)
			delete m_pMatchingMigrationNotification;
		m_pMatchingMigrationNotification = NULL;

		if (m_pSessionKeyTracker)
			delete m_pSessionKeyTracker;
		m_pSessionKeyTracker = NULL;

		m_ppPartyMatchmaking = NULL;
		m_ppRouter = NULL;
		m_ppPlayerMgr = NULL;
		m_Initialised = false;
	}

	void C_NetParty::Poll(const F32 frameTimeInSeconds, const bool raceSessionRunning)
	{
		if (!m_Initialised)
			return;

		// track security keys still in use
		PollSecurityKeyTracker();

		// continue the join procedure if active.
		PollJoin();

		// track any failed join attempts to allow for message to reach peer before disconnecting.
		JoinPollFailures(*(*m_ppRouter));

		// setup who we send voice to.
		PollVoice();

		// update matching server with latest player data.
		PollMatchingServer(raceSessionRunning);

		// run disconnection logic
		PollDisconnects(frameTimeInSeconds);
	}

	void C_NetParty::DisconnectAndClear(void)
	{
		if (!m_Initialised)
			return;
			
		JoinSetPeerJoinFailureNameSetValid(false);

		if (m_Running)
		{
			LOGDEBUG(g_Log, "C_NetParty::DisconnectAndClear()");

			if ((*m_ppPlayerMgr)->TotalSize() > 0)
			{
				const C_NetPlayer* pLocal = (*m_ppPlayerMgr)->Index(0);
				BIZ_ASSERT(pLocal->LocalPlayer());
				if (pLocal->m_PartyConnection.m_JoinId > 0)
				{
					C_NetMessage& msg = (*m_ppRouter)->GetOutgoingUnReliable();
					msg.Flush();
					bool ok = msg.CreateUnReliable(eNetMessage_Leaving, pLocal->m_PartyConnection.m_JoinId, true);
					ok = ok && msg.WriteU64(pLocal->m_ConsoleId);
					msg.TerminateMessage();

					// send to peers who have a connected socket.
					for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
					{
						C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
						if (p->LocalPlayer())
							continue;

						if (p->PrimaryPlayer() && p->m_PartyConnection.IsConnected())
						{
							for (U8 z=0; z<3; z++)
								(*m_ppRouter)->SendMessage(msg, p->m_PartyConnection.m_Address);
						}
					}
				}
			}

			DestroyAllPlayers();

			FlushJoinHelper(false, *(*m_ppRouter));
		}
	
		// matchmaking
		(*m_ppPartyMatchmaking)->Delete();

		m_MatchingPlayerCommandState = ePartyMatchingStatePlayersRemoved;
		m_Migrating = false;
		m_MatchingMigrationState = eMigrationStateNotActive;
		m_MatchingMigrationTimer = 0.0f;
		m_MatchingMigrationNotificationId = 0;
		m_pMatchingMigrationNotification->Reset();
		m_pMatchingRemovePlayersNotification->Reset();
		m_MatchingRemovePlayersNotificationId = 0;
		m_pSessionKeyTracker->Flush();
		m_MatchingToRemove.Clear();
		(*m_ppPlayerMgr)->Flush(true);
		m_BigError = false;
		m_VoiceEnabled = false;
		m_ReturnToLobby = false;
		m_QoSEnabled = false;
		m_DisconnectResendMsgTimer = 0.0f;
		m_pJoinRaceFromPartyData->Clear();
		m_Running = false;
	}

	bool C_NetParty::CreateOnlineSession(C_NetCreationHelper& helper)
	{
		if (!m_Initialised || m_Running)
			return false;

		if (0 == helper.m_ConsoleId)
			return false; // invalid console identifier.

		FlushJoinHelper(true, *(*m_ppRouter));

		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE && (*m_ppPartyMatchmaking)->SessionState() == NetPlay::C_Session::eStateRunning)
		{
			// get secure key data from matching session and store in our key tracker..
			NetPlay::SessionSecurity sec;
			if ((*m_ppPartyMatchmaking)->GetSecurityInfo(sec))
			{
				m_pSessionKeyTracker->Add(sec);

				if (CreateLocalPlayers(helper, true))
				{
					m_QoSEnabled = true;
					m_Running = true;
					JoinSetState(eJoinStateComplete);
				}
			}
		}

		if (!m_Running)
			DisconnectAndClear();
		return m_Running;
	}

	bool C_NetParty::JoinOnlineSession(C_NetCreationHelper& helper, NetPlay::SessionSecurity& security, const U8* pHostCommonAddress, const U64 hostConsoleId, const SysProfile::BaseProfile& hostName)
	{
		if (!m_Initialised || m_Running || NULL == pHostCommonAddress)
			return false;

		if (0 == helper.m_ConsoleId)
			return false; // invalid coneole identifier.

		FlushJoinHelper(true, *(*m_ppRouter));

		// store the host name just incase we fail to connect.
		JoinSetPeerJoinFailureName(hostName);

		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE && (*m_ppPartyMatchmaking)->SessionState() == NetPlay::C_Session::eStateRunning)
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

	bool C_NetParty::ReceiveReliable(C_NetMessage& msg, const U8 msgType, const U8 senderId)
	{
		if (!m_Initialised || !m_Running)
			return false;

		switch (msgType)
		{
		case eNetMessage_JoinData:
			ReceiveJoinDataMessage(msg, senderId);
			return true;
			break;

		case eNetMessage_InitialData:
			ReceiveInitialDataMessage(msg, senderId);
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

		case eNetMessage_PartyPeerJoinRace:
			ReceivePeerJoinRaceSessionMessage(msg, senderId);
			return true;
			break;

		case eNetMessage_PartyPeerReturnToLobby:
			ReceivePeerReturnToLobbyMessage(msg, senderId);
			return true;
			break;

		case eNetMessage_CommunicatorStatusUpdate:
			ReceiveCommunicatorStatusChangedMessage(msg, senderId);
			return true;
			break;
		}

		return false;
	}

	bool C_NetParty::ReceiveUnReliable(C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, bdAddrHandleRef& addr)
	{
		if (!m_Initialised || !m_Running)
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

		return false;
	}

	bool C_NetParty::ReceiveJoinRequest(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, bdAddrHandleRef& addr)
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

		// will push back any player who joined into this stack for tracking.
		NetJoinReferencesSingleConsole joined;	

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
		if ((*m_ppPlayerMgr)->LocalPrimaryJoinId(true) != 0)
		{
			myPrimaryJoinId = (*m_ppPlayerMgr)->LocalPrimaryJoinId(true);
		}
		else
		{
			// set local primary join id since we have yet to receive one from the host, its possible multiple
			// peers are joining and we have yet to recive the join reply from the host.
			C_NetPlayer* pLocalPrimary = (*m_ppPlayerMgr)->Index(0);
			BIZ_ASSERT(pLocalPrimary->LocalPlayer());
			pLocalPrimary->m_PartyConnection.m_JoinId = myPrimaryJoinId;
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
		if (g_Join_ForceError_Generic_Party)
		{
			JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorNoConnectionToHost, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), true);
			return true;
		}
		else if (g_Join_ForceError_SlotCount_Party)
		{
			JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorNotEnoughOpenSlots, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), true);
			return true;
		}
		else if (g_Join_ForceError_NoComms_Party)
		{
			JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorNoCommunications, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), true);
			return true;
		}
		else if (g_Join_ForceError_Started_Party)
		{
			JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorPartyClosed, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), true);
			return true;
		}
#endif

		if (ok)
		{
			// does the host console id match that of the current race host ?
			C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(true);
			if (NULL != pHost && hostConsoleId == pHost->m_ConsoleId && pHost->m_PartyConnection.IsActive() && eNetPlayerDisconnectionTypeNone == pHost->m_PartyConnection.m_DisconnectRequested)
			{
				// check is this player already in our race session.
				bool addPlayers = false;
				C_NetPlayer *pPeer = (*m_ppPlayerMgr)->FindFromConsoleId(joiningConsoleId, 0);
				if (NULL == pPeer)
					addPlayers = true;
				else if (!pPeer->m_PartyConnection.IsActive())
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

					if (!ok || numToJoin > NET_MAX_PARTY_MEMBERS || 0 == numToJoin)
					{
						// dodgy data.
						msg.ReadClose();
						(*m_ppRouter)->Disconnect(addr);
					}
					else
					{
						if (pHost->LocalPlayer())
						{
							if (pHost->m_PartyConnection.m_PlayerState != eNetPlayerStateIdle)
							{
								// fail because party is closed.
								msg.ReadClose();
								JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorPartyClosed, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), true);
								return true;
							}
							else
							{
								// check number of slots remaining.	
								const U8 remainingSlots =  NET_MAX_PARTY_MEMBERS  - (*m_ppPlayerMgr)->Size(true);
								if (remainingSlots < numToJoin || remainingSlots > NET_MAX_PARTY_MEMBERS)
								{
									// fail because not enough player slots remaining.
									msg.ReadClose();
									JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorNotEnoughOpenSlots, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), true);
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
								// if peer packet routing is enabled we dont need to fail players because of nat issues.
								if (!(*m_ppRouter)->PeerRoutingEnabled())
								{
									// check against most restrictive nat already in the game.
									SysProfile::BaseProfile natUser;
									if (!(*m_ppPlayerMgr)->CheckAgainstMostRestrictiveNat(true, profile.m_NatType, natUser))
									{
										LOGINFO(g_Log, "'%s' join Denied because of NAT incompatbility with current peers.", bs.Name());
										JoinFailConnectionWithName(eNetMessage_JoinFailure,  eJoinErrorNoCommunications, natUser, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), true);

										// destroy any players we may have alreadys etup.
										for (S32 i=0; i<joined.Size(); i++)
											DestroyPlayer(joined[i]);

										msg.ReadClose();
										return true;
									}
								}

								C_NetPlayer* pPlayer = (*m_ppPlayerMgr)->AllocatePlayer(true);
								if (pPlayer)
								{
									// notify game logic a new player has joined the our party session.
									// only notify once local player has called EnterLobby()
									if (m_VoiceEnabled)
										C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->PlayerNotification(bs.Name(), true, true);

									// overwrite generic player data. 
									pPlayer->m_User = bs;

									for (U32 z=0; z<BD_COMMON_ADDR_SERIALIZED_SIZE; z++)
										pPlayer->m_CommonAddress[z] = commonAddr[z];		

									pPlayer->m_ConsoleId		= joiningConsoleId;												
									pPlayer->m_UpStreamKbps		= bandwidthUp;												
									pPlayer->m_DownStreamKbps	= bandwidthDown;	
									pPlayer->m_DlcMask			= dlcMask;
									pPlayer->m_VoiceBanned		= voiceBanned != 0 ? true : false;
									pPlayer->m_ProfileData		= profile;
									pPlayer->m_LocalIndex		= i;		

									// copy race specific data.
									pPlayer->m_PartyConnection.m_PlayerState = eNetPlayerStateJoining;
									pPlayer->m_PartyConnection.m_HandshakingComplete = true;

									// store remote address for primary player
									if (i == 0)
										pPlayer->m_PartyConnection.m_Address = addr;

									// if its an invalid join id then allocate one now, only host console will need to be doing this.
									if (0 == joinId)
									{
										BIZ_ASSERT(pHost->LocalPlayer());
										pPlayer->m_PartyConnection.m_JoinId = (*m_ppPlayerMgr)->GenerateJoinReference(true);
									}
									else
										pPlayer->m_PartyConnection.m_JoinId = joinId;

									// enable keepalives now
									pPlayer->m_PartyConnection.StartKeepAliveTimer();

									// player is now valid so push into joined list.
									BIZ_ASSERT(pPlayer->m_PartyConnection.m_JoinId != 0); // 0 is an invalid join id
									joined.PushBack(pPlayer->m_PartyConnection.m_JoinId);
								}
								else 
									ok = false;
							}
						}

						if (ok)
						{
							BIZ_ASSERT(joined.Size() > 0); 

							// send back join reply 
							C_NetPlayer* pJoinReplyPlayer = (*m_ppPlayerMgr)->FindFromJoinId(joined[0], true);
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
										ok = ok && reply.WriteU8(pH->m_PartyConnection.m_JoinId);
										ok = ok && reply.WriteU8(pH->m_VoiceBanned ? 1 : 0);
										ok = ok && reply.WriteBuffer(&(pH->m_ProfileData), sizeof(pH->m_ProfileData));
										ok = ok && reply.WriteU8(pH->m_PartyConnection.m_Host);	
										ok = ok && reply.WriteU8(pH->m_PartyConnection.m_PlayerState);
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
										ok = ok && reply.WriteU8(pPeer->m_PartyConnection.m_PlayerState);
									}
								}

								reply.TerminateMessage();

								// add message into reliable message queue of primary remote player.
								// flag as initial connection mesage.
								if (ok)
									pJoinReplyPlayer->SendReliable(reply, eNetMessage_JoinReply, true, myPrimaryJoinId, true);

								// if hosting we need to send back a list of what other peers the joining player needs to make a connection to.
								if (ok && pHost->LocalPlayer())
								{
									// build a list of additional peer data that need to go to the joining peer
									NetJoinReferences additionalPeers;
									for (U8 z=0; z<(*m_ppPlayerMgr)->TotalSize(); z++)
									{
										C_NetPlayer* pAdditional = (*m_ppPlayerMgr)->Index(z);
										if (pAdditional->LocalPlayer() || !pAdditional->m_PartyConnection.IsActive())
											continue;

										bool add = true;
										for (S32 q=0; q<joined.Size(); q++)
										{
											if (joined[q] == pAdditional->m_PartyConnection.m_JoinId)
												add = false;
										}

										if (add)
											additionalPeers.PushBack(pAdditional->m_PartyConnection.m_JoinId);
									}

									reply.Flush();
									ok = ok && reply.CreateReliable();
									ok = ok && reply.WriteU8((U8)additionalPeers.Size());

									for (U8 z=0; z<additionalPeers.Size(); z++)
									{
										C_NetPlayer* pAdditional = (*m_ppPlayerMgr)->FindFromJoinId(additionalPeers[z], true);
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
										ok = ok && reply.WriteU8(pAdditional->m_PartyConnection.m_JoinId);
										ok = ok && reply.WriteU8(pAdditional->m_PartyConnection.m_PlayerState);
									}

									reply.TerminateMessage();

									if (ok)
										pJoinReplyPlayer->SendReliable(reply, eNetMessage_JoinData, true, myPrimaryJoinId);
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
				JoinFailConnection(eNetMessage_JoinFailure,  eJoinErrorHostDiffersOnPeer, joiningConsoleId, myPrimaryJoinId, addr, *(*m_ppRouter), true);
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

	bool C_NetParty::ReceiveJoinReply(C_NetMessage& msg, const DatagramReliablePacketHeader& hdr, bdAddrHandleRef& addr, const U8 senderId)
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
									pLocal->m_PartyConnection.m_JoinId = joinId;
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
											C_NetPlayer* pPlayer = NULL;

											C_NetPlayer *pPartyPlayer = (*m_ppPlayerMgr)->FindFromConsoleId(consoleId, i);
											if (NULL == pPartyPlayer)
												pPlayer = (*m_ppPlayerMgr)->AllocatePlayer(true);
											else
												pPlayer = pPartyPlayer; 

											if (pPlayer)
											{
												pPlayer->m_User				= bs;
												pPlayer->m_ConsoleId		= consoleId;												
												pPlayer->m_UpStreamKbps		= bandwidthUp;												
												pPlayer->m_DownStreamKbps	= bandwidthDown;	
												pPlayer->m_DlcMask			= dlcMask;
												pPlayer->m_VoiceBanned		= voiceBanned != 0 ? true : false;
												pPlayer->m_ProfileData		= profile;
												pPlayer->m_LocalIndex		= i;	

												for (U32 z=0; z<BD_COMMON_ADDR_SERIALIZED_SIZE; z++)
													pPlayer->m_CommonAddress[z] = commonAddr[z];

												// copy race specific data.
												pPlayer->m_PartyConnection.m_PlayerState = eNetPlayerStateJoining;
												pPlayer->m_PartyConnection.m_HandshakingComplete = true;
												pPlayer->m_PartyConnection.m_JoinId = joinid;
												pPlayer->m_PartyConnection.m_Host = (host == 1) ? true : false;
												pPlayer->m_PartyConnection.m_PlayerState = static_cast<eNetPlayerState> (state);

												// store remote address for primary player only
												if (i == 0)
													pPlayer->m_PartyConnection.m_Address = addr;

												pPlayer->m_PartyConnection.StartKeepAliveTimer();
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
										BIZ_ASSERT(p->m_PartyConnection.IsActive());

										p->m_PartyConnection.m_HandshakingComplete = true;
										p->m_PartyConnection.m_PlayerState = static_cast<eNetPlayerState> (state);
										p->m_PartyConnection.StartKeepAliveTimer();
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

	bool C_NetParty::ReceiveJoinFailure(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, bdAddrHandleRef& addr)
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
				DisconnectAndClear();
				JoinSetFailed(static_cast<eNetJoinSessionErrors> (errorCode), *(*m_ppRouter));

				// this case is to catch the fact that the host knows we cant join to a peer so why try.
				// the host gives us back the name of the peer who we wwould not beable to connect to.
				if (eJoinErrorNoCommunications == errorCode)
				{
					JoinSetPeerJoinFailureName(prof);
					JoinSetPeerJoinFailureNameSetValid(true);
				}
			}
			else
			{
				LOGWARN(g_Log, "C_NetParty::ReceiveJoinFailure() - invalid data received.");
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

	bool C_NetParty::JoinInProgressAvailable(bdSecurityID& sessionId)
	{
		if (!m_Initialised || !m_Running || m_Migrating)
			return false;

		if (JoinGetState() != eJoinStateComplete)
			return false;

		// is the host player still flagged as being idle (ie in the party lobby)
		C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(true);
		if (NULL == pHost)
			return false;
		if (pHost->m_PartyConnection.m_PlayerState != eNetPlayerStateIdle)
			return false;

		if (NetPlay::C_SessionDeletion::Instance().ActiveCount() > 0)
			return false;

		if ((*m_ppPlayerMgr)->Size(true) < NET_MAX_PARTY_MEMBERS)
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

	bool C_NetParty::CanInvite(NetPlay::SessionSecurity& sessionSec)
	{
		bdSecurityID id;
		if (JoinInProgressAvailable(id))
		{
			if (m_pSessionKeyTracker->Current(sessionSec))
				return true;
		}
		return false;
	}

	bool C_NetParty::InSession(const bdSecurityID& sessionId)
	{
		if (!m_Initialised || !m_Running)
			return false;

		return m_pSessionKeyTracker->IsRegistered(sessionId);
	}

	bool C_NetParty::MatchingSessionFullyDeActivated(void)
	{
		if (!m_Initialised || !m_Running)
			return true;
		
		if (!(*m_ppPartyMatchmaking)->IsIdle() || m_MatchingToRemove.Size() > 0 || m_Migrating)
			return false;

		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (p->m_PartyConnection.m_AddedToOnlineSession)
				return false;
		}

		return m_MatchingPlayerCommandState == ePartyMatchingStatePlayersRemoved ? true : false;
	}

	bool C_NetParty::GetJoinRaceFromPartyData(NetJoinRaceFromPartyData& out)
	{
		out.Clear();
		if (!m_Initialised || !m_Running)
			return false;

		out = *m_pJoinRaceFromPartyData;
		return true;
	}

	void C_NetParty::SetJoinRaceFromPartyData(const NetJoinRaceFromPartyData& in)
	{
		if (!m_Initialised || !m_Running)
			return ;
		*m_pJoinRaceFromPartyData = in;
	}

	bool C_NetParty::ReceivedReturnToLobbyRequest(const bool clear)
	{
		if (!m_Initialised || !m_Running)
			return false;
		const bool r = m_ReturnToLobby;
		if (clear)
			m_ReturnToLobby = false;
		return r;
	}

	void C_NetParty::SendPeerJoinRaceSessionMessage(const NetJoinRaceFromPartyData& data)
	{
		if (!m_Initialised || !m_Running)
			return;

		if (JoinGetState() != eJoinStateComplete)
		{
			BIZ_ASSERT_FAIL("C_NetParty::SendPeerJoinRaceSessionMessage() - called before join complete.");
			return;  
		}

		BIZ_ASSERT(data.m_Active);
		if (!data.m_Active)
			return;

		C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(true);
		if (NULL == pHost || !pHost->LocalPlayer())
		{
			LOGERROR(g_Log, "C_NetParty::SendPeerJoinRaceSessionMessage() - called for via non party host player !!");
			return;
		}

		BIZ_ASSERT(pHost->m_PartyConnection.m_JoinId > 0);

		NetJoinRaceFromPartyData dataCopy = data;

		bool ok = true;
		C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
		ok = msg.CreateReliable();
		ok = ok && msg.WriteU8(pHost->m_PartyConnection.m_JoinId);
		ok = ok && msg.WriteBuffer(&dataCopy, sizeof(data));

		// Send nonce 
		for ( U16 i = 0; i < pHost->m_PlayerNonce.Size(); i++ )
			ok = ok && msg.WriteU8( pHost->m_PlayerNonce.m_Nonce[i] );

		msg.TerminateMessage();

		LOGINFO( g_Log, "C_NetParty::SendPeerJoinRaceSessionMessage() - sent party id %d %d %d %d %d %d %d %d", 
					 pHost->m_PlayerNonce.m_Nonce[0], pHost->m_PlayerNonce.m_Nonce[1], pHost->m_PlayerNonce.m_Nonce[2], pHost->m_PlayerNonce.m_Nonce[3],
					 pHost->m_PlayerNonce.m_Nonce[4], pHost->m_PlayerNonce.m_Nonce[5], pHost->m_PlayerNonce.m_Nonce[6], pHost->m_PlayerNonce.m_Nonce[7] );

		if (ok)
			SendReliableMessage(msg, eNetMessage_PartyPeerJoinRace);
		else
		{
			LOGWARN(g_Log, "C_NetParty::SendPeerJoinRaceSessionMessage() - unable to create network message");
		}
	}

	void C_NetParty::SendPeerReturnToLobbyMessage(void)
	{
		if (!m_Initialised || !m_Running)
			return;

		if (JoinGetState() != eJoinStateComplete)
		{
			BIZ_ASSERT_FAIL("C_NetParty::SendPeerReturnToLobbyMessage() - called before join complete.");
			return;  
		}

		C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(true);
		if (NULL == pHost || !pHost->LocalPlayer())
		{
			LOGERROR(g_Log, "C_NetParty::SendPeerReturnToLobbyMessage() - called for via non party host player !!");
			return;
		}

		BIZ_ASSERT(pHost->m_PartyConnection.m_JoinId > 0);

		bool ok = true;
		C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
		ok = msg.CreateReliable();
		ok = ok & msg.WriteU8(pHost->m_PartyConnection.m_JoinId);
		msg.TerminateMessage();

		if (ok)
			SendReliableMessage(msg, eNetMessage_PartyPeerReturnToLobby);
		else
		{
			LOGWARN(g_Log, "C_NetParty::SendPeerReturnToLobbyMessage() - unable to create network message");
		}
	}

	U32 C_NetParty::GenerateAvailableDlc(void)
	{
		if (!m_Initialised || !m_Running)
			return 0;

		U32 dlc = 0;

		for (U32 z=0; z<32; z++)
		{
			// make sure all current peers have access to this content pack.
			bool addPack = true;
			for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
			{
				C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
				if (!p->m_PartyConnection.IsActive())
					continue;

				if (!(p->m_DlcMask&(1<<z)))
					addPack = false;
			}

			if (addPack)
				dlc |= (1 << z);
		}

		return dlc;
	}

	void C_NetParty::SendCommunicatorStatusChangedMessage(void)
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

			ok = ok && msg.WriteU8(p->m_PartyConnection.m_JoinId);
			ok = ok && msg.WriteU8(p->m_HasCommunicator == true ? 1 : 0);
		}
		msg.TerminateMessage();

		if (ok)
			SendReliableMessage(msg, eNetMessage_CommunicatorStatusUpdate);
		else
		{
			LOGWARN(g_Log, "C_NetParty::SendCommunicatorStatusChangedMessage() - unable to create communicator status update message.");
		}
	}

	// peer has changed communicator status
	bool C_NetParty::ReceiveCommunicatorStatusChangedMessage(C_NetMessage& msg, const U8 senderId)
	{
		LOGINFO(g_Log, "C_NetParty::ReceiveCommunicatorStatusChangedMessage()");

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
					C_NetPlayer* plr = (*m_ppPlayerMgr)->FindFromJoinId(joinId, true);
					if (plr)
						plr->m_HasCommunicator = (hasCommunicator == 1) ? true : false;
				}
			}
			msg.ReadClose();
		}

		if (!ok)
		{
			LOGWARN(g_Log, "C_NetParty::ReceiveCommunicatorStatusChangedMessage() - malformed data received.");
		}

		return true;
	}

	void C_NetParty::VoiceMuteChanged(void)
	{
		if (!m_Initialised || !m_Running)
			return;

		// realtime mute only applicable once we have fully joined a lobby.
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
			(*m_ppPlayerMgr)->BuildMuteList(i, muteList, true);

			ok = ok && msg.WriteU8(p->m_PartyConnection.m_JoinId);
			ok = ok && msg.WriteBuffer(&muteList, sizeof(muteList));
		}
		msg.TerminateMessage();

		if (ok)
			SendReliableMessage(msg, eNetMessage_MuteChange);
		else
		{
			LOGWARN(g_Log, "C_NetParty::MuteInfoModified() - unable to create mute message.");
		}
	}

	void C_NetParty::SendLocalPlayerStatesMessage(void)
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

			BIZ_ASSERT(p->m_PartyConnection.m_JoinId != 0);
			added += 1;

			ok = ok && msg.WriteU8(p->m_PartyConnection.m_JoinId);				// write player identifier
			ok = ok && msg.WriteU8(p->m_PartyConnection.m_PlayerState);			// write actual player state

			if (!ok)
			{
				LOGERROR(g_Log, "C_NetParty::SendLocalPlayerStatesMessage() - failed to build network message");
				return;
			}
		}

		msg.TerminateMessage();
		if (0 == added)
			return;

		// attempt to send message to connected peers
		SendReliableMessage(msg, eNetMessage_PlayerStates);
	}

	void C_NetParty::SendJoinFailed(const eNetJoinSessionErrors reason, const bdAddrHandleRef& returnAddr)
	{
		if (!m_Initialised || !m_Running)
			return;

		C_NetMessage& msg = (*m_ppRouter)->GetOutgoingUnReliable();

		// create base join failed message.
		msg.CreateUnReliable(eNetMessage_JoinReply, (*m_ppPlayerMgr)->LocalPrimaryJoinId(true), true);

		// pack failed reason.
		msg.WriteU8(reason);

		// end writing.
		msg.TerminateMessage();

		(*m_ppRouter)->SendMessage(msg, returnAddr);
	}

	// run disconnection logic
	void C_NetParty::PollDisconnects(const F32 frameTimeInSeconds)
	{
		if (!m_Initialised || !m_Running)
			return;

		C_NetPlayerManager& mgr = *(*m_ppPlayerMgr);

		if (mgr.LocalPrimaryJoinId(true) == 0)
			return; // no disconnect logic until contacted host at least.

		// host should never be null since we migrate
		C_NetPlayer* pHost = mgr.HostPlayer(true);

		if (eJoinStateComplete != JoinGetState())
		{
			if (NULL == pHost || pHost->m_ConsoleId != JoinGetHostConsoleId() || eNetPlayerDisconnectionTypeNone != pHost->m_PartyConnection.m_DisconnectRequested)
			{
				// host has left before we have fully joined.
				LOGINFO(g_Log, "C_NetParty::PollDisconnects() - host left before fully connected to all peers.");
				DisconnectAndClear();
				JoinSetFailed(eJoinErrorNoConnectionToHost, *(*m_ppRouter));
			}
			return;
		}

		// do we need to migrate the host ?
		if (NULL == pHost || !pHost->m_PartyConnection.IsActive() || eNetPlayerDisconnectionTypeNone != pHost->m_PartyConnection.m_DisconnectRequested)
		{
			StartHostMigration();
			pHost = mgr.HostPlayer(true); // get new host player
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
			m_MatchingMigrationState = eMigrationStateNotActive;

			// Check to see if we have disconnected from everybody in the lobby and its more than one peer as this is quite normal.
			for (U8 i=0; i<mgr.TotalSize(); i++)
			{
				C_NetPlayer* p = mgr.Index(i);
				if (p->LocalPlayer() || !p->PrimaryPlayer())
					continue;		

				if (p->m_PartyConnection.IsActive() && eNetPlayerDisconnectionTypeNone != p->m_PartyConnection.m_DisconnectRequested)
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

		bool newDisconnection = false;
		// connection timed out ?
		for (U8 i=0; i<mgr.TotalSize(); i++)
		{
			C_NetPlayer* p = mgr.Index(i);
			if (p->LocalPlayer() || !p->m_PartyConnection.IsActive() || eNetPlayerDisconnectionTypeNone != p->m_PartyConnection.m_DisconnectRequested)
				continue;

			F32 timeOut = NET_PEER_DROPOUT_TIMER;
			if (pHost && pHost->LocalPlayer())
				timeOut = NET_HOST_DROPOUT_TIMER;
			if (p->m_PartyConnection.KeepAliveTimedOut(timeOut))
			{
				LOGINFO(g_Log, "C_NetParty::PollDisconnects() - lost connection to %S", p->m_User.Name());
				newDisconnection = true;
				p->m_PartyConnection.m_DisconnectRequested = eNetPlayerDisconnectionTypeLostConnection;

				playersLostCount++;
			}
		}

		NetConsoleIds toDrop;
		for (U8 i=0; i<mgr.TotalSize(); i++)
		{
			C_NetPlayer* p = mgr.Index(i);
			if (p->LocalPlayer() || !p->PrimaryPlayer())
				continue;		

			if (p->m_PartyConnection.IsActive() && eNetPlayerDisconnectionTypeNone != p->m_PartyConnection.m_DisconnectRequested)
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
					ok = ok && msg.WriteU8(pd->m_PartyConnection.m_DisconnectRequested); // disconnect reason
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
					LOGERROR(g_Log, "C_NetParty::PollDisconnects() - unable to create disconnect consoles message. Hosting.");
				}
			}
			else
			{
				m_DisconnectResendMsgTimer += frameTimeInSeconds;
				if (m_DisconnectResendMsgTimer >= NET_DISCONNECT_RESEND_TIMER || newDisconnection)
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
						ok = ok && msg.WriteU8(pd->m_PartyConnection.m_DisconnectRequested); // disconnect reason
					}
					msg.TerminateMessage();

					if (ok)
					{
						// peer sends a request to the current host.
						if (pHost && eNetPlayerDisconnectionTypeNone == pHost->m_PartyConnection.m_DisconnectRequested)
							pHost->SendReliable(msg, eNetMessage_DisconnectConsoles, true, (*m_ppPlayerMgr)->LocalPrimaryJoinId(true));
					}
					else
					{
						LOGERROR(g_Log, "C_NetParty::PollDisconnects() - unable to create disconnect consoles message. Peer.");
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
	}

	// continue the join procedure if active.
	void C_NetParty::PollJoin(void)
	{
		if (!m_Initialised || !m_Running)
			return;
		if (!IsJoining())
			return;

		// time join out if it looks like we can't connect to either the host or peers..
		float join_timeOut = ((*m_ppRouter)->PeerRoutingEnabled() == true) ? NET_MAX_JOIN_TIME_WITH_PEER_ROUTING : NET_MAX_JOIN_TIME;
		if (JoinTimedOut(join_timeOut))
		{
			LOGINFO(g_Log, "C_NetParty::PollJoin() - unable to connect to game.");
			const eNetJoinSessionErrors errorCode = (JoinGetState() == eJoinStateWaitingForHostReply) ? eJoinErrorNoConnectionToHost : eJoinErrorNoCommunications;
			
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
			if (!(*m_ppPlayerMgr)->AnyPlayersHandshaking(true))
			{
				// put local players into new join complete state..
				for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
				{
					C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
					if (!p->LocalPlayer() || !p->m_PartyConnection.IsActive())
						continue;
					p->m_PartyConnection.m_PlayerState = eNetPlayerStateJoinComplete;
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
						if (p->PrimaryPlayer() && p->m_PartyConnection.IsActive())
						{
							if (p->m_PartyConnection.IsConnected() && !p->m_PartyConnection.m_HandshakingComplete)
							{
								msg.Flush();
								if (BuildJoinRequestMessage(msg, p)) 
									(*m_ppRouter)->SendMessage(msg, p->m_PartyConnection.m_Address);
							}
						}
					}
				}
			}
		}

	}

	void C_NetParty::PollSecurityKeyTracker(void)
	{
		if (!m_Initialised || !m_Running)
			return;
		SecurityKeyIds activeKeyIds;
		C_NetPlayerManager& mgr = **m_ppPlayerMgr;

		for (S32 i=0; i<mgr.TotalSize(); i++)
		{
			const C_NetPlayer* pPlayer = mgr.Index(i);
			if (pPlayer->LocalPlayer() || !pPlayer->PrimaryPlayer())
				continue; // dont check against local players or non remote primary players.

			if (pPlayer->m_PartyConnection.m_Address.isNull())
				continue; // player is not connected on the party channel.

			bool alreadyAdded = false;
			for (S32 z=0; !alreadyAdded && z<activeKeyIds.Size(); z++)
			{
				if (activeKeyIds[z] == pPlayer->m_PartyConnection.m_Address->m_endpoint.getSecID())
					alreadyAdded = true;
			}

			if (!alreadyAdded)
				activeKeyIds.PushBack(pPlayer->m_PartyConnection.m_Address->m_endpoint.getSecID());
		}

		m_pSessionKeyTracker->Poll(activeKeyIds);
	}

	void C_NetParty::PollVoice(void)
	{
		if (!m_Initialised || !m_Running || !m_VoiceEnabled)
			return;

		if (eVoiceControlStatePartyConnection != (*m_ppPlayerMgr)->GetVoiceControlState())
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

				if (rp->PrimaryPlayer() && rp->m_PartyConnection.IsActive() && eNetPlayerDisconnectionTypeNone == rp->m_PartyConnection.m_DisconnectRequested)
				{
					if(rp->m_PartyConnection.m_PlayerState >= eNetPlayerStateIdle)
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

	void C_NetParty::SendReliableMessage(C_NetMessage& msg, const U8 messageId)
	{
		if (!m_Initialised || !m_Running)
			return;

		// put message into the peers message queues.
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (p->LocalPlayer())
				continue;
			if (p->PrimaryPlayer() && p->m_PartyConnection.CanMessage() && eNetPlayerDisconnectionTypeNone == p->m_PartyConnection.m_DisconnectRequested)
				p->SendReliable(msg, messageId, true, (*m_ppPlayerMgr)->LocalPrimaryJoinId(true), false);
		}
	}

	bool C_NetParty::CreateLocalPlayers(C_NetCreationHelper& helper, const bool hostingSession)
	{
		if (helper.m_LocalPlayerData.Size() == 0)
			return false; // no local players defined so can't create.

		// must have already checked for complete connection.
		bdNetImpl *const net = bdNet::getInstance();
		bdCommonAddrRef commonAddress = net->getLocalCommonAddr();
		if (commonAddress.isNull())
			return false;

		for (S32 i=0; i<helper.m_LocalPlayerData.Size(); i++)
		{
			C_NetPlayer* newPlayer = (*m_ppPlayerMgr)->AllocatePlayer(false);
			if (NULL == newPlayer)
				return false;

			// generic 
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

			// party connection specific.
			if (hostingSession)
			{
				newPlayer->m_PartyConnection.m_PlayerState = eNetPlayerStateJoinComplete;
				newPlayer->m_PartyConnection.m_Host = (i == 0) ? true : false;
				newPlayer->m_PartyConnection.m_JoinId = (*m_ppPlayerMgr)->GenerateJoinReference(true);
			}
			else
			{
				newPlayer->m_PartyConnection.m_PlayerState = eNetPlayerStateJoining;
			}

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

		return true;
	}

	void C_NetParty::EnterLobby(void)
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
					(*m_ppPlayerMgr)->SetVoiceControlState(eVoiceControlStatePartyConnection);
					m_VoiceEnabled = true;
					SetupRemoteVoice();

					// update mute info for all remote players based on the data we currently have stored.
					for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
					{
						const C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
						if (p->LocalPlayer())
							continue;

						if (p->m_PartyConnection.IsActive())
							(*m_ppPlayerMgr)->ReceiveMuteList(p->m_PartyConnection.m_JoinId, p->m_PartyConnection.m_VoiceMuteData, true);
					}
				}

				if (eNetPlayerStateIdle != p->m_PartyConnection.m_PlayerState)
				{
					// upon entering lobby player goes back to idle state
					// states updated means we will post the local players new states to connected peers.
					statesUpdated = true;
					p->m_PartyConnection.m_PlayerState = eNetPlayerStateIdle;
				}
			}
		}

		if (statesUpdated)
			SendLocalPlayerStatesMessage();
	}

	void C_NetParty::EnableMatchingQoS(const bool enabled)
	{
		if (!m_Initialised || !m_Running)
			return;
		m_QoSEnabled = enabled;
	}

	void C_NetParty::SetupRemoteVoice(void)
	{
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

	void C_NetParty::SetupQoS(void)
	{
		if (!m_Initialised || !m_Running)
			return;

		if (JoinGetState() != eJoinStateComplete)
			return;

		C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(true);
		if (NULL == pHost || !pHost->LocalPlayer())
			return;

		NetQoSParty q;
		q.m_Version			= NET_PACKET_VERSION;
		q.m_Host			= pHost->m_User;
		q.m_HostConsoleId	= pHost->m_ConsoleId;
		q.m_PackVersion		= S32(		((C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion << 16) & 0xffff0000) 
									|	((C_MultiplayerData::Instance().m_pConfig->m_PackVersion << 0) & 0x0000ffff));
		m_QoS				= q;

		// if QoS already enabled then bail, no need to do it more than once.
		if ((*m_ppPartyMatchmaking)->QosEnabled())
			return;

		LOGINFO(g_Log, "C_NetParty::SetupQoS() - updating server with new qos data.");
		const bool ok = (*m_ppPartyMatchmaking)->EnableQos(16384, (U8*)&m_QoS, sizeof(NetQoSParty));
		if (!ok)
		{
			LOGWARN(g_Log, "C_NetParty::SetupQoS() - failed to set the QoS data to the matching session");
		}
	}

	void C_NetParty::PollMatchingServer(const bool raceSessionRunning)
	{
		if (!m_Initialised || !m_Running)
			return;

		// check matching session for going into an invalid state.
		if (!m_BigError)
		{
			switch ((*m_ppPartyMatchmaking)->SessionState())
			{
			case NetPlay::C_Session::eStateRunning:
			case NetPlay::C_Session::eStatePlaying:
				break;
			default:
				LOGWARN(g_Log, "C_NetParty::PollMatchingServer() - matching session is in invalid state!.");
				m_BigError = true;
				break;
			}
		}

		// are we in the process or re,moving players from the matching session because the host has entered matching ?
		if (ePartyMatchingStatePlayersRemoving == m_MatchingPlayerCommandState)
		{
			if (m_pMatchingRemovePlayersNotification->m_Finished)
			{
				if (m_MatchingRemovePlayersNotificationId == m_pMatchingRemovePlayersNotification->m_Id)
					m_MatchingPlayerCommandState = ePartyMatchingStatePlayersRemoved;
				m_pMatchingRemovePlayersNotification->Reset();
			}
		}
		else
			m_pMatchingRemovePlayersNotification->Reset();

		// we should only update the session once we have fully joined the session if the host leaves before we fully join then the
		// disconnect logic will fail the join attempt.
		if (JoinGetState() == eJoinStateComplete)
		{
			const C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(true);
			if (pHost && pHost->m_PartyConnection.IsActive())
			{
				// should we be adding or removing players from the server ?.
				bool safeToAddPlayersToServer = false;
				if (!raceSessionRunning)
				{
					if (pHost->m_PartyConnection.m_PlayerState == eNetPlayerStateIdle || pHost->m_PartyConnection.m_PlayerState == eNetPlayerStateDisconnecting)
						safeToAddPlayersToServer = true;
				}

				// if we are still deleting our race session then dont add players into this session just yet..
				// this will happen when we disconnect from the race session and make our way back to the party lobby.
				if (NetPlay::C_SessionDeletion::Instance().ActiveCount() > 0)
					safeToAddPlayersToServer = false;

				NetPlay::C_Session::SessionUserList serverList;

				// remove player(s) first.
				for (S32 i=0; i<m_MatchingToRemove.Size(); i++)
				{
					NetPlay::C_Session::SessionUser user;
					user.m_User = m_MatchingToRemove[i];
					user.m_PublicSlot = false;
					serverList.PushBack(user);

					if (serverList.Capacity() == serverList.Size() || serverList.Size() == m_MatchingToRemove.Size())
					{
						bool ok = (*m_ppPartyMatchmaking)->RemovePlayers(serverList);
						if (ok)
						{
							LOGINFO(g_Log, "C_NetParty::PollMatchingServer() - removed %d player(s) from matching session.", serverList.Size());
						}
						else
						{
							m_BigError = true;
							LOGWARN(g_Log, "C_NetParty::PollMatchingServer() - unable to remove %d player(s) from matching session.", serverList.Size());
						}
						serverList.Clear();
					}
				}
				m_MatchingToRemove.Clear();
				serverList.Clear();

				if (safeToAddPlayersToServer)
				{
					// fully joined so see what players need adding to the session. 
					// only players who have completed the join process will be added.
					MatchingPlayersPartyList addList;
					for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
					{
						C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
						if (p->m_PartyConnection.IsActive() && p->m_PartyConnection.m_PlayerState >= eNetPlayerStateJoinComplete && !p->m_PartyConnection.m_AddedToOnlineSession)
						{
							p->m_PartyConnection.m_AddedToOnlineSession = true;
							addList.PushBack(p->m_User);
						}
					}

					for (S32 i=0; i<addList.Size(); i++)
					{
						NetPlay::C_Session::SessionUser user;
						user.m_User = addList[i];
						user.m_PublicSlot = false;
						serverList.PushBack(user);

						if (serverList.Capacity() == serverList.Size() || serverList.Size() == addList.Size())
						{
							bool ok = (*m_ppPartyMatchmaking)->AddPlayers(serverList);
							if (ok)
							{
								LOGINFO(g_Log, "C_NetParty::PollMatchingPlayers() - added %d player(s) to the matching session.", serverList.Size());
							}
							else
							{
								m_BigError = true;
								LOGWARN(g_Log, "C_NetParty::PollMatchingPlayers() - unable to add %d player(s) to the matching session.", serverList.Size());
							}
							serverList.Clear();
						}
					}
					serverList.Clear();
					m_MatchingPlayerCommandState = ePartyMatchingStatePlayersAdded;
				}
				else
				{
					// need to remove any players since host is no longer in the lobby..
					bool newCommand = false;
					MatchingPlayersPartyList removeList;
					for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
					{
						C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
						if (p->m_PartyConnection.m_AddedToOnlineSession)
						{
							p->m_PartyConnection.m_AddedToOnlineSession = false;
							removeList.PushBack(p->m_User);
						}
					}

					for (S32 i=0; i<removeList.Size(); i++)
					{
						NetPlay::C_Session::SessionUser user;
						user.m_User = removeList[i];
						user.m_PublicSlot = false;
						serverList.PushBack(user);

						if (serverList.Capacity() == serverList.Size() || serverList.Size() == removeList.Size())
						{
							if (++m_MatchingRemovePlayersNotificationId == 0)
								m_MatchingRemovePlayersNotificationId = 1; // needs to be anything but 0.

							m_pMatchingRemovePlayersNotification->Reset(m_MatchingRemovePlayersNotificationId);

							bool ok = (*m_ppPartyMatchmaking)->RemovePlayers(serverList, m_pMatchingRemovePlayersNotification);
							if (ok)
							{
								newCommand = true;
								LOGINFO(g_Log, "C_NetParty::PollMatchingServer() - removed %d player(s) from matching session. host busy", serverList.Size());
							}
							else
							{
								m_BigError = true;
								LOGWARN(g_Log, "C_NetParty::PollMatchingServer() - unable to remove %d player(s) from matching session. host busy", serverList.Size());
							}
							serverList.Clear();
						}
					}
					serverList.Clear();

					// if we have issued a new server command then we need to wait until completion.
					if (newCommand)
						m_MatchingPlayerCommandState = ePartyMatchingStatePlayersRemoving;
					else if (ePartyMatchingStatePlayersAdded == m_MatchingPlayerCommandState)
						m_MatchingPlayerCommandState = ePartyMatchingStatePlayersRemoved;
				}

				// handle matching QoS
				if (m_QoSEnabled)
				{
					if (safeToAddPlayersToServer && !m_Migrating)
						SetupQoS();
				}
				else
				{
					if ((*m_ppPartyMatchmaking)->QosEnabled())
					{
						LOGINFO(g_Log, "C_NetParty::PollMatchingServer() - QoS listener disabled.");
						(*m_ppPartyMatchmaking)->DisableQos();
					}
				}
			}
		}
	}

	void C_NetParty::DestroyAllPlayers(void)
	{
		NetJoinReferences toDestroy;

		// destroy remote players first
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (!p->LocalPlayer() && p->m_PartyConnection.IsActive())
				toDestroy.PushBack(p->m_PartyConnection.m_JoinId);
		}

		// destroy local players last
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (p->LocalPlayer() && p->m_PartyConnection.IsActive())
				toDestroy.PushBack(p->m_PartyConnection.m_JoinId);
		}

		for (S32 i=0; i<toDestroy.Size(); i++)
			DestroyPlayer(toDestroy[i]);
	}

	void C_NetParty::DestroyPlayer(const U8 joinId, const bool showNotification)
	{
		C_NetPlayer* p = (*m_ppPlayerMgr)->FindFromJoinId(joinId, true);
		if (NULL == p)
			return;

		// if applicable remove player from the matching session.
		if (p->m_PartyConnection.m_AddedToOnlineSession)
			RemoveMatchingPlayer(p->m_User);

		// only give a player notification if we have called EnterLobby()
		if (!p->LocalPlayer() && m_VoiceEnabled)
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->PlayerNotification(p->m_User.Name(), false, true);

		// disconenct from party session.
		// only remote primary players need their address unregistering.
		p->m_PartyConnection.Disconnect(!p->LocalPlayer() && p->PrimaryPlayer());

		// clear party data
		p->m_PartyData.Flush();

		// if no race connection active totally destroy this network player.
		// otherise leave the players race connection active for the race manager to delete.
		if (!p->m_RaceConnection.IsActive())
		{
			// unregister voice since we have no race..
			if (p->LocalPlayer())
			{
				Communicator::C_Core::Instance().UnRegisterLocalTalker(p->m_ControllerPort);

				// local primary player leaving party with no race means voice mode goes to default.
				if (p->PrimaryPlayer())
					(*m_ppPlayerMgr)->SetVoiceControlState(eVoiceControlStateNone);
			}
			else
				Communicator::C_Core::Instance().UnRegisterRemoteTalker(p->m_User);

			(*m_ppPlayerMgr)->DeallocatePlayer(p);
		}
		else
		{
			// remove any party channel reliable messages from this players message queue since they can no longer be sent or acknowledged.
			// this will allow any messages queued up on the race channel to be sent correctly.
			p->RemovePendingReliableMessages(true);
		}
	}

	void C_NetParty::RemoveMatchingPlayer(const SysProfile::UserIdentifier& plr)
	{
		if (!m_Initialised || !m_Running)
			return;
		m_MatchingToRemove.PushBack(plr);
	}

	void C_NetParty::StartHostMigration(void)
	{
		NetConsoleIds consoleIds;
		U8 lowestJoinId = 255;
		C_NetPlayerManager& mgr = *(*m_ppPlayerMgr);
		for (U8 i=0; i<mgr.TotalSize(); i++)
		{
			C_NetPlayer* p = mgr.Index(i);
			if (p->m_PartyConnection.IsActive())
			{
				if (p->PrimaryPlayer())
					consoleIds.PushBack(p->m_ConsoleId);

				if (eNetPlayerDisconnectionTypeNone == p->m_PartyConnection.m_DisconnectRequested)
				{
					if (255 == lowestJoinId || p->m_PartyConnection.m_JoinId < lowestJoinId)
						lowestJoinId = p->m_PartyConnection.m_JoinId;
				}
			}
		}

		C_NetPlayer* pNewHost = mgr.FindFromJoinId(lowestJoinId, true);
		if (NULL == pNewHost)
			return;

		if (!pNewHost->LocalPlayer())
			return; // a remote console will migrate to the new host.

		LOGINFO(g_Log, "C_NetParty::StartHostMigration()");

		mgr.PromoteToHost(pNewHost->m_PartyConnection.m_JoinId, true);

		// send message to peers so they can promote me to host.
		C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
		bool ok = msg.CreateReliable();

		// new host data
		ok = ok && msg.WriteU64(pNewHost->m_ConsoleId);
		ok = ok && msg.WriteU8(pNewHost->m_LocalIndex);

		// list of known console ids on the party channel.
		ok = ok && msg.WriteU8((U8)consoleIds.Size());
		for (S32 i=0; i<consoleIds.Size(); i++)
			ok = ok && msg.WriteU64(consoleIds[i]);

		msg.TerminateMessage();

		if (ok)
			SendReliableMessage(msg, eNetMessage_Migrate);
		else
		{
			LOGERROR(g_Log, "C_NetParty::StartHostMigration() - unable to create migrate message.");
		}

		m_Migrating = true;
		m_MatchingMigrationState = eMigrationStateHostStart;
	}

	bool C_NetParty::ReceiveMigrateMessage(C_NetMessage& msg, const U8 senderId)
	{
		LOGINFO(g_Log, "C_NetParty::ReceiveMigrateMessage()");

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
			if (p && p->m_PartyConnection.IsActive())
			{
				(*m_ppPlayerMgr)->PromoteToHost(p->m_PartyConnection.m_JoinId, true);

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

			// disconnect any consoles that we know about that are not in the list supplied by the new host
			if (ok)
			{
				for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
				{
					C_NetPlayer* pc = (*m_ppPlayerMgr)->Index(i);
					if (pc->LocalPlayer() || !pc->PrimaryPlayer() || !pc->m_PartyConnection.IsActive())
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
					DestroyConsole(toDestroy[z]);
			}
		}

		// need to force an update so players may need adding / removing from the matching session if it's running.
		PollMatchingServer(false);

		msg.ReadClose();

		if (ok)
		{
			m_Migrating = true;
			m_MatchingMigrationTimer = 0.0f;
			m_MatchingMigrationState = eMigrationStatePeerWaitingForHost; // peer waits until host tells him to start matching session migration
		}
		else
		{
			LOGWARN(g_Log, "C_NetParty::ReceiveMigrateMessage() - malformed data received.");
		}

		return true;
	}

	bool C_NetParty::PollMatchingServerMigration(const F32 frameTimeInSeconds)
	{
		if (!m_Initialised || !m_Running || !m_Migrating)
			return false;

		if (eMigrationStateHostStart == m_MatchingMigrationState)
		{
			if (++m_MatchingMigrationNotificationId == 0)
				m_MatchingMigrationNotificationId = 1; // needs to be anything but 0.

			// setup our notification with unique id.
			m_pMatchingMigrationNotification->Reset(m_MatchingMigrationNotificationId);

			const U8 numPlayers = (*m_ppPlayerMgr)->Size(true);
			BIZ_ASSERT(numPlayers != 0);

			RacerNet::NetMatchingServerData matchingServerCreationData;
			matchingServerCreationData.m_MatchingMode		= eNetMatchingModeNormal;
			matchingServerCreationData.m_RankRestriction	= 0;
			matchingServerCreationData.m_MatchingRegion		= 0;
			matchingServerCreationData.m_PackVersion		= S32(		((C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion << 16) & 0xffff0000) 
																	|	((C_MultiplayerData::Instance().m_pConfig->m_PackVersion << 0) & 0x0000ffff));
			matchingServerCreationData.m_EventId			= 0;
			matchingServerCreationData.m_CustomRace			= 0;
			matchingServerCreationData.m_OpenSlotCount		= 0;
			matchingServerCreationData.m_Searchable			= 0;
			matchingServerCreationData.m_InRace				= 0;
			matchingServerCreationData.m_Nat				= (*m_ppRouter)->PeerRoutingEnabled() ? (*m_ppRouter)->GetNatType() : (*m_ppPlayerMgr)->GetMostRestrictiveNatType(true);

			// build platform specific matching server parameters list.
			RacerNet::NetMatchingParameterList matchingServerParamList;
			RacerNet::BuildMatchingParameters(matchingServerCreationData, matchingServerParamList, false, true);

			NetPlay::C_Session::SlotConfig matchSlotConfig;
			matchSlotConfig.m_MaxPublic		= 0; 
			matchSlotConfig.m_PublicInUse	= 0; 
			matchSlotConfig.m_MaxPrivate	= NET_MAX_PARTY_MEMBERS; 
			matchSlotConfig.m_PrivateInUse	= numPlayers;

			const bool ok = (*m_ppPartyMatchmaking)->Migrate(m_pMatchingMigrationNotification, &matchingServerParamList[0], matchingServerParamList.Size(), matchSlotConfig);
			if (ok)
			{
				LOGINFO(g_Log, "C_NetParty::PollMatchingServerMigration() - Host : started matching migration.");
				m_MatchingMigrationState = eMigrationStateHostContinue;
			}
			else
			{
				LOGWARN(g_Log, "C_NetParty::PollMatchingServerMigration() - unable to start matching server migration.");
				m_Migrating = false;
				m_BigError = true; // we should delete the session asap.
			}
		}
		else if (eMigrationStateHostContinue == m_MatchingMigrationState)
		{
			if (m_pMatchingMigrationNotification->m_Finished)
			{
				if (m_MatchingMigrationNotificationId == m_pMatchingMigrationNotification->m_Id)
				{
					LOGINFO(g_Log, "C_NetParty::PollMatchingServerMigration() - Host : matching migration complete, informing peers.");

					// get the new security keys from the migrated session.
					NetPlay::SessionSecurity newSec;
					if ((*m_ppPartyMatchmaking)->GetSecurityInfo(newSec))
					{
						// update our key tracker so it knows about the new new session key.
						m_pSessionKeyTracker->Add(newSec);

						// put peers in migration state.
						(*m_ppPlayerMgr)->SetPeersToMigrationPhase(true);

						// Send msg to peers so they start their matching migration process.. 
						SendMigrateMatchingSessionMessage();

						m_MatchingMigrationState = eMigrationStateHostWaitForPeers;
						m_MatchingMigrationTimer = 0.0f;
					}
					else
					{
						LOGWARN(g_Log, "C_NetParty::PollMatchingServerMigration() - Host : unable to get security data from migrated session");
						m_Migrating = false;
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
			if (!(*m_ppPlayerMgr)->AnyPeersStillMigrating(true) || timeOut)
			{
				// if timed out then flag any peers that didn't respond to be dropped.
				if (timeOut)
					(*m_ppPlayerMgr)->PeerMigrationTimeout(true);

				// when using demonware we need to add players into the session again since they won't be migrated over like on xbox360
				for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
				{
					C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
					if (p->m_PartyConnection.IsActive())
						p->m_PartyConnection.m_AddedToOnlineSession = false;
				}
				m_MatchingToRemove.Clear(); // sanity.. no point removing players now as we have just created a new session
				LOGINFO(g_Log, "C_NetParty::PollMatchingServerMigration() - Host : process complete.");
				m_Migrating = false;
			}
		}
		else if (eMigrationStatePeerWaitingForHost == m_MatchingMigrationState)
		{
			m_MatchingMigrationTimer += frameTimeInSeconds;
			const bool timeOut = m_MatchingMigrationTimer >= 10.0f ? true : false;
			if (timeOut)
			{
				LOGWARN(g_Log, "C_NetParty::PollMatchingServerMigration() - Peer waited too long for host to start matching migration.");
				m_Migrating = false;
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
						if (p->m_PartyConnection.IsActive())
							p->m_PartyConnection.m_AddedToOnlineSession = false;
					}
					m_MatchingToRemove.Clear(); // sanity.. no point removing players now as we have just created a new session
					// get the new security keys from the migrated session.
					NetPlay::SessionSecurity newSec;
					if ((*m_ppPartyMatchmaking)->GetSecurityInfo(newSec))
					{
						LOGINFO(g_Log, "C_NetParty::PollMatchingServerMigration() - Peer : matching migration complete, informing host.");

						// update our key tracker so it knows about the new new session key.
						m_pSessionKeyTracker->Add(newSec);

						// msg host to say our migration is complete.
						SendMigrateMatchingSessionReplyMessage();
					}
					else
					{
						LOGWARN(g_Log, "C_NetParty::PollMatchingServerMigration() - Peer : unable to get security data from migrated session.");
						m_BigError = true;
					}

					m_Migrating = false;
				}
				else
					m_pMatchingMigrationNotification->Reset(0); // old notification so reset and wait for correct one.
			}
		}
		else
		{
			BIZ_ASSERT_FAIL("C_NetParty::PollMatchingServerMigration() - unknown migration state.");
		}
		return m_Migrating; 
	}

	void C_NetParty::DestroyConsole(const U64& consoleId)
	{
		NetJoinReferencesSingleConsole toDestroy;
		for (U8 i=0; i<(*m_ppPlayerMgr)->TotalSize(); i++)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->Index(i);
			if (p->m_PartyConnection.IsActive() && consoleId == p->m_ConsoleId)
				toDestroy.PushBack(p->m_PartyConnection.m_JoinId);
		}

		for (S32 i=0; i<toDestroy.Size(); i++)
			DestroyPlayer(toDestroy[i], true);
	}

	bool C_NetParty::ReceiveLeavingMessage(C_NetMessage& msg, const U8 senderId)
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
				if (consoleId == p->m_ConsoleId && p->m_PartyConnection.IsActive())
				{
					if (eNetPlayerDisconnectionTypeNone == p->m_PartyConnection.m_DisconnectRequested)
						p->m_PartyConnection.m_DisconnectRequested = eNetPlayerDisconnectionTypeNormal;	// players are leaving the race the normal way.
				}
			}
		}

		if (!ok)
		{
			LOGWARN(g_Log, "C_NetParty::ReceiveLeavingMessage() - malformed data received.");
		}

		return true;
	}

	bool C_NetParty::ReceiveMuteChangeMessage(C_NetMessage& msg, const U8 senderId)
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
					C_NetPlayer* plr = (*m_ppPlayerMgr)->FindFromJoinId(joinId, true);
					if (plr)
						plr->m_PartyConnection.m_VoiceMuteData = muteList; 

					(*m_ppPlayerMgr)->ReceiveMuteList(joinId, muteList, true);
				}
			}
			msg.ReadClose();
		}

		if (!ok)
		{
			LOGWARN(g_Log, "C_NetParty::ReceiveMuteChangeMessage() - malformed data received.");
		}
		return true;
	}

	bool C_NetParty::ReceivePlayerStatesMessage(C_NetMessage& msg, const U8 senderId)
	{
		// this simple message can be parsed via the player manager since all the data is going in there anyway.
		(*m_ppPlayerMgr)->ReceivePlayerStatesMessage(msg, senderId, true);
		return true;
	}

	bool C_NetParty::ReceiveDisconnectConsolesMessage(C_NetMessage& msg, const U8 senderId)
	{
		C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(true);

		LOGDEBUG(g_Log, "C_NetParty::ReceiveDisconnectConsolesMessage()");

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
								if (p->m_PartyConnection.IsActive())
									p->m_PartyConnection.m_DisconnectRequested = static_cast<eNetPlayerDisconnectionType> (reason);
							}
						}

						if (!found)
							unknownConsoleIds.PushBack(console);
					}
					else
					{
						// peers just do as requested and destroy the specified console players.
						DestroyConsole(console);	
					}
				}
			}

			// host has the option of replying to the peer if the peer has requested we disconnect a console that we know nothing about.
			// when the peer receives this reply they will disconnect from that console. maintains session consistency.
			if (unknownConsoleIds.Size() > 0)
			{
				C_NetPlayer* pSender = (*m_ppPlayerMgr)->FindFromJoinId(senderId, true);
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
						pSender->SendReliable(reply, eNetMessage_DisconnectConsoles, true, (*m_ppPlayerMgr)->LocalPrimaryJoinId(true));
					else
					{
						LOGWARN(g_Log, "C_NetParty::ReceiveDisconnectConsolesMessage() - unable to reply to sender as unknown consoles exist.");
					}
				}
			}
		}

		if (!ok)
		{
			LOGWARN(g_Log, "C_NetParty::ReceiveDisconnectConsolesMessage() - malformed data received.");
		}
		return true;
	}

	bool C_NetParty::ReceiveJoinDataMessage(C_NetMessage& msg, const U8 senderId)
	{
		if (!m_Running)
			return false;
		BIZ_ASSERT(JoinGetState() == eJoinStateWaitingForPeerData);

		U8 numPeersToJoin = 0;
		bool ok = msg.ReadOpen();
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

			bdNetImpl *const net = bdNet::getInstance();
			bdCommonAddrRef myCommonAddress = net->getLocalCommonAddr();
			if (myCommonAddress.isNull())
				ok = false;

			if (ok)
			{
				C_NetPlayer *p = (*m_ppPlayerMgr)->FindFromConsoleId(consoleId, localIndex);
				if (NULL == p)
					p = (*m_ppPlayerMgr)->AllocatePlayer(true);
				
				if (p)
				{
					p->m_User			= bs;
					p->m_ConsoleId		= consoleId;
					p->m_LocalIndex		= localIndex;
					p->m_DlcMask		= dlcMask;
					p->m_VoiceBanned	= voiceBanned != 0 ? true : false;
					p->m_ProfileData	= profile;
					p->m_UpStreamKbps	= upBandwidth;
					p->m_DownStreamKbps = downBandwidth;

					p->m_PartyConnection.m_PlayerState = static_cast <eNetPlayerState> (state);
					p->m_PartyConnection.m_JoinId = joinId;

					for (U32 z=0; z<BD_COMMON_ADDR_SERIALIZED_SIZE; z++)
						p->m_CommonAddress[z] = commonAddr[z];	

					// if primary remote player then attempt to start connection process..
					if (p->PrimaryPlayer())
					{
						BIZ_ASSERT(p->m_PartyConnection.m_Address.isNull());
						bdCommonAddrRef addr = new bdCommonAddr();
						if (addr->deserialize(myCommonAddress, p->m_CommonAddress))
						{
							NetPlay::SessionSecurity security;
							ok = m_pSessionKeyTracker->Current(security);

							if (ok)
							{
								if (NULL != net->getSocketRouter() && net->getSocketRouter()->getAddressMap()->getAddrHandle(addr, security.m_Id, p->m_PartyConnection.m_Address))
									ok = net->getSocketRouter()->connect(p->m_PartyConnection.m_Address);
								else 
								{
									ok = false;
									addr = BD_NULL;
								}
							}
							else
								addr = BD_NULL;
						}
						else
						{
							ok = false;
							addr = BD_NULL;
						}
					}

					p->m_PartyConnection.StartKeepAliveTimer();
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

	void C_NetParty::SendInitialDataMessage(C_NetPlayer* pRecipient)
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
			BIZ_ASSERT(p->m_PartyConnection.m_JoinId > 0);

			// re check communicator status
			if (!p->m_VoiceBanned)
				p->m_HasCommunicator = Communicator::C_Core::Instance().CommunicatorConnected(p->m_ControllerPort);

			ok = ok && msg.WriteU8(p->m_PartyConnection.m_JoinId);	
			ok = ok && msg.WriteU8(p->m_PartyConnection.m_PlayerState);
			ok = ok && msg.WriteBuffer(&(p->m_ProfileData), sizeof(p->m_ProfileData));

			NetMuteListJoinReferences muteList;
			(*m_ppPlayerMgr)->BuildMuteList(p->m_LocalIndex, muteList, true);
			ok = ok && msg.WriteBuffer(&muteList, sizeof(muteList));

			ok = ok && msg.WriteU8(p->m_HasCommunicator ? 1 : 0);

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
				if (pRecipient->PrimaryPlayer() && pRecipient->m_PartyConnection.CanMessage())
					pRecipient->SendReliable(msg, eNetMessage_InitialData, true, (*m_ppPlayerMgr)->LocalPrimaryJoinId(true));
			}
		}
		else
		{
			LOGERROR(g_Log, "C_NetParty::SendInitialDataMessage() - failed to send initial data message.");
		}
	}

	bool C_NetParty::ReceiveInitialDataMessage(C_NetMessage& msg, const U8 senderId)
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

						ok = ok && msg.ReadU8(joinId);
						ok = ok && msg.ReadU8(playerState);
						ok = ok && msg.ReadBuffer(&profile, sizeof(profile));
						ok = ok && msg.ReadBuffer(&muteList, sizeof(muteList));
						ok = ok && msg.ReadU8(hasCommunicator);

						if (ok)
						{
							C_NetPlayer* p = (*m_ppPlayerMgr)->FindFromJoinId(joinId, true);
							if (p)
							{
								p->m_PartyConnection.m_PlayerState = static_cast<eNetPlayerState> (playerState);
								p->m_PartyConnection.m_VoiceMuteData = muteList;
								p->m_ProfileData = profile;
								p->m_HasCommunicator = (hasCommunicator == 1) ? true : false;
								(*m_ppPlayerMgr)->ReceiveMuteList(joinId, muteList, true);
							}
						}
					}
				}
			}

			if (ok && request > 0)
			{
				if (JoinGetState() == eJoinStateComplete)
				{
					C_NetPlayer* p = (*m_ppPlayerMgr)->FindFromJoinId(senderId, true);
					if (p)
						SendInitialDataMessage(p); // reply straight back to the sender with our initial data. 
				}
			}
		}

		if (!ok)
		{
			LOGERROR(g_Log, "C_NetParty::ReceiveInitialDataMessage() - failed to read initial data message.");
		}

		msg.ReadClose();
		return true;
	}

	void C_NetParty::SendMigrateMatchingSessionMessage(void)
	{
		if (!m_Running)
			return;

		BIZ_ASSERT(eMigrationStateHostContinue == m_MatchingMigrationState);
		BIZ_ASSERT(m_Migrating);

		// msg peers with new security info so they can migrate their session also.
		NetPlay::MigrationInfo migrationInfo;
		if ((*m_ppPartyMatchmaking)->GetMigrationInfo(migrationInfo))
		{
			C_NetMessage& msg = (*m_ppRouter)->GetOutgoingReliable();
			bool ok = msg.CreateReliable();

			ok = ok && msg.WriteBuffer(&migrationInfo, sizeof(migrationInfo));
			msg.TerminateMessage();
			if (ok)
				SendReliableMessage(msg, eNetMessage_MigrateMatchingSession);
			else
			{
				LOGWARN(g_Log, "C_NetParty::SendMigrateMatchingSessionMessage() - unable to create message.");
			}
		}
		else
		{
			LOGWARN(g_Log, "C_NetParty::SendMigrateMatchingSesionMessage() - unable to get migration data from matching session.");
		}
	}

	bool C_NetParty::ReceiveMigrateMatchingSessionMessage(C_NetMessage& msg, const U8 senderId)
	{
		if (!m_Running)
			return false;

		if (eMigrationStatePeerWaitingForHost == m_MatchingMigrationState && m_Migrating)
		{
			const C_NetPlayer* pHost = (*m_ppPlayerMgr)->HostPlayer(true);
			if (pHost && senderId == pHost->m_PartyConnection.m_JoinId)
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

					const U8 numPlayers = (*m_ppPlayerMgr)->Size(true);

					NetPlay::C_Session::SlotConfig matchSlotConfig;
					matchSlotConfig.m_MaxPublic		= 0;
					matchSlotConfig.m_PublicInUse	= 0;
					matchSlotConfig.m_MaxPrivate	= NET_MAX_PARTY_MEMBERS;
					matchSlotConfig.m_PrivateInUse	= numPlayers;

					const bool ok = (*m_ppPartyMatchmaking)->MigratePeer(migrationInfo, m_pMatchingMigrationNotification, matchSlotConfig);
					if (ok)
					{
						LOGINFO(g_Log, "C_NetParty::ReceiveMigrateMatchingSessionMessage() - Peer : started matching migration.");
						m_MatchingMigrationState = eMigrationStatePeerContinue;
					}
					else
					{
						LOGWARN(g_Log, "C_NetParty::ReceiveMigrateMatchingSessionMessage() - unable to start matching server migration.");
						m_Migrating = false;
						m_MatchingMigrationState = eMigrationStateNotActive;
						m_BigError = true; // we should delete the session asap.
					}
				}
				else
				{
					LOGWARN(g_Log, "C_NetParty::ReceiveMigrateMatchingSessionMessage() - malformed msg received.");
				}
			}
			else
			{
				LOGWARN(g_Log, "C_NetParty::ReceiveMigrateMatchingSessionMessage() - received from not the current host, ignoring.");
			}
		}

		return true;
	}

	void C_NetParty::SendMigrateMatchingSessionReplyMessage(void)
	{
		if (!m_Running)
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
				LOGWARN(g_Log, "C_NetParty::SendMigrateMatchingSessionReplyMessage() - unable to create message.");
			}
		}
	}

	bool C_NetParty::ReceiveMigrateMatchingSessionReplyMessage(C_NetMessage& msg, const U8 senderId)
	{
		if (!m_Running)
			return false;

		if (eMigrationStateHostWaitForPeers == m_MatchingMigrationState && m_Migrating)
			(*m_ppPlayerMgr)->SetPeerMigrationComplete(senderId, true);
		return true;
	}

	bool C_NetParty::BuildJoinRequestMessage(C_NetMessage& msg, C_NetPlayer* pRecipient)
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
		msg.CreateUnReliable(eNetMessage_JoinRequest, (*m_ppPlayerMgr)->LocalPrimaryJoinId(true), true);

		// pack network transport version.
		bool ok = msg.WriteU32(NET_PACKET_VERSION);

		// pack join id of the recipient who we will be sending this join request to.
		// we need this for the following fringe case. 
		// 2 players join at the same time, because of latency player 2 starts peer negotiations
		// before player 1 has even received the initial join acknowledgement from the host. 
		// for player 1 to accept the join request of player 2 he will need his own join id 
		// (which would normaly come in the join reply from the host).
		ok = ok && msg.WriteU8(pRecipient != NULL ? pRecipient->m_PartyConnection.m_JoinId : 0);

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

			// specific party connection data.
			ok = ok && msg.WriteU8(p->m_PartyConnection.m_JoinId); // pack join id.
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

	bool C_NetParty::ReceivePeerJoinRaceSessionMessage(C_NetMessage& msg, const U8 senderId)
	{
		NetJoinRaceFromPartyData data;
		U8 hostJoinId = 0;

		bool ok = msg.ReadOpen();
		ok = ok && msg.ReadU8(hostJoinId);
		ok = ok && msg.ReadBuffer(&data, sizeof(data));

		// Read nonce 
		NetPlay::Nonce playerNonce;
		for ( U16 i = 0; i < playerNonce.Size(); i++ )
			ok = ok && msg.ReadU8( playerNonce.m_Nonce[i] );

		msg.ReadClose();

		if (ok)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->FindFromJoinId(hostJoinId, true);
			if (NULL == p || !p->m_PartyConnection.IsActive() || !p->m_PartyConnection.m_Host)
			{
				LOGWARN(g_Log, "C_NetParty::ReceivePeerJoinRaceSessionMessage() - received fron non party host !");
				return true;
			}
			else
				SetJoinRaceFromPartyData(data);

			// Set my nonce so its the same as the party hosts
			for ( S8 i = 0; i < (*m_ppPlayerMgr)->TotalSize(); i++ )
			{
				C_NetPlayer* pPlayer = (*m_ppPlayerMgr)->Index( i );
				if ( pPlayer->LocalPlayer() )
				{
					LOGINFO( g_Log, "C_NetParty::ReceivePeerJoinRaceSessionMessage() - Player %S party id %d %d %d %d %d %d %d %d", 
								 pPlayer->m_User.m_Name, pPlayer->m_PlayerNonce.m_Nonce[0], pPlayer->m_PlayerNonce.m_Nonce[1], pPlayer->m_PlayerNonce.m_Nonce[2], pPlayer->m_PlayerNonce.m_Nonce[3],
								 pPlayer->m_PlayerNonce.m_Nonce[4], pPlayer->m_PlayerNonce.m_Nonce[5], pPlayer->m_PlayerNonce.m_Nonce[6], pPlayer->m_PlayerNonce.m_Nonce[7] );

					pPlayer->m_PlayerNonce = playerNonce;
					break;
				}
			}
		}
		else
		{
			LOGWARN(g_Log, "C_NetParty::ReceivePeerJoinRaceSessionMessage() - malformed msg received.");
		}
		return true;
	}

	bool C_NetParty::ReceivePeerReturnToLobbyMessage(C_NetMessage& msg, const U8 senderId)
	{
		U8 hostJoinId = 0;
		bool ok = msg.ReadOpen();
		ok = ok && msg.ReadU8(hostJoinId);
		msg.ReadClose();
		
		if (ok)
		{
			C_NetPlayer* p = (*m_ppPlayerMgr)->FindFromJoinId(hostJoinId, true);
			if (NULL == p || !p->m_PartyConnection.IsActive() || !p->m_PartyConnection.m_Host)
			{
				LOGWARN(g_Log, "C_NetParty::ReceivePeerReturnToLobbyMessage() - received fron non party host !");
				return true;
			}
			else
			{
				LOGINFO(g_Log, "C_NetParty::ReceivePeerReturnToLobbyMessage() - host wants us to return to party lobby");
				m_ReturnToLobby = true;

				// if local party members are currently in a race session then flag that they should not migrate to host since they
				// will be getting dragged out of it asap.
				for (U8 i=0; i<(*m_ppPlayerMgr)->LocalSize(); i++)
				{
					C_NetPlayer* pl = (*m_ppPlayerMgr)->Index(i);
					BIZ_ASSERT(pl->LocalPlayer());
					if (pl->m_RaceConnection.IsActive())
						pl->m_RaceConnection.m_IgnoreMigration = true;
				}
			}
		}
		else
		{
			LOGWARN(g_Log, "C_NetParty::ReceivePeerReturnToLobbyMessage() - malformed msg received.");
		}
		return true;
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
