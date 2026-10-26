// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include "MatchMakingQueries.h"



// for MMP3 - use the following TitleIDs
#define GAME_TITLE_ID (0x1398)
#define COMMUNITY_TITLE_ID (0x1399)

#define USERNAME1 "CTExample1"
#define USERNAME2 "CTExample2"
#define PASSWORD "MyPassword"
#define INITIAL_GAME_TYPE (1)

Simulation::Simulation()
: m_usingCommunityConnections(false),
  m_currentAuthService(BD_NULL), 
  m_currentLobbyService(BD_NULL),
  m_commonAuthService(BD_NULL),
  m_commonLobbyService(BD_NULL),
  m_authService(BD_NULL),
  m_lobbyService(BD_NULL),
  m_taskStatus(READY),
  m_task(START_NETWORK),
  m_changedScreen(true),
  m_authGetHostByName(BD_NULL),
  m_lsgGetHostByName(BD_NULL),
  m_remoteTask(BD_NULL),
  m_pumpLobbyConnection(false),
  m_lobbyErrorCode(BD_NO_ERROR)
{
	// By default:
	usePlayerOne();

	// Invite a friend to an imaginary game session whose securityId 
	// is set to 'SECRETI'.
	bdMemcpy(m_sessionSecurityID.ab, static_cast<const void* const>("SET_ME"), BD_SECURITY_ID_LENGTH);
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

	if(m_pumpLobbyConnection && m_lobbyService)
	{
		bdBool lostConnection = true;
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

	if(m_pumpLobbyConnection && m_commonLobbyService)
	{
		bdBool lostConnection = true;
		const bdLobbyConnection::Status status = m_commonLobbyService->getStatus();
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

	getScreen().printToScreen(m_screenText + m_messageText);

	if(m_changedScreen)
	{
		m_screenText = "";
	}

	return !quitKey;
}

void Simulation::quit()
{

	// Ensure the lobby service connection is closed.
	startLobbyServiceDisconnect();
	pumpLobbyServiceDisconnect();

	stopNetwork();


	if(m_authGetHostByName)
	{
		m_authGetHostByName->quit();
		delete m_authGetHostByName;
	}
	if(m_lsgGetHostByName)
	{	
		m_lsgGetHostByName->quit();
		delete m_lsgGetHostByName;
	}

	if(m_authService)
	{
		delete m_authService;
		m_authService = BD_NULL;
	}

	if (m_commonAuthService != BD_NULL)
	{
		delete m_commonAuthService;
		m_commonAuthService = BD_NULL;
	}

	if (m_lobbyService != BD_NULL)
	{
		delete m_lobbyService;
		m_lobbyService = BD_NULL;
	}

	if (m_commonLobbyService != BD_NULL)
	{
		delete m_commonLobbyService;
		m_commonLobbyService = BD_NULL;
	}
}

void Simulation::useGameSpecificLobbyAndAuth()
{
	BD_ASSERT(m_lobbyService != BD_NULL, "m_lobbyService IS NOT CONNECTED" );

	m_currentLobbyService = m_lobbyService;

	m_usingCommunityConnections = false;
}

void Simulation::useCommunityWideLobbyAndAuth()
{
	BD_ASSERT(m_commonLobbyService != BD_NULL, "m_commonLobbyService IS NOT CONNECTED" );

	m_currentLobbyService = m_commonLobbyService;

	m_usingCommunityConnections = true;
}

void Simulation::usePlayerOne()
{
	m_usePlayerOne = true;
}

void Simulation::usePlayerTwo()
{
	m_usePlayerOne = false;
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
			case CHOOSE_PLAYER:
			{
				choosePlayerDescription(taskDescription);
				if(startKey)
				{
					startChoosePlayer();
				}
			}
			break;
			case CREATE_COMMON_ACCOUNT:
				
				m_usingCommunityConnections = true;

			case CREATE_GAME_ACCOUNT:
				{
					createAccountDescription(taskDescription);
					if(startKey)
					{
						startCreateAccount();
					}
				}
				break;
			case AUTHENTICATE_COMMON_ACCOUNT:

				m_usingCommunityConnections = true;

			case AUTHENTICATE_GAME_ACCOUNT:
				{
					authenticateAccountDescription(taskDescription);
					if(startKey)
					{
						startAuthenticateAccount();
					}
				}
				break;
			case GAME_LOBBY_CONNECT:
				{
					// switch state
					m_usingCommunityConnections = false;
					lobbyServiceConnectDescription(taskDescription);
					if(startKey)
					{
						startLobbyServiceConnect();
					}
				}
				break;
			case COMMON_LOBBY_CONNECT:
				{
					// switch state
					m_usingCommunityConnections = true;
					lobbyServiceConnectDescription(taskDescription);
					if(startKey)
					{
						startLobbyServiceConnect();
					}
				}
				break;
			case ADVERTISE_SESSION:
				{
					useGameSpecificLobbyAndAuth();
					
					advertiseSessionDescription(taskDescription);
					if(startKey)
					{
						startAdvertiseSession();
					}
				}
				break;
			case FIND_SESSION_BY_ID:
			{
				useGameSpecificLobbyAndAuth();
				
				findSessionByIDDescription(taskDescription);
				if(startKey)
				{
					startFindSessionByID();
				}
			}
			break;

			case ACCEPT_PROPOSAL:
				{
					useCommunityWideLobbyAndAuth();

					createAcceptProposalDescription(taskDescription);
					if(startKey)
					{
						startAcceptProposal();
					}
				}
				break;
			case GET_FRIENDS:
				{
					useCommunityWideLobbyAndAuth();

					createGetFriendsDescription(taskDescription);
					if(startKey)
					{
						startGetFriends();
					}
				}
				break;
			case PROPOSE_FRIENDSHIP:
				{
					useCommunityWideLobbyAndAuth();

					createProposeFriendshipDescription(taskDescription);
					if(startKey)
					{
						startProposeFriendship();
					}
				}
				break;
			case INVITE_FRIEND:
				{
					useCommunityWideLobbyAndAuth();

					createInviteDescription(taskDescription);
					if (startKey)
					{
						startInvite();
					}
				}
				break;

			case GAME_LOBBY_DISCONNECT:
			case COMMON_LOBBY_DISCONNECT:
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
		case CHOOSE_PLAYER:
		{
			choosePlayerDescription(taskDescription);
			pumpChoosePlayer();
		}
		break;
		case CREATE_COMMON_ACCOUNT:
		case CREATE_GAME_ACCOUNT:
		{
			createAccountDescription(taskDescription);
			pumpCreateAccount();
		}
		break;
		case AUTHENTICATE_COMMON_ACCOUNT:
		case AUTHENTICATE_GAME_ACCOUNT:
		{
			authenticateAccountDescription(taskDescription);
			pumpAuthenticateAccount();
		}
		break;
		case GAME_LOBBY_CONNECT:
		case COMMON_LOBBY_CONNECT:
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
		case GAME_LOBBY_DISCONNECT:
		case COMMON_LOBBY_DISCONNECT:
		{
			lobbyServiceDisconnectDescription(taskDescription);
			pumpLobbyServiceDisconnect();
		}
		break;
		case ACCEPT_PROPOSAL:
			{
				createAcceptProposalDescription(taskDescription);
				pumpAcceptProposal();
			}
			break;
		case GET_FRIENDS:
			{
				createGetFriendsDescription(taskDescription);
				pumpGetFriends();
			}
			break;
		case PROPOSE_FRIENDSHIP:
			{
				createProposeFriendshipDescription(taskDescription);
				pumpProposeFriendship();
			}
			break;
		case INVITE_FRIEND:
			{
				createInviteDescription(taskDescription);
				pumpInvite();
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

Simulation::Task Simulation::determineNextTask(Task currentTask)
{
	Task nextTask = STOP_NETWORK;
	// Inspect the message store to see if there are any proposals or invites
	// outstanding.
	bdUInt numProposals = 0;
	bdUInt numInvites = 0;
	/*
	bdMessageStore *const messageStore = GetMessageStore();
	bdUInt i;
	for(i = 0; i < messageStore->getNumMessages(); i++) 
	{
		if(messageStore->getAt(i)->m_msgType == BD_FRIENDSHIP_PROPOSAL_MSG)
		{
			numProposals++;
		}
		else if(messageStore->getAt(i)->m_msgType == BD_FRIEND_SESSION_INVITATION_MSG)
		{
			numInvites++;
		}
	}
	switch(currentTask)
	{
	case GET_MESSAGES:
		{
			if(numProposals > 0)
			{
				nextTask = ACCEPT_PROPOSAL;
			}
			else if(numInvites > 0)
			{
				nextTask = ACCEPT_INVITATION;
			}
			else
			{
				nextTask = GET_MESSAGES;
			}
		}
		break;
	case ACCEPT_PROPOSAL:
		{
			if(numProposals > 1)
			{
				nextTask = ACCEPT_PROPOSAL;
			}
			else if(numInvites > 0)
			{
				nextTask = ACCEPT_INVITATION;
			}
			else
			{
				nextTask = GET_MESSAGES;
			}
		}
		break;
	case ACCEPT_INVITATION:
		{
			if(numInvites > 1)
			{
				nextTask = ACCEPT_INVITATION;
			}
			else
			{
				nextTask = GET_MESSAGES;
			}
		}
		break;
	default:
		{
			//don't care..
		}
		break;
	}
	*/
	return nextTask;
}


void Simulation::displayTaskResult()
{

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
			nextTaskIfDone = CHOOSE_PLAYER;
			nextTaskIfFailed = CHECK_DNS;
			nextTaskIfSkipped = CHOOSE_PLAYER;
			displayLobbyError = false;
		}
		break;
		case CHOOSE_PLAYER:
		{
			choosePlayerDescription(m_lastTaskDescription);
			nextTaskIfDone = CREATE_GAME_ACCOUNT;
			nextTaskIfFailed = CREATE_GAME_ACCOUNT;
			nextTaskIfSkipped = CREATE_GAME_ACCOUNT;
			displayLobbyError = false;
		}
		break;
		case CREATE_GAME_ACCOUNT:
		{
			m_usingCommunityConnections = false;
			createAccountDescription(m_lastTaskDescription);
			nextTaskIfDone = AUTHENTICATE_GAME_ACCOUNT;
			nextTaskIfFailed = CREATE_GAME_ACCOUNT;
			nextTaskIfSkipped = AUTHENTICATE_GAME_ACCOUNT;

			//m_lobbyErrorCode = m_currentAuthService->getErrorCode();
		}
		break;
		case AUTHENTICATE_GAME_ACCOUNT:
		{
			m_usingCommunityConnections = false;
			authenticateAccountDescription(m_lastTaskDescription);
			nextTaskIfDone = GAME_LOBBY_CONNECT;
			nextTaskIfFailed = AUTHENTICATE_GAME_ACCOUNT;
			nextTaskIfSkipped = GAME_LOBBY_CONNECT;		

			if (m_currentAuthService)
			{
				m_lobbyErrorCode = m_currentAuthService->getErrorCode();
			}
		}
		break;		
		case GAME_LOBBY_CONNECT:
		{
			m_usingCommunityConnections = false;
			lobbyServiceConnectDescription(m_lastTaskDescription);
			nextTaskIfDone = CREATE_COMMON_ACCOUNT;
			nextTaskIfFailed = GAME_LOBBY_CONNECT;
			nextTaskIfSkipped = CREATE_COMMON_ACCOUNT;

		}
		break;

		case CREATE_COMMON_ACCOUNT:
			{
				m_usingCommunityConnections = true;
				createAccountDescription(m_lastTaskDescription);
				nextTaskIfDone = AUTHENTICATE_COMMON_ACCOUNT;
				nextTaskIfFailed = CREATE_COMMON_ACCOUNT;
				nextTaskIfSkipped = AUTHENTICATE_COMMON_ACCOUNT;			
			}
			break;
		case AUTHENTICATE_COMMON_ACCOUNT:
			{
				m_usingCommunityConnections = true;
				authenticateAccountDescription(m_lastTaskDescription);
				nextTaskIfDone = COMMON_LOBBY_CONNECT;
				nextTaskIfFailed = AUTHENTICATE_COMMON_ACCOUNT;
				nextTaskIfSkipped = COMMON_LOBBY_CONNECT;		
				if (m_currentAuthService)
				{
					m_lobbyErrorCode = m_currentAuthService->getErrorCode();
				}
			}
			break;		
		case COMMON_LOBBY_CONNECT:
			{
				m_usingCommunityConnections = true;
				lobbyServiceConnectDescription(m_lastTaskDescription);
				nextTaskIfDone = ADVERTISE_SESSION;
				nextTaskIfFailed = COMMON_LOBBY_CONNECT;
				nextTaskIfSkipped = ADVERTISE_SESSION;
			}
			break;


		case ADVERTISE_SESSION:
			{
				advertiseSessionDescription(m_lastTaskDescription);
				nextTaskIfDone = PROPOSE_FRIENDSHIP;
				nextTaskIfFailed = ADVERTISE_SESSION;
				nextTaskIfSkipped = PROPOSE_FRIENDSHIP;

			}
			break;

		case PROPOSE_FRIENDSHIP:
			{
				createProposeFriendshipDescription(m_lastTaskDescription);
				nextTaskIfDone = GET_FRIENDS;
				nextTaskIfFailed = PROPOSE_FRIENDSHIP;
				nextTaskIfSkipped = GET_FRIENDS;
			}
			break;

		// This state is arrived as a result of an incoming message 
		case ACCEPT_PROPOSAL:
			{
				createAcceptProposalDescription(m_lastTaskDescription);
				nextTaskIfDone = determineNextTask(m_task);
				nextTaskIfFailed = ACCEPT_PROPOSAL;
				nextTaskIfSkipped = GET_FRIENDS;
			}
			break;

		case GET_FRIENDS:
			{
				createGetFriendsDescription(m_lastTaskDescription);
				nextTaskIfDone = INVITE_FRIEND;
				nextTaskIfFailed = GET_FRIENDS;
				nextTaskIfSkipped = INVITE_FRIEND;

			}
			break;

		case INVITE_FRIEND:
			{
				createInviteDescription(m_lastTaskDescription);
				nextTaskIfDone = FIND_SESSION_BY_ID;
				nextTaskIfFailed = INVITE_FRIEND;
				nextTaskIfSkipped = FIND_SESSION_BY_ID;

			}
			break;

		case FIND_SESSION_BY_ID:
			{
				findSessionByIDDescription(m_lastTaskDescription);
				nextTaskIfDone = GAME_LOBBY_DISCONNECT;
				nextTaskIfFailed = FIND_SESSION_BY_ID;
				nextTaskIfSkipped = GAME_LOBBY_DISCONNECT;
			}
			break;

		case GAME_LOBBY_DISCONNECT:
		case COMMON_LOBBY_DISCONNECT:
		{
			lobbyServiceDisconnectDescription(m_lastTaskDescription);			
			nextTaskIfDone = STOP_NETWORK;
			nextTaskIfFailed = GAME_LOBBY_DISCONNECT;
			nextTaskIfSkipped = STOP_NETWORK;
		}
		break;
		case STOP_NETWORK:
		{
			stopNetworkDescription(m_lastTaskDescription);
			nextTaskIfDone = START_NETWORK;
			nextTaskIfFailed = STOP_NETWORK;
			nextTaskIfSkipped = CREATE_GAME_ACCOUNT;
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

void Simulation::createInviteDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
		"Invite Friend to Session",						// READY
			"Inviting Friend to Session",					// PENDING
			"Friend invited to session",					// DONE
			"Invite Friend to Session Failed",				// FAILED
			"Invite Friend to Session Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startInvite()
{
	// fail safe
	m_taskStatus = FAILED;

	bdMatchMaking *const matchmaking = m_lobbyService->getMatchMaking();
	if(matchmaking)
	{
		// Invite a friend to the previously advertised game session 
		
		// invite can include arbitrary attachments
		const bdNChar8 *attachment = "Join my session";
		const bdNChar8 *const otherUserName = (m_playerNumber == 1 ? USERNAME2 : USERNAME1);
		bdOnlineUserID otherID[1];
		otherID[0] = bdAuthUtility::getUserID(otherUserName);

		m_remoteTask = matchmaking->inviteToSession(m_sessionID, otherID, 1, attachment, sizeof(attachment));
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpInvite()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
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
		//TODO: check the return value here
		bdExamplePlatformInit::init(params, &platformParams);
		
		if(net->start(params))
		{
			m_taskStatus = PENDING;
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
			case bdNetImpl::BD_NET_PARAMS_CONFIG_ERROR:
			case bdNetImpl::BD_NET_BIND_FAILED:
			case bdNetImpl::BD_NET_ONLINE_FAILED:
			case bdNetImpl::BD_NET_INIT_FAILED:
			case bdNetImpl::BD_NET_STOPPED:
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

void Simulation::choosePlayerDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Choose Player 2",			// READY
		"Choosing Player 2",	// PENDING
		"Player 2 Chosen",	// DONE
		"Player 1 Chosen",	// FAILED
		"Using Player 1"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}	

void Simulation::startChoosePlayer()
{
	usePlayerTwo();
	m_taskStatus = DONE;
}

void Simulation::pumpChoosePlayer()
{
	m_taskStatus = DONE;
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
		m_authService = new bdAuthService(GAME_TITLE_ID, authAddr);
	}

	if(m_commonAuthService == BD_NULL)
	{
		const bdAddr authAddr(m_authAddr, AUTH_PORT);
		m_commonAuthService = new bdAuthService(COMMUNITY_TITLE_ID, authAddr);
	}

	if (m_usingCommunityConnections == true)
	{
		return m_commonAuthService;
	}
	else
	{
		return m_authService;
	}
}


const bdAuthInfo* Simulation::getAuthInfo()
{
	if ((m_usingCommunityConnections == true) && (m_commonAuthService))
	{
		return m_commonAuthService->getAuthInfo();
	}
	else if ((m_usingCommunityConnections == false) && (m_authService))
	{
		return m_authService->getAuthInfo();
	}
	else
	{
		return BD_NULL;
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
	const bdNChar8 *const descriptionsStart[] = {"Create ",	// READY
				"Creating ",								// PENDING
				"Created ",									// DONE
				"Create ",									// FAILED
				"Create "};							// SKIPPED

	bdString lobbyName(m_usingCommunityConnections == true ? "Community Account for " : "Game Account for ");
	bdString userName(m_usePlayerOne == true ? USERNAME1 : USERNAME2);

	const bdNChar8 *const descriptionsEnd[] = {"",	// READY
		"",											// PENDING
		"",											// DONE
		" Failed",									// FAILED
		" Skipped"};								// SKIPPED

	description = bdString(descriptionsStart[m_taskStatus]);
	description += lobbyName;
	description += userName;
	description += descriptionsEnd[m_taskStatus];

}

void Simulation::startCreateAccount()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->createAccount(m_usePlayerOne == true ? USERNAME1 : USERNAME2, PASSWORD))
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
	const bdNChar8 *const descriptionsStart[] = {"Authenticate ",	// READY
		"Authenticating ",									// PENDING
		"Authenticated ",									// DONE
		"Authenticate ",									// FAILED
		"Authenticate "};									// SKIPPED

	bdString lobbyName(m_usingCommunityConnections == true ? "Community Account for " : "Game Account for ");
	bdString userName(m_usePlayerOne == true ? USERNAME1 : USERNAME2);

	const bdNChar8 *const descriptionsEnd[] = {"",	// READY
		"",											// PENDING
		"",											// DONE
		" Failed",									// FAILED
		" Skipped"};								// SKIPPED

	description = bdString(descriptionsStart[m_taskStatus]);
	description += lobbyName;
	description += userName;
	description += descriptionsEnd[m_taskStatus];
}

void Simulation::startAuthenticateAccount()
{
	// fail safe
	m_taskStatus = FAILED;

	m_currentAuthService = getAuthService();
	if(m_currentAuthService)
	{
		if(m_currentAuthService->authorizeAccount(m_usePlayerOne == true ? USERNAME1 : USERNAME2, PASSWORD))
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
	const bdNChar8 *const descriptionsStart[] = {"Connect ",	// READY
		"Connecting ",									// PENDING
		"Connected ",									// DONE
		"Connect ",									// FAILED
		"Connect "};									// SKIPPED

	bdString lobbyName(m_usingCommunityConnections == true ? "to Community Lobby Service for " : "to Game Lobby Serivce for ");
	bdString userName(m_usePlayerOne == true ? USERNAME1 : USERNAME2);

	const bdNChar8 *const descriptionsEnd[] = {"",	// READY
		"",											// PENDING
		"",											// DONE
		" Failed",									// FAILED
		" Skipped"};								// SKIPPED

	description = bdString(descriptionsStart[m_taskStatus]);
	description += lobbyName;
	description += userName;
	description += descriptionsEnd[m_taskStatus];
}

void Simulation::startLobbyServiceConnect()
{
	// fail safe
	m_taskStatus = FAILED;

	if (m_usingCommunityConnections)
	{
		m_commonLobbyService = new bdLobbyService();
		m_currentLobbyService = m_commonLobbyService;
	}
	else
	{
		m_lobbyService = new bdLobbyService();
		m_currentLobbyService = m_lobbyService;
	}
	
	if(m_currentLobbyService)
	{
		const bdAddr lsgAddr(m_lsgAddr, LSG_PORT);

		const bdBool connecting = m_currentLobbyService->connect(lsgAddr, getAuthInfo());
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

	if(m_currentLobbyService)
	{
		const bdLobbyConnection::Status connectionStatus = m_currentLobbyService->getStatus();
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

void Simulation::printSessionList(bdString& string) const
{
	const bdUInt numSessions = m_foundSessions.getSize();
	bdNChar8 buf[32];
	bdSnprintf(buf, sizeof(buf), "%u session(s) found:\n", numSessions);
	string += buf;
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
		const bdNChar8 *const sessionName = "my Game";
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


void Simulation::createAcceptProposalDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
		"Accept Proposal",						// READY
			"Accepting Proposal",					// PENDING
			"Proposal Accepted",					// DONE
			"Proposal Accept Failed",				// FAILED
			"Proposal Accept Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAcceptProposal()
{
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();

	if(friends)	
	{
		if (m_friendProposalList[0].m_userID != 0)
		{
			m_remoteTask = friends->acceptProposal(m_friendProposalList[0].m_userID);
			m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
		}
	}
}

void Simulation::pumpAcceptProposal()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::createGetFriendsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
		"Get Friends List",						// READY
			"Getting Friends List",					// PENDING
			"Friends List:\n",						// DONE
			"Get Friends List Failed",				// FAILED
			"Get Friends List Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == DONE)
	{
		if (m_listString.getLength()) 
		{
			description += m_listString;
		}
		else
		{
			description += "No friends found.";
		}
	}
}

bdFriends *const Simulation::GetFriends()
{
	BD_ASSERT(m_commonLobbyService != BD_NULL, "Simulation::GetFriends, "
		"You must call connect to common lobby before "
		"calling GetFriends().");

	return m_commonLobbyService->getFriends();	
}



bdMatchMaking *const Simulation::GetMatchMaking()
{
	BD_ASSERT(m_commonLobbyService != BD_NULL, "Simulation::GetMatchMaking, "
		"You must call connect to common lobby before "
		"calling GetMatchMaking().");

	return m_lobbyService->getMatchMaking();	
}

void Simulation::startGetFriends()
{
	// fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{
		m_remoteTask = friends->getFriends(m_friendsList, MAX_NUM_FRIENDS);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetFriends()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{		
		m_listString = "";

		const bdUInt numFriends = m_remoteTask->getNumResults();
		for(bdUInt i=0; i < numFriends; i++)
		{
			const bdNChar8 *const onlineText = m_friendsList[i].m_isOnline?"Online":"Offline";
			bdNChar8 buffer[128];
			bdSnprintf(buffer, sizeof(buffer), "\nFriend: %s (%s)", m_friendsList[i].m_userName, onlineText);				
			m_listString += buffer;				
		}

	}

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::createProposeFriendshipDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
		"Propose Friendship",						// READY
			"Proposing Friendship",						// PENDING
			"Friendship Proposed",						// DONE
			"Propose Friendship Failed",			// FAILED
			"Propose Friendship Skipped"			// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startProposeFriendship()
{
	// fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{	
		const bdNChar8 *const otherUserName = (m_playerNumber == 1 ? USERNAME2 : USERNAME1);
		const bdOnlineUserID otherID = bdAuthUtility::getUserID(otherUserName);
		const bdNChar8 *attachment = "Be my friend";
		m_remoteTask = friends->proposeFriendship(otherID, attachment, bdStrnlen(attachment, 50)+1);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpProposeFriendship()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

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
	
	if(m_commonLobbyService)
	{
		m_commonLobbyService->disconnect();
		m_taskStatus = PENDING;
	}
}

void Simulation::pumpLobbyServiceDisconnect()
{
	// fail safe
	m_taskStatus = FAILED;

	bdLobbyService *const lobbyService = m_currentLobbyService;
	if(lobbyService)
	{
		const bdLobbyConnection::Status status = lobbyService->getStatus();
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

