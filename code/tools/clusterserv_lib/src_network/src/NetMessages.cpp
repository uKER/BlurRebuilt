#include "Network/stdafx.h"
#include "Network/NetMessages.h"
#include "Network/JobScheduler.h"
#include "Network/Network.h"
#include "Network/Connection.h"
#include "Network/Server.h"
#include "Network/JobQueue.h"
#include "Network/Interface.h"
#include "Network/Client.h"
#include "Network/ClientInterface.h"
#include "Network/Debug.h"

IMPLEMENT_PERSISTENCE( NETMSG_GENERIC,			"__GEN");
IMPLEMENT_PERSISTENCE( NETMSG_JOB,				"__JOB");
IMPLEMENT_PERSISTENCE( NETMSG_JOB_RESULT,		"__RESULT");
IMPLEMENT_PERSISTENCE( NETMSG_REQUEST_JOB,		"__REQ");
IMPLEMENT_PERSISTENCE( NETMSG_INITIALUPDATE,	"__UPDATE");
IMPLEMENT_PERSISTENCE( NETMSG_JOB_CLIENTINFO,	"__INFO");
IMPLEMENT_PERSISTENCE( NETMSG_QUIT,				"__QUIT");

/*
 * Server-side message processing functions
 */

// this function is called when the server receives a job result
void NETMSG_JOB_RESULT::Process(Connection *pConnection, JobServer *p)
{
	p->GetJobSchedulerPtr()->SetResult(m_iJob, m_pResult, pConnection->m_pszHostName);

	pConnection->m_nJobsCompleted++;
}

// this function is called when the server receives a request for a job from a client
void NETMSG_REQUEST_JOB::Process(Connection *pConnection, JobServer *p)
{
	int iJob;
	BaseObject *pJob;
	if (p->GetJobSchedulerPtr()->GetJob(&iJob, &pJob))
	{
		// get a job and send it back
		NETMSG_JOB msg(iJob, pJob);
		pConnection->SendMessage(&msg);

		p->GetJobSchedulerPtr()->ReleaseJob(iJob);
	}
	else
	{
		// TODO: check whether any jobs are still in progress and if so, send one of those instead.
	}
}

// this function is called when the server receives the name of a client
void NETMSG_JOB_CLIENTINFO::Process(Connection *pConnection, JobServer *p)
{
//	strcpy(pConnection->m_pszHostName, m_csName.c_str());
	strcpy(pConnection->m_pszHostName, pConnection->getPeer().getHostname());
	S_DEBUG( g_NetLog, "connected: '" << pConnection->m_pszHostName << "'\n");
}


/*
 * Client-side message processing functions
 */
// client receiving a job to do
void NETMSG_JOB::Process(Connection *pConnection, JobClient *p)
{
	// save the new job in the job queue
	p->m_JobQueue.AddJob(m_iJob, m_pJob);

	// increase semaphore count
	p->m_JobCountSemaphore->post();
}

// client receiving the initial data (the integrator + spatial hierarchy)
void NETMSG_INITIALUPDATE::Process(Connection *pConnection, JobClient *p)
{
	// save the update info in the client class
	p->m_pInterface = (ClientInterface*)pData;

	// signal initial update received
	p->m_ReceivedInitialUpdateEvent->signal();
}

// message to clients saying we're quitting out
void NETMSG_QUIT::Process(Connection *pConnection, JobClient *p)
{
	// signal the client to quit out
	p->m_bCompletionError = true;
}

