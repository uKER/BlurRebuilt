// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include "TagQueries.h"

#include <bdLobby/bdContentStreaming/bdContentStreaming.h>
#include <bdLobby/bdTags/bdTags.h>

#define USERNAME "TagExample"
#define PASSWORD "tagexamplepass"

#define TITLE_ID (0x14BF)

#ifdef BD_PLATFORM_XENON
#include <bdExample/bdExampleCommonLobby/LiveServices.h>

// XBox Live LSP Service ID
#define DEFAULT_LIVE_LSP_SERVICE_ID (0x415607E9)
#define DEFAULT_LSP_PORT (1020)

#define LSP_TITLE_ID (0x41560845)

#include <xtitleidbegin.h>
XEX_TITLE_ID(LSP_TITLE_ID)
#include <xtitleidend.h>
#endif


Simulation::Simulation()
: m_taskStatus(READY),
  m_task(START_NETWORK),
  m_changedScreen(true),
  m_authGetHostByName(BD_NULL),
#ifndef BD_PLATFORM_XENON
  m_authService(BD_NULL),
#endif
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
#ifndef BD_PLATFORM_XENON
	delete m_authService;
#endif
	delete m_authGetHostByName;
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
#ifndef BD_PLATFORM_XENON
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
#endif
			case LOBBY_CONNECT:
			{
				lobbyServiceConnectDescription(taskDescription);
				if(startKey)
				{
					startLobbyServiceConnect();
				}
			}
			break;


			case UPLOAD_FILE:
			{
				uploadFileDescription(taskDescription);
				if(startKey)
				{
					startUploadFile();
				}
			}
			break;
			case SET_TAGS_FOR_ENTITYID:
			{
				setTagsDescription(taskDescription);
				if(startKey)
				{
					startSetTags();
				}
			}
			break;
			case GET_TAGS_FOR_ENTITYIDS:
			{
				getTagsDescription(taskDescription);
				if(startKey)
				{
					startGetTags();
				}
			}
			break;
			case SEARCY_BY_TAGS:
			{
				searchByTagsDescription(taskDescription);
				if(startKey)
				{
					startSearchByTags();
				}
			}
			break;
			case REMOVE_TAGS_FOR_ENTITYID:
			{
				removeTagsDescription(taskDescription);
				if(startKey)
				{
					startRemoveTags();
				}
			}
			break;
			case GET_TAGS_FOR_ENTITYIDS2:
			{
				getTagsDescription(taskDescription);
				if(startKey)
				{
					startGetTags();
				}
			}
			break;
			case REMOVE_ALL_TAGS_FOR_ENTITYID:
			{
				removeAllTagsDescription(taskDescription);
				if(startKey)
				{
					startRemoveAllTags();
				}
			}
			break;
			case GET_TAGS_FOR_ENTITYIDS3:
			{
				getTagsDescription(taskDescription);
				if(startKey)
				{
					startGetTags();
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
#ifndef BD_PLATFORM_XENON
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
#endif
		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(taskDescription);
			pumpLobbyServiceConnect();
		}
		break;

		case UPLOAD_FILE:
		{
			uploadFileDescription(taskDescription);
			pumpUploadFile();
		}
		break;
		case SET_TAGS_FOR_ENTITYID:
		{
			setTagsDescription(taskDescription);
			pumpSetTags();
		}
		break;
		case GET_TAGS_FOR_ENTITYIDS:
		case GET_TAGS_FOR_ENTITYIDS2:
		case GET_TAGS_FOR_ENTITYIDS3:
		{
			getTagsDescription(taskDescription);
			pumpGetTags();
		}
		break;
		case SEARCY_BY_TAGS:
		{
			searchByTagsDescription(taskDescription);
			pumpSearchByTags();
		}
		break;
		case REMOVE_TAGS_FOR_ENTITYID:
		{
			removeTagsDescription(taskDescription);
			pumpRemoveTags();
		}
		break;
		case REMOVE_ALL_TAGS_FOR_ENTITYID:
		{
			removeAllTagsDescription(taskDescription);
			pumpRemoveAllTags();
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

	bdBool authStage = false;
	bdBool displayLobbyError = true;

	switch(m_task)
	{
		case START_NETWORK:
		{
			startNetworkDescription(m_lastTaskDescription);
#ifdef BD_PLATFORM_XENON
			nextTaskIfDone = LOBBY_CONNECT;
#else
			nextTaskIfDone = CHECK_DNS;
#endif
			nextTaskIfFailed = START_NETWORK;
			nextTaskIfSkipped = START_NETWORK;
			displayLobbyError = false;
		}
		break;
#ifndef BD_PLATFORM_XENON
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
			authStage = true;
		}
		break;
		case AUTHENTICATE_ACCOUNT:
		{
			authenticateAccountDescription(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = LOBBY_CONNECT;
			nextTaskIfFailed = AUTHENTICATE_ACCOUNT;
			nextTaskIfSkipped = LOBBY_CONNECT;
			authStage = true;
		}
		break;
#endif
		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(m_lastTaskDescription);
			nextTaskIfDone = UPLOAD_FILE;
			nextTaskIfFailed = LOBBY_CONNECT;
			nextTaskIfSkipped = UPLOAD_FILE;
		}
		break;

		case UPLOAD_FILE:
		{
			uploadFileDescription(m_lastTaskDescription);
			nextTaskIfDone = SET_TAGS_FOR_ENTITYID;
			nextTaskIfFailed = UPLOAD_FILE;
			nextTaskIfSkipped = SET_TAGS_FOR_ENTITYID;
		}
		break;
		case SET_TAGS_FOR_ENTITYID:
		{
			setTagsDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_TAGS_FOR_ENTITYIDS;
			nextTaskIfFailed = SET_TAGS_FOR_ENTITYID;
			nextTaskIfSkipped = GET_TAGS_FOR_ENTITYIDS;
		}
		break;
		case GET_TAGS_FOR_ENTITYIDS:
		{
			getTagsDescription(m_lastTaskDescription);
			nextTaskIfDone = SEARCY_BY_TAGS;
			nextTaskIfFailed = GET_TAGS_FOR_ENTITYIDS;
			nextTaskIfSkipped = SEARCY_BY_TAGS;
		}
		break;
		case SEARCY_BY_TAGS:
		{
			searchByTagsDescription(m_lastTaskDescription);
			nextTaskIfDone = REMOVE_TAGS_FOR_ENTITYID;
			nextTaskIfFailed = SEARCY_BY_TAGS;
			nextTaskIfSkipped = REMOVE_TAGS_FOR_ENTITYID;
		}
		break;
		case REMOVE_TAGS_FOR_ENTITYID:
		{
			removeTagsDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_TAGS_FOR_ENTITYIDS2;
			nextTaskIfFailed = REMOVE_TAGS_FOR_ENTITYID;
			nextTaskIfSkipped = GET_TAGS_FOR_ENTITYIDS2;
		}
		break;
		case GET_TAGS_FOR_ENTITYIDS2:
		{
			getTagsDescription(m_lastTaskDescription);
			nextTaskIfDone = REMOVE_ALL_TAGS_FOR_ENTITYID;
			nextTaskIfFailed = GET_TAGS_FOR_ENTITYIDS2;
			nextTaskIfSkipped = REMOVE_ALL_TAGS_FOR_ENTITYID;
		}
		break;
		case REMOVE_ALL_TAGS_FOR_ENTITYID:
		{
			removeAllTagsDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_TAGS_FOR_ENTITYIDS3;
			nextTaskIfFailed = REMOVE_ALL_TAGS_FOR_ENTITYID;
			nextTaskIfSkipped = LOBBY_DISCONNECT;
		}
		break;
		case GET_TAGS_FOR_ENTITYIDS3:
		{
			getTagsDescription(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_DISCONNECT;
			nextTaskIfFailed = GET_TAGS_FOR_ENTITYIDS3;
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
		if(authStage)
		{
#ifndef BD_PLATFORM_XENON
			checkAuthServiceError();
#endif
		}
		else
		{
			if (m_remoteTask != BD_NULL)
			{
				m_lobbyErrorCode = m_remoteTask->getErrorCode();
			}
		}

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

#ifndef BD_PLATFORM_XENON
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
	const bdNChar8 *const descriptions[] = {"Authenticate Account (" USERNAME "/" PASSWORD ")",	// READY
											"Authenticating Account",									// PENDING
											"Authenticated Account",									// DONE
											"Authenticate Account Failed",								// FAILED
											"Authenticate Account Skipped"};							// SKIPPED

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
#endif

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

#ifdef BD_PLATFORM_XENON
void Simulation::startLobbyServiceConnect()
{
	// fail safe
	m_taskStatus = FAILED;

	if(m_lobbyService)
	{
		LiveServices liveServices;
		liveServices.signOn(0);
		bdBool m_errorFlag = false;
		while (	(false == liveServices.isSignedOn()) && 
			(false == m_errorFlag))
		{
			m_errorFlag = !(liveServices.pump());
			bdPlatformTiming::sleep(50);
		}

		if(false == m_errorFlag)
		{
			bdAddr ca;
			bdSecurityID unusedID;
			bdSecurityKey key;
			if (!liveServices.resolveTitleAddr(DEFAULT_LIVE_LSP_SERVICE_ID, DEFAULT_LSP_PORT, ca, unusedID, key))
			{
				BD_WARN("test", "Failed to resolve LSG address. Failing now.");
				m_errorFlag = true;
			}
			else
			{
				// setup local user
				bdNChar8 buffer [BD_MAX_USERNAME_LENGTH];
				liveServices.getXUserName(buffer);

				BD_INFO("test", "SG Lite address found.");

				// Prepare auth info
				bdAuthTicket ticket;
				ticket.m_userID = liveServices.getXUserId();

				ticket.m_titleID = TITLE_ID;
				liveServices.getXUserName(ticket.m_username);

				bdAuthInfo authInfo;
				authInfo.m_titleID = TITLE_ID;
				ticket.serialize(authInfo.m_data);

				BD_INFO("test", "Connecting to matchmaking...");
				m_errorFlag = !(m_lobbyService->connect(ca, &authInfo));

				bdPrintf ("Connecting to MatchMaking+ sandbox...");
			}
		}
		if(false == m_errorFlag)
		{
			m_taskStatus = PENDING;
		}
	}
}
#else
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
#endif

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
#ifdef BD_PLATFORM_XENON
				m_lobbyService->getContentStreaming()->init(DEFAULT_LIVE_LSP_SERVICE_ID);
#endif
			}
			break;			
		}
	}
}


void Simulation::printFileIDsList(bdString& string)
{
	bdNChar8 buf[32];
	bdSnprintf(buf, sizeof(buf), "%u fileIDs(s) found:\n", m_numFilesIDs);
	string += buf;
	m_foundFileIDs.clear();
	for (bdUInt i = 0; i < m_numFilesIDs; i++)
	{
		m_foundFileIDs.add(m_fileIDs[i]);
	}
	m_foundFileIDs.print(string);
}

void Simulation::printFileIDTagsList(bdString& string)
{
	m_foundTags.clear();
	for (bdUInt i = 0; i < m_numFilesIDs; i++)
	{
		m_foundTags.add(m_tagResults[i]);
	}
	m_foundTags.print(string);
}


void Simulation::uploadFileDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Upload File",			// READY
											"Uploading File",			// PENDING
											"Upload File Done",			// DONE
											"Upload File Failed",		// FAILED
											"Upload File Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}
void Simulation::startUploadFile()
{

	bdContentStreaming *const contentStreaming = m_lobbyService->getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not acquire handle to content streaming service. Are you still connected to LSG?");
		m_taskStatus = FAILED;
		return;
	}


	bdUInt fileSize = 1024;
	bdUByte8 fileData[1024];
	bdMemset(fileData,'*', fileSize); // 2A in hex

	bdUInt thumbSize = 32;
	bdUByte8 thumbData[32];
	bdMemset(thumbData,'!',thumbSize);

	bdUInt32 numTags = 5;
	bdTag tags[5];
	tags[0].set(1,2);
	tags[1].set(2,31);
	tags[2].set(3,52);
	tags[2].set(4,66);
	tags[2].set(5,77);

	bdUInt16 fileSlot = 1;
	bdUInt16 category = 0;
	m_remoteTask = contentStreaming->upload(fileSlot,
											fileData,
											fileSize,
											"exampleFileName",
											category,
											thumbData,
											thumbSize,
											numTags,
											tags,
											&m_uploadedFileID); 


	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == PENDING)
	{
		BD_INFO("sim","Uploading file of size %d commenced.", fileSize);
	}
	else
	{
		BD_INFO("sim","Uploading file of size %d failed to start.", fileSize);
	}
}
void Simulation::pumpUploadFile()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	
	bdContentStreaming *const contentStreaming = m_lobbyService->getContentStreaming();
	contentStreaming->pump();

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
	else if(m_taskStatus == DONE)
	{
		m_numFilesIDs = 1;
	}
}



void Simulation::setTagsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Set Tags",			// READY
											"Setting Tags",			// PENDING
											"Set Tags Done",			// DONE
											"Set Tags Done Failed",		// FAILED
											"Set Tags Done Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}
void Simulation::startSetTags()
{
	m_taskStatus = FAILED;

	bdTags *const tagService = m_lobbyService->getTags();
	if(tagService)
	{
		bdUInt32 numTags = 3;
		bdTag tags[3];
		tags[0].set(1,3);
		tags[1].set(2,3);
		tags[2].set(2,4);
		
		m_remoteTask = tagService->setTagsForEntityID(m_uploadedFileID.m_fileID, numTags, tags);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);	
	}
}
void Simulation::pumpSetTags()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}



void Simulation::getTagsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Tags",			// READY
											"Getting Tags",			// PENDING
											"Get Tags Done",			// DONE
											"Get Tags Failed",		// FAILED
											"Get Tags Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == DONE)
	{
		description += "\n";
		printFileIDTagsList(description);
	}
}
void Simulation::startGetTags()
{
	m_taskStatus = FAILED;

	bdTags *const tagService = m_lobbyService->getTags();
	if(tagService)
	{
		m_remoteTask = tagService->getTagsForEntityIDs(1, &m_uploadedFileID.m_fileID, m_tagResults);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);	
	}
}
void Simulation::pumpGetTags()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
	else if(m_taskStatus == DONE)
	{
		m_numFilesIDs = m_remoteTask->getNumResults();
	}

}



void Simulation::searchByTagsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Search By Tags",			// READY
											"Searching By Tags",			// PENDING
											"Search By Tags Done",			// DONE
											"Search By Tags Failed",		// FAILED
											"Search By Tags Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == DONE)
	{
		description += "\n";
		printFileIDsList(description);
	}
}
void Simulation::startSearchByTags()
{
	m_taskStatus = FAILED;

	bdTags *const tagService = m_lobbyService->getTags();
	if(tagService)
	{		
		bdUInt startIndex = 0;
		bdUInt maxNumResults = MAX_NUM_FILES;
		bdUInt numTags = 2;
		bdTag tags[2];
		tags[0].set(1,3);
		tags[1].set(2,4);

		m_remoteTask = tagService->searchByTags(startIndex, maxNumResults, numTags, tags, m_fileIDs);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}
void Simulation::pumpSearchByTags()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		m_numFilesIDs = m_remoteTask->getNumResults();
	}

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}


// Remove Tags
void Simulation::removeTagsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Remove Tags Query",			// READY
											"Removing Tags Query",			// PENDING
											"Remove Tags Query Done",			// DONE
											"Remove Tags Query Failed",		// FAILED
											"Remove Tags Query Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}
void Simulation::startRemoveTags()
{
	m_taskStatus = FAILED;

	bdTags *const tagService = m_lobbyService->getTags();
	if(tagService)
	{
		bdUInt numTags = 2;
		bdTag tags[2];
		tags[0].set(1,3);
		tags[1].set(2,4);
		m_remoteTask = tagService->removeTagsForEntityID(m_uploadedFileID.m_fileID, numTags, tags);

		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}
void Simulation::pumpRemoveTags()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

// Remove All Tags
void Simulation::removeAllTagsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Remove All Tags Query",			// READY
											"Removing All Tags Query",			// PENDING
											"Remove All Tags Query Done",			// DONE
											"Remove All Tags Query Failed",		// FAILED
											"Remove TAll ags Query Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}
void Simulation::startRemoveAllTags()
{
	m_taskStatus = FAILED;

	bdTags *const tagService = m_lobbyService->getTags();
	if(tagService)
	{
		m_remoteTask = tagService->removeAllTagsForEntityID(m_uploadedFileID.m_fileID);

		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}
void Simulation::pumpRemoveAllTags()
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

