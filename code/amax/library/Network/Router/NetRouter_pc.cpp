#include <stdafx.h>
#include <bdLobby/bdTitleUtilities/bdTitleUtilities.h>	// [Blur2PC]
#include <Core/core.h>
#include <Network/Core.h>
#include <Network/Network.h>
#include <SystemProfile/SystemProfileManager.h>
#include <SystemProfile/GameInvite.h>
#include "../Misc/NetDebug.h"
#include "../Misc/NetDefines.h"
#include "NetRouter_pc.h"
#include "NetAccountCreationListener.h"
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>
#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include <Iphlpapi.h> // for ip address discovery.

// enable this to run a bandwidth scan during the connection process, it takes ages though.
//#define RUN_BANDWIDTH_SCAN_ON_CONNECTION

#if !defined ( CFG_GOLD )
Debug::Bool g_DisconnectDemonwareLobby("Network.Demonware.Lobby.ForceDisconnect", false);
#endif

#define USE_DEMONWARE_CONTENT_SERVER

#if defined ( USE_DEMONWARE_CONTENT_SERVER )
	#define DW_TITLE_ID						(0x149F) 	
	#define DW_AUTH_ADDRESS					"sbox200-auth.mmp3.demonware.net" 
	#define DW_AUTH_PORT					(3074)
	#define DW_LSG_ADDRESS_DEV				"amax-pc-dev.lsg.mmp3.demonware.net"
	#define DW_LSG_ADDRESS_CERT				"amax-pc-dev.lsg.mmp3.demonware.net"
	#define DW_LSG_ADDRESS_LIVE				"amax-pc-dev.lsg.mmp3.demonware.net"
	#define DW_LSG_PORT						(3074)
	//#define DW_STREAMING_SERVICE_ADDRESS	"amax.contentserver.mmp3.demonware.ne"	// not used since its the same address as the lsg
	#define DW_STREAMING_SERVICE_PORT		(3075)
#else
	// original demonware server details before content server
	#define DW_TITLE_ID						(0x1403)	
	#define DW_AUTH_ADDRESS					"amax-pc.auth.mmp3.demonware.net" 
	#define DW_AUTH_PORT					(3074)
	#define DW_LSG_ADDRESS_DEV				"amax-pc.lsg.mmp3.demonware.net"
	#define DW_LSG_ADDRESS_CERT				"amax-pc.lsg.mmp3.demonware.net"
	#define DW_LSG_ADDRESS_LIVE				"amax-pc.lsg.mmp3.demonware.net"
	#define DW_LSG_PORT						(3074)
#endif

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	// constructor
	C_NetRouterPC::C_NetRouterPC() :	m_Initialised(false), m_Lsg(NULL), m_PcAuthState(ePCAuthStateNotActive),
										m_pAuthGetHostByName(NULL), m_pLsgGetHostByName(NULL), m_pAuthService(NULL),
										m_UserName(NULL), m_Password(NULL), m_License(NULL), m_pAccountCreationListener(NULL), m_ConnectionTimer(0),
										m_StringVerifyActive(false)
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

		if (m_OnlineMode)
		{
			if (NULL == m_UserName || NULL == m_Password)			{
				LOGWARN(g_Log, "C_NetRouterPC::PlatformConnect() - invalid  username or password supplied.");
				m_PlatformConnectionState = eConnectionStateFailed;
				m_LastConnectionErrorCode = eConnectionResultGeneric;
				PlatformDisconnect();
				return false;
			}
		}

		m_LastConnectionErrorCode = eConnectionResultNone;

		LOGINFO(g_Log, "C_NetRouterPC - Connection started - %s", m_OnlineMode ? "Online mode" : "Offline mode");

		return StartConnect();
	}

	void C_NetRouterPC::PlatformDisconnect(void)
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

		LOGINFO(g_Log, "C_NetRouterPC - Connection terminated");

		m_OnlineMode = false;
		m_Lsg = NULL;
		m_UserName = NULL;
		m_Password = NULL;
		m_License = NULL;
		m_pAccountCreationListener = NULL;
		m_PcAuthState = ePCAuthStateNotActive;
		m_ConnectionTimer = 0;

		m_PlatformConnectionState = eConnectionStateNotActive;

		m_pGameInvite->m_Active = false;
	}

	void C_NetRouterPC::Poll(const F32 frameTimeInSeconds)
	{
		if (!m_Initialised)
			return;

		C_NetRouterBase::Poll(frameTimeInSeconds);
		
		bool pollLobbyConnection = false;

		if (eConnectionStateInProgress == m_PlatformConnectionState)
		{
			// we now impose a maximum service connection time.
			const U64 passedTime = (Time::GetPerformanceCounter() - m_ConnectionTimer) / 1000; // convert to miliseconds
			if (passedTime >= 90*1000)
			{
				LOGWARN(g_Log, "C_NetRouterPC::Poll() - timed out connection to demonware srevice");
				m_PlatformConnectionState = eConnectionStateFailed;
				PlatformDisconnect();
				return;
			}

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

			case ePCAuthStateEvaluatingBandwidth:
				pollLobbyConnection = true;
				if (!BandwidthEvaluationInProgress())
				{
					m_PcAuthState = ePCAuthStateNotActive;
					m_PlatformConnectionState = eConnectionStateComplete;
				}
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
			pollLobbyConnection = true;

		// monitor connection to lobby service.
		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
	
		if (IsOnlineMode())
		{
			bool lost = false;
			if (pollLobbyConnection)
			{
				if(lobbyService)
				{
					lobbyService->pump();

#if defined ( USE_DEMONWARE_CONTENT_SERVER )
					if (m_ContentStreamingEnabled)
					{
						bdContentStreaming* contentStreaming = lobbyService->getContentStreaming();
						if(contentStreaming)
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
					LOGWARN(g_Log, "C_NetRouterPC::Poll() - Lost connection to demonware services");

					KillStringVerification();
					StopBandwidthEvaluation();
					EnableContentStreaming(false);

					m_PlatformConnectionState = eConnectionStateFailed;
					m_LastConnectionErrorCode = eConnectionResultGeneric;
					if (lobbyService)
						lobbyService->disconnect();
					m_pGameInvite->m_Active = false;
				}
				SysProfile::C_Manager::Instance().SetDemonwareAuthenticatedPlayerData(m_UserName);
			}
		}
	}

	bool C_NetRouterPC::StartConnect(void)
	{
		BIZ_ASSERT(m_Initialised);
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
			BIZ_ASSERT(heapOutBuf);
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

		m_ConnectionTimer = Time::GetPerformanceCounter();

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
				if (net->getStatus() == bdNetImpl::BD_NET_DONE)
				{
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
		BIZ_ASSERT(m_Initialised);
		BIZ_ASSERT(m_PcAuthState == ePCAuthStateStartingNetwork);

		bdNetImpl *const net = bdNet::getInstance();
		net->pump();
		if (net->getStatus() == bdNetImpl::BD_NET_PENDING)
			return;

		const bool net_ok = (net->getStatus() == bdNetImpl::BD_NET_DONE) ? true : false;

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
		BIZ_ASSERT(m_pAuthService == NULL);

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

					bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
					const bdAddr lsgAddr(m_LsgAddr, DW_LSG_PORT);

#if defined ( USE_DEMONWARE_CONTENT_SERVER )
					// content streaming address is the same as the lsg with a different port.
					m_ContentStreamingAddress = bdAddr(m_LsgAddr, DW_STREAMING_SERVICE_PORT);
#endif

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
			LOGWARN(g_Log, "C_NetRouterPC::ContinueLobbyConnection() - unable to connect to Lobby Services.");
			m_PlatformConnectionState = eConnectionStateFailed;
			m_LastConnectionErrorCode = eConnectionResultGeneric;
			PlatformDisconnect();
			break;

		case bdLobbyConnection::BD_CONNECTING:
			break;

		case bdLobbyConnection::BD_CONNECTED:
			LOGINFO(g_Log, "C_NetRouterPC - Connected to Demonware services.");
			SysProfile::C_Manager::Instance().SetDemonwareAuthenticatedPlayerData(m_UserName);

#if defined (RUN_BANDWIDTH_SCAN_ON_CONNECTION)
			StartBandwidthEvaluation();
#endif
			m_PcAuthState = ePCAuthStateEvaluatingBandwidth;

#if defined ( USE_DEMONWARE_CONTENT_SERVER )
			// now that we are connected we should set the address of the content streaming server.
			//not used in latest version of demonware code. lobbyService->setStreamingServiceAddress(m_ContentStreamingAddress);
#endif
			break;			
		}
	}

	bool C_NetRouterPC::EnableContentStreaming(const bool enabled)
	{
		if (m_Initialised)
		{
			// no additional logic for pc.
			m_ContentStreamingEnabled = enabled;
			return true;
		}
		return false;
	}

	bool C_NetRouterPC::VerifyString(const char* pString)
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

	bool C_NetRouterPC::PollVerifyString(bool& validString)
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

	void C_NetRouterPC::KillStringVerification(void)
	{
		NetPlay::DemonwareRemoteTask::Close(m_StringVerifyTask);
		m_StringVerifyActive = false;
	}

	eNatType C_NetRouterPC::GetNatType(void)
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
							LOGWARN(g_Log, "C_NetRouterPC::GetNatType() - NAT type not handled correctly.");
							break;
						}
					}
				}
			}
		}

		return t;
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
