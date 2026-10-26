#include <stdafx.h>
#include <compression.h>
#include <Core/core.h>
#include <system/debug/BuildStamp.h>
#include "../Misc/NetVersion.h"
#include "../Misc/NetDebug.h"
#include "../Misc/NetDefines.h"
#include "../Misc/NetJoinErrors.h"
#include "NetServices.h"
#include <network/Core.h>
#include <network/SessionDeletion.h>
#include <Network/ScoreboardReaderByRank.h>
#include <Network/ScoreboardReaderByUsers.h>
#include <Network/ScoreboardWriter.h>
#include <Network/PlayerSkillWriter.h>
#include <Network/CommunicatorManager.h>
#include <Network/Session.h>
#include <Network/SessionEnumerator.h>
#include <SystemProfile/GameInvite.h>
#include <SystemProfile/SystemProfileManager.h>
#include <lua_shared/LuaHelper.h>
#include <GameLogic/GameData/LocData.h>
#include <GameLogic/GameData/MultiplayerData.h>
#include <ui/Game/UILocaliser.h>
#include <ui/Game/Profiles/ProfileInterface.h>
#include "../Misc/NetDatagramHeader.h"
#include "../Misc/NetDatagramPacketTypes.h"
#include "../Core/NetTimer.h"
#include "../Core/NetCreationHelper.h"
#include "../Core/NetRichPresence.h"
#include "../Player/NetPlayerManager.h"
#include "NetRace.h"
#include "NetParty.h"
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>
#include <RaceBook/RaceBookManager.h>
#include <PlatformCallbacks/PlatformCallbacks.h>
#include <GameApp.h>


#if defined (PLATFORM_PS3)
#include "../Router/NetRouter_ps3.h"
#elif defined (PLATFORM_PC)
#include "../Router/NetRouter_pc.h"
#else
#error ("*** ERROR: Unknown build configuration within NetService.cpp")
#endif

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

#if !defined(CFG_GOLD)// && !defined(CFG_HACKED_MASTER)
	Debug::Bool g_DebugUseTitleStorage( "Multiplayer.TitleStorage.UseTitleStorage_Persist", false, Debug::Persist() );
	Debug::Bool g_debugEnableMultiplayerPakDownload("Multiplayer.TitleStorage.EnableDownloadOfMultiplayerPak", false);
#else 
	bool g_DebugUseTitleStorage=true;
	bool g_debugEnableMultiplayerPakDownload=true;
#endif

namespace RacerNet
{
	static const char	*emptyString = "\0";
	static U32			reqId = 0;

	// static data.
	C_NetServices C_NetServices::s_NetServices;
	static const F32 net30fps = 1.0f/30.0f; 

	static S32 sHackScoreboardReadState = 0;

	C_NetServices::C_NetServices() :	m_pLock(NULL), m_Initialised(false), m_pRouter(NULL), m_pPlayerManager(NULL), m_pScoreboardReaderByRank(NULL), 
										m_pScoreboardReaderByUsers(NULL), m_pScoreboardWriter(NULL), m_pPlayerSkillWriter(NULL), m_pSessionEnumerator(NULL), 
										m_pNetRace(NULL), m_pNetParty(NULL), m_pFrameTimer(NULL), m_pNetCreationHelper(NULL), 
										m_MuteUpdateDelay(0.0f), m_CommunicatorInsertedDelay(0.0f), m_TotalRecentPlayersToAdd(0), m_RemainingRecentPlayersToAdd(0), m_RecentPlayersToAddStartOffset(0),
										m_CanAddMoreRecentPlayers(true), m_SetGroupTaskActive(false), m_GroupCountRetrieved(false), m_GroupCountTaskActive(false), m_HaveGroupCounts(false),
										m_TitleStorage(eTitleStorageWaitingForInit), m_pReader(NULL), m_pTSBuffer(NULL), m_MatchingByRequiredRegion(true), m_MatchingRegion(eMatchingRegionUnknown),
										m_ServerTimeStampTaskActive( false ),m_ServerTimeTaskFailed(false),m_JoinSessionsInRace(true)
	{
		for (S32 i=0; i<eMatchmakingSessionMax; i++)
			m_pMatchmakingSessions[i] = NULL;
	}

	C_NetServices::~C_NetServices()
	{
		TitleStorageStopAndCleanup();
	}

	bool C_NetServices::Initialise(void)
	{
		if (m_Initialised)
			return true; 

		bool ok = false;

		if (NetPlay::C_Core::Instance().IsInitialised())
		{
			ok = NetPlay::C_SessionDeletion::Instance().Initialise(10);

			ok = ok && CreateOnlineSessions();
			ok = ok && CreateScoreboardWriter();
			ok = ok && CreateScoreboardReaders();
			ok = ok && CreateRouter();
			ok = ok && CreatePlayerManager();
			ok = ok && CreateParty();
			ok = ok && CreateRace();
			ok = ok && CreateMisc();
		
			if (ok)
			{
#if !defined (CFG_GOLD)
				// we want to hear about all warnings and errors.
				m_dwLog.addChannel("warn");
				m_dwLog.addChannel("err");
				//m_dwLog.addChannel("info");
#endif
				m_Initialised = true;
			}
			else
				Shutdown();
		}

		return m_Initialised;
	}

	void C_NetServices::Shutdown(void)
	{
		KillGroupCountTask();
		KillSetGroupTask();
			
		if (m_pNetRace)
		{
			m_pNetRace->Shutdown();
			delete m_pNetRace;
		}
		m_pNetRace = NULL;

		if (m_pNetParty)
		{
			m_pNetParty->Shutdown();
			delete m_pNetParty;
		}
		m_pNetParty = NULL;

		if (m_pPlayerManager)
		{
			m_pPlayerManager->Shutdown();
			delete m_pPlayerManager;
		}
		m_pPlayerManager = NULL;

		if (m_pRouter)
		{
			m_pRouter->Shutdown();
			delete m_pRouter;
		}
		m_pRouter = NULL;

		if (m_pLock)
			delete m_pLock;
		m_pLock = NULL;

		if (m_pFrameTimer)
		{
			m_pFrameTimer->Stop();
			delete m_pFrameTimer;
		}
		m_pFrameTimer = NULL;

		if (m_pNetCreationHelper)
			delete m_pNetCreationHelper;
		m_pNetCreationHelper = NULL;

		if (m_pSessionEnumerator)
			delete m_pSessionEnumerator;
		m_pSessionEnumerator = NULL;

		for (S32 i=0; i<eMatchmakingSessionMax; i++)
		{
			if (m_pMatchmakingSessions[i] != NULL)
				delete m_pMatchmakingSessions[i];
			m_pMatchmakingSessions[i] = NULL;
		}

		if (m_pScoreboardReaderByRank)
		{
			m_pScoreboardReaderByRank->StopAndCleanup();
			delete m_pScoreboardReaderByRank;
		}
		m_pScoreboardReaderByRank = NULL;

		if (m_pScoreboardReaderByUsers)
		{
			m_pScoreboardReaderByUsers->StopAndCleanup();
			delete m_pScoreboardReaderByUsers;
		}
		m_pScoreboardReaderByUsers = NULL;

		if (m_pPlayerSkillWriter)
		{
			m_pPlayerSkillWriter->StopAndCleanup();
			delete m_pPlayerSkillWriter;
		}
		m_pPlayerSkillWriter = NULL;

		if (m_pScoreboardWriter)
		{
			m_pScoreboardWriter->StopAndCleanup();
			delete m_pScoreboardWriter;
		}
		m_pScoreboardWriter = NULL;
		
		if (NetPlay::C_Core::Instance().IsInitialised())
			NetPlay::C_SessionDeletion::Instance().Shutdown();

		m_CommunicatorInsertedDelay = 0.0f;
		m_MuteUpdateDelay = 0.0f;
		m_TotalRecentPlayersToAdd = 0;
		m_RemainingRecentPlayersToAdd = 0;
		m_RecentPlayersToAddStartOffset = 0;
		m_CanAddMoreRecentPlayers = true;
		m_Initialised = false;		

	}

	void C_NetServices::StopScoreboardRequests(void)
	{
		if (!m_Initialised)
			return;

		m_pScoreboardReaderByRank->StopAndCleanup();
		m_pScoreboardReaderByUsers->StopAndCleanup();

		m_pScoreboardWriter->StopAndCleanup();
		m_pPlayerSkillWriter->StopAndCleanup();
	}

	bool C_NetServices::ReceiveNetMessage(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, const U32 numUnEncryptedBytes, bdAddrHandleRef& addr)
	{
		//PERF_EVENT("NS.ReceiveNetMessage(3)");

		if (!m_Initialised)
			return false;

		// update keep alive for player only is the message is not a join request.
		if (eNetMessage_JoinRequest != hdr.m_PacketType && eNetMessage_JoinFailure != hdr.m_PacketType)
			m_pPlayerManager->UpdateKeepAliveForPlayer(hdr.m_SenderReference, hdr.m_PartyChannel ? true : false, addr);
		
		// if keep alive then all processing has been done already so tell router that packet 
		// has been processed and not to pass packet onto anyother listeners.
		if (eNetMessage_KeepAlive == hdr.m_PacketType)
			return true; 

		if (eNetMessage_BasicVoice == hdr.m_PacketType)
		{
			m_pPlayerManager->ReceiveBasicVoiceMessage(msg, hdr.m_SenderReference, hdr.m_PartyChannel ? true : false);
			return true;
		}

		if (hdr.m_PartyChannel)
		{
			if (eNetMessage_JoinRequest == hdr.m_PacketType)
				return m_pNetParty->ReceiveJoinRequest(msg, hdr, addr);
			else if (eNetMessage_JoinFailure == hdr.m_PacketType)
				return m_pNetParty->ReceiveJoinFailure(msg, hdr, addr);
			else
				return m_pNetParty->ReceiveUnReliable(msg, hdr.m_PacketType, numUnEncryptedBytes, hdr.m_SenderReference, addr);	
		}
		else
		{
			if (eNetMessage_JoinRequest == hdr.m_PacketType)
				return m_pNetRace->ReceiveJoinRequest(msg, hdr, addr);
			else if (eNetMessage_JoinFailure == hdr.m_PacketType)
				return m_pNetRace->ReceiveJoinFailure(msg, hdr, addr);
			else
				return m_pNetRace->ReceiveUnReliable(msg, hdr.m_PacketType, numUnEncryptedBytes, hdr.m_SenderReference, addr);	
		}
		
		return false;
	}

	bool C_NetServices::ReceiveNetMessage(C_NetMessage& msg, const DatagramReliablePacketHeader& hdr, bdAddrHandleRef& addr)
	{
		//PERF_EVENT("NS.ReceiveNetMessage(2)");

		if (!m_Initialised)
			return false;

		if (hdr.m_PartyChannel)
		{
			if (eNetMessage_JoinReply == hdr.m_PacketType)
			{
				m_pNetParty->ReceiveJoinReply(msg, hdr, addr, hdr.m_SenderReference);
				
				if (m_pNetParty->IsRunning())	
				{
					// since join replies are sent reliably but don't get added into the inbox we must manually reply with an acknowledgement packet.
					C_NetMessage& ackMsg  = m_pRouter->GetOutgoingUnReliable();
					ackMsg.CreateAcknowledgement(m_pPlayerManager->LocalPrimaryJoinId(true), hdr.m_PacketSequence , true);
					m_pRouter->SendMessage(ackMsg, addr);
				}
				return true;
			}
		}
		else
		{
			if (eNetMessage_JoinReply == hdr.m_PacketType)
			{
				m_pNetRace->ReceiveJoinReply(msg, hdr, addr, hdr.m_SenderReference);

				if (m_pNetRace->IsRunning())
				{
					// since join replies are sent reliably but don't get added into the inbox we must manually reply with an acknowledgement packet.
					C_NetMessage& ackMsg  = m_pRouter->GetOutgoingUnReliable();
					ackMsg.CreateAcknowledgement(m_pPlayerManager->LocalPrimaryJoinId(false), hdr.m_PacketSequence , false);
					m_pRouter->SendMessage(ackMsg, addr);
				}
				return true;
			}
		}

		C_NetPlayer *pPlr =m_pPlayerManager->FindFromJoinId(hdr.m_SenderReference, hdr.m_PartyChannel ? true : false);
		if (!pPlr)
			return false;

		m_pPlayerManager->UpdateKeepAliveForPlayer(hdr.m_SenderReference, hdr.m_PartyChannel ? true : false, addr);

		// reliable messages should only be sent by the primary player on each console.
		BIZ_ASSERT(pPlr->m_LocalIndex == 0);

		// add reliable message to inbox for later processing, acknowlegement will be sent internally.
		if (pPlr->m_pInBox)
			pPlr->m_pInBox->Add(msg, addr, m_pPlayerManager->LocalPrimaryJoinId(hdr.m_PartyChannel ? true : false), *m_pRouter);

		return true;
	}

	bool C_NetServices::ReceiveNetMessage(const DatagramAcknowledgementHeader& hdr, bdAddrHandleRef& addr)
	{
		//PERF_EVENT("NS.ReceiveNetMessage(1)");

		if (!m_Initialised)
			return false;

		C_NetPlayer *pPlr = m_pPlayerManager->FindFromJoinId(hdr.m_SenderReference, hdr.m_PartyChannel ? true : false);
		if (!pPlr)
			return false;

		m_pPlayerManager->UpdateKeepAliveForPlayer(hdr.m_SenderReference, hdr.m_PartyChannel ? true : false, addr);

		// reliable messages should only be sent by the primary player on each console.
		BIZ_ASSERT(pPlr->m_LocalIndex == 0);

		// acknowledge message in outbox if valid.
		if (pPlr->m_pOutBox)
			pPlr->m_pOutBox->ReceiveAcknowledgement(hdr);

		// return true to say packet has been processed. router won't pass packet on to any other listener.
		return true;
	}

	void C_NetServices::Poll(void)
	{
		if (!m_Initialised)
			return;

		if (!m_pLock->TryEnter())
			return; 

		F32 frameTimeInSeconds = net30fps;
		if (m_pFrameTimer->IsActive())
			frameTimeInSeconds = m_pFrameTimer->ElapsedTimeInSeconds();
		m_pFrameTimer->Start();

		//LOGINFO(g_Log, "C_NetServices::Poll - %f", frameTimeInSeconds);

		// poll mercury network stack.
		NetPlay::C_Core::Instance().Poll(false);

		// pump any game invites that are being sent.
		bool giError = false;
		SysProfile::C_Manager& sysProfileMgr = SysProfile::C_Manager::Instance();
		if (sysProfileMgr.SendingGameInvite())
			sysProfileMgr.ContinueSendGameInvite(giError);

		// poll matchmaking sessions
		for (S32 i=0; i<eMatchmakingSessionMax; i++)
		{
			if (m_pMatchmakingSessions[i])
				m_pMatchmakingSessions[i]->Poll();
		}

		// poll session deletion.
		NetPlay::C_SessionDeletion::Instance().Poll();

		// poll socket router for incomming datagrams.
		m_pRouter->Poll(frameTimeInSeconds);

		// poll players connections and outgoing message queues.
		// send any keep alives if required.
		m_pPlayerManager->Poll(frameTimeInSeconds, *m_pRouter);

		// poll incoming reliable message queues and pass to required destination session..
		bool restart = false;
		do
		{	
			restart = false;
			for (S32 i=0; restart == false && i<m_pPlayerManager->TotalSize(); i++)
			{
				C_NetPlayer* pPlr = m_pPlayerManager->Index(i);
				if (pPlr && !pPlr->LocalPlayer())
				{
					if (!pPlr->LocalPlayer() && pPlr->PrimaryPlayer())
					{
						// remote primary players should have in / out boxes..
						BIZ_ASSERT(pPlr->m_pInBox != NULL);
						BIZ_ASSERT(pPlr->m_pOutBox != NULL);

						U8 msgType = 0;
						bool partySession = false;
						C_NetMessage& tempMsg = m_pRouter->GetReceiveMessage();
						if (pPlr->m_pInBox->Poll(tempMsg, msgType, partySession))
						{
							if (partySession)
								m_pNetParty->ReceiveReliable(tempMsg, msgType, pPlr->m_PartyConnection.m_JoinId);
							else
								m_pNetRace->ReceiveReliable(tempMsg, msgType, pPlr->m_RaceConnection.m_JoinId);
								
							restart = true;
						}
					}
				}
			}
		}while(restart);

		// poll party and race controllers.
		m_pNetRace->Poll(frameTimeInSeconds);
		m_pNetParty->Poll(frameTimeInSeconds, m_pNetRace->IsRunning());

		// to service the task used for players in the multiplayer groups.
		PumpGroupTasks();
		
		if (m_MuteUpdateDelay != 0.0f)
		{
			m_MuteUpdateDelay -= frameTimeInSeconds;
			if (m_MuteUpdateDelay <= 0.0f)
			{
				m_MuteUpdateDelay = 0.0f;
				m_pNetParty->VoiceMuteChanged();
				m_pNetRace->VoiceMuteChanged();
			}
		}

		Communicator::C_Core::Instance().MuteUpdate();
		
		// update messaging from demonware.
		RaceBook::C_RaceBookManager::Instance().UpdateDemonware(); 

#if !defined ( CFG_GOLD )
		// debug scoreboard read code..
		static bool readMe = false;
		/*if(readMe && !stricmp(Debug::GetBuildUser(), "dworswick"))
		{
			if (m_pRouter->GetPlatformConnectionState() == eConnectionStateComplete)
			{
				const U32 sb = 1;
				if (sHackScoreboardReadState == 0)
				{
					Scoreboard::ColumnIdentifierList cols;
					Scoreboard::ColumnTypeList colTypes;
					if (m_pScoreboardReaderByRank->AddScoreboard(sb, cols, colTypes, true))
					{
						if (m_pScoreboardReaderByRank->StartReadByRank(0, 1, 10))
							sHackScoreboardReadState = 1;
						else
							sHackScoreboardReadState = -1;
					}
					else
						sHackScoreboardReadState = -1;
				}
				else if (sHackScoreboardReadState == 1)
				{
					bool error = false;
					if (m_pScoreboardReaderByRank->Continue(error))
					{
						if (!error)
						{
							U32 total = 0;
							Scoreboard::RowData rows[10];
							U32 numRows = m_pScoreboardReaderByRank->ExtractRows(sb, rows, 10, total);
							if (numRows)
							{
							}
						}
						sHackScoreboardReadState = -1;
						m_pScoreboardReaderByRank->StopAndCleanup();
					}
				}
			}
		}*/
#endif

		C_GameApp::GetInstance()->PollPlatformCallbacks();

		UpdateAddToRecentPlayers();
		UpdateCommunicatorInserted(frameTimeInSeconds);
		UpdatePeerRouting();

		m_pLock->Exit();
	}

	void C_NetServices::EnterLock(void)
	{
		if (m_Initialised)
		{
			m_pLock->Enter();
		}
	}

	void C_NetServices::Exitlock(void)
	{
		if (m_Initialised)
		{
			m_pLock->Exit();
		}
	}

	void C_NetServices::CreateLuaEnums(lua_State *pState, char *tableName)
	{
		lua_getglobal(pState, tableName);

		if(lua_isnil(pState,-1))
		{
			LOGFATAL(g_Log,  "%s table not found.", tableName);
			lua_pop(pState,1);
			return;
		}

		lua_pushstring(pState, "Network");
		lua_newtable(pState);

			// generic 
			Lua::AddDataMember(pState,"MaxPlayers",	NET_MAX_VEHICLES);

			// router
			lua_pushstring(pState, "Router");
			lua_newtable(pState);
				// connection enums
				Lua::AddDataMember(pState,"eConnectionStateNotActive",	eConnectionStateNotActive);
				Lua::AddDataMember(pState,"eConnectionStateInProgress",	eConnectionStateInProgress);
				Lua::AddDataMember(pState,"eConnectionStateFailed",		eConnectionStateFailed);
				Lua::AddDataMember(pState,"eConnectionStateComplete",	eConnectionStateComplete);

				// connection fail reasons.
				Lua::AddDataMember(pState,"eConnectionResultNone",				eConnectionResultNone);
				Lua::AddDataMember(pState,"eConnectionResultAgeError",			eConnectionResultAgeError);
				Lua::AddDataMember(pState,"eConnectionResultGeneric",			eConnectionResultGeneric);
				Lua::AddDataMember(pState,"eConnectionResultNoServiceProvider",	eConnectionResultNoServiceProvider);
				Lua::AddDataMember(pState,"eConnectionResultNoNetwork",			eConnectionResultNoNetwork);
				Lua::AddDataMember(pState,"eConnectionResultInvalidUserName",	eConnectionResultInvalidUserName);
				Lua::AddDataMember(pState,"eConnectionResultInvalidPassword",	eConnectionResultInvalidPassword);

				// account creation enums.
				Lua::AddDataMember(pState,"eAccountCreationResultSuccess",			eAccountCreationResultSuccess);
				Lua::AddDataMember(pState,"eAccountCreationResultGenericError",		eAccountCreationResultGenericError);
				Lua::AddDataMember(pState,"eAccountCreationResultNoNetwork",		eAccountCreationResultNoNetwork);
				Lua::AddDataMember(pState,"eAccountCreationResultUserNameExists",	eAccountCreationResultUserNameExists);
				Lua::AddDataMember(pState,"eAccountCreationResultUserNameIllegal",	eAccountCreationResultUserNameIllegal);
				Lua::AddDataMember(pState,"eAccountCreationResultUserNameVulgar",	eAccountCreationResultUserNameVulgar);
				Lua::AddDataMember(pState,"eAccountCreationResultTooManyAccounts",	eAccountCreationResultTooManyAccounts);
				Lua::AddDataMember(pState,"eAccountCreationResultLicenseInvalid",	eAccountCreationResultLicenseInvalid);
				Lua::AddDataMember(pState,"eAccountCreationResultLicenseLocked",	eAccountCreationResultLicenseLocked);
			lua_rawset(pState,-3);

			// network transport version comparison
			lua_pushstring(pState, "Version");
			lua_newtable(pState);
				Lua::AddDataMember(pState,"eNetVersionSame",	eNetVersionSame);
				Lua::AddDataMember(pState,"eNetVersionOlder",	eNetVersionOlder);
				Lua::AddDataMember(pState,"eNetVersionNewer",	eNetVersionNewer);
			lua_rawset(pState,-3);

			// join error codes
			lua_pushstring(pState, "JoinErrors");
				lua_newtable(pState);
				Lua::AddDataMember(pState,"eJoinErrorInvalid",				eJoinErrorInvalid);
				Lua::AddDataMember(pState,"eJoinErrorNone",					eJoinErrorNone);
				Lua::AddDataMember(pState,"eJoinErrorNoConnectionToHost",	eJoinErrorNoConnectionToHost);
				Lua::AddDataMember(pState,"eJoinErrorHostDiffersOnPeer",	eJoinErrorHostDiffersOnPeer);
				Lua::AddDataMember(pState,"eJoinErrorNotEnoughOpenSlots",	eJoinErrorNotEnoughOpenSlots);
				Lua::AddDataMember(pState,"eJoinErrorBannedFromSession",	eJoinErrorBannedFromSession);
				Lua::AddDataMember(pState,"eJoinErrorNoCommunications",		eJoinErrorNoCommunications);
				Lua::AddDataMember(pState,"eJoinErrorToManyTeams",			eJoinErrorToManyTeams);
				Lua::AddDataMember(pState,"eJoinErrorToManyTeamMembers",	eJoinErrorToManyTeamMembers);
				Lua::AddDataMember(pState,"eJoinErrorContentMismatch",		eJoinErrorContentMismatch);
				Lua::AddDataMember(pState,"eJoinErrorRankedMatchStarted",	eJoinErrorRankedMatchStarted);
				Lua::AddDataMember(pState,"eJoinErrorChampionshipStarted",	eJoinErrorChampionshipStarted);
				Lua::AddDataMember(pState,"eJoinErrorPartyClosed",			eJoinErrorPartyClosed);
				Lua::AddDataMember(pState,"eJoinErrorProfileProblem",		eJoinErrorProfileProblem);
				Lua::AddDataMember(pState,"eJoinErrorPlaylistVerClient",	eJoinErrorPlaylistVerClient);
				Lua::AddDataMember(pState,"eJoinErrorPlaylistVerHost",		eJoinErrorPlaylistVerHost);
			lua_rawset(pState,-3);
			
			// network player states
			lua_pushstring(pState, "PlayerStates");
			lua_newtable(pState);
				Lua::AddDataMember(pState,"eNetPlayerStateUnSet",					eNetPlayerStateUnSet);
				Lua::AddDataMember(pState,"eNetPlayerStateJoining",					eNetPlayerStateJoining);
				Lua::AddDataMember(pState,"eNetPlayerStateDisconnecting",			eNetPlayerStateDisconnecting);
				Lua::AddDataMember(pState,"eNetPlayerStateJoinComplete",			eNetPlayerStateJoinComplete);
				Lua::AddDataMember(pState,"eNetPlayerStateIdle",					eNetPlayerStateIdle);
				Lua::AddDataMember(pState,"eNetPlayerStateChangingSettings",		eNetPlayerStateChangingSettings);
				Lua::AddDataMember(pState,"eNetPlayerStateChangingCar",				eNetPlayerStateChangingCar);
				Lua::AddDataMember(pState,"eNetPlayerStateLoadingRace",				eNetPlayerStateLoadingRace);
				Lua::AddDataMember(pState,"eNetPlayerStateLoadingUi",				eNetPlayerStateLoadingUi);
				Lua::AddDataMember(pState,"eNetPlayerStatePartyBusy",				eNetPlayerStatePartyBusy);
				Lua::AddDataMember(pState,"eNetPlayerStatePartySyncJoinRaceStart",	eNetPlayerStatePartySyncJoinRaceStart);
				Lua::AddDataMember(pState,"eNetPlayerStatePartyJoinRace",			eNetPlayerStatePartyJoinRace);
				Lua::AddDataMember(pState,"eNetPlayerStatePartySyncJoinRaceEnd",	eNetPlayerStatePartySyncJoinRaceEnd);
				Lua::AddDataMember(pState,"eNetPlayerStatePartyJoinRaceFailed",		eNetPlayerStatePartyJoinRaceFailed);
				Lua::AddDataMember(pState,"eNetPlayerStatePartyJoinRaceSuccessful",	eNetPlayerStatePartyJoinRaceSuccessful);
				Lua::AddDataMember(pState,"eNetPlayerStateWaitingForOthers",		eNetPlayerStateWaitingForOthers);

				// In race states
				Lua::AddDataMember(pState,"eNetPlayerStateRacing",					eNetPlayerStateRacing);
				Lua::AddDataMember(pState,"eNetPlayerStateFinished",				eNetPlayerStateFinished);
				Lua::AddDataMember(pState,"eNetPlayerStateViewingResults",			eNetPlayerStateViewingResults);
				Lua::AddDataMember(pState,"eNetPlayerStateViewingResultsIdle",		eNetPlayerStateViewingResultsIdle);
			lua_rawset(pState,-3);

		lua_rawset(pState,-3); // table UIEnums.Network

		lua_pushstring(pState, "RichPresence");

			lua_newtable(pState);
			Lua::AddDataMember(pState,"Idle",				eRichPresence_Idle);				
			Lua::AddDataMember(pState,"SignedIn",			eRichPresence_SignedIn);			
			Lua::AddDataMember(pState,"Stickerbook",		eRichPresence_Stickerbook);		
			Lua::AddDataMember(pState,"Leaderboards",		eRichPresence_Leaderboards);		
			Lua::AddDataMember(pState,"Help",				eRichPresence_Help);				
			Lua::AddDataMember(pState,"Showroom",			eRichPresence_Showroom);			
			Lua::AddDataMember(pState,"InRace",				eRichPresence_InRace);			
			Lua::AddDataMember(pState,"SpMain",				eRichPresence_SpMain);			
			Lua::AddDataMember(pState,"Photos",				eRichPresence_Photos);			
			Lua::AddDataMember(pState,"FriendChallenges",	eRichPresence_FriendChallenges);	
			Lua::AddDataMember(pState,"Career",				eRichPresence_SpCareer);			
			Lua::AddDataMember(pState,"SpChoosing",			eRichPresence_Sp_Choosing);		
			Lua::AddDataMember(pState,"MpMain",				eRichPresence_MpMain);			
			Lua::AddDataMember(pState,"SplitScreen",		eRichPresence_SplitScreen);		
			Lua::AddDataMember(pState,"RaceHistory",		eRichPresence_RaceHistory);
			Lua::AddDataMember(pState,"LAN",				eRichPresence_LAN);
			Lua::AddDataMember(pState,"OnlineMain",			eRichPresence_OnlineMain);		
			Lua::AddDataMember(pState,"ModShop",			eRichPresence_ModShop);				
			Lua::AddDataMember(pState,"Challenges",			eRichPresence_Challenges);		
			Lua::AddDataMember(pState,"MpLobby",			eRichPresence_MpLobby);
		lua_rawset(pState,-3); // table UIEnums.RichPresence

		lua_pop(pState,1);
	}

	void C_NetServices::CommunicatorMuteInformationChanged(void)
	{
		if (!m_Initialised)
			return;

		m_pLock->Enter();

		// we delay sending the latest mute data to the peers to allow the player to make more changes before being sent.
		// this should stop the player spamming mute net messages.
		m_MuteUpdateDelay = 1.5f;

		// ascertain what players are actually muted by the system.
		Communicator::C_Core::Instance().MuteUpdate();

		m_pLock->Exit();
	}

	void C_NetServices::AcceptGameInvite(const SysProfile::GameInvite& gi)
	{
		if (!m_Initialised)
			return;

		m_pLock->Enter();

		bool store = true;
		bool check_session = true;
		if (gi.m_Active)
		{
			const bdSecurityID& secId = gi.m_Id;

			// check party and race session to make sure invite is not for a session we are already a member of.
			if ( check_session )
			{
				if (m_pNetParty->InSession(secId))
					store = false;
				if (m_pNetRace->InSession(secId))
					store = false;
			}
		}
		
		if (store)
			m_pRouter->GetGameInvite() = gi;

		m_pLock->Exit();
	}

	bool C_NetServices::CreateRouter(void)
	{
		if (NULL != m_pRouter)
			return true;

#if defined (PLATFORM_PS3)
		m_pRouter = new C_NetRouterPS3;
#elif defined (PLATFORM_PC)
		m_pRouter = new C_NetRouterPC;
#endif
		
		if (m_pRouter)
		{
			if (m_pRouter->Initialise(1024*5)) // maximum we can send in a single reliable packet.
			{
				m_pRouter->RegisterListener(this);
				return true;
			}
			else
			{
				delete m_pRouter;
				m_pRouter = NULL;
			}
		}
		else
		{
			// failed to create router..
			BIZ_ASSERT(false);
		}

		return false;
	}

	bool C_NetServices::CreatePlayerManager(void)
	{
		BIZ_ASSERT(NULL == m_pPlayerManager);

		bool ok = false;
		m_pPlayerManager = new C_NetPlayerManager;

		if (m_pPlayerManager)
			ok = m_pPlayerManager->Initialise();

		return ok;
	}

	bool C_NetServices::CreateScoreboardReaders(void)
	{
		BIZ_ASSERT(NULL == m_pScoreboardReaderByRank);
		BIZ_ASSERT(NULL == m_pScoreboardReaderByUsers);

		bool ok = false;

		m_pScoreboardReaderByRank = new Scoreboard::C_ReaderByRank;
		m_pScoreboardReaderByUsers = new Scoreboard::C_ReaderByUsers;

		if (m_pScoreboardReaderByRank && m_pScoreboardReaderByUsers)
		{
			ok = m_pScoreboardReaderByRank->Initialise();
			ok = ok && m_pScoreboardReaderByUsers->Initialise();
		}

		return ok;
	}

	bool C_NetServices::CreateScoreboardWriter(void)
	{
		BIZ_ASSERT(NULL == m_pScoreboardWriter);
		BIZ_ASSERT(NULL == m_pPlayerSkillWriter);

		bool ok = false;
		m_pScoreboardWriter = new Scoreboard::C_Writer;
		m_pPlayerSkillWriter = new NetPlay::C_PlayerSkillWriter;
		if (m_pScoreboardWriter && m_pPlayerSkillWriter)
		{
			ok = m_pScoreboardWriter->Initialise(6, 75);
			ok = ok && m_pPlayerSkillWriter->Initialise(NET_MAX_VEHICLES);
		}

		return ok;
	}

	bool C_NetServices::CreateRace(void)
	{
		BIZ_ASSERT(NULL == m_pNetRace);

		bool ok = false;
		m_pNetRace = new C_NetRace;
		if (m_pNetRace)
			ok = m_pNetRace->Initialise(&m_pRouter, &m_pPlayerManager, &(m_pMatchmakingSessions[eMatchmakingSessionTypeRace]));
		return ok;
	}

	bool C_NetServices::CreateParty(void)
	{
		BIZ_ASSERT(NULL == m_pNetParty);

		bool ok = false;
		m_pNetParty = new C_NetParty;
		if (m_pNetParty)
			ok = m_pNetParty->Initialise(&m_pRouter, &m_pPlayerManager, &(m_pMatchmakingSessions[eMatchmakingSessionTypeParty]));
		return ok;
	}

	bool C_NetServices::CreateMisc(void)
	{
		BIZ_ASSERT(NULL == m_pFrameTimer);

		bool ok = false;
		
		m_pFrameTimer = new C_NetTimer;
		m_pLock = new MT::Lock;

		ok = m_pFrameTimer != NULL ? true : false;
		ok = ok && m_pLock != NULL ? true : false;

		return ok;
	}

	bool C_NetServices::CreateOnlineSessions(void)
	{
		BIZ_ASSERT(m_pMatchmakingSessions[eMatchmakingSessionTypeParty] == NULL);
		BIZ_ASSERT(m_pMatchmakingSessions[eMatchmakingSessionTypeRace] == NULL);
	
		//Shutdown() will release all resources if not correctly setup.
		m_pNetCreationHelper = new C_NetCreationHelper;
		if (NULL == m_pNetCreationHelper)
			return false;

		m_pSessionEnumerator = new NetPlay::C_SessionEnumerator;
		if (NULL == m_pSessionEnumerator)
			return false;

		if (!m_pSessionEnumerator->Initialise(10, 2, NET_MAX_MATCHING_SESSIONS_TO_ENUMERATE))
			return false;

		// party online session
		m_pMatchmakingSessions[eMatchmakingSessionTypeParty] = new NetPlay::C_Session;
		if (NULL == m_pMatchmakingSessions[eMatchmakingSessionTypeParty])
			return false;

		if (!m_pMatchmakingSessions[eMatchmakingSessionTypeParty]->Initialise(NET_MAX_PARTY_MEMBERS, 50))
			return false;

		// race online session
		m_pMatchmakingSessions[eMatchmakingSessionTypeRace] = new NetPlay::C_Session;
		if (NULL == m_pMatchmakingSessions[eMatchmakingSessionTypeRace])
			return false;

		if (!m_pMatchmakingSessions[eMatchmakingSessionTypeRace]->Initialise(NET_MAX_VEHICLES, 50))
			return false;

		return true;
	}

	void C_NetServices::DebugGetBandwidthUsage(F32& in, F32& out)
	{
		out = in = 0.0f;
#if !defined (CFG_GOLD)
		if (!m_Initialised)
			return;
		m_pRouter->DebugGetBandwidthUsage(in, out);
#endif
	}

	bdContentStreaming*	C_NetServices::GetContentStreamingServer(void)
	{
		if (!m_Initialised)
			return NULL;
		return m_pRouter->GetContentStreamingServer();
	}

	bdVoteRank*	C_NetServices::GetVoteRank(void)
	{
		if (!m_Initialised)
			return NULL;
		return m_pRouter->GetVoteRank();
	}

	void C_NetServices::AddToRecentPlayers(void)
	{
		if (!m_Initialised)
			return;

#if defined ( PLATFORM_PS3 )
		m_TotalRecentPlayersToAdd = 0;
		m_RemainingRecentPlayersToAdd = 0;
		m_RecentPlayersToAddStartOffset = 0;
		m_CanAddMoreRecentPlayers = true;

		for (U8 i=0; i<m_pPlayerManager->Size(false); i++)
		{
			C_NetPlayer* p = m_pPlayerManager->Index(i);
			if (p->LocalPlayer() || !p->m_RaceConnection.IsActive() || p->m_User.m_IsGuest)
				continue;

			m_RecentPlayers[m_TotalRecentPlayersToAdd] = p->m_User.m_Uid;
			m_TotalRecentPlayersToAdd += 1;
			if (m_TotalRecentPlayersToAdd >= m_RecentPlayers.Capacity())
				break;
		}

		m_RemainingRecentPlayersToAdd = m_TotalRecentPlayersToAdd;
#endif
	}

	void C_NetServices::SetUserGroup( const S32 &groupID )
	{
		if (!m_Initialised)
			return;
	
		m_pLock->Enter();

		KillSetGroupTask();

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (lobbyService)
		{
			bdGroup *const group = lobbyService->getGroup();
			if(group)
			{			
				LOGINFO(g_Log, "Setting your group to: %i", groupID);
				const bdUInt32 id = groupID;
				m_SetGroupTask = group->setGroups(&id, 1);
				m_SetGroupTaskActive = true;
			}
			else
				LOGWARN(g_Log, "Unable to set group, no bdGroup service to use");
		}
		else
			LOGWARN(g_Log, "Unable to set group, no lobby service!");

		m_pLock->Exit();
	}

	bool C_NetServices::StartServerTimeTask()
	{
		bool serverTimeTaskComplete = false; 

		if (!m_Initialised)
			return false;
	
		m_pLock->Enter();

		m_ServerTimeTaskFailed = false; 

		bdTitleUtilities* title_utilities = Router().GetTitleUtilities();
		if (title_utilities )
		{
			// If not already obtained or already active..
			if(m_ServerTimeStampAtSessionStart.m_timeStamp == 0) // Then we havent obtained a time stamp
			{
				if ( !m_ServerTimeStampTaskActive )
				{
					KillServerTimeStampTask();
					m_ServerTimeStampTask = title_utilities->getServerTime( &m_ServerTimeStampAtSessionStart );
					if (m_ServerTimeStampTask->getStatus()!= bdRemoteTask::BD_PENDING)
					{
						LOGWARN(g_Log, "Unable to StartServerTimeTask.");

						serverTimeTaskComplete = true;
						m_ServerTimeTaskFailed = true; 
					}

					m_ServerTimeStampTaskActive = true;
				}
			}
			else
			{
				serverTimeTaskComplete = true; 
			}
		}
		else
		{
			LOGWARN(g_Log, "Unable to StartServerTimeTask, no title Utilities!");
			serverTimeTaskComplete = true;		 // Cant connect to servers, task finished (try again later!)
			m_ServerTimeTaskFailed = true; 
		}

		m_pLock->Exit();

		return serverTimeTaskComplete; 

	}

	void C_NetServices::PumpGroupTasks( void )
	{
		if (!m_Initialised)
			return;

		if (m_SetGroupTaskActive)
		{
			switch (NetPlay::DemonwareRemoteTask::CheckState(m_SetGroupTask))
			{
			case NetPlay::DemonwareRemoteTask::ePending:
				break;

			default:
				KillSetGroupTask();
				break;
			}
		}

		if (m_GroupCountTaskActive)
		{
			switch (NetPlay::DemonwareRemoteTask::CheckState(m_GroupCountTask))
			{
				case NetPlay::DemonwareRemoteTask::ePending:
					return;
					break;

				default:
					break;
			}

			KillGroupCountTask();
		}

		if (m_ServerTimeStampTaskActive)
		{
			switch (NetPlay::DemonwareRemoteTask::CheckState(m_ServerTimeStampTask))
			{
				case NetPlay::DemonwareRemoteTask::ePending:
					return;
					break;

				case NetPlay::DemonwareRemoteTask::eFailed:
					m_ServerTimeTaskFailed = true; 
					return;
					break;

				default:
					break;
			}

			KillServerTimeStampTask();
		}

		if(m_StopWatch.getElapsedTimeInSeconds() > DW_NUM_SECONDS_BETWEEN_GROUP_REFRESH)
		{
			if (m_GroupCountTaskActive == false)
			{
				AllowPlayerCountUpdate();
				RetrieveGroupsPlayerCount();
			}
		}
	}

	void C_NetServices::KillGroupCountTask( void )
	{		
		if (!m_Initialised)
			return;

		if (m_GroupCountTaskActive)
		{
			if( NetPlay::DemonwareRemoteTask::eDone == NetPlay::DemonwareRemoteTask::CheckState(m_GroupCountTask) )
				m_HaveGroupCounts = true;
			else
				m_HaveGroupCounts = false;

			m_GroupCountRetrieved = true;

			for( int i(0); i < DW_MAX_NUM_COUNTVALUES; ++i )
			{
				m_GroupCountsBuffer[i].m_groupID = m_GroupCounts[i].m_groupID;
				m_GroupCountsBuffer[i].m_groupCount = m_GroupCounts[i].m_groupCount;
			}

			m_StopWatch.reset();
			m_StopWatch.start();
		}
		
		NetPlay::DemonwareRemoteTask::Close(m_GroupCountTask);
		m_GroupCountTaskActive = false;
	}

	void C_NetServices::KillSetGroupTask(void)
	{
		if (!m_Initialised)
			return;

		NetPlay::DemonwareRemoteTask::Close(m_SetGroupTask);
		m_SetGroupTaskActive = false;
	}

	void C_NetServices::KillServerTimeStampTask(void)
	{
		if (!m_Initialised)
			return;

		NetPlay::DemonwareRemoteTask::Close(m_ServerTimeStampTask);
		m_ServerTimeStampTaskActive = false;
	}

	bool C_NetServices::ShowPlayerCountData( void )
	{
		if(!m_GroupCountRetrieved)
			RetrieveGroupsPlayerCount();

		if( false == m_HaveGroupCounts )
			return false;

		// Last one is the default group (menus) so don't count that
		U32 returnValue = 0;
		for(U32 loopPos = 0; loopPos < DW_MAX_NUM_COUNTVALUES-1; loopPos++)
		{
			returnValue += m_GroupCountsBuffer[loopPos].m_groupCount;
		}
		return ( returnValue > C_MultiplayerData::Instance().m_pConfig->m_MinTotalPlayerShowCount );
	}


	U32 C_NetServices::GetOnlinePlayerCount( const bool rankedEventsOnly )
	{
		if (!m_Initialised)
			return 0;

		if(!m_GroupCountRetrieved)
			RetrieveGroupsPlayerCount();

		if( false == m_HaveGroupCounts )
			return 0;

		U32 returnValue = 0;
		for(U32 loopPos = 0; loopPos < DW_MAX_NUM_COUNTVALUES; loopPos++)
		{
			returnValue += m_GroupCountsBuffer[loopPos].m_groupCount;
		}
		return returnValue;
	}


	U32 C_NetServices::GetRankedEventPlayerCount( const U32 &groupID )
	{
		if(!m_GroupCountRetrieved)
			RetrieveGroupsPlayerCount();

		if( false == m_HaveGroupCounts )
			return 0;

		if(groupID < DW_MAX_NUM_COUNTVALUES)
		{
			for(U32 loopPos = 0; loopPos < DW_MAX_NUM_COUNTVALUES; loopPos++)
			{
				if(m_GroupCountsBuffer[loopPos].m_groupID == groupID)
				{
					return m_GroupCountsBuffer[loopPos].m_groupCount;
				}
			}
			return 0;
		}
		else
		{
			LOGERROR(g_Log, "Event ID passed in: %i is out of bounds: %i", groupID, DW_MAX_NUM_COUNTVALUES);
		}

		LOGERROR(g_Log, "Unable to find count for group with ID: %i", groupID);
		return 0;
	}
	
	void C_NetServices::RetrieveGroupsPlayerCount( void )
	{
		if (!m_Initialised)
			return;

		if(!m_GroupCountRetrieved && m_GroupCountTaskActive == false)
		{							
			bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
			if (lobbyService)
			{
				bdGroup *const group = lobbyService->getGroup();
				if(group)
				{
					bdUInt32 groupIDs[DW_MAX_NUM_COUNTVALUES];
					for(bdUInt32 i = 0; i < DW_MAX_NUM_COUNTVALUES; i++)
					{
						groupIDs[i] = i;
					}
					groupIDs[DW_MAX_NUM_COUNTVALUES - 1] = DW_DEFAULT_GROUP_ID;

					m_GroupCountTask = group->getGroupCounts(groupIDs, DW_MAX_NUM_COUNTVALUES, m_GroupCounts, DW_MAX_NUM_COUNTVALUES);
					m_GroupCountTaskActive = true;
					return;
				}
				else
				{
					LOGERROR(g_Log, "Unable to get group counts, no bdGroup service to use");
				}
			}
			else
			{
				LOGERROR(g_Log, "Unable to get group counts, no lobby service!");
			}

			m_HaveGroupCounts = false;
		}
	}


	void C_NetServices::AllowPlayerCountUpdate( void )
	{
		if (!m_Initialised)
			return;

		m_pLock->Enter();
		m_GroupCountRetrieved = false;
		m_StopWatch.reset();
		m_pLock->Exit();
	}

	void C_NetServices::StopPlayerCountUpdate(void)
	{
		m_pLock->Enter();
		KillGroupCountTask();
		m_GroupCountRetrieved = false;
		m_StopWatch.reset();
		m_pLock->Exit();
	}

	void C_NetServices::UpdateAddToRecentPlayers(void)
	{
		if (!m_Initialised || 0 == m_TotalRecentPlayersToAdd || !m_CanAddMoreRecentPlayers)
			return;

#if defined ( PLATFORM_PS3 )
		if (0 == m_RemainingRecentPlayersToAdd)
		{
			m_TotalRecentPlayersToAdd = 0;
			m_RemainingRecentPlayersToAdd = 0;
			m_RecentPlayersToAddStartOffset = 0;
			m_CanAddMoreRecentPlayers = true;
			//LOGINFO(g_Log, "Add recent players complete.");
			return;
		}
		else
		{
			U16 count = (m_RemainingRecentPlayersToAdd > SCE_NP_BASIC_PLAYER_HISTORY_MAX_PLAYERS) ? SCE_NP_BASIC_PLAYER_HISTORY_MAX_PLAYERS : m_RemainingRecentPlayersToAdd;
			m_RemainingRecentPlayersToAdd -= count;
			BIZ_ASSERT(m_RemainingRecentPlayersToAdd < m_TotalRecentPlayersToAdd);

			LOGINFO(g_Log, "sceNpBasicAddPlayersHistoryAsync() - adding %d player(s)", count);
			S32 ret = sceNpBasicAddPlayersHistoryAsync(&m_RecentPlayers[m_RecentPlayersToAddStartOffset], count, emptyString, &reqId);
			if (ret != 0)
			{
				LOGWARN(g_Log, "sceNpBasicAddPlayersHistoryAsync() failed 0x%x", ret);
			}
			else
				m_CanAddMoreRecentPlayers = false; // wait for task to complete.

			m_RecentPlayersToAddStartOffset = count;
		}
#endif
	}

	void C_NetServices::UpdateCommunicatorInserted(F32 frameTimeInSeconds)
	{
		if (!m_Initialised)
			return;

		if (m_pNetRace->IsRunning() || m_pNetParty->IsRunning())
		{
			bool peerNotificationRequired = false;
			const F32 pollTime = 1.5f; // how long to wait between polls to the communicator library
			m_CommunicatorInsertedDelay += frameTimeInSeconds;
			if (m_CommunicatorInsertedDelay >= pollTime)
			{
				for (U8 i=0; i<m_pPlayerManager->LocalSize(); i++)
				{
					C_NetPlayer* p = m_pPlayerManager->Index(i);
					if (p->LocalPlayer() && !p->m_VoiceBanned)
					{
						const bool communicatorConnected = Communicator::C_Core::Instance().CommunicatorConnected(p->m_ControllerPort);
						if (communicatorConnected != p->m_HasCommunicator)
						{
							// differing status so we need to inform peers..
							p->m_HasCommunicator = communicatorConnected;
							peerNotificationRequired = true;
						}
					}
				}
				m_CommunicatorInsertedDelay -= pollTime;
				if (m_CommunicatorInsertedDelay < 0.0f)
					m_CommunicatorInsertedDelay = 0.0f;
			}
			
			if (peerNotificationRequired)
			{
				if (m_pNetRace->IsRunning())
					m_pNetRace->SendCommunicatorStatusChangedMessage();

				if (m_pNetParty->IsRunning())
					m_pNetParty->SendCommunicatorStatusChangedMessage();
			}
		}
		else
			m_CommunicatorInsertedDelay = 0.0f;
	}

	void C_NetServices::UpdatePeerRouting(void)
	{
		if (!m_Initialised)
			return;

		if ((m_pNetRace->IsRunning() && m_pNetRace->IsOnline()) || m_pNetParty->IsRunning())
		{
			if (m_pRouter->PeerRoutingEnabled())
			{	
				bdNetImpl *const net = bdNet::getInstance();
				bdRelaySocketRouter *pRelayRouter = reinterpret_cast<bdRelaySocketRouter*> (net->getSocketRouter());
				if (pRelayRouter)
				{
					bdUInt32 dummy;
					bdUInt32 numConnections;
					pRelayRouter->getRelayerStats(dummy, dummy, dummy, numConnections);

					Perf::AddNamedCounter((F32)numConnections, "NetRB - Relays");

					U32 maxNumConnections = C_MultiplayerData::Instance().m_pConfig->m_PeerProxyMaxConnections;
					if (maxNumConnections <= 0)
						maxNumConnections = 8;

					pRelayRouter->setMaxRelayedConnections(maxNumConnections);
				}
			}
		}
	}

	void C_NetServices::TitleStorageStatusReset(void)
	{ 
		m_TitleStorage = ( true == g_DebugUseTitleStorage ) ? eTitleStorageWaitingForInit : eTitleStorageCompleted; 
	}

	void C_NetServices::ResetTimeOutTimer(void)
	{
		m_TitleStorageStatusTimeoutTimer.reset();
		m_TitleStorageStatusTimeoutTimer.start();
	}

	int C_NetServices::TitleStorageStatus(void)
	{
		//UIEnums.lua
		//TitleStorageState =
		//{
		//	Busy		= 1,
		//	Failed		= 2,
		//	Finished	= 3,
		//},

		if( (m_TitleStorage == eTitleStorageCompleted && m_TitleStorageRefreshTimer.getElapsedTimeInSeconds() > NUM_SECONDS_BETWEEN_TITLESTORAGE_REFRESH) ||
			(m_TitleStorage == eTitleStorageWaitingForInit && m_TitleStorageStatusTimeoutTimer.getElapsedTimeInSeconds() > NUM_SECONDS_WAITING_FOR_INIT_TIMEOUT) )
		{
			m_TitleStorage = eTitleStorageInit;
		}

		switch(m_TitleStorage)
		{
			case eTitleStorageWaitingForInit:
			case eTitleStorageInit:
			case eTitleStorageSetDownload:
			case eTitleStorageDownloading:
				return 1; //Busy
			case eTitleStorageError:
				m_TitleStorage = eTitleStorageInit; //reset flag
				return 2; //Failed
			case eTitleStorageCompleted:
				{
					C_MultiplayerData::Instance().ReloadMultiplayerPackData(); //Check to see if we need to reload the data
					return 3; //Finished
				}
		}
		return 2; //Failed
	}


	void C_NetServices::TitleStorageUpdate(void)
	{
		if( eTitleStorageCompleted == m_TitleStorage|| eTitleStorageWaitingForInit == m_TitleStorage)
		{
			return;
		}

		const S32 primaryUserIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();


		switch(m_TitleStorage)
		{
			case eTitleStorageInit:
			{
				TitleStorageStopAndCleanup();
				m_TitleStorage = eTitleStorageSetDownload;

				//start the download process
				m_pReader = new NetPlay::C_PublisherServerStorageReader;
				m_pTSBuffer = new U8[TITLE_STORAGE_BUFFER_SIZE];

				if ( NULL == m_pReader || NULL == m_pTSBuffer )
				{	
					LOGERROR( g_Log, "Title storage download failed.");
					TitleStorageStopAndCleanup();
					m_TitleStorage = eTitleStorageError;
				}

				break;
			}

			case eTitleStorageSetDownload:
			{
				if( eTitleStorageFilesMax == m_CurrFileDownload )
				{
					LOGINFO( g_Log, "Title storage downloads have completed.");
					TitleStorageStopAndCleanup();			
					m_TitleStorage = eTitleStorageCompleted;
					m_TitleStorageTimeoutTimer.reset();
					m_TitleStorageRefreshTimer.reset();
					m_TitleStorageRefreshTimer.start();
					return;
				}

				const SysProfile::FullProfile& localPlayer = UI::C_ProfileInterface::Instance().GetPadProfile(primaryUserIndex);
				if (false == m_pReader->Start(localPlayer, m_TitleStorageFiles[m_CurrFileDownload], m_pTSBuffer, sizeof( m_pTSBuffer ) * TITLE_STORAGE_BUFFER_SIZE))
				{
					TitleStorageStopAndCleanup();
					m_TitleStorage = eTitleStorageError;
					return;
				}
				m_TitleStorage = eTitleStorageDownloading;
				m_TitleStorageTimeoutTimer.start();

				break;
			}

			case eTitleStorageDownloading:
			{
				//check to see if the file has downloaded
				U32 numBytesDownloaded = 0;

				char* tsFile = m_TitleStorageFiles[m_CurrFileDownload];

				switch (m_pReader->Continue(numBytesDownloaded))
				{
					case NetPlay::C_PublisherServerStorageReader::eReadInProgress:
						if (m_TitleStorageTimeoutTimer.getElapsedTimeInSeconds() >= 10)
						{
							//error, don't let the user go online
							LOGERROR( g_Log, "Timedout downloading '%s' from Title Storage.", tsFile );
							m_TitleStorage = eTitleStorageError;
							TitleStorageStopAndCleanup();			
						}
						return;

					case NetPlay::C_PublisherServerStorageReader::eReadSuccessful:
						//load the downloaded file
						if( true == TitleStorageLoad(m_CurrFileDownload, numBytesDownloaded) )
						{				
							LOGINFO( g_Log, "Downloaded file '%s' from Title Storage was loaded successfully.", tsFile );
							++m_CurrFileDownload;
							m_TitleStorage = eTitleStorageSetDownload;
						}
						else
						{
							LOGERROR( g_Log, "Downloaded file '%s' from Title Storage failed to load.", tsFile );
							m_TitleStorage = eTitleStorageError;
							TitleStorageStopAndCleanup();
						}
						break;

					case NetPlay::C_PublisherServerStorageReader::eReadFileDoesNotExist:
						//no file in title storage, one on the disc must be ok
						LOGINFO( g_Log, "No File '%s' on Title Storage, file on disc must be ok.", tsFile );
						++m_CurrFileDownload;
						m_TitleStorage = eTitleStorageSetDownload;
						break;

					default:
						//error, don't let the user go online
						LOGERROR( g_Log, "Failed to download '%s' from Title Storage.", tsFile );
						m_TitleStorage = eTitleStorageError;
						TitleStorageStopAndCleanup();					
						break;
				}			

				break;
			}
		}
	}

	static void SwapEndian(U32 &val)
	{
		val = (val<<24) | ((val<<8) & 0x00ff0000) |
			((val>>8) & 0x0000ff00) | (val>>24);
	}

	bool C_NetServices::TitleStorageLoad( U8 currFileDownload, U32 numBytesDownloaded )
	{
		U32 decompressedSize(0);
		memcpy(&decompressedSize, m_pTSBuffer, sizeof(U32));
		SwapEndian( decompressedSize );

		if( TITLE_STORAGE_BUFFER_SIZE * 2 < decompressedSize )
		{
			LOGERROR( g_Log, "Decompressed size of downloaded file is too big Title Storage." );
			return false;
		}

		U8* pDecompressedBuffer = new U8[decompressedSize];

		CCompression compress;
		bool ret = compress.Decompress( &m_pTSBuffer[sizeof(U32)]
							, numBytesDownloaded - sizeof(U32)
							, pDecompressedBuffer
							, decompressedSize );
		if( false == ret )
		{
			LOGERROR( g_Log, "Failed to decompress download file from Title Storage." );
		}
		else
		{
			switch ( currFileDownload )
			{
				case eMultiplayer_Pack:
					if (g_debugEnableMultiplayerPakDownload)
					{
						ret = C_MultiplayerData::Instance().LoadMultiplayerPackData( pDecompressedBuffer, decompressedSize, true );
						pDecompressedBuffer = NULL;
					}
					break;
				case eMultiplayerTS_Loc:
					{
						const char* pLanguageCode = UI::C_UILocaliser::Instance().GetLanguageCode();

						ret = UI::C_TextStringID::LocTable().LoadMultiplayerTSFromMemory( pDecompressedBuffer, decompressedSize, pLanguageCode );
						break;
					}
			}

			if( false == ret )
			{
				LOGERROR( g_Log, "Failed to load decompressed downloaded file from Title Storage." );
			}
		}

		delete[] pDecompressedBuffer;

		return ret;		
	}


	void C_NetServices::TitleStorageStopAndCleanup(void)
	{
		m_CurrFileDownload = 0;

		if (m_pReader)
		{
			m_pReader->StopAndCleanup();
			delete m_pReader;
			m_pReader = NULL;
		}

		if (m_pTSBuffer)
		{
			delete[] m_pTSBuffer;
			m_pTSBuffer = NULL;
		}
	}

	void C_NetServices::InitialiseTitleStorage( Btl::Vector<Btl::String> &filesToDownLoad )
	{
		m_TitleStorage = ( true == g_DebugUseTitleStorage ) ? eTitleStorageInit : eTitleStorageCompleted; 

		if( false == m_TitleStorageFilesInit)
		{		
			BIZ_ASSERT(filesToDownLoad.Size()==2);

			strcpy_s(m_TitleStorageFiles[eMultiplayer_Pack], lengthof(m_TitleStorageFiles[eMultiplayer_Pack]), filesToDownLoad[0].Chars());
			strcpy_s(m_TitleStorageFiles[eMultiplayerTS_Loc], lengthof(m_TitleStorageFiles[eMultiplayerTS_Loc]), filesToDownLoad[1].Chars());

			m_TitleStorageFilesInit = true;
		}
	}

	void C_NetServices::RefreshLocalPlayerName( const WCHAR* pName )
	{
		if ( !m_Initialised || !m_pPlayerManager )
			return;

		// Only run if NetRace is active
		if ( !m_pNetRace || !m_pNetRace->IsRunning() )
			return;

		for ( U8 i = 0; i < m_pPlayerManager->TotalSize(); i++ )
		{
			// Find the local player and update his user name
			C_NetPlayer* pPlayer = m_pPlayerManager->Index( i );
			if ( pPlayer && pPlayer->LocalPlayer() )
			{
				UI::WideStrCopy( pPlayer->m_User.m_Name, SYSTEM_PROFILE_NAME_LENGTH, pName );
				break;
			}
		}
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
