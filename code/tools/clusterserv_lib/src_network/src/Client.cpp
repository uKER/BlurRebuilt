#include "Network/StdAfx.h"
#include "Network/Network.h"
#include "Network/Connection.h"
#include "Network/JobQueue.h"
#include "Network/Interface.h"
#include "Network/Client.h"
#include "Network/NetMessages.h"
#include "Network/ClientInterface.h"
#include "Network/Debug.h"

#include <farm_library/Farmer.h>
#include <system\memory\MemoryTracker.h>

IMPLEMENT_PERSISTENCE(ClientInterface, "ClientInte");

// start client in a separate thread
ClientThread *StartClient(char *pszIP)
{
	JobClient *pClient = new JobClient(pszIP);
	pClient->start();
	return pClient;
}

JobClient::JobClient(char *pszAddr) :
	m_pConnection(NULL),
	m_bDone(false),
	m_nClientStall(0)
{
	m_ServerAddr = ost::InetHostAddress(pszAddr);
	m_bCompletionError = false;
	m_iExitCode = 0;

	m_ReceivedInitialUpdateEvent = new ost::Event();
	m_JobCountSemaphore = new ost::Semaphore( 0 );
}

JobClient::~JobClient()
{
	// stop the io completion thread
	// (check whether it is running. If we didn't manage to connect it might not be)
	for (int i=0; i<m_Services.Size(); i++)
	{
		m_Services[i]->cancel();
		m_Services[i]->join();
	}

	m_bCompletionError = true;

	delete m_JobCountSemaphore;
	delete m_ReceivedInitialUpdateEvent;

	if (m_pConnection)
		delete m_pConnection;
}

// start thread
Connection *gpClientConnection;

// send a message to the server
void JobClient::SendMessage(BaseObject *pMsg)
{
	m_pConnection->SendMessage(pMsg);
}

// main job client processing function
void JobClient::run()
{
	LOGDEBUG( g_NetLog, "client: connecting\n");

	m_pConnection = new Connection("client");
	unsigned int iClock = ost::getTicks();
	int iRetry = 120;
	// try connecting a few times
	while (m_pConnection->Connect(m_ServerAddr, NetworkPort, this) == false)
	{
		unsigned int iCur = ost::getTicks();
		if (iCur - iClock > (1000*120) )
		{
			m_iExitCode = 2;
			return;	// give up
		}
	}

	gpClientConnection = m_pConnection;

	// send our client info
	NETMSG_JOB_CLIENTINFO clientInfoMsg;

	ost::IPV4Address localhost = m_pConnection->getLocal();
	char buffer[512];
	strcpy(buffer, localhost.getHostname());
	gethostname(buffer,512);
	clientInfoMsg.m_csName = buffer;
	SendMessage(&clientInfoMsg);

	// request a couple of jobs to get us started
	NETMSG_REQUEST_JOB requestMsg;
	for (int i=0; i<JOB_QUEUE_SIZE-1; i++)
		SendMessage(&requestMsg);

	// wait for us to receive the initial update from the server
	int t=0;
	while ( !m_ReceivedInitialUpdateEvent->wait(1000)  )
	{
		// check for error out, or timeout at 5 minutes if we didn't receive initial update
		if (m_bCompletionError || t>300)
		{
			LOGDEBUG( g_NetLog, "CLIENT: Timed out.\n");
			m_iExitCode = 1;
			return;
		}
		t++;
	}

	LOGDEBUG( g_NetLog, "client: received initial update\n");

	Memory::Tracker::Instance().DumpNodeStats(false, true);

	// run initialization
	m_pInterface->Initialize();

#ifndef COMPILER_VC
	LOGDEBUG (g_NetLog, "client: initialised\n");
#endif

	// process jobs here
	while (!m_bCompletionError)
	{
		NETMSG_JOB_RESULT resultMsg;
		BaseObject *pJob;
		// there might not be any jobs available right now, in which case wait a second and try again
		if (!m_JobCountSemaphore->wait( 100 ))
		{
			m_nClientStall += 1;
			continue;
		}
		// get a job to process
		if (!m_JobQueue.GetJob(&resultMsg.m_iJob, &pJob))
		{
			assert(0);	// should never happen
		}

		// process this job
#ifndef COMPILER_VC
		LOGDEBUG( g_NetLog, "client: Processing job\n");
#endif
		resultMsg.m_pResult = m_pInterface->Process(pJob);
		if (resultMsg.m_pResult == NULL)
			break;
		// send the result
#ifndef COMPILER_VC
		LOGDEBUG( g_NetLog, "client: Sending job result\n");
#endif
		SendMessage(&resultMsg);
		delete resultMsg.m_pResult;
		delete pJob;
		// request a new job
		SendMessage(&requestMsg);
	}

	m_pInterface->CleanUp();

	delete m_pInterface;
	S_DEBUG( g_NetLog, "client: Stalled " << m_nClientStall*100 << " ms\n");

	// delete any jobs that haven't been processed yet
	int dummy;
	BaseObject *pObj;
	while (m_JobQueue.GetJob(&dummy, &pObj))
		delete pObj;

	LOGDEBUG( g_NetLog, "client exited\n");
}

// process a message object
void JobClient::ProcessMessage(Connection *pConnection, BaseObject *pMsg)
{
//	g_netdebugManager << "CLIENT:received a " << pMsg->getPersistenceID() << " object.\n";

	NETMSG_GENERIC *p = (NETMSG_GENERIC*)pMsg;
	p->Process(pConnection, this);

	delete pMsg;
}


