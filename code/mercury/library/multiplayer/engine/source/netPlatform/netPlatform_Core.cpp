
#include "stdafx.h"
#include "./netPlatform_Core.h"
#include "./netPlatform_Messages.h"
#include "./Demonware_LobbyEventHandler.h"
#include "netDebug/netDebug.h"
#include "netTime/netTime.h"
#include "netSession/netEnumeratedSession.h"
#include "netPlatform/netPlatform_Core.h"

#include <Network/Core.h>
#include <Network/SessionDeletion.h>
#include <Network/BandwidthEvaluator.h>
#include <SystemProfile/SystemProfileManager.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
#include "ui/Game/Profiles/ProfileInterfaceMessages.h"

#include <bdCore/bdCore.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>
#include <bdNet/bdNet.h>
#include <bdNet/bdLANDiscovery/bdGameInfoFactory.h>
#if !defined (CFG_GOLD)
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h> // for demonware logging
#endif
#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>

#ifdef PLATFORM_PC
#include "netPlatform/netPlatform_PC.h"
#endif
#ifdef PLATFORM_PS3
#include "netPlatform/netPlatform_PS3.h"
#endif

#define NET_MINIMUM_BANDWIDTH_KBpS (16)

namespace netPlatform
{
	Core* Core::s_instance = NULL;
	Debug::Logger& Core::g_Log = Debug::GetLogger("netdebug.platform");

	Core* Core::instance() 
	{ 
		return s_instance; 
	}

	void Core::createInstance() 
	{ 
		assert(s_instance == NULL); 

		#if defined( PLATFORM_PS3 )
			s_instance = new netPlatform::PS3; 
		#elif defined ( PLATFORM_PC )
			s_instance = new netPlatform::PC; 
		#endif
	}

	void Core::destroyInstance()
	{ 
		if(s_instance)
			delete s_instance;
		s_instance = NULL;
	}

	Core::Core()
	{
		m_UpstreamBandwithKBpS			= NET_MINIMUM_BANDWIDTH_KBpS;
		m_DownstreamBandwithKBpS		= NET_MINIMUM_BANDWIDTH_KBpS;											
		m_pBandwidthEvaluator			= NULL;
		m_cableConnected				= false;
		m_cableConnectChecked			= false;
		m_ticketState					= TICKET_NOT_SET;
		m_lastBandwidthEvaluationTime	= 0;
		m_startBandwidthEvaluation		= false;
		m_signInState					= SIGNIN_DIALOG_NOT_STARTED;
		m_onlineMode					= MODE_NOT_SET;
		m_onlineModeRequested			= MODE_NOT_SET;
		m_state							= STATE_NOT_SET;
		m_status						= STATUS_NOT_SET;
		m_Lsg							= NULL;
		m_pAuthGetHostByName			= NULL;
		m_pAuthService					= NULL;
		m_pLsgGetHostByName				= NULL;
		m_AuthenticationTimer			= 0;
		m_pLobbyEventHandler			= NULL;
	}

	Core::~Core()
	{
		shutdown();
	}

	void Core::initialise()
	{
		platformInitialise();

		SetSubscriberSystem(&(UI::gButtons.GetSubscriberSystem()));
		AddSubscriber(UI::E_ProfMsg_GameInvitePending);
		AddSubscriber(UI::E_ProfMsg_SignInChanged);
		AddSubscriber(UI::E_ProfMsg_DuplicateSignIn);
		AddSubscriber(UI::E_ProfMsg_ProfilesChanged);
		AddSubscriber(UI::E_ProfMsg_ProfileSettingsChanged);
		AddSubscriber(UI::E_ProfMsg_StorageDeviceChanged);
		AddSubscriber(UI::E_ProfMsg_NewContentInstalled);
		AddSubscriber(UI::E_ProfMsg_InputDeviceChanged);
		AddSubscriber(UI::E_ProfMsg_MuteInfoChanged);
		AddSubscriber(UI::E_ProfMsg_ServiceConnectionLost);
		AddSubscriber(UI::E_ProfMsg_ServiceConnectionEstablished);
		AddSubscriber(UI::E_ProfMsg_FriendRequestPending);
		AddSubscriber(UI::E_ProfMsg_FriendSignedIn);
		AddSubscriber(UI::E_ProfMsg_DeviceConfigChanged);
		AddSubscriber(UI::E_ProfMsg_SignedIn);
		AddSubscriber(UI::E_ProfMsg_SignedOut);

		SetSubscriberSystem(&m_subscriberSystem);
		AddSubscriber(netPlatform::E_Msg_SignInStart);
		AddSubscriber(netPlatform::E_Msg_NetworkServicesStart);
		AddSubscriber(netPlatform::E_Msg_StartBandwidthEvaluation);
	
		m_cableConnectChecked = false;
		m_cableConnected = false;

		#if !defined (CFG_GOLD)
		m_logSubscriber.addChannel("warn");
		m_logSubscriber.addChannel("err");
		m_logSubscriber.addChannel("info");
		#endif
	}

	void Core::shutdown()
	{
		cleanup();

		m_state					= STATE_NOT_SET;
		m_step					= 0;
		m_status				= STATUS_NOT_SET;
		m_cableConnectChecked	= false;
		m_cableConnected		= false;

		stopBandwidthEvaluation();

		stopLobbyEventHandler();

		RemoveAllSubscribers();

		platformShutdown();
	}

	void Core::update()
	{
		platformUpdate();

		updateState();
		updateLobbyEventHandler();
		updateNetPlay();
		updateCable();
	}

	void Core::updateLobbyEventHandler()
	{
		bdLobbyService* pLobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		
		bool hasEventHandler = (m_pLobbyEventHandler != NULL);
		bool shouldHaveEventHandler = (pLobbyService && pLobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED);
		bool shouldNotHaveEventHandler = (!pLobbyService || !platformRequiresLobby());
		
		// create a new event handler if we need one.
		if(!hasEventHandler && shouldHaveEventHandler)
		{
			m_pLobbyEventHandler = new Demonware::C_LobbyEventHandler();
		}

		// destroy event handler if we do not need one.
		if(hasEventHandler && shouldNotHaveEventHandler)
		{
			delete m_pLobbyEventHandler;
			m_pLobbyEventHandler = NULL;
		}

		if(m_pLobbyEventHandler)
		{
			m_pLobbyEventHandler->RetrieveNotifications(!m_BandwidthEvaluationStarted);
			m_pLobbyEventHandler->Update();
		}
	}

	void Core::updateNetPlay()
	{
		NetPlay::C_SessionDeletion::Instance().Poll();
		NetPlay::C_Core::Instance().Poll(!m_cableConnectChecked);
	}

	void Core::updateCable()
	{
		bool nowConnected = NetPlay::C_Core::Instance().CableConnected();

		if(nowConnected && (!m_cableConnected || !m_cableConnectChecked))
		{
			netPlatform::C_Msg_CableConnected message;
			GetSubscriberSystem().BroadcastMessage(&message);		
		}
		else if(!nowConnected && (m_cableConnected || !m_cableConnectChecked))
		{
			netPlatform::C_Msg_CableDisconnected message;
			GetSubscriberSystem().BroadcastMessage(&message);		
		}

		m_cableConnected = nowConnected;
		m_cableConnectChecked = true;
	}

	bool Core::cableConnected()  
	{ 
		return (m_cableConnected && m_cableConnectChecked);
	}

	bool Core::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		switch(pMessage->Type())
		{
		default:
			{
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %d not handled", pMessage->Type());
				return true;
			}

		case UI::E_ProfMsg_GameInvitePending: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_GameInvitePending");
				return true; 
			}
		case UI::E_ProfMsg_SignInChanged: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_SignInChanged");
				return true; 
			}
		case UI::E_ProfMsg_DuplicateSignIn:
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_DuplicateSignIn");
				return true; 
			}
		case UI::E_ProfMsg_ProfilesChanged: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_ProfilesChanged");
				return true; 
			}
		case UI::E_ProfMsg_ProfileSettingsChanged: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_ProfileSettingsChanged");
				return true; 
			}
		case UI::E_ProfMsg_StorageDeviceChanged: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_StorageDeviceChanged");
				return true; 
			}
		case UI::E_ProfMsg_NewContentInstalled: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_NewContentInstalled");
				return true; 
			}
		case UI::E_ProfMsg_InputDeviceChanged: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_InputDeviceChanged");
				return true; 
			}
		case UI::E_ProfMsg_MuteInfoChanged: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_MuteInfoChanged");
				return true; 
			}
		case UI::E_ProfMsg_ServiceConnectionLost: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_ServiceConnectionLost");
				return true; 
			}
		case UI::E_ProfMsg_ServiceConnectionEstablished: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_ServiceConnectionEstablished");
				return true; 
			}
		case UI::E_ProfMsg_FriendRequestPending: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_FriendRequestPending");
				return true; 
			}
		case UI::E_ProfMsg_FriendSignedIn: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_FriendSignedIn");
				return true; 
			}
		case UI::E_ProfMsg_DeviceConfigChanged: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_DeviceConfigChanged");
				return true; 
			}
		case UI::E_ProfMsg_SignedIn: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_SignedIn");
				return true; 
			}
		case UI::E_ProfMsg_SignedOut: 
			{ 
				LOGWARN(netDebug::g_Log, "<netPlatform::Core> message %s.", "UI::E_ProfMsg_SignedOut");
				return true; 
			}
		case netPlatform::E_Msg_StartBandwidthEvaluation:
			{
				m_startBandwidthEvaluation = true;		
				return true;
			}
		case netPlatform::E_Msg_SignInStart:
			{
				startSignInDialog();
				return true;
			}
		case netPlatform::E_Msg_NetworkServicesStart:
			{
				const netPlatform::C_Msg_NetworkServicesStart* msg = Messaging::MessageCast_Reckless<const netPlatform::C_Msg_NetworkServicesStart>(pMessage);
				requestMode(msg->m_mode);
				return true;
			}
		}
		return true;
	}
	
	bool Core::startSignInDialog()
	{
		// no sign in required. skip.
		if(!platformRequiresSignIn())
		{
			m_signInState = SIGNIN_DIALOG_SUCCESS;
			netPlatform::C_Msg_SignInSuccess msg;
			GetSubscriberSystem().BroadcastMessage( &msg );
			return true;
		}

		if(m_signInState == SIGNIN_DIALOG_STARTED)
			return true;

		// clear dialog state.
		m_signInState = SIGNIN_DIALOG_NOT_STARTED;

		// call platform-specific signin
		return platformSignIn();
	}
	
	bool Core::setState(int state)
	{
		m_state = state;
		m_step = 0;
		return true;
	}

	bool Core::setStatus(int status)
	{
		m_status = status;
		return true;
	}

	bool Core::updateState()
	{
		switch(m_state)
		{
		default:
			{
				LOGWARN(netDebug::g_Log, "<netPlatform::Core::updateState()> unhandled state(%d)", m_state);
				return true;
			}
		case STATE_NOT_SET:
			{
				return true;
			}
		case STATE_RESTART:
			{
				if(m_step == 0)
				{
					// clear components
					cleanup();

					m_step++;
				}

				// wait demonware to stop...
				bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
				if(lobbyService)
				{
					lobbyService->pump();

					if(lobbyService->getStatus() == bdLobbyConnection::BD_DISCONNECTING)
						return true;
				}

				// wait network to stop...
				bdNetImpl *const net = bdNet::getInstance();
				if(net)
				{
					net->pump();

					if(net->getStatus() != bdNetImpl::BD_NET_STOPPED)
						return true;
				}

				// clear status and sign in state
				m_onlineMode	= m_onlineModeRequested;
				m_status		= STATUS_NOT_SET;
				
				switch(m_onlineMode)
				{
				default:
					{
						LOGFATAL(netDebug::g_Log, "<netPlatform::Core> unknown online mode (%d).", m_onlineMode);
						return setStatus(STATUS_ERROR) && setState(STATE_ERROR);
					}
				case MODE_OFFLINE:
				case MODE_ONLINE:
				case MODE_SYSTEMLINK:
					{
						return setState(STATE_SIGNING_IN);
					}
				}
				return true;
			}
		case STATE_SIGNING_IN:
			{
				if(m_step == 0)
				{
					// notification that we restarted services.
					LOGINFO(netDebug::g_Log, "<netPlatform::Core> initiating service.");
					netPlatform::C_Msg_NetworkServicesStarted msg(m_onlineMode);
					GetSubscriberSystem().BroadcastMessage( &msg );
									
					if(!startSignInDialog())
					{
						return setStatus(STATUS_SIGNIN_FAILED) && setState(STATE_ERROR);
					}
					m_step++;
				}
				
				switch(m_signInState)
				{
				default:
				case SIGNIN_DIALOG_NOT_STARTED:
					{
						startSignInDialog();
						return true;
					}
				case SIGNIN_DIALOG_STARTED:
					{
						return true;
					}
				case SIGNIN_DIALOG_FAILED:
					{
						//return setStatus(STATUS_SIGNIN_FAILED) && setState(STATE_ERROR);
					}
				case SIGNIN_DIALOG_SUCCESS:
					{
						return setState(STATE_SIGNED_IN);
					}
				}
				return true;
			}
		case STATE_SIGNED_IN:
			{
				// move forward, connect to demonware services.
				if((m_onlineMode == MODE_SYSTEMLINK) || (m_onlineMode == MODE_ONLINE))
				{
					return setState(STATE_STARTING_NETWORK);
				}

				// just go offline
				if(m_onlineMode == MODE_OFFLINE)
				{
					return setState(STATE_OFFLINE);
				}
				return true;
			}
		case STATE_STARTING_NETWORK:
			{
				bdNetImpl *const net = bdNet::getInstance();
					
				if(net == NULL)
				{
					LOGERROR(netDebug::g_Log, "<netPlatform::Core> no bdNet instance.");
					return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
				}

							
				if(m_step == 0)
				{
					net->stop();
					m_step++;
				}
			
				net->pump();

				switch(net->getStatus())
				{
				default:
					{
						LOGFATAL(netDebug::g_Log, "<netPlatform::Core> unknown error.");
						return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
					}
				case bdNetImpl::BD_NET_STOPPED:
					{
						bdNetStartParams params;
						params.m_onlineGame = (m_onlineMode == MODE_ONLINE)? true : false;

						// setup net params
						if(!platformNetParams(params))
						{
							return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
						}
		
						if (!net->start(params))
						{
							LOGERROR(netDebug::g_Log, "<netPlatform::Core> unable to start network in online mode.");
							return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
						}
						LOGINFO(netDebug::g_Log, "<netPlatform::Core> network mode '%s' started...", params.m_onlineGame? "online" : "systemlink");
						m_step++;
						return true;
					}
				case bdNetImpl::BD_NET_PENDING:
					{
						return true;
					}
				case bdNetImpl::BD_NET_PARAMS_CONFIG_ERROR:
					{
						LOGFATAL(netDebug::g_Log, "<netPlatform::Core> net cofiguration error.");
						return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
					}
				case bdNetImpl::BD_NET_BIND_FAILED:
					{
						LOGFATAL(netDebug::g_Log, "<netPlatform::Core> net bind failed.");
						return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
					}
				case bdNetImpl::BD_NET_ONLINE_FAILED:
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> net online failed. trying offline sign in.");
						
						// Le Fail.
						return setStatus(STATUS_ONLINE_FAILED) && setState(STATE_ERROR);
					}
				case bdNetImpl::BD_NET_INIT_FAILED:
					{
						LOGFATAL(netDebug::g_Log, "<netPlatform::Core> net init failed.");
						return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
					}
				case bdNetImpl::BD_NET_DONE:
					{
						bdCommonAddrRef localAddr = net->getLocalCommonAddr();
						if(localAddr.isNull())
							return true;

						char addrstr[128];

						// done.
						LOGINFO(netDebug::g_Log, "<netPlatform::Core> local address [%s].", getAddrInfo(localAddr, addrstr, sizeof(addrstr)));
					
						// done.
						LOGINFO(netDebug::g_Log, "<netPlatform::Core> net started (%s).", modeToString(m_onlineMode));
							
						// system link mode. no authetication / lobby.
						if(m_onlineMode == MODE_SYSTEMLINK)
						{
							return setState(STATE_SYSTEMLINK);
						}
						else
						{
							return setState(STATE_AUTHDNS_CHECK);
						}
					}
				}
				return true;
			}	
		case STATE_AUTHDNS_CHECK:
			{
				// no authentiation. skip.
				if(!platformRequiresAuthentication())
				{
					LOGINFO(netDebug::g_Log, "<netPlatform::Core> platform requires no authentication.");
					return setState(STATE_LSGDNS_CHECK);
				}

				if(m_step == 0)
				{	
					assert(m_pAuthGetHostByName == NULL);

					const TaskStatus t = StartGetHostByName(platformAuthAddress(), &m_pAuthGetHostByName, m_AuthAddr);

					switch(t)
					{
					default: 
						{
							m_step++;
							return true;
						}
					case FAILED:
						{
							LOGERROR(netDebug::g_Log, "<netPlatform::Core> unable to start dns lookup for auth server.");
							return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
						}
					case DONE:
						{
							LOGINFO(netDebug::g_Log, "<netPlatform::Core> auth dns check done.");
							return setState(STATE_TICKET_REQUEST);
						}
					}
				}

				const TaskStatus t = PumpGetHostByName(&m_pAuthGetHostByName, m_AuthAddr);

				switch(t)
				{
				default: 
					{
						return true;
					}
				case SKIPPED:
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> dns lookup task skipped.");
						return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
					}
				case FAILED:
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> dns lookup failed for auth server.");
						return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
					}
				case DONE:
					{
						LOGINFO(netDebug::g_Log, "<netPlatform::Core> auth dns check done.");
						return setState(STATE_TICKET_REQUEST);
					}
				}
				return true;
			}
		case STATE_TICKET_REQUEST:
			{
				if(m_step == 0)
				{
					assert(m_pAuthService == NULL);

					// create auth service now..
					const bdAddr authAddr(m_AuthAddr, platformAuthPort());
					m_pAuthService = new bdAuthService(platformTitleId(), authAddr);

					if (!m_pAuthService)
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> NULL auth services");
						return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
					}

					// no ticketing. skip.
					if(!platformRequiresTicket())
					{
						LOGINFO(netDebug::g_Log, "<netPlatform::Core> platform requires no ticket for cross-authentication.");
						return setState(STATE_AUTHENTICATION);
					}

					m_ticketState = TICKET_NOT_SET;
					
					// platform-specific ticketing
					if(!platformRequestTicket())
					{
						return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
					}
	
					m_ticketState = TICKET_PENDING;
					m_step++;
				}

				if(m_ticketState == TICKET_PENDING)
					return true;
				
				if(m_ticketState == TICKET_SUCCESS)
				{
					LOGINFO(netDebug::g_Log, "<netPlatform::Core> ticket received.");
					return setState(STATE_AUTHENTICATION);
				}

				if(m_ticketState == TICKET_ERROR)
				{
					LOGERROR(netDebug::g_Log, "<netPlatform::Core> ticket request denied.");
					return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
				}
				return true;
			}
		case STATE_AUTHENTICATION:
			{				
				if(m_step == 0)
				{
					if(!platformAuthentication())
					{
						return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
					}

					m_AuthenticationTimer = Time::GetPerformanceCounter();
					
					LOGINFO(netDebug::g_Log, "<netPlatform::Core> ps3 ticket authorisation started...");
					m_step++;
				}

				const bdAuthService::bdStatus status = m_pAuthService->getStatus();

				const U64 passedTime = (Time::GetPerformanceCounter() - m_AuthenticationTimer); // microseconds
				
				if (status != bdAuthService::BD_READY)
				{
					if(passedTime >= 30 * 1000 * 1000)
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> failed to authenticate NP ticket with demonware. timed out.");
						return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
					}
					else
					{
						return true;
					}
				}
				
				// ready. 
				const bdLobbyErrorCode lobbyErrorCode = m_pAuthService->getErrorCode();

				if(BD_AUTH_NO_ERROR != lobbyErrorCode)
				{
					LOGERROR(netDebug::g_Log, "<netPlatform::Core> failed to authenticate NP ticket with demonware.");
					return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
				}

				LOGINFO(netDebug::g_Log, "<netPlatform::Core> NP ticket authentication with demonware success!");
				return setState(STATE_LSGDNS_CHECK);
			}

		case STATE_LSGDNS_CHECK:
			{
				if(!platformRequiresLobby())
				{
					LOGINFO(netDebug::g_Log, "<netPlatform::Core> platform requires no lobby connection.");
					return setState(STATE_ONLINE);
				}
				
				if(m_step == 0)
				{
					// get address for service
					m_Lsg = platformLsgAddress();

					if(m_Lsg == NULL)
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> failed to resolve lsg.");
						return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
					}

					LOGINFO(netDebug::g_Log, "<netPlatform::Core> lsg server is \"%s\"", m_Lsg);
					
					// authenticated ok, now we need lookup the correct lobby dns address..
					assert(m_pLsgGetHostByName == NULL);

					const TaskStatus t = StartGetHostByName(m_Lsg, &m_pLsgGetHostByName, m_LsgAddr);

					if (FAILED == t) 
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> unable to start dns lookup for LSG server.");
						return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
					}
					else if (DONE == t)
					{
						LOGINFO(netDebug::g_Log, "<netPlatform::Core> LSG dns lookup done.");
						return setState(STATE_LOBBY_CONNECT);
					}
					LOGINFO(netDebug::g_Log, "<netPlatform::Core> LSG dns lookup started.");
					m_step++;
				}

				// m_pLsgGetHostByName will be released and set to NULL inside PumpGetHostByName() when it finishes.
				const TaskStatus t = PumpGetHostByName(&m_pLsgGetHostByName, m_LsgAddr);

				switch(t)
				{
				default:
					{
						return true;
					}
				case SKIPPED:
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> lsg dns lookup task skipped.");
						return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
					}
				case FAILED:
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> lsg dns lookup failed.");
						return setStatus(STATUS_AUTH_FAILED) && setState(STATE_ERROR);
					}
				case DONE:
					{
						LOGINFO(netDebug::g_Log, "<netPlatform::Core> lsg dns lookup finished.");
						return setState(STATE_LOBBY_CONNECT);
					}
				}
				return true;
			}
		case STATE_LOBBY_CONNECT:
			{
				if(m_step == 0)
				{
					assert(m_pAuthService);

					bdAddr LSGAddr(m_LsgAddr, platformLsgPort());

					#if defined ( USE_DEMONWARE_CONTENT_SERVER )
							// content streaming address is the same as the lsg with a different port.
							m_ContentStreamingAddress = bdAddr(m_LsgAddr, DW_STREAMING_SERVICE_PORT);
					#endif

					bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();

					if (!lobbyService || !lobbyService->connect(LSGAddr, m_pAuthService->getAuthInfo()))
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> unable to connect to demonware lobby service.");
						return setStatus(STATUS_LOBBY_FAILED) && setState(STATE_ERROR);
					}

					// can delete auth service now as its not needed anymore..
					delete m_pAuthService;
					m_pAuthService = NULL;
					m_step++;
				}

				bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
				if (!lobbyService)
				{
					LOGERROR(netDebug::g_Log, "<netPlatform::Core> no demonware lobby service.");
					return setStatus(STATUS_LOBBY_FAILED) && setState(STATE_ERROR);
				}

				lobbyService->pump();

				const bdLobbyConnection::Status connectionStatus = lobbyService->getStatus();

				switch(connectionStatus)
				{			
				case bdLobbyConnection::BD_NOT_CONNECTED:
				case bdLobbyConnection::BD_DISCONNECTING:
				case bdLobbyConnection::BD_DISCONNECTED:
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> unable to connect to demonware lobby service.");
						return setStatus(STATUS_LOBBY_FAILED) && setState(STATE_ERROR);
					}
				case bdLobbyConnection::BD_CONNECTING:
					{
						return true;
					}
				case bdLobbyConnection::BD_CONNECTED:
					{
					#if defined ( USE_DEMONWARE_CONTENT_SERVER )
						// now that we are connected we should set the address of the content streaming server.
						lobbyService->setStreamingServiceAddress(m_ContentStreamingAddress);
					#endif

						LOGINFO(netDebug::g_Log, "<netPlatform::Core> Connected to Demonware Lobby services.");
						return setState(STATE_ONLINE);
					}
				}
				return true;
			}
		case STATE_ONLINE:
			{
				if(m_step == 0)
				{
					LOGINFO(netDebug::g_Log, "<netPlatform::Core> running (%s).", modeToString(m_onlineMode));
					netPlatform::C_Msg_NetworkServicesSuccess msg(m_onlineMode);
					GetSubscriberSystem().BroadcastMessage( &msg );
					m_step++;
				}

				SysProfile::C_Manager::eServiceState serviceState = SysProfile::C_Manager::Instance().ServiceConnectionState();

				if(serviceState != SysProfile::C_Manager::eServiceStateConnected && !netDebug::dbg_stayConnected)
				{
					LOGINFO(netDebug::g_Log, "<netPlatform::Core> lost services.");
					return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
				}

				bdNetImpl *const net = bdNet::getInstance();

				if(!net || net->getStatus() != bdNetImpl::BD_NET_DONE)
					return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);

				net->pump();

				if(net->getSocketRouter())
				{
					net->getSocketRouter()->pump();
				}

				// platform doesn't have a lobby.
				if(!platformRequiresLobby())
				{
					updateBandwidthEvaluation();
				}
				// platform has a lobby service. update.
				else
				{
					
					bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
					if (!lobbyService)
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> no demonware lobby service.");
						return setStatus(STATUS_LOBBY_FAILED) && setState(STATE_ERROR);
					}

					lobbyService->pump();

					const bdLobbyConnection::Status connectionStatus = lobbyService->getStatus();

					switch(connectionStatus)
					{			
					default:
						{
							return true;
						}
					case bdLobbyConnection::BD_CONNECTED:
						{
							updateBandwidthEvaluation();
							return true;
						}					
					case  bdLobbyConnection::BD_CONNECTING:
					case bdLobbyConnection::BD_DISCONNECTING:
						{
							return true;
						}
					case bdLobbyConnection::BD_NOT_CONNECTED:
					case bdLobbyConnection::BD_DISCONNECTED:
						{
							if(!netDebug::dbg_stayConnected)
							{
								LOGERROR(netDebug::g_Log, "<netPlatform::Core> disconnected from demonware lobby service.");
								return setStatus(STATUS_LOBBY_FAILED) && setState(STATE_ERROR);
							}
						}
					}
				}
				return true;
			}
		case STATE_SYSTEMLINK:
			{
				if(m_step == 0)
				{
					LOGINFO(netDebug::g_Log, "<netPlatform::Core> running (%s).", modeToString(m_onlineMode));
					netPlatform::C_Msg_NetworkServicesSuccess msg(m_onlineMode);
					GetSubscriberSystem().BroadcastMessage( &msg );
					m_step++;
				}

				bdNetImpl *const net = bdNet::getInstance();

				if(net == NULL)
				{
					LOGERROR(netDebug::g_Log, "<netPlatform::Core> no bdNet instance.");
					return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
				}

				net->pump();

				if(net->getSocketRouter())
				{
					net->getSocketRouter()->pump();
				}

				switch(net->getStatus())
				{
				default:
					{
						LOGFATAL(netDebug::g_Log, "<netPlatform::Core> unknown error.");
						return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
					}
				case bdNetImpl::BD_NET_STOPPED:
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> net stopped.");
						return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
					}
				case bdNetImpl::BD_NET_PENDING:
					{
						return true;
					}
				case bdNetImpl::BD_NET_PARAMS_CONFIG_ERROR:
					{
						LOGFATAL(netDebug::g_Log, "<netPlatform::Core> net cofiguration error.");
						return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
					}
				case bdNetImpl::BD_NET_BIND_FAILED:
					{
						LOGFATAL(netDebug::g_Log, "<netPlatform::Core> net bind failed.");
						return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
					}
				case bdNetImpl::BD_NET_ONLINE_FAILED:
					{
						LOGERROR(netDebug::g_Log, "<netPlatform::Core> net online failed. trying offline sign in.");
						return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
					}
				case bdNetImpl::BD_NET_INIT_FAILED:
					{
						LOGFATAL(netDebug::g_Log, "<netPlatform::Core> net init failed.");
						return setStatus(STATUS_NETWORK_FAILED) && setState(STATE_ERROR);
					}
				case bdNetImpl::BD_NET_DONE:
					{
						//updateBandwidthEvaluation();
						return true;
					}
				}
				return true;
			}
		case STATE_OFFLINE:
			{
				if(m_step == 0)
				{
					LOGINFO(netDebug::g_Log, "<netPlatform::Core> running (%s).", modeToString(m_onlineMode));
					// clear components
					cleanup();
					netPlatform::C_Msg_NetworkServicesSuccess msg(m_onlineMode);
					GetSubscriberSystem().BroadcastMessage( &msg );
					m_step++;
				}
				return true;
			}
		case STATE_ERROR:
			{
				if(m_step == 0)
				{
					LOGINFO(netDebug::g_Log, "<netPlatform::Core> error '%s' (%s).", statusToString(m_status), modeToString(m_onlineMode));
					
					// clear components
					cleanup();

					netPlatform::C_Msg_NetworkServicesFailed msg(m_onlineMode, m_status);
					GetSubscriberSystem().BroadcastMessage( &msg );
					m_step++;
				}
				return true;
			}		
		}
	}

	const char* Core::statusToString(int status)
	{
		#undef CASETOSTR
		#define CASETOSTR(x) case STATUS_##x: return "STATUS_"#x; 
		switch(status)
		{
		default: return "UNHANDLED STATUS.";
		CASETOSTR(NOT_SET)
		CASETOSTR(SUCCESS)
		CASETOSTR(ERROR)
		CASETOSTR(SIGNIN_FAILED)
		CASETOSTR(NETWORK_FAILED)
		CASETOSTR(ONLINE_FAILED)
		CASETOSTR(AUTH_FAILED)
		CASETOSTR(LOBBY_FAILED)
		CASETOSTR(LOBBY_DISCONNECTED)
		}	
	}

	const char* Core::stateToString(int state)
	{
		#undef CASETOSTR
		#define CASETOSTR(x) case STATE_##x: return "STATE_"#x; 
		switch(state)
		{
		default: return "UNHANDLED STATE.";
		CASETOSTR(RESTART)
		CASETOSTR(SIGNING_IN)
		CASETOSTR(STARTING_NETWORK)
		CASETOSTR(AUTHDNS_CHECK)
		CASETOSTR(TICKET_REQUEST)
		CASETOSTR(AUTHENTICATION)
		CASETOSTR(LSGDNS_CHECK)
		CASETOSTR(LOBBY_CONNECT)
		CASETOSTR(SYSTEMLINK)
		CASETOSTR(ONLINE)
		CASETOSTR(OFFLINE)
		CASETOSTR(ERROR)
		}
	};

	const char* Core::modeToString(int mode)
	{
		#undef CASETOSTR
		#define CASETOSTR(x) case MODE_##x: return "MODE_"#x; 
		switch(mode)
		{
		default: return "UNHANDLED MODE.";
		CASETOSTR(NOT_SET)
		CASETOSTR(DEFAULT)
		CASETOSTR(SYSTEMLINK)
		CASETOSTR(ONLINE)
		CASETOSTR(OFFLINE)
		}	
	}

	void Core::cleanup(void)
	{
			if (m_pAuthGetHostByName)
			{
				m_pAuthGetHostByName->quit();
				delete m_pAuthGetHostByName;
			}

			if (m_pLsgGetHostByName)
			{
				m_pLsgGetHostByName->quit();
				delete m_pLsgGetHostByName;
			}
			if (m_pAuthService)
				delete m_pAuthService;

		// stop demonware..
		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (lobbyService)
			lobbyService->disconnect();

		bdNetImpl *const net = bdNet::getInstance();
		if(net)
			net->stop();
		
		// stop any bandwidth evaluation
		stopBandwidthEvaluation();

		// reset pointers
		m_Lsg					= NULL;
		m_pAuthGetHostByName	= NULL;
		m_pAuthService			= NULL;
		m_pLsgGetHostByName		= NULL;
		m_AuthenticationTimer	= 0;
	
		// clean up platform-dependant stuff
		platformCleanup();
	}

	bool Core::connecting()
	{
		return (m_state < STATE_SYSTEMLINK) && (m_state > STATE_NOT_SET);
	}
	
	bool Core::error()
	{
		return (m_state == STATE_ERROR);
	}
	
	bool Core::requestMode(int mode)
	{
		// the requested mode.
		m_onlineModeRequested = mode;
		
		// log
		LOGINFO(netDebug::g_Log, "<netPlatform::Core> mode %s requested.", modeToString(m_onlineModeRequested));
		
		// sel;ect mode based on account type.
		if(m_onlineModeRequested == MODE_DEFAULT)
		{
			m_onlineModeRequested = (platformHasOnlineAccount())? MODE_ONLINE : MODE_OFFLINE;
		}
		
		// we want to go offline
		if((m_onlineModeRequested == MODE_OFFLINE) && !runningOffline())
		{
			// go offline
			setState(STATE_RESTART);
			return true;
		}
		
		// we want to go online, but we started in systemlink (or offline).
		if((m_onlineModeRequested == MODE_ONLINE) && !runningOnline())
		{
			// restart services.
			setState(STATE_RESTART);
			return true;
		}
		
		// we want to go systemlink, but not currently in systemlink or online.
		if((m_onlineModeRequested == MODE_SYSTEMLINK) && !runningOnline() && !runningSystemLink())
		{
			// restart services.
			setState(STATE_RESTART);
			return true;
		}
		
		//assert(m_onlineMode == m_onlineModeRequested);
		LOGINFO(netDebug::g_Log, "<netPlatform::Core> running (%s).", modeToString(m_onlineMode));
		netPlatform::C_Msg_NetworkServicesSuccess notification(m_onlineMode);
		GetSubscriberSystem().BroadcastMessage( &notification );
		return true;
	}
	
	bool Core::runningMode(int mode)
	{
		switch(mode)
		{
		default:
			{
				LOGERROR(netDebug::g_Log, "<netPlatform::Core::runningMode()> invalid mode (%d) specified.", mode);
				return false;
			}
		case MODE_ONLINE:
			{
				return runningOnline();
			}
		case MODE_OFFLINE:
			{
				return runningOffline();

			}
		case MODE_SYSTEMLINK:
			{
				return runningSystemLink();
			}
		}
	}
	
	bool Core::runningOffline()
	{
		return (m_state == STATE_OFFLINE);
	}

	bool Core::runningSystemLink()
	{
		return (m_state == STATE_SYSTEMLINK);
	}

	bool Core::runningOnline()
	{
		return (m_state == STATE_ONLINE);
	}

	void Core::stopLobbyEventHandler()
	{
		if(m_pLobbyEventHandler)
			delete m_pLobbyEventHandler;
		m_pLobbyEventHandler = NULL;
	}

	bool Core::getBandwidthEvaluation(float& upstreamKBps, float& downstreamKBps)
	{
		upstreamKBps = m_UpstreamBandwithKBpS;
		downstreamKBps = m_DownstreamBandwithKBpS;
		return true;
	}

	bool Core::bandwidthEvaluationInProgress(void)
	{
		return (m_pBandwidthEvaluator != NULL);
	}

	bool Core::stopBandwidthEvaluation()
	{
		// shutdown bandwidth evaluator
		if (m_pBandwidthEvaluator)
		{
			m_pBandwidthEvaluator->Stop();
			delete m_pBandwidthEvaluator;
			m_pBandwidthEvaluator = NULL;
		}
		return true;
	}

	bool Core::continueBandwidthEvaluation(bool& error)
	{
		error = false;
		
		// evaluator finished.
		if(m_pBandwidthEvaluator == NULL)
			return false;

		// poll.
		m_pBandwidthEvaluator->Update();

		// still evaluating.
		if (m_pBandwidthEvaluator->IsActive())
			return true;

		// finished.
		U32 up, down;
		m_pBandwidthEvaluator->GetResults(up, down);

		// convert to kB/s from b/s.
		m_UpstreamBandwithKBpS = up / 8024.0f;
		m_DownstreamBandwithKBpS = down / 8024.0f;

		// cap to sane rate.
		if(m_UpstreamBandwithKBpS < NET_MINIMUM_BANDWIDTH_KBpS) m_UpstreamBandwithKBpS = NET_MINIMUM_BANDWIDTH_KBpS;
		if(m_DownstreamBandwithKBpS < NET_MINIMUM_BANDWIDTH_KBpS) m_DownstreamBandwithKBpS = NET_MINIMUM_BANDWIDTH_KBpS;

		LOGINFO(netDebug::g_Log, "<netPlatform::Core::continueBandwidthEvaluation()> Bandwidth evaluation complete, Up : %.2f kbytes/s  Down %.2f kbytes/s", m_UpstreamBandwithKBpS, m_DownstreamBandwithKBpS);
		
		stopBandwidthEvaluation();
		return false;
	}

	bool Core::startBandwidthEvaluation(void)
	{
		if (m_pBandwidthEvaluator)
		{
			LOGWARN(netDebug::g_Log, "<netPlatform::Core::startBandwidthEvaluation()> bandwidth evaluation already in progress.");
			return false;
		}
		
		LOGINFO(netDebug::g_Log, "<netPlatform::Core::startBandwidthEvaluation()> bandwidth evaluation started.");
		m_pBandwidthEvaluator = new NetPlay::C_BandwidthEvaluator;
		m_pBandwidthEvaluator->Start();
		return true;
	}	

	void Core::updateBandwidthEvaluation(void)
	{
		if(m_startBandwidthEvaluation)
		{
			// 10 minutes interval between bandwidth evaluations
			if((m_lastBandwidthEvaluationTime == 0) || ((Time::GetPerformanceCounter() - m_lastBandwidthEvaluationTime) > 10 * 60 * 1000 * 1000))
			{
				if(startBandwidthEvaluation())
				{
					// signal message
					netPlatform::C_Msg_BandwidthEvaluationStarted msg;
					GetSubscriberSystem().BroadcastMessage( &msg );
				
					m_BandwidthEvaluationStarted	= true;
					m_lastBandwidthEvaluationTime	= Time::GetPerformanceCounter();
					m_startBandwidthEvaluation		= false;
				}
			}
		}

		if(m_BandwidthEvaluationStarted)
		{
			m_lastBandwidthEvaluationTime = Time::GetPerformanceCounter();

			bool error;
			if(!continueBandwidthEvaluation(error))
			{
				// signal message
				netPlatform::C_Msg_BandwidthEvaluationFinished msg(!error, m_UpstreamBandwithKBpS, m_DownstreamBandwithKBpS);
				GetSubscriberSystem().BroadcastMessage( &msg );
				m_BandwidthEvaluationStarted = false;
			}
		}
	}

	int Core::getHandleStatus(const bdAddrHandleRef& handle)
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement getHandleStatus()");
		return ADDRESS_STATUS_CONNECTED; // less buggy!
	}		

	const char* Core::getHandleInfo(const bdAddrHandleRef &handle, char* str, int len)
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement getHandleInfo()");
		str[0] = 0;
		return str;
	}

	const char*	Core::getAddrInfo(const bdCommonAddrRef& addr, char* str, int len)
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement getAddrInfo()");
		str[0] = 0;
		return str;
	}

	const char* Core::getConsoleName(char* name, int len)
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement getConsoleName()");
		name[0] = 0;
		return name;
	}

	const WCHAR* Core::getConsoleName(WCHAR* name, int len)
	{
		char* cname = new char[len];
		
		getConsoleName(cname, len);
		
		for(int i = 0; i < len; i ++)
			name[i] = cname[i];

		delete[] cname;
		return name;
	}

	int Core::NATtype() 
	{
		#ifndef CFG_GOLD
		if(netDebug::dbg_NAToverride != E_DbgNAT_NotSet)
			return netDebug::dbg_NAToverride;
		#endif
		return platformNATtype(); 
	}			
	
	int Core::platformNATtype()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformInitialise()");
		return netPlatform::NAT_TYPE_NOT_SET;
	}
		
	// platform functions for state machine
	void Core::platformInitialise()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformInitialise()");
	}

	void Core::platformShutdown()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformShutdown()");
	}

	void Core::platformUpdate()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformUpdate()");
	}

	bool Core::platformRequiresSignIn()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformRequiresSignIn()");
		return false;
	}

	bool Core::platformRequiresAuthentication()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformRequiresAuthentication()");
		return false;
	}

	bool Core::platformRequiresTicket()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformRequiresTicket()");
		return false;
	}

	bool Core::platformRequiresLobby()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformRequiresLobby()");
		return false;
	}

	int	Core::platformTitleId()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformTitleId()");
		return 0;
	}
	
	const char* Core::platformAuthAddress()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformAuthAddress()");
		return "NO AUTH ADDRESS";
	}

	int	Core::platformAuthPort()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformAuthPort()");
		return 0;
	}

	const char* Core::platformLsgAddress()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformLsgAddress()");
		return "NO LSG ADDRESS";
	}

	int Core::platformLsgPort()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformLsgPort()");
		return 0;
	}

	bool Core::platformSignIn()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core()> implement platformSignIn()");
		return false;
	}

	bool Core::platformNetParams(bdNetStartParams& params)
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformNetParams()");
		return false;
	}

	bool Core::platformRequestTicket()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformRequestTicket()");
		return false;
	}

	bool Core::platformAuthentication()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformAuthentication()");
		return false;
	}

	bool Core::platformCleanup()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformCleanup()");
		return false;
	}

	bool Core::platformHasOnlineAccount()
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformHasOnlineAccount()");
		return false;
	}

	bool Core::profileIdentifiable(const SysProfile::FullProfile& profile)
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformProfileIdentifiable()");
		return false;
	}
	
	bdOnlineUserID Core::getUserId(const SysProfile::UserIdentifier &user)
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformGetUserId()");
		return false;
	}

	bool Core::sameUser(const SysProfile::UserIdentifier &a, const SysProfile::UserIdentifier &b)
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::Core> implement platformSameUser()");
		return false;
	}

	bool Core::sameSecurityKey(const bdSecurityKey& key1, const bdSecurityKey& key2)
	{
		return (bdMemcmp(&key1, &key2, sizeof(bdSecurityKey)) == 0);
	}

	bool Core::sameSecurityId(const bdSecurityID& id1, const bdSecurityID& id2)
	{
		return (bdMemcmp(&id1, &id2, sizeof(bdSecurityID)) == 0);
	}

	bool Core::sameCommonAddr(const bdCommonAddrRef& addr1, const bdCommonAddrRef& addr2)
	{
		if(addr1.isNull() || addr2.isNull())
		{
			BIZ_ASSERT_MSG(false, "null common address");
			return false;
		}
		return ((*addr1) == (*addr2));
	}

	bool Core::sameAddrHandle(const bdAddrHandleRef& handle1, const bdAddrHandleRef& handle2)
	{
		bdAddrHandleRefWrapper w1(handle1);
		bdAddrHandleRefWrapper w2(handle2);
		return (w1 == w2);
	}
	
	void Core::random(void* data, int len)
	{
		bdTrulyRandom::getInstance()->getRandomUByte8((bdUByte8*)data, len);
	}

	int	Core::randomInt(int min, int max)
	{
		int range = ((max - min) + 1);
		return min + randomInt(range);
	}

	int	Core::randomInt(int range)
	{
		int val;
		random(&val, sizeof(val));
		return (abs(val) % range);
	}

	float Core::randomFloat(float range)
	{
		int r = 10000;
		int integer = randomInt(r);
		return ((float)integer / (float)(r-1)) * range;
	}

	void Core::generateRandom(void* ptr, int len)
	{
		bdTrulyRandom::getInstance()->getRandomUByte8((bdUByte8*)ptr, len);
	}

	netNonce Core::generateNonce()
	{
		netNonce nonce;
		generateRandom(&nonce, sizeof(nonce));
		return nonce;
	}

	bool Core::generateKey(bdSecurityID& secId, bdSecurityKey& secKey)
	{
		bdNetImpl *const net = bdNet::getInstance();
		if(!net) return false;

		bdSecurityKeyMap *const keyMap = net->getSocketRouter()->getKeyMap();
		if(!keyMap) return false;

		keyMap->initKey(secId, secKey);
		return true;
	}

	bool Core::sameNonce(const netNonce& nonce1, const netNonce& nonce2)
	{
		return (nonce1 == nonce2);
	}

	const char* Core::nonceToString	(const netNonce& nonce)
	{
		#define NETBYTE(i, b) ((i >> (8*(b))) & (0xFF))

		static char str[4][48]; // 4 rotation of static buffers (could be debugging several nonces at once).
		static int index = 0;
		
		int i = index;
		sprintf_s(	str[i], sizeof(str[i]), 
					"%02x.%02x.%02x.%02x", 
					NETBYTE(nonce, 3), NETBYTE(nonce, 2), NETBYTE(nonce, 1), NETBYTE(nonce, 0));

		index = (index + 1) & 3;
		
		return str[i];
	}

	bool Core::registerKey(bool online, const bdSecurityID& secId, const bdSecurityKey& secKey)
	{
		bdNetImpl *const net = bdNet::getInstance();
		bdSecurityKeyMap *const keyMap = net->getSocketRouter()->getKeyMap();

		bdSecurityKey tempKey;
		if(keyMap->get(secId, tempKey))
		{
			if(!(tempKey == secKey))
			{
				LOGFATAL(g_Log, "<netPlatform::Core::registerKey()> security Id(%s) already registered with different security key(%s instead of %s))", netDebug::getByteInfo(&secId, sizeof(secId)), netDebug::getByteInfo(&tempKey, sizeof(tempKey)), netDebug::getByteInfo(&secKey, sizeof(secKey)));
				return false;
			}
			LOGWARN(g_Log, "<netPlatform::Core::registerKey()> security Id(%s) key(%s) already registered.", netDebug::getByteInfo(&secId, sizeof(secId)), netDebug::getByteInfo(&secKey, sizeof(secKey)));
			return true;
		}

		// check if key already registered
		LOGINFO(g_Log, "<netPlatform::Core::registerKey()> registering security pair : id(%s), key(%s)", netDebug::getByteInfo(&secId, sizeof(secId)), netDebug::getByteInfo(&secKey, sizeof(secKey)));
		return keyMap->registerKey(secId, secKey);
	}

	bool Core::unregisterKey(bool online, const bdSecurityID& secId, const bdSecurityKey& secKey)
	{
		bdNetImpl *const net = bdNet::getInstance();
		bdSecurityKeyMap *const keyMap = net->getSocketRouter()->getKeyMap();

		bdSecurityKey tempKey;
		if(keyMap->get(secId, tempKey))
		{
			if(!(tempKey == secKey))
			{
				LOGFATAL(g_Log, "<netPlatform::Core::unregisterKey()> security Id(%s) registered with different security key(%s instead of %s))", netDebug::getByteInfo(&secId, sizeof(secId)), netDebug::getByteInfo(&tempKey, sizeof(tempKey)), netDebug::getByteInfo(&secKey, sizeof(secKey)));
				return false;
			}
			LOGINFO(g_Log, "<netPlatform::Core::registerKey()> unregistering security pair : id(%s), key(%s)", netDebug::getByteInfo(&secId, sizeof(secId)), netDebug::getByteInfo(&secKey, sizeof(secKey)));
			keyMap->unregisterKey(secId);
			return true;
		}
		LOGWARN(g_Log, "<netPlatform::Core::unregisterKey()> security Id(%s) key(%s) not found.", netDebug::getByteInfo(&secId, sizeof(secId)), netDebug::getByteInfo(&secKey, sizeof(secKey)));
		return true;
	}

	bdAddrHandleRef	Core::registerAddress(const bdCommonAddrRef& addr, const bdSecurityID& secId)
	{
		bdAddrHandleRef addrHandle;

		bdNetImpl *const net = bdNet::getInstance();
		bdAddressMap *const addressMap = net->getSocketRouter()->getAddressMap();
		
		if(!addressMap->getAddrHandle(addr, secId, addrHandle))
		{
			return NULL;
		}

		char handle[128];
		char ip[64];	
		LOGINFO(g_Log, "<netPlatform> Created address handle (%s) for [%s]", getHandleInfo(addrHandle, handle, sizeof(handle)), getAddrInfo(addr, ip, sizeof(ip)));
		return addrHandle;
	}

	bool Core::unregisterAddress(bdAddrHandleRef addrHandle)
	{
		if (addrHandle.isNull())
			return false;

		bdNetImpl *const net = bdNet::getInstance();
		bdAddressMap *const addressMap = net->getSocketRouter()->getAddressMap();
			
		//return addressMap->unregisterAddr(addrHandle);
		
		char buff[128];
		LOGINFO(g_Log, "<netPlatform> Unregistered address handle (%s)", getHandleInfo(addrHandle, buff, sizeof(buff)));
		return net->getSocketRouter()->disconnect(addrHandle);
	}

	bool Core::connectAddress(const bdCommonAddrRef& addr, bdAddrHandleRef &addrHandle)
	{

		if(addrHandle.isNull())
		{
			LOGERROR(g_Log, "<netPlatform> address handle is null.");
			return false;
		}

		char handle[128];
		char ip[64];	
		
		if(!addr->isLoopback())
		{
			bdNetImpl *const net = bdNet::getInstance();

			if(net->getSocketRouter())
			{
				if(!net->getSocketRouter()->connect(addrHandle))
				{
					LOGERROR(g_Log, "<netPlatform> failed to connect address (%s) [%s]", getHandleInfo(addrHandle, handle, sizeof(handle)), getAddrInfo(addr, ip, sizeof(ip)));
					return false;
				}
			}
		}
		else
		{
			LOGINFO(g_Log, "<netPlatform> address (%s) [%s] is loopback.", getHandleInfo(addrHandle, handle, sizeof(handle)), getAddrInfo(addr, ip, sizeof(ip)));
			return true;
			//addrHandle->setStatus(bdAddrHandle::BD_ADDR_RESOLVED);
		}
		LOGINFO(g_Log, "<netPlatform> started connection with address (%s) [%s]", getHandleInfo(addrHandle, handle, sizeof(handle)), getAddrInfo(addr, ip, sizeof(ip)));
		return true;
	}

	bdCommonAddrRef Core::myCommonAddr()
	{
		bdNetImpl *const net = bdNet::getInstance();
		bdCommonAddrRef localAddr = net->getLocalCommonAddr();
		return localAddr;
	}
};