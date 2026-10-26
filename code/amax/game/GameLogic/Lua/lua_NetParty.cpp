#include "stdafx.h"
#include <Game/Game.h>
#include <ui/Game/UILua.h>
#include <Network/Core.h>
#include <Network/Session.h>
#include <Network/SessionEnumerator.h>
#include <Network/Securekey.h>
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
#include <Network/Player/NetPlayer.h>
#include <Network/Player/NetPlayerManager.h>
#include <Network/Player/NetPlayerConnection.h>
#include <Network/Player/NetPlayerStates.h>

#include <system/debug/BuildStamp.h>
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>

// temp includes while we have to hack the party lobby gadget after creation
#include <ui/game/gadgets/ButtonController.h>
#include <Ui/UIGameDefines.h>
#include <Ui/game/Gadgets\UILobby.h>
// end temp includes.

#include <Ui/Game/Multiplayer/MultiplayerCache.h>
#include <GameLogic/GameData/MultiplayerData.h>

#if !defined ( CFG_GOLD )
Debug::Bool g_ForcePartySessionError("Network.Matching.ForcePartySessionError", false);
Debug::Bool g_ForcePartySessionCreationError("Network.Matching.ForcePartySessionCreationError", false);
#endif

namespace Game
{
	static S32 Delete_Party(lua_State* pState)
	{
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			ns.EnterLock();
			ns.Party().DisconnectAndClear();
			ns.Exitlock();
		}
		return 0;
	}

	static S32 UnrecoverableError_Party(lua_State* pState)
	{
		bool error = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
			error = ns.Party().BigError();

#if !defined ( CFG_GOLD )
		if (g_ForcePartySessionError)
		{
			error = true;
			g_ForcePartySessionError = false;
		}
#endif

		lua_pushboolean(pState, error);
		return 1;
	}

	static S32 ClearJoinRaceSessionData(lua_State* pState)
	{
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			RacerNet::NetJoinRaceFromPartyData blank;
			ns.Party().SetJoinRaceFromPartyData(blank);
		}
		return 0;
	}

	static S32 ValidateGroupToEnter(lua_State* pState)
	{
		bool ok = true;
		/*
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			if (ns.Party().IsRunning())
			{
				if (lua_isnumber(pState, 1))
				{
					const U8 num_in_party = ns.PlayerManager().Size(true);

					const U32 group_index = (U32)lua_tointeger(pState, 1);
					UI::C_MultiplayerCache& data = UI::C_MultiplayerCache::Instance();
					
					if (data.NumGroups() > (S32)group_index)
					{
						if (data.Group(group_index)->MaxPlayers() < num_in_party)
							ok = false;
					}
					else
					{
						ok = false;
						BIZ_ASSERT_FAIL("NetParty.ValidateGroupToEnter()  - group index out of range.");
					}
				}
				else
					ok = false;
			}
		}
		*/

		lua_pushboolean(pState, ok);
		return 1;
	}

	static S32 IsRunning_Party(lua_State* pState)
	{
		bool running = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
			running = ns.Party().IsRunning();
		lua_pushboolean(pState, running);
		return 1;
	}

	static S32 IsHost_Party(lua_State* pState)
	{
		bool host = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			if (ns.Party().IsRunning())
			{
				RacerNet::C_NetPlayer* p = ns.PlayerManager().HostPlayer(true);
				if (p && p->LocalPlayer())
					host = true;
			}
		}
		lua_pushboolean(pState, host);
		return 1;
	}

	static S32 IsActive_Party(lua_State* pState)
	{
		bool active = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			if (ns.Party().IsRunning())
				active = ( ns.PlayerManager().Size( true ) > 1 );
		}
		lua_pushboolean(pState, active);
		return 1;
	}

	static S32 CanSendGameInvite_Party(lua_State* pState)
	{
		bool ok = false;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			NetPlay::SessionSecurity sec;
			if (ns.Party().IsRunning())
				ok = ns.Party().CanInvite(sec);
		}

		lua_pushboolean(pState, ok);
		return 1;
	}

	static S32 CreateOnlineServer_Party(lua_State* pState)
	{
		bool started = false;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			RacerNet::C_NetParty& party = services.Party();

			if (race.IsRunning())
			{
				LOGWARN(RacerNet::g_Log, "NetParty.CreateOnlineServer() - trying to create online session after race session has been created..");
			}

			// parties can't be created after race sessions so we will delete them both before creating the party.
			race.DisconnectAndClear();
			party.DisconnectAndClear();

			if (services.Router().BandwidthEvaluationInProgress())
			{
				LOGWARN(RacerNet::g_Log, "NetParty.CreateOnlineServer() - trying to create online session before bandwidth evaluation has finished.");
			}

			U64 consoleId = 0;
			if (NetPlay::C_Core::Instance().GenerateMachineIdentifier(consoleId))
			{
				RacerNet::C_NetCreationHelper& helper = services.SessionCreationHelper();
				RacerNet::C_NetRouterBase& router = services.Router();
				if (helper.BuildLocalPlayersForOnline(router.GetNatType(), consoleId, router.EvaluatedUpstreamBandwidthKbps(), router.EvaluatedDownstreamBandwidthKbps()))
				{
					// what game settings do we need to pass to the matching server.
					// all we need to make sure is that this session is never returned in any search queries by mistake.
					RacerNet::NetMatchingServerData matchingServerCreationData;
					matchingServerCreationData.m_MatchingMode		= RacerNet::eNetMatchingModeNormal;
					matchingServerCreationData.m_PackVersion		= S32(		((C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion << 16) & 0xffff0000) 
																			|	((C_MultiplayerData::Instance().m_pConfig->m_PackVersion << 0) & 0x0000ffff));
					matchingServerCreationData.m_RankRestriction	= 0;
					matchingServerCreationData.m_MatchingRegion		= 0;
					matchingServerCreationData.m_EventId			= 0;
					matchingServerCreationData.m_CustomRace			= 0;
					matchingServerCreationData.m_OpenSlotCount		= 0;
					matchingServerCreationData.m_Searchable			= 0;
					matchingServerCreationData.m_InRace				= 0;
					matchingServerCreationData.m_Nat				= router.GetNatType();

					// build platform specific matching server parameters list.
					RacerNet::NetMatchingParameterList matchingServerParamList;
					RacerNet::BuildMatchingParameters(matchingServerCreationData, matchingServerParamList, false, false);
					if (matchingServerParamList.Size() > 0)
					{
						NetPlay::C_Session& matchSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeParty);

						NetPlay::C_Session::SlotConfig matchSlotConfig;
						matchSlotConfig.m_MaxPublic		= 0; 
						matchSlotConfig.m_PublicInUse	= 0; 
						matchSlotConfig.m_MaxPrivate	= RacerNet::NET_MAX_PARTY_MEMBERS; 
						matchSlotConfig.m_PrivateInUse	= helper.NumPlayers();

						const U32 serverFlags = NetPlay::SESSION_CREATE_ONLINE_MULTIPLAYER_STANDARD | NetPlay::SESSION_CREATE_HOST;
						started = matchSession.Create(helper.SessionManagerIndex(), serverFlags, &matchingServerParamList[0], matchingServerParamList.Size(), matchSlotConfig);
					}
				}
			}
		}

#if !defined ( CFG_GOLD )
		if (g_ForcePartySessionCreationError)
		{
			g_ForcePartySessionCreationError = false;
			started = false;
		}
#endif

		lua_pushboolean(pState, started);
		return 1;
	}

	static S32 ContinueCreateOnlineServer_Party(lua_State* pState)
	{
		bool finished = true;
		bool success = false;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			NetPlay::C_Session& matchSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeParty);
			switch (matchSession.SessionState())
			{
			case NetPlay::C_Session::eStateCreating:
				finished = false;
				break;

			case NetPlay::C_Session::eStateError:
			case NetPlay::C_Session::eStateNotActive:
				services.Party().DisconnectAndClear(); // this will internally delete the matching session
				break;

			case NetPlay::C_Session::eStateRunning:
				// online session is running so try and create net game.
				success = services.Party().CreateOnlineSession(services.SessionCreationHelper());
				break;
			}
		}

		lua_pushboolean(pState, finished);
		lua_pushboolean(pState, success);
		return 2;
	}

	static S32 JoinOnlineServer_Party(lua_State* pState)
	{
		bool started = false;

		if (lua_isnumber(pState, 1))
		{
			const U32 serverIndex = (U32)lua_tointeger(pState, 1);

			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			if (services.IsInitialised())
			{
				RacerNet::C_NetRace& race = services.Race();
				RacerNet::C_NetParty& party = services.Party();

				if (race.IsRunning())
				{
					LOGWARN(RacerNet::g_Log, "NetParty.CreateOnlineServer() - trying to create online session after race session has been created..");
				}

				if (services.Router().BandwidthEvaluationInProgress())
				{
					LOGWARN(RacerNet::g_Log, "NetParty.JoinOnlineServer() - trying to join online session before bandwidth evaluation has finished.");
				}

				// parties can't be created after race sessions so we will delete them both before creating the party.
				race.DisconnectAndClear();
				party.DisconnectAndClear();

				NetPlay::C_SessionEnumerator& enumerator = services.MatchmakingSessionEnumerator();
				RacerNet::C_NetServices::NetMatchingRemapList& remapList = services.MatchingRemapList();
				if (serverIndex >= 0 && serverIndex < (U32)remapList.Size())
				{
					NetPlay::C_SessionEnumerator::SessionQoSData qos;
					if (enumerator.GetQoSDataFromEnumerated(remapList[serverIndex], qos))
					{
						const RacerNet::NetQoSParty* pPartyQos = reinterpret_cast<RacerNet::NetQoSParty*> (qos.m_pQoSData);
						if (NULL != pPartyQos)
						{
							BIZ_ASSERT(pPartyQos->m_Type == RacerNet::NetQoSBase::QoSTypeParty);
						
							U64 consoleId = 0;
							if (NetPlay::C_Core::Instance().GenerateMachineIdentifier(consoleId))
							{
								RacerNet::C_NetCreationHelper& helper = services.SessionCreationHelper();
								RacerNet::C_NetRouterBase& router = services.Router();
								if (helper.BuildLocalPlayersForOnline(router.GetNatType(), consoleId, router.EvaluatedUpstreamBandwidthKbps(), router.EvaluatedDownstreamBandwidthKbps()))
								{
									NetPlay::SessionSecurity security;
									bdCommonAddrRef hostAddress;
									if (enumerator.GetHostInfoFromEnumerated(remapList[serverIndex], security, hostAddress))
									{
										RacerNet::NetMatchingServerData matchingServerCreationData;
										matchingServerCreationData.m_MatchingMode		= RacerNet::eNetMatchingModeNormal;
										matchingServerCreationData.m_PackVersion		= S32(		((C_MultiplayerData::Instance().m_pConfig->m_MatchingVersion << 16) & 0xffff0000) 
																								|	((C_MultiplayerData::Instance().m_pConfig->m_PackVersion << 0) & 0x0000ffff));
										matchingServerCreationData.m_RankRestriction	= 0;
										matchingServerCreationData.m_MatchingRegion		= 0;
										matchingServerCreationData.m_EventId			= 0;
										matchingServerCreationData.m_CustomRace			= 0;
										matchingServerCreationData.m_OpenSlotCount		= 0;
										matchingServerCreationData.m_Searchable			= 0;
										matchingServerCreationData.m_InRace				= 0;
										matchingServerCreationData.m_Nat				= router.GetNatType();

										// build platform specific matching server parameters list.
										RacerNet::NetMatchingParameterList matchingServerParamList;
										RacerNet::BuildMatchingParameters(matchingServerCreationData, matchingServerParamList, false, false);
										if (matchingServerParamList.Size() > 0)
										{
											NetPlay::C_Session::SlotConfig matchSlotConfig;
											matchSlotConfig.m_MaxPublic		= 0; 
											matchSlotConfig.m_PublicInUse	= 0; 
											matchSlotConfig.m_MaxPrivate	= RacerNet::NET_MAX_PARTY_MEMBERS; 
											matchSlotConfig.m_PrivateInUse	= 0;

											NetPlay::C_Session& matchSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeParty);

											const U32 serverFlags = NetPlay::SESSION_CREATE_ONLINE_MULTIPLAYER_STANDARD;
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
		}

		lua_pushboolean(pState, started);
		return 1;
	}

	static S32 MonitorJoinOnlineServer_Party(lua_State* pState)
	{
		bool complete = true;
		RacerNet::eNetJoinSessionErrors code = RacerNet::eJoinErrorInvalid;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			NetPlay::C_Session& matchSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeParty);
			RacerNet::C_NetParty& party = services.Party();

			switch (matchSession.SessionState())
			{
			case NetPlay::C_Session::eStateCreating:
				complete = false;
				break;

			case NetPlay::C_Session::eStateError:
				party.DisconnectAndClear();
				break;

			case NetPlay::C_Session::eStateNotActive:
				{
					// we should only get here if the host sent us a join failure reply, the session will have been deleted.
					RacerNet::C_NetJoinHelper& join = party.JoinHelper();
					if (join.JoinGetState() == RacerNet::eJoinStateFailed)
						code = join.JoinGetLastError(); // failed to join so get reason why.
				}
				break;

			case NetPlay::C_Session::eStateRunning:
				// online peer session is running so lets join that net session.
				if (!party.IsRunning())
				{
					// start join procedure..
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
									const RacerNet::NetQoSParty* pPartyQos = reinterpret_cast<RacerNet::NetQoSParty*> (qos.m_pQoSData);
									if (NULL != pPartyQos && hostAddress.notNull())
									{
										U8 hostTextAddr[BD_COMMON_ADDR_SERIALIZED_SIZE];	
										hostAddress->serialize(hostTextAddr);
										if (party.JoinOnlineSession(services.SessionCreationHelper(), security, hostTextAddr, pPartyQos->m_HostConsoleId, pPartyQos->m_Host))
											complete = false;
									}
								}
							}
						}
					}

					if (complete) // failed to start join procedure in some way.
						party.DisconnectAndClear();
				}
				else
				{
					// monitor join procedure.
					RacerNet::C_NetJoinHelper& join = party.JoinHelper();
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

	static S32 OnlineJoinFailedNameAvailable_Party(lua_State* pState)
	{
		bool avail = false;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			RacerNet::C_NetJoinHelper& join = party.JoinHelper();
			avail = join.IsPeerJoinFailureDataValid();
		}
		lua_pushboolean(pState, avail);
		return 1;
	}

	static S32 EnterDisconnecting_Party(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			if (party.IsInitialised() && party.IsRunning())
			{
				// change all local players to disconnecting from race
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p && p->LocalPlayer() && p->m_PartyConnection.IsActive())
						p->m_PartyConnection.m_PlayerState = RacerNet::eNetPlayerStateDisconnecting; 
				}

				RacerNet::C_NetJoinHelper& join = party.JoinHelper();
				if (join.JoinGetState() == RacerNet::eJoinStateComplete)
				{
					// inform peers of new state(s)
					party.SendLocalPlayerStatesMessage();
				}
				else
				{
					LOGWARN(RacerNet::g_Log, "NetParty.EnterDisconnecting() - called before join has finished.");
				}
			}
		}
		return 0;
	}


	static S32 EnterLobby_Party(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
			services.Party().EnterLobby();
		return 0;
	}

	static S32 EnableMatchingQoS(lua_State* pState)
	{
		if (lua_type(pState, 1) == LUA_TBOOLEAN)
		{
			bool enabled = lua_toboolean(pState, 1) ? true : false;
			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			if (services.IsInitialised())
			{
				RacerNet::C_NetParty& party = services.Party();
				if (party.IsInitialised() && party.IsRunning())
					party.EnableMatchingQoS(enabled);
			}
		}
		return 0;
	}

	static S32 CanEnterMatchmaking(lua_State* pState)
	{
		bool ok = false;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			if (party.IsInitialised() && party.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				RacerNet::C_NetPlayer* pHost = mgr.HostPlayer(true);
				if (pHost && pHost->LocalPlayer())
				{
					ok = true;
					for (U8 i=0; i<mgr.TotalSize() && ok; i++)
					{
						RacerNet::C_NetPlayer* p = mgr.Index(i);
						if (p && p->m_PartyConnection.IsActive() && p->m_PartyConnection.m_PlayerState != RacerNet::eNetPlayerStateIdle)
							ok = false;
					}
				}
			}
		}
		lua_pushboolean(pState, ok);
		return 1;
	}

	static S32 EnterMatchmaking(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			if (party.IsInitialised() && party.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				RacerNet::C_NetPlayer* pHost = mgr.HostPlayer(true);
				if (pHost && pHost->LocalPlayer())
				{
					for (U8 i=0; i<mgr.LocalSize(); i++)
					{
						RacerNet::C_NetPlayer* p = mgr.Index(i);
						BIZ_ASSERT(p->LocalPlayer());
						BIZ_ASSERT(p->m_PartyConnection.IsActive());
						if (p->LocalPlayer() && p->m_PartyConnection.IsActive())
							p->m_PartyConnection.m_PlayerState = RacerNet::eNetPlayerStatePartyBusy;
					}

					party.SendLocalPlayerStatesMessage();

					// party host should never have one of these active or it will stop him joining any race sessions he enumerates later.
					RacerNet::NetJoinRaceFromPartyData blank;
					party.SetJoinRaceFromPartyData(blank);
				}
			}
		}
		return 0;
	}

	static S32 MatchmakingSessionIdle(lua_State* pState)
	{
		bool idle = true;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			if (party.IsInitialised() && party.IsRunning())
				idle = party.IsMatchingServerIdle();
		}
		lua_pushboolean(pState, idle);
		return 1;
	}

	static S32 AllPeersIdle(lua_State* pState)
	{
		bool ok = false;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			if (party.IsInitialised() && party.IsRunning())
			{
				ok = true;
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				for (U8 i=0; i<mgr.TotalSize() && ok; i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p && p->m_PartyConnection.IsActive() && p->PrimaryPlayer() && !p->m_PartyConnection.m_Host)
					{
						if (p->m_PartyConnection.m_PlayerState != RacerNet::eNetPlayerStateIdle)
							ok = false;
					}
				}
			}
		}
		lua_pushboolean(pState, ok);
		return 1;
	}

	static S32 ChangeLocalPlayersState(lua_State* pState)
	{
		if (lua_type(pState, 1) == LUA_TNUMBER)
		{
			const U32 s = (U32)lua_tointeger(pState, 1); 
			if (s < RacerNet::eNetPlayerStateMax)
			{
				RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
				if (services.IsInitialised())
				{
					RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
					RacerNet::C_NetParty& party = services.Party();
					if (party.IsInitialised() && party.IsRunning())
					{
						bool  changed = false;
						RacerNet::eNetPlayerState state = static_cast<RacerNet::eNetPlayerState> (s);
						
						for (U8 i=0; i<mgr.LocalSize(); i++)
						{
							RacerNet::C_NetPlayer* p = mgr.Index(i);
							BIZ_ASSERT(p->LocalPlayer());
							BIZ_ASSERT(p->m_PartyConnection.IsActive());
							if (p->LocalPlayer() && p->m_PartyConnection.IsActive())
							{
								if (p->m_PartyConnection.m_PlayerState != state)
								{
									p->m_PartyConnection.m_PlayerState = state;
									changed = true;
								}
							}
						}

						if (changed)
							party.SendLocalPlayerStatesMessage();
					}
				}
			}
			else
			{
				BIZ_ASSERT_MSG(false, "NetParty.ChangeLocalPlayerState() - called with invalid state value.");
			}
		}
		else
		{
			BIZ_WARNING_MSG(false, "NetParty.ChangeLocalPlayerState() - called without argument from lua.");
		}
		return 0;
	}

	static S32 SendJoinRaceSession(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			RacerNet::C_NetRace& race = services.Race();
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
			if (party.IsInitialised() && party.IsRunning())
			{
				RacerNet::C_NetPlayer* pPartyHost = mgr.HostPlayer(true);
				RacerNet::C_NetPlayer* pRaceHost = mgr.HostPlayer(false);

				if (pPartyHost && pPartyHost->LocalPlayer())
				{
					if (race.IsRunning() && race.JoinHelper().JoinGetState() == RacerNet::eJoinStateComplete && NULL != pRaceHost)
					{
						RacerNet::NetJoinRaceFromPartyData data;
						if  (race.BuildPartyPeerJoinSessionData(data, pPartyHost->m_ConsoleId))
							party.SendPeerJoinRaceSessionMessage(data);
						else
						{
							BIZ_ASSERT_MSG(false, "NetParty.SendJoinRaceSession() - unable to build session data to send to peers.");
						}
					}
					else
					{
						BIZ_ASSERT_MSG(false, "NetParty.SendJoinRaceSession() - race session invalid !!.");
					}
				}
				else
				{
					BIZ_ASSERT_MSG(false, "NetParty.SendJoinRaceSession() - called for non host player.");
				}
			}
		}
		return 0;
	}

	static S32 SendReturnToPartyLobby(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
			if (party.IsInitialised() && party.IsRunning())
			{
				RacerNet::C_NetPlayer* pHost = mgr.HostPlayer(true);
				if (pHost && pHost->LocalPlayer())
					party.SendPeerReturnToLobbyMessage();
				else
				{
					BIZ_ASSERT_MSG(false, "NetParty.SendReturnToPartyLobby() - called for non host player.");
				}
			}
		}
		return 0;
	}

	static S32 ClearRaceInvite(lua_State* pState)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			if (party.IsRunning())
			{
				// by setting default data we clear the race invite.
				RacerNet::NetJoinRaceFromPartyData data;
				party.SetJoinRaceFromPartyData(data);
			}
		}
		return 0;
	}

	static S32 ReceivedRaceInvite(lua_State* pState)
	{
		bool valid = false;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			if (party.IsRunning())
			{
				RacerNet::NetJoinRaceFromPartyData data;
				if (party.GetJoinRaceFromPartyData(data))
					valid = data.m_Active; 
			}
		}
		lua_pushboolean(pState, valid);
		return 1;
	}

	static S32 GetRaceInviteData( lua_State* pState )
	{
		bool ranked = false;
		bool custom_race = false;
		U32 eventID = 0;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			if (party.IsRunning())
			{
				RacerNet::NetJoinRaceFromPartyData data;
				if (party.GetJoinRaceFromPartyData(data))
				{
					ranked = data.m_Ranked;
					custom_race = data.m_CustomRace;
					eventID = data.m_EventID;
				}
			}
		}
		lua_pushboolean( pState, ranked );
		lua_pushboolean( pState, custom_race );
		lua_pushnumber( pState, eventID );
		return 3;
	}

	static S32 ReceivedReturnToLobby(lua_State* pState)
	{
		bool ret = false;
		if (lua_type(pState, 1) == LUA_TBOOLEAN)
		{
			const bool clear = lua_toboolean(pState, 1) ? true : false; 

			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			if (services.IsInitialised())
			{
				RacerNet::C_NetParty& party = services.Party();
				if (party.IsRunning())
					ret = party.ReceivedReturnToLobbyRequest(clear);
			}
		}

		lua_pushboolean(pState, ret);
		return 1;
	}

	static S32 AllPlayersJoinedRace(lua_State* pState)
	{
		bool ok = true;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager(); 
			if (party.IsInitialised() && party.IsRunning())
			{
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p->m_PartyConnection.IsActive())
					{
						if (p->m_PartyConnection.m_PlayerState != RacerNet::eNetPlayerStatePartyJoinRaceSuccessful && p->m_PartyConnection.m_PlayerState != RacerNet::eNetPlayerStateDisconnecting)
							ok = false;
					}
				}
			}
		}
		lua_pushboolean(pState, ok);
		return 1;
	}

	static S32 AnyPlayersFailedToJoinRace(lua_State* pState)
	{
		bool failure = false;
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
			if (party.IsInitialised() && party.IsRunning())
			{
				RacerNet::C_NetPlayer* pHost = mgr.HostPlayer(true);
				if (pHost && pHost->LocalPlayer())
				{
					for (U8 i=0; i<mgr.TotalSize(); i++)
					{
						RacerNet::C_NetPlayer* p = mgr.Index(i);
						if (p->m_PartyConnection.IsActive())
						{
							if (p->m_PartyConnection.m_PlayerState == RacerNet::eNetPlayerStatePartyJoinRaceFailed)
								failure = true;
						}
					}
				}
			}
		}
		lua_pushboolean(pState, failure);
		return 1;
	}

	static S32 UserCreatedContentAvailable(lua_State* pState)
	{
		bool avail = false;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetParty& party = services.Party();
			RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
			if (party.IsRunning())
			{
				avail = true;
				for (U8 i=0; i<mgr.TotalSize() && avail; i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p->m_PartyConnection.IsActive())
					{
						if (!p->CanUseUserCreatedContent())
							avail = false;
					}
				}
			}
		}
		lua_pushboolean(pState, avail);
		return 1;
	}

	static S32 GetMaxPartySize( lua_State* pState)
	{
		lua_pushnumber( pState, RacerNet::NET_MAX_PARTY_MEMBERS );
		return 1;
	}

	/**********************************************************************************************************************************************************************/
	/**********************************************************************************************************************************************************************/
	static const luaL_reg net_party_lib[] =  
	{
		{"Delete",							Delete_Party},
		{"UnrecoverableError",				UnrecoverableError_Party},					// has an error occurred that we cant recover from.
		{"IsRunning",						IsRunning_Party},							// is the party currently running.
		{"IsHost",							IsHost_Party},								// is the party host a local player.
		{"IsActive",						IsActive_Party},							// Returns true if more than 1 person is in the lobby	
		{"CanSendGameInvite",				CanSendGameInvite_Party},					// can we invite other players into this session.
		{"GetMaxPartySize",					GetMaxPartySize},

		{"ValidateGroupToEnter",			ValidateGroupToEnter},						// given a reference to a group, can this party enter it. player count check.
		{"ClearJoinRaceSessionData",		ClearJoinRaceSessionData},					// clear the data structure that allows party peers to join the party hosts sspecified session

		{"CreateOnlineServer",				CreateOnlineServer_Party},					// starts the process of creating a matchmaking server
		{"ContinueCreateOnlineServer",		ContinueCreateOnlineServer_Party},			// pumps the async task used to create the online matchmaking server.

		{"JoinOnlineServer",				JoinOnlineServer_Party},					// start the process of joining an enumerated matching (online) server.
		{"MonitorJoinOnlineServer",			MonitorJoinOnlineServer_Party},				// track the progress of joining a matching session until completion.
		{"OnlineJoinFailedNameAvailable",	OnlineJoinFailedNameAvailable_Party},		// if the join failed do we have the name of the player who we could not connect to.
		
		{"EnterDisconnecting",				EnterDisconnecting_Party},					// called when players are disconnecting from the party
		{"EnterLobby",						EnterLobby_Party},							// sets initial state and enabled voice. etc
		{"EnableMatchingQoS",				EnableMatchingQoS},							// does the host need to enable matching qos so peers can accept party invites

		{"CanEnterMatchmaking",				CanEnterMatchmaking},						// can the host currently start the matching process..
		{"EnterMatchmaking",				EnterMatchmaking},							// host leaves the party lobby to enter matchmaking.
		{"MatchmakingSessionIdle",			MatchmakingSessionIdle},					// idle denotes that the player have been removed.
		{"AllPeersIdle",					AllPeersIdle},								// are all peers flagged as being idle.

		{"ChangeLocalPlayersState",			ChangeLocalPlayersState},					// changes the states of the local players and informs peers

		{"SendJoinRaceSession",				SendJoinRaceSession},						// hosts sends this out to get all party members to join the supplied race session..
		{"SendReturnToPartyLobby",			SendReturnToPartyLobby},					// hosts sends this out to get all party members back into the party lobby.

		{"ClearRaceInvite",					ClearRaceInvite},							// clear the race invite structure.
		{"ReceivedRaceInvite",				ReceivedRaceInvite},						// has the host sent us a race invite that is waiting to be joined.
		{"ReceivedReturnToLobby",			ReceivedReturnToLobby},						// has the host sent us a request to return to the party lobby.
		{"GetRaceInviteData",				GetRaceInviteData},

		{"AllPlayersJoinedRace",			AllPlayersJoinedRace},						// have all the party players joined the race session.
		{"AnyPlayersFailedToJoinRace",		AnyPlayersFailedToJoinRace},				// host can call this at any time during the join race process to check for join failures.

		{"UserCreatedContentAvailable",		UserCreatedContentAvailable},				// is user created content available to all party members.
		{NULL,							NULL}		// end of list
	};  

	LUALIB_API int luaopen_net_party(lua_State *pState)
	{
		luaL_openlib(pState,"NetParty", net_party_lib, 0);
		return 1;
	}

} // end namespace Game
