// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"

#define TITLE_ID (0x13D3)
#define BD_WII_USER_NAME  "DEMONWARE_WII_TOKEN_AUTH_USER"
#define BD_WII_USER_ID (007u)

// Wii required defines.
#define GAME_CODE		'12AB'		// Game Code


//////////////////////////////////////////////////////////////////////////////
// NB NB NB
//
//  The game code must also be set in the *.ddf file on the Wii in order for
//  DWC_SvlGetTokenAsync() to work
//
// NB NB NB
//////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------*
  Function for allocating memory
 *---------------------------------------------------------------------------*/
static void*
AllocFunc( DWCAllocType name, u32   size, int align )
{
    void * ptr;
    (void)name;
    (void)align;

    ptr = bdMemory::alignedAllocate2(size, 32); ;

    return ptr;
}

/*---------------------------------------------------------------------------*
  Function for freeing memory
 *---------------------------------------------------------------------------*/
static void
FreeFunc( DWCAllocType name, void* ptr,  u32 size  )
{
    (void)name;
    (void)size;

    if ( !ptr ) return;

	bdMemory::alignedDeallocate2(ptr);

}

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
		// incase the task fails or completes in a single loop
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

	delete m_authGetHostByName;
	delete m_authService;
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
			case NAS_LOGON:
			{
				NASLogonDescription(taskDescription);
				if(startKey)
				{
					startNASLogon();
				}
			}
			break;
			case GET_SVL_TOKEN:
			{
				getSVLTokenDescription(taskDescription);
				if(startKey)
				{
					startGetSVLToken();
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
		case NAS_LOGON:
		{
			NASLogonDescription(taskDescription);
			pumpNASLogon();
		}
		break;
		case GET_SVL_TOKEN:
		{
			getSVLTokenDescription(taskDescription);
			pumpGetSVLToken();
		}
		break;
		case AUTHENTICATE_ACCOUNT:
		{
			authenticateAccountDescription(taskDescription);
			pumpAuthenticateAccount();
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
			nextTaskIfDone = NAS_LOGON;
			nextTaskIfFailed = CHECK_DNS;
			nextTaskIfSkipped =NAS_LOGON;
			displayLobbyError = false;
		}
		break;
		case NAS_LOGON:
		{
			NASLogonDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_SVL_TOKEN;
			nextTaskIfFailed = NAS_LOGON;
			nextTaskIfSkipped = GET_SVL_TOKEN;
		}
		break;
		case GET_SVL_TOKEN:
		{
			getSVLTokenDescription(m_lastTaskDescription);
			nextTaskIfDone = AUTHENTICATE_ACCOUNT;
			nextTaskIfFailed = GET_SVL_TOKEN;
			nextTaskIfSkipped = AUTHENTICATE_ACCOUNT;
		}
		break;
		case AUTHENTICATE_ACCOUNT:
		{
			authenticateAccountDescription(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = STOP_NETWORK;
			nextTaskIfFailed = AUTHENTICATE_ACCOUNT;
			nextTaskIfSkipped = STOP_NETWORK;
		}
		break;
		case STOP_NETWORK:
		{
			stopNetworkDescription(m_lastTaskDescription);
			nextTaskIfDone = START_NETWORK;
			nextTaskIfFailed = STOP_NETWORK;
			nextTaskIfSkipped = AUTHENTICATE_ACCOUNT;
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


		bdExamplePlatformInit::init(params );
		if(net->start(params))
		{
			m_taskStatus = PENDING;
		}
		else
		{
			bdExamplePlatformInit::quit();
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
	if(m_taskStatus == PENDING)
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


void Simulation::checkAuthServiceError()
{
	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		m_lobbyErrorCode = authService->getErrorCode();

		// no need to keep the bdAuthService instance
		delete m_authService;
		m_authService = BD_NULL;
	}
}

void Simulation::NASLogonDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Nintendo Authentication Server[NAS] Logon ",			// READY
											"Logging on to NAS",			// PENDING
											"NAS Logon Complete",	// DONE
											"NAS Logon Failed",		// FAILED
											"NAS Logon Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}

void Simulation::startNASLogon()
{
	// sneaking in the blocking call to DWC_INIT here !!
	int res = DWC_Init( DWC_AUTHSERVER_DEBUG, "testinguser", GAME_CODE, AllocFunc, FreeFunc );

	const bdBool loginRes = DWC_NasLoginAsync();
	if(false == loginRes)
	{
		BD_ERR(BD_LOG_LEVEL, "Failed to start NAS logon.");
		m_taskStatus = FAILED;
	}
	else
	{
		m_taskStatus = PENDING;
	}
}

void Simulation::pumpNASLogon()
{
	const DWCNasLoginState state = DWC_NasLoginProcess();
	if (state != DWC_NASLOGIN_STATE_HTTP)
	{
		DWCErrorType	dwcerrortype;
		int				errorcode;

		DWCError dwcerror = DWC_GetLastErrorEx(&errorcode, &dwcerrortype);
		if(DWC_ERROR_NONE == errorcode)
		{
			m_taskStatus = DONE;
		}
		else
		{
			bdPrintf("DWCError = %d, errorcode = %d, DWCErrorType = %d\n", dwcerror, errorcode, dwcerrortype);

			m_taskStatus = FAILED;
			BD_ERR(BD_LOG_LEVEL, "NAS Logon Failed.");
		}

	}
}

void Simulation::getSVLTokenDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Service Locator[SVL] Token ",				// READY
											"Getting SVL Token",			// PENDING
											"Getting SVL Token Complete",			// DONE
											"Getting SVL Token Failed",		// FAILED
											"Getting SVL Token Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetSVLToken()
{
	// The service type should be left blank.
	bdNChar8* serviceType = "";
	const bdBool getTokenStarted = DWC_SvlGetTokenAsync(serviceType , &m_NASTokenResult);
	if(false == getTokenStarted)
	{
		BD_ERR(BD_LOG_LEVEL, "Failed to start get token.");
		m_taskStatus = FAILED;
	}
	else
	{
		m_taskStatus = PENDING;
	}

}

void Simulation::pumpGetSVLToken()
{
	const DWCSvlState state = DWC_SvlProcess();
	if(state == DWC_SVL_STATE_SUCCESS)
	{
		bdPrintf("Successed to get SVL Status\n");
		bdPrintf("status = %s\n", m_NASTokenResult.status==TRUE ? "TRUE" : "FALSE");
		bdPrintf("svlhost = %s\n", m_NASTokenResult.svlhost);
		bdPrintf("svltoken = %s\n", m_NASTokenResult.svltoken);

		m_taskStatus = DONE;
    }
	else if(state == DWC_SVL_STATE_ERROR)
	{
		DWCErrorType	dwcerrortype;
		int				errorcode;

		DWCError dwcerror = DWC_GetLastErrorEx(&errorcode, &dwcerrortype);
		bdPrintf("Failed to get SVL Token\n");
		bdPrintf("DWCError = %d, errorcode = %d, DWCErrorType = %d\n", dwcerror, errorcode, dwcerrortype);

		m_taskStatus = FAILED;
	}
	else if(state == DWC_SVL_STATE_CANCELED)
	{
		bdPrintf("SVL canceled.\n");
		m_taskStatus = FAILED;

	}

}

void Simulation::authenticateAccountDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Authenticate Account",				// READY
											"Authenticating Account ",			// PENDING
											"Authenticated Account ",			// DONE
											"Authenticate Account Failed",		// FAILED
											"Authenticate Account Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}

void Simulation::startAuthenticateAccount()
{
	// fail safe
	m_taskStatus = FAILED;

	u8 macAddrBuffer[NCD_MACADDRESS_LENGTH];
	s32 result = NCDiGetWirelessMacAddress(macAddrBuffer);

	// Wii Token authentication uses "authentication by assertion"
	//    - so the game code must supply the username & pasword to use
	const bdOnlineUserID userID = BD_WII_USER_ID;
	const bdNChar8* userName = BD_WII_USER_NAME;

	// Wii Token authentication currently uses the MAC address of the console
    // as the encryption key.
	bdMACAddr macAddr;
	bdMemcpy(macAddr.m_data , macAddrBuffer , BD_MAC_ADDR_SIZE);

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		const bdBool authRes = authService->authorizeNASToken(m_NASTokenResult.svltoken , macAddr, userID, userName);
		if(authRes == false)
		{
			m_taskStatus = FAILED;
		}
		else
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpAuthenticateAccount()
{

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->getStatus() == bdAuthService::BD_READY)
		{
			const bdLobbyErrorCode authError = authService->getErrorCode();

			//	We should handle these errors.
			//	BD_AUTH_WII_TOKEN_VERIFICATION_FAILED
			//	BD_AUTH_WII_AUTHENTICATION_FAILED
			//	BD_AUTH_BAD_REQUEST

			if(BD_AUTH_NO_ERROR == authError )
			{
				m_taskStatus = DONE;
				// everything is good, hurrah
			}
			else if(BD_AUTH_BAD_REQUEST == authError )
			{
				m_taskStatus = FAILED;
				BD_ERR(BD_LOG_LEVEL, "Auth server recieved badly formed request.");
			}
			else if(BD_AUTH_WII_TOKEN_VERIFICATION_FAILED == authError )
			{
				m_taskStatus = FAILED;
				BD_ERR(BD_LOG_LEVEL, "Auth server failed to decrypt the SVL token.");
			}
			else if(BD_AUTH_WII_AUTHENTICATION_FAILED == authError )
			{
				m_taskStatus = FAILED;
				BD_ERR(BD_LOG_LEVEL, "Auth server failed to decrypt the bdWiiAuthInfo.");
			}
			else
			{
				m_taskStatus = FAILED;
				// somethign unexpected has happened.
				// most likely the auth server version os not new enough to handle Wii auth
				BD_ERR(BD_LOG_LEVEL, "Auth server failed in an unexpected manner.");
			}
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

