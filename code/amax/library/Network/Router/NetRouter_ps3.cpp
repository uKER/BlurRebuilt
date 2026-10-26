#include <stdafx.h>
#include <Core/core.h>
#include <Network/Core.h>
#include <Network/Network.h>
#include "../Misc/NetDebug.h"
#include "../Misc/NetDefines.h"
#include <SystemProfile/SystemProfileManager.h>
#include <SystemProfile/GameInvite.h>
#include <cell/sysmodule.h>
#include <cell/http.h>
#include <cell/ssl.h>
#include <np.h>
#include <netex/libnetctl.h>
#include "NetRouter_ps3.h"
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>
#include <bdLobby/bdTitleUtilities/bdTitleUtilities.h>
#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include <GameLogic/GameData/MultiplayerData.h>

namespace PlatformPS3 { extern char g_ps3_titleId[]; }

#if !defined ( CFG_GOLD )
Debug::Bool g_DisconnectDemonwareLobby("Network.Demonware.Lobby.ForceDisconnect", false);

#if defined ( CFG_DEBUG )
	Debug::Bool g_DisableNpConnection("Network.PS3.SkipNetworkPlatformConnection", false);
#else
	Debug::Bool g_DisableNpConnection("Network.PS3.SkipNetworkPlatformConnection", false);
#endif

#endif

// enable this to run a bandwidth scan during the connection process, it takes ages though.
//#define RUN_BANDWIDTH_SCAN_ON_CONNECTION

#if defined ( RACER_NET_OPTIMIZE_OFF )
#pragma optimize( "", off )
#endif

// max time to wait on demonware connection process before failing (in seconds)
#define MAX_DEMONWARE_CONNECTION_TIME_SEC (25)	

#define ISSUER_ID_SP_INT	1
#define ISSUER_ID_PROD_QA	2 
#define ISSUER_ID_NP		256

#define USE_DEMONWARE_CONTENT_SERVER

#if defined ( USE_DEMONWARE_CONTENT_SERVER )
	#define DW_TITLE_ID						(0x14B4) 
	#define DW_AUTH_ADDRESS					"amax-ps3.auth.mmp3.demonware.net" 
	#define DW_AUTH_PORT					(3074)
	#define DW_LSG_ADDRESS_DEV				"amax-ps3-dev.lsg.mmp3.demonware.net"
	#define DW_LSG_ADDRESS_CERT				"amax-ps3-cert.lsg.mmp3.demonware.net"
	#define DW_LSG_ADDRESS_LIVE				"amax-ps3-live.lsg.mmp3.demonware.net"
	#define DW_LSG_PORT						(3074)
	//#define DW_STREAMING_SERVICE_ADDRESS	"amax.contentserver.mmp3.demonware.net"	// not used since its the same address as the lsg
	#define DW_STREAMING_SERVICE_PORT		(3075)
#else
	#define DW_TITLE_ID						(0x1401)
	#define DW_AUTH_ADDRESS					"amax-ps3.auth.mmp3.demonware.net" 
	#define DW_AUTH_PORT					(3074)
	#define DW_LSG_ADDRESS_DEV				"amax-ps3-dev.lsg.mmp3.demonware.net"
	#define DW_LSG_ADDRESS_CERT				"amax-ps3-cert.lsg.mmp3.demonware.net"
	#define DW_LSG_ADDRESS_LIVE				"amax-ps3-live.lsg.mmp3.demonware.net"
	#define DW_LSG_PORT						(3074)
#endif

namespace SonyNetworkPlatform
{
	// defined in PlatformCallbacks_ps3.cpp
	extern const char* get_amax_network_platform_service_id();
}

namespace RacerNet
{
	// constructor
	C_NetRouterPS3::C_NetRouterPS3() :	m_Initialised(false), m_ShutDownNetStartDialog(false), m_Lsg(NULL), m_Ps3Cookie(NULL), m_pHttpPool(NULL), m_pHttpsPool(NULL),
										m_Ps3CookieSize(0), m_Ps3AuthState(ePS3AuthStateNotActive),
										m_pAuthGetHostByName(NULL), m_pAuthService(NULL), m_pLsgGetHostByName(NULL), m_NetStartDialogComplete(true), 
										m_NetStartDialogSuccessful(false), m_AuthTicketTimer(0), m_ConnectionTimer(0), m_StringVerifyActive(false)
	{
	}

	// destructor
	C_NetRouterPS3::~C_NetRouterPS3()
	{
		Shutdown();
	}

	bool C_NetRouterPS3::Initialise(const U32 sendBufferSize)
	{
		if (m_Initialised)
			return true;

		if (C_NetRouterBase::Initialise(sendBufferSize))
			m_Initialised = true;

		return m_Initialised;
	}

	void C_NetRouterPS3::Shutdown(void)
	{
		PlatformDisconnect();
		C_NetRouterBase::Shutdown();	
		m_Initialised = false;
	}

	bool C_NetRouterPS3::PlatformConnect(const bool onlineMode, char* userName, char* password)
	{
		if (!m_Initialised)
			return false;

#if !defined ( CFG_GOLD )
			if (g_DisableNpConnection)
				return false;
#endif
		// no point disconnecting and re connecting unless online mode changes.
		if (GetPlatformConnectionState() != eConnectionStateNotActive && GetPlatformConnectionState() != eConnectionStateFailed)
		{
			if (onlineMode == IsOnlineMode() &&
				(C_MultiplayerData::Instance().m_pConfig && (C_MultiplayerData::Instance().m_pConfig->m_PeerProxyEnabled == m_UseRelaySocketRouter)))
				return true; // may as well carry on the same connection attempt.
		}

		// disconnect if we are currently connected.
		PlatformDisconnect();

		// store the connection mode
		m_OnlineMode = onlineMode;

		m_LastConnectionErrorCode = eConnectionResultNone;

		LOGINFO(g_Log, "C_NetRouterPS3 - Connection started - %s", m_OnlineMode ? "Online mode" : "Offline mode");

		return StartInitialDelay();
	}

	void C_NetRouterPS3::PlatformDisconnect(void)
	{
		if (!m_Initialised)
			return;

		KillStringVerification();
		StopBandwidthEvaluation();
		EnableContentStreaming(false);

		if (GetPlatformConnectionState() == eConnectionStateNotActive)
			return;

		if (m_pAuthGetHostByName)
		{
			m_pAuthGetHostByName->quit();
			delete m_pAuthGetHostByName;
			m_pAuthGetHostByName = NULL;
		}

		if (m_pLsgGetHostByName)
		{
			m_pLsgGetHostByName->quit();
			delete m_pLsgGetHostByName;
			m_pLsgGetHostByName = NULL;
		}

		if (m_pAuthService)
			delete m_pAuthService;
		m_pAuthService = NULL;

		// stop demonware..
		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (lobbyService)
			lobbyService->disconnect();

		net->stop();

		m_OnlineMode = false;
		m_Lsg = NULL;
		m_Ps3Cookie = NULL;
		m_Ps3CookieSize = 0;
		m_Ps3AuthState = ePS3AuthStateNotActive;
		m_NetStartDialogSuccessful = false;
		m_NetStartDialogComplete = true;
		m_AuthTicketTimer = 0;
		m_ConnectionTimer = 0;

		LOGINFO(g_Log, "C_NetRouterPS3 - Connection terminated");

		m_PlatformConnectionState = eConnectionStateNotActive;

		if (m_LastConnectionErrorCode != eConnectionResultNoServiceProvider && m_LastConnectionErrorCode != eConnectionResultAgeError)
			m_LastConnectionErrorCode = eConnectionResultGeneric; 

		m_pGameInvite->m_Active = false;
	}

	void C_NetRouterPS3::Poll(const F32 frameTimeInSeconds)
	{
		if (!m_Initialised)
			return;

		C_NetRouterBase::Poll(frameTimeInSeconds);

		bool pollLobbyConnection = false;
		bool checkNpState = false;
		if (eConnectionStateInProgress == m_PlatformConnectionState)
		{
			// we now impose a maximum service connection time.
			const U64 passedTime = (Time::GetPerformanceCounter() - m_ConnectionTimer) / 1000; // convert to miliseconds
			if (passedTime >= MAX_DEMONWARE_CONNECTION_TIME_SEC*1000)
			{
				LOGWARN(g_Log, "C_NetRouterPS3::Poll() - timed out connection to demonware service");
				m_PlatformConnectionState = eConnectionStateFailed;
				PlatformDisconnect();
				return;
			}

			// do we need to unload the net start dialog ?
			UnloadNetStartDialog();

			switch (m_Ps3AuthState)
			{
			case ePS3AuthStateInitialDelay:
				ContinueInitialDelay();
				break;

			case ePS3AuthStateStartingService:
				ContinueStartService();
				break;

			case ePS3AuthStateStartingNetwork:
				checkNpState = m_OnlineMode;
				ContinueStartConnect();
				break;
					
			case ePS3AuthStateAuthDnsCheck:
				checkNpState = m_OnlineMode;
				ContinueAuthDnsLookup();
				break;

			case ePS3AuthStateRequestingTicket:
				checkNpState = m_OnlineMode;
				break;

			case ePS3AuthStateGotTicket:
				checkNpState = m_OnlineMode;
				ContinueTicketRequest();
				break;
					
			case ePS3AuthStateAuthorizingTicket:
				checkNpState = m_OnlineMode;
				ContinueCrossAuth();
				break;

			case ePS3AuthStateLsgDnsCheck:
				checkNpState = m_OnlineMode;
				ContinueLsgDnsCheck();
				break;
					
			case ePS3AuthStateConnectingToLobby:
				checkNpState = m_OnlineMode;
				ContinueLobbyConnect();
				break;

			case ePS3AuthStateEvaluatingBandwidth:
				pollLobbyConnection = true;
				checkNpState = m_OnlineMode;
				if (!BandwidthEvaluationInProgress())
				{
					m_Ps3AuthState = ePS3AuthStateNotActive;
					m_PlatformConnectionState = eConnectionStateComplete;
				}
				break;

			case ePS3AuthStateError:
				LOGWARN(g_Log, "C_NetRouterPS3::Poll() - unable to authorise np/demonware.");
				m_PlatformConnectionState = eConnectionStateFailed;
				PlatformDisconnect();
				break;

			default:
				LOGWARN(g_Log, "C_NetRouterPS3::Poll() - unknown authentication state.");
			}
		}
		else if (eConnectionStateComplete == m_PlatformConnectionState)
		{
			pollLobbyConnection = true;
			checkNpState = m_OnlineMode;
		}

		// monitor connection to lobby service.
		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (IsOnlineMode())
		{		
			bool lost = false;
			if (checkNpState)
			{
				if (SysProfile::C_Manager::Instance().ServiceConnectionState() != SysProfile::C_Manager::eServiceStateConnected)
					lost = true; // lost connection to network platform, we should disconnect from demonware..
			}

			if (pollLobbyConnection)
			{
				if(lobbyService)				
				{
					lobbyService->pump();

#if defined ( USE_DEMONWARE_CONTENT_SERVER )
					if (m_ContentStreamingEnabled)
					{
						// content streaming server polling
						bdContentStreaming* contentStreaming = lobbyService->getContentStreaming();
						if (contentStreaming)
							contentStreaming->pump();
					}
#endif
					const bdLobbyConnection::Status connectionStatus = lobbyService->getStatus();
					switch(connectionStatus)
					{			
					case bdLobbyConnection::BD_NOT_CONNECTED:
					case bdLobbyConnection::BD_DISCONNECTED:
						lost = true;
						break;

					case bdLobbyConnection::BD_DISCONNECTING:
						break;
					}
				}

#if !defined ( CFG_GOLD )
				if (g_DisconnectDemonwareLobby)
				{
					g_DisconnectDemonwareLobby = false;
					lost = true;
				}
#endif
			}

			if (lost)
			{
				// if we disconnected during the connection phase then its safe to disconnect 
				// if not then we should clean upa all networking race/ party etc before disconnecting.
				if (eConnectionStateComplete != m_PlatformConnectionState)
					PlatformDisconnect();
				else
				{
					// TODO -> how should we handle loss of connection maybe a UI message?
					LOGWARN(g_Log, "C_NetRouterPS3::Poll() - Lost connection to demonware or network platform.");

					KillStringVerification();
					StopBandwidthEvaluation();
					EnableContentStreaming(false);

					m_PlatformConnectionState = eConnectionStateFailed;
					m_LastConnectionErrorCode = eConnectionResultGeneric;
					if (lobbyService)
						lobbyService->disconnect();
					m_pGameInvite->m_Active = false;
				}
			}
		}
	}

	void C_NetRouterPS3::PlatformManagerCallback(S32 ev, S32 result, void *arg)
	{
		if (!m_Initialised)
			return;

		if (SCE_NP_MANAGER_EVENT_GOT_TICKET == ev)
		{
			if (eConnectionStateInProgress == m_PlatformConnectionState && ePS3AuthStateRequestingTicket == m_Ps3AuthState)
			{
				LOGINFO(g_Log, "C_NetRouterPS3::PlatformManagerCallback() - got auth ticket from Sony");

				bool error = true;
				if (result > 0)
				{
					// ticket request succeeded
					error = false;
					m_Ps3AuthState = ePS3AuthStateGotTicket;
				}

				if (error)
					m_Ps3AuthState = ePS3AuthStateError;
			}
		}
	}

	void C_NetRouterPS3::NetStartDialogFinished(void)
	{
		if (!m_Initialised)
			return;

		LOGINFO(g_Log, "C_NetRouterPS3::NetStartDialogFinished()");
		m_ShutDownNetStartDialog = true;
	}

	void C_NetRouterPS3::NetStartDialogUnloaded(void)
	{
		if (!m_Initialised)
			return;

		LOGINFO(g_Log, "C_NetRouterPS3::NetStartDialogUnloaded()");
		if (eConnectionStateInProgress == m_PlatformConnectionState && ePS3AuthStateStartingService == m_Ps3AuthState)
			m_NetStartDialogComplete = true;
	}

	bool C_NetRouterPS3::StartInitialDelay(void)
	{
		LOGINFO(g_Log, "C_NetRouterPS3::StartInitialDelay()");
		BIZ_ASSERT(m_Initialised);

		m_Ps3AuthState = ePS3AuthStateInitialDelay;
		m_PlatformConnectionState = eConnectionStateInProgress;

		m_ConnectionTimer = Time::GetPerformanceCounter();
		m_InitialDelay = Time::GetPerformanceCounter();

		return true;
	}

	bool C_NetRouterPS3::ContinueInitialDelay(void)
	{
		BIZ_ASSERT(m_Initialised);

		// keep resetting this
		m_ConnectionTimer = Time::GetPerformanceCounter();

		const U64 passedTime = (Time::GetPerformanceCounter() - m_InitialDelay) / 1000; // convert to miliseconds
		if (passedTime >= 1*1000)
			StartService();
	}

	bool C_NetRouterPS3::StartService(void)
	{
		BIZ_ASSERT(m_Initialised);
		m_Ps3AuthState = ePS3AuthStateStartingService;
		m_PlatformConnectionState = eConnectionStateInProgress;

		LOGINFO(g_Log, "C_NetRouterPS3::StartService()");

		m_NetStartDialogSuccessful = false;
		m_NetStartDialogComplete = false;

		// keep resetting this until np dialog has finished.
		m_ConnectionTimer = Time::GetPerformanceCounter();

		CellNetCtlNetStartDialogParam npParams;
		memset(&npParams, 0, sizeof(npParams));
		npParams.size = sizeof(npParams);
		npParams.type = m_OnlineMode ? CELL_NET_CTL_NETSTART_TYPE_NP : CELL_NET_CTL_NETSTART_TYPE_NET;

		S32 res = cellNetCtlNetStartDialogLoadAsync(&npParams);
		if (0 == res)
			return true;
		
		LOGINFO(g_Log, "C_NetRouterPS3::StartService() - failed to init Sony Net Start dialog..");

		m_PlatformConnectionState = eConnectionStateFailed;
		m_LastConnectionErrorCode = eConnectionResultNoServiceProvider;
		PlatformDisconnect();
		return false;
	}
	
	void C_NetRouterPS3::ContinueStartService(void)
	{
		// keep resetting this until np overlay has closed
		m_ConnectionTimer = Time::GetPerformanceCounter();

		if (!m_NetStartDialogComplete)
			return; // still waiting for connection dialog to complete

		if (!m_NetStartDialogSuccessful)
		{
			m_PlatformConnectionState = eConnectionStateFailed;
			m_LastConnectionErrorCode = eConnectionResultNoServiceProvider;
			PlatformDisconnect();
		}
		else
			StartConnect();
	}

	bool C_NetRouterPS3::StartConnect(void)
	{
		LOGINFO(g_Log, "C_NetRouterPS3::StartConnect()");

		BIZ_ASSERT(m_Initialised);
		m_Ps3AuthState = ePS3AuthStateStartingNetwork;

		bdNetImpl *const net = bdNet::getInstance();
		bdNetStartParams params;
		params.m_onlineGame = m_OnlineMode;

		m_UseRelaySocketRouter = false;

		// if online mode then we want to use peer proxy to bypass NAT issues.
		if (params.m_onlineGame)
		{
			if (C_MultiplayerData::Instance().m_pConfig && C_MultiplayerData::Instance().m_pConfig->m_PeerProxyEnabled)
			{
				params.m_useRelaySocketRouter = true;
				m_UseRelaySocketRouter = true;
				LOGINFO(g_Log, "C_NetRouterPS3 : Peer routing enabled.");
			}
		}

		// force an update for latest network state.
		NetPlay::C_Core::Instance().Poll(true);

		CellNetCtlInfo ctlInfo;
		if (!NetPlay::C_Core::Instance().CableConnected() || cellNetCtlGetInfo(CELL_NET_CTL_INFO_IP_ADDRESS, &ctlInfo) != 0)
		{
			m_LastConnectionErrorCode = eConnectionResultNoNetwork;
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
			return false;
		}

		// store the address we will use to bind the router to.
		params.m_localAddresses.pushBack(bdInetAddr(ctlInfo.ip_address));
		
		if (m_OnlineMode)
		{
			if (!CheckOnlineAgeRestriction())
			{
				m_LastConnectionErrorCode = eConnectionResultAgeError;
				m_PlatformConnectionState = eConnectionStateFailed;
				PlatformDisconnect();
				return false;
			}

			cellNetCtlGetInfo(CELL_NET_CTL_INFO_DEFAULT_ROUTE, &ctlInfo);
			params.m_UPnPConfig.m_gatewayAddr = bdInetAddr(ctlInfo.default_route); 

			m_PlatformConnectionState = eConnectionStateInProgress;
			if (net->start(params))
			{
				m_Ps3AuthState = ePS3AuthStateStartingNetwork;
				return true;
			}
			else
			{
				LOGWARN(g_Log, "C_NetRouterPS3::StartConnect() - unable to start network in online mode.");
				m_PlatformConnectionState = eConnectionStateFailed;
				PlatformDisconnect();
				return false;
			}
		}
		else
		{
			if (net->start(params))
			{
				// wait for bdNetImpl::start to complete, will be straight away in offline mode.
				while (net->getStatus() == bdNetImpl::BD_NET_PENDING)
					net->pump();

				if (net->getStatus() == bdNetImpl::BD_NET_DONE)
				{
					// if offline mode then connection phase is now complete, since we don't actuallty connect to any online services
					m_Ps3AuthState = ePS3AuthStateNotActive;
					m_PlatformConnectionState = eConnectionStateComplete;
					return true;
				}
				else
				{
					LOGWARN(g_Log, "C_NetRouterPS3::StartConnect() - unable to start network in offline mode.");
					m_PlatformConnectionState = eConnectionStateFailed;
					PlatformDisconnect();
				}
			}
			else
			{
				LOGWARN(g_Log, "C_NetRouterPS3::StartConnect() - unable to start network in offline mode.");
				m_PlatformConnectionState = eConnectionStateFailed;
				PlatformDisconnect();
			}
		}

		return false;
	}

	void C_NetRouterPS3::ContinueStartConnect(void)
	{
		BIZ_ASSERT(m_Initialised);
		BIZ_ASSERT(m_Ps3AuthState == ePS3AuthStateStartingNetwork);

		bdNetImpl *const net = bdNet::getInstance();
		net->pump();
		if (net->getStatus() == bdNetImpl::BD_NET_PENDING)
			return;

		const bool net_ok = (net->getStatus() == bdNetImpl::BD_NET_DONE) ? true : false;

		if (!net_ok)
		{
			LOGWARN(g_Log, "C_NetRouterPS3::ContinueStartConnect() - unable to start network in online mode.");
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
		}
		else
		{
			BIZ_ASSERT(m_pAuthGetHostByName == NULL);
			const TaskStatus t = StartGetHostByName(DW_AUTH_ADDRESS, &m_pAuthGetHostByName, m_AuthAddr);
			if (FAILED == t) 
			{
				LOGWARN(g_Log, "C_NetRouterPS3::ContinueStartConnect() - unable to start dns lookup for auth server.");
				m_PlatformConnectionState = eConnectionStateFailed;
				PlatformDisconnect();
			}
			else if (DONE == t)
			{
				// dns lookup already done so skip this bit.
				StartTicketRequest();
			}
			else
			{
				// pending dns request..
				m_Ps3AuthState = ePS3AuthStateAuthDnsCheck;
			}
		}
	}

	void C_NetRouterPS3::ContinueAuthDnsLookup(void)
	{
		// m_pAuthGetHostByName will be released and set to NULL inside PumpGetHostByName() when it finishes.
		const TaskStatus t = PumpGetHostByName(&m_pAuthGetHostByName, m_AuthAddr);
		if (PENDING == t)
			return;
		if (FAILED == t)
		{
			LOGWARN(g_Log, "C_NetRouterPS3::ContinueAuthDnsLookup() - unable to continue dns lookup for auth server.");
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
			return;
		}

		BIZ_ASSERT(t == DONE);
		StartTicketRequest();
	}

	void C_NetRouterPS3::StartTicketRequest(void)
	{
		LOGINFO(g_Log, "C_NetRouterPS3::StartTicketRequest()");

		BIZ_ASSERT(m_pAuthService == NULL);

		// create auth service now..
		const bdAddr authAddr(m_AuthAddr, DW_AUTH_PORT);
		m_pAuthService = new bdAuthService(DW_TITLE_ID, authAddr);

		if (m_pAuthService)
		{
			m_Ps3Cookie = (char*)m_pAuthService->getPS3Cookie(m_Ps3CookieSize);

			SceNpId npId;
			S32 ret = sceNpManagerGetNpId(&npId);
			if (0 == ret)
			{
				SceNpTicketVersion ver;
				ver.major = 2;
				ver.minor = 1;
			
				ret = sceNpManagerRequestTicket2( &npId, &ver, SonyNetworkPlatform::get_amax_network_platform_service_id(), m_Ps3Cookie, m_Ps3CookieSize, NULL, 0);
				if (0 == ret)
					m_Ps3AuthState = ePS3AuthStateRequestingTicket;
				else
				{
					LOGWARN(g_Log, "C_NetRouterPS3::StartTicketRequest() - failed to request ticket from Network Platform..");
					m_PlatformConnectionState = eConnectionStateFailed;
					PlatformDisconnect();
					return;
				}
			}
			else
			{
				LOGWARN(g_Log, "C_NetRouterPS3::StartTicketRequest() - looks like local player is not connected to Network Platform..");
				m_PlatformConnectionState = eConnectionStateFailed;
				PlatformDisconnect();
				return;
			}
		}
		else
		{
			LOGWARN(g_Log, "C_NetRouterPS3::StartTicketRequest() - unable to create auth service.");
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
			return;
		}
	}

	void C_NetRouterPS3::ContinueTicketRequest(void)
	{
		// successfully got ticket from NP.
		bool error = false;
		size_t ticketSize = SCE_NP_TICKET_MAX_SIZE;
		ChooseLsgAddressFromTicket(); // choose what lsg address to connect to based on ticket data supplied by NP.
		if (NULL != m_Lsg)
		{
			U8* pTicket = new U8[ticketSize];
			if (pTicket)
			{
				memset(pTicket, 0, ticketSize);

				// request ticket data ab=nd get actual ticket size.
				S32 res = sceNpManagerGetTicket(pTicket, &ticketSize);

				if (0 == res)
				{
					SceNpTicketParam tp;
					res = sceNpManagerGetTicketParam(SCE_NP_TICKET_PARAM_SUBJECT_REGION, &tp);
					if (0 == res)
					{
						SceNpCountryCode cc;
						SCE_NP_SUBJECT_REGION_GET_COUNTRY_CODE(tp.data, &cc);
						UpdateMatchingRegion(cc.data);
					}
					else
					{
						LOGWARN(g_Log, "sceNpManagerGetTicketParam() - failed. unable to set matching region");
						SetMatchingRegion(eMatchingRegionNorthAmerica);
					}

					const bdPS3AuthInfoRef ps3AuthInfo = new bdPS3AuthInfo();

					if (ps3AuthInfo.notNull())
					{
						if (m_pAuthService->authorizePS3Ticket(pTicket, ticketSize, ps3AuthInfo))
						{
							m_AuthTicketTimer = Time::GetPerformanceCounter();
							m_Ps3AuthState = ePS3AuthStateAuthorizingTicket;
						}
						else
							error = true;
					}
					else
						error = true;
				}
				else
					error = true;

				delete [] pTicket;
			}
			else
				error = true;
		}
		else
			error = true;

		if (error)
		{
			LOGWARN(g_Log, "C_NetRouterPS3::ContinueTicketRequest() - unable to get process from OS.");
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
		}
	}

	void C_NetRouterPS3::ContinueCrossAuth(void)
	{
		const bdAuthService::bdStatus status = m_pAuthService->getStatus();

		if (status == bdAuthService::BD_READY)
		{
			const bdLobbyErrorCode lobbyErrorCode = m_pAuthService->getErrorCode();
			if(BD_AUTH_NO_ERROR == lobbyErrorCode)
			{
				// authenticated ok, now we need lookup the correct lobby dns address..
				BIZ_ASSERT(m_pLsgGetHostByName == NULL);
				const TaskStatus t = StartGetHostByName(m_Lsg, &m_pLsgGetHostByName, m_LsgAddr);
				if (FAILED == t) 
				{
					LOGWARN(g_Log, "C_NetRouterPS3::ContinueCrossAuth() - unable to start dns lookup for LSG server.");
					m_PlatformConnectionState = eConnectionStateFailed;
					PlatformDisconnect();
				}
				else if (DONE == t)
				{
					// dns lookup already done so skip this bit.
					StartLobbyConnect();
				}
				else
				{
					// pending dns request..
					m_Ps3AuthState = ePS3AuthStateLsgDnsCheck;
				}
			}
			else
			{
				LOGWARN(g_Log, "C_NetRouterPS3::ContinueCrossAuth() - failed to authenticate NP ticket with demonware.");
				m_PlatformConnectionState = eConnectionStateFailed;
				PlatformDisconnect();
			}
		}
		else
		{
			const U64 passedTime = (Time::GetPerformanceCounter() - m_AuthTicketTimer) / 1000; // convert to miliseconds
			if (passedTime >= 30*1000)
			{
				LOGWARN(g_Log, "C_NetRouterPS3::ContinueCrossAuth() - failed to authenticate NP ticket with demonware. timed out");
				m_PlatformConnectionState = eConnectionStateFailed;
				PlatformDisconnect();
			}
		}
	}

	void C_NetRouterPS3::ContinueLsgDnsCheck(void)
	{
		// m_pLsgGetHostByName will be released and set to NULL inside PumpGetHostByName() when it finishes.
		const TaskStatus t = PumpGetHostByName(&m_pLsgGetHostByName, m_LsgAddr);
		if (PENDING == t)
			return;
		if (FAILED == t)
		{
			LOGWARN(g_Log, "C_NetRouterPS3::ContinueLsgDnsCheck() - unable to continue dns lookup for LSG server.");
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
			return;
		}

		BIZ_ASSERT(t == DONE);
		StartLobbyConnect();
	}

	void C_NetRouterPS3::StartLobbyConnect(void)
	{
		LOGINFO(g_Log, "C_NetRouterPS3::StartLobbyConnect()");

		BIZ_ASSERT(m_pAuthService != NULL);

		bdAddr LSGAddr(m_LsgAddr, DW_LSG_PORT);

#if defined ( USE_DEMONWARE_CONTENT_SERVER )
		// content streaming address is the same as the lsg with a different port.
		m_ContentStreamingAddress = bdAddr(m_LsgAddr, DW_STREAMING_SERVICE_PORT);
#endif

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (lobbyService && lobbyService->connect(LSGAddr, m_pAuthService->getAuthInfo()))
		{
			// can delete auth service now as its not needed anymore..
			delete m_pAuthService;
			m_pAuthService = NULL;

			m_Ps3AuthState = ePS3AuthStateConnectingToLobby;
		}
		else
		{
			LOGWARN(g_Log, "C_NetRouterPS3::StartLobbyConnect() - unable to connect to demonware lobby service.");
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
		}
	}

	void C_NetRouterPS3::ContinueLobbyConnect(void)
	{
		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (!lobbyService)
		{
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
			return;
		}
		lobbyService->pump();
		const bdLobbyConnection::Status connectionStatus = lobbyService->getStatus();
		switch(connectionStatus)
		{			
		case bdLobbyConnection::BD_NOT_CONNECTED:
		case bdLobbyConnection::BD_DISCONNECTING:
		case bdLobbyConnection::BD_DISCONNECTED:
			LOGWARN(g_Log, "C_NetRouterPS3::ContinueLobbyConnect() - unable to connect to demonware lobby service.");
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
			break;

		case bdLobbyConnection::BD_CONNECTING:
			break;
  
		case bdLobbyConnection::BD_CONNECTED:
			LOGINFO(g_Log, "C_NetRouterPS3 - Connected to Demonware services.");
#if defined ( RUN_BANDWIDTH_SCAN_ON_CONNECTION )
			StartBandwidthEvaluation();
#endif
			m_Ps3AuthState = ePS3AuthStateEvaluatingBandwidth;

#if defined ( USE_DEMONWARE_CONTENT_SERVER )
			// now that we are connected we should set the address of the content streaming server.
			// not used in latest version of demonware code. lobbyService->setStreamingServiceAddress(m_ContentStreamingAddress);
#endif
			break;			
		}
	}

	void C_NetRouterPS3::ChooseLsgAddressFromTicket(void)
	{
		BIZ_ASSERT(m_Initialised);

		m_Lsg = NULL;
		union SceNpTicketParam outParam;

		bool ret = sceNpManagerGetTicketParam(SCE_NP_TICKET_PARAM_ISSUER_ID, &outParam);
		if (0 == ret)
		{
			switch (outParam.u32)
			{
			case ISSUER_ID_SP_INT:
				m_Lsg = DW_LSG_ADDRESS_DEV;
				break;
			case ISSUER_ID_PROD_QA:
				m_Lsg = DW_LSG_ADDRESS_CERT;
				break;
			case ISSUER_ID_NP:
				m_Lsg = DW_LSG_ADDRESS_LIVE;
				break;
			default:
				m_Lsg = DW_LSG_ADDRESS_CERT;
				break;
			}
		}	
	}

	bool C_NetRouterPS3::EnableContentStreaming(const bool enabled)
	{
		if (m_Initialised)
		{
			if (enabled)
			{
				if (!m_ContentStreamingEnabled)
				{
					bool httpOk = false;
					bool httpsOk = false;
					bool cellHttpInitOk = false;

					if (cellSysmoduleLoadModule(CELL_SYSMODULE_HTTP) == CELL_OK)
					{
						httpOk = true;
						if (cellSysmoduleLoadModule(CELL_SYSMODULE_HTTPS) == CELL_OK)
						{
							httpsOk = true;

							const U32 HTTP_POOL_SIZE = 1024*64;
							const U32 SSL_POOL_SIZE = 1024*256;
								
							BIZ_ASSERT(NULL == m_pHttpPool);
							BIZ_ASSERT(NULL == m_pHttpsPool);
							
							m_pHttpPool = new U8[HTTP_POOL_SIZE];
							m_pHttpsPool = new U8[SSL_POOL_SIZE];

							if (m_pHttpPool && m_pHttpsPool)
							{
								if (cellHttpInit(m_pHttpPool, HTTP_POOL_SIZE) == CELL_OK)
								{
									cellHttpInitOk = true;
									if (cellSslInit(m_pHttpsPool, SSL_POOL_SIZE) == CELL_OK)
										m_ContentStreamingEnabled = true;
								}
							}
						}
					}

					if (!m_ContentStreamingEnabled)
					{
						bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
						if (lobbyService)
							lobbyService->deleteContentStreaming();

						LOGWARN(g_Log, "C_NetRouterPS3::EnableContentStreaming() - unable enable to initialise ps3 HTTP.");

						// https wont be running if we reach this error condition since its the last to be initialised.
						if (cellHttpInitOk)
							cellHttpEnd();

						if (m_pHttpsPool)
							delete [] m_pHttpsPool;
						m_pHttpsPool = NULL;

						if (m_pHttpPool)
							delete [] m_pHttpPool;
						m_pHttpPool = NULL;

						if (httpsOk)
							cellSysmoduleUnloadModule(CELL_SYSMODULE_HTTPS);
						if (httpOk)
							cellSysmoduleUnloadModule(CELL_SYSMODULE_HTTP);
					}
				}

				return m_ContentStreamingEnabled;
			}
			else
			{
				if (m_ContentStreamingEnabled)
				{
					bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();

					if (lobbyService)
					{
						bdContentStreaming* cs = lobbyService->getContentStreaming();
						if (cs)
							cs->abortHTTPOperation();

						lobbyService->deleteContentStreaming();
					}

					cellSslEnd();
					cellHttpEnd();

					if (m_pHttpsPool)
						delete [] m_pHttpsPool;
					m_pHttpsPool = NULL;

					if (m_pHttpPool)
						delete [] m_pHttpPool;
					m_pHttpPool = NULL;

					cellSysmoduleUnloadModule(CELL_SYSMODULE_HTTPS);
					cellSysmoduleUnloadModule(CELL_SYSMODULE_HTTP);
					
					m_ContentStreamingEnabled = false;
				}
				return true;
			}
		}

		return false;
	}

	bool C_NetRouterPS3::VerifyString(const char* pString)
	{
		if (!m_Initialised)
			return false;

		KillStringVerification();

		if (NULL == pString)
			return false;

		const U32 len = strlen(pString) + 1; // account for termination.
		if (1 == len)
			return false; // no string supplied.

		if (m_PlatformConnectionState != eConnectionStateComplete || !m_OnlineMode)
			return false;

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService)
			return false;

		const bdLobbyConnection::Status connectionStatus = lobbyService->getStatus();
		if (bdLobbyConnection::BD_CONNECTED != connectionStatus)
			return false;

		m_StringVerifyTask = lobbyService->getTitleUtilities()->verifyString(pString, len, &m_StringVerifyResult);
		m_StringVerifyActive = NetPlay::DemonwareRemoteTask::IsBusy(m_StringVerifyTask);
		
		return m_StringVerifyActive;
	}

	bool C_NetRouterPS3::PollVerifyString(bool& validString)
	{
		validString = false;
		if (m_StringVerifyActive)
		{
			switch (NetPlay::DemonwareRemoteTask::CheckState(m_StringVerifyTask))
			{
			case NetPlay::DemonwareRemoteTask::ePending:
				return false;
				break;

			case NetPlay::DemonwareRemoteTask::eDone:
				validString = m_StringVerifyResult.m_verified;
				break;

			default:
				break;
			}
			
			KillStringVerification();
		}

		return true;
	}

	void C_NetRouterPS3::KillStringVerification(void)
	{
		NetPlay::DemonwareRemoteTask::Close(m_StringVerifyTask);
		m_StringVerifyActive = false;
	}

	void C_NetRouterPS3::UnloadNetStartDialog(void)
	{
		if (!m_Initialised)
			return;

		if (!m_ShutDownNetStartDialog)
			return;

		LOGINFO(g_Log, "C_NetRouterPS3::UnloadNetStartDialog()");

		m_ShutDownNetStartDialog = false;

		CellNetCtlNetStartDialogResult net_start_gui_result;
		memset(&net_start_gui_result, 0, sizeof(net_start_gui_result));
		net_start_gui_result.size = sizeof(net_start_gui_result);

		S32 ret = cellNetCtlNetStartDialogUnloadAsync(&net_start_gui_result);
		if (ret < 0)
		{
			LOGERROR(g_Log, "cellNetCtlNetStartDialogUnloadAsync() - failed");
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
			return;
		}

		switch (net_start_gui_result.result)
		{
		case 0: // net start dialog finished successfully.
			m_NetStartDialogSuccessful = true;
			break;

		default: // net start gui failed or canceled.
			m_NetStartDialogSuccessful = false;
			break;
		}
	}

	eNatType C_NetRouterPS3::GetNatType(void)
	{
		if (!m_Initialised)
			return eNatOpen;

		eNatType t = eNatOpen;
		if (eConnectionStateComplete == m_PlatformConnectionState && m_OnlineMode)
		{
			bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
			if (lobbyService)
			{
				if (bdLobbyConnection::BD_CONNECTED == lobbyService->getStatus())
				{
					bdNetImpl *const net = bdNet::getInstance();
					bdCommonAddrRef addr = net->getLocalCommonAddr();

					if (addr.notNull())
					{
						switch (addr->getNATType())
						{
						case BD_NAT_OPEN:
							t = eNatOpen;
							break;
						case BD_NAT_MODERATE:
							t = eNatModerate;
							break;
						case BD_NAT_STRICT:
							t = eNatStrict;
							break;
						default:
							LOGWARN(g_Log, "C_NetRouterPS3::GetNatType() - NAT type not handled correctly.");
							break;
						}
					}
				}
			}
		}

		return t;
	}

	bool C_NetRouterPS3::CheckOnlineAgeRestriction(void)
	{
		S32 age = 0;
		S32 isRestricted = 0;
		S32 ret = sceNpManagerGetContentRatingFlag(&isRestricted, &age);
		if (0 == ret) 
		{
			const S32 MIN_AGE_BLUS = 10;
			const S32 MIN_AGE_BLES = 12;
			const S32 MON_AGE_BLJM = 10;
			
			S32 minAge = MIN_AGE_BLUS;

			if (memcmp(PlatformPS3::g_ps3_titleId,"ABCD", 4) == 0)
			{
				// Default for testing only !
				minAge = MIN_AGE_BLUS; 
			} 
			else if (memcmp(PlatformPS3::g_ps3_titleId,"BLES", 4) == 0)
			{
				// Europe Product Code/Region;
				minAge = MIN_AGE_BLES;
			} 
			else if (memcmp(PlatformPS3::g_ps3_titleId,"BLUS", 4) == 0)
			{
				// NorthAmerica Product Code/Region;
				minAge = MIN_AGE_BLUS;
			} 
			else if (memcmp(PlatformPS3::g_ps3_titleId,"BLJM", 4) == 0)
			{
				LOGWARN(g_Log, "C_NetRouterPS3::CheckOnlineAgeRestriction() - Age Not Specified for Japan Region. assuming BLUS");
				minAge = MON_AGE_BLJM;
			}
			else
			{
				// Other/Unknown Region;
				LOGWARN(g_Log, "C_NetRouterPS3::CheckOnlineAgeRestriction() - unknown age region. assuming BLUS");
				minAge = MIN_AGE_BLUS;
			}

			if (isRestricted && age < minAge) 
				return false;
			return true;
		}

		LOGINFO(g_Log, "sceNpManagerGetContentRatingFlag() - failed.");
		return false;
	}

	void C_NetRouterPS3::UpdateMatchingRegion(const char* code)
	{
		eMatchingRegion region = eMatchingRegionUnknown;

		if (!strcmp(code, "ca") || !strcmp(code, "us"))
		{
			region = eMatchingRegionNorthAmerica;
		}
		else if (!strcmp(code, "mx") || !strcmp(code, "cl") || !strcmp(code, "pe") || !strcmp(code, "ar") || !strcmp(code, "co") || !strcmp(code, "br"))
		{
			region = eMatchingRegionSouthAmerica;
		}
		else if (!strcmp(code, "au") || !strcmp(code, "nz"))
		{
			region = eMatchingRegionAunz;
		}
		else if (	!strcmp(code, "jp") || !strcmp(code, "tw") || !strcmp(code, "hk") || !strcmp(code, "ae") || !strcmp(code, "za") || !strcmp(code, "sg") || 
					!strcmp(code, "kr") || !strcmp(code, "sa") || !strcmp(code, "bh") || !strcmp(code, "kw") || !strcmp(code, "lb") || !strcmp(code, "om") ||
					!strcmp(code, "qa") || !strcmp(code, "il") || !strcmp(code, "in") || !strcmp(code, "my"))
		{
			region = eMatchingRegionAsia;
		}
		else
			region = eMatchingRegionEurope;

		SetMatchingRegion(region);
	}
} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
