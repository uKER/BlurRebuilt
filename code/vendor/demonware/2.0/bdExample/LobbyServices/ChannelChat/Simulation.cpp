// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include <bdLobby/bdMessaging/bdMessaging.h>

#define TITLE_ID (0x14BF)

#define PLAYER1 "ChatExample"
#define PASS1 "MyPassword"
#define PLAYER2 "ChatExample2"
#define PASS2 "MyPassword"

#define USERNAME "ChatExample"
#define PASSWORD "MyPassword"


Simulation::Simulation()
: m_taskStatus(READY),
  m_task(START_NETWORK),
  m_changedScreen(true),
  m_authGetHostByName(BD_NULL),
  m_authService(BD_NULL),
  m_lsgGetHostByName(BD_NULL),
  m_remoteTask(BD_NULL),
  m_subscribingChannelID(0),
  m_pumpLobbyConnection(false),
  m_lobbyErrorCode(BD_NO_ERROR)
{
}

Simulation::~Simulation()
{
}

void Simulation::onChatChannelMessage(bdUInt64 channelID,
                                      bdOnlineUserID senderID,
									  bdNChar8 *senderName,
									  void *message,
									  bdUInt messageSize)
{
	bdPrintf("Received chat message from %s :\n", senderName);
	bdNChar8 msg[BD_MAX_CHAT_MESSAGE_SIZE];
	bdMemset(msg, 0, BD_MAX_CHAT_MESSAGE_SIZE);
	bdSnprintf(msg, BD_MAX_CHAT_MESSAGE_SIZE, "%s", message);
	bdPrintf("%s\n", msg);
}

void Simulation::onChatChannelUpdate(bdUInt64 channelID,
										 bdOnlineUserID userID,
										 bdNChar8 *userName,
										 bdUByte8 status)
{
	if (status == BD_USER_SUBSCRIBED)
	{
		bdPrintf("%s joined the chat room\n", userName);

	}
	else if (status == BD_USER_UNSUBSCRIBED)
	{
		bdPrintf("%s left the chat room\n", userName);
	}
}

bdBool Simulation::init(bdInt32 ,
						bdNChar8* [])
{
	bdBool ok = true;

	// start logging
	// we want to hear about all warnings and errors
	m_logSubscriber.addChannel("warn");
	m_logSubscriber.addChannel("err");
	m_logSubscriber.addChannel("info");
	m_lobbyService = new bdLobbyService();
	return ok;
}

bdBool Simulation::step(const bdFloat32)
{
	const bdBool quitKey = getScreen().wasKeyPressed(BD_QUIT);

	TaskStatus oldTaskStatus = m_taskStatus;

	if(m_taskStatus == READY)
	{
		startTask();
	}

	if(m_taskStatus == PENDING || m_taskStatus == DONE || m_taskStatus == FAILED)
	{
		// pump even if the status is DONE or FAILED to read out any results
		// and close any remote task handles
		pumpTask();
	}

	if(m_taskStatus == DONE || m_taskStatus == FAILED || m_taskStatus == SKIPPED)
	{
		// in case the task fails or completes in a single loop
		if(oldTaskStatus == READY)
		{
			oldTaskStatus = m_taskStatus;
		}

		displayTaskResult();
	}

	m_changedScreen = (oldTaskStatus != m_taskStatus);

	if(m_pumpLobbyConnection)
	{
		bdBool lostConnection = true;

		if(m_lobbyService)
		{
			m_lobbyService->pump();
			const bdLobbyConnection::Status status = m_lobbyService->getStatus();
			switch(status)
			{
				case bdConnection::BD_NOT_CONNECTED:
				case bdConnection::BD_CONNECTING:
				case bdConnection::BD_DISCONNECTING:
				case bdConnection::BD_DISCONNECTED:
				{
					lostConnection = true;
				}
				break;
				case bdConnection::BD_CONNECTED:
				{
					lostConnection = false;
					m_lobbyService->registerEventHandler(this);
				}
				break;
			}
		}

		if(lostConnection)
		{
			// set up for a reconnect
			m_pumpLobbyConnection = false;
			m_task = STOP_NETWORK;
			m_taskStatus = READY;
			m_changedScreen = true;
			m_remoteTask = BD_NULL;
			m_lastTaskDescription = "Lost connection to lobby.";
		}
	}

	m_changedScreen = (oldTaskStatus != m_taskStatus);
	getScreen().printToScreen(m_screenText + m_messageText);

	if(m_changedScreen)
	{
		m_screenText = "";
	}

	return !quitKey;
}

void Simulation::quit()
{
	m_logSubscriber.removeChannel("warn");
	m_logSubscriber.removeChannel("err");
	m_logSubscriber.removeChannel("info");
	
	

	delete m_authGetHostByName;
	delete m_authService;
	delete m_lsgGetHostByName;
	delete m_lobbyService;

	stopNetwork();
}

void Simulation::startTask()
{	
	const bdBool skipKey = getScreen().wasKeyPressed(SKIP_KEY);

	if (skipKey)
	{
		m_taskStatus = SKIPPED;
	}
	else
	{
		const bdBool startKey = getScreen().wasKeyPressed(START_KEY);
		bdString taskDescription("Unknown task");

		switch(m_task)
		{
			case START_NETWORK:
			{
				startNetworkDescription(taskDescription);
				if(startKey)
				{
					startNetwork();
				}
			}
			break;
			case CHECK_DNS:
			{
				checkDNSDescription(taskDescription);
				if(startKey)
				{
					startCheckDNS();
				}
			}
			break;
			case CREATE_PLAYER1:
			{
				createPlayer1Description(taskDescription);
				if(startKey)
				{
					startCreatePlayer1();
				}
			}
			break;
			case CREATE_PLAYER2:
			{
				createPlayer2Description(taskDescription);
				if(startKey)
				{
					startCreatePlayer2();
				}
			}
			break;
			case AUTHENTICATE_PLAYER1:
			{
				authenticatePlayer1Description(taskDescription);
				if(startKey)
				{
					startAuthenticatePlayer1();
				}
			}
			break;
			case AUTHENTICATE_PLAYER2:
			{
				authenticatePlayer2Description(taskDescription);
				if(startKey)
				{
					startAuthenticatePlayer2();
				}
			}
			break;
			case LOBBY_CONNECT:
			{
				lobbyServiceConnectDescription(taskDescription);
				if(startKey)
				{
					startLobbyServiceConnect();
				}
			}
			break;
			case GET_CHANNEL_LIST:
			{
				getChannelListDescription(taskDescription);
				if(startKey)
				{
					startGetChannelList();
				}
			}
			break;
			case CREATE_CHANNEL:
			{
				getCreateChannelDescription(taskDescription);
				if(startKey)
				{
					startCreateChannel();
				}
			}
			break;
			case SUBSCRIBE_TO_CHANNEL:
			{
				subscribeToChannelDescription(taskDescription);
				if(startKey)
				{
					startSubscribeToChannel();
				}
			}
			break;
			case SEND_TO_CHANNEL:
			{
				sendToChannelDescription(taskDescription);
				if(startKey)
				{
					startSendToChannel();
				}
			}
			break;
			case GET_CHANNEL_MEMBERS:
			{
				getChannelMembersDescription(taskDescription);
				if(startKey)
				{
					startGetChannelMembers();
				}
			}
			break;
			case WHISPER_TO_MEMBER:
			{
				whisperToMemberDescription(taskDescription);
				if(startKey)
				{
					startWhisperToMember();
				}
			}
			break;
			case UNSUBSCRIBE_FROM_CHANNEL:
			{
				unsubscribeFromChannelDescription(taskDescription);
				if(startKey)
				{
					startUnsubscribeFromChannel();
				}
			}
			break;
			case LOBBY_DISCONNECT:
			{
				lobbyServiceDisconnectDescription(taskDescription);
				if(startKey)
				{
					startLobbyServiceDisconnect();
				}
			}
			break;
			case STOP_NETWORK:
			{
				stopNetworkDescription(taskDescription);
				if(startKey)
				{
					stopNetwork();
				}
			}
			break;
		}
		
		if(m_changedScreen)
		{
			m_screenText += taskDescription;
			m_screenText += "\n" START_TEXT;
			m_screenText += "\n" SKIP_TEXT;

			if(m_lastTaskDescription.getLength())
			{
				m_screenText += "\n\n";
				m_screenText += m_lastTaskDescription;				
			}
		}
	}
}

void Simulation::pumpTask()
{
	bdString taskDescription("Unknown task");

	switch(m_task)
	{
		case START_NETWORK:
		{
			startNetworkDescription(taskDescription);
			pumpStartNetwork();
		}
		break;
		case CHECK_DNS:
		{
			checkDNSDescription(taskDescription);
			pumpCheckDNS();
		}
		break;
	case CREATE_PLAYER1:
		{
			createPlayer1Description(taskDescription);
			pumpCreatePlayer1();
		}
		break;
		case CREATE_PLAYER2:
		{
			createPlayer2Description(taskDescription);
			pumpCreatePlayer2();
		}
		break;
		case AUTHENTICATE_PLAYER1:
		{
			authenticatePlayer1Description(taskDescription);
			pumpAuthenticatePlayer1();
		}
		break;
		case AUTHENTICATE_PLAYER2:
		{
			authenticatePlayer2Description(taskDescription);
			pumpAuthenticatePlayer2();
		}
		break;
		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(taskDescription);
			pumpLobbyServiceConnect();
		}
		break;
		case GET_CHANNEL_LIST:
		{
			getChannelListDescription(taskDescription);
			pumpGetChannelList();
		}
		break;
		case CREATE_CHANNEL:
		{
			getCreateChannelDescription(taskDescription);
			pumpCreateChannel();
		}
		break;
		case SUBSCRIBE_TO_CHANNEL:
		{
			subscribeToChannelDescription(taskDescription);
			pumpSubscribeToChannel();
		}
		break;
		case SEND_TO_CHANNEL:
		{
			sendToChannelDescription(taskDescription);
			pumpSendToChannel();
		}
		break;
		case GET_CHANNEL_MEMBERS:
		{
			getChannelMembersDescription(taskDescription);
			pumpGetChannelMembers();
		}
		break;
		case WHISPER_TO_MEMBER:
		{
			whisperToMemberDescription(taskDescription);
			pumpWhisperToMember();
		}
		break;
		case UNSUBSCRIBE_FROM_CHANNEL:
		{
			unsubscribeFromChannelDescription(taskDescription);
			pumpUnsubscribeFromChannel();
		}
		break;
		case LOBBY_DISCONNECT:
		{
			lobbyServiceDisconnectDescription(taskDescription);
			pumpLobbyServiceDisconnect();
		}
		break;
		case STOP_NETWORK:
		{
			// nothing to to here
		}
		break;
	}

	if(m_changedScreen)
	{
		m_screenText += taskDescription;
	}
}

void Simulation::displayTaskResult()
{
	bdString taskDescription("Unknown task result");

	Task nextTaskIfDone = m_task;
	Task nextTaskIfFailed = m_task;
	Task nextTaskIfSkipped = m_task;

	bdBool displayLobbyError = true;

	switch(m_task)
	{
		case START_NETWORK:
		{
			startNetworkDescription(m_lastTaskDescription);
			nextTaskIfDone = CHECK_DNS;
			nextTaskIfFailed = START_NETWORK;
			nextTaskIfSkipped = START_NETWORK;
			displayLobbyError = false;
		}
		break;
		case CHECK_DNS:
		{
			checkDNSDescription(m_lastTaskDescription);
			nextTaskIfDone = CREATE_PLAYER1;
			nextTaskIfFailed = CHECK_DNS;
			nextTaskIfSkipped = CREATE_PLAYER1;
			displayLobbyError = false;
		}
		break;
		case CREATE_PLAYER1:
		{
			createPlayer1Description(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = AUTHENTICATE_PLAYER1;
			nextTaskIfFailed = CREATE_PLAYER1;
			nextTaskIfSkipped = CREATE_PLAYER2;			
		}
		break;
		case CREATE_PLAYER2:
		{
			createPlayer1Description(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = AUTHENTICATE_PLAYER2;
			nextTaskIfFailed = CREATE_PLAYER2;
			nextTaskIfSkipped = AUTHENTICATE_PLAYER1;			
		}
		break;
		case AUTHENTICATE_PLAYER1:
		{
			authenticatePlayer1Description(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = LOBBY_CONNECT;
			nextTaskIfFailed = AUTHENTICATE_PLAYER1;
			nextTaskIfSkipped = AUTHENTICATE_PLAYER2;			
		}
		break;		
		case AUTHENTICATE_PLAYER2:
		{
			authenticatePlayer2Description(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = LOBBY_CONNECT;
			nextTaskIfFailed = AUTHENTICATE_PLAYER2;
			nextTaskIfSkipped = LOBBY_CONNECT;			
		}
		break;
		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_CHANNEL_LIST;
			nextTaskIfFailed = LOBBY_CONNECT;
			nextTaskIfSkipped = GET_CHANNEL_LIST;
		}
		break;
		case GET_CHANNEL_LIST:
		{
			getChannelListDescription(m_lastTaskDescription);
			nextTaskIfDone = CREATE_CHANNEL;
			nextTaskIfFailed = GET_CHANNEL_LIST;
			nextTaskIfSkipped = CREATE_CHANNEL;
		}
		break;
		case CREATE_CHANNEL:
		{
			getChannelListDescription(m_lastTaskDescription);
			nextTaskIfDone = SUBSCRIBE_TO_CHANNEL;
			nextTaskIfFailed = CREATE_CHANNEL;
			nextTaskIfSkipped = SUBSCRIBE_TO_CHANNEL;
		}
		break;
		case SUBSCRIBE_TO_CHANNEL:
		{
			subscribeToChannelDescription(m_lastTaskDescription);
			nextTaskIfDone = SEND_TO_CHANNEL;
			nextTaskIfFailed = SUBSCRIBE_TO_CHANNEL;
			nextTaskIfSkipped = SEND_TO_CHANNEL;
		}
		break;
		case SEND_TO_CHANNEL:
		{
			sendToChannelDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_CHANNEL_MEMBERS;
			nextTaskIfFailed = SEND_TO_CHANNEL;
			nextTaskIfSkipped = GET_CHANNEL_MEMBERS;
		}
		break;
		case GET_CHANNEL_MEMBERS:
		{
			getChannelMembersDescription(m_lastTaskDescription);
			nextTaskIfDone = WHISPER_TO_MEMBER;
			nextTaskIfFailed = GET_CHANNEL_MEMBERS;
			nextTaskIfSkipped = WHISPER_TO_MEMBER;
		}
		break;
		case WHISPER_TO_MEMBER:
		{
			whisperToMemberDescription(m_lastTaskDescription);
			nextTaskIfDone = UNSUBSCRIBE_FROM_CHANNEL;
			nextTaskIfFailed = WHISPER_TO_MEMBER;
			nextTaskIfSkipped = UNSUBSCRIBE_FROM_CHANNEL;
		}
		break;
		case UNSUBSCRIBE_FROM_CHANNEL:
		{
			unsubscribeFromChannelDescription(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_DISCONNECT;
			nextTaskIfFailed = UNSUBSCRIBE_FROM_CHANNEL;
			nextTaskIfSkipped = LOBBY_DISCONNECT;
		}
		break;
		case LOBBY_DISCONNECT:
		{
			lobbyServiceDisconnectDescription(m_lastTaskDescription);			
			nextTaskIfDone = STOP_NETWORK;
			nextTaskIfFailed = LOBBY_DISCONNECT;
			nextTaskIfSkipped = STOP_NETWORK;
		}
		break;
		case STOP_NETWORK:
		{
			stopNetworkDescription(m_lastTaskDescription);
			nextTaskIfDone = START_NETWORK;
			nextTaskIfFailed = STOP_NETWORK;
			nextTaskIfSkipped = CREATE_PLAYER1;
		}
		break;
	}

	switch(m_taskStatus)
	{
		case READY:
		case PENDING:
		{
			BD_WARN(BD_LOG_LEVEL, "Unexpected task status.");
		}
		break;
		case DONE:
		{
			m_task = nextTaskIfDone;
		}
		break;
		case FAILED:
		{
			m_task = nextTaskIfFailed;
		}
		break;
		case SKIPPED:
		{
			m_task = nextTaskIfSkipped;
		}
		break;
	}

	// if task uses sets a lobby error code and has failed
	// display the error description
	if(displayLobbyError && (m_taskStatus == FAILED))
	{
		bdNChar8 buf[32];
		LobbyErrorCodeToString(m_lobbyErrorCode, buf, sizeof(buf));
		m_lastTaskDescription += "\nError: ";
		m_lastTaskDescription += buf;
		m_lobbyErrorCode = BD_NO_ERROR;
	}

	m_taskStatus = READY;
}

void Simulation::startNetworkDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Start Network",				// READY
											"Starting Network",				// PENDING
											"Started Network",				// DONE
											"Starting Network Failed",		// FAILED
											"Starting Network Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startNetwork()
{
	// fail safe
	m_taskStatus = FAILED;
	bdNetImpl *const net = bdNet::getInstance();
	if(net)
	{
		bdNetStartParams params;
		// platform specific startup parameters
		bdPlatformParams platformParams;
		const bdInt initRes = bdExamplePlatformInit::init(params, &platformParams);
		if (0 == initRes)
		{
			if(net->start(params))
			{
				m_taskStatus = PENDING;
			}
		}
	}
}

void Simulation::pumpStartNetwork()
{
	// fail safe
	m_taskStatus = FAILED;

	bdNetImpl *const net = bdNet::getInstance();
	if(net)
	{
		net->pump();

		const bdNetImpl::bdNetStatus networkStatus = net->getStatus();
		switch(networkStatus)
		{
			case bdNetImpl::BD_NET_BIND_FAILED:
			case bdNetImpl::BD_NET_ONLINE_FAILED:
			case bdNetImpl::BD_NET_INIT_FAILED:
			case bdNetImpl::BD_NET_STOPPED:
			case bdNetImpl::BD_NET_PARAMS_CONFIG_ERROR:
			{
				// error conditions
				m_taskStatus = FAILED;
			}
			break;
			case bdNetImpl::BD_NET_PENDING:
			{
				// wait
				m_taskStatus = PENDING;
			}
			break;
			case bdNetImpl::BD_NET_DONE:
			{
				// complete
				m_taskStatus = DONE;
			}
		}
	}
}

void Simulation::checkDNSDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Start DNS Lookup",			// READY
											"Performing DNS Lookup",	// PENDING
											"DNS Lookup Succeeded",		// DONE
											"DNS Lookup Failed",		// FAILED
											"DNS Lookup Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == DONE)
	{
		bdNChar8 addrString[BD_INET_ADDR_STRING_SIZE];
		m_authAddr.toString(addrString, sizeof(addrString));
		description +=  "\n " AUTH_ADDRESS ":";
		description += addrString;
		description +=  "\n";

		m_lsgAddr.toString(addrString, sizeof(addrString));
		description +=  " " LSG_ADDRESS ":";
		description += addrString;
	}

}		

void Simulation::startCheckDNS()
{
	const TaskStatus authDNSStatus = StartGetHostByName(AUTH_ADDRESS, &m_authGetHostByName, m_authAddr);
	const TaskStatus lsgDNSStatus = StartGetHostByName(LSG_ADDRESS, &m_lsgGetHostByName, m_lsgAddr);
	if((authDNSStatus == FAILED) || (lsgDNSStatus == FAILED))
	{
		m_taskStatus = FAILED;
	}

	if((authDNSStatus == PENDING) || (lsgDNSStatus == PENDING))
	{
		m_taskStatus = PENDING;
	}

	if((authDNSStatus == DONE) && (lsgDNSStatus == DONE))
	{
		m_taskStatus = DONE;
	}
}

void Simulation::pumpCheckDNS()
{
	TaskStatus authDNSStatus = DONE;
	TaskStatus lsgDNSStatus = DONE;

	if(m_authGetHostByName)
	{
		authDNSStatus = PumpGetHostByName(&m_authGetHostByName, m_authAddr);
	}

	if(m_lsgGetHostByName)
	{
		lsgDNSStatus = PumpGetHostByName(&m_lsgGetHostByName, m_lsgAddr);
	}

	if((authDNSStatus == FAILED) || (lsgDNSStatus == FAILED))
	{
		m_taskStatus = FAILED;
	}

	if((authDNSStatus == PENDING) || (lsgDNSStatus == PENDING))
	{
		m_taskStatus = PENDING;
	}

	if((authDNSStatus == DONE) && (lsgDNSStatus == DONE))
	{
		m_taskStatus = DONE;
	}
}

bdAuthService* Simulation::getAuthService()
{
	if(m_authService == BD_NULL)
	{
		const bdAddr authAddr(m_authAddr, AUTH_PORT);
		m_authService = new bdAuthService(TITLE_ID, authAddr);
	}

	return m_authService;
}

const bdAuthInfo *Simulation::getAuthInfo()
{
	if(m_authService != BD_NULL)
	{
		return m_authService->getAuthInfo();
	}

	return BD_NULL;
}

void Simulation::checkAuthServiceError()
{
	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		m_lobbyErrorCode = authService->getErrorCode();
	}
}

void Simulation::pumpAuthServiceTask()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		m_taskStatus = PENDING;
		const bdAuthService::bdStatus status = authService->getStatus();
		if(status == bdAuthService::BD_READY)
		{			
			m_lobbyErrorCode = authService->getErrorCode();
			if(m_lobbyErrorCode == BD_AUTH_NO_ERROR)
			{
				m_taskStatus = DONE;
			}
			else if (BD_AUTH_CREATE_USERNAME_EXISTS == m_lobbyErrorCode )
			{
				// If we find that the username already exists then we should continue
				// as if things are happy.
				m_taskStatus = DONE;
			}
			else
			{
				m_taskStatus = FAILED;
			}
		}
	}
}

void Simulation::createPlayer1Description(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Create Player 1 (" PLAYER1 "/" PASS1 ")",	// READY
		"Creating Account",								// PENDING
		"Created Account",								// DONE
		"Create Account Failed",						// FAILED
		"Create Account Skipped",						// SKIPPED
		"Account Already Exists"};						// EXISTS

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);

	// There is a special condition when creating an account.
	// If the account already exists, then the task is considered done:
	//  however it should display alternative text.
	bdUInt forDisplay = m_taskStatus;
	if((DONE == m_taskStatus) &&(BD_AUTH_CREATE_USERNAME_EXISTS == m_lobbyErrorCode ))
	{
		// The "Account Already Exists" error in above string array.
		forDisplay = 5;
	}
	IndexStrings(descriptions, numDescriptions, forDisplay, description);
}

void Simulation::startCreatePlayer1()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->createAccount(PLAYER1, PASS1))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpCreatePlayer1()
{
	pumpAuthServiceTask();
}

void Simulation::createPlayer2Description(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Create Player 2 (" PLAYER2 "/" PASS2 ")",	// READY
		"Creating Account",							// PENDING
		"Created Account",							// DONE
		"Create Account Failed",					// FAILED
		"Create Account Skipped",					// SKIPPED
		"Account Already Exists"};					// EXISTS

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);

	// There is a special condition when creating an account.
	// If the account already exists, then the task is considered done:
	//  however it should display alternative text.
	bdUInt forDisplay = m_taskStatus;
	if((DONE == m_taskStatus) &&(BD_AUTH_CREATE_USERNAME_EXISTS == m_lobbyErrorCode ))
	{
		// The "Account Already Exists" error in above string array.
		forDisplay = 5;
	}
	IndexStrings(descriptions, numDescriptions, forDisplay, description);
}

void Simulation::startCreatePlayer2()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->createAccount(PLAYER2, PASS2))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpCreatePlayer2()
{
	pumpAuthServiceTask();
}

void Simulation::authenticatePlayer1Description(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Authenticate Account (" PLAYER1 "/" PASS1 ")",	// READY
											"Authenticating Player 1",									// PENDING
											"Authenticated Player 1",									// DONE
											"Authenticate Player 1 Failed",								// FAILED
											"Authenticate Player 1 Skipped"};							// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAuthenticatePlayer1()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();	
	if(authService)
	{
		if(authService->authorizeAccount(PLAYER1, PASS1))
		{
			m_taskStatus = PENDING;
			m_playerNumber = 1; // note which player is being used
		}
	}
}

void Simulation::pumpAuthenticatePlayer1()
{
	pumpAuthServiceTask();
}

void Simulation::authenticatePlayer2Description(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Authenticate Account (" PLAYER2 "/" PASS2 ")",	// READY
		"Authenticating Player 2",									// PENDING
		"Authenticated Player 2",									// DONE
		"Authenticate Player 2 Failed",								// FAILED
		"Authenticate Player 2 Skipped"};							// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAuthenticatePlayer2()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();	
	if(authService)
	{
		if(authService->authorizeAccount(PLAYER2, PASS2))
		{
			m_taskStatus = PENDING;
			m_playerNumber = 2; // note which player is being used
		}
	}
}

void Simulation::pumpAuthenticatePlayer2()
{
	pumpAuthServiceTask();
}

void Simulation::lobbyServiceConnectDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Connect to Lobby Service",				// READY
											"Connecting to Lobby Service",			// PENDING
											"Connected to Lobby Service",			// DONE
											"Connect to Lobby Service Failed",		// FAILED
											"Connect to Lobby Service Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startLobbyServiceConnect()
{
	// fail safe
	m_taskStatus = FAILED;

	if(m_lobbyService)
	{
		const bdAddr lsgAddr(m_lsgAddr, LSG_PORT);
		const bdBool connecting = m_lobbyService->connect(lsgAddr, getAuthInfo());
		if(connecting)
		{
			m_taskStatus = PENDING;
		}
		// no need to keep the bdAuthService instance after we are connected to the lobby
		delete m_authService;
		m_authService = BD_NULL;
	}
}

void Simulation::pumpLobbyServiceConnect()
{
	// fail safe
	m_taskStatus = FAILED;

	m_pumpLobbyConnection = false;

	if(m_lobbyService)
	{
		m_lobbyService->pump();
		const bdLobbyConnection::Status connectionStatus = m_lobbyService->getStatus();
		switch(connectionStatus)
		{			
			case bdConnection::BD_NOT_CONNECTED:
			case bdConnection::BD_DISCONNECTING:
			case bdConnection::BD_DISCONNECTED:
			{
				m_taskStatus = FAILED;
			}
			break;
			case bdConnection::BD_CONNECTING:
			{
				m_taskStatus = PENDING;
			}
			break;
			case bdConnection::BD_CONNECTED:
			{
				m_taskStatus = DONE;
				m_pumpLobbyConnection = true;
			}
			break;			
		}
	}
}

void Simulation::getChannelListDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Channel List",				// READY
		"Getting Channel List",			// PENDING
		"Got Channel List",				// DONE
		"Get Channel List Failed",		// FAILED
		"Get Channel List Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == DONE)
	{
		if (m_numChatChannels < 1)
		{
			description += "\n No public channels found; example may not function correctly!";
		}
		else
		{
			description += "\nChannel list:";
			for (bdUInt i =0; i<m_numChatChannels; i++)
			{
				bdNChar8 channelID[30];
				bdMemset(channelID,0,30);
				sprintf( channelID, "\nChannel ID:  " BD_UINT_64_FORMAT , m_channelInfos[i].m_channelID);
				description += channelID;
			}
		}
	}
}

void Simulation::startGetChannelList()
{
	// fail safe
	m_taskStatus = FAILED;

	bdMessaging *messaging = m_lobbyService->getMessaging();	
	if(messaging)
	{
		m_remoteTask = messaging->getChannelList(m_channelInfos, MAX_NUM_CHANNELS);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetChannelList()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		m_numChatChannels = m_remoteTask->getNumResults();
	}
	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}


void Simulation::getCreateChannelDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Create Chat Channel",				// READY
											"Creating  Channel",			// PENDING
											"Created Channel",			// DONE
											"Crate Chat Channel Failed",		// FAILED
											"Create Chat to Channel Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}

void Simulation::startCreateChannel()
{
		// fail safe
	m_taskStatus = FAILED;

	bdMessaging *messaging = m_lobbyService->getMessaging();	
	if(messaging)
	{
		m_remoteTask = messaging->createChatChannel(&m_myChannelInfo);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpCreateChannel()
{
	pumpGenericChannelChatTask();
}


void Simulation::subscribeToChannelDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Subscribe to Channel",				// READY
											"Subscribing to Channel",			// PENDING
											"Subscribed to Channel",			// DONE
											"Subscribe to Channel Failed",		// FAILED
											"Subscribe to Channel Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == FAILED && m_subscribedChannels.getSize() == 0)
	{
		description += "\nNo channels to subscribe to.";
	}
}

void Simulation::startSubscribeToChannel()
{
	// fail safe
	m_taskStatus = FAILED;

	bdMessaging *messaging = m_lobbyService->getMessaging();	
	if(messaging)
	{
		bdUInt64 channelID = m_channelInfos[0].m_channelID;
		m_remoteTask = messaging->subscribeToChannel(channelID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpSubscribeToChannel()
{
	pumpGenericChannelChatTask();
}

void Simulation::sendToChannelDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Send to Channel",				// READY
											"Sending to Channel",			// PENDING
											"Sent to Channel",				// DONE
											"Send to Channel Failed",		// FAILED
											"Send to Channel Skipped"};		// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == FAILED && m_subscribedChannels.getSize() == 0)
	{
		description += "\nNot subscribed to any channels.";
	}
}

void Simulation::startSendToChannel()
{
	// fail safe
	m_taskStatus = FAILED;

	bdMessaging * messaging = m_lobbyService->getMessaging();	
	if(messaging)
	{		
		bdNChar8 str[128];
		bdStrlcpy(str, "Example message", sizeof(str));		
		const bdUInt strLength = bdStrnlen(str, sizeof(str));

		bdUInt64 channelID = m_channelInfos[0].m_channelID;
		m_remoteTask = messaging->sendToChannel( channelID, str, strLength);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpSendToChannel()
{
	pumpGenericChannelChatTask();
}

void Simulation::getChannelMembersDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Channel Members",				// READY
											"Getting Channel Members",			// PENDING
											"Got Channel Members",				// DONE
											"Get Channel Members Failed",		// FAILED
											"Get Channel Members Skipped"};		// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}

void Simulation::startGetChannelMembers()
{
	// fail safe
	m_taskStatus = FAILED;

	bdMessaging *messaging = m_lobbyService->getMessaging();	
	if(messaging)
	{
		m_remoteTask = messaging->getChannelMembers( m_channelInfos[0].m_channelID, m_channelMembers, MAX_NUM_DISPLAY_MEMBERS);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetChannelMembers()
{
	pumpGenericChannelChatTask();
}

void Simulation::whisperToMemberDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Whisper to Member",			// READY
											"Whispering to Member",			// PENDING
											"Whispered to Member",			// DONE
											"Whisper to Member Failed",		// FAILED
											"Whisper to Member Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == FAILED)
	{
		if(m_channelMembers[0].m_userID == 0)
		{
			description += "\nNo members to whisper to.";
		}

		if(m_channelInfos[0].m_channelID == 0)
		{
			description += "\nNot subscribed to any channels.";
		}
	}
}

void Simulation::startWhisperToMember()
{
	// fail safe
	m_taskStatus = FAILED;

	bdMessaging *messaging = m_lobbyService->getMessaging();	
	
	if(messaging )
	{		
		// Choose a userID that is not the one this instance is connected with
		bdOnlineUserID localUserID;
		if (m_playerNumber == 1)
		{
			localUserID = bdAuthUtility::getUserID(PLAYER1);	
		}
		else
		{
			localUserID = bdAuthUtility::getUserID(PLAYER2);	
		}
		bdBool found = false;
		for(bdUInt i=0; !found && (i < m_numChannelMembers); i++)
		{
			bdOnlineUserID userID = m_channelMembers[i].m_userID;
			found = (userID != localUserID);
			if(found)
			{			
				bdNChar8 str[] = "Whisper message";

				// assumes m_channelList[0] corresponds to m_memberList
				m_remoteTask = messaging->whisperToChannelMember(	m_channelInfos[0].m_channelID,
																	userID,
																	str, 
																	sizeof(str));
				m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
			}
		}		
	}
}

void Simulation::pumpWhisperToMember()
{
	pumpGenericChannelChatTask();
}

void Simulation::unsubscribeFromChannelDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Unsubscribe from Channel",				// READY
											"Unsubscribing from Channel",			// PENDING
											"Unsubscribed from Channel",			// DONE
											"Unsubscribe from Channel Failed",		// FAILED
											"Unsubscribe from Channel Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if((m_taskStatus == FAILED) && m_subscribedChannels.isEmpty())
	{
		description += "\nNot subscribed to any channels.";
	}
}

void Simulation::startUnsubscribeFromChannel()
{
	// fail safe
	m_taskStatus = FAILED;

	bdMessaging *messaging = m_lobbyService->getMessaging();	
	if(messaging)
	{
		m_remoteTask = messaging->unsubscribeFromChannel(m_channelInfos[0].m_channelID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpUnsubscribeFromChannel()
{
	pumpGenericChannelChatTask();
}

void Simulation::lobbyServiceDisconnectDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Disconnect from Lobby Service",			// READY
											"Disconnecting from Lobby Service",			// PENDING
											"Disconnected from Lobby Service",			// DONE
											"Disconnect from Lobby Service Failed",		// FAILED
											"Disconnect from Lobby Service Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startLobbyServiceDisconnect()
{
	// fail safe
	m_taskStatus = FAILED;

	if(m_lobbyService)
	{
		m_lobbyService->disconnect();
		m_taskStatus = PENDING;
	}
}

void Simulation::pumpLobbyServiceDisconnect()
{
	// fail safe
	m_taskStatus = FAILED;
	if(m_lobbyService)
	{
		const bdLobbyConnection::Status status = m_lobbyService->getStatus();
		switch(status)
		{
			case bdConnection::BD_DISCONNECTED:
			case bdConnection::BD_NOT_CONNECTED:
			{
				m_taskStatus = DONE;
			}
			break;
			case bdConnection::BD_CONNECTING:
			case bdConnection::BD_CONNECTED:
			{
				m_taskStatus = FAILED;
			}
			break;
			case bdConnection::BD_DISCONNECTING:
			{
				m_taskStatus = PENDING;
			}
			break;
		}
	}
}

void Simulation::stopNetworkDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Stop Network",					// READY
											"Stopping Network",				// PENDING
											"Stopped Network",				// DONE
											"Stopping Network Failed",		// FAILED
											"Stopping Network Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::stopNetwork()
{
	// fail safe
	m_taskStatus = FAILED;

	bdNetImpl *const net = bdNet::getInstance();
	if(net)
	{
		net->stop();
		if(net->getStatus() == bdNetImpl::BD_NET_STOPPED)
		{
			bdExamplePlatformInit::quit();
			m_taskStatus = DONE;
		}		
	}
}

void Simulation::pumpGenericChannelChatTask()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);


	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}


