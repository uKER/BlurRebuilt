#include "stdafx.h"
#include <GameLogic/Layers/RaceBookLayer.h>	// [Blur2PC]

#include <ui/Game/UILUA.h>

#include <Game/Game.h>
#include <GameLogic\profile\GameProfileManager.h>
#include <Network/SessionDeletion.h>
#include <Network/SessionEnumerator.h>
#include <Network/Presence.h>
#include <SystemProfile/SystemProfile.h>
#include <SystemProfile/SystemProfileManager.h>
#include <SystemProfile/GameInvite.h>
#include <ui/Game/UILua.h>
#include <ui/Game/Profiles/ProfileInterface.h>
#include <GameLogic/GameData/MultiplayerData.h>

#include <Network/Misc/NetDefines.h>
#include <Network/Core/NetCreationHelper.h>
#include <Network/Core/NetMatchingParameters.h>
#include <Network/Core/NetServices.h>
#include "Network/Core/NetRace.h"
#include <Network/Core/NetRichPresence.h>
#include <Network/Misc/NetQoS.h>
#include <Network/Misc/NetVersion.h>
#include <Network/Core/NetLogger.h>
#include <Network/Router/NetRouterBase.h>
#include <Network/Player/NetPlayer.h>
#include <Network/Player/NetPlayerManager.h>

// Might want to take these out if they arnt needed
#include <network/PerUserServerStorageReader.h>
#include <network/PerUserServerStorageWriter.h>

#include <GameApp.h>

#include <GameLogic/Structures/LevelState.h>
#include <Gamelogic\ScoreboardManager.h>
#include <GameLogic\SocialNetworking\SocialNetworkManager.h>

#if !defined ( CFG_GOLD )
Debug::Bool g_EntryRequirements_ProfileCount("Network.Race.EntryRequirements.ForceTooManyProfiles", false);
Debug::Bool g_EntryRequirements_NoOnline("Network.Race.EntryRequirements.ForceNonOnlineAccount", false);
Debug::Bool g_EntryRequirements_Parental("Network.Race.EntryRequirements.ForceParentalControls", false);
#endif

#ifndef CFG_GOLD
namespace RacerNet
{
	extern Debug::Enum<eNetBuildVersion> NET_PACKET_VERSION;
}
#endif

namespace Game
{
	static S32 StopScoreboardRequests(lua_State* pState)
	{
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			ns.EnterLock();
			ns.StopScoreboardRequests();
			ns.Exitlock();
		}
		return 0;
	}

	static S32 FlushEnumertedSessions(lua_State* pState)
	{
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			ns.EnterLock();
			ns.MatchmakingSessionEnumerator().KillSearch();
			ns.Exitlock();
		}
		return 0;
	}

	static S32 StopEnumertedSessions(lua_State* pState)
	{
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			ns.EnterLock();
			ns.MatchmakingSessionEnumerator().StopSearch();
			ns.Exitlock();
		}
		return 0;
	}

	static S32 SesionDeletionInProgress(lua_State* pState)
	{
		bool deleting = false;
		NetPlay::C_SessionDeletion& del = NetPlay::C_SessionDeletion::Instance();
		if (del.IsActive())
			deleting = del.ActiveCount() > 0 ? true : false;
		lua_pushboolean(pState, deleting);
		return 1;
	}

	static S32 OnlineConnectionPossible(lua_State* pState)
	{
		bool possible = false;

		const S32 primaryUserIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		const U32 numProfiles = UI::C_ProfileInterface::Instance().NumActive();
		BIZ_ASSERT_MSG(primaryUserIndex != -1, "No primary user defined.");
		BIZ_ASSERT_MSG(numProfiles != 0, "No active game profiles");

#if defined ( PLATFORM_PS3 )
		// for ps3 if the primary user has a psn account created
		if (-1 != primaryUserIndex && numProfiles > 0)
			possible = SysProfile::C_Manager::Instance().NetworkPlatformAccountAvailable();

#elif defined ( PLATFORM_PC )
		// for pc has the user already created a demonware account, and we have username + password stored somewhere.
		if (-1 != primaryUserIndex && numProfiles > 0)
		{
			char* userName = Game::C_GameProfileManager::Instance().GetDemonwareUserName();
			char* password = Game::C_GameProfileManager::Instance().GetDemonwarePassword();
			if (userName != NULL && password != NULL)
				possible = true;
		}
#endif

		lua_pushboolean(pState, possible);
		return 1;
	}

	static S32 Connect(lua_State* pState)
	{
		bool started = false;
		if (lua_isboolean(pState,1))
		{
			const bool online = lua_toboolean(pState, 1) ? true : false;

			RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
			ns.EnterLock();
			if (ns.IsInitialised())
			{
				char* userName = Game::C_GameProfileManager::Instance().GetDemonwareUserName();
				char* password = Game::C_GameProfileManager::Instance().GetDemonwarePassword();

				started = ns.Router().PlatformConnect(online, userName, password); 
			}

			ns.Exitlock();
		}
		lua_pushboolean(pState, started);
		return 1;
	}

	static S32 Disconnect(lua_State* pState)
	{
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			ns.EnterLock();
			ns.Router().PlatformDisconnect();
			ns.Exitlock();
		}
		return 0;
	}

	static S32 ConnectionStatus(lua_State* pState)
	{
		S32 state = RacerNet::eConnectionStateNotActive;
		S32 result = RacerNet::eConnectionResultNone;
		bool online_mode = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{	
			ns.EnterLock();
			state = ns.Router().GetPlatformConnectionState();
			result = ns.Router().GetLastConnectionErrorCode();
			online_mode = ns.Router().IsOnlineMode();
			ns.Exitlock();
		}

		lua_pushnumber(pState, state);
		lua_pushnumber(pState, result);
		lua_pushboolean(pState, online_mode);
		return 3;
	}

	static S32 ConnectionStatusIsOnline( lua_State* pState )
	{
		bool online_mode = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{	
			ns.EnterLock();
			S32 state = ns.Router().GetPlatformConnectionState();

			// [WillB] #37418 Only if we have a valid connection and its in online mode
			online_mode = ( state != RacerNet::eConnectionStateNotActive && state != RacerNet::eConnectionStateFailed && ns.Router().IsOnlineMode() );
			ns.Exitlock();
		}

		lua_pushboolean(pState, online_mode);
		return 1;
	}

	static S32 StartBandwidthEvaluation(lua_State* pState)
	{
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			if (ns.Router().GetPlatformConnectionState() == RacerNet::eConnectionStateComplete && ns.Router().IsOnlineMode())
				ns.Router().StartBandwidthEvaluation();
		}
		return 0;
	}

	static S32 BandwidthEvaluationRunning(lua_State* pState)
	{
		bool running = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
			running = ns.Router().BandwidthEvaluationInProgress();
		lua_pushboolean(pState, running);
		return 1;
	}

	static S32 StartOnlineSessionEnumerationFromInvitation(lua_State* pState)
	{
		bool started = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			SysProfile::GameInvite invCopy = ns.Router().GetGameInvite(); // take a copy of invite
			if (invCopy.m_Active)
			{
				// clear stored invite now as we are about to process it.
				ns.Router().GetGameInvite().m_Active = false;

				NetPlay::C_SessionEnumerator& enumerator = ns.MatchmakingSessionEnumerator();

				// flush old data.
				ns.MatchingRemapList().Clear();
				enumerator.KillSearch();

				RacerNet::C_NetCreationHelper& helper = ns.SessionCreationHelper();
				if (helper.BuildLocalPlayersForOnline(ns.Router().GetNatType(), 0, 0, 0)) // parameters are ignored, we only want the main player
				{
					NetPlay::SesionQueryReturnTypes platformSearchReturnTypes;
					RacerNet::BuildMatchingReturnTypes(platformSearchReturnTypes);
					const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(helper.SessionManagerIndex());

					started = enumerator.StartSearch(	fp, 
														invCopy,
														platformSearchReturnTypes, 
														true);
				}
			}
		}

		lua_pushboolean(pState, started);
		return 1;
	}

	static S32 ContinueOnlineSessionEnumerationFromInvitation(lua_State* pState)
	{
		bool finished = true;
		bool error = true;
		bool raceInvite = true;
		bool rankedMode = false;
		bool customRace = false;
		U32	eventID = 0;
		int versionNumDiff = 0;  //1 = your playlist ver < host; 2 = host playlist ver < yours;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			NetPlay::C_SessionEnumerator& enumerator = services.MatchmakingSessionEnumerator();
			enumerator.Poll();
			if (enumerator.IsSearching())
				finished = false;
			else
			{
				// only sessions we can actually join get pushed into our remap list.
				services.MatchingRemapList().Clear();
				if (enumerator.NumSessionsEnumerated() == 1)
				{
					NetPlay::C_SessionEnumerator::SessionQoSData qos;
					if (enumerator.GetQoSDataFromEnumerated(0, qos))
					{
						if (NULL != qos.m_pQoSData && qos.m_QoSDataSize >= sizeof(RacerNet::NetQoSBase))
						{
							const RacerNet::NetQoSBase* pBaseQos = reinterpret_cast<RacerNet::NetQoSBase*> (qos.m_pQoSData);
							if (RacerNet::NET_PACKET_VERSION == pBaseQos->m_Version)
							{
								if (RacerNet::NetQoSBase::QoSTypeRace == pBaseQos->m_Type)
								{
									bool allow = true;

#if defined ( LSP_GROUPS_IS_CUSTOM_CONTENT )
									// is this game user create content. if so we chack against the loacl players to make sure their
									// privileges allow this.
									if (pBaseQos->m_UserCreated != 0)
									{
										RacerNet::C_NetCreationHelper& helper = services.SessionCreationHelper();
										if (helper.NumPlayers() > 0)
											allow = helper.CanUseUserCreatedContent(); // we could return another error if we wish the ui to explain why the session could not be joined.
										else
										{
											LOGERROR(RacerNet::g_Log, "game invite : session creation helper is invalid.");
											allow = false;
										}
									}
#endif
									if (allow)
									{
										S32 matchingVersion = pBaseQos->m_PackVersion >> 16;
										S32 packVersion = pBaseQos->m_PackVersion & 0x0000ffff;
										if (C_MultiplayerData::Instance().m_pConfig->m_PackVersion == packVersion && 
											C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion == matchingVersion)
										{
											services.MatchingRemapList().PushBack(0);
											error = false;
											raceInvite = true;

											const RacerNet::NetQoSRace* pRaceQos = reinterpret_cast<RacerNet::NetQoSRace*> (qos.m_pQoSData);
											eventID = pRaceQos->m_Race.m_EventID;
											rankedMode = ( pRaceQos->m_Race.m_Ranked > 0 ) ? true : false;
											customRace = ( pRaceQos->m_Race.m_CustomRace > 0 ) ? true : false;

											LOGINFO(RacerNet::g_Log, "game invite : race session enumerated.");
										}
										else
										{
											LOGINFO(RacerNet::g_Log, "game invite : race session enumerated but not joinable due to invalid data version.");
											RacerNet::C_NetServices::Instance().TitleStorageStatusReset();

											if	(C_MultiplayerData::Instance().m_pConfig->m_PackVersion < packVersion ||
												 C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion < matchingVersion)
											{												
												LOGINFO(RacerNet::g_Log, "Your playlist data is older than the host's. You must restart the game or reconnect to Live in order to get the new playlist data.");
												versionNumDiff = 1;
											}
											else
											{
												LOGINFO(RacerNet::g_Log, "Your playlist data is newer than the host's. You will be unable to play with them until they restart to get the new playlists.");
												versionNumDiff = 2;
											}
										}
									}
									else
									{
										LOGINFO(RacerNet::g_Log, "game invite : race session enumerated but not joinable due to privileges.");
									}
								}
								else if (RacerNet::NetQoSBase::QoSTypeParty == pBaseQos->m_Type)
								{
									S32 matchingVersion = pBaseQos->m_PackVersion >> 16;
									S32 packVersion = pBaseQos->m_PackVersion & 0x0000ffff;
									if (C_MultiplayerData::Instance().m_pConfig->m_PackVersion == packVersion && 
										C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion == matchingVersion)
									{
										services.MatchingRemapList().PushBack(0);
										error = false;
										raceInvite = false;									
										LOGINFO(RacerNet::g_Log, "party invite : party session enumerated.");
									}
									else
									{
										LOGINFO(RacerNet::g_Log, "party invite : race session enumerated but not joinable due to invalid data version.");
										RacerNet::C_NetServices::Instance().TitleStorageStatusReset();

										if(C_MultiplayerData::Instance().m_pConfig->m_PackVersion < pBaseQos->m_PackVersion)
										{												
											LOGINFO(RacerNet::g_Log, "Your playlist data is older than the host's. You must restart the game or reconnect to Live in order to get the new playlist data.");
											versionNumDiff = 1;
										}
										else
										{
											LOGINFO(RacerNet::g_Log, "Your playlist data is newer than the host's. You will be unable to play with them until they restart to get the new playlists.");
											versionNumDiff = 2;
										}
									}
								}
							}
						}
					}
				}
				
				if (error)
					enumerator.KillSearch();
			}
		}
		
		lua_pushboolean(pState, finished);
		lua_pushboolean(pState, error);
		lua_pushboolean(pState, raceInvite);
		lua_pushboolean(pState, rankedMode);
		lua_pushboolean(pState, customRace);
		lua_pushnumber(pState, eventID);
		lua_pushnumber(pState, versionNumDiff);

		return 7;
	}

	static S32 SetRichPresence(lua_State* pState)
	{
		if (!lua_isnumber(pState,1))
			return 0;

		const U32 value = (U32)lua_tointeger(pState, 1);

		RacerNet::C_NetServices::Instance().EnterLock();

		// Join in progress is disabled when its a private game.
		const bool ranked = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsRanked();
		
		// user not really applicable on ps3 and Pc.
		SysProfile::C_Manager::Instance().SetPresence(0, static_cast<RacerNet::eRichPresence> (value), ranked );
		RacerNet::C_NetServices::Instance().Exitlock();

		return 0;
	}

	static S32 SetRichPresenceSplitScreen(lua_State* pState)
	{
		if ( lua_type( pState, 1 ) != LUA_TNUMBER )
			return 0;

		S32 sPad = (S32)lua_tonumber( pState, 1 );

		RacerNet::C_NetServices::Instance().EnterLock();

		SysProfile::C_Manager::Instance().SetPresence(0, RacerNet::eRichPresence_SplitScreen, false);
		RacerNet::C_NetServices::Instance().Exitlock();

		return 0;
	}

	static S32 ShowSystemCommunityUI(lua_State* pState)
	{
		return 0;
	}

	static S32 GameInviteBeingSent(lua_State* pState)
	{
		bool active	= SysProfile::C_Manager::Instance().SendingGameInvite();
		lua_pushboolean(pState, active);
		return 1;
	}

	static S32 GameInvitePending(lua_State* pState)
	{
		bool pending = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			SysProfile::GameInvite& inv = ns.Router().GetGameInvite();
			if (SysProfile::C_Manager::Instance().ServiceConnectionState() == SysProfile::C_Manager::eServiceStateConnected)
				pending = inv.m_Active;
			else
				inv.m_Active = false; // no online connection so game invites cant be processed.
		}
		lua_pushboolean(pState, pending);
		return 1;
	}

	static S32 ClearGameInvite(lua_State* pState)
	{
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		SysProfile::GameInvite blank;
		ns.AcceptGameInvite(blank);
		return 0;
	}

	static S32 FindGameInvitePadIndex( lua_State* pState )
	{
		// Default it to the primary players pad.
		S32 padIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();

		// Need to check weather the game invite is for the primary player
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			SysProfile::GameInvite& inv = ns.Router().GetGameInvite();
			if (SysProfile::C_Manager::Instance().ServiceConnectionState() == SysProfile::C_Manager::eServiceStateConnected )
			{
				if ( inv.m_Active == true ) // need an online connection to proceed
				{
#if defined(LIVE_ON_WINDOWS)
					SysProfile::UserIdentifier userID;
					userID.m_Uid = inv.m_InviteData.xuidInvitee;
					userID.m_IsGuest = false;

					padIndex = UI::C_ProfileInterface::Instance().GetPadIndexFromID( userID );
#endif // PLATFORM_XENON
				}
			}
		}
		lua_pushnumber(pState, padIndex);	

		return 1;
	}

	static S32 IsInExternalPartySession(lua_State* pState)
	{
		// local player in nxe party session ?.
		// only applicable to 360, other platforms will always return false.
		bool inExternal = SysProfile::C_Manager::Instance().IsInExternalParty(1);
		lua_pushboolean(pState, inExternal);
		return 1;
	}

	static S32 SendGameInviteToExternalPartySession(lua_State* pState)
	{
		bool ok = false;
	
		if (lua_isnumber(pState,1))
		{
			// async task is pumped to completion in the uimanager..
			const U32 userIndex = (U32)lua_tointeger(pState, 1);
			ok = SysProfile::C_Manager::Instance().StartSendExternalPartyInvite(userIndex, 1);
		}

		lua_pushboolean(pState, ok);
		return 1;
	}

	/** return values
	*	0 = ok to enter mode
	*	1 = too many players active
	*	2 = player does not have an online account
	*	3 = parental controls say player cant enter mode
	*	4 = no connection to service
	*	5 = generic error
	*/
	static S32 CheckEntryRequirements(lua_State* pState)
	{
		S32 ret = 0;
		if (lua_isboolean(pState,1))
		{
			const bool online = lua_toboolean(pState, 1) ? true : false;

			const S32 primaryUserIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
			BIZ_ASSERT(-1 != primaryUserIndex);

			const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(primaryUserIndex);

			// all platforms only support 1 player over the network game.
			//if (UI::C_ProfileInterface::Instance().NumActive() != 1)
			//	ret = 1;
			//else
			{
#if defined ( PLATFORM_PS3 )
				
				if (online)
				{
					if (!SysProfile::C_Manager::Instance().NetworkPlatformAccountAvailable())
						ret = 2;
				}

#elif defined ( PLATFORM_PC )
				
				if (online)
				{
					char* userName = Game::C_GameProfileManager::Instance().GetDemonwareUserName();
					char* password = Game::C_GameProfileManager::Instance().GetDemonwarePassword();
					if (userName == NULL || password == NULL)
						ret = 2;
				}
#else
				// unknown platform type.
				ret = 4;
#endif
			}
		}
		else
			ret = 4;

#if !defined ( CFG_GOLD )
		if (g_EntryRequirements_ProfileCount)
			ret = 1;
		else if (g_EntryRequirements_NoOnline)
			ret = 2;
		else  if( g_EntryRequirements_Parental)
			ret = 3;
#endif

		lua_pushnumber(pState, ret);
		return 1;
	}

	static S32 CanViewGamerCard(lua_State* pState)
	{
		bool viewable = false;

		lua_pushboolean(pState, viewable);
		return 1;
	}

	/* parameters 
		1: controller port of player wants to view
		2: join id of player who's gamercard should be viewed
		3: true if the join id is for the party session or false for race session
		4: true if in race or false if in main ui
	*/ 
	static S32 ViewGamerCard(lua_State* pState)
	{
		return 0;
	}

	static S32 ViewScoreboardGamerCard(lua_State* pState)
	{
		return 0;
	}

	static S32 ViewSessionScoreboardGamerCard(lua_State* pState)
	{
		return 0;
	}

	static S32 ViewRaceSummaryGamerCard(lua_State* pState)
	{
		return 0;
	}

	static S32 CanSubmitPlayerReview(lua_State* pState)
	{
		bool review = false;

		lua_pushboolean(pState, review);
		return 1;
	}

	/* parameters 
	 1: controller port of player wants to submit
	 2: join id of player who we are reviewing
	 3: true if in race or false if in race lobby
	*/ 
	static S32 SubmitPlayerReview(lua_State* pState)
	{
		return 0;
	}

	// pass in controller index of local player to see if quick mute functionality can be used.
	static S32 QuickMuteAvailable(lua_State* pState)
	{
		bool qmAvailable = false;

		if ( lua_type( pState, 1 ) == LUA_TNUMBER && lua_type( pState, 2 ) == LUA_TNUMBER && lua_type( pState, 3 ) == LUA_TBOOLEAN )
		{
			const U8 userIndex = (U8)lua_tointeger(pState, 1);
			const U8 remoteRef = (U8)lua_tointeger(pState, 2);
			const bool partyLobby = lua_toboolean(pState, 3) ? true : false;

			RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
			if (ns.IsInitialised())
			{
				bool done = false;
				for (U8 i=0; i<ns.PlayerManager().TotalSize() && !done; i++)
				{
					RacerNet::C_NetPlayer* p = ns.PlayerManager().Index(i);
					if (p->LocalPlayer() && userIndex == (U8)p->m_ControllerPort)
					{
						done = true;

						qmAvailable = !p->m_VoiceBanned;

						// cant view yourself
						const U8 joinId = ( partyLobby ) ? p->m_PartyConnection.m_JoinId : p->m_RaceConnection.m_JoinId;

						qmAvailable = ( remoteRef == joinId ) ? false : qmAvailable;
					}
				}
			}
		}

		lua_pushboolean(pState, qmAvailable);
		return 1;
	}

	/* parameters 
		1: controller port of local player
		2: join id of remote player who we are checking the qquick mute for
		3: true if the join id is from the party session or false for race session
	*/ 
	static S32 IsRemotePlayerQuickMuted(lua_State* pState)
	{
		bool isQm = false;

		if (lua_isnumber(pState, 1) && lua_isnumber(pState, 2) && lua_isboolean(pState, 3))
		{
			const U8 userIndex = (U8)lua_tointeger(pState, 1);
			const U8 joinRef = (U8)lua_tointeger(pState, 2);
			const bool partySession = lua_toboolean(pState, 3) ? true : false;

			RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
			if (ns.IsInitialised())
			{
				RacerNet::C_NetPlayer* p = ns.PlayerManager().FindFromJoinId(joinRef, partySession);
				if (p)
				{
					if (p->m_QuickMute&(1<<userIndex))
						isQm = true;
				}
			}
		}

		lua_pushboolean(pState, isQm);
		return 1;
	}

	static S32 GetVoiceMap( lua_State* pState )
	{
		if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
		{
			UI::g_pUILua->StackDump( pState, "GetVoiceMap(bool) - incorrect arguments\n");
			return 0;
		}

		const bool party = ( lua_toboolean( pState, 1 ) ) ? true : false; // party or race connection?

		lua_newtable( pState );

		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if ( ns.IsInitialised() )
		{
			for ( U8 i = 0; i < ns.PlayerManager().TotalSize(); i++ )
			{
				RacerNet::C_NetPlayer* pPlayer = ns.PlayerManager().Index( i );

				lua_newtable( pState );
					Lua::AddDataMember( pState, "speaking", ( pPlayer->m_IsSpeaking > 0.0f ) ? true : false );
					Lua::AddDataMember( pState, "active", pPlayer->m_HasCommunicator );
					Lua::AddDataMember( pState, "muted", ( pPlayer->m_QuickMute != 0 ) ? true : false );
				lua_rawseti( pState, -2, ( party ) ? pPlayer->m_PartyConnection.m_JoinId : pPlayer->m_RaceConnection.m_JoinId );
			}
		}
		return 1;
	}

	/* parameters 
		1: controller port of player wants toggle the mute
		2: join id of remote player who we are checking the qquick mute for
		3: true if the join id is from the party session or false for race session
	*/ 
	static S32 ToggleRemotePlayerQuickMute(lua_State* pState)
	{		
		if (lua_isnumber(pState, 1) && lua_isnumber(pState, 2) && lua_isboolean(pState, 3))
		{
			const U8 userIndex = (U8)lua_tointeger(pState, 1);
			const U8 joinRef = (U8)lua_tointeger(pState, 2);
			const bool partySession = lua_toboolean(pState, 3) ? true : false;

			RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
			if (ns.IsInitialised())
				ns.PlayerManager().QuickMuteToggle(userIndex, joinRef, partySession);
		}

		return 0;
	}

	static S32 CanUseChat( lua_State* pState )
	{
		const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
		if (!fp.m_CommunicationsPrivilege)
		{
#if defined (PLATFORM_PS3)	//this is a PS3 only popup that appears from this check
			lua_pushboolean(pState, false);
#else
			lua_pushboolean(pState, true);
#endif
		}
		else
		{
			lua_pushboolean(pState, true);
		}

		return 1;
	}

	static S32 AllowPlayerCountUpdate( lua_State* pState )
	{
		RacerNet::C_NetServices::Instance().AllowPlayerCountUpdate();
		return 0;
	}

	static S32 StopPlayerCountUpdate( lua_State* pState )
	{
		RacerNet::C_NetServices::Instance().StopPlayerCountUpdate();
		return 0;
	}


	static S32 TitleStorageStatus( lua_State* pState )
	{
		RacerNet::C_NetServices::Instance().TitleStorageUpdate();
		lua_pushnumber(pState, RacerNet::C_NetServices::Instance().TitleStorageStatus());
		return 1;
	}

	static S32 InitTimeOutTimer( lua_State* pState )
	{
		RacerNet::C_NetServices::Instance().ResetTimeOutTimer();

		return 0;
	}

	static S32 SetMatchingRegionInfo( lua_State* pState )
	{
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if ( !ns.IsInitialised() )
			return 0;

		// Set matching rule
		const RacerNet::eMatchingRule rule = (RacerNet::eMatchingRule)C_GameProfileManager::Instance().GetPrimaryProfile()->mOptions.mMatchingRule;

		if ( rule == RacerNet::eMatchingRuleAny )
		{
			ns.SetMatchingRegionInfo( false, RacerNet::eMatchingRegionUnknown );
		}
		else
		{
			ns.SetMatchingRegionInfo( ( rule == RacerNet::eMatchingRuleEnforced ) ? true : false, ns.Router().GetMatchingRegion() );
		}

		// Update this too
		bool joinSessionsInRace = true;
		if (C_MultiplayerData::Instance().m_pConfig)
			joinSessionsInRace = C_MultiplayerData::Instance().m_pConfig->m_JoinSessionsInRace;
		ns.SetJoinSessionsInRace( joinSessionsInRace );

		return 0;
	}

	static S32 MpBetaExpired(lua_State* pState)
	{
		bool disabled = C_MultiplayerData::Instance().m_pConfig->m_OnlineModeDisabled;

		lua_pushboolean(pState, disabled);
		return 1;
	}

	static S32 GetPartyMap(lua_State* pState)
	{
		lua_newtable( pState );

		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if ( ns.IsInitialised() )
		{
			for ( U8 i = 0; i < ns.PlayerManager().TotalSize(); i++ )
			{
				RacerNet::C_NetPlayer* pPlayer = ns.PlayerManager().Index( i );

				lua_newtable( pState );
					Lua::AddDataMember( pState, "active", pPlayer->m_PartyConnection.IsActive() );
					Lua::AddDataMember( pState, "leader", pPlayer->m_PartyConnection.m_Host );
				lua_rawseti( pState, -2, pPlayer->m_RaceConnection.m_JoinId );
			}
		}
		return 1;
	}

	static S32 GetNatType( lua_State* pState )
	{
		RacerNet::eNatType type = RacerNet::eNatStrict;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if ( ns.IsInitialised() )
			type = ns.Router().GetNatType();

		lua_pushnumber( pState, type );

		return 1;
	}

	static S32 GetSystemRegion( lua_State* pState )
	{
		RacerNet::eMatchingRegion region = RacerNet::eMatchingRegionUnknown;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if ( ns.IsInitialised() )
			region = ns.Router().GetMatchingRegion();

		lua_pushnumber( pState, region );

		return 1;
	}

	static S32 PeerRoutingEnabled(lua_State* pState)
	{
		bool enabled = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
			enabled = ns.Router().PeerRoutingEnabled();

		lua_pushboolean(pState, enabled);
		return 1;
	}

	static S32 StartServerTimeStampTask(lua_State* pState)
	{
		bool finished = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			finished = ns.StartServerTimeTask();
		}

		lua_pushboolean(pState, finished);
		return 1;
	}

	static S32 ShowTotalPlayerCount( lua_State* pState )
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();

		lua_pushboolean( pState, services.ShowPlayerCountData() );

		return 1;
	}

	static S32 ShowPlaylistPlayerCount( lua_State* pState )
	{
		if ( lua_type( pState, 1 ) != LUA_TNUMBER )
		{
			UI::g_pUILua->StackDump( pState, "ShowPlaylistPlayerCount(index) - incorrect arguments\n");
			return 0;
		}

		const S32 index = (S32)lua_tonumber( pState, 1 );
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		
		// Check against the minimal amount
		lua_pushboolean( pState, services.GetRankedEventPlayerCount( index ) > C_MultiplayerData::Instance().m_pConfig->m_MinPlaylistPlayerShowCount );

		return 1;
	}

	static S32 NetworkConnectionActive( lua_State* pState )
	{
		bool result = NetPlay::C_Core::Instance().ValidLocalAddress();

		lua_pushboolean( pState, result );
		return 1;
	}

	static S32 UserHasAgeRestriction( lua_State* pState )
	{
		bool result = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			ns.EnterLock();
			result = ns.Router().CheckOnlineAgeRestriction();
			ns.Exitlock();
		}

		lua_pushboolean( pState, !result );
		return 1;
	}

	/*--------------------------------------------------------------------------------------
		User storage functions
	--------------------------------------------------------------------------------------*/
	static S32 StartProfileUpload( lua_State* pState )
	{
		// Get the active players profile.
		SysProfile::FullProfile profile;
		SysProfile::C_Manager::Instance().GetProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo(), profile );

		const Game::E_MPS_Result result = C_GameProfileManager::Instance().GetPrimaryProfile()->UploadMultiplayerProfile( profile );

		lua_pushnumber( pState, result );
		return 1;
	}

	static S32 ContinueProfileUpload( lua_State* pState )
	{
		Game::E_MPS_Result result;
		C_GameProfileManager::Instance().GetPrimaryProfile()->IsUploadMultiplayerProfileFinished( result );

		lua_pushnumber( pState, result );
		return 1;
	}

	static S32 StartProfileDowload( lua_State* pState )
	{
		// Get the active players profile.
		SysProfile::FullProfile profile;
		SysProfile::C_Manager::Instance().GetProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo(), profile );

		const Game::E_MPS_Result result = C_GameProfileManager::Instance().GetPrimaryProfile()->DownloadMultiplayerProfile( profile, profile );

		lua_pushnumber( pState, result );
		return 1;
	}

	static S32 ContinueProfileDownload( lua_State* pState )
	{
		Game::E_MPS_Result result;
		C_GameProfileManager::Instance().GetPrimaryProfile()->IsDownloadMultiplayerProfileFinished( result );

		lua_pushnumber( pState, result );
		return 1;
	}

	static S32 UserProfileDownloaded( lua_State* pState )
	{
		const bool result = C_GameProfileManager::Instance().GetPrimaryProfile()->HasValidDownloadedMPProfile();
		lua_pushboolean( pState, result );
		return 1;
	}

	static S32 ApplyDownloadedProfile( lua_State* pState )
	{
		Game::E_MPS_Result result = C_GameProfileManager::Instance().GetPrimaryProfile()->ApplyMultiplayerProfile();
		
		lua_pushnumber( pState, result );
		return 1;
	}

	static S32 ResetMultiplayerProfile( lua_State* pState )
	{
		C_GameProfileManager::Instance().GetPrimaryProfile()->GetMultiplayerProfile()->Init();
		return 0;
	}
	
	/**********************************************************************************************************************************************************************/
	/**********************************************************************************************************************************************************************/
	static const luaL_reg net_services_lib[] = 
	{
		{"StopScoreboardRequests",							StopScoreboardRequests},						// stop any active scoreboard reads / writes.
		{"FlushEnumertedSessions",							FlushEnumertedSessions},						// releses the memory taken by the online session enumeration.
		{"SesionDeletionInProgress",						SesionDeletionInProgress},						// is an online network session in the process of being deleted.
		{"StopEnumertedSessions",							StopEnumertedSessions},							// stops the current enumeration and keeps the games around for joining.

		{"OnlineConnectionPossible",						OnlineConnectionPossible},						// should we actually attempt to connect to the platform servers ?.
		{"Connect",											Connect},										// start a connection attempt to the game servers.
		{"Disconnect",										Disconnect},									// totally disconnect from the game servers.
		{"ConnectionStatus",								ConnectionStatus},								// get the current connection status to the platform servers.
		{"ConnectionStatusIsOnline",						ConnectionStatusIsOnline},						// get the online state of the connection to the platform servers
		{"NetworkConnectionActive",							NetworkConnectionActive},						// returns true if we have a valid network connection. Lan or Wireless
		
		{"StartBandwidthEvaluation",						StartBandwidthEvaluation},						// start a bandwidth scan
		{"BandwidthEvaluationRunning",						BandwidthEvaluationRunning},					// is a bandwidth scan currently running.			

		{"StartOnlineSessionEnumerationFromInvitation",		StartOnlineSessionEnumerationFromInvitation},	// start searching for the matching session that related to a game invite we have received
		{"ContinueOnlineSessionEnumerationFromInvitation",	ContinueOnlineSessionEnumerationFromInvitation},// continue searching for the game invite matching session.

		{"SetRichPresence",									SetRichPresence},								// sets the rich presence for all local players.
		{"SetRichPresenceSplitScreen",						SetRichPresenceSplitScreen},						

		{"ShowSystemCommunityUI",							ShowSystemCommunityUI},							// xbox360 has a builtin party Ui, this shows the blade.

		{"GameInviteBeingSent",								GameInviteBeingSent},							// are we in the process of sending a game invite.
		{"GameInvitePending",								GameInvitePending},								// do we currently have a game invite pending
		{"ClearGameInvite",									ClearGameInvite},								// if a game invite is stored for later processing this will clear it.
		{"IsInExternalPartySession",						IsInExternalPartySession},						// xbox360 only. is local player in a nxe party session.
		{"SendGameInviteToExternalPartySession",			SendGameInviteToExternalPartySession},			// xbxo360 only. send invite to nxe guide session. if none active friends list will be displayed.

		{"CheckEntryRequirements",							CheckEntryRequirements},						// checks the entry requirements before going into network multiplayer

		{"CanViewGamerCard",								CanViewGamerCard},								// can a specific local player view other players gamercards..
		{"ViewGamerCard",									ViewGamerCard},									// show specific gamer card
		{"ViewScoreboardGamerCard",							ViewScoreboardGamerCard},
		{"ViewSessionScoreboardGamerCard",					ViewSessionScoreboardGamerCard},
		{"ViewRaceSummaryGamerCard",						ViewRaceSummaryGamerCard},

		{"CanSubmitPlayerReview",							CanSubmitPlayerReview},							// can a specific local player submit player reviews
		{"SubmitPlayerReview",								SubmitPlayerReview},							// show trhe player review ui.

		{"QuickMuteAvailable",								QuickMuteAvailable},							// can the local player use the quick mute function.
		{"IsRemotePlayerQuickMuted",						IsRemotePlayerQuickMuted},						// is a specific remote player quick muted.
		{"ToggleRemotePlayerQuickMute",						ToggleRemotePlayerQuickMute},					// toggle the quick mute on/off for a specific remote player
		{"GetVoiceMap",										GetVoiceMap},
		{"GetPartyMap",										GetPartyMap},
		{"GetNatType",										GetNatType},
		{"GetSystemRegion",									GetSystemRegion},
		{"UserHasAgeRestriction",							UserHasAgeRestriction},

		{"CanUseChat",										CanUseChat},
		{"AllowPlayerCountUpdate",							AllowPlayerCountUpdate},
		{"StopPlayerCountUpdate",							StopPlayerCountUpdate},

		{"InitTimeOutTimer",								InitTimeOutTimer},
		{"TitleStorageStatus",								TitleStorageStatus},
		{"SetMatchingRegionInfo",							SetMatchingRegionInfo},

		{"MpBetaExpired",									MpBetaExpired},						// check sorcery to see if the online mode is currently disabled. beta only..
		{"PeerRoutingEnabled",								PeerRoutingEnabled},				// is the net router enabled for peer routing of data (to stop nat issues) ?
		{"FindGameInvitePadIndex",							FindGameInvitePadIndex},

		{"StartServerTimeStampTask",						StartServerTimeStampTask},
		{"ShowTotalPlayerCount",							ShowTotalPlayerCount},
		{"ShowPlaylistPlayerCount",							ShowPlaylistPlayerCount},

		{ "StartProfileUpload",								StartProfileUpload },
		{ "ContinueProfileUpload",							ContinueProfileUpload },
		{ "StartProfileDowload",							StartProfileDowload },
		{ "ContinueProfileDownload",						ContinueProfileDownload },
		{ "UserProfileDownloaded",							UserProfileDownloaded },
		{ "ApplyDownloadedProfile",							ApplyDownloadedProfile },
		{ "ResetMultiplayerProfile",						ResetMultiplayerProfile },

		{NULL,												NULL}											// end of list
	};  

	LUALIB_API int luaopen_net_services(lua_State *pState)
	{
		luaL_openlib(pState,"NetServices", net_services_lib, 0);
		return 1;
	}

} // end namespace Game
