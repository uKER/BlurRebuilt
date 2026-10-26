// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include <bdLobby/bdStorage/bdStorage.h>
#include <bdLobby/bdLobbyCommon/bdAuthCommon.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>

#define TITLE_ID (0x148D)

#define USERNAME "VoteRankExample"
#define PASSWORD "MyPassword"

#ifdef BD_PLATFORM_XENON
#include <bdExample/bdExampleCommonLobby/LiveServices.h>

// XBox Live LSP Service ID
#define DEFAULT_LIVE_LSP_SERVICE_ID (0x415607E9)
#define DEFAULT_LSP_PORT (1020)

#define LSP_TITLE_ID (0x56789DEF)

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
  m_lsgGetHostByName(BD_NULL),
  m_remoteTask(BD_NULL),
  m_pumpLobbyConnection(false),
  m_lobbyErrorCode(BD_NO_ERROR),
  m_fileSizeIndex(0),
  m_fileSize(FILE_SIZE_HTTP),
  m_fileSlot(0),
  m_myCategory(JPEG)
{
	// create the file data dynamically so as not to overflow stack on limited 
	// memory platforms
	m_fileData = new bdUByte8[FILE_SIZE_HTTP];
}

Simulation::~Simulation()
{
	delete [] m_fileData;
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
			m_remoteTask = BD_NULL;
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

	delete m_authGetHostByName;
#ifndef BD_PLATFORM_XENON
	delete m_authService;
#endif
	delete m_lsgGetHostByName;
	
	m_remoteTask = BD_NULL;
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
				generateUploadDescription(taskDescription);
				if(startKey)
				{
					startUpload();
				}
			}
			break;
			case SUBMIT_RATING:
			{
				generateSubmitRatingDescription(taskDescription);
				if(startKey)
				{
					startSubmitRating();
				}
			}
			break;
		
			case GET_RATING:
			{
				generateGetRatingDescription(taskDescription);
				if(startKey)
				{
					startGetRating();
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
			generateUploadDescription(taskDescription);
			pumpUpload();
		}
		break;

		case SUBMIT_RATING:
		{
			generateSubmitRatingDescription(taskDescription);
			pumpSubmitRating();
		}
		break;

		case GET_RATING:
		{
			generateGetRatingDescription(taskDescription);
			pumpGetRating();
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
			generateSubmitRatingDescription(m_lastTaskDescription);
			nextTaskIfDone = SUBMIT_RATING;
			nextTaskIfFailed = UPLOAD_FILE;
			nextTaskIfSkipped = SUBMIT_RATING;
		}
		break;

		case SUBMIT_RATING:
		{
			generateSubmitRatingDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_RATING;
			nextTaskIfFailed = SUBMIT_RATING;
			nextTaskIfSkipped = GET_RATING;
		}
		break;
		
		case GET_RATING:
		{
			generateGetRatingDescription(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_DISCONNECT;
			nextTaskIfFailed = GET_RATING;
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

void Simulation::generateUploadDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Upload file (via HTTP)",				// READY
		"Uploading file (via HTTP)",			// PENDING
		"File uploaded (via HTTP)",			// DONE
		"File upload failed (via HTTP)",		// FAILED
		"File upload skipped (via HTTP)"		// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}

void Simulation::startUpload()
{
	bdContentStreaming *const contentStreaming = m_lobbyService->getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not acquire handle to content streaming service. Are you still connected to LSG?");
		m_taskStatus = FAILED;
		return;
	}

	bdRandom rand;
	rand.nextUBytes(static_cast<bdUByte8*>(m_fileData), m_fileSize); 
	
#ifndef BD_PLATFORM_XENON
	bdUInt resSize = 24;
	m_tigerHash.hash(
		static_cast<bdUByte8*>(m_fileData),
		m_fileSize,
		m_hashFile,
		resSize);
#else
	bdMemcpy(m_filePortion, m_fileData, FILE_PORTION_SIZE); 
#endif

	bdUInt32 numTags = 5;
	bdTag tags[5];
	tags[0].set(1,5);
	tags[1].set(2,3);
	tags[2].set(3,2);
	tags[3].set(4,1);
	tags[4].set(6,4);

	bdUInt32 thumbSize = 16;
	bdUByte8* thumbData = new bdUByte8[thumbSize];
	bdMemset(thumbData,'!',thumbSize);

	if (rand.nextUInt() % 2 == 0)
	{
		bdSnprintf(m_myFileNameHTTP, sizeof(m_myFileNameHTTP), "votableFile");
	}
	else
	{
		bdSnprintf(m_myFileNameHTTP, sizeof(m_myFileNameHTTP), "File%d\0", rand.nextUInt() % 100);
	}

	m_fileSlot = m_fileSlot++ == MAX_SLOTS_PER_USER ? 1 : m_fileSlot;
	m_myCategory = JPEG;
	m_remoteTask = contentStreaming->upload(m_fileSlot,	
		m_fileData,
		m_fileSize,
		m_myFileNameHTTP, 
		m_myCategory,
		thumbData,
		thumbSize,
		numTags,
		tags,
		&m_uploadedFileID); 

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == PENDING)
	{
		BD_INFO("sim","Uploading file of size %d commenced to slot %d.", m_fileSize, m_fileSlot);
	}
	else
	{
		BD_INFO("sim","Uploading file of size %d failed to start.",m_fileSize);
	}
}

void Simulation::pumpUpload()
{
	bdContentStreaming *const contentStreaming = m_lobbyService->getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not access bdContentStreaming object");
		return;
	}

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	switch(m_taskStatus)
	{
	case PENDING:
		{
			contentStreaming->pump();
		}
		break;
	case DONE:
		{

			m_lobbyService->pump();

			const bdUInt sizeStrLen = 128;
			bdNChar8 fileIDString[sizeStrLen];
			bdSnprintf(fileIDString, sizeStrLen, "Uploaded file ID: %u of size %d to slot %d.", m_uploadedFileID.m_fileID, m_fileSize, m_fileSlot);
			m_fileListString = "";
			m_fileListString += fileIDString;

			BD_INFO("sim","Uploading file of ID %d complete.", m_uploadedFileID.m_fileID);

		}
		break;
	case FAILED:
		{
			BD_WARN("sim","Uploading failed.");
		}
		break;
	default:
		break;
	}
}

void Simulation::generateSubmitRatingDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Submit Rating",				// READY
		"Submitting Rating",			// PENDING
		"Rating Submitted",			// DONE
		"Rating Submit failed",		// FAILED
		"Rating Submit skipped"		// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}

void Simulation::startSubmitRating()
{
	// fail safe
	m_taskStatus = FAILED;
	
	// we want to vote on the file just uploaded and if possible the previous N files also,
	// so that if we run upload a bunch of times we can vote on those files also
	// If the files don't exist the votes won't count
	bdUInt64 entityID = m_uploadedFileID.m_fileID;
	entityID -= MAX_NUM_DISPLAY_RANKS;
	entityID += 1;
	entityID = BD_MAX(entityID,0);

	bdRandom rand;
	for (bdUInt32 i = 0; i < MAX_NUM_DISPLAY_RANKS; i++)
	{
		m_newRating[i].m_entityID = entityID + i;
		m_newRating[i].m_rating = rand.nextUInt() % 11;
		bdUInt rating = bdUInt(m_newRating[i].m_rating);
		BD_INFO(BD_LOG_LEVEL, "entity: %I64d vote: %d", 
			m_newRating[i].m_entityID, 
			rating);
	}

	// N.B. m_newRating is submitting entityIDs which do NOT exist, except for the file the example just uploaded.
	// The server will ignore ratings submitting for entityIDs which do not exist.
	bdVoteRank *const voterank = m_lobbyService->getVoteRank();
	m_remoteTask = voterank->submitRating(m_newRating, MAX_NUM_DISPLAY_RANKS);
	
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
}

void Simulation::pumpSubmitRating()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if((m_taskStatus == DONE) || (m_taskStatus == FAILED))
	{
		m_remoteTask = BD_NULL;
	}
}



void Simulation::generateGetRatingDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Get Rating - Wait for the cron to run",			// READY
		"Getting Rating",		// PENDING
		"Got Rating",			// DONE
		"Rating get failed",	// FAILED
		"Rating get skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}

void Simulation::startGetRating()
{	
	bdUInt32 i;
	bdEntityID entityIDs[MAX_NUM_DISPLAY_RANKS];
	for (i = 0; i < MAX_NUM_DISPLAY_RANKS; i++)
	{
		entityIDs[i] = m_newRating[i].m_entityID;
	}

	

	bdStats *const stats = m_lobbyService->getStats();
	m_remoteTask = stats->readStatsByEntityID(BD_VOTERANK_LB_ID_CATEGORY_JPEG, &m_uploadedFileID.m_fileID, 1, m_voteRankStatsInfo);
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
}

void Simulation::pumpGetRating()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		m_taskStatus = DONE;

		bdUInt numResults = m_remoteTask->getNumResults();
		if (numResults == 0)
		{
			BD_WARN(BD_LOG_LEVEL, "get rating returned no results");
		}
		for (bdUInt i(0); i<numResults; i++)
		{
			bdNChar8 entityName[64];
			if (bdStrcmp(m_voteRankStatsInfo->m_entityName, ""))
			{
				bdSnprintf(entityName, 64, "none");
			}
			else
			{
				bdSnprintf(entityName, 64, m_voteRankStatsInfo->m_entityName);
			}

			bdUInt leaderboardID = m_voteRankStatsInfo[i].m_leaderboardID;
			bdUInt rank = m_voteRankStatsInfo[i].m_rank;
			bdUInt rating = m_voteRankStatsInfo[i].m_rating;
			bdUInt avgVote = m_voteRankStatsInfo[i].m_avgVoteValue; 
			bdUInt totalVotes = m_voteRankStatsInfo[i].m_totalVotes;

			BD_INFO(BD_LOG_LEVEL, "entity: %I64d (%s) ldrbrd: %d rank: %d rating: %d avg vote (*100): %d vote count: %d", 
				m_voteRankStatsInfo[i].m_entityID, 
				entityName,
				leaderboardID,
				rank,
				rating,
				avgVote, 
				totalVotes);
		}
	}
	if((m_taskStatus == DONE) || (m_taskStatus == FAILED))
	{
		m_remoteTask = BD_NULL;
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
				m_UserId = 0;
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
