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
#define PASS "MyPassword"
#define PLAYER2 "FrndExample2"
#define PLAYER3 "FrndExample3"

Simulation::Simulation()
: m_taskStatus(READY),
  m_task(START_NETWORK),
  m_changedScreen(true),
  m_authGetHostByName(BD_NULL),
  m_authService(BD_NULL),
  m_lsgGetHostByName(BD_NULL),
  m_remoteTask(BD_NULL),
  m_pumpLobbyConnection(false),
  m_lobbyErrorCode(BD_NO_ERROR),
  m_step(0)
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
		
		/*
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
		*/
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

void Simulation::onSessionInvite(bdSessionInvite& invite)
{

	if (invite.m_attachmentSize > 0)
	{
		bdPrintf("Received session invitation: %s\n", (bdNChar8*)invite.m_attachment);
	}
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
				m_step = 0;
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
			case CHOOSE_USER_ACCOUNT_1:
			{
				chooseUserAccount1Description(taskDescription);
				if(startKey)
				{
					startChooseUserAccount1();
				}
			}
			break;
			case CHOOSE_USER_ACCOUNT_2:
			{
				chooseUserAccount2Description(taskDescription);
				if(startKey)
				{
					startChooseUserAccount2();
				}
			}
			break;
			case CHOOSE_USER_ACCOUNT_3:
			{
				chooseUserAccount3Description(taskDescription);
				if(startKey)
				{
					startChooseUserAccount3();
				}
			}
			break;
			case CREATE_PLAYER:
			{
				createPlayerDescription(taskDescription);
				if(startKey)
				{
					startCreatePlayer();
				}
			}
			break;
			case AUTHENTICATE_PLAYER:
			{
				authenticatePlayerDescription(taskDescription);
				if(startKey)
				{
					startAuthenticatePlayer();
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
			case ACCEPT_PROPOSALS:
			{
				createAcceptProposalDescription(taskDescription);
				if(startKey)
				{
					startAcceptProposal();
				}
			}
			break;
			case REJECT_PROPOSALS:
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
			case PROPOSE_FRIENDSHIPS:
			{
				createProposeFriendshipsDescription(taskDescription);
				if(startKey)
				{
					startProposeFriendships();
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
			case WITHDRAW_PROPOSALS:
			{
				createWithdrawProposalDescription(taskDescription);
				if(startKey)
				{
					startWithdrawProposal();
				}
			}
			break;
			case REMOVE_FRIENDS:
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
		case CHOOSE_USER_ACCOUNT_1:
		{
			chooseUserAccount1Description(taskDescription);
			pumpChooseUserAccount1();
		}
		break;
		case CHOOSE_USER_ACCOUNT_2:
		{
			chooseUserAccount2Description(taskDescription);
			pumpChooseUserAccount2();
		}
		break;
		case CHOOSE_USER_ACCOUNT_3:
		{
			chooseUserAccount3Description(taskDescription);
			pumpChooseUserAccount3();
		}
		break;
		case CREATE_PLAYER:
		{
			createPlayerDescription(taskDescription);
			pumpCreatePlayer();
		}
		break;
		case AUTHENTICATE_PLAYER:
		{
			authenticatePlayerDescription(taskDescription);
			pumpAuthenticatePlayer();
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
		case ACCEPT_PROPOSALS:
		{
			createAcceptProposalDescription(taskDescription);
			pumpAcceptProposal();
		}
		break;
		case REJECT_PROPOSALS:
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
		case PROPOSE_FRIENDSHIPS:
		{
			createProposeFriendshipsDescription(taskDescription);
			pumpProposeFriendships();
		}
		break;
		case LIST_PROPOSALS:
		{
			createListProposalsDescription(taskDescription);
			pumpListProposals();
		}
		break;
		case WITHDRAW_PROPOSALS:
		{
			createWithdrawProposalDescription(taskDescription);
			pumpWithdrawProposal();
		}
		break;
		case REMOVE_FRIENDS:
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
			nextTaskIfDone = CHOOSE_USER_ACCOUNT_1;
			nextTaskIfFailed = CHECK_DNS;
			nextTaskIfSkipped = CHOOSE_USER_ACCOUNT_1;
			displayLobbyError = false;
		}
		break;
		case CHOOSE_USER_ACCOUNT_1:
		{
			chooseUserAccount1Description(m_lastTaskDescription);
			nextTaskIfDone = CREATE_PLAYER;
			nextTaskIfFailed = CHOOSE_USER_ACCOUNT_1;
			nextTaskIfSkipped = CHOOSE_USER_ACCOUNT_2;
			displayLobbyError = false;
		}
		break;
		case CHOOSE_USER_ACCOUNT_2:
		{
			chooseUserAccount2Description(m_lastTaskDescription);
			nextTaskIfDone = CREATE_PLAYER;
			nextTaskIfFailed = CHOOSE_USER_ACCOUNT_2;
			nextTaskIfSkipped = CHOOSE_USER_ACCOUNT_3;
			displayLobbyError = false;
		}
		break;
		case CHOOSE_USER_ACCOUNT_3:
		{
			chooseUserAccount3Description(m_lastTaskDescription);
			nextTaskIfDone = CREATE_PLAYER;
			nextTaskIfFailed = CHOOSE_USER_ACCOUNT_3;
			nextTaskIfSkipped = CHOOSE_USER_ACCOUNT_1;
			displayLobbyError = false;
		}
		break;
		case CREATE_PLAYER:
		{
			createPlayerDescription(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = AUTHENTICATE_PLAYER;
			nextTaskIfFailed = CREATE_PLAYER;
			nextTaskIfSkipped = AUTHENTICATE_PLAYER;			
		}
		break;
		case AUTHENTICATE_PLAYER:
		{
			authenticatePlayerDescription(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = LOBBY_CONNECT;
			nextTaskIfFailed = AUTHENTICATE_PLAYER;
			nextTaskIfSkipped = LOBBY_CONNECT;			
		}
		break;		
		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(m_lastTaskDescription);
			if( m_step < 15 )
			{
				nextTaskIfDone = LOBBY_DISCONNECT;
				nextTaskIfFailed = LOBBY_CONNECT;
				nextTaskIfSkipped = LOBBY_DISCONNECT;
			}
			else if( m_step == 15 )
			{
				nextTaskIfDone = PROPOSE_FRIENDSHIPS;
				nextTaskIfFailed = LOBBY_CONNECT;
				nextTaskIfSkipped = PROPOSE_FRIENDSHIPS;
			}
			else if( m_step == 33 ||  m_step == 25 )
			{
				nextTaskIfDone = GET_FRIENDSHIP_PROPOSALS;
				nextTaskIfFailed = LOBBY_CONNECT;
				nextTaskIfSkipped = GET_FRIENDSHIP_PROPOSALS;
			}
			else
			{
				nextTaskIfDone = GET_FRIENDS;
				nextTaskIfFailed = LOBBY_CONNECT;
				nextTaskIfSkipped = GET_FRIENDS;
			}
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
			if( m_step == 26 || m_step == 46 )
			{
				nextTaskIfDone = REJECT_PROPOSALS;
				nextTaskIfFailed = GET_FRIENDSHIP_PROPOSALS;
				nextTaskIfSkipped = REJECT_PROPOSALS;
			}
			else if( m_step == 34 )
			{
				nextTaskIfDone = ACCEPT_PROPOSALS;
				nextTaskIfFailed = GET_FRIENDSHIP_PROPOSALS;
				nextTaskIfSkipped = ACCEPT_PROPOSALS;
			}
			else
			{
				nextTaskIfDone = LOBBY_DISCONNECT;
				nextTaskIfFailed = GET_FRIENDSHIP_PROPOSALS;
				nextTaskIfSkipped = LOBBY_DISCONNECT;
			}
		}
		break;
		case ACCEPT_PROPOSALS:
		{
			createAcceptProposalDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_FRIENDS;
			nextTaskIfFailed = ACCEPT_PROPOSALS;
			nextTaskIfSkipped = GET_FRIENDS;
		}
		break;
		case REJECT_PROPOSALS:
		{
			createRejectProposalDescription(m_lastTaskDescription);
			if( m_step == 27 )
			{
				nextTaskIfDone = PROPOSE_FRIENDSHIPS;
				nextTaskIfFailed = REJECT_PROPOSALS;
				nextTaskIfSkipped = PROPOSE_FRIENDSHIPS;
			}
			else if( m_step == 47 )
			{
				nextTaskIfDone = GET_FRIENDSHIP_PROPOSALS;
				nextTaskIfFailed = REJECT_PROPOSALS;
				nextTaskIfSkipped = GET_FRIENDSHIP_PROPOSALS;
			}
			else
			{
				nextTaskIfDone = LOBBY_DISCONNECT;
				nextTaskIfFailed = REJECT_PROPOSALS;
				nextTaskIfSkipped = LOBBY_DISCONNECT;
			}
		}
		break;
		case GET_FRIENDS:
		{
			createGetFriendsDescription(m_lastTaskDescription);
			if( m_step < 29 || m_step == 34 )
			{
				nextTaskIfDone = LOBBY_DISCONNECT;
				nextTaskIfFailed = GET_FRIENDS;
				nextTaskIfSkipped = LOBBY_DISCONNECT;
			}
			else if( m_step == 36 )
			{
				nextTaskIfDone = REMOVE_FRIENDS;
				nextTaskIfFailed = GET_FRIENDS;
				nextTaskIfSkipped = REMOVE_FRIENDS;
			}
			else if( m_step == 38 ) 
			{
				nextTaskIfDone = PROPOSE_FRIENDSHIPS;
				nextTaskIfFailed = GET_FRIENDS;
				nextTaskIfSkipped = PROPOSE_FRIENDSHIPS;
			}
			else
			{
				nextTaskIfDone = GET_FRIENDSHIP_PROPOSALS;
				nextTaskIfFailed = GET_FRIENDS;
				nextTaskIfSkipped = GET_FRIENDSHIP_PROPOSALS;
			}
		}
		break;
		case PROPOSE_FRIENDSHIPS:
		{
			createProposeFriendshipsDescription(m_lastTaskDescription);
			if( m_step < 19 )
			{
				nextTaskIfDone = LIST_PROPOSALS;
				nextTaskIfFailed = PROPOSE_FRIENDSHIPS;
				nextTaskIfSkipped = LIST_PROPOSALS;
			}
			else
			{
				nextTaskIfDone = LOBBY_DISCONNECT;
				nextTaskIfFailed = PROPOSE_FRIENDSHIPS;
				nextTaskIfSkipped = LOBBY_DISCONNECT;
			}
		}
		break;
		case LIST_PROPOSALS:
		{
			createListProposalsDescription(m_lastTaskDescription);
			if( m_step == 17 )
			{
				nextTaskIfDone = WITHDRAW_PROPOSALS;
				nextTaskIfFailed = LIST_PROPOSALS;
				nextTaskIfSkipped = WITHDRAW_PROPOSALS;
			}
			else if( m_step == 19 )
			{
				nextTaskIfDone = PROPOSE_FRIENDSHIPS;
				nextTaskIfFailed = LIST_PROPOSALS;
				nextTaskIfSkipped = PROPOSE_FRIENDSHIPS;
			}
			else
			{
				nextTaskIfDone = LOBBY_DISCONNECT;
				nextTaskIfFailed = LIST_PROPOSALS;
				nextTaskIfSkipped = LOBBY_DISCONNECT;
			}

		}
		break;
		case WITHDRAW_PROPOSALS:
		{
			createWithdrawProposalDescription(m_lastTaskDescription);
			nextTaskIfDone = LIST_PROPOSALS;
			nextTaskIfFailed = WITHDRAW_PROPOSALS;
			nextTaskIfSkipped = LIST_PROPOSALS;
		}
		break;
		case REMOVE_FRIENDS:
		{
			createRemoveFriendDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_FRIENDS;
			nextTaskIfFailed = REMOVE_FRIENDS;
			nextTaskIfSkipped = GET_FRIENDS;
		}
		break;
		
		case LOBBY_DISCONNECT:
		{
			lobbyServiceDisconnectDescription(m_lastTaskDescription);
			if( m_step == 21 || m_step == 6 )
			{
				nextTaskIfDone = CHOOSE_USER_ACCOUNT_2;
				nextTaskIfFailed = LOBBY_DISCONNECT;
				nextTaskIfSkipped = CHOOSE_USER_ACCOUNT_2;
			}
			else if( m_step == 40 || m_step == 11 )
			{
				nextTaskIfDone = CHOOSE_USER_ACCOUNT_3;
				nextTaskIfFailed = LOBBY_DISCONNECT;
				nextTaskIfSkipped = CHOOSE_USER_ACCOUNT_3;
			}
			else if( m_step == 29 )
			{
				nextTaskIfDone = CHOOSE_USER_ACCOUNT_1;
				nextTaskIfFailed = LOBBY_DISCONNECT;
				nextTaskIfSkipped = CHOOSE_USER_ACCOUNT_1;
			}
			else
			{
				nextTaskIfDone = STOP_NETWORK;
				nextTaskIfFailed = LOBBY_DISCONNECT;
				nextTaskIfSkipped = STOP_NETWORK;
			}
		}
		break;
		case STOP_NETWORK:
		{
			stopNetworkDescription(m_lastTaskDescription);
			nextTaskIfDone = START_NETWORK;
			nextTaskIfFailed = STOP_NETWORK;
			nextTaskIfSkipped = CREATE_PLAYER;
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
			m_step++;
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
			m_step++;
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

void Simulation::startChooseUserAccount1()
{
	// fail safe
	m_taskStatus = DONE;
	m_userName = PLAYER1;
}

void Simulation::pumpChooseUserAccount1()
{
}

void Simulation::chooseUserAccount1Description(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Choose User Account 1",			// READY
											"Choosing User Account 1",			// PENDING
											"Chose User Account 1",				// DONE
											"Choose User Account 1 Failed",		// FAILED
											"Choose User Account 1 Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startChooseUserAccount2()
{
	// fail safe
	m_taskStatus = DONE;
	m_userName = PLAYER2;
}

void Simulation::pumpChooseUserAccount2()
{
}

void Simulation::chooseUserAccount2Description(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Choose User Account 2",			// READY
											"Choosing User Account 2",			// PENDING
											"Chose User Account 2",			// DONE
											"Choose User Account 2 Failed",		// FAILED
											"Choose User Account 2 Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startChooseUserAccount3()
{
	// fail safe
	m_taskStatus = DONE;
	m_userName = PLAYER3;
}

void Simulation::pumpChooseUserAccount3()
{
}

void Simulation::chooseUserAccount3Description(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Choose User Account 3",			// READY
											"Choosing User Account 3",			// PENDING
											"Chose User Account 3",			// DONE
											"Choose User Account 3 Failed",		// FAILED
											"Choose User Account 3 Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::createPlayerDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Create Player",	// READY
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

void Simulation::startCreatePlayer()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->createAccount(m_userName, PASS))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpCreatePlayer()
{
	pumpAuthServiceTask();
}

void Simulation::authenticatePlayerDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Authenticate Account",	// READY
											"Authenticating Player ",									// PENDING
											"Authenticated Player ",									// DONE
											"Authenticate Player Failed",								// FAILED
											"Authenticate Player Skipped"};							// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAuthenticatePlayer()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();	
	if(authService)
	{
		if(authService->authorizeAccount(m_userName, PASS))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpAuthenticatePlayer()
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
		m_numFriendProposals = numProposals;
		bdNChar8 buffer[128];
		m_listString = "";

		bdSnprintf(buffer, sizeof(buffer), "\nReceived %u new friendship proposals\n", numProposals);
		m_listString += buffer;
		for (bdUInt i=0; i<numProposals; i++)
		{
			bdNChar8 buffer[128];
			bdSnprintf(buffer, sizeof(buffer), "%s wants to be your friend\n", m_friendProposalList[i].m_userName);
			m_listString += buffer;	
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

	if( m_taskStatus == DONE )
	{
		if( m_numFriendProposals == 0 )
		{
			description += "\nAccepted 0 proposals";
		}
		else
		{
			for( bdUInt i = 0;i<m_numFriendProposals;i++)
			{
				bdNChar8 buffer[128];
				bdSnprintf(buffer,sizeof(buffer), "\nAccepted %s", m_friendProposalList[i].m_userName );
				description += buffer;
			}
		}
	}
}

void Simulation::startAcceptProposal()
{
	m_taskStatus = FAILED;
	
	bdFriends *const friends = m_lobbyService->getFriends();

	if(friends)	
	{
		bdOnlineUserID toAccept[ MAX_NUM_FRIENDS ];
		for( bdUInt i = 0;i<m_numFriendProposals;i++)
		{
			toAccept[i] = m_friendProposalList[i].m_userID;
		}

		m_remoteTask = friends->acceptProposals(toAccept, m_numFriendProposals);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
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

	if( m_taskStatus == DONE )
	{
		if( m_numFriendProposals == 0 )
		{
			description += "\nRejected 0 proposals";
		}
		else
		{
			for( bdUInt i = 0;i<m_numFriendProposals;i++)
			{
				bdNChar8 buffer[128];
				bdSnprintf(buffer,sizeof(buffer), "\nRejected %s", m_friendProposalList[i].m_userName );
				description += buffer;
			}
		}
	}
}

void Simulation::startRejectProposal()
{
	// fail safe
	m_taskStatus = FAILED;	

	// find a friend proposal message
	bdFriends *const friends = m_lobbyService->getFriends();

	if(friends)	
	{
		bdOnlineUserID toReject[ MAX_NUM_FRIENDS ];
		for( bdUInt i = 0;i<m_numFriendProposals;i++)
		{
			toReject[i] = m_friendProposalList[i].m_userID;
		}

		m_remoteTask = friends->rejectProposals(toReject, m_numFriendProposals);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
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
		m_numFriends = numFriends;
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

void Simulation::createProposeFriendshipsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = 
	{	
			"Propose Friendship",						// READY
			"Proposing Friendship",						// PENDING
			"Friendship Proposed to ",						// DONE
			"Propose Friendship Failed",			// FAILED
			"Propose Friendship Skipped"			// SKIPPED
	};	

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if( m_taskStatus == DONE )
	{
		description += m_listString;
	}
}

void Simulation::startProposeFriendships()
{
	// fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{	
		bdFriendProposal proposals[2];
		m_listString = "";

		if( m_step < 26 )
		{
			bdStrlcpy( proposals[0].m_userName, PLAYER1, bdStrnlen( PLAYER1, 50)+1 );
			bdStrlcpy( proposals[1].m_userName, PLAYER2, bdStrnlen( PLAYER2, 50)+1 );

			const bdNChar8 *attachment = "Be My Friend";
			proposals[0].m_userID = bdAuthUtility::getUserID(proposals[0].m_userName);
			proposals[0].m_attachmentSize = bdStrnlen( attachment, 50)+1;
			bdMemcpy( proposals[0].m_attachment, attachment, proposals[0].m_attachmentSize );
			
			proposals[1].m_userID = bdAuthUtility::getUserID(proposals[1].m_userName);
			proposals[1].m_attachmentSize = bdStrnlen( attachment, 50)+1;
			bdMemcpy( proposals[1].m_attachment, attachment, proposals[1].m_attachmentSize );

			m_remoteTask = friends->proposeFriendships(proposals, 2);

			m_listString += PLAYER1;
			m_listString += " ";
			m_listString += PLAYER2;
			m_listString += "\n";
		}
		else if( m_step == 28 )
		{
			bdStrlcpy( proposals[0].m_userName, PLAYER1, bdStrnlen( PLAYER1, 50)+1 );
			bdStrlcpy( proposals[1].m_userName, PLAYER3, bdStrnlen( PLAYER3, 50)+1 );

			const bdNChar8 *attachment = "Be My Friend";
			proposals[0].m_userID = bdAuthUtility::getUserID(proposals[0].m_userName);
			proposals[0].m_attachmentSize = bdStrnlen( attachment, 50)+1;
			bdMemcpy( proposals[0].m_attachment, attachment, proposals[0].m_attachmentSize );
			
			proposals[1].m_userID = bdAuthUtility::getUserID(proposals[1].m_userName);
			proposals[1].m_attachmentSize = bdStrnlen( attachment, 50)+1;
			bdMemcpy( proposals[1].m_attachment, attachment, proposals[1].m_attachmentSize );

			m_remoteTask = friends->proposeFriendships(proposals, 2);

			m_listString += PLAYER1;
			m_listString += " ";
			m_listString += PLAYER3;
			m_listString += "\n";
		}
		else
		{
			bdStrlcpy( proposals[0].m_userName, PLAYER3, bdStrnlen( PLAYER3, 50)+1 );

			const bdNChar8 *attachment = "Be My Friend";
			proposals[0].m_userID = bdAuthUtility::getUserID(proposals[0].m_userName);
			proposals[0].m_attachmentSize = bdStrnlen( attachment, 50)+1;
			bdMemcpy( proposals[0].m_attachment, attachment, proposals[0].m_attachmentSize );
			
			m_remoteTask = friends->proposeFriendships(proposals, 1);

			m_listString += PLAYER3;
			m_listString += "\n";
		}
		
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpProposeFriendships()
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
		m_numFriendProposals = numProposals;
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

	if( m_taskStatus == DONE )
	{
		if( m_numFriendProposals == 0 )
		{
			description += "\nWithdrew 0 proposals";
		}
		else
		{
			for( bdUInt i = 0;i<m_numFriendProposals;i++)
			{
				bdNChar8 buffer[128];
				bdSnprintf(buffer,sizeof(buffer), "\nWithdrew proposal to %s", m_friendProposalList[i].m_userName );
				description += buffer;
			}
		}
	}
}

void Simulation::startWithdrawProposal()
{
	// fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{	
		bdOnlineUserID toWithdraw[MAX_NUM_FRIENDS];
		for(bdUInt i=0;i<m_numFriendProposals;i++)
		{
			toWithdraw[i] = m_friendProposalList[i].m_userID;
		}

		m_remoteTask = friends->withdrawProposals(toWithdraw, m_numFriendProposals);
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

	if( m_taskStatus == DONE )
	{
		if( m_numFriendProposals == 0 )
		{
			description += "\nRemoved 0 Friends";
		}
		else
		{
			for( bdUInt i = 0;i<m_numFriends;i++)
			{
				bdNChar8 buffer[128];
				bdSnprintf(buffer,sizeof(buffer), "\nRemoved %s as a friend", m_friendsList[i].m_userName );
				description += buffer;
			}
		}
	}
}

void Simulation::startRemoveFriend()
{
	//fail safe
	m_taskStatus = FAILED;

	bdFriends *const friends = m_lobbyService->getFriends();
	if(friends)
	{
		bdOnlineUserID toRemove[MAX_NUM_FRIENDS];
		for( bdUInt i=0;i<m_numFriends;i++ )
		{
			toRemove[i] = m_friendsList[i].m_userID;
		}
		m_remoteTask = friends->removeFriends(toRemove, m_numFriends);
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




