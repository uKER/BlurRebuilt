// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"

#define TITLE_ID (0x1408)
#define USERNAME "LicenceExample"
#define PASSWORD "MyPassword"
#define LICENSE_CODE "EA8C-QI1H-81K0-BJ0H-80J5"
#define CHANGED_PASSWORD "MyNewPassword"

Simulation::Simulation()
: m_taskStatus(READY),
  m_task(START_NETWORK),
  m_changedScreen(true),
  m_authGetHostByName(BD_NULL),
  m_authService(BD_NULL),
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

	if(m_authGetHostByName)
	{
		m_authGetHostByName->quit();
		delete m_authGetHostByName;
	}

	if(m_authService)
	{
		delete m_authService;
		m_authService = BD_NULL;
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
			case CREATE_ACCOUNT:
			{
				createAccountDescription(taskDescription);
				if(startKey)
				{
					startCreateAccount();
				}
			}
			break;
			case CHANGE_PASSWORD:
			{
				changePasswordDescription(taskDescription);
				if(startKey)
				{
					startChangePassword();
				}
			}
			break;
			case RESET_ACCOUNT:
			{
				resetAccountDescription(taskDescription);
				if(startKey)
				{
					startResetAccount();
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
			case GET_USERNAMES_FOR_LICENSE:
				{
					getUsernamesForLicenseDescription(taskDescription);
					if(startKey)
					{
						startGetUsernamesForLicense();
					}
				}
				break;
			case DELETE_ACCOUNT:
			{
				deleteAccountDescription(taskDescription);
				if(startKey)
				{
					startDeleteAccount();
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
		case CHANGE_PASSWORD:
		{
			changePasswordDescription(taskDescription);
			pumpChangePassword();
		}
		break;
		case RESET_ACCOUNT:
		{
			resetAccountDescription(taskDescription);
			pumpResetAccount();
		}
		break;
		case GET_USERNAMES_FOR_LICENSE:
			{
				getUsernamesForLicenseDescription(taskDescription);
				pumpGetUsernamesForLicense();
			}
			break;
		case AUTHENTICATE_ACCOUNT:
		{
			authenticateAccountDescription(taskDescription);
			pumpAuthenticateAccount();
		}
		break;
		case DELETE_ACCOUNT:
		{
			deleteAccountDescription(taskDescription);
			pumpDeleteAccount();
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
			nextTaskIfDone = CHANGE_PASSWORD;
			nextTaskIfFailed = CREATE_ACCOUNT;
			nextTaskIfSkipped = CHANGE_PASSWORD;			
		}
		break;
		case CHANGE_PASSWORD:
		{
			changePasswordDescription(m_lastTaskDescription);
			nextTaskIfDone = RESET_ACCOUNT;
			nextTaskIfFailed = CHANGE_PASSWORD;	
			nextTaskIfSkipped = RESET_ACCOUNT;
		}
		break;
		case RESET_ACCOUNT:
		{
			resetAccountDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_USERNAMES_FOR_LICENSE;
			nextTaskIfFailed = RESET_ACCOUNT;
			nextTaskIfSkipped = GET_USERNAMES_FOR_LICENSE;
		}
		break;
		case GET_USERNAMES_FOR_LICENSE:
		{
			getUsernamesForLicenseDescription(m_lastTaskDescription);
			nextTaskIfDone = AUTHENTICATE_ACCOUNT;
			nextTaskIfFailed = GET_USERNAMES_FOR_LICENSE;
			nextTaskIfSkipped = AUTHENTICATE_ACCOUNT;
		}
		break;
		case AUTHENTICATE_ACCOUNT:
		{
			authenticateAccountDescription(m_lastTaskDescription);
			nextTaskIfDone = DELETE_ACCOUNT;
			nextTaskIfFailed = AUTHENTICATE_ACCOUNT;
			nextTaskIfSkipped = DELETE_ACCOUNT;
		}
		break;
		case DELETE_ACCOUNT:
		{
			deleteAccountDescription(m_lastTaskDescription);
			nextTaskIfDone = STOP_NETWORK;
			nextTaskIfFailed = DELETE_ACCOUNT;
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
	}
}		

void Simulation::startCheckDNS()
{
	m_taskStatus = StartGetHostByName(AUTH_ADDRESS, &m_authGetHostByName, m_authAddr);	
}

void Simulation::pumpCheckDNS()
{
	if(m_authGetHostByName)
	{
		m_taskStatus = PumpGetHostByName(&m_authGetHostByName, m_authAddr);
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
		if(authService->createAccount(USERNAME, PASSWORD, LICENSE_CODE))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpCreateAccount()
{
	pumpAuthServiceTask();
}

void Simulation::changePasswordDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Change Password (" CHANGED_PASSWORD ")",	// READY
											"Changing Password",						// PENDING
											"Changed Password",							// DONE
											"Change Password Failed",					// FAILED
											"Change Password Skipped"};					// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startChangePassword()
{	
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->changePassword(USERNAME, PASSWORD, CHANGED_PASSWORD))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpChangePassword()
{
	pumpAuthServiceTask();
}

void Simulation::resetAccountDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Reset Account (" CHANGED_PASSWORD ")",	// READY
											"Resetting Account",					// PENDING
											"Account Reset",						// DONE
											"Reset Account Failed",					// FAILED
											"Reset Account Skipped"};				// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startResetAccount()
{	
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->resetAccount(USERNAME, CHANGED_PASSWORD, LICENSE_CODE))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpResetAccount()
{
	pumpAuthServiceTask();
}

void Simulation::authenticateAccountDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Authenticate Account (" USERNAME "/" CHANGED_PASSWORD ")",	// READY
											"Authenticating Account ",									// PENDING
											"Authenticated Account ",									// DONE
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
		if(authService->authorizeAccount(USERNAME, CHANGED_PASSWORD))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpAuthenticateAccount()
{
	pumpAuthServiceTask();
}

void Simulation::getUsernamesForLicenseDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Usernames for License",	// READY
											"Getting Usernames for License",									// PENDING
											"Got Usernames for License",									// DONE
											"Get Usernames for License Failed",								// FAILED
											"Get Usernames for License Skipped"};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == DONE){
		int resultCount = m_authService->m_usernamesForLicenseResult.getSize();
		description+="\nUsernames:\n";
		for(int i=0; i<resultCount; i++){
			description += m_authService->m_usernamesForLicenseResult[i];
			description += "\n";
		}
	}
}

void Simulation::startGetUsernamesForLicense()
{
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->getUsernamesForLicense(LICENSE_CODE))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpGetUsernamesForLicense()
{
	pumpAuthServiceTask();
}

void Simulation::deleteAccountDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Delete Account (" USERNAME ")",	// READY
											"Deleting Account",					// PENDING
											"Deleted Account",					// DONE
											"Delete Account Failed",			// FAILED
											"Delete Account Skipped"};			// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startDeleteAccount()
{	
	bdAuthService *const authService = getAuthService();
	m_taskStatus = FAILED;
	if(authService)
	{
		if(authService->deleteAccount(USERNAME, CHANGED_PASSWORD))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpDeleteAccount()
{
	pumpAuthServiceTask();
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

