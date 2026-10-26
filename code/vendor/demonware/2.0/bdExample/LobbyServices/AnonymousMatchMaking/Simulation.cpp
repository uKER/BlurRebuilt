// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include "MatchMakingQueries.h"


#define TITLE_ID (0x1409)

#define INITIAL_GAME_TYPE (1)
#define CHANGED_GAME_TYPE (2)


Simulation::Simulation()
: m_taskStatus(READY),
  m_task(START_NETWORK),
  m_changedScreen(true),
  m_authGetHostByName(BD_NULL),
  m_authService(BD_NULL),
  m_lobbyService(BD_NULL),
  m_lsgGetHostByName(BD_NULL),
  m_remoteTask(BD_NULL),
  m_pumpLobbyConnection(false),
  m_lobbyErrorCode(BD_NO_ERROR)
{
}

Simulation::~Simulation()
{
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
	m_lobbyService->registerEventHandler(this);
	return ok;
}


void Simulation::onNotifyLeave(bdOnlineUserID userID, bdSessionID secID)
{
	bdPrintf("User disconnected from matchmaking service\n");
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
//					m_lobbyService->getMessaging()->registerEventHandler(this);
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
			CloseRemoteTask(m_remoteTask);
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

	stopNetwork();

	delete m_authGetHostByName;
	delete m_lsgGetHostByName;
	delete m_lobbyService;

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
			case ADVERTISE_SESSION:
			{
				advertiseSessionDescription(taskDescription);
				if(startKey)
				{
					startAdvertiseSession();
				}
			}
			break;
			case FIND_SESSION_BY_ID:
			{
				findSessionByIDDescription(taskDescription);
				if(startKey)
				{
					startFindSessionByID();
				}
			}
			break;
			case UPDATE_SESSION:
			{
				updateSessionDescription(taskDescription);
				if(startKey)
				{
					startUpdateSession();
				}
			}
			break;
			case FIND_SESSIONS_BY_GAME_TYPE:
			{
				findSessionsByGameTypeDescription(taskDescription);
				if(startKey)
				{
					startFindSessionsByGameType();
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
		case ADVERTISE_SESSION:
		{
			advertiseSessionDescription(taskDescription);
			pumpAdvertiseSession();
		}
		break;
		case FIND_SESSION_BY_ID:
		{
			findSessionByIDDescription(taskDescription);
			pumpFindSessionByID();
		}
		break;
		case UPDATE_SESSION:
		{
			updateSessionDescription(taskDescription);
			pumpUpdateSession();
		}
		break;
		case FIND_SESSIONS_BY_GAME_TYPE:
		{
			findSessionsByGameTypeDescription(taskDescription);
			pumpFindSessionsByGameType();
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
			nextTaskIfDone = AUTHENTICATE_ACCOUNT;
			nextTaskIfFailed = CHECK_DNS;
			nextTaskIfSkipped = AUTHENTICATE_ACCOUNT;
			displayLobbyError = false;
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
			nextTaskIfDone = ADVERTISE_SESSION;
			nextTaskIfFailed = LOBBY_CONNECT;
			nextTaskIfSkipped = ADVERTISE_SESSION;
		}
		break;
		case ADVERTISE_SESSION:
		{
			advertiseSessionDescription(m_lastTaskDescription);
			nextTaskIfDone = FIND_SESSION_BY_ID;
			nextTaskIfFailed = ADVERTISE_SESSION;
			nextTaskIfSkipped = FIND_SESSION_BY_ID;
		}
		break;
		case FIND_SESSION_BY_ID:
		{
			findSessionByIDDescription(m_lastTaskDescription);
			nextTaskIfDone = UPDATE_SESSION;
			nextTaskIfFailed = FIND_SESSION_BY_ID;
			nextTaskIfSkipped = UPDATE_SESSION;
		}
		break;
		case UPDATE_SESSION:
		{
			updateSessionDescription(m_lastTaskDescription);
			nextTaskIfDone = FIND_SESSIONS_BY_GAME_TYPE;
			nextTaskIfFailed = UPDATE_SESSION;
			nextTaskIfSkipped = FIND_SESSIONS_BY_GAME_TYPE;
		}
		break;
		case FIND_SESSIONS_BY_GAME_TYPE:
		{
			findSessionsByGameTypeDescription(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_DISCONNECT;
			nextTaskIfFailed = FIND_SESSIONS_BY_GAME_TYPE;
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
			nextTaskIfSkipped = START_NETWORK;
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
			else
			{
				m_taskStatus = FAILED;
			}
		}
	}
}


void Simulation::authenticateAccountDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Authenticate Anonymous",	// READY
											"Authenticating Anonymous",									// PENDING
											"Authenticated Anonymous",									// DONE
											"Authenticate Anonymous Failed",								// FAILED
											"Authenticate Anonymous Skipped"};							// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAuthenticateAccount()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->authorizeAnonymous())
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

	// Connect to the Lobby Server
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


void Simulation::printSessionList(bdString& string)
{
	bdNChar8 buf[32];
	bdSnprintf(buf, sizeof(buf), "%u session(s) found:\n", m_numSessions);
	string += buf;
	for (bdUInt i = 0; i < m_numSessions; i++)
	{
		m_foundSessions.add(m_sessionInfos[i]);
	}
	m_foundSessions.print(string);
}

void Simulation::advertiseSessionDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Advertise Session",			// READY
											"Advertising Session",			// PENDING
											"Advertised Session",			// DONE
											"Advertise Session Failed",		// FAILED
											"Advertise Session Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAdvertiseSession()
{
	// fail safe
	m_taskStatus = FAILED;

	bdNetImpl *const net = bdNet::getInstance();
	bdMatchMaking *const matchMaking = m_lobbyService->getMatchMaking();

	if(matchMaking && net)
	{
		// the session details
		const bdNChar8 *const sessionName = "AnonymousGame";
		const bdCommonAddrRef hostAddress = net->getLocalCommonAddr();

		m_session.setHostAddr(hostAddress);
		m_session.m_gameType = INITIAL_GAME_TYPE;
		m_session.m_maxPlayers = 10;
		m_session.setSessionName(sessionName);
		m_session.m_levelNum = 1;
		// send to the lobby service
		m_remoteTask = matchMaking->createSession(&m_session, &m_sessionID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpAdvertiseSession()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}



void Simulation::findSessionByIDDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Find Session by Security ID",			// READY
											"Finding Session by Security ID",		// PENDING
											"Find Session by Security ID Complete",	// DONE
											"Find Session by Security ID Failed",	// FAILED
											"Find Session by Security ID Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == DONE)
	{
		description += "\n";
		printSessionList(description);
	}
}

void Simulation::startFindSessionByID()
{
	// fail safe
	m_taskStatus = FAILED;
	m_numSessions = 0;
	m_foundSessions.clear();
	bdMatchMaking *const matchMaking = m_lobbyService->getMatchMaking();
	if(matchMaking)
	{
		m_remoteTask = matchMaking->findSessionFromID(m_sessionID,
													  &m_sessionInfos[0]);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpFindSessionByID()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == DONE)
	{
		m_numSessions = m_remoteTask->getNumResults();
	}
	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::updateSessionDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Update Session",			// READY
											"Updating Session",			// PENDING
											"Updated Session",			// DONE
											"Update Session Failed",	// FAILED
											"Update Session Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startUpdateSession()
{
	// fail safe
	m_taskStatus = FAILED;

	bdMatchMaking *const matchMaking = m_lobbyService->getMatchMaking();
	if(matchMaking)
	{
		// change the game type
		m_session.m_gameType = CHANGED_GAME_TYPE;

		m_remoteTask = matchMaking->updateSession(m_sessionID, &m_session);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpUpdateSession()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::findSessionsByGameTypeDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Find Sessions by Game Type",			// READY
											"Finding Sessions by Game Type",		// PENDING
											"Found Sessions by Game Type",			// DONE
											"Find Session by Game Type Failed",		// FAILED
											"Find Session by Game Type Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == DONE)
	{
		description += "\n";
		printSessionList(description);
	}
}

void Simulation::startFindSessionsByGameType()
{
	// fail safe
	m_taskStatus = FAILED;
	m_numSessions = 0;
	m_foundSessions.clear();
	bdMatchMaking *const matchMaking = m_lobbyService->getMatchMaking();
	if(matchMaking)
	{
		FindSessionsByGameType query;
		query.m_gameType = CHANGED_GAME_TYPE;
		bdUInt gueryID = 1;
		m_remoteTask = matchMaking->findSessions(gueryID,
												 0,
												 MAX_NUM_SESSIONS,
												 &query,
												 m_sessionInfos);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpFindSessionsByGameType()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		m_numSessions = m_remoteTask->getNumResults();
	}

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
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

