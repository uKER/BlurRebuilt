#include "stdafx.h"
#include <Game/Game.h>
#include <ui/Game/UILua.h>
#include <Network/Core.h>
#include <Network/Session.h>
#include <Network/Securekey.h>
#include <Network/SessionEnumerator.h>
#include <racingphysics/physics/physicstimer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Structures/NetGameVars.h>
#include <GameLogic\Structures\LevelState.h>
#include <GameApp.h>
#include <Network/Misc/NetVersion.h>
#include <Network/Misc/NetDefines.h>
#include <Network/Misc/NetJoinRaceFromPartyData.h>
#include <Network/Core/NetLogger.h>
#include <Network/Core/NetServices.h>
#include <Network/Core/NetRace.h>
#include <Network/Core/NetParty.h>
#include <Network/Misc/NetQoS.h>
#include <Network/Core/NetMatchingParameters.h>
#include <Network/Router/NetRouterBase.h>
#include <Network/Message/NetMessage.h>
#include <Network/Misc/NetDatagramPacketTypes.h>
#include <Network/Core/NetCreationHelper.h>
#include <Network/Lan/NetLanDiscoveryDefines.h>
#include <Network/Lan/NetLanDiscovery.h>
#include <Network/Player/NetPlayer.h>
#include <Network/Player/NetPlayerManager.h>
#include <Network/Player/NetPlayerConnection.h>
#include <Network/Player/NetPlayerStates.h>
#include <GameLogic/GameData/GameData.h>
#include <GameLogic/GameData/MultiplayerData.h>

#include <UI/Game/Profiles/ProfileInterface.h>
#include <UI/UIManager.h>
#include <Ui\game\Multiplayer\MultiplayerHelpers.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>
#include <GameLogic/Profile/MultiplayerProfile.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic\RaceRules\Multiplayer\MpRaceAction_GiftPerk.h>
#include <GameLogic/RaceModes/RaceModeMultiplayer.h>

#include <AI/AI_AmaxXt.h>
#include <utilityai/utilityai.h>
#include <RaceBook/Data/RaceBookData.h>
#include <system/debug/BuildStamp.h>
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>

#include <GameLogic/GameData/GameData.h>
#include <GameLogic/Structures/MpResults.h>

#include <UI/UiHelpers.h>
#include "UI/UIMessages.h"

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/GameComponents.h>
#include <Game/GameActors/Components/VehicleLogic/HandOfGod/VLC_HandOfGod.h>
#include <Game/messaging/GameflowMessages.h>

#include <UI/Game/Multiplayer/MultiplayerCache.h>
#include <GameLogic/GameData/MultiplayerData.h>
#include <GameLogic\Profile\GameProfileManager.h>

#include "Automation\GameDataLogging.h"

extern Debug::Logger &gLUAMessageLogger;

#if !defined ( CFG_GOLD )
Debug::Bool g_ForceRaceSessionError("Network.Matching.ForceRaceSessionError", false);
Debug::Bool g_ForceRaceSessionCreationError("Network.Matching.ForceRaceSessionCreationError", false);
Debug::Bool g_ForceIgnoreExistingGames("Network.Matching.IgnoreExistingGames", false);
#endif

extern Debug::Int g_debugShieldsEarnedRepairMultiPlayer;
extern Debug::Int g_debugShieldsEarnedHitByPerkMultiPlayer;
extern Debug::Int g_debugShieldsEarnedFansMultiPlayer;
extern Debug::Int g_debugShieldsEarnedFansRequiredMultiPlayer;
extern Debug::Int g_debugShieldsInitialMultiPlayer;

#ifndef CFG_GOLD
namespace RacerNet
{
	extern Debug::Enum<eNetBuildVersion> NET_PACKET_VERSION;
}
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace Game 
{ 
	// gets setup before we enumerate online sessions.
	static RacerNet::NetMatchingSearchData sMatchingParameters;

	static S32 Delete(lua_State* pState)
	{
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			ns.EnterLock();
			ns.Race().DisconnectAndClear();
			ns.Exitlock();
		}

		// flush game logic data since we are closing the session.
		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().Clear();

		return 0;
	}

	static S32 UnrecoverableError(lua_State* pState)
	{
		bool error = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
			error = ns.Race().BigError();

#if !defined ( CFG_GOLD )
		if (g_ForceRaceSessionError)
		{
			error = true;
			g_ForceRaceSessionError = false;
		}
#endif

		lua_pushboolean(pState, error);
		return 1;
	}

	static S32 IsPlayerMigrating(lua_State* pState)
	{
		bool migrating = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			if (ns.Race().IsRunning())
				migrating = ns.Race().IsMigrating();
		}
		lua_pushboolean(pState, migrating);
		return 1;
	}

	static S32 IsSessionMigrating(lua_State* pState)
	{
		bool migrating = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			if (ns.Race().IsRunning())
				migrating = ns.Race().IsSessionMigrating();
		}
		lua_pushboolean(pState, migrating);
		return 1;
	}

	static S32 CanSendGameInvite(lua_State* pState)
	{
		bool ok = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			NetPlay::SessionSecurity sec;
			if (ns.Race().IsRunning())
				ok = ns.Race().CanInvite(sec);
		}

		lua_pushboolean(pState, ok);
		return 1;
	}

	static S32 IsRunning(lua_State* pState)
	{
		bool running = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
			running = ns.Race().IsRunning();
		lua_pushboolean(pState, running);
		return 1;
	}

	static S32 IsOnlineMode(lua_State* pState)
	{
		bool online = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised() && ns.Race().IsRunning())
			online = ns.Race().IsOnline();
		lua_pushboolean(pState, online);
		return 1;
	}

	static S32 EnumerateLanGames(lua_State* pState)
	{
		if (lua_isboolean(pState, 1))
		{
			const bool start = lua_toboolean(pState, 1) ? true : false;
				
			RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
			if (ns.IsInitialised() && ns.Race().IsInitialised())
			{ 
				RacerNet::C_LanDiscoveryManager& mgr = ns.Race().LanDiscoveryManager();
					
				if (start)
				{
					RacerNet::C_NetCreationHelper& hlp = ns.SessionCreationHelper();

					hlp.BuildLocalPlayersForLan(0, 0, 0); // settings are ignored when enumerating lan games.
					if (hlp.NumPlayers() > 0)
						mgr.Enumerate(hlp.NumPlayers());// TODO_MP_SYSLINK
				}
				else
					mgr.DestroyEnumerated();
			}
		}
		return 0;
	}

	static S32 IsEnumeratingLanGames(lua_State* pState)
	{
		bool enumerating = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised() && ns.Race().IsInitialised())
		{
			RacerNet::C_LanDiscoveryManager& mgr = ns.Race().LanDiscoveryManager();
			enumerating = mgr.IsEnumerating();
		}
		lua_pushboolean(pState, enumerating);
		return 1;
	}

	static S32 NumLanServersEnumerated(lua_State* pState)
	{
		U32 found = 0;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised() && ns.Race().IsInitialised())
		{
			RacerNet::C_LanDiscoveryManager& mgr = ns.Race().LanDiscoveryManager();
			found = mgr.NumEnumerated();
		}
		lua_pushnumber(pState, found);
		return 1;
	}

	static S32 CheckLanServerNetworkVersion(lua_State* pState)
	{
		U32 code = RacerNet::eNetVersionUnknown;
		U8 pack_version = RacerNet::eNetVersionUnknown;

		if (lua_isnumber(pState, 1))
		{
			const U32 serverIndex  = (U32)lua_tointeger(pState, 1);
			RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
			if (ns.IsInitialised() && ns.Race().IsInitialised())
			{
				RacerNet::C_LanDiscoveryManager& mgr = ns.Race().LanDiscoveryManager();
				if (serverIndex < mgr.NumEnumerated())
				{
					RacerNet::LanDiscoveryReply data;
					if (mgr.GetEnumeratedSesionData(serverIndex, data))
					{
						if (RacerNet::NET_PACKET_VERSION == (S32)data.m_PacketVersion) //Fixed unsigned/signed comparison warning
							code = RacerNet::eNetVersionSame;
						else if (RacerNet::NET_PACKET_VERSION > (S32)data.m_PacketVersion)
							code = RacerNet::eNetVersionNewer;
						else if (RacerNet::NET_PACKET_VERSION < (S32)data.m_PacketVersion)
							code = RacerNet::eNetVersionOlder; 

						// TODO_MP_SYSLINK - use a system link pack version
						if ( C_MultiplayerData::Instance().m_pConfig->m_PackVersion == data.m_RaceQos.m_PackVersion )
							pack_version = RacerNet::eNetVersionSame;
						else if ( C_MultiplayerData::Instance().m_pConfig->m_PackVersion > data.m_RaceQos.m_PackVersion )
							pack_version = RacerNet::eNetVersionNewer;
						else if ( C_MultiplayerData::Instance().m_pConfig->m_PackVersion < data.m_RaceQos.m_PackVersion )
							pack_version = RacerNet::eNetVersionOlder;
					}
				}
			}
		}
		lua_pushnumber(pState, code);
		lua_pushnumber(pState, pack_version);
		return 2;
	}

	static S32 SetRaceParameters(lua_State* pState)
	{
		if (lua_istable(pState, 1))
		{
			Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
			BIZ_ASSERT(pLayer->GetGameModePtr()->IsNetwork());
			pLayer->SetupRace(pState);
		}
		else
		{
			BIZ_ASSERT_MSG(false, "NetRace.SetRaceParameters() - invalid data passed from lua.");
		}
		return 0;
	}

	static S32 CreateLanServer(lua_State* pState)
	{
		bool created = false;

		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().Clear();

		if (lua_istable(pState, 1))
		{
			Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();

			// parse lua input table to get initial race settings.
			pLayer->SetupRace(pState);

			RacerNet::NetRaceSettings settings;
			if (pLayer->GetGameModePtr()->GetNetRaceSettings(settings))
			{
				// check settings for validity
				if (0 == settings.m_MaxDrivers || settings.m_MaxDrivers > RacerNet::NET_MAX_VEHICLES)
				{
					LOGERROR(RacerNet::g_Log, "CreateLanServer() - supplied player slot data is invalid for a lan game.");
				}
				else
				{
					RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
					if (services.IsInitialised())
					{
						// copy active players into the helper object so net session knows what local players are playing.
						U64 consoleId = 0;
						if (NetPlay::C_Core::Instance().GenerateMachineIdentifier(consoleId))
						{
							RacerNet::C_NetCreationHelper& helper = services.SessionCreationHelper();
							if (helper.BuildLocalPlayersForLan(consoleId, RacerNet::NET_LAN_BANDWIDTH_KBPS, RacerNet::NET_LAN_BANDWIDTH_KBPS))
							{
								RacerNet::C_NetRace& race = services.Race();
								created = race.CreateLanSession(helper);
							}
						}
					}
				}
			}
		}
		else
		{
			BIZ_ASSERT_MSG(false, "NetRace.CreateLanServer() - invalid data passed from lua.");
		}

		lua_pushboolean(pState, created);
		return 1;
	}

	static S32 JoinLanServer(lua_State* pState)
	{
		bool started = false;
		bool customRace = false;
		U32 eventID = 0;

		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().Clear();
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();

		Game::C_GameModeNetwork* netGameMode = (Game::C_GameModeNetwork*)(pLayer->GetGameModePtr());
		if(netGameMode)
			netGameMode->ResetGameValues();

		if (lua_isnumber(pState, 1))
		{
			const U32 serverIndex = (U32)lua_tointeger(pState, 1);

			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			if (services.IsInitialised())
			{
				RacerNet::C_NetRace& race = services.Race();
				RacerNet::C_LanDiscoveryManager& mgr = race.LanDiscoveryManager(); 

				if (serverIndex >= 0 && serverIndex < mgr.NumEnumerated())
				{
					U64 consoleId = 0;
					if (NetPlay::C_Core::Instance().GenerateMachineIdentifier(consoleId))
					{
						RacerNet::C_NetCreationHelper& helper  = services.SessionCreationHelper();

						if (helper.BuildLocalPlayersForLan(consoleId, RacerNet::NET_LAN_BANDWIDTH_KBPS, RacerNet::NET_LAN_BANDWIDTH_KBPS))
						{
							RacerNet::LanDiscoveryReply serverData;
							if (mgr.GetEnumeratedSesionData(serverIndex, serverData))
							{
								if (RacerNet::NET_PACKET_VERSION == serverData.m_PacketVersion)
								{
									started = race.JoinLanSession(helper, serverData.m_SessionSecurity, serverData.m_HostCommonAddress, serverData.m_ConsoleId);
									
									customRace = serverData.m_RaceQos.m_CustomRace;
									eventID = serverData.m_RaceQos.m_EventID;
								}
							}
						}
					}
				}
			}
		}

		lua_pushboolean(pState, started);
		lua_pushboolean(pState, customRace);
		lua_pushnumber(pState, eventID);
		return 3;
	}

	static S32 MonitorJoinLanServer(lua_State* pState)
	{
		bool complete = true;
		RacerNet::eNetJoinSessionErrors code = RacerNet::eJoinErrorInvalid;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetJoinHelper& join = services.Race().JoinHelper();
			if (join.IsJoining())
				complete = false;
			else
			{
				switch (join.JoinGetState())
				{
				case RacerNet::eJoinStateComplete:
					code = RacerNet::eJoinErrorNone; // join successfull.
					break;

				case RacerNet::eJoinStateFailed:
					code = join.JoinGetLastError(); // failed to join so get reason why.
					break;
				}
			}
		}

		lua_pushboolean(pState, complete);
		lua_pushnumber(pState, code);
		return 2;
	}

	static S32 CreateOnlineServer(lua_State* pState)
	{
		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().Clear();

		bool started = false;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();

			RacerNet::C_NetRace& race = services.Race();
			RacerNet::C_NetParty& party = services.Party();
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

			race.DisconnectAndClear();

			RacerNet::NetRaceSettings settings;
			if (pLayer->GetGameModePtr()->GetNetRaceSettings(settings))
			{
				if (0 == settings.m_MaxDrivers || settings.m_MaxDrivers > RacerNet::NET_MAX_VEHICLES)
				{
					LOGERROR(RacerNet::g_Log, "CreateOnlineServer() - supplied player slot data is invalid.");
				}
				else
				{
					if (services.Router().BandwidthEvaluationInProgress())
					{
						LOGWARN(RacerNet::g_Log, "NetRace.CreateOnlineServer() - trying to create online session before bandwidth evaluation has finished.");
					}

					if (!party.IsRunning())
					{
						// copy active players into the helper object so net session knows what local players are playing.
						U64 consoleId = 0;
						if (NetPlay::C_Core::Instance().GenerateMachineIdentifier(consoleId))
						{
							RacerNet::C_NetCreationHelper& helper = services.SessionCreationHelper();
							RacerNet::C_NetRouterBase& router = services.Router();
							if (helper.BuildLocalPlayersForOnline(router.GetNatType(), consoleId, router.EvaluatedUpstreamBandwidthKbps(), router.EvaluatedDownstreamBandwidthKbps()))
							{
								// what game settings do we need to pass to the matching server.
								RacerNet::NetMatchingServerData matchingServerCreationData;
								matchingServerCreationData.m_MatchingMode		= RacerNet::eNetMatchingModeNormal;
								matchingServerCreationData.m_PackVersion		= S32(		((C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion << 16) & 0xffff0000) 
																						|	((C_MultiplayerData::Instance().m_pConfig->m_PackVersion << 0) & 0x0000ffff));
								matchingServerCreationData.m_RankRestriction	= helper.PrimaryOnlinePlayersRank();
								matchingServerCreationData.m_MatchingRegion		= router.GetMatchingRegion();
								matchingServerCreationData.m_EventId			= settings.m_EventId;
								matchingServerCreationData.m_CustomRace			= settings.m_CustomRace;
								matchingServerCreationData.m_OpenSlotCount		= settings.m_MaxDrivers - helper.NumPlayers();
								matchingServerCreationData.m_Searchable			= settings.m_PublicGame;
								matchingServerCreationData.m_InRace				= 0;
								matchingServerCreationData.m_Nat				= router.GetNatType();

								U32 serverFlags = NetPlay::SESSION_CREATE_ONLINE_MULTIPLAYER_STANDARD | NetPlay::SESSION_CREATE_HOST;

								// build platform specific matching server parameters list.
								RacerNet::NetMatchingParameterList matchingServerParamList;
								RacerNet::BuildMatchingParameters(matchingServerCreationData, matchingServerParamList, true, false);
								if (matchingServerParamList.Size() > 0)
								{
									NetPlay::C_Session& matchSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);

									NetPlay::C_Session::SlotConfig matchSlotConfig;
									matchSlotConfig.m_MaxPublic		= (settings.m_PublicGame != 0) ? settings.m_MaxDrivers : 0;
									matchSlotConfig.m_PublicInUse	= (settings.m_PublicGame != 0) ? helper.NumPlayers() : 0;
									matchSlotConfig.m_MaxPrivate	= (settings.m_PublicGame == 0) ? settings.m_MaxDrivers : 0;
									matchSlotConfig.m_PrivateInUse	= (settings.m_PublicGame == 0) ? helper.NumPlayers() : 0;
									
									started = matchSession.Create(helper.SessionManagerIndex(), serverFlags, &matchingServerParamList[0], matchingServerParamList.Size(), matchSlotConfig);
								}
							}
						}
					}
					else
					{
						if (mgr.Size(true) <= settings.m_MaxDrivers)
						{
							RacerNet::C_NetCreationHelper& helper = services.SessionCreationHelper();
							RacerNet::C_NetRouterBase& router = services.Router();
							if (helper.BuildLocalPlayersForOnlineFromParty(router.GetNatType(), party, mgr))
							{
								// what game settings do we need to pass to the matching server.
								RacerNet::NetMatchingServerData matchingServerCreationData;
								matchingServerCreationData.m_MatchingMode		= RacerNet::eNetMatchingModeNormal;
								matchingServerCreationData.m_PackVersion		= S32(		((C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion << 16) & 0xffff0000) 
																						|	((C_MultiplayerData::Instance().m_pConfig->m_PackVersion << 0) & 0x0000ffff));
								matchingServerCreationData.m_RankRestriction	= helper.PrimaryOnlinePlayersRank();
								matchingServerCreationData.m_MatchingRegion		= router.GetMatchingRegion();
								matchingServerCreationData.m_EventId			= settings.m_EventId;
								matchingServerCreationData.m_CustomRace			= settings.m_CustomRace;
								matchingServerCreationData.m_OpenSlotCount		= settings.m_MaxDrivers - helper.NumPlayers();
								matchingServerCreationData.m_Searchable			= settings.m_PublicGame;
								matchingServerCreationData.m_InRace				= 0;
								matchingServerCreationData.m_Nat				= (S8) services.Router().PeerRoutingEnabled() ? router.GetNatType() : services.PlayerManager().GetMostRestrictiveNatType(false);
								// [OLI] if peer relay is on, only the host nat type is useful. If peer relay off, find the worse case.

								RacerNet::NetMatchingParameterList matchingServerParamList;
								RacerNet::BuildMatchingParameters(matchingServerCreationData, matchingServerParamList, true, false);

								if (matchingServerParamList.Size() > 0)
								{
									NetPlay::C_Session& matchSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);

									NetPlay::C_Session::SlotConfig matchSlotConfig;
									matchSlotConfig.m_MaxPublic		= (settings.m_PublicGame != 0) ? settings.m_MaxDrivers : 0;
									matchSlotConfig.m_PublicInUse	= (settings.m_PublicGame != 0) ? helper.NumPlayers() : 0;
									matchSlotConfig.m_MaxPrivate	= (settings.m_PublicGame == 0) ? settings.m_MaxDrivers : 0;
									matchSlotConfig.m_PrivateInUse	= (settings.m_PublicGame == 0) ? helper.NumPlayers() : 0;

									U32 serverFlags = NetPlay::SESSION_CREATE_ONLINE_MULTIPLAYER_STANDARD | NetPlay::SESSION_CREATE_HOST;

									started = matchSession.Create(helper.SessionManagerIndex(), serverFlags, &matchingServerParamList[0], matchingServerParamList.Size(), matchSlotConfig);
								}
							}
						}
						else
						{
							LOGWARN(RacerNet::g_Log, "NetRace.CreateOnlineServer() - trying to create online session with less slots then party size.");
						}
					}
				}
				//Data gathering time
				if(started)
				{
					pLayer->GetDataGathering().StoreLobbyChoice(true); //True = created lobby
				}
			}
		}

#if !defined( CFG_GOLD )
			if (g_ForceRaceSessionCreationError)
			{
				started = false;
				g_ForceRaceSessionCreationError = false;
			}
#endif

		lua_pushboolean(pState, started);
		return 1; 
	}

	static S32 ContinueCreateOnlineServer(lua_State* pState)
	{
		bool finished = true;
		bool success = false;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			NetPlay::C_Session& matchSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);;
			switch (matchSession.SessionState())
			{
			case NetPlay::C_Session::eStateCreating:
				finished = false;
				break;

			case NetPlay::C_Session::eStateError:
			case NetPlay::C_Session::eStateNotActive:
				services.Race().DisconnectAndClear(); // this will internally delete the matching session
				break;

			case NetPlay::C_Session::eStateRunning:
				// online session is running so try and create net game.
				success = services.Race().CreateOnlineSession(services.SessionCreationHelper());
				break;
			}
		}

		lua_pushboolean(pState, finished);
		lua_pushboolean(pState, success);
		return 2;
	}


	static S32 ChangeLocalPlayerVehicle( lua_State* pState )
	{
		if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
		{
			UI::g_pUILua->StackDump( pState, "ChangePlayerVehicle( num, num ) - incorrect arguments\n" );
			return 0;
		}

		Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

		const U32 padIndex	= (U32)lua_tointeger( pState, 1 );
		const U32 vehicleID	= (U32)lua_tointeger( pState, 2 );

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
			if (race.IsRunning() && race.ValidateVehicle(vehicleID))
			{
				AmaxVehicle* p = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)vehicleID];
				if (p)
				{
					RacerNet::NetPlayerVehicle vehicleData;
					vehicleData.m_AutoGears				= true;
					vehicleData.m_xtVehicleId			= vehicleID;
					vehicleData.m_CharacterId			= static_cast<C_GameModeNetwork*>(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr())->GetRandomCharacterID();
					vehicleData.m_xtColourId			= pGameProfile ? pGameProfile->ResprayGet( p->ID, true ) : 0;
					vehicleData.m_VehicleContentPack	= p->contentPack;

					// find local player
					bool  done = false;
					for (U8 i=0; i<mgr.LocalSize() && !done; i++)
					{
						RacerNet::C_NetPlayer* p = mgr.Index(i);
						if (p->LocalPlayer() && p->m_RaceConnection.IsActive() && padIndex == p->m_ControllerPort)
						{
							done = true;

							//	If we're playing online, we use our Multiplayer profile to fill out our info
							if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsOnline())
							{
								//	Now vehicle has been selected, we need to find out how many pickup slots etc the player has (since vehicles can give bonuses)
								Game::C_MultiplayerProfile* pMultiProf = pGameProfile->GetMultiplayerProfile();
								if(pMultiProf)
								{
									RacerNet::NetRaceSettings settings;
									C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( settings );
									pMultiProf->BuildNetworkVehicleUpgradesData(vehicleData, (UI::E_CarUpgrades)settings.m_Race.m_Upgrades);
								}
							}
							else
							{
								//	We must be in system link, so we just some default values - fair that way.
								vehicleData.m_PickupSlots = 3;
								vehicleData.m_ShieldSlots = 3;
							}
									
							// overwrite players stored vehicle with newly selected one.
							p->m_RaceData.m_VehicleData = vehicleData;

							// actual send is done from within GameModeNetwork.cpp
							for (U8 z=0; z<mgr.TotalSize(); z++)
							{
								RacerNet::C_NetPlayer* pl = mgr.Index(z);
								if (pl->LocalPlayer() || !pl->PrimaryPlayer())
									continue;

								// remote primary player.. 
								if (pl->m_RaceConnection.IsActive())
								{
									// clear vehicle received bit so data is resent on next update.
									pl->m_RaceData.m_BitArray.ClearBit(RacerNet::NetPlayerRaceGameData::eBitArrayData_HasMyLatestVehicleData);
								}
							}
						}
					}
				}
			}
		}

		return 0;
	}

	static S32 ChangeLocalPlayerState(lua_State* pState)
	{
		if (lua_type(pState, 1) == LUA_TNUMBER && lua_type(pState, 2) == LUA_TNUMBER)
		{
			const U32 padIndex	= (U32)lua_tointeger( pState, 1 );
			const U32 s = (U32)lua_tointeger( pState, 2 ); 

			if (s < RacerNet::eNetPlayerStateMax)
			{
				RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
				if (services.IsInitialised())
				{
					RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
					RacerNet::C_NetRace& race = services.Race();
					if (race.IsInitialised() && race.IsRunning())
					{
						bool  changed = false;
						RacerNet::eNetPlayerState state = static_cast<RacerNet::eNetPlayerState> (s);

						bool done = false;
						for (U8 i=0; i<mgr.LocalSize() && !done; i++)
						{
							RacerNet::C_NetPlayer* p = mgr.Index(i);
							BIZ_ASSERT(p->LocalPlayer());
							BIZ_ASSERT(p->m_RaceConnection.IsActive());
							if (p->LocalPlayer() && p->m_RaceConnection.IsActive() && padIndex == p->m_ControllerPort)
							{
								done = true;
								if (p->m_RaceConnection.m_PlayerState != state)
								{
									p->m_RaceConnection.m_PlayerState = state;
									changed = true;
								}
							}
						}

						if (changed)
							race.SendLocalPlayerStatesMessage();
					}
				}
			}
			else
			{
				BIZ_ASSERT_FAIL("NetRace.ChangeLocalPlayerState() - called with invalid state value.");
			}
		}
		else
		{
			BIZ_WARNING_FAIL("NetRace.ChangeLocalPlayerState() - called without arguments from lua.");
		}
		return 0;
	}


	static S32 ChangeLocalPlayerLoadout( lua_State* pState )
	{
		if ( lua_type( pState, 1 ) != LUA_TNUMBER )
		{
			UI::g_pUILua->StackDump( pState, "ChangeLocalPlayerLoadout( num ) - incorrect arguments\n" );
			return 0;
		}

		const U32 padIndex	= (U32)lua_tointeger( pState, 1 );

		Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetProfile( padIndex );
		Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();
		BIZ_ASSERT_MSG( pProfile, "No multiplayer profile" );
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		bool isRanked = !pLayer || !pLayer->GetGameModePtr() || pLayer->GetGameModePtr()->IsRanked();

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
			if ( !race.IsRunning() )
				return 0;

			// find local player
			bool  done = false;
			for (U8 i=0; i<mgr.LocalSize() && !done; i++)
			{
				RacerNet::C_NetPlayer* p = mgr.Index(i);
				if (p->LocalPlayer() && p->m_RaceConnection.IsActive() && padIndex == p->m_ControllerPort)
				{
					done = true;

					// overwrite players loadout with newly selected one.
					p->m_RaceData.m_LoadOut.Flush();
					pProfile->GetLoadouts(isRanked).BuildNetworkLoadOutData(p->m_RaceData.m_LoadOut, isRanked);

					// actual send is done from within GameModeNetwork.cpp
					for (U8 z=0; z<mgr.TotalSize(); z++)
					{
						RacerNet::C_NetPlayer* pl = mgr.Index(z);
						if (pl->LocalPlayer() || !pl->PrimaryPlayer())
							continue;

						// remote primary player.. 
						if (pl->m_RaceConnection.IsActive())
						{
							// clear vehicle received bit so data is resent on next update.
							pl->m_RaceData.m_BitArray.ClearBit(RacerNet::NetPlayerRaceGameData::eBitArrayData_HasMyLatestVehicleData);
						}
					}
				}
			}
		}
		return 0;
	}


	static S32 SetOnlineSessionEnumerationParameters(lua_State* pState)
	{
		// default the filter parameters (if not changed this will act like a search with no filter, all available sessions will be returned)
		RacerNet::NetMatchingSearchData blank;
		sMatchingParameters = blank;

		if (lua_istable(pState, 1))
		{
			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			if (services.IsInitialised())
			{	
				RacerNet::C_NetRouterBase& router = services.Router();

				// defauult to ,local players nat type.
				sMatchingParameters.m_Nat			= (S8)router.GetNatType();

				// sorcery pack data version.
				sMatchingParameters.m_PackVersion		= S32(		((C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion << 16) & 0xffff0000) 
																|	((C_MultiplayerData::Instance().m_pConfig->m_PackVersion << 0) & 0x0000ffff));

				// quick searches have some matching parameters ignored by default.
				if (Lua::DataMemberType(pState, "quick_search") == LUA_TBOOLEAN)
				{
					// shouldnt be attempting quick search while in a party.
					BIZ_ASSERT(services.Party().IsRunning() == false);

					RacerNet::C_NetCreationHelper& helper = services.SessionCreationHelper();
					helper.BuildLocalPlayersForOnline(router.GetNatType(), 0, 0, 0); 

					sMatchingParameters.m_MatchingMode		= RacerNet::eNetMatchingModeNormal;	
					sMatchingParameters.m_RankRestriction	= -1;
					sMatchingParameters.m_EventId			= -1;
					sMatchingParameters.m_CustomRace		= -1;
					sMatchingParameters.m_NumToJoin			= (S8)helper.NumPlayers();
					sMatchingParameters.m_LobbyOnly			= !(services.GetJoinSessionsInRace());

#if defined ( LSP_GROUPS_IS_CUSTOM_CONTENT )
					if (!helper.CanUseUserCreatedContent())
						sMatchingParameters.m_UserCreatedGroup = 1; // New quick race functionality only finds user content, not ranked / world tour
#endif
				}
				else
				{
					sMatchingParameters.m_MatchingMode		= RacerNet::eNetMatchingModeNormal;
					sMatchingParameters.m_EventId			= UI::C_MultiplayerCache::Instance().GetCurrentEventID();
					sMatchingParameters.m_CustomRace		= false;
					sMatchingParameters.m_LobbyOnly			= !(services.GetJoinSessionsInRace());

					// how many players do we wish to join ? this paramerter should always be present in the serch query..
					if (services.Party().IsRunning())
					{
						RacerNet::C_NetCreationHelper& helper	= services.SessionCreationHelper(); // will already be configured if party is active.
						sMatchingParameters.m_NumToJoin			= (S8)services.PlayerManager().Size(true);
						sMatchingParameters.m_Nat				= (S8)services.PlayerManager().GetMostRestrictiveNatType(true); // override with most restrictive players nat from the party.

						//sMatchingParameters.m_RankRestriction	= helper.PrimaryOnlinePlayersRank();
						sMatchingParameters.m_RankRestriction	= -1;	// disable matching based on players rank
					}
					else
					{
						// privileges should have alread been checked before we get here so we should have at least 1 valid online player
						RacerNet::C_NetCreationHelper& helper = services.SessionCreationHelper();
						RacerNet::C_NetRouterBase& router = services.Router();
						helper.BuildLocalPlayersForOnline(router.GetNatType(), 0, 0, 0); 
						sMatchingParameters.m_NumToJoin			= (S8)helper.NumPlayers();

						//sMatchingParameters.m_RankRestriction	= helper.PrimaryOnlinePlayersRank();	
						sMatchingParameters.m_RankRestriction	= -1;	// disable matching based on players rank
					}
				}

				BIZ_ASSERT(sMatchingParameters.m_NumToJoin > 0);
			}
		}

		return 0;
	}

	static S32 StartOnlineSessionEnumeration(lua_State* pState)
	{
		bool started = false;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			bool filterOnRequiredMatchingRegion = false;
			RacerNet::eMatchingRegion requiredMatchingRegion = RacerNet::eMatchingRegionUnknown;
			services.GetMatchingRegionInfo(filterOnRequiredMatchingRegion, requiredMatchingRegion);

			// do we need to override the matching region ?
			if (filterOnRequiredMatchingRegion && requiredMatchingRegion != RacerNet::eMatchingRegionUnknown)
				sMatchingParameters.m_MatchingRegion = requiredMatchingRegion;
			else
				sMatchingParameters.m_MatchingRegion = -1;

			NetPlay::C_SessionEnumerator& enumerator = services.MatchmakingSessionEnumerator();
			RacerNet::C_NetCreationHelper& helper = services.SessionCreationHelper(); // should be valid from either creating a party or calling SetOnlineSessionEnumerationParameters()

			enumerator.KillSearch();
			services.MatchingRemapList().Clear();

			BIZ_ASSERT(sMatchingParameters.m_NumToJoin > 0);
			if (sMatchingParameters.m_NumToJoin > 0 && helper.SessionManagerIndex() >= 0 && (U32)helper.SessionManagerIndex() < UI::C_ProfileInterface::Instance().Max())
			{
				RacerNet::NetMatchingParameterList platformSearchParams;
				NetPlay::SesionQueryReturnTypes platformSearchReturnTypes;

				RacerNet::BuildMatchingParameters(sMatchingParameters, platformSearchParams);
				RacerNet::BuildMatchingReturnTypes(platformSearchReturnTypes);

				const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(helper.SessionManagerIndex());
				
				started = enumerator.StartSearch(	fp, 
													RacerNet::MatchingQueryId(), 
													NET_MAX_MATCHING_SESSIONS_TO_ENUMERATE, 
													&platformSearchParams[0], 
													platformSearchParams.Size(), 
													platformSearchReturnTypes, 
													true);
			}
		}

		lua_pushboolean(pState, started);
		return 1;
	}

	static S32 ContinueOnlineSessionEnumeration(lua_State* pState)
	{
		bool finished = true;
		S32 count = 0;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			NetPlay::C_SessionEnumerator& enumerator = services.MatchmakingSessionEnumerator();
			enumerator.Poll();
			if (enumerator.IsSearching())
				finished = false;
			else
			{
				U32 availableDlc = C_GameData::Instance().GenerateAvailableDlcBits();
				if (services.Party().IsRunning())
					availableDlc = services.Party().GenerateAvailableDlc();

				RacerNet::C_NetServices::NetMatchingRemapList inLobby;
				RacerNet::C_NetServices::NetMatchingRemapList inRace;

				bool filterOnRequiredMatchingRegion = false;
				RacerNet::eMatchingRegion requiredMatchingRegion = RacerNet::eMatchingRegionUnknown;
				services.GetMatchingRegionInfo(filterOnRequiredMatchingRegion, requiredMatchingRegion);
				bool sortOnPreferredMatchingRegion = !filterOnRequiredMatchingRegion;
				
				// only sessions we can actually join get pushed into our remap list.
				services.MatchingRemapList().Clear();
				for (U32 i=0; i<enumerator.NumSessionsEnumerated(); i++)
				{
					NetPlay::C_SessionEnumerator::SessionQoSData qos;
					if (enumerator.GetQoSDataFromEnumerated(i, qos))
					{
						if (NULL != qos.m_pQoSData && qos.m_QoSDataSize >= sizeof(RacerNet::NetQoSBase))
						{
							const RacerNet::NetQoSBase* pBaseQos = reinterpret_cast<RacerNet::NetQoSBase*> (qos.m_pQoSData);
							if (RacerNet::NET_PACKET_VERSION == pBaseQos->m_Version && RacerNet::NetQoSBase::QoSTypeRace == pBaseQos->m_Type)
							{
								const RacerNet::NetQoSRace* pRaceQos = reinterpret_cast<RacerNet::NetQoSRace*> (qos.m_pQoSData);

								if (sizeof(RacerNet::NetQoSRace) == qos.m_QoSDataSize)
								{
									bool add = true;

									// is their any content in use by this sesison that we dont have access to ?
									for (U32 z=0; z<32 && add; z++)
									{
										if (pRaceQos->m_DlcSettings.m_AdditionalContentPacksInUse&(1<<z))
										{
											if (!(availableDlc&(1<<z)))
												add = false;
									 	}
									}

									S32 matchingVersion = pRaceQos->m_PackVersion >> 16;
									S32 packVersion = pRaceQos->m_PackVersion & 0x0000ffff;
									if (C_MultiplayerData::Instance().m_pConfig->m_PackVersion != packVersion ||
										C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion != matchingVersion)
										add = false;

									// do we wish to filter the returned sessions for specific regions ?
									// in this case returned sessions that are not from the user chosen region are not shown to the user.
									if (filterOnRequiredMatchingRegion && RacerNet::eMatchingRegionUnknown != requiredMatchingRegion)
									{
										if (requiredMatchingRegion != pRaceQos->m_MatchingRegion)
											add = false;
									}

									if (add)
									{ 
										if (RacerNet::NetQoSBase::QoSStateInRace == pRaceQos->m_SessionState)
											inRace.PushBack(i);									
										else if (RacerNet::NetQoSBase::QoSStateInLobby == pRaceQos->m_SessionState)
											inLobby.PushBack(i);
									}
								}
							}
						}
					}
				}

				// does the player wish to sort the returned sessions to prefer those from a specific matching region.
				if (sortOnPreferredMatchingRegion && RacerNet::eMatchingRegionUnknown != requiredMatchingRegion)
				{						
					if (inLobby.Size() > 1)
					{
						for (S32 a=0; a<inLobby.Size()-1; a++)
						{
							NetPlay::C_SessionEnumerator::SessionQoSData qosA;
						
							enumerator.GetQoSDataFromEnumerated(inLobby[a], qosA);
							BIZ_ASSERT(qosA.m_pQoSData != NULL);
							const RacerNet::NetQoSRace* pRaceQosA = reinterpret_cast<RacerNet::NetQoSRace*> (qosA.m_pQoSData);

							// if this session is already the preferred matching region then don't move it.
							if (requiredMatchingRegion == pRaceQosA->m_MatchingRegion)
								continue;

							bool swapped = false;
							for (S32 b=a+1; b<inLobby.Size() && !swapped; b++)
							{
								NetPlay::C_SessionEnumerator::SessionQoSData qosB;
								enumerator.GetQoSDataFromEnumerated(inLobby[b], qosB);
								BIZ_ASSERT(qosB.m_pQoSData != NULL);
								const RacerNet::NetQoSRace* pRaceQosB = reinterpret_cast<RacerNet::NetQoSRace*> (qosB.m_pQoSData);

								if (requiredMatchingRegion == pRaceQosB->m_MatchingRegion)
								{
									swapped = true;
									U8 temp = inLobby[a];
									inLobby[a] = inLobby[b];
									inLobby[b] = temp;
								}
							}
						}
					}

					if (inRace.Size() > 1)
					{
						for (S32 a=0; a<inRace.Size()-1; a++)
						{
							NetPlay::C_SessionEnumerator::SessionQoSData qosA;
							enumerator.GetQoSDataFromEnumerated(inRace[a], qosA);
							BIZ_ASSERT(qosA.m_pQoSData != NULL);
							const RacerNet::NetQoSRace* pRaceQosA = reinterpret_cast<RacerNet::NetQoSRace*> (qosA.m_pQoSData);

							// if this session is already the preferred matching region then don't move it.
							if (requiredMatchingRegion == pRaceQosA->m_MatchingRegion)
								continue;

							bool swapped = false;
							for (S32 b=a+1; b<inRace.Size() && !swapped; b++)
							{
								NetPlay::C_SessionEnumerator::SessionQoSData qosB;
								enumerator.GetQoSDataFromEnumerated(inRace[b], qosB);
								BIZ_ASSERT(qosB.m_pQoSData != NULL);
								const RacerNet::NetQoSRace* pRaceQosB = reinterpret_cast<RacerNet::NetQoSRace*> (qosB.m_pQoSData);

								if (requiredMatchingRegion == pRaceQosB->m_MatchingRegion)
								{
									swapped = true;
									U8 temp = inRace[a];
									inRace[a] = inRace[b];
									inRace[b] = temp;
								}
							}
						}
					}
				}

				// build final remap table so races currently in the lobby appear first.
				for (S32 i=0; i<inLobby.Size(); i++)
					services.MatchingRemapList().PushBack(inLobby[i]); 
				for (S32 i=0; i<inRace.Size(); i++)
					services.MatchingRemapList().PushBack(inRace[i]); 

				count = services.MatchingRemapList().Size();
				if (0 == count)
					enumerator.KillSearch();
			}
		}

#if !defined ( CFG_GOLD )
		if(g_ForceIgnoreExistingGames)
			count = 0;
#endif

		lua_pushboolean(pState, finished);
		lua_pushnumber(pState, count);
		return 2;
	}

	static S32 NumOnlineSessionsEnumerated(lua_State* pState)
	{
		S32 count = 0;
		if (RacerNet::C_NetServices::Instance().IsInitialised())
			count = RacerNet::C_NetServices::Instance().MatchingRemapList().Size();

		lua_pushnumber(pState, count);
		return 1;
	}

	static S32 IsEnumeratedOnlineSessionInRace(lua_State* pState)
	{
		bool inProgress = false;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			if (lua_isnumber(pState, 1))
			{
				const S32 index = (S32)lua_tonumber(pState, 1);
				if (services.MatchingRemapList().Size() > 0)
				{
					if (index >= 0 && index < services.MatchingRemapList().Size())
					{
						NetPlay::C_SessionEnumerator& enumerator = services.MatchmakingSessionEnumerator();
						NetPlay::C_SessionEnumerator::SessionQoSData qos;
						if (enumerator.GetQoSDataFromEnumerated(services.MatchingRemapList()[index], qos))
						{
							if (NULL != qos.m_pQoSData && qos.m_QoSDataSize >= sizeof(RacerNet::NetQoSBase))
							{
								const RacerNet::NetQoSBase* pBaseQos = reinterpret_cast<RacerNet::NetQoSBase*> (qos.m_pQoSData);
								if (RacerNet::NET_PACKET_VERSION == pBaseQos->m_Version && RacerNet::NetQoSBase::QoSTypeRace == pBaseQos->m_Type)
								{
									const RacerNet::NetQoSRace* pRaceQos = reinterpret_cast<RacerNet::NetQoSRace*> (qos.m_pQoSData);
									inProgress = (pRaceQos->m_SessionState == RacerNet::NetQoSBase::QoSStateInRace) ? true : false;
								}
							}
						}
					}
				}
			}
		}

		lua_pushboolean(pState, inProgress);
		return 1;
	}

	static S32 JoinOnlineServer(lua_State* pState)
	{
		bool started = false;

		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().Clear();
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();

		Game::C_GameModeNetwork* netGameMode = (Game::C_GameModeNetwork*)(pLayer->GetGameModePtr());
		if(netGameMode)
			netGameMode->ResetGameValues();

		if (lua_isnumber(pState, 1))
		{
			const U32 serverIndex = (U32)lua_tointeger(pState, 1);

			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			if (services.IsInitialised())
			{
				RacerNet::C_NetRace& race = services.Race();
				RacerNet::C_NetParty& party = services.Party();

				if (services.Router().BandwidthEvaluationInProgress())
				{
					LOGWARN(RacerNet::g_Log, "NetRace.JoinOnlineServer() - trying to join online session before bandwidth evaluation has finished.");
				}

				race.DisconnectAndClear();
				
				bool enumeratedJoin = true;
				RacerNet::NetJoinRaceFromPartyData partyData;
				if (party.IsRunning())
				{
					// party peer joining from a data set supplied from the host as opposed to data
					// enumerated from a matching query.
					if (party.GetJoinRaceFromPartyData(partyData))
						enumeratedJoin = !partyData.m_Active;
				}

				if (enumeratedJoin)
				{				
					NetPlay::C_SessionEnumerator& enumerator = services.MatchmakingSessionEnumerator();
					RacerNet::C_NetServices::NetMatchingRemapList& remapList = services.MatchingRemapList();
					if (serverIndex >= 0 && serverIndex < (U32)remapList.Size())
					{
						U64 consoleId = 0;
						if (NetPlay::C_Core::Instance().GenerateMachineIdentifier(consoleId))
						{
							RacerNet::C_NetCreationHelper& helper = services.SessionCreationHelper();
							RacerNet::C_NetRouterBase& router = services.Router();
							RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

							// [WillB] #33790 - Party host needs to send his party console id to the game host, otherwise the party clients will create another player for him.
							bool result = false;
							if ( party.IsRunning() )
							{
								result = helper.BuildLocalPlayersForOnlineFromParty(router.GetNatType(), party, services.PlayerManager());
							}
							else
							{
								result = helper.BuildLocalPlayersForOnline(router.GetNatType(), consoleId, router.EvaluatedUpstreamBandwidthKbps(), router.EvaluatedDownstreamBandwidthKbps());
							}

							if ( result )
							{
								NetPlay::C_SessionEnumerator::SessionQoSData qos;
								if (enumerator.GetQoSDataFromEnumerated(remapList[serverIndex], qos))
								{
									NetPlay::SessionSecurity security;
									bdCommonAddrRef hostAddress;
									if (enumerator.GetHostInfoFromEnumerated(remapList[serverIndex], security, hostAddress))
									{
										const RacerNet::NetQoSRace* pRaceQos = reinterpret_cast<RacerNet::NetQoSRace*> (qos.m_pQoSData);
										if (NULL != pRaceQos)
										{
											// session parameters will be modified as soon as the player has fully joined the game,
											// on creation of the peer session we just need basic data.
											RacerNet::NetMatchingServerData matchingServerCreationData;
											matchingServerCreationData.m_MatchingMode		= RacerNet::eNetMatchingModeNormal;
											matchingServerCreationData.m_PackVersion		= S32(		((C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion << 16) & 0xffff0000) 
																									|	((C_MultiplayerData::Instance().m_pConfig->m_PackVersion << 0) & 0x0000ffff));
											matchingServerCreationData.m_RankRestriction	= 0;
											matchingServerCreationData.m_MatchingRegion		= router.GetMatchingRegion();
											matchingServerCreationData.m_EventId			= 0;
											matchingServerCreationData.m_CustomRace			= 0;
											matchingServerCreationData.m_OpenSlotCount		= 0;
											matchingServerCreationData.m_Searchable			= 0;
											matchingServerCreationData.m_InRace				= 0;
											matchingServerCreationData.m_Nat				= router.GetNatType();

											// build platform specific matching server parameters list.
											RacerNet::NetMatchingParameterList matchingServerParamList;
											RacerNet::BuildMatchingParameters(matchingServerCreationData, matchingServerParamList, true, false);
											if (matchingServerParamList.Size() > 0)
											{
												NetPlay::C_Session& matchSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);
												
												NetPlay::C_Session::SlotConfig matchSlotConfig;
												matchSlotConfig.m_MaxPublic		= (pRaceQos->m_PublicGame != 0) ? pRaceQos->m_MaxPlayers : 0;
												matchSlotConfig.m_PublicInUse	= 0;
												matchSlotConfig.m_MaxPrivate	= (pRaceQos->m_PublicGame == 0) ? pRaceQos->m_MaxPlayers : 0;
												matchSlotConfig.m_PrivateInUse	= 0;

												U32 serverFlags = NetPlay::SESSION_CREATE_ONLINE_MULTIPLAYER_STANDARD;

												started = matchSession.CreatePeer(	helper.SessionManagerIndex(), 
																					serverFlags, 
																					&matchingServerParamList[0], 
																					matchingServerParamList.Size(), 
																					security,
																					matchSlotConfig);
											}
										}
									}
								}
							}
						}
					}
				}
				else
				{
					BIZ_ASSERT(partyData.m_MaxPlayers > 0);
					BIZ_ASSERT(partyData.m_MaxPlayers <= RacerNet::NET_MAX_VEHICLES);

					RacerNet::C_NetCreationHelper& helper = services.SessionCreationHelper();
					RacerNet::C_NetRouterBase& router = services.Router();
					if (helper.BuildLocalPlayersForOnlineFromParty(router.GetNatType(), party, services.PlayerManager()))
					{
						// session parameters will be modified as soon as the player has fully joined the game,
						// on creation of the peer session we just need basic data.
						RacerNet::NetMatchingServerData matchingServerCreationData;
						matchingServerCreationData.m_MatchingMode		= RacerNet::eNetMatchingModeNormal;
						matchingServerCreationData.m_PackVersion		= S32(		((C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion << 16) & 0xffff0000) 
																				|	((C_MultiplayerData::Instance().m_pConfig->m_PackVersion << 0) & 0x0000ffff));
						matchingServerCreationData.m_RankRestriction	= 0;
						matchingServerCreationData.m_MatchingRegion		= services.Router().GetMatchingRegion();
						matchingServerCreationData.m_EventId			= 0;
						matchingServerCreationData.m_CustomRace			= 0;
						matchingServerCreationData.m_OpenSlotCount		= 0;
						matchingServerCreationData.m_Searchable			= 0;
						matchingServerCreationData.m_InRace				= 0;
						matchingServerCreationData.m_Nat				= (S8)services.PlayerManager().GetMostRestrictiveNatType(true); // override with most restrictive players nat from the party.
						
						// build platform specific matching server parameters list.
						RacerNet::NetMatchingParameterList matchingServerParamList;
						RacerNet::BuildMatchingParameters(matchingServerCreationData, matchingServerParamList, true, false);
						if (matchingServerParamList.Size() > 0)
						{
							NetPlay::C_Session& matchSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);

							NetPlay::C_Session::SlotConfig matchSlotConfig;
							matchSlotConfig.m_MaxPublic		= (partyData.m_PublicGame != 0) ? partyData.m_MaxPlayers : 0;
							matchSlotConfig.m_PublicInUse	= 0;
							matchSlotConfig.m_MaxPrivate	= (partyData.m_PublicGame == 0) ? partyData.m_MaxPlayers : 0;
							matchSlotConfig.m_PrivateInUse	= 0;

							const U32 serverFlags = NetPlay::SESSION_CREATE_ONLINE_MULTIPLAYER_STANDARD;
							started = matchSession.CreatePeer(	helper.SessionManagerIndex(), 
																serverFlags, 
																&matchingServerParamList[0], 
																matchingServerParamList.Size(), 
																partyData.m_Security,
																matchSlotConfig);
						}
					}
				}
			}
		}

		//Data gathering time
		if(started)
		{
			pLayer->GetDataGathering().StoreLobbyChoice(false); //False = Joined lobby
		}

		lua_pushboolean(pState, started);
		return 1;
	}

	static S32 MonitorJoinOnlineServer(lua_State* pState)
	{
		bool complete = true;
		RacerNet::eNetJoinSessionErrors code = RacerNet::eJoinErrorInvalid;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			NetPlay::C_Session& matchSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);
			RacerNet::C_NetRace& race = services.Race();
			RacerNet::C_NetParty& party = services.Party();
			switch (matchSession.SessionState())
			{
			case NetPlay::C_Session::eStateCreating:
				complete = false;
				break;

			case NetPlay::C_Session::eStateError:
				race.DisconnectAndClear();
				break;

			case NetPlay::C_Session::eStateNotActive:
				{
					// we should only get here if the host sent us a join failure reply, the session will have been deleted.
					RacerNet::C_NetJoinHelper& join = race.JoinHelper();
					if (join.JoinGetState() == RacerNet::eJoinStateFailed)
						code = join.JoinGetLastError(); // failed to join so get reason why.
				}
				break;

			case NetPlay::C_Session::eStateRunning:
				// online peer session is running so lets join that thang.
				if (!race.IsRunning())
				{
					bool enumeratedJoin = true;
					RacerNet::NetJoinRaceFromPartyData partyData;
					if (party.IsRunning())
					{
						// party peer joining from a data set supplied from the host as opposed to data
						// enumerated from a matching query.
						if (party.GetJoinRaceFromPartyData(partyData))
							enumeratedJoin = !partyData.m_Active;
					}

					// start join procedure..
					if (enumeratedJoin)
					{
						if (lua_isnumber(pState, 1))
						{	
							const U32 serverIndex = (U32)lua_tointeger(pState, 1);
							NetPlay::C_SessionEnumerator& enumerator = services.MatchmakingSessionEnumerator();
							RacerNet::C_NetServices::NetMatchingRemapList& remapList = services.MatchingRemapList();
							if (serverIndex >= 0 && serverIndex < (U32)remapList.Size())
							{
								NetPlay::C_SessionEnumerator::SessionQoSData qos;
								if (enumerator.GetQoSDataFromEnumerated(remapList[serverIndex], qos))
								{
									NetPlay::SessionSecurity security;
									bdCommonAddrRef hostAddress;
									if (enumerator.GetHostInfoFromEnumerated(remapList[serverIndex], security, hostAddress))
									{
										const RacerNet::NetQoSRace* pRaceQos = reinterpret_cast<RacerNet::NetQoSRace*> (qos.m_pQoSData);
										if (NULL != pRaceQos && hostAddress.notNull())
										{
											U8 hostTextAddr[BD_COMMON_ADDR_SERIALIZED_SIZE];	
											hostAddress->serialize(hostTextAddr);
											if (race.JoinOnlineSession(services.SessionCreationHelper(), security, hostTextAddr, pRaceQos->m_HostConsoleId, pRaceQos->m_Host))
												complete = false;
										}
									}
								}
							}
						}
					}
					else
					{
						if (race.JoinOnlineSession(services.SessionCreationHelper(), partyData.m_Security, partyData.m_HostTextAddress, partyData.m_RaceHostConsoleId, partyData.m_HostName))
							complete = false;
					}

					if (complete) // failed to start join procedure in some way.
						race.DisconnectAndClear();
				}
				else
				{
					// monitor join procedure.
					RacerNet::C_NetJoinHelper& join = race.JoinHelper();
					if (join.IsJoining())
						complete = false;
					else
					{
						switch (join.JoinGetState())
						{
						case RacerNet::eJoinStateComplete:
							code = RacerNet::eJoinErrorNone; // join successful.
							break;

						case RacerNet::eJoinStateFailed:
							code = join.JoinGetLastError(); // failed to join so get reason why.
							break;
						}
					}
				}

				break;
			}
		}

		lua_pushboolean(pState, complete);
		lua_pushnumber(pState, code);
		return 2;
	}

	static S32 OnlineJoinFailedNameAvailable(lua_State* pState)
	{
		bool avail = false;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			RacerNet::C_NetJoinHelper& join = race.JoinHelper();
			avail = join.IsPeerJoinFailureDataValid();
		}
		lua_pushboolean(pState, avail);
		return 1;
	}

	static S32 EnterDisconnecting(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				U8 count = 0;
				// change all local player to disconnecting from race
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p && p->LocalPlayer() && p->m_RaceConnection.IsActive() && p->m_RaceConnection.m_PlayerState != RacerNet::eNetPlayerStateDisconnecting )
					{
						p->m_RaceConnection.m_PlayerState = RacerNet::eNetPlayerStateDisconnecting; 
						count++;
					}
				}

				//End the subession
				Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
				if(pLayer)
				{
					pLayer->GetDataGathering().FlagPostEvent(Game::DATA_GATHERING_RECORD_TYPE_QUIT_RACE);
				}
#ifndef CFG_GOLD
				AmaxDataLogging::LogRaceEnd();
#endif
				RacerNet::C_NetJoinHelper& join = race.JoinHelper();
				if (join.JoinGetState() == RacerNet::eJoinStateComplete)
				{
					// inform peers of new state(s)
					if ( count > 0 )
						race.SendLocalPlayerStatesMessage();
				}
				else
				{
					LOGWARN(RacerNet::g_Log, "NetRace.EnterDisconnecting() - called before join has finished.");
				}
			}
		}
		return 0;
	}

	static S32 EnterLobby(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			C_GameApp::GetInstance()->GetGame()->GetNetGameVars().GetLobbyWaitingRaceInfo().Reset();
			
			services.Race().EnterLobby();
			Game::C_GameModeNetwork* netGameMode = (Game::C_GameModeNetwork*)(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr());
			if(netGameMode)
			{
				netGameMode->PreSelectRandom();
	
				// [WillB] #37103 - Possible fix for the lobby auto booting the race. If we ever enter the lobby in this state and we are the host, then its gone screwy. If im a client
				// then i will recieve an update from the host or the start race packet so we will boot the race correctly.
				if ( C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyState == UI::E_LobbyState_Ready )
					netGameMode->UpdateLobbyState( UI::E_LobbyState_Waiting, 0.0f );

				C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_StartRaceDataValid = false;
			}
		}
		return 0;
	}

	static int EnterRaceLoad(lua_State* pState)
	{
		LOGINFO(RacerNet::g_Log, "## NetRace.EnterRaceLoad() ##");

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				// let net race layer know we are now falgged as being in a race event.
				race.FlagInRace(true);

				// change all local player to loading race state
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p && p->LocalPlayer() && p->m_RaceConnection.IsActive())
						p->m_RaceConnection.m_PlayerState = RacerNet::eNetPlayerStateLoadingRace; 
				}

				// inform peers of new state(s)
				race.SendLocalPlayerStatesMessage();

				// is this an online race ? if so we need to update the server with latest params and start play.
				if (race.IsOnline())
				{
					// force an update of the matching params and add / remove any players to the matching session
					race.UpdateMatchingServer(true); 

					// on ps3 add players who are racing into my met players list.
					services.AddToRecentPlayers();

					// inform the matching session that play has started.
					services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace).StartPlay();
				}
			}
		}

		return 0;
	} 

	static int EnterSyncAfterRaceLoad(lua_State* pState)  
	{
		LOGINFO(RacerNet::g_Log, "## NetRace.EnterSyncAfterRaceLoad() ##");

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance(); 
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				// change all local player to loading race state
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p  && p->m_RaceConnection.IsActive())
					{
						if (p->LocalPlayer()) // new state
							p->m_RaceConnection.m_PlayerState = RacerNet::eNetPlayerStateWaitingForOthers;

						// clear this since next race has loaded now. local and remote
						p->m_RaceData.m_BitArray.ClearBit(RacerNet::NetPlayerRaceGameData::eBitArrayData_WantsToSkipNextRace);
					}
				}

				// inform peers of new state(s)
				race.SendLocalPlayerStatesMessage();

				// race logic can now accept incoming data.
				Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
				netGameVars.m_InRace = true;
			}
		}

		return 0;
	}

	static int StartPlay(lua_State* pState)  
	{
		LOGINFO(RacerNet::g_Log, "## NetRace.StartPlay() ##");

		Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance(); 

		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				// change all local player to loading race state
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

				U8 numLocal = 0;
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p && p->LocalPlayer() && p->m_RaceConnection.IsActive())
					{
						numLocal += 1;
						p->m_RaceConnection.m_PlayerState = RacerNet::eNetPlayerStateRacing;
					}
				}

				// inform peers of new state(s)
				if (numLocal)
				{
					// switch voice over to in game play processing. 
					mgr.SetVoiceControlState(RacerNet::eVoiceControlStateGamePlay);

					race.SendLocalPlayerStatesMessage();

					// enable packet transfer, race logic can start sending packets now durring the 321.
					netGameVars.m_PacketTransferEnabled = true;
				}

				// reset the racing physics frame count for network interpolation.
				RacingPhysics::PhysicsTimer::Instance().StoreRaceStartFrameCount();
			}
		}

		return 0;
	}

	static int EnterResults(lua_State* pState)
	{
		LOGINFO(RacerNet::g_Log, "## NetRace.EnterResults() ##");

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance(); 
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

				// unregister race mode listener, no more notifications need passing
				//race.RegisterRaceLogicListener(NULL);

				// voice switches back to race connection.
				//mgr.SetVoiceControlState(RacerNet::eVoiceControlStateRaceConnection);

				// change all local player to viewing results. other online functionality may be running asynchronously.
				U8 numLocal = 0;
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p && p->m_RaceConnection.IsActive())
					{
						if (p->LocalPlayer())
						{
							numLocal += 1;
							p->m_RaceConnection.m_PlayerState = RacerNet::eNetPlayerStateViewingResults;
						}
					}
				}

				// inform peers of new state(s)
				if (numLocal)
					race.SendLocalPlayerStatesMessage();
			}
		}
		return 0;
	}

	static int EnterResultsIdle(lua_State* pState)
	{
		LOGINFO(RacerNet::g_Log, "## NetRace.EnterResultsIdle() ##");

		Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		netGameVars.m_PacketTransferEnabled = false;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance(); 
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

				// change all local player to idle in the results screen..
				U8 numLocal = 0;
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p && p->m_RaceConnection.IsActive())
					{
						if (p->LocalPlayer())
						{
							numLocal += 1;
							p->m_RaceConnection.m_PlayerState = RacerNet::eNetPlayerStateViewingResultsIdle;
						}

						// clear vehicle index that was used by this player during that race. no longer needed.
						p->m_RaceData.m_InRaceVehicleIndex = -1;
					}
				}

				// inform peers of new state(s)
				if (numLocal)
					race.SendLocalPlayerStatesMessage();
			}
		}
		return 0;
	}

	static int EndPlay(lua_State* pState)  
	{
		Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		
		// are we actually going back to the lobby.
		bool backToLobby = false;
		if (lua_isboolean(pState, 1))
			backToLobby = lua_toboolean(pState, 1) ? true : false;

		// leave any perisistent data since we are going back to the lobby.
		const bool dataValid = netGameVars.m_StartRaceDataValid;
		netGameVars.Clear();

		if (backToLobby)
		{
			LOGINFO(RacerNet::g_Log, "## NetRace.EndPlay() - lobby ##");
		}
		else
		{
			LOGINFO(RacerNet::g_Log, "## NetRace.EndPlay() - next race ##");

			// we will have alrady received the start next race notification so we need to enable the variable again
			// since we will have reset it in the Clear() above..
			netGameVars.m_StartRaceDataValid = dataValid;  	
		}

		// set no active race mode if returning to lobby. E_RaceMode_Nothing resolves to an empty race mode.
		if (backToLobby)
		{
			C_GameApp::GetInstance()->GetRaceBookLayer()->SetRaceMode(Game::E_RaceMode_Nothing);
		}

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance(); 
		if (services.IsInitialised())
		{
			// flush scoreboard stuff..
			services.StopScoreboardRequests();

			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

				// unregister race mode listener, no more notifications need passing
				race.RegisterRaceLogicListener(NULL);

				// voice switches back to race connection.
				mgr.SetVoiceControlState(RacerNet::eVoiceControlStateRaceConnection);

				// no longer considered in the race.
				if (backToLobby)
					race.FlagInRace(false);

				// if session is online update the server parameters.
				if (race.IsOnline())
					race.UpdateMatchingServer(true); 

				// change all local player to loading UI.
				U8 numLocal = 0;
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p && p->m_RaceConnection.IsActive())
					{
						// clear vehicle index that was used by this player during that race.
						p->m_RaceData.m_InRaceVehicleIndex = -1;

						if (p->LocalPlayer())
						{
							numLocal += 1;
							if (backToLobby)
								p->m_RaceConnection.m_PlayerState = RacerNet::eNetPlayerStateLoadingUi;
							else
								p->m_RaceConnection.m_PlayerState = RacerNet::eNetPlayerStateLoadingRace;
						}
					}
				}

				// inform peers of new state(s)
				if (numLocal)
					race.SendLocalPlayerStatesMessage();
			}
		}

		return 0;
	}

	static S32 CanStartCustomRace(lua_State* pState)
	{
		bool canStart = false;
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if (pLayer)
		{
			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			if (services.IsInitialised())
			{
				RacerNet::C_NetRace& race = services.Race();
				if (race.IsInitialised() && race.IsRunning())
				{
					RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				
					// is a local player hosting the custom race ?
					RacerNet::C_NetPlayer* hostPlayer = mgr.HostPlayer(false);
					if (hostPlayer && hostPlayer->LocalPlayer())
					{
						RacerNet::NetRaceSettings raceSettings;
						if (pLayer->GetGameModePtr()->GetNetRaceSettings(raceSettings))
						{	
							if (mgr.Size(false) > 0) // must have at least 1 race player defined.
							{
								bool allIdle = true;			// all race players must be idle
								bool allVehiclesValid = true;	// all race players vehicles must be valid 
								U8 numRemote = 0;				// must have at least 1 remote player to race
								for (U8 i=0; i<mgr.TotalSize(); i++)
								{
									RacerNet::C_NetPlayer* p = mgr.Index(i);
									if (!p->m_RaceConnection.IsActive())
										continue; // ignore network players who dont have a race connection, could have a party connection only. 

									if ( RacerNet::eNetPlayerStateJoining == p->m_RaceConnection.m_PlayerState || RacerNet::eNetPlayerStateJoinComplete == p->m_RaceConnection.m_PlayerState )
										continue; // Ignore players who are connecting

									numRemote += (p->LocalPlayer() == true) ? 0 : 1;

									if (RacerNet::eNetPlayerStateIdle != p->m_RaceConnection.m_PlayerState && RacerNet::eNetPlayerStateChangingCar != p->m_RaceConnection.m_PlayerState)
										allIdle = false;

									if (!race.ValidateVehicle(p->m_RaceData.m_VehicleData.m_xtVehicleId))
										allVehiclesValid = false;
								}

								if (allIdle && allVehiclesValid && numRemote > 0 && !race.IsMigrating())
									canStart = true;
							}
						}
					}
				}
			}
		} 

#if !defined ( CFG_GOLD )
		/*
		if (!stricmp(Debug::GetBuildUser(), "dworswick") || !stricmp(Debug::GetBuildUser(), "psnape") || !stricmp(Debug::GetBuildUser(), "gayoung") || !stricmp(Debug::GetBuildUser(), "wbrown") || !stricmp(Debug::GetBuildUser(), "pkerby"))
		{
			canStart = true;
		}
		*/

		if ( !canStart )
			canStart = g_AllowSingleRacer;
#endif

		lua_pushboolean(pState, canStart);
		return 1;
	}

	static S32 CanStartNextCustomRace(lua_State* pState)
	{
		bool canStart = false;
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if (pLayer)
		{
			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			if (services.IsInitialised())
			{
				RacerNet::C_NetRace& race = services.Race();
				if (race.IsInitialised() && race.IsRunning())
				{
					RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

					// is a local player hosting the custom race ?
					RacerNet::C_NetPlayer* hostPlayer = mgr.HostPlayer(false);
					if (hostPlayer && hostPlayer->LocalPlayer())
					{
						RacerNet::NetRaceSettings raceSettings;
						if (pLayer->GetGameModePtr()->GetNetRaceSettings(raceSettings))
						{	
							if (mgr.Size(false) > 0) // must have at least 1 race player defined.
							{
								bool allIdle = true;				// all race players must be idle
								bool allVehiclesValid = true;		// all race players vehicles must be valid
								bool enoughActivePlayers = false;	// make sure we've still got enough active players						
								U8 activePlayers = 0;

								for (U8 i=0; i<mgr.TotalSize(); i++)
								{
									RacerNet::C_NetPlayer* p = mgr.Index(i);
									if (!p->m_RaceConnection.IsActive())
										continue; // ignore network players who dont have a race connection, could have a party connection only. 

									// idle will bring players in the lobby into the next race.
									// viewing results idle is for the players who just finished the current event. in a state where they can leave at any time.
									if (RacerNet::eNetPlayerStateIdle != p->m_RaceConnection.m_PlayerState && RacerNet::eNetPlayerStateViewingResultsIdle != p->m_RaceConnection.m_PlayerState)
										allIdle = false;

									if (!race.ValidateVehicle(p->m_RaceData.m_VehicleData.m_xtVehicleId))
										allVehiclesValid = false;

									activePlayers++;
								}

								//Bug 26060 - Players dropping out between games
								if(activePlayers >= raceSettings.m_MinDrivers)
								{
									enoughActivePlayers = true;
								}

								if (allIdle && allVehiclesValid && !race.IsMigrating() && enoughActivePlayers)
									canStart = true;
							}
						}
					}
				}
			}
		} 

		lua_pushboolean(pState, canStart);
		return 1;
	}

	static S32 CanStartResultsWinnner( lua_State* pState )
	{
			bool ok = true;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (!p->m_RaceConnection.IsActive())
						continue; // ignore network players who dont have a race connection, could have a party connection only. 

					switch (p->m_RaceConnection.m_PlayerState)
					{
					case RacerNet::eNetPlayerStateRacing:
					case RacerNet::eNetPlayerStateLoadingRace:
						ok = false;
						break;

					}
				}
			}
		}

		lua_pushboolean(pState, ok);
		return 1;
	}

	static S32 CanStartResultsCountdown(lua_State* pState)
	{
		bool ok = true;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (!p->m_RaceConnection.IsActive())
						continue; // ignore network players who dont have a race connection, could have a party connection only. 

					switch (p->m_RaceConnection.m_PlayerState)
					{
					case RacerNet::eNetPlayerStateRacing:		
					case RacerNet::eNetPlayerStateViewingResults:
						ok = false;
						break;

					}
				}
			}
		}

		lua_pushboolean(pState, ok);
		return 1;
	}

	static S32 CanStartVoting( lua_State* pState )
	{
		bool ok = true;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (!p->m_RaceConnection.IsActive())
						continue; // ignore network players who dont have a race connection, could have a party connection only. 

					switch (p->m_RaceConnection.m_PlayerState)
					{
					case RacerNet::eNetPlayerStateRacing:
					case RacerNet::eNetPlayerStateFinished:
					case RacerNet::eNetPlayerStateViewingResults:
					case RacerNet::eNetPlayerStateViewingResultsIdle:
						ok = false;
						break;

					}
				}
			}
		}
		lua_pushboolean( pState, ok );
		return 1;
	}

	static S32 LaunchCustomRace(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(Game::E_GameflowMessage_StopGameRendering);
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(Game::E_GameflowMessage_StopGameUpdate);

			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				C_GameModeBase* pGameMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

				Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();

				// we are hosting the race that is about to start.
				netGameVars.m_RaceHost = true;

				RacerNet::NetStartRacePacket& startRacePacket = netGameVars.m_StartRaceData;

				pGameMode->BuildStartRacePacket(&startRacePacket);
				C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_StartRaceDataValid = true;

				RacerNet::C_NetMessage& netMsg = services.Router().GetOutgoingReliable();
				netMsg.CreateReliable();
				netMsg.WriteBuffer(&startRacePacket, sizeof(startRacePacket));
				netMsg.TerminateMessage();

				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);

					// Want to ignore players who are still connecting
					if (!p->LocalPlayer() && p->PrimaryPlayer() && p->m_RaceConnection.CanMessage() && ( p->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateIdle || p->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateChangingCar ) )
					{
						p->SendReliable(netMsg, RacerNet::eNetMessage_StartRace, false, mgr.LocalPrimaryJoinId(false));
					}
				}
				//Host needs to build championship standings structure as they won't be receiving the star race message which tells everyone else to build it
				if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
				{
					((C_GameModeNetwork*)pGameMode)->InitChampionshipStandingsStructure();
				}
			}
		}

		return 0;
	}

	static S32 IsStartRaceDataValid(lua_State* pState)
	{
		bool ready = false;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				bool ok = true;
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p->LocalPlayer() && p->m_RaceConnection.IsActive())
					{
						if (RacerNet::eNetPlayerStateDisconnecting == p->m_RaceConnection.m_PlayerState)
							ok = false;
					}
				}

				// check to be sure no local players are in the disconnecting state. dont want to start the race is so.
				if (ok)
					ready = C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_StartRaceDataValid;
			}
		}

		lua_pushboolean(pState, ready);
		return 1;
	}

	static int RaceStartSynchronised(lua_State* pState)
	{
		bool synchronised = false;
		Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				// if we have already received packets from a remote peer then start the race now.
				if (!netGameVars.m_ReceivedRemoteRacePackets)
				{
					// all players who started fully loaded ?
					synchronised = true;
					RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
					for (U8 i=0; i<mgr.TotalSize(); i++)
					{
						RacerNet::C_NetPlayer* p = mgr.Index(i);
						if (p->m_RaceConnection.IsActive() && p->m_RaceData.m_InRaceVehicleIndex != -1)
						{
							if (RacerNet::eNetPlayerStateWaitingForOthers > p->m_RaceConnection.m_PlayerState)
								synchronised = false;
						}
					}
				}
				else
					synchronised = true;
			}
			else
				synchronised = true;
		}
		else 
			synchronised = true;

		lua_pushboolean(pState, synchronised);
		return 1;
	}

	static int DisconnectIdleLoadingPlayers(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				//	Find the players who haven't said "I'M READY!" and kill them
				race.DisconnectIdleLoadingPlayers();
			}
		}
		return 0;
	}

	static S32 GetOnlineLobbyList( lua_State* pState )
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		BIZ_ASSERT( services.IsInitialised() );

		RacerNet::C_NetServices::NetMatchingRemapList& remapList = services.MatchingRemapList();

		NetPlay::C_SessionEnumerator& enumerator = services.MatchmakingSessionEnumerator();
		NetPlay::C_SessionEnumerator::SessionQoSData qos;

		lua_newtable( pState );

		for ( S32 i = 0; i < remapList.Size(); i++ )
		{
			bool race_in_progress = false;

			if ( enumerator.GetQoSDataFromEnumerated( remapList[i], qos ) || qos.m_pQoSData == NULL )
			{
				const RacerNet::NetQoSRace* pQos = reinterpret_cast<RacerNet::NetQoSRace*> (qos.m_pQoSData);
				BIZ_ASSERT(pQos->m_Type == RacerNet::NetQoSBase::QoSTypeRace);

				race_in_progress = ( RacerNet::NetQoSBase::QoSStateInRace == pQos->m_SessionState ) ? true : false;
			}

			lua_pushboolean( pState, race_in_progress );
			lua_rawseti( pState, -2, i+1 );
		}
		return 1;
	}


	static S32 GetLanLobbyList( lua_State* pState )
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		BIZ_ASSERT( services.IsInitialised() );

		RacerNet::C_LanDiscoveryManager& lanDiscovery = services.Race().LanDiscoveryManager();

		lua_newtable( pState );

		for ( U32 i = 0; i < lanDiscovery.NumEnumerated(); i++ ) 
		{
			// their wouldnt be race in progress right???
			bool race_in_progress = false;

			lua_pushboolean( pState, race_in_progress );
			lua_rawseti( pState, -2, i+1 );
		}
		return 1;
	}

	static S32 FlagInRace(lua_State* pState)
	{
		if (lua_isboolean(pState, 1))
		{
			const bool inRace = lua_toboolean(pState, 1) ? true : false;

			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			if (services.IsInitialised())
			{
				RacerNet::C_NetRace& race = services.Race();
				if (race.IsInitialised() && race.IsRunning())
				{
					race.FlagInRace(inRace);

					if (race.IsOnline()) // need to change the matching state on the server based opn if we are racing or available to be joined.
						race.UpdateMatchingServer(true);
				}
			}
		}
		return 0;
	}

	static S32 SendNewRaceSettings(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised() && services.Race().IsRunning())
		{
			services.Race().SendNewRaceDataMessage();

			// Inform the lobby to refresh
			_UI_MESSAGE1(UI::E_UiGameMsg_RefreshLobby, false);
		}
		return 0;
	}

	static S32 GetSkipNextRaceValues(lua_State* pState)
	{
		bool isHost = false;
		U32 skipCount = 0;
		U32 skipTotal = 0;
		
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p && p->m_RaceConnection.IsActive())
					{
						if (p->LocalPlayer() && p->m_RaceConnection.m_Host)
							isHost = true;

						switch (p->m_RaceConnection.m_PlayerState)
						{
						case RacerNet::eNetPlayerStateViewingResults:
						case RacerNet::eNetPlayerStateViewingResultsIdle:
						case RacerNet::eNetPlayerStateIdle:
							skipTotal += 1;
							if (p->m_RaceData.m_BitArray.GetBitValue(RacerNet::NetPlayerRaceGameData::eBitArrayData_WantsToSkipNextRace))
								skipCount += 1;
							break;
						}
					}
				}
			}
		}

		lua_pushboolean(pState, isHost);
		lua_pushnumber(pState, skipCount);
		lua_pushnumber(pState, skipTotal);
		return 3;
	}

	static S32 HasVotingStarted(lua_State* pState)
	{
		lua_pushboolean(pState, C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_VoteStarted);
		return 1;
	}

	static S32 SendBeginVoteMessage(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised() && services.Race().IsRunning())
		{
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
			RacerNet::C_NetPlayer* hostPlayer = mgr.HostPlayer(false);
			if (hostPlayer && hostPlayer->LocalPlayer())
			{
				if (hostPlayer->m_RaceConnection.IsActive())
				{
					RacerNet::C_NetMessage& netMsg = services.Router().GetOutgoingReliable();
					netMsg.CreateReliable();

					netMsg.WriteS8( 1 );
					netMsg.TerminateMessage();

					for (U8 i=0; i<mgr.TotalSize(); i++)
					{
						RacerNet::C_NetPlayer* p = mgr.Index(i);
						if (!p->LocalPlayer())
						{
							if (p->PrimaryPlayer() && p->m_RaceConnection.CanMessage() && p->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateIdle)
								p->SendReliable(netMsg, RacerNet::eNetMessage_BeginVoting, false, mgr.LocalPrimaryJoinId(false));
						}
					}
				}
			}
			else
			{
				LOGWARN(RacerNet::g_Log, "NetRace.SendBeginVoteMessage() - called for non host player !");
			}
		}
		return 0;
	}

	static int VoteToSkipNextRace(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
				RacerNet::NetRaceSettings settings;
				if (pLayer->GetGameModePtr()->GetNetRaceSettings(settings))
				{
					if (settings.m_Ranked)
					{
						RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
						if (mgr.LocalSize() > 0)
						{
							RacerNet::C_NetPlayer* pl = mgr.Index(0);
							if (pl && pl->m_RaceConnection.IsActive())
							{
								if (!pl->m_RaceData.m_BitArray.GetBitValue(RacerNet::NetPlayerRaceGameData::eBitArrayData_WantsToSkipNextRace))
								{
									RacerNet::C_NetMessage& netMsg = services.Router().GetOutgoingReliable();
									netMsg.CreateReliable();
									netMsg.WriteU64(pl->m_ConsoleId);
									netMsg.TerminateMessage();

									for (U8 i=0; i<mgr.TotalSize(); i++)
									{
										RacerNet::C_NetPlayer* p = mgr.Index(i);
										if (!p->LocalPlayer())
										{
											if (p->PrimaryPlayer() && p->m_RaceConnection.CanMessage())
												p->SendReliable(netMsg, RacerNet::eNetMessage_VoteToSkipNextRace, false, mgr.LocalPrimaryJoinId(false));
										}
										else
										{
											// update all local players bits to say they have voted.
											if (p->m_RaceConnection.IsActive())
												p->m_RaceData.m_BitArray.SetBit(RacerNet::NetPlayerRaceGameData::eBitArrayData_WantsToSkipNextRace);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return 0;
	}

	static S32 VoteOnNextRace( lua_State* pState )
	{
		if ( lua_type( pState, 1 ) != LUA_TNUMBER )
		{
			UI::g_pUILua->StackDump( pState, "VoteOnNextRace( num ) - incorrect arguments\n" );
			return 0;
		}

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if ( !services.IsInitialised() )
			return 0;

		RacerNet::C_NetRace& race = services.Race();
		if ( !race.IsInitialised() || !race.IsRunning() )
			return 0;

		const S8 vote = (S8)lua_tonumber( pState, 1 );

		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		Game::C_GameModeMultiPlayer* pGameMode = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr());

		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		if ( mgr.LocalSize() <= 0 )
			return 0;

		// Local player only
		RacerNet::C_NetPlayer* pl = mgr.Index(0);
		if ( pl && pl->m_RaceConnection.IsActive() )
		{
			if ( !C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_VoteCast )
			{
				VoteInfo& voteInfo = const_cast<VoteInfo&>(pGameMode->GetVoteInfo());
				voteInfo.myVote = vote;

				RacerNet::C_NetMessage& netMsg = services.Router().GetOutgoingReliable();
				netMsg.CreateReliable();
				netMsg.WriteU64(pl->m_ConsoleId);
				netMsg.WriteS8( vote );
				netMsg.TerminateMessage();

				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (!p->LocalPlayer())
					{
						if (p->PrimaryPlayer() && p->m_RaceConnection.CanMessage())
							p->SendReliable(netMsg, RacerNet::eNetMessage_VoteOnNextRace, false, mgr.LocalPrimaryJoinId(false));
					}
					else
					{
						// update all local players bits to say they have voted.
						if ( p->m_RaceConnection.IsActive() )
						{
							C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_VoteCast = true;

							pGameMode->VoteOnRace( vote );
						}
					}
				}

			}
		}
		return 0;
	}

	static S32 SendVoteResults( lua_State* pState )
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if ( !services.IsInitialised() )
			return 0;

		RacerNet::C_NetRace& race = services.Race();
		if ( !race.IsInitialised() || !race.IsRunning() )
			return 0;

		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		BIZ_ASSERT( pLayer->GetGameModePtr()->IsMultiplayer() );

		Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();


		pGameMode->CalculateVoteResults();
		const VoteInfo& voteInfo = pGameMode->GetVoteInfo();

		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		if ( mgr.LocalSize() <= 0 )
			return 0;

		// Local player only who is the host
		RacerNet::C_NetPlayer* pl = mgr.Index(0);
		if ( pl && pl->m_RaceConnection.IsActive() && pl->m_RaceConnection.m_Host )
		{
			RacerNet::C_NetMessage& netMsg = services.Router().GetOutgoingReliable();
			netMsg.CreateReliable();
			netMsg.WriteS8( voteInfo.result );
			netMsg.WriteS16( pGameMode->GetPlaylist().GetRaceIndex() ); // TODO_MP_REFACTOR
			netMsg.TerminateMessage();

			for (U8 i=0; i<mgr.TotalSize(); i++)
			{
				RacerNet::C_NetPlayer* p = mgr.Index(i);
				if (!p->LocalPlayer())
				{
					if (p->PrimaryPlayer() && p->m_RaceConnection.CanMessage())
						p->SendReliable(netMsg, RacerNet::eNetMessage_VoteResults, false, mgr.LocalPrimaryJoinId(false));
				}
			}

			//Data gathering time
			const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
			if(pEvent)
			{
				const MultiplayerRace* pLeftRace = pGameMode->GetPlaylist().GetRaceData( voteInfo.indexA );
				const MultiplayerRace* pRightRace = pGameMode->GetPlaylist().GetRaceData( voteInfo.indexB );

				if(pLeftRace && pRightRace)
				{
					if(voteInfo.countA > voteInfo.countB) //Left wins
					{
						pLayer->GetDataGathering().StoreVoteResults(voteInfo.countA, pLeftRace->route->ID, 
																		pRightRace->route->ID, voteInfo.countB);
#ifndef CFG_GOLD
						AmaxDataLogging::LogMpVoteResults(pLeftRace->route->ID, voteInfo.countA);
#endif
					}
					else //Right wins (Or a draw... but we can easily hide draws with a database query)
					{
						pLayer->GetDataGathering().StoreVoteResults(voteInfo.countB, pRightRace->route->ID, 
																		pLeftRace->route->ID, voteInfo.countA);
#ifndef CFG_GOLD
						AmaxDataLogging::LogMpVoteResults(pRightRace->route->ID, voteInfo.countB);
#endif
					}
				}
			}
		}

		return 0;
	}

	static S32 ReceivedVotingResults( lua_State* pState )
	{
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		Game::C_GameModeMultiPlayer* pGameMode = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr());

		const VoteInfo& voteInfo = pGameMode->GetVoteInfo();
		lua_pushboolean( pState, ( voteInfo.result != -1 ) );
#ifndef CFG_GOLD
		if(voteInfo.result != -1)
		{
			const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
			const MultiplayerRace* pWinningRace = pEvent->races[voteInfo.result == 0 ? voteInfo.indexA : voteInfo.indexB ];

			if(voteInfo.myVote == voteInfo.result)
			{
				AmaxDataLogging::LogMpWinningVoteCast(pWinningRace->route->ID);
			}
			//End the subsession so we can get a new one for the next race
			AmaxDataLogging::LogRaceEnd();
		}
#endif
		return 1;
	}

	static S32 MaxPlayersFound( lua_State* pState )
	{
		RacerNet::NetRaceSettings raceSettings;
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( raceSettings );

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

		bool result = ( mgr.Size(false) >= raceSettings.m_MaxDrivers );

		lua_pushboolean( pState, result );
		return 1;
	}

	static S32 MaxPlayerCount( lua_State* pState )
	{
		RacerNet::NetRaceSettings raceSettings;
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( raceSettings );

		lua_pushnumber( pState, raceSettings.m_MaxDrivers );
		return 1;
	}

	static S32 EnoughPlayersFound( lua_State* pState )
	{
		RacerNet::NetRaceSettings raceSettings;
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( raceSettings );

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

		// Only include players who are ready to play. Not connecting players
		U8 playersReady = 0;
		for ( U8 i = 0; i < mgr.TotalSize(); i++)
		{
			RacerNet::C_NetPlayer* p = mgr.Index( i );
			if ( !p || !p->m_RaceConnection.IsActive() )
				continue; // ignore network players who dont have a race connection, could have a party connection only. 

			if ( p->m_RaceConnection.m_PlayerState >= RacerNet::eNetPlayerStateIdle )
				playersReady++;
		}

		bool result = ( playersReady >= raceSettings.m_MinDrivers );
		
#ifndef CFG_GOLD
		if ( -1 != g_MinRankedPlayers )
			result = ( playersReady >= g_MinRankedPlayers );

		if ( !result )
			result = g_AllowSingleRacer;
#endif // CFG_GOLD

		lua_pushboolean( pState, result );
		return 1;
	}

	static S32 IsFirstRace( lua_State* pState )
	{
		RacerNet::NetRaceSettings raceSettings;
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( raceSettings );

		bool result = !raceSettings.m_HasAnyRacesStarted;
		lua_pushboolean( pState, result );
		return 1;
	}


	static S32 IsHost( lua_State* pState )
	{
		bool ishost = false;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		RacerNet::C_NetPlayer* hostPlayer = mgr.HostPlayer(false);

		if ( hostPlayer && hostPlayer->LocalPlayer() )
			ishost = true;

		lua_pushboolean( pState, ishost );
		return 1;
	}

	static S32 GetCarClass( lua_State* pState )
	{
		// TODO_MP_REFACTOR
		UI::E_VehicleClass raceClass = UI::E_VehicleClass_A;
		AmaxCarClass validClass = ((Game::C_GameModeMultiPlayer*)UI::GetRaceBookLayer()->GetGameModePtr())->GetRaceCarClass();
		
		raceClass = (UI::E_VehicleClass)validClass;

		lua_pushnumber(pState, (S32)raceClass);

		return 1;
	}

	static S32 IsTeamGame( lua_State* pState )
	{
		RacerNet::NetRaceSettings raceSettings;
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( raceSettings );

		lua_pushboolean( pState, ( raceSettings.m_Teams ) ? true : false );
		return 1;
	}

	static S32 AreTeamsBalanced( lua_State* pState )
	{
		bool result = true;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if ( services.IsInitialised() )
		{
			RacerNet::C_NetRace& race = services.Race();
			if ( race.IsInitialised() && race.IsRunning() )
				result = race.GetTeamBalancer().Balanced();
		}

#ifndef CFG_GOLD
		if ( !result )
			result = g_AllowSingleRacer;
#endif

		lua_pushboolean( pState, result );
		return 1;
	}

	static S32 AreTeamsValid( lua_State* pState )
	{
		bool result = true;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if ( services.IsInitialised() )
		{
			RacerNet::C_NetRace& race = services.Race();
			if ( race.IsInitialised() && race.IsRunning() )
				result = race.GetTeamBalancer().Valid();
		}

#ifndef CFG_GOLD
		if ( !result )
			result = g_AllowSingleRacer;
#endif

		lua_pushboolean( pState, result );
		return 1;
	}

	static S32 GetLocalPlayerTeam( lua_State* pState )
	{
		// Get the game mode pointer
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		Game::C_GameModeNetwork* pGameMode = ((Game::C_GameModeNetwork*)pLayer->GetGameModePtr());

		lua_pushnumber( pState, pGameMode->GetLocalPlayerTeam() );
		return 1;
	}

	static S32 GetGarageMiniClassFilters( lua_State* pState )
	{
		if ( !UI::GetRaceBookLayer() || !UI::GetRaceBookLayer()->GetGameModePtr() )
			return 0;

		AmaxCarClass validClass = ((Game::C_GameModeMultiPlayer*)UI::GetRaceBookLayer()->GetGameModePtr())->GetRaceCarClass();

		lua_newtable( pState );

		lua_newtable( pState );
		lua_rawseti( pState, -2, (S32)validClass );

		return 1;
	}	

	// TODO - Move me to GameMode layer
	static S32 SendLobbyState(lua_State* pState)
	{
		if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2) != LUA_TNUMBER )
		{
			UI::g_pUILua->StackDump( pState, "SendTimer( num ) - incorrect arguments\n" );
			return 0;
		}

		// Lua is controlling the lobby state
		const UI::E_LobbyState state = (UI::E_LobbyState)(S32)lua_tonumber( pState, 1 );
		const F32 time = (F32)lua_tonumber( pState, 2 );

		// Get the game mode pointer
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		Game::C_GameModeNetwork* pGameMode = ((Game::C_GameModeNetwork*)pLayer->GetGameModePtr());
		
		pGameMode->SendLobbyUpdateMessage( state, time );

		return 0;
	}

	static S32 GetLobbyState( lua_State* pState )
	{
		lua_pushnumber( pState, C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyState );
		lua_pushnumber( pState, C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyTimer );

		return 2;
	}

	static S32 ResetLobbyState( lua_State* pState )
	{
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();

		// Only in system link or online
		if ( pLayer->GetGameModePtr()->GetID() == Game::E_GameMode_Online || pLayer->GetGameModePtr()->GetID() == Game::E_GameMode_SystemLink )
		{
			Game::C_GameModeNetwork* pGameMode = ((Game::C_GameModeNetwork*)pLayer->GetGameModePtr());
			
			// reset it based on it being ranked
			pGameMode->UpdateLobbyState( UI::E_LobbyState_Waiting, 0.0f );
		}
		return 0;
	}

	static S32 HasRaceFinished(lua_State* pState)
	{
		bool finished = true;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (!p->m_RaceConnection.IsActive())
						continue; // ignore network players who dont have a race connection, could have a party connection only. 

					switch (p->m_RaceConnection.m_PlayerState)
					{
					case RacerNet::eNetPlayerStateRacing:
					case RacerNet::eNetPlayerStateLoadingRace:
					case RacerNet::eNetPlayerStateWaitingForOthers:
						finished = false;
						break;
					}
				}
			}
		}

		lua_pushboolean(pState, finished);
		return 1;
	}

	static S32 CanShowWinner( lua_State* pState )
	{
		bool result = true;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (!p->m_RaceConnection.IsActive())
						continue; // ignore network players who dont have a race connection, could have a party connection only. 

					switch (p->m_RaceConnection.m_PlayerState)
					{
					case RacerNet::eNetPlayerStateRacing:
						result = false;
						break;
					}
				}
			}
		}

		lua_pushboolean(pState, result);
		return 1;
	}

	static S32 RaceInProgress( lua_State* pState )
	{
		bool result = false;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (!p->m_RaceConnection.IsActive())
						continue; // ignore network players who dont have a race connection, could have a party connection only. 

					switch ( p->m_RaceConnection.m_PlayerState )
					{
						case RacerNet::eNetPlayerStateLoadingRace:
						case RacerNet::eNetPlayerStateWaitingForOthers:
						case RacerNet::eNetPlayerStateRacing:
						case RacerNet::eNetPlayerStateFinished:
						case RacerNet::eNetPlayerStateViewingResults:
						case RacerNet::eNetPlayerStateViewingResultsIdle:
						case RacerNet::eNetPlayerStateLoadingUi:
							result = true;
							break;
					}
				}
			}
		}

		lua_pushboolean(pState, result);
		return 1;
	}

	static S32 LocalPlayerDisconnecting(lua_State* pState)
	{
		bool disconnecting = false;
		// check player isnt in a disconnect stage
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				// change all local player to disconnecting from race
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p && p->LocalPlayer() && p->m_RaceConnection.IsActive() && p->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateDisconnecting )
						disconnecting = true; // hang players on the race results screen until they disconnect
				}
			}
		}

		lua_pushboolean( pState, disconnecting );
		return 1;
	}


	static S32 IsPrivateGame( lua_State* pState )
	{
		bool privateGame = true;
		RacerNet::NetRaceSettings settings;
		if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( settings ) )
			privateGame = !settings.m_PublicGame;

		lua_pushboolean( pState, privateGame );
		return 1;
	}

	static S32 IsPlayerInactive( lua_State* pState)
	{
		lua_pushboolean( pState, C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_InactivePlayer );
		return 1;
	}

	static S32 CanReturnPlayersToLobby( lua_State* pState )
	{
		bool allowed = false;

		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if(pLayer && pLayer->GetGameModePtr()->IsNetwork() && pLayer->GetGameModePtr()->GetPlayMode() == Game::E_PlayMode_CustomRace)
		{
			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
			RacerNet::C_NetPlayer* hostPlayer = mgr.HostPlayer(false);

			if( hostPlayer && hostPlayer->LocalPlayer() ) //Need to be the host player
			{
				allowed = true; //Then we are allowed to use the force event timeout functionality
			}
		}

		lua_pushboolean( pState, allowed );
		return 1;
	}

	static S32 ForceEventTimeout( lua_State* pState )
	{
		//Tell everyone else to time out
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		RacerNet::C_NetRace& race = services.Race();
		race.ForceRaceTimeout();

		//Then time out local players
		((Game::C_RaceModeMultiplayer*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr())->ForceTimeOutLocalVehicles();

		return 0;
	}

	/**********************************************************************************************************************************************************************/
	/**********************************************************************************************************************************************************************/
	static const luaL_reg net_race_lib[] = 
	{
		{"Delete",									Delete},								// kill the race connections and delete onlines sessions
		{"UnrecoverableError",						UnrecoverableError},					// has an error occurred that we cant recover from.
		{"IsPlayerMigrating",						IsPlayerMigrating},						// is the local player in the process of migration.
		{"IsSessionMigrating",						IsSessionMigrating},					// is the race session in the process of migration.	
		{"IsRunning",								IsRunning},								// is the race currently running.
		{"IsOnlineMode",							IsOnlineMode},							// is the race currently running in online mode.

		{"CanSendGameInvite",						CanSendGameInvite},						// can we invite other players into this session.

		{"SetRaceParameters",						SetRaceParameters},						// race parameters passed from lua to the game mode.

		{"EnumerateLanGames",						EnumerateLanGames},						// start enumerating lan games.
		{"IsEnumeratingLanGames",					IsEnumeratingLanGames},					// are we in the process of enumerating lan games ?
		{"NumLanServersEnumerated",					NumLanServersEnumerated},				// how many lan servers did our enumeration find.
		{"CheckLanServerNetworkVersion",			CheckLanServerNetworkVersion},			// can we join a specific lan server that we have enumerated. validates network transport version of server host.			
		
		{"CreateLanServer",							CreateLanServer},						// creates a lan server based on options supplied from lua
		{"JoinLanServer",							JoinLanServer},							// start the process of joining an enumerated lan server.
		{"MonitorJoinLanServer",					MonitorJoinLanServer},					// track the progress of joining a lan session until completion.

		{"CreateOnlineServer",						CreateOnlineServer},				// starts the process of creating a matchmaking server
		{"ContinueCreateOnlineServer",				ContinueCreateOnlineServer},			// pumps the async task used to create the online matchmaking server.
		
		{"SetOnlineSessionEnumerationParameters",	SetOnlineSessionEnumerationParameters},	// store the parameters that will be passed to the matching server as a filter.
		{"StartOnlineSessionEnumeration",			StartOnlineSessionEnumeration},			// start searching for matching session(s)
		{"ContinueOnlineSessionEnumeration",		ContinueOnlineSessionEnumeration},		// continue searching for matching session(s)
		{"NumOnlineSessionsEnumerated",				NumOnlineSessionsEnumerated},			// how many matching (online) sessions did our query return.
		{"IsEnumeratedOnlineSessionInRace",			IsEnumeratedOnlineSessionInRace},		// checks an enymerated session to see if its race is in progress.

		{"JoinOnlineServer",						JoinOnlineServer},						// start the process of joining an enumerated matching (online) server.
		{"MonitorJoinOnlineServer",					MonitorJoinOnlineServer},				// track the progress of joining a matching session until completion.
		{"OnlineJoinFailedNameAvailable",			OnlineJoinFailedNameAvailable},				// if the join failed do we have the name of the player who we failed to connect to.

		{"EnterDisconnecting",						EnterDisconnecting},					// called when the local players are disconnecting from the race.
		{"EnterLobby",								EnterLobby},							// called every time the player makes it back into the lobby screen. changes local states etc.
		{"EnterRaceLoad",							EnterRaceLoad},							// called when the player transitions from the lobby to loading the race.
		{"EnterSyncAfterRaceLoad",					EnterSyncAfterRaceLoad},				// called when the player transitions from loading the race into the race.
		{"StartPlay",								StartPlay},								// called when the player starts the 321 go countdown.
		{"EnterResults",							EnterResults},							// called when the player enters the post race results screen.
		{"EnterResultsIdle",						EnterResultsIdle},						// called when the player on the results screen is in a state where that can leave (all online functionality complete)
		{"EndPlay",									EndPlay},								// called when the player leaves the race and returns to the lobby.

		{"CanStartCustomRace",						CanStartCustomRace},					// if hosting, is it possible to actually start the race.
		{"CanStartNextCustomRace",					CanStartNextCustomRace},				// if hosting, is it possible to actually start the nect event in a championship.
		{"CanStartResultsWinnner",					CanStartResultsWinnner},				// is all players who raced idle on the results screen.
		{"CanStartResultsCountdown",				CanStartResultsCountdown},				// is all players who raced idle on the results screen.
		{"CanStartVoting",							CanStartVoting},

		{"LaunchCustomRace",						LaunchCustomRace},						// Send message to all players to start the race.
	
		{"ChangeLocalPlayerVehicle",				ChangeLocalPlayerVehicle},				// attempt to change a local players vehicle
		{"ChangeLocalPlayerState",					ChangeLocalPlayerState},				// attempt to change the network state for a local player
		{"ChangeLocalPlayerLoadout",				ChangeLocalPlayerLoadout},				// attempt to change the local players loadout

		{"IsStartRaceDataValid",					IsStartRaceDataValid},					// Is the race data valid?
		{"RaceStartSynchronised",					RaceStartSynchronised},					// are the players synchronised and ready to start the race.
		{"DisconnectIdleLoadingPlayers",			DisconnectIdleLoadingPlayers},			// kill any players who claim that they are still loading the race.

		{"GetOnlineLobbyList",						GetOnlineLobbyList},
		{"GetLanLobbyList",							GetLanLobbyList},

		{"FlagInRace",								FlagInRace},							// updates the Qos data to say wea re racing or in lobby.
		{"SendNewRaceSettings",						SendNewRaceSettings},					// send new race settings to all peers - used when host changes settings in custom races
		{"GetSkipNextRaceValues",					GetSkipNextRaceValues},					// how many players currently want to skip the next race ?
		
		{"VoteToSkipNextRace",						VoteToSkipNextRace},					// local players have voted to skip the nect race in world tour.
		{"VoteOnNextRace",							VoteOnNextRace},
		{"SendVoteResults",							SendVoteResults},
		{"ReceivedVotingResults",					ReceivedVotingResults},
		{"HasVotingStarted",						HasVotingStarted},
		{"SendBeginVoteMessage",					SendBeginVoteMessage},

		{"MaxPlayersFound",							MaxPlayersFound},
		{"MaxPlayerCount",							MaxPlayerCount},
		{"EnoughPlayersFound",						EnoughPlayersFound},
		{"IsFirstRace",								IsFirstRace},
		{"IsHost",									IsHost},
		{"GetCarClass",								GetCarClass},
		{"IsTeamGame",								IsTeamGame},
		{"AreTeamsValid",							AreTeamsValid},
		{"AreTeamsBalanced",						AreTeamsBalanced},
		{"GetLocalPlayerTeam",						GetLocalPlayerTeam},

		{"GetGarageMiniClassFilters",				GetGarageMiniClassFilters},	

		{"SendLobbyState",							SendLobbyState},
		{"GetLobbyState",							GetLobbyState},
		{"ResetLobbyState",							ResetLobbyState},

		{"HasRaceFinished",							HasRaceFinished},
		{"CanShowWinner",							CanShowWinner},
		{"RaceInProgress",							RaceInProgress},
		{"LocalPlayerDisconnecting",				LocalPlayerDisconnecting},
		{"IsPrivateGame",							IsPrivateGame},

		{"IsPlayerInactive",						IsPlayerInactive},

		{"CanReturnPlayersToLobby",					CanReturnPlayersToLobby},
		{"ForceEventTimeout",						ForceEventTimeout},

		{NULL,										NULL}									// end of list
	};  

	LUALIB_API int luaopen_net_race(lua_State *pState)
	{
		luaL_openlib(pState,"NetRace", net_race_lib, 0);
		return 1;
	}

} // end namespace Game

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif
