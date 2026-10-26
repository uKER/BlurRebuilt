// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"

// Sony includes
#include <sysutil/sysutil_common.h>
#include <cell/sysmodule.h>
#include <np.h>

#define TITLE_ID (0x13AB)
#define NP_SERVICE_ID "YOUR_NP_SERVICE_ID_HERE"

#define NP_POOL_SIZE (128*1024)
bdUByte8 np_pool[NP_POOL_SIZE];

NPAuthStatus g_authStatus = IDLE;
bdInt g_ticketSize = 0;

void BD_CALL NPManagerCallback(bdInt NPEvent,
							   bdInt result,
							   void *arg)
{
	if(NPEvent == SCE_NP_MANAGER_EVENT_GOT_TICKET)
	{
		if(result > 0)
		{
			g_authStatus = GOT_NP_TICKET;
		}
		else
		{
			g_authStatus = IDLE;
		}

		g_ticketSize = result;
	}
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

	ok  = ok && (cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP) == 0);
	ok = ok && (sceNpInit(NP_POOL_SIZE, np_pool) == 0);

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

	cellSysutilCheckCallback();

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
			nextTaskIfDone = AUTHENTICATE_ACCOUNT;
			nextTaskIfFailed = CHECK_DNS;
			nextTaskIfSkipped =AUTHENTICATE_ACCOUNT;
			displayLobbyError = false;
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

		// platform specific startup parameters
		bdPlatformParams platformParams;
		platformParams.m_NPSignIn = true;
		
		const bdInt ret = bdExamplePlatformInit::init(params, &platformParams);
		if(ret == 0 && net->start(params))
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

		// no need to keep the bdAuthService instance
		delete m_authService;
		m_authService = BD_NULL;
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
			const bdLobbyErrorCode lobbyErrorCode = authService->getErrorCode();
			if(lobbyErrorCode == BD_AUTH_NO_ERROR)
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
	const bdNChar8 *const descriptions[] = {"Authenticate Account",				// READY
											"Authenticating Account ",			// PENDING
											"Authenticated Account ",			// DONE
											"Authenticate Account Failed",		// FAILED
											"Authenticate Account Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == READY)
	{
		SceNpId npId;
		const bdInt ret = sceNpManagerGetNpId(&npId);
		if(ret == 0)
		{
			description += " (";
			description += npId.handle.data;
			description += ")";
		}		
	}
}

void Simulation::startAuthenticateAccount()
{
	// fail safe
	m_taskStatus = FAILED;
	g_authStatus = IDLE;

	SceNpId npId;
	bdInt ret = sceNpManagerGetNpId(&npId);
	if(ret == 0)
	{
		bdAuthService *const authService = getAuthService();
		if(authService)
		{
			ret = sceNpManagerUnregisterCallback();
			ret = sceNpManagerRegisterCallback(NPManagerCallback, BD_NULL);

			bdUInt cookieSize = 0;
			const bdByte8 *cookie = authService->getPS3Cookie(cookieSize);

			g_authStatus = REQUESTING_NP_TICKET;
			g_ticketSize = 0;			
			ret = sceNpManagerRequestTicket(&npId, NP_SERVICE_ID, cookie, cookieSize, BD_NULL, 0);			
			if(ret == 0)
			{
				m_taskStatus = PENDING;
			}			
		}
	}
}

void Simulation::pumpAuthenticateAccount()
{
	// fail safe
	m_taskStatus = FAILED;

	switch(g_authStatus)
	{
		case IDLE:
		{	
		}
		break;
		case REQUESTING_NP_TICKET:
		{
			m_taskStatus = PENDING;
		}
		break;
		case GOT_NP_TICKET:
		{
			size_t ticketSize = 0;
			bdInt ret = sceNpManagerGetTicket(BD_NULL, &ticketSize);
			if(ret == 0)
			{
				BD_ASSERT(static_cast<bdInt>(ticketSize) == g_ticketSize, "NP ticket size mismatch");
			}

			bdUByte8 *const ticketBuffer = bdAllocate<bdUByte8>(ticketSize);
			ret = sceNpManagerGetTicket(ticketBuffer, &ticketSize);

			bdAuthService *const authService = getAuthService();
			if((ret == 0) && authService)
			{
				const bdPS3AuthInfoRef ps3AuthInfo = new bdPS3AuthInfo();
				if (authService->authorizePS3Ticket(ticketBuffer, ticketSize, ps3AuthInfo))
				{
					m_taskStatus = PENDING;
					g_authStatus = DW_AUTHORIZING;
				}
			}
			bdDeallocate<bdUByte8>(ticketBuffer);
		}
		break;
		case DW_AUTHORIZING:
		{
			pumpAuthServiceTask();
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

