// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// This example illustrates how secure dedicated servers are supported in DemonWare.
// This is only relevant if your design accommodates trusted dedicated servers
// that report statistics on behalf of the players that connect to them. Such
// servers are often called "ranked servers".
// Such a design implied that credentials are required to run the server. When
// using Matchmaking+, a special license code is required so that the dedicated
// server can authenticate. Furthermore, the server's license ID (a public value 
// derived from the secret license code) can be used by a client to obtain an 
// authentication ticket so it can prove its identity to the dedicated server.
// This provides the basis for a highly secure dedicated server infrastructure.
// If your design specifies the use of game servers (either dedicated- or "listen"-
// servers), but you do not intend to use them to report ranked stats, then this
// example is not relevant to you.
// Please note that it is up to the studio/publisher to manage the distribution
// of license codes to dedicated server administrators.

#include "Simulation.h"
#include "MatchMakingQueries.h"
#include "LeaderboardEntry.h"

#include <bdLobby/bdLobbyCommon/bdAuthCommon.h>
#include <bdConnection/bdMessage/bdReceivedMessage.h>
#include <bdConnection/bdConnectionStore.h>

#define TITLE_ID (0x1408)


#define USERNAME "HostExample"
#define PASSWORD "MyPassword"

/// The key the dedicated server uses when authorizing with Demonware's Authorization server.
#define HOST_LICENSE_CODE "AE9B-AN1T-71C7-SC6L-66R8"

/// The key that came on the clients cd, so that the client can create an account
#define CLIENT_LICENSE_CODE "EA8C-QI1H-81K0-BJ0H-80J5"

// Wait 10 seconds before doing notify calls.
#define NOTIFY_BATCH_PERIOD (10.0f)

Simulation::Simulation()
: m_taskStatus(READY),
  m_task(START_NETWORK),
  m_changedScreen(true),
  m_authGetHostByName(BD_NULL),
  m_authService(BD_NULL),
  m_lsgGetHostByName(BD_NULL),
  m_remoteTask(BD_NULL),
  m_runningAsHost(false),
  m_pumpLobbyConnection(false),
  m_lobbyErrorCode(BD_NO_ERROR),
  m_notifyJoinTask(BD_NULL),
  m_notifyLeaveTask(BD_NULL)
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

	// Register this instance as a dispatch interceptor so it can pick up
	// messages from clients if it is running as a dedicated host.
	bdNet::getInstance()->getConnectionStore()->registerDispatchInterceptor(this);

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
		else
		{
		
		}
	}

	getScreen().printToScreen(m_screenText);

	if(m_changedScreen)
	{
		m_screenText = "";
	}

	// Pump the connection layer.
	if(bdNet::getInstance()->getStatus() == bdNetImpl::BD_NET_DONE)
	{
		bdNet::getInstance()->sendAll();
		bdNet::getInstance()->receiveAll();
		bdNet::getInstance()->dispatchAll();
	}
	
	return !quitKey;
}

void Simulation::quit()
{
	m_logSubscriber.removeChannel("warn");
	m_logSubscriber.removeChannel("err");
	m_logSubscriber.removeChannel("info");
	
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

	if(m_lobbyService)
	{
		delete m_lobbyService;
		m_lobbyService = BD_NULL;
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
			case START_HOST:
			{
				startHostDescription(taskDescription);
				if(startKey)
				{
					// We are not performing an online operation here, so
					// we just set the flag and set the task status to DONE
					// so that we immediately jump to the next state.
					m_runningAsHost = true;
					m_taskStatus = DONE;
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
			case AUTHENTICATE_CLIENT_ACCOUNT:
			{
				authenticateClientAccountDescription(taskDescription);
				if(startKey)
				{
					startAuthenticateClientAccount();
				}
			}
			break;
			case AUTHENTICATE_HOST_LICENSE:
				{
					authenticateHostLicenseDescription(taskDescription);
					if(startKey)
					{
						startAuthenticateHostLicense();
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
			case WAIT_FOR_TICKETS:
			{
				waitForTicketsDescription(taskDescription);
				if(startKey)
				{
					startWaitForTickets();
				}
			}
			break;
			case WRITE_DEDICATED_STATS:
			{
				writeDedicatedStatsDescription(taskDescription);
				if(startKey)
				{
					startWriteDedicatedStats();
				}
			}
			break;
			case FIND_SESSION:
			{
				findSessionDescription(taskDescription);
				if(startKey)
				{
					startFindSession();
				}
			}
			break;
			case CLIENT_GETTING_HOST_TICKET:
			{
				authenticateClientForHostDescription(taskDescription);
				if(startKey)
				{
					startAuthenticateClientForHost();
				}
			}
			break;
			case CLIENT_CONNECTING_TO_HOST:
			{
				connectToHostDescription(taskDescription);
				if(startKey)
				{
					startConnectToHost();
				}
			}
			break;
			case DELETE_SESSION:
			{
				deleteSessionDescription(taskDescription);
				if(startKey)
				{
					startDeleteSession();
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
		break;
		case AUTHENTICATE_CLIENT_ACCOUNT:
		{
			authenticateClientAccountDescription(taskDescription);
			pumpAuthenticateClientAccount();
		}
		break;
		case AUTHENTICATE_HOST_LICENSE:
			{
				authenticateHostLicenseDescription(taskDescription);
				pumpAuthenticateHostLicense();
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
		case WAIT_FOR_TICKETS:
		{
			waitForTicketsDescription(taskDescription);
			pumpWaitForTickets();
		}
		break;
		case WRITE_DEDICATED_STATS:
		{
			writeDedicatedStatsDescription(taskDescription);
			pumpWriteDedicatedStats();
		}
		break;
		case FIND_SESSION:
		{
			findSessionDescription(taskDescription);
			pumpFindSession();
		}
		break;
		case CLIENT_GETTING_HOST_TICKET:
		{
			authenticateClientForHostDescription(taskDescription);
			pumpAuthenticateClientForHost();
		}
		break;
		case CLIENT_CONNECTING_TO_HOST:
		{
			connectToHostDescription(taskDescription);
			pumpConnectToHost();
		}
		break;
		case DELETE_SESSION:
		{
			deleteSessionDescription(taskDescription);
			pumpDeleteSession();
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
		default:
		{
			// don't care..
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
			nextTaskIfDone = START_HOST;
			nextTaskIfFailed = CHECK_DNS;
			nextTaskIfSkipped = START_HOST;
			displayLobbyError = false;
		}
		break;
		case START_HOST:
		{
			nextTaskIfDone = AUTHENTICATE_HOST_LICENSE;
			nextTaskIfFailed = CHECK_DNS;
			nextTaskIfSkipped = CREATE_ACCOUNT;
			displayLobbyError = false;
		}
		break;
		case CREATE_ACCOUNT:
		{
			createAccountDescription(m_lastTaskDescription);
			nextTaskIfDone = AUTHENTICATE_CLIENT_ACCOUNT;
			nextTaskIfFailed = CREATE_ACCOUNT;
			nextTaskIfSkipped = AUTHENTICATE_CLIENT_ACCOUNT;			
		}
		break;
		case AUTHENTICATE_CLIENT_ACCOUNT:
		{
			authenticateClientAccountDescription(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_CONNECT;
			nextTaskIfFailed = AUTHENTICATE_CLIENT_ACCOUNT;
			nextTaskIfSkipped = LOBBY_CONNECT;		
		}
		break;
		case AUTHENTICATE_HOST_LICENSE:
		{
			authenticateHostLicenseDescription(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_CONNECT;
			nextTaskIfFailed = AUTHENTICATE_HOST_LICENSE;
			nextTaskIfSkipped = LOBBY_CONNECT;
		}
		break;
		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(m_lastTaskDescription);
			if(m_runningAsHost)
			{
				nextTaskIfDone = ADVERTISE_SESSION;
				nextTaskIfFailed = LOBBY_CONNECT;
				nextTaskIfSkipped = ADVERTISE_SESSION;
			}
			else
			{
				nextTaskIfDone = FIND_SESSION;
				nextTaskIfFailed = LOBBY_CONNECT;
				nextTaskIfSkipped = FIND_SESSION;
			}
		}
		break;
		case ADVERTISE_SESSION:
		{
			advertiseSessionDescription(m_lastTaskDescription);
			nextTaskIfDone = WAIT_FOR_TICKETS;
			nextTaskIfFailed = ADVERTISE_SESSION;
			nextTaskIfSkipped = WAIT_FOR_TICKETS;
		}
		break;
		case WAIT_FOR_TICKETS:
		{
			waitForTicketsDescription(m_lastTaskDescription);
			nextTaskIfDone = WAIT_FOR_TICKETS;
			nextTaskIfFailed = WAIT_FOR_TICKETS;
			nextTaskIfSkipped = WRITE_DEDICATED_STATS;
		}
		break;
		case WRITE_DEDICATED_STATS:
		{
			writeDedicatedStatsDescription(m_lastTaskDescription);
			nextTaskIfDone = DELETE_SESSION;
			nextTaskIfFailed = WRITE_DEDICATED_STATS;
			nextTaskIfSkipped = DELETE_SESSION;
		}
		break;
		case FIND_SESSION:
		{
			findSessionDescription(m_lastTaskDescription);
			nextTaskIfDone = CLIENT_GETTING_HOST_TICKET;
			nextTaskIfFailed = FIND_SESSION;
			nextTaskIfSkipped = CLIENT_GETTING_HOST_TICKET;
		}
		break;
		case CLIENT_GETTING_HOST_TICKET:
		{
			authenticateClientForHostDescription(m_lastTaskDescription);
			nextTaskIfDone = CLIENT_CONNECTING_TO_HOST;
			nextTaskIfFailed = FIND_SESSION;
			nextTaskIfSkipped = CLIENT_CONNECTING_TO_HOST;
		}
		break;
		case CLIENT_CONNECTING_TO_HOST:
		{
			connectToHostDescription(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_DISCONNECT;
			nextTaskIfFailed = FIND_SESSION;
			nextTaskIfSkipped = LOBBY_DISCONNECT;
		}
		break;
		case DELETE_SESSION:
		{
			deleteSessionDescription(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_DISCONNECT;
			nextTaskIfFailed = DELETE_SESSION;
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
			nextTaskIfSkipped = START_HOST;
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

void Simulation::startHostDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Start Host",			// READY
											"Starting Host",		// PENDING
											"Start Host Succeeded",		// DONE
											"Start Host Failed",		// FAILED
											"Start Host Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
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
	const bdNChar8 *const descriptions[] = {"Create Account (" USERNAME "/" PASSWORD ")",	// READY
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

void Simulation::startCreateAccount()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->createAccount(USERNAME, PASSWORD, CLIENT_LICENSE_CODE))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpCreateAccount()
{
	pumpAuthServiceTask();
}

void Simulation::authenticateClientAccountDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Authenticate Account (" USERNAME "/" PASSWORD ")",	// READY
											"Authenticating Account",									// PENDING
											"Authenticated Account",									// DONE
											"Authenticate Account Failed",								// FAILED
											"Authenticate Account Skipped"};						// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAuthenticateClientAccount()
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

void Simulation::pumpAuthenticateClientAccount()
{
	pumpAuthServiceTask();
}

void Simulation::authenticateHostLicenseDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Authenticate Host License (" HOST_LICENSE_CODE ")",	// READY
		"Authenticating License",									// PENDING
		"Authenticated License",									// DONE
		"Authenticate License Failed",								// FAILED
		"Authenticate License Skipped"};							// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAuthenticateHostLicense()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService )
	{
		if(authService->authorizeDedicatedHost(HOST_LICENSE_CODE))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpAuthenticateHostLicense()
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

void Simulation::printSessionList(bdString& string) const
{
	bdNChar8 buf[32];
	bdSnprintf(buf, sizeof(buf), "%u session(s) found:\n", m_numSessions);
	string += buf;
//	m_foundSessions.print(string);
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
	bdMatchMaking *const matchmaking = m_lobbyService->getMatchMaking();

	if(matchmaking && net)
	{
		// the session details
		const bdCommonAddrRef hostAddress = net->getLocalCommonAddr();
		bdUInt64 licenseID = bdAuthUtility::getLicenseID(HOST_LICENSE_CODE);

		m_session.setHostAddr(hostAddress);
		m_session.m_gameType = 1;
		m_session.m_maxPlayers = 10;
		m_session.setLicenseID(reinterpret_cast<bdUByte8*>(&licenseID), sizeof(licenseID));

		// send to the lobby service
		m_remoteTask = matchmaking->createSession(&m_session, &m_sessionID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpAdvertiseSession()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	
	if(m_taskStatus == DONE)
	{
		// fail safe
		m_taskStatus = FAILED;

	
//		m_session.m_sessionID = m_sessionID;
	
	}

	if((m_taskStatus == DONE) || (m_taskStatus == FAILED))
	{
//		CloseRemoteTask(&m_remoteTask);
	}
}

void Simulation::waitForTicketsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Wait for Client Tickets",				// READY
											"Waiting for Client Tickets",			// PENDING
											"Received Client Ticket",				// DONE
											"Decryption of Client Ticket Failed",	// FAILED
											"Waiting for Client Tickets Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == DONE && !m_connectedUsers.isEmpty())
	{
		bdAuthTicket &ticket = m_connectedUsers[m_connectedUsers.getSize() - 1];
		description += "\nUsername: ";
		description += ticket.m_username;
		description += "\n";
	}
}

void Simulation::startWaitForTickets()
{
	m_taskStatus = PENDING;
	m_lastNotify.reset();
	m_lastNotify.start();
}

void Simulation::pumpWaitForTickets()
{
	if (m_lastNotify.getElapsedTimeInSeconds() > NOTIFY_BATCH_PERIOD &&
		m_notifyJoinTask == BD_NULL &&
		m_notifyLeaveTask == BD_NULL)
	{
		m_lastNotify.reset();
		m_lastNotify.start();
		// Time to notify, if there's anything.
		bdMatchMaking *matchmaking = m_lobbyService->getMatchMaking();
		if (m_usersPendingNotifyLeave.getSize() > 0)
		{
			BD_INFO(BD_LOG_LEVEL, "Calling notifyLeave on %u users", m_usersPendingNotifyLeave.getSize());
			
			m_notifyLeaveTask = matchmaking->notifyLeave(m_sessionID,
														 m_usersPendingNotifyLeave.begin(),
														 m_usersPendingNotifyLeave.getSize());
			m_usersPendingNotifyLeave.clear();
		}
		if (m_usersPendingNotifyJoin.getSize() > 0)
		{
			BD_INFO(BD_LOG_LEVEL, "Calling notifyJoin on %u users", m_usersPendingNotifyJoin.getSize());
			m_notifyJoinTask = matchmaking->notifyJoin(m_sessionID,
													   m_usersPendingNotifyJoin.begin(),
													   m_usersPendingNotifyJoin.getSize());
			m_usersPendingNotifyJoin.clear();
		}
	}
	if (m_notifyJoinTask)
	{
		TaskStatus status = CheckRemoteTaskStatus(m_notifyJoinTask);
		if (status != PENDING)
		{
			m_taskStatus = DONE;
		}
	}
	if (m_notifyLeaveTask)
	{
		TaskStatus status = CheckRemoteTaskStatus(m_notifyLeaveTask);
		if (status != PENDING)
		{
			m_taskStatus = DONE;
		}
	}
}

void Simulation::writeDedicatedStatsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Write Stats",						// READY
											"Writing Stats",					// PENDING
											"Write Stats Complete",				// DONE
											"Write Stats Failed",				// FAILED
											"Write Stats Skipped"};				// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startWriteDedicatedStats()
{
	// fail safe
	m_taskStatus = FAILED;
/*
	bdStats *const stats = m_lobbyService->getStats();
	if(stats && !m_connectedUsers.isEmpty())
	{
		// We use a bdArray for convenience here. Most games will have a max 
		// number of players, and could therefore use a non-dynamic array for this.
		//bdArray<SampleLeaderboard1> statsData(32);
		bdArray<LeaderboardEntry> statsData;

		// fill in stats data (using fake data here) for all connected users
		bdUInt i;
		for (i = 0; i < m_connectedUsers.getSize(); i++)
		{
			LeaderboardEntry s;

			// the stats values
			const bdInt64 rating = 10;
			const bdInt64 kills = 5;
			const bdInt32 deaths = 1;
			const bdInt32 assists = 3;
			const bdInt32 started = 20;
			const bdInt32 completed = 15;
			const bdFloat32 killRate = 0.26f;

			s.setRating(rating);
			s.setKills(kills);
			s.setDeaths(deaths);
			s.setAssists(assists);
			s.setStarted(started);
			s.setCompleted(completed);
			s.setKillRate(killRate);
			s.setEntityName(m_connectedUsers[i].m_username);
			s.setEntityID(m_connectedUsers[i].m_userID);

			statsData.pushBack(s);
		}

		// send to the lobby service
		bdStatsInfo *statsPtr = statsData.begin();
		m_remoteTask = stats->writeStats(	0,
													LEADERBOARD_ID,
													bdStats::STAT_WRITE_REPLACE,
													&statsPtr,
													statsData.getSize());
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);		
	}
	*/
}

void Simulation::pumpWriteDedicatedStats()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		// fail safe
		m_taskStatus = FAILED;

	}

	if((m_taskStatus == DONE) || (m_taskStatus == FAILED))
	{
	}
}

void Simulation::findSessionDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Find Session",							// READY
											"Finding Session",						// PENDING
											"Find Session Complete",				// DONE
											"Find Session Failed",					// FAILED
											"Find Session Skipped"};				// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
	
	if(m_taskStatus == DONE)
	{
		description += "\n";
		printSessionList(description);
	}
}

void Simulation::startFindSession()
{
	// fail safe
	m_taskStatus = FAILED;

	bdMatchMaking *const matchmaking = m_lobbyService->getMatchMaking();
	if(matchmaking)
	{
		FindSessions queryParams;
		m_remoteTask = matchmaking->findSessions(FINDSESSIONS_QUERY_ID, 0, 10, &queryParams, m_foundSessions);

		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpFindSession()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	
	if(m_taskStatus == DONE)
	{
		m_numSessions = m_remoteTask->getNumResults();
	}
}

void Simulation::authenticateClientForHostDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Authenticate Account (" USERNAME "/" PASSWORD ") for Host",	// READY
											"Authenticating Account for Host",								// PENDING
											"Authenticated Account for Host",								// DONE
											"Authenticate Account for Host Failed",							// FAILED
											"Authenticate Account for Host Skipped"};						// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAuthenticateClientForHost()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService && m_numSessions>0)
	{
		bdUInt64 foundHostLicenseID;
		m_session.getLicenseID(reinterpret_cast<bdUByte8*>(&foundHostLicenseID), sizeof(foundHostLicenseID));
		if(authService->authorizeAccount(USERNAME, foundHostLicenseID, PASSWORD))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpAuthenticateClientForHost()
{
	pumpAuthServiceTask();
}

void Simulation::connectToHostDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Connect to Host",							// READY
											"Connecting to Host",						// PENDING
											"Connected to Host",						// DONE
											"Connect to Host Failed",					// FAILED
											"Connect to Host Skipped"};					// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startConnectToHost()
{
	// fail safe
	m_taskStatus = FAILED;

	//  connect to the server address
	bdConnectionStore *store = bdNet::getInstance()->getConnectionStore();
	m_connection = store->create(m_session.getHostAddrAsCommonAddr(), m_sessionID.m_sessionID);
	if(m_connection)
	{
		if(m_connection->connect())
		{
			m_taskStatus = PENDING;
			
			// Register this as a listener so we know when the connection has
			// succeeded.
			m_connection->registerListener(this);
		}
	}
}

void Simulation::pumpConnectToHost()
{
}

void Simulation::sendCredentialsToDedicatedServer()
{
	bdMessageRef msg = new bdMessage(DEDICATED_SERVER_CREDENTIALS_MSG);
	bdBitBufferRef buffer = msg->getPayload();
	buffer->writeUInt32(getAuthInfo()->m_IVSeed);
	buffer->writeBlob(getAuthInfo()->m_data, BD_AUTH_TICKET_SIZE);
	m_connection->send(msg, true);
}

void Simulation::deleteSessionDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Delete Session",			// READY
											"Deleting Session",			// PENDING
											"Deleted Session",			// DONE
											"Delete Session Failed",		// FAILED
											"Delete Session Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startDeleteSession()
{
	// fail safe
	m_taskStatus = FAILED;
	
	bdMatchMaking *const matchmaking = m_lobbyService->getMatchMaking();
	if(matchmaking)
	{
		// send to the lobby service
		m_remoteTask = matchmaking->deleteSession(m_sessionID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpDeleteSession()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
	
	}

	if((m_taskStatus == DONE) || (m_taskStatus == FAILED))
	{
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

bdBool Simulation::accept(const bdReceivedMessage &receivedMsg)
{
	bdBool handled = false;
	const bdMessageRef msg = receivedMsg.getMessage();
	const bdBitBufferRef buffer = msg->getPayload();
	if(msg.notNull() && buffer.notNull() && (msg->getType()  == DEDICATED_SERVER_CREDENTIALS_MSG))	
	{
		bdUInt32 IVSeed;
		bdBool ok = buffer->readUInt32(IVSeed);

		bdUByte8 ticket[BD_AUTH_TICKET_SIZE];
		bdUInt32 ticketSize = BD_AUTH_TICKET_SIZE;
		ok = ok && buffer->readBlob(ticket, ticketSize);

		if(ok && (ticketSize == BD_AUTH_TICKET_SIZE))
		{
			bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
			bdCryptoUtils::calculateInitialVector(IVSeed, initialVector);

			bdByte8 licenseKey[BD_ENCRYPTION_KEY_SIZE];
			bdAuthUtility::getLicenseKey(HOST_LICENSE_CODE, licenseKey);

			bdCryptoUtils::decrypt(licenseKey, initialVector, ticket, ticket, BD_AUTH_TICKET_SIZE);

			bdAuthTicket authTicket;
			authTicket.deserialize(ticket);
			if (authTicket.m_magicNumber != BD_MAGIC_NUMBER)
			{
				BD_ERR(BD_LOG_LEVEL, "Decryption of auth ticket failed.");
				m_taskStatus = FAILED;
			}
			else if (static_cast<bdUInt32>(time(NULL)) > authTicket.m_timeExpires)
			{
				BD_ERR(BD_LOG_LEVEL, "Timestamp expired.");
				m_taskStatus = FAILED;
			}
			else
			{
				m_userConnections.remove(receivedMsg.getConnection());
				m_userConnections.put(receivedMsg.getConnection(), authTicket.m_userID);
				m_connectedUsers.pushBack(authTicket);
				m_usersPendingNotifyJoin.pushBack(authTicket.m_userID);

				//m_taskStatus = DONE;
			}
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Error deserializing message from client.");
			m_taskStatus = FAILED;
		}
		handled = true;
	}
	return handled;
}

void Simulation::onConnect(bdConnectionRef connection)
{
	sendCredentialsToDedicatedServer();
	m_taskStatus = DONE;
}

void Simulation::onConnectFailed(bdConnectionRef connection)
{
	m_taskStatus = FAILED;
}

void Simulation::onDisconnect(bdConnectionRef connection)
{
	if (m_userConnections.containsKey(connection))
	{
		bdOnlineUserID userID;
		m_userConnections.get(connection, userID);

		// Check if the user is still in notifyJoin queue
		bdUInt userIndex = 0;
		if (m_usersPendingNotifyJoin.findFirst(userID, userIndex))
		{
			// Never sent notifyJoin for the user.
			m_usersPendingNotifyJoin.removeAll(userID);
			BD_INFO(BD_LOG_LEVEL, "User " BD_UINT_64_FORMAT " was not registered with notifyJoin.", userID);
		}
		else
		{
			// Ok, must've been notifyJoin'ed
			BD_INFO(BD_LOG_LEVEL, "Scheduling user " BD_UINT_64_FORMAT " for notifyLeave.", userID);
			m_usersPendingNotifyLeave.pushBack(userID);

		}

		m_userConnections.remove(connection);
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "Disconnect on connection where no credentials were sent.");
	}
}
