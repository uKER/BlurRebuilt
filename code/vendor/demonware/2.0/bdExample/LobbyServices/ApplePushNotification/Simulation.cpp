// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"

#define TITLE_ID (0x7B)

#define USERNAME "APNsExample"
#define PASSWORD "MyPassword"

#define PAYLOAD "{\"aps\":{\"alert\":\"DemonWare push notification!\"}}"


Simulation::Simulation()
: m_taskStatus(READY),
m_task(START_NETWORK),
m_changedScreen(true),
m_authGetHostByName(BD_NULL),
m_authService(BD_NULL),
m_lsgGetHostByName(BD_NULL),
m_remoteTask(BD_NULL),
m_pumpLobbyConnection(false),
m_lobbyErrorCode(BD_NO_ERROR)
{
}

Simulation::~Simulation()
{
}

bdBool Simulation::init(bdInt32, bdNChar8* [])
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
		// incase the task fails or completes in a single loop
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
		m_lobbyService->pump();
		if(m_lobbyService)
		{
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
			m_lastTaskDescription = "Lost connection to lobby.";
		}
	}
	
	getScreen().printToScreen(m_screenText);
	
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
	
	stopNetwork();
	
	delete m_authGetHostByName;
	delete m_authService;
	delete m_lsgGetHostByName;
	delete m_lobbyService;
	
	m_remoteTask = BD_NULL;
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
			case CREATE_ACCOUNT:
			{
				createAccountDescription(taskDescription);
				if(startKey)
				{
					startCreateAccount();
				}
			}
				break;
			case AUTHENTICATE_ACCOUNT:
			{
				authenticateAccountDescription(taskDescription);
				if(startKey)
				{
					startAuthenticateAccount();
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
			case SET_DEVICE_TOKEN:
			{
				setDeviceTokenDescription(taskDescription);
				if(startKey)
				{
					startSetDeviceToken();
				}
			}
				break;
			case SEND_PUSH_NOTIFICATION:
			{
				sendPushNotificationDescription(taskDescription);
				if(startKey)
				{
					startSendPushNotification();
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
		case CREATE_ACCOUNT:
		{
			createAccountDescription(taskDescription);
			pumpCreateAccount();
		}
			break;
		case AUTHENTICATE_ACCOUNT:
		{
			authenticateAccountDescription(taskDescription);
			pumpAuthenticateAccount();
		}
			break;
		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(taskDescription);
			pumpLobbyServiceConnect();
		}
			break;
		case SET_DEVICE_TOKEN:
		{
			setDeviceTokenDescription(taskDescription);
			pumpSetDeviceToken();
		}
			break;
		case SEND_PUSH_NOTIFICATION:
		{
			sendPushNotificationDescription(taskDescription);
			pumpSendPushNotification();
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
			nextTaskIfDone = CREATE_ACCOUNT;
			nextTaskIfFailed = CHECK_DNS;
			nextTaskIfSkipped = CREATE_ACCOUNT;
			displayLobbyError = false;
		}
			break;
		case CREATE_ACCOUNT:
		{
			createAccountDescription(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = AUTHENTICATE_ACCOUNT;
			nextTaskIfFailed = CREATE_ACCOUNT;
			nextTaskIfSkipped = AUTHENTICATE_ACCOUNT;
		}
			break;
		case AUTHENTICATE_ACCOUNT:
		{
			authenticateAccountDescription(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = LOBBY_CONNECT;
			nextTaskIfFailed = AUTHENTICATE_ACCOUNT;
			nextTaskIfSkipped = LOBBY_CONNECT;
		}
			break;
		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(m_lastTaskDescription);
			nextTaskIfDone = SET_DEVICE_TOKEN;
			nextTaskIfFailed = LOBBY_CONNECT;
			nextTaskIfSkipped = SET_DEVICE_TOKEN;
		}
			break;
		case SET_DEVICE_TOKEN:
		{
			setDeviceTokenDescription(m_lastTaskDescription);
			checkLobbyError();
			nextTaskIfDone = SEND_PUSH_NOTIFICATION;
			nextTaskIfFailed = SET_DEVICE_TOKEN;
			nextTaskIfSkipped = SEND_PUSH_NOTIFICATION;
		}
			break;
		case SEND_PUSH_NOTIFICATION:
		{
			sendPushNotificationDescription(m_lastTaskDescription);
			checkLobbyError();
			nextTaskIfDone = LOBBY_DISCONNECT;
			nextTaskIfFailed = SEND_PUSH_NOTIFICATION;
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
			nextTaskIfSkipped = CREATE_ACCOUNT;
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
	const bdNChar8 *const descriptions[] = {
		"Start Network",				// READY
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
	const bdNChar8 *const descriptions[] = {
		"Start DNS Lookup",			// READY
		"Performing DNS Lookup",	// PENDING
		"DNS Lookup Succeeded",		// DONE
		"DNS Lookup Failed",		// FAILED
		"DNS Lookup Skipped"};		// SKIPPED
	
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

void Simulation::createAccountDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {
		"Create Account (" USERNAME "/" PASSWORD ")",	// READY
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
	if((DONE == m_taskStatus) && (BD_AUTH_CREATE_USERNAME_EXISTS == m_lobbyErrorCode))
	{
		// The "Account Already Exists" error in above string array.
		forDisplay = 5;
	}
	IndexStrings(descriptions, numDescriptions, forDisplay, description);
	
}

void Simulation::startCreateAccount()
{
	// fail safe
	m_taskStatus = FAILED;
	
	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->createAccount(USERNAME, PASSWORD))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpCreateAccount()
{
	pumpAuthServiceTask();
}

void Simulation::authenticateAccountDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {
		"Authenticate Account (" USERNAME "/" PASSWORD ")",	// READY
		"Authenticating Account",							// PENDING
		"Authenticated Account",							// DONE
		"Authenticate Account Failed",						// FAILED
		"Authenticate Account Skipped"};					// SKIPPED
	
	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAuthenticateAccount()
{
	// fail safe
	m_taskStatus = FAILED;
	
	bdAuthService *const authService = getAuthService();
	if(authService )
	{
		if(authService->authorizeAccount(USERNAME, PASSWORD))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpAuthenticateAccount()
{
	pumpAuthServiceTask();
}

void Simulation::lobbyServiceConnectDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {
		"Connect to Lobby Service",				// READY
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
		const bdBool connecting = m_lobbyService->connect(lsgAddr, getAuthService()->getAuthInfo());
		if(connecting)
		{
			m_taskStatus = PENDING;
		}
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

void Simulation::checkLobbyError()
{
	if(m_remoteTask)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::setDeviceTokenDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {
		"Set device token",				// READY
		"Setting device token",			// PENDING
		"Device token set",				// DONE
		"Set device token failed",		// FAILED
		"Set device token skipped"};	// SKIPPED
	
	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
	
}

void Simulation::startSetDeviceToken()
{
#ifdef BD_PLATFORM_APPLE
	// Get the device token.
	// This calls through to the app delegate class.
	this->m_deviceTokenSize = this->getDeviceToken(this->m_deviceToken);
	
	bdApplePushNotification *const applePushNotification = m_lobbyService->getApplePushNotification();
	
	if(applePushNotification)
	{
		m_remoteTask = applePushNotification->setDeviceToken(this->m_deviceToken, this->m_deviceTokenSize);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
#endif
}

void Simulation::pumpSetDeviceToken()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
}

void Simulation::sendPushNotificationDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {
		"Send push notification",				// READY
		"Sending push notification",			// PENDING
		"Sent push notification",				// DONE
		"Sending push notification failed",		// FAILED
		"Sending push notification skipped"};	// SKIPPED
	
	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
	
}

void Simulation::startSendPushNotification()
{
	bdApplePushNotification *const applePushNotification = m_lobbyService->getApplePushNotification();
	
	if(applePushNotification)
	{
		bdUInt64 userIDs[1];
		userIDs[0] = bdAuthUtility::getUserID(USERNAME);
		m_remoteTask = applePushNotification->sendPushNotification(PAYLOAD, userIDs, 1);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpSendPushNotification()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
//	if(m_taskStatus == DONE)
//	{
//		for(int i =0; i < MAX_NUM_COUNTVALUES; i++)
//		{
//			BD_INFO(BD_LOG_LEVEL, "The current count for groupID %d is %d.", m_groupCounts[i].m_groupID, m_groupCounts[i].m_groupCount);
//		}
//	}
}

void Simulation::lobbyServiceDisconnectDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {
		"Disconnect from Lobby Service",			// READY
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
		m_lobbyService->pump();
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
	const bdNChar8 *const descriptions[] = {
		"Stop Network",					// READY
		"Stopping Network",				// PENDING
		"Stopped Network",				// DONE
		"Stopping Network Failed",		// FAILED
		"Stopping Network Skipped"};	// SKIPPED
	
	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::stopNetwork()
{
	bdNetImpl *const net = bdNet::getInstance();
	net->stop();
	if(net->getStatus() == bdNetImpl::BD_NET_STOPPED)
	{
		bdExamplePlatformInit::quit();
		m_taskStatus = DONE;
	}
	else
	{
		m_taskStatus = FAILED;
	}
}

