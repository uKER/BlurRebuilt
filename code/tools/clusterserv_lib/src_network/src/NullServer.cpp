#include "Network/StdAfx.h"
#include "Network/Network.h"
#include "Network/Connection.h"
#include "Network/Server.h"
#include "Network/NetMessages.h"
#include "Network/Connection.h"
#include "Network/JobScheduler.h"
#include "Network/SerializeBin.h"
#include "Network/NullServer.h"
#include "Network/ClientInterface.h"

// constructor
NullServer::NullServer(BaseObject *pInitialUpdate)
{
	m_pInitialUpdate = (ClientInterface*)pInitialUpdate;
	m_pInitialUpdate->Initialize();
	m_pScheduler = new JobScheduler;
	m_bExitThread = false;
	start();
}

// deconstructor
NullServer::~NullServer()
{
	m_pInitialUpdate->CleanUp();
	delete m_pInitialUpdate;
	delete m_pScheduler;
}

// set a flag to make this thread exit
void NullServer::Exit()
{
	m_bExitThread = true;
	join();
}

// run a server on a separate thread which processes the jobs itself, rather than across the network
void NullServer::ProcessJobs()
{	
	while (!m_bExitThread)
	{
		BaseObject *pJob, *pResult;
		int iID;
		if (!m_pScheduler->GetJob( &iID, &pJob ))
		{
			// if there are no jobs, wait a second
			sleep(1000);
			continue;
		}

		// process a job
		pResult = m_pInitialUpdate->Process( pJob );

		m_pScheduler->ReleaseJob(iID);

		m_pScheduler->SetResult( iID, pResult, "" );
	};

	m_pInitialUpdate->CleanUp();
}

// thread entry point
void NullServer::run()
{
	ProcessJobs();
}
