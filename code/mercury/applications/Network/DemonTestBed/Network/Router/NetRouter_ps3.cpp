#include <stdafx.h>
#include <Core/core.h>
#include <Network/Core.h>
#include <Network/Network.h>
#include "../Misc/NetDebug.h"
#include <SystemProfile/SystemProfileManager.h>
#include <np.h>
#include "NetRouter_ps3.h"
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyService.h>
#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include <bdconnection/bdConnection.h>

#define ISSUER_ID_SP_INT	1
#define ISSUER_ID_PROD_QA	2 
#define ISSUER_ID_NP		256

// TODO -> set correct data for demonware authentication.
#define DW_AUTH_ADDRESS		"auth.eval.v1.80.mmp3.demonware.net" 
#define DW_AUTH_PORT		(3074)

#define DW_LSG_ADDRESS_DEV	"sandbox190-lsg.mmp3.demonware.net"
#define DW_LSG_ADDRESS_CERT	"sandbox190-lsg.mmp3.demonware.net"
#define DW_LSG_ADDRESS_LIVE	"sandbox190-lsg.mmp3.demonware.net"
#define DW_LSG_PORT			(3074)

#define NP_SERVICE_ID		"set_me_please" // this needs setting from a pre-preocessor define since it changes per region (scee, scea etc..)
#define DW_TITLE_ID			(0x13CB)	

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	// constructor
	C_NetRouterPS3::C_NetRouterPS3() :	m_Lsg(NULL), m_Ps3Cookie(NULL), m_Ps3CookieSize(0), m_Ps3AuthState(ePS3AuthStateNotActive),
										m_pAuthGetHostByName(NULL), m_pAuthService(NULL), m_pLsgGetHostByName(NULL)
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

		// no point disconnecting and re connecting unless online mode changes.
		if (GetPlatformConnectionState() != eConnectionStateNotActive && GetPlatformConnectionState() != eConnectionStateFailed)
		{
			if (onlineMode == IsOnlineMode())
				return true; // may as well carry on the same connection attempt.
		}

		// disconnect if we are currently connected .
		PlatformDisconnect();

		// are we currently connected to NP ?
		if (onlineMode)
		{
			if (SysProfile::C_Manager::Instance().ServiceConnectionState() != SysProfile::C_Manager::eServiceStateConnected)
			{
				LOGWARN(g_Log, "C_NetRouterPS3::PlatformConnect() - unable to connect to demonware without NP connection first.");
				m_LastConnectionErrorCode = eConnectionResultNoServiceProvider;
				return false;
			}
		}

		// store the connection mode
		m_OnlineMode = onlineMode;

		m_LastConnectionErrorCode = eConnectionResultNone;

		return StartConnect();
	}

	void C_NetRouterPS3::PlatformDisconnect(void)
	{
		if (!m_Initialised)
			return;

		StopBandwidthEvaluation();

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
		bdLobbyService *const lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (lobbyService)
			lobbyService->disconnect();

		net->stop();

		m_OnlineMode = false;
		m_Lsg = NULL;
		m_Ps3Cookie = NULL;
		m_Ps3CookieSize = 0;
		m_Ps3AuthState = ePS3AuthStateNotActive;

		if (m_PlatformConnectionState != eConnectionStateFailed)
			m_PlatformConnectionState = eConnectionStateNotActive;
		else
			m_LastConnectionErrorCode = eConnectionResultGeneric; // all connection errors are flagged as generic on ps3.
	}

	void C_NetRouterPS3::Poll(void)
	{
		if (!m_Initialised)
			return;

		C_NetRouterBase::Poll();

		if (eConnectionStateInProgress == m_PlatformConnectionState)
		{
			switch (m_Ps3AuthState)
			{
			case ePS3AuthStateStartingNetwork:
				ContinueNetworkStart();
				break;
					
			case ePS3AuthStateAuthDnsCheck:
				ContinueAuthDnsLookup();
				break;

			case ePS3AuthStateGotTicket:
				ContinueTicketRequest();
				break;
					
			case ePS3AuthStateAuthorizingTicket:
				ContinueCrossAuth();
				break;

			case ePS3AuthStateLsgDnsCheck:
				ContinueLsgDnsCheck();
				break;
					
			case ePS3AuthStateConnectingToLobby:
				ContinueLobbyConnect();
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
			// monitor connection to lobby service.
			if (IsOnlineMode())
			{
				bdLobbyService *const lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
				if(lobbyService)
				{
					const bdLobbyConnection::Status connectionStatus = lobbyService->getStatus();
					switch(connectionStatus)
					{			
					case bdConnection::BD_NOT_CONNECTED:
					case bdConnection::BD_DISCONNECTED:
						// TOOD -> how should we handle loss of connection maybe a UI message?
						LOGWARN(g_Log, "C_NetRouterPS3::Poll() - Lost connection to demonware lobby service");
						m_PlatformConnectionState = eConnectionStateFailed;
						PlatformDisconnect();
						break;

					case bdConnection::BD_DISCONNECTING:
						break;
					}
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

	bool C_NetRouterPS3::StartConnect(void)
	{
		assert(m_Initialised);
		m_Ps3AuthState = ePS3AuthStateStartingNetwork;

		bdNetImpl *const net = bdNet::getInstance();
		bdNetStartParams params;
		params.m_onlineGame = m_OnlineMode;

		if (m_OnlineMode)
		{
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

		return false;
	}

	void C_NetRouterPS3::ContinueNetworkStart(void)
	{
		assert(m_Initialised);
		assert(m_Ps3AuthState == ePS3AuthStateStartingNetwork);

		bdNetImpl *const net = bdNet::getInstance();
		net->pump();
		if (net->getStatus() == bdNetImpl::BD_NET_PENDING)
			return;

		const bool net_ok = (net->getStatus() == bdNetImpl::BD_NET_DONE);

		if (!net_ok)
		{
			LOGWARN(g_Log, "C_NetRouterPS3::ContinueNetworkStart() - unable to start network in online mode.");
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
		}
		else
		{
			assert(m_pAuthGetHostByName);
			const TaskStatus t = StartGetHostByName(DW_AUTH_ADDRESS, &m_pAuthGetHostByName, m_AuthAddr);
			if (FAILED == t) 
			{
				LOGWARN(g_Log, "C_NetRouterPS3::ContinueNetworkStart() - unable to start dns lookup for auth server.");
				m_PlatformConnectionState = eConnectionStateFailed;
				PlatformDisconnect();
			}
			else if (DONE == t)
			{
				// dna lookup already done so skip this bit.
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

		assert(t == DONE);
		StartTicketRequest();
	}

	void C_NetRouterPS3::StartTicketRequest(void)
	{
		assert(m_pAuthService);

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
				ret = sceNpManagerRequestTicket2( &npId, &ver, NP_SERVICE_ID, m_Ps3Cookie, m_Ps3CookieSize, NULL, 0);
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
				LOGWARN(g_Log, "C_NetRouterPS3::StartTicketRequest() - looks like local plsyer is not connected to Network Platform..");
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
		size_t ticketSize = 0;
		S32 ret = sceNpManagerGetTicket(NULL, &ticketSize);
		if(ret == 0)
		{
			ChooseLsgAddressFromTicket(); // choose wat lsg address to connect to based on ticket data supplied by NP.
			if (NULL != m_Lsg)
			{
				U8* pTicket = new U8[ticketSize];
				if (pTicket)
				{
					const bdPS3AuthInfoRef ps3AuthInfo = new bdPS3AuthInfo();
					if (m_pAuthService->authorizePS3Ticket(pTicket, ticketSize, ps3AuthInfo))
						m_Ps3AuthState = ePS3AuthStateAuthorizingTicket;
					else
						error = true;

					delete [] pTicket;
				}
				else
					error = true;
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
		if(status == bdAuthService::BD_READY)
		{
			const bdLobbyErrorCode lobbyErrorCode = m_pAuthService->getErrorCode();
			if(BD_AUTH_NO_ERROR == lobbyErrorCode)
			{
				// authenticated ok, now we need lookup the correct lobby dns address..
				assert(m_pLsgGetHostByName);
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
	}

	void C_NetRouterPS3::ContinueLsgDnsCheck(void)
	{
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

		assert(t == DONE);
		StartLobbyConnect();
	}

	void C_NetRouterPS3::StartLobbyConnect(void)
	{
		assert(m_pAuthService);

		bdAddr LSGAddr(m_LsgAddr, DW_LSG_PORT);
		bdLobbyService *const lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (lobbyService->connect(LSGAddr, m_pAuthService->getAuthInfo()))
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
		bdLobbyService *const lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		const bdLobbyConnection::Status connectionStatus = lobbyService->getStatus();
		switch(connectionStatus)
		{			
		case bdConnection::BD_NOT_CONNECTED:
		case bdConnection::BD_DISCONNECTING:
		case bdConnection::BD_DISCONNECTED:
			LOGWARN(g_Log, "C_NetRouterPS3::ContinueLobbyConnect() - unable to connect to demonware lobby service.");
			m_PlatformConnectionState = eConnectionStateFailed;
			PlatformDisconnect();
			break;

		case bdConnection::BD_CONNECTING:
			break;

		case bdConnection::BD_CONNECTED:
			m_Ps3AuthState = ePS3AuthStateNotActive;
			m_PlatformConnectionState = eConnectionStateComplete;
			LOGINFO(g_Log, "C_NetRouterPS3 - Connected to Demonware services.");		
			break;			
		}
	}

	void C_NetRouterPS3::ChooseLsgAddressFromTicket(void)
	{
		assert(m_Initialised);

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
} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
