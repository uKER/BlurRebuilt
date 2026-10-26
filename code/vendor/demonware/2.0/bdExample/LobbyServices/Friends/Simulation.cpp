// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "UserInfo.h"
#include "Simulation.h"

#include <bdLobby/bdFriends/bdFriends.h>

#define TITLE_ID (0x14BF)

#define PLAYER1 "FrndExample1"
#define PASS1 "MyPassword"
#define PLAYER2 "FrndExample2"
#define PASS2 "MyPassword"


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
	m_numNotifications = 0;
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


	getScreen().printToScreen(m_messageText + m_screenText);

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

void Simulation::onFriendshipProposal()
{
	bdPrintf("Received new friendship proposal\n");
}
		
void Simulation::onNewNotification()
{
	bdPrintf("Received new notification\n");
}

void Simulation::onPlayerStatusUpdate(bdOnlineUserID userID, 
							      bdNChar8 *userName,
								  bdUByte8 status)
{
	if (status == BD_STATUS_ONLINE)
	{
		bdPrintf("Player %s is now online\n", userName);

	}
	else
	{
		bdPrintf("Player %s is now offline\n", userName);
	}
}

void Simulation::onRichPresenceUpdate(bdOnlineUserID userID,
									  bdNChar8 *userName,
									  void *data,
									  bdUInt dataSize,
									  bdTeamID teamID)
{
	bdPrintf("Received rich presence update from %s : %s\n", userName, data);
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
			case GET_NOTIFICATIONS:
			{
				createGetNotificationsDescription(taskDescription);
				if(startKey)
				{
					startGetNotifications();
				}
			}
				break;
			case GET_FRIENDSHIP_PROPOSALS:
			{
				createGetFriendshipProposalsDescription(taskDescription);
				if(startKey)
				{
					startGetFriendshipProposals();
				}
			}
			break;
			case ACCEPT_PROPOSAL:
			{
				createAcceptProposalDescription(taskDescription);
				if(startKey)
				{
					startAcceptProposal();
				}
			}
			break;
			case REJECT_PROPOSAL:
			{
				createRejectProposalDescription(taskDescription);
				if(startKey)
				{
					startRejectProposal();
				}
			}
				break;
			case GET_FRIENDS:
			{
				createGetFriendsDescription(taskDescription);
				if(startKey)
				{
					startGetFriends();
				}
			}
			break;
			case PROPOSE_FRIENDSHIP:
			{
				createProposeFriendshipDescription(taskDescription);
				if(startKey)
				{
					startProposeFriendship();
				}
			}
			break;
			case LIST_PROPOSALS:
			{
				createListProposalsDescription(taskDescription);
				if(startKey)
				{
					startListProposals();
				}
			}
			break;
			case WITHDRAW_PROPOSAL:
			{
				createWithdrawProposalDescription(taskDescription);
				if(startKey)
				{
					startWithdrawProposal();
				}
			}
			break;
			case SEND_MESSAGE:
				{
					createSendCustomDescription(taskDescription);
					if (startKey)
					{
						startSendCustom();
					}
				}
			break;
			case UPDATE_RICH_PRESENCE:
				{
					createSetRichPresenceDescription(taskDescription);
					if (startKey)
					{
						startSetRichPresence();
					}
				}
			break;
			case CREATE_GROUP:
				{
					createGroupDescription(taskDescription);
					if (startKey)
					{
						startGroup();
					}
				}
			break;
			case LIST_GROUPS:
				{
					createListGroupsDescription(taskDescription);
					if (startKey)
					{
						startListGroups();
					}
				}
			break;
			case ADD_FRIEND_TO_GROUP:
				{
					createAddFriendToGroupDescription(taskDescription);
					if (startKey)
					{
						startAddFriendToGroup();
					}
				}
			break;
			
			case DELETE_GROUP:
				{
					createDeleteGroupDescription(taskDescription);
					if (startKey)
					{
						startDeleteGroup();
					}
				}
			break;
			case REMOVE_FRIEND:
				{
					createRemoveFriendDescription(taskDescription);
					if (startKey)
					{
						startRemoveFriend();
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
		case GET_NOTIFICATIONS:
		{
			createGetNotificationsDescription(taskDescription);
			pumpGetNotifications();
		}
		break;
		case GET_FRIENDSHIP_PROPOSALS:
		{
			createGetFriendshipProposalsDescription(taskDescription);
			pumpGetFriendshipProposals();
		}
		break;
		case ACCEPT_PROPOSAL:
		{
			createAcceptProposalDescription(taskDescription);
			pumpAcceptProposal();
		}
		break;
		case REJECT_PROPOSAL:
		{
			createRejectProposalDescription(taskDescription);
			pumpRejectProposal();
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
		case LIST_PROPOSALS:
		{
			createListProposalsDescription(taskDescription);
			pumpListProposals();
		}
		break;
		case WITHDRAW_PROPOSAL:
		{
			createWithdrawProposalDescription(taskDescription);
			pumpWithdrawProposal();
		}
		break;
		case SEND_MESSAGE:
		{
			createSendCustomDescription(taskDescription);
			pumpSendCustom();
		}
		break;
		case UPDATE_RICH_PRESENCE:
		{
			createSetRichPresenceDescription(taskDescription);
			pumpSetRichPresence();
		}
		break;
		case CREATE_GROUP:
		{
			createGroupDescription(taskDescription);
			pumpGroup();
		}
		break;
		case LIST_GROUPS:
		{
			createListGroupsDescription(taskDescription);
			pumpListGroups();
		}
		break;
		case ADD_FRIEND_TO_GROUP:
		{
			createAddFriendToGroupDescription(taskDescription);
			pumpAddFriendToGroup();
		}
		break;
		
		case DELETE_GROUP:
		{
			createDeleteGroupDescription(taskDescription);
			pumpDeleteGroup();
		}
		break;
		case REMOVE_FRIEND:
		{
			createRemoveFriendDescription(taskDescription);
			pumpRemoveFriend();
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
			nextTaskIfDone = GET_NOTIFICATIONS;
			nextTaskIfFailed = LOBBY_CONNECT;
			nextTaskIfSkipped = GET_NOTIFICATIONS;
		}
		break;
		case GET_NOTIFICATIONS:
		{
			createGetNotificationsDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_FRIENDSHIP_PROPOSALS;
			nextTaskIfFailed = GET_NOTIFICATIONS;
			nextTaskIfSkipped = GET_FRIENDSHIP_PROPOSALS;
		}
		break;
		case GET_FRIENDSHIP_PROPOSALS:
		{
			createGetFriendshipProposalsDescription(m_lastTaskDescription);
			nextTaskIfDone = ACCEPT_PROPOSAL;
			nextTaskIfFailed = GET_FRIENDSHIP_PROPOSALS;
			nextTaskIfSkipped = GET_FRIENDS;
		}
		break;
		case ACCEPT_PROPOSAL:
		{
			createAcceptProposalDescription(m_lastTaskDescription);
			nextTaskIfDone = REJECT_PROPOSAL;
			nextTaskIfFailed = ACCEPT_PROPOSAL;
			nextTaskIfSkipped = REJECT_PROPOSAL;
		}
		break;
		case REJECT_PROPOSAL:
		{
			createRejectProposalDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_FRIENDS;
			nextTaskIfFailed = REJECT_PROPOSAL;
			nextTaskIfSkipped = GET_FRIENDS;
		}
		break;
		
		case GET_FRIENDS:
		{
			createGetFriendsDescription(m_lastTaskDescription);
			nextTaskIfDone = PROPOSE_FRIENDSHIP;
			nextTaskIfFailed = GET_FRIENDS;
			nextTaskIfSkipped = PROPOSE_FRIENDSHIP;
		}
		break;
		case PROPOSE_FRIENDSHIP:
		{
			createProposeFriendshipDescription(m_lastTaskDescription);
			nextTaskIfDone = LIST_PROPOSALS;
			nextTaskIfFailed = PROPOSE_FRIENDSHIP;
			nextTaskIfSkipped = LIST_PROPOSALS;
		}
		break;
		case LIST_PROPOSALS:
		{
			createListProposalsDescription(m_lastTaskDescription);
			nextTaskIfDone = WITHDRAW_PROPOSAL;
			nextTaskIfFailed = LIST_PROPOSALS;
			nextTaskIfSkipped = WITHDRAW_PROPOSAL;
		}
		break;
		case WITHDRAW_PROPOSAL:
		{
			createWithdrawProposalDescription(m_lastTaskDescription);
			nextTaskIfDone = SEND_MESSAGE;
			nextTaskIfFailed = PROPOSE_FRIENDSHIP;
			nextTaskIfSkipped = SEND_MESSAGE;
		}
		break;
		case SEND_MESSAGE:
		{
			createSendCustomDescription(m_lastTaskDescription);
			nextTaskIfDone = UPDATE_RICH_PRESENCE;
			nextTaskIfFailed = SEND_MESSAGE;
			nextTaskIfSkipped = UPDATE_RICH_PRESENCE;
		}
		break;
		case UPDATE_RICH_PRESENCE:
		{
			createSetRichPresenceDescription(m_lastTaskDescription);
			nextTaskIfDone = CREATE_GROUP;
			nextTaskIfFailed = UPDATE_RICH_PRESENCE;
			nextTaskIfSkipped = CREATE_GROUP;
		}
		break;
		case CREATE_GROUP:
		{
			createGroupDescription(m_lastTaskDescription);
			nextTaskIfDone = LIST_GROUPS;
			nextTaskIfFailed = CREATE_GROUP;
			nextTaskIfSkipped = LIST_GROUPS;
		}
		break;
		case LIST_GROUPS:
		{
			createListGroupsDescription(m_lastTaskDescription);
			nextTaskIfDone = ADD_FRIEND_TO_GROUP;
			nextTaskIfFailed = LIST_GROUPS;
			nextTaskIfSkipped = ADD_FRIEND_TO_GROUP;
		}
		break;
		case ADD_FRIEND_TO_GROUP:
		{
			createAddFriendToGroupDescription(m_lastTaskDescription);
			nextTaskIfDone = DELETE_GROUP;
			nextTaskIfFailed = ADD_FRIEND_TO_GROUP;
			nextTaskIfSkipped = DELETE_GROUP;
		}
		break;
	
		case DELETE_GROUP:
		{
			createDeleteGroupDescription(m_lastTaskDescription);
			nextTaskIfDone = REMOVE_FRIEND;
			nextTaskIfFailed = DELETE_GROUP;
			nextTaskIfSkipped = REMOVE_FRIEND;
		}
		break;
		case REMOVE_FRIEND:
		{
			createRemoveFriendDescription(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_DISCONNECT;
			nextTaskIfFailed = REMOVE_FRIEND;
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
		checkFriendsError();
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

void Simulation::createGetNotificationsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
			"Get Notifications",					// READY
			"Getting Notifications",					// PENDING
			"Got Notifications",					// DONE
			"Get Notifications failed",			// FAILED
			"Get Notifications Skipped"			// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
	if(m_taskStatus == DONE)
	{
		for (bdUInt i=0; i<m_numNotifications; i++)
		{
			switch (m_notifications[i].m_type)
			{
				case BD_FRIENDSHIP_ACCEPTED:
				{
					description += m_notifications[i].m_sourceUserName;
					description += "accepted your friendship proposal\n";
				}
				break;
				case BD_FRIENDSHIP_REVOKED:
				{
					description += m_notifications[i].m_sourceUserName;
					description += "revoked his friendship \n";
				}
				break;
				case BD_FRIENDSHIP_REJECTED:
				{
					description += m_notifications[i].m_sourceUserName;
					description += "rejected your friendship proposal\n";
				}
				break;
				default:
				{
					//TODO it might be nice to print more info on the type of 
					// notification being ignored ? 
					description += "ignoring irrelevant notification \n";
				}
			}
		}
	}
}

void Simulation::startGetNotifications()
{
	// fail safe
	m_taskStatus = FAILED;

	bdMessaging *const messaging = m_lobbyService->getMessaging();
	if(messaging)
	{
		m_remoteTask = messaging->getNotifications(m_notifications, 0, MAX_NUM_NOTIFICATIONS, false);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetNotifications()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		const bdUInt numNotifications = m_remoteTask->getNumResults();
		const bdUInt totalNumNotifications = m_remoteTask->getTotalNumResults();
		bdPrintf("Retrieved %u notifications out of %u \n", numNotifications , totalNumNotifications);

		for (bdUInt i=0; i<numNotifications; i++)
		{
			switch (m_notifications[i].m_type)
			{
				case BD_FRIENDSHIP_ACCEPTED:
				{
					bdPrintf("%s accepted your friendship proposal\n", m_notifications[i].m_sourceUserName);
				}
				break;
				case BD_FRIENDSHIP_REVOKED:
				{
					bdPrintf("%s revoked his friendship \n", m_notifications[i].m_sourceUserName);
				}
				break;
				case BD_FRIENDSHIP_REJECTED:
				{
					bdPrintf("%s rejected your friendship proposal\n", m_notifications[i].m_sourceUserName);
				}
				break;
				default:
				{
					bdPrintf("Ignoring irrelevant notification of type: %u" , m_notifications[i].m_type);
				}
			}
		}
	}

	if((m_taskStatus == DONE) || (m_taskStatus == FAILED))
	{
		m_remoteTask = BD_NULL;
	}
}

void Simulation::createGetFriendshipProposalsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
		"Retrieve friendship proposals",					// READY
		"Retrieving friendship proposals",					// PENDING
		"Friendship proposals retrieved",					// DONE
		"Friendship proposals could not be retrieved",		// FAILED
		"Friendship proposals retrieval Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetFriendshipProposals()
{
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if (friends)
	{
		m_remoteTask = friends->getIncomingProposals(m_friendProposalList, MAX_NUM_FRIENDS);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}


}

void Simulation::pumpGetFriendshipProposals()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		// Nothing to do here
		bdUInt numProposals = m_remoteTask->getNumResults();
		bdPrintf("Received %u new friendship proposals\n", numProposals);
		for (bdUInt i=0; i<numProposals; i++)
		{
			bdPrintf("%s wants to be your friend\n", m_friendProposalList[i].m_userName);
		}
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

void Simulation::createRejectProposalDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
			"Reject Proposal",						// READY
			"Rejecting Proposal",					// PENDING
			"Proposal Rejected",					// DONE
			"Proposal Reject Failed",				// FAILED
			"Proposal Reject Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startRejectProposal()
{
	// fail safe
	m_taskStatus = FAILED;	

	// find a friend proposal message
	bdFriends *const friends = m_lobbyService->getFriends();

	if(friends)	
	{
		if (m_friendProposalList[0].m_userID != 0)
		{
			m_remoteTask = friends->rejectProposal(m_friendProposalList[0].m_userID);
			m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
		}
	}

}

void Simulation::pumpRejectProposal()
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
		const bdNChar8 *const otherUserName = (m_playerNumber == 1 ? PLAYER2 : PLAYER1);
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

void Simulation::createListProposalsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
			"List Friendship Proposals",						// READY
			"Listing Friendship Proposals",						// PENDING
			"Friendship Proposals:\n",							// DONE
			"List Friendship Proposals Failed",			// FAILED
			"List Friendship Proposals Skipped"			// SKIPPED
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
			description += "No friend proposals found.";
		}
	}
}

void Simulation::startListProposals()
{
	// fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{	
		m_remoteTask = friends->getOutgoingProposals(m_friendProposalList, MAX_NUM_FRIENDS);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpListProposals()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		// fail safe
		m_taskStatus = FAILED;

		
		m_taskStatus = DONE;
		m_listString = "";

		const bdUInt numProposals = m_remoteTask->getNumResults();
		for(bdUInt i = 0; i < numProposals; i++)
		{
			bdNChar8 buffer[128];
			bdSnprintf(buffer, sizeof(buffer), "\nPotential Friend: %s", m_friendProposalList[i].m_userName);
			m_listString += buffer;					
		}

	}

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::createWithdrawProposalDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
			"Withdraw Friendship Proposal",						// READY
			"Withdrawing Friendship Proposal",					// PENDING
			"Friendship Proposal Withdrawn",					// DONE
			"Withdraw Friendship Proposal Failed",				// FAILED
			"Withdraw Friendship Proposal Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startWithdrawProposal()
{
	// fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{	
		const bdNChar8 *const otherUserName = (m_playerNumber == 1 ? PLAYER2 : PLAYER1);
		const bdOnlineUserID otherID = bdAuthUtility::getUserID(otherUserName);
		m_remoteTask = friends->withdrawProposal(otherID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpWithdrawProposal()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::createSetRichPresenceDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Set rich presence",							// READY
		"Setting rich presence",							// PENDING
		"Set rich presence",							// DONE
		"Set rich presence Failed",					// FAILED
		"Set rich presence Skipped"					// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startSetRichPresence()
{
	//fail safe
	m_taskStatus = FAILED;

	bdProfiles *const profiles = m_lobbyService->getProfiles();
	bdFriends *const friends = m_lobbyService->getFriends();
	if(profiles)
	{
		const bdNChar8 *rp = "Analyse this";	
		m_remoteTask = friends->setRichPresence(rp, 13);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpSetRichPresence()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}


void Simulation::createSendCustomDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
			"Send Instant message",						// READY
			"Sending  message",					// PENDING
			"Sent message",						// DONE
			"Send Message Failed",				// FAILED
			"Send Message Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startSendCustom()
{
	//fail safe
	m_taskStatus = FAILED;

	bdFriends *friends  = m_lobbyService->getFriends();
	if(friends)
	{
		const bdNChar8 *const msg = "How's things my friend?\n";
		const bdUInt msgsize = static_cast<bdUInt>(bdStrlen(msg))+1;

		
		const bdNChar8 *const otherUserName = (m_playerNumber == 1 ? PLAYER2 : PLAYER1);
		const bdOnlineUserID otherID = bdAuthUtility::getUserID(otherUserName);
		
		m_remoteTask = friends ->sendInstantMessage(otherID, (void*)msg, msgsize);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpSendCustom()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::createGroupDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
			"Create Group",						// READY
			"Creating Group",					// PENDING
			"Group Created",					// DONE
			"Create Group Failed",				// FAILED
			"Create Group Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGroup()
{
	//fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{
		bdGroupInfo groupInfo;
		groupInfo.m_groupID = 1;
		bdStrlcpy(groupInfo.m_groupName, "test_group1", BD_MAX_GROUPNAME_LENGTH);
		m_remoteTask = friends->setGroupName(&groupInfo);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGroup()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::createListGroupsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
			"List Groups",						// READY
			"Listing Groups",					// PENDING
			"Groups:\n",							// DONE
			"List Groups Failed",				// FAILED
			"List Groups Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == DONE)
	{
		bdBool groupFound = false;
		for (bdUInt i = 0;i < MAX_NUM_GROUPS; i++)
		{
			if(m_groupList[i].m_groupID != 0)
			{
				description += "\n Group: ";
				description += m_groupList[i].m_groupName;
				groupFound = true;
			}
		}
		if (false == groupFound)
		{
			description += "No groups found.";
		}
	}
}

void Simulation::startListGroups()
{
	//fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{
		m_remoteTask = friends->getGroupNames(m_groupList, MAX_NUM_GROUPS);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpListGroups()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::createAddFriendToGroupDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
			"Add Friend to Group",						// READY
			"Adding Friend to Group",					// PENDING
			"Added Friend to Group",					// DONE
			"Add Friend to Group Failed",				// FAILED
			"Add Friend to Group Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAddFriendToGroup()
{
	//fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{
		const bdNChar8 *const otherUserName = (m_playerNumber == 1 ? PLAYER2 : PLAYER1);
		const bdOnlineUserID otherID = bdAuthUtility::getUserID(otherUserName);	
		m_remoteTask = friends->setFriendGroup(otherID, 1);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpAddFriendToGroup()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}



void Simulation::createDeleteGroupDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
			"Delete Group",						// READY
			"Deleting Group",					// PENDING
			"Deleted Group",					// DONE
			"Delete Group Failed",				// FAILED
			"Delete Group Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startDeleteGroup()
{
	//fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{
		m_remoteTask = friends->deleteGroupName(1);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpDeleteGroup()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::createRemoveFriendDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
			"Remove Friend",					// READY
			"Removing Friend",					// PENDING
			"Removed Friend",					// DONE
			"Remove Friend Failed",				// FAILED
			"Remove Friend Skipped"				// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startRemoveFriend()
{
	//fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{
		const bdNChar8 *const otherUserName = (m_playerNumber == 1 ? PLAYER2 : PLAYER1);
		const bdOnlineUserID otherID = bdAuthUtility::getUserID(otherUserName);

		m_remoteTask = friends->removeFriend(otherID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpRemoveFriend()
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

void Simulation::checkProfilesError()
{
	const bdProfiles *const profiles = m_lobbyService->getProfiles();
	if(profiles)
	{
		//m_lobbyErrorCode = profiles->getErrorCode();
	}
}

void Simulation::checkFriendsError()
{
	const bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{
		//m_lobbyErrorCode = friends->getErrorCode();
	}
}




