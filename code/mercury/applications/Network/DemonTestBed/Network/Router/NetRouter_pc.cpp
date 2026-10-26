#include <stdafx.h>
#include <Core/core.h>
#include <Network/Core.h>
#include <Network/Network.h>
#include "../Misc/NetDebug.h"
#include "NetRouter_pc.h"
#include "NetAccountCreationListener.h"
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdLobbyService.h>

#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include <bdconnection/bdConnection.h>
#include <Iphlpapi.h> // for ip address discovery.

#define DW_TITLE_ID  (0x13CB)	

#define DW_AUTH_ADDRESS	"auth.eval.v1.80.mmp3.demonware.net" 
#define DW_AUTH_PORT	(3074)

#define DW_LSG_ADDRESS_DEV	"sandbox190-lsg.mmp3.demonware.net"
#define DW_LSG_ADDRESS_CERT	"sandbox190-lsg.mmp3.demonware.net"
#define DW_LSG_ADDRESS_LIVE	"sandbox190-lsg.mmp3.demonware.net"
#define DW_LSG_PORT			(3074)

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	// constructor
	C_NetRouterPC::C_NetRouterPC() :	m_Initialised(false), m_Lsg(NULL), m_PcAuthState(ePCAuthStateNotActive),
										m_pAuthGetHostByName(NULL), m_pLsgGetHostByName(NULL), m_pAuthService(NULL),
										m_UserName(NULL), m_Password(NULL), m_License(NULL), m_pAccountCreationListener(NULL)
	{
	}

	// destructor
	C_NetRouterPC::~C_NetRouterPC()
	{
		Shutdown();
	}

	bool C_NetRouterPC::Initialise(const U32 sendBufferSize)
	{
		if (m_Initialised)
			return true;

		if (C_NetRouterBase::Initialise(sendBufferSize))
			m_Initialised = true;

		return m_Initialised;
	}

	void C_NetRouterPC::Shutdown(void)
	{
		PlatformDisconnect();
		C_NetRouterBase::Shutdown();	
		m_Initialised = false;
	}

	bool C_NetRouterPC::CreateOnlineAccount(char* userName, char *password, char* licenseCode, C_NetAccountCreationListener* pListener)
	{
		if (!m_Initialised)
			return false;

		PlatformDisconnect();

		if (NULL == userName || NULL == password || NULL == licenseCode || NULL == pListener)
		{
			LOGERROR(g_Log, "C_NetRouterPC::CreateOnlineAccount() - invalid parameters passed.");
			return false;
		}

		// store pointers to username + password + license for account creation..
		m_UserName = userName;
		m_Password = password;
		m_License = licenseCode;
		m_pAccountCreationListener = pListener;
		
		// need to be in online mode to create a platform account.
		m_OnlineMode = true;

		m_LastConnectionErrorCode = eConnectionResultNone;

		return StartConnect();
	}

	bool C_NetRouterPC::PlatformConnect(const bool onlineMode,  char* userName,  char* password)
	{
		if (!m_Initialised)
			return false;

		// was we in the process of creating an account
		if (m_pAccountCreationListener)
		{
			LOGERROR(g_Log, "C_NetRouterPC::PlatformConnect() - called while already creating an account, that must finish first!");
			return false;
		}

		// no point disconnecting and re connecting unless online mode changes.
		if (GetPlatformConnectionState() != eConnectionStateNotActive && GetPlatformConnectionState() != eConnectionStateFailed)
		{
			if (onlineMode == IsOnlineMode())
				return true; // may as well carry on the same connection attempt.
		}

		// disconnect if we are currently connected .
		PlatformDisconnect();

		// store the connection mode
		m_OnlineMode = onlineMode;

		// store pointers to username + password for authentication.
		m_UserName = userName;
		m_Password = password;
		m_License = NULL;
		m_pAccountCreationListener = NULL;

		if (NULL == m_UserName || NULL == m_Password)
		{
			LOGWARN(g_Log, "C_NetRouterPC::PlatformConnect() - invalid  username or password supplied.");
			m_PlatformConnectionState = eConnectionStateFailed;
			m_LastConnectionErrorCode = eConnectionResultGeneric;
			PlatformDisconnect();
			return false;
		}

		m_LastConnectionErrorCode = eConnectionResultNone;

		return StartConnect();
	}

	void C_NetRouterPC::PlatformDisconnect(void)
	{
		StopBandwidthEvaluation();

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
		m_UserName = NULL;
		m_Password = NULL;
		m_License = NULL;
		m_pAccountCreationListener = NULL;
		m_PcAuthState = ePCAuthStateNotActive;

		if (m_PlatformConnectionState != eConnectionStateFailed)
			m_PlatformConnectionState = eConnectionStateNotActive;
	}

	void C_NetRouterPC::Poll(void)
	{
		if (!m_Initialised)
			return;

		C_NetRouterBase::Poll();

		if (eConnectionStateInProgress == m_PlatformConnectionState)
		{
			switch (m_PcAuthState)
			{
			case ePCAuthStateStartingNetwork:
				ContinueNetworkStart();
				break;

			case ePCAuthStateDnsChecks:
				ContinueDnsLookup();
				break;
			
			case ePCAuthStateAuthenticating:
				ContinueAuthentication();
				break;

			case ePCAuthStateConnectingToLobby:
				ContinueLobbyConnection();
				break;

			case ePCAuthStateError:
				LOGWARN(g_Log, "C_NetRouterPC::Poll() - unable to authorise with demonware");
				m_PlatformConnectionState = eConnectionStateFailed;
				m_LastConnectionErrorCode = eConnectionResultGeneric;
				PlatformDisconnect();
				break;
	
			default:
				LOGWARN(g_Log, "C_NetRouterPC::Poll() - unknown authentication state.");
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
						LOGWARN(g_Log, "C_NetRouterPC::Poll() - Lost connection to demonware services");
						m_PlatformConnectionState = eConnectionStateFailed;
						m_LastConnectionErrorCode = eConnectionResultGeneric;
						PlatformDisconnect();
						break;

					case bdConnection::BD_DISCONNECTING:
						break;
					}
				}
			}
		}
	}

	bool C_NetRouterPC::StartConnect(void)
	{
		assert(m_Initialised);
		m_PcAuthState = ePCAuthStateStartingNetwork;

		bdNetImpl *const net = bdNet::getInstance();
		bdNetStartParams params;
		params.m_onlineGame = m_OnlineMode;

		// build the list of local IP addresses..
		// GetAdaptersInfo treats first argument as a chunk of memory.
		U8* heapOutBuf = NULL;
		U8 stackOutBuf[sizeof(IP_ADAPTER_INFO)];
		ULONG outBufLen = sizeof(IP_ADAPTER_INFO);
		PIP_ADAPTER_INFO adapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(&stackOutBuf);

		// Make an initial call to GetAdaptersInfo to get
		// the necessary size into the outBufLen variable
		if (GetAdaptersInfo(adapterInfo, &outBufLen) == ERROR_BUFFER_OVERFLOW)
		{
			// need a bigger buffer
			heapOutBuf = new U8[outBufLen];
			assert(heapOutBuf);
			adapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(heapOutBuf);
		}

		//Get info for the adapters
		if (GetAdaptersInfo(adapterInfo, &outBufLen) != ERROR_SUCCESS)
		{
			// no network adapters ?
			if (heapOutBuf)
				delete [] heapOutBuf;

			// account creation mode ?
			if (m_pAccountCreationListener)
				m_pAccountCreationListener->Finished(eAccountCreationResultNoNetwork);

			LOGWARN(g_Log, "C_NetRouterPC::StartConnect() - no network adapters detected.");
			m_PlatformConnectionState = eConnectionStateFailed;
			m_LastConnectionErrorCode = eConnectionResultNoNetwork;
			PlatformDisconnect();
			return false;
		}
		else
		{
			S32 index = 0;
			while (adapterInfo)
			{
				PIP_ADDR_STRING ipStr = &adapterInfo->IpAddressList;
				while(ipStr)
				{
					if (strcmp(ipStr->IpAddress.String, "0.0.0.0") != 0)
						params.m_localAddresses.pushBack(bdInetAddr(ipStr->IpAddress.String));

					ipStr = ipStr->Next;
				}

				adapterInfo = adapterInfo->Next;
				index++;
			}
		}

		// release adapter enumeration buffer if it exists.
		if (heapOutBuf)
			delete [] heapOutBuf;

		if (params.m_localAddresses.isEmpty())
		{
			// Machine is not connected to any kind of network. Add loopback as local.

			// account creation mode ?
			if (m_pAccountCreationListener)
				m_pAccountCreationListener->Finished(eAccountCreationResultNoNetwork);

			LOGWARN(g_Log, "C_NetRouterPC::StartConnect() - no IP address defined.");
			m_PlatformConnectionState = eConnectionStateFailed;
			m_LastConnectionErrorCode = eConnectionResultNoNetwork;
			PlatformDisconnect();
			return false;
		}

		// start network..
		if (m_OnlineMode)
		{
			m_PlatformConnectionState = eConnectionStateInProgress;
			if (net->start(params))
			{
				m_PcAuthState = ePCAuthStateStartingNetwork;
				return true;
			}
			else
			{
				// account creation mode ?
				if (m_pAccountCreationListener)
					m_pAccountCreationListener->Finished(eAccountCreationResultNoNetwork);

				LOGWARN(g_Log, "C_NetRouterPC::StartConnect() - unable to start network in online mode.");
				m_PlatformConnectionState = eConnectionStateFailed;
				m_LastConnectionErrorCode = eConnectionResultNoNetwork;
				PlatformDisconnect();
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
				m_PcAuthState = ePCAuthStateNotActive;
				m_PlatformConnectionState = eConnectionStateComplete;
				return true;
			}
			else
			{
				LOGWARN(g_Log, "C_NetRouterPC::StartConnect() - unable to start network in offline mode.");
				m_PlatformConnectionState = eConnectionStateFailed;
				m_LastConnectionErrorCode = eConnectionResultNoNetwork;
				PlatformDisconnect();
			}
		}

		return false;
	}

	void C_NetRouterPC::ContinueNetworkStart(void)
	{
		assert(m_Initialised);
		assert(m_PcAuthState == ePCAuthStateStartingNetwork);

		bdNetImpl *const net = bdNet::getInstance();
		net->pump();
		if (net->getStatus() == bdNetImpl::BD_NET_PENDING)
			return;

		const bool net_ok = (net->getStatus() == bdNetImpl::BD_NET_DONE);

		if (!net_ok)
		{
			LOGWARN(g_Log, "C_NetRouterPC::ContinueNetworkStart() - unable to start network in online mode.");

			// account creation mode ?
			if (m_pAccountCreationListener)
				m_pAccountCreationListener->Finished(eAccountCreationResultGenericError);
			
			m_PlatformConnectionState = eConnectionStateFailed;
			m_LastConnectionErrorCode = eConnectionResultGeneric;
			PlatformDisconnect();
		}
		else
		{
			// start dns lookup for LSP and auth services..
#if defined CFG_GOLD
			m_Lsg = DW_LSG_ADDRESS_LIVE;
#else
			m_Lsg = DW_LSG_ADDRESS_DEV;
#endif
			const TaskStatus authDNSStatus = StartGetHostByName(DW_AUTH_ADDRESS, &m_pAuthGetHostByName, m_AuthAddr);
			const TaskStatus lsgDNSStatus = StartGetHostByName(m_Lsg, &m_pLsgGetHostByName, m_LsgAddr);
			if (FAILED == authDNSStatus || FAILED == lsgDNSStatus)
			{
				LOGWARN(g_Log, "C_NetRouterPC::ContinueNetworkStart() - unable to start dns address lookup.");

				// account creation mode ?
				if (m_pAccountCreationListener)
					m_pAccountCreationListener->Finished(eAccountCreationResultGenericError);

				m_PlatformConnectionState = eConnectionStateFailed;
				m_LastConnectionErrorCode = eConnectionResultGeneric;
				PlatformDisconnect();
			}
			else
			{
				if (DONE == authDNSStatus && DONE == lsgDNSStatus)
					StartAuthentication();
				else
					m_PcAuthState = ePCAuthStateDnsChecks;
			}
		}
	}

	void C_NetRouterPC::ContinueDnsLookup(void)
	{
		TaskStatus authDNSStatus = DONE;
		TaskStatus lsgDNSStatus = DONE;

		if (m_pAuthGetHostByName)
			authDNSStatus = PumpGetHostByName(&m_pAuthGetHostByName, m_AuthAddr);

		if (m_pLsgGetHostByName)
			lsgDNSStatus = PumpGetHostByName(&m_pLsgGetHostByName, m_LsgAddr);
	
		if (FAILED == authDNSStatus || FAILED == lsgDNSStatus)
		{
			LOGWARN(g_Log, "C_NetRouterPC::ContinueDnsLookup() - unable to continue dns address lookup.");

			// account creation mode ?
			if (m_pAccountCreationListener)
				m_pAccountCreationListener->Finished(eAccountCreationResultGenericError);

			m_PlatformConnectionState = eConnectionStateFailed;
			m_LastConnectionErrorCode = eConnectionResultGeneric;
			PlatformDisconnect();
			return;
		}

		if (PENDING == authDNSStatus || PENDING == lsgDNSStatus)
			return; // still doing lookup.

		// dns lookup complete, so lets authenticate user.
		StartAuthentication();
	}

	void C_NetRouterPC::StartAuthentication(void)
	{
		assert(m_pAuthService);

		const bdAddr authAddr(m_AuthAddr, DW_AUTH_PORT);
		m_pAuthService = new bdAuthService(DW_TITLE_ID, authAddr);
		if (NULL == m_pAuthService)
		{
			LOGWARN(g_Log, "C_NetRouterPC::StartAuthentication() - unable to create authentication object.");

			// account creation mode ?
			if (m_pAccountCreationListener)
				m_pAccountCreationListener->Finished(eAccountCreationResultGenericError);

			m_PlatformConnectionState = eConnectionStateFailed;
			m_LastConnectionErrorCode = eConnectionResultGeneric;
			PlatformDisconnect();
			return;
		}

		if (NULL == m_pAccountCreationListener)
		{
			// normal sign in mode
			if (m_pAuthService->authorizeAccount(m_UserName, m_Password)) 
				m_PcAuthState = ePCAuthStateAuthenticating;
			else
			{
				LOGWARN(g_Log, "C_NetRouterPC::StartAuthentication() - unable to start authentication.");
				m_PlatformConnectionState = eConnectionStateFailed;
				m_LastConnectionErrorCode = eConnectionResultGeneric;
				PlatformDisconnect();
				return;
			}
		}
		else
		{
			// account creation mode

			// TODO -> how do we define when we need to use the license code ???
			char* license = NULL;
#ifdef CFG_GOLD
			license = m_License;
#endif
			if (m_pAuthService->createAccount(m_UserName, m_Password, license)) 
				m_PcAuthState = ePCAuthStateAuthenticating;
			else
			{
				LOGWARN(g_Log, "C_NetRouterPC::StartAuthentication() - unable to start account creation.");

				m_pAccountCreationListener->Finished(eAccountCreationResultGenericError);

				m_PlatformConnectionState = eConnectionStateFailed;
				m_LastConnectionErrorCode = eConnectionResultGeneric;
				PlatformDisconnect();
				return;
			}
		}
	}

	void C_NetRouterPC::ContinueAuthentication(void)
	{
		const bdAuthService::bdStatus status = m_pAuthService->getStatus();
		if(status == bdAuthService::BD_READY)
		{
			const bdLobbyErrorCode errorCode = m_pAuthService->getErrorCode();
			if(BD_AUTH_NO_ERROR == errorCode)
			{
				if (NULL == m_pAccountCreationListener)
				{
					bdAuthInfo authInfo = *m_pAuthService->getAuthInfo();
					delete m_pAuthService;
					m_pAuthService = NULL;

					bdLobbyService *const lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
					const bdAddr lsgAddr(m_LsgAddr, DW_LSG_PORT);
					if (lobbyService->connect(lsgAddr, &authInfo))
						m_PcAuthState = ePCAuthStateConnectingToLobby;
					else
					{
						LOGWARN(g_Log, "C_NetRouterPC::StartAuthentication() - unable to start LSG connection.");
						m_PlatformConnectionState = eConnectionStateFailed;
						m_LastConnectionErrorCode = eConnectionResultGeneric;
						PlatformDisconnect();
					}
				}
				else
				{
					// successfully created user
					LOGINFO(g_Log, "Successfully created new online account.");
					m_pAccountCreationListener->Finished(eAccountCreationResultSuccess);
					PlatformDisconnect();
				}
			}
			else
			{
				if (NULL == m_pAccountCreationListener)
				{
					LOGWARN(g_Log, "C_NetRouterPC::ContinueAuthentication() - failed to authenticate user.");
					switch (m_pAuthService->getErrorCode())
					{
					case BD_AUTH_BAD_ACCOUNT:
						m_LastConnectionErrorCode = eConnectionResultInvalidUserName;
						break;
					case BD_AUTH_INCORRECT_PASSWORD:
						m_LastConnectionErrorCode = eConnectionResultInvalidPassword;
						break;
					default:
						m_LastConnectionErrorCode = eConnectionResultGeneric;
						break;
					}

					m_PlatformConnectionState = eConnectionStateFailed;
					PlatformDisconnect();
				}
				else
				{
					// account creation error codes.
					const bdLobbyErrorCode ec = m_pAuthService->getErrorCode();
					LOGWARN(g_Log, "Uunable to create new online user account, error %d", ec);

					switch (ec)
					{
					case BD_AUTH_INCORRECT_LICENSE_CODE:
						m_pAccountCreationListener->Finished(eAccountCreationResultLicenseInvalid);
						break;

					case BD_AUTH_ACCOUNT_LOCKED:
						m_pAccountCreationListener->Finished(eAccountCreationResultLicenseLocked);
						break;

					case BD_AUTH_CREATE_USERNAME_EXISTS:
						m_pAccountCreationListener->Finished(eAccountCreationResultUserNameExists);
						break;

					case BD_AUTH_CREATE_USERNAME_ILLEGAL:
						m_pAccountCreationListener->Finished(eAccountCreationResultUserNameIllegal);
						break;

					case BD_AUTH_CREATE_USERNAME_VULGAR:
						m_pAccountCreationListener->Finished(eAccountCreationResultUserNameVulgar);
						break;

					case BD_AUTH_CREATE_MAX_ACC_EXCEEDED:
						m_pAccountCreationListener->Finished(eAccountCreationResultTooManyAccounts);
						break;

					default:
						m_pAccountCreationListener->Finished(eAccountCreationResultGenericError);
						break;
					}

					m_PlatformConnectionState = eConnectionStateFailed;
					PlatformDisconnect();
				}
			}
		}
	}

	void C_NetRouterPC::ContinueLobbyConnection(void)
	{
		bdLobbyService *const lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		const bdLobbyConnection::Status connectionStatus = lobbyService->getStatus();
		switch(connectionStatus)
		{			
		case bdConnection::BD_NOT_CONNECTED:
		case bdConnection::BD_DISCONNECTING:
		case bdConnection::BD_DISCONNECTED:
			LOGWARN(g_Log, "C_NetRouterPC::ContinueLobbyConnection() - unable to connect to LSG.");
			m_PlatformConnectionState = eConnectionStateFailed;
			m_LastConnectionErrorCode = eConnectionResultGeneric;
			PlatformDisconnect();
			break;

		case bdConnection::BD_CONNECTING:
			break;

		case bdConnection::BD_CONNECTED:
			m_PlatformConnectionState = eConnectionStateComplete;
			m_PcAuthState = ePCAuthStateNotActive;
			LOGINFO(g_Log, "C_NetRouterPC - Connected to Demonware services.");
			break;			
		}
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
