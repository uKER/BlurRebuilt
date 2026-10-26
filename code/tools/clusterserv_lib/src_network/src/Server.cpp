#include "Network/StdAfx.h"
#include "Network/Network.h"
#include "Network/Connection.h"
#include "Network/Server.h"
#include "Network/NetMessages.h"
#include "Network/Connection.h"
#include "Network/JobScheduler.h"
#include "Network/SerializeBin.h"
#include "Network/Debug.h"
#include "zlib/compression.h"

#ifndef __GNUC__
#include <farm_library/Farmer.h>
#endif

ost::Semaphore *g_InitialUpdateSemaphore;

JobServer *CreateServer(BaseObject *pInitialUpdate)
{
	JobServer *pServer = new JobServer(pInitialUpdate);
	return pServer;
}

// compress and store the initial update object
InitialUpdateObject::InitialUpdateObject(BaseObject *pInitialUpdate)
{
	// wrap the object in a message
	NETMSG_INITIALUPDATE msg;
	msg.pData = pInitialUpdate;

	// serialize the message
	CBinarySerialize data;
	data.SerializeObject(&msg, 5*1024*1024);

/*	
	#define IU_PAD (1000*1000*100)

	// append dummy data to the end - this is to easily stress-test the app
	if (gbDebugMode)
	{
		BYTE *pOldData = data.m_pData;

		data.m_pData = new BYTE[data.m_iFileLength + IU_PAD];
		memcpy(data.m_pData, pOldData, data.m_iFileLength);
		// pad the data with random numbers, so it doesn't compress
		for (int i=data.m_iFileLength; i<data.m_iFileLength + IU_PAD; i++)
			data.m_pData[i] = rand();

		data.m_iFileLength += IU_PAD;
	}
*/	

	char 	*pCompressedData = new char[data.m_iFileLength];
	bool	bCompressed = true;

	LOGINFO( g_NetLog, "Compressing initial update...\n");
	// compress the message using zlib and store the result
	CCompression c;
	c.Compress(data.m_pData, data.m_iFileLength, &pCompressedData, &m_iCompressedSize);
	m_iOriginalSize = data.m_iFileLength;
	delete data.m_pData;

	m_pCompressedData = new char[m_iCompressedSize + 8];
	memcpy(m_pCompressedData + 8, pCompressedData, m_iCompressedSize);
	// set the total packet size
	((int*)m_pCompressedData)[0] = htonl( m_iCompressedSize + 4 );
	// set the original data length
	((int*)m_pCompressedData)[1] = htonl( m_iOriginalSize );

//	printf("Original size: %7dK\n",m_iOriginalSize/1000);
//	printf("Compressed:    %7dK\n",m_iCompressedSize/1000);

	delete pCompressedData;
	LOGINFO( g_NetLog, "Done.\n");
}

// constructor for the server
JobServer::JobServer(BaseObject *pInitialUpdate) :
	m_iLastClientID(0),
	m_InitialUpdate(pInitialUpdate),
	m_QuitThread(false),
	m_iMsSinceLastCheckedClient(0),
	C_Network()
{
	g_InitialUpdateSemaphore = new ost::Semaphore( MAX_SIMULTANEOUS_INITIAL_UPDATES );
	m_pScheduler = new JobScheduler;
}

void JobServer::Exit()
{
	// set a flag so the server thread exits itself
	m_QuitThread = true;
	Thread::join();
	for (int i=0; i<m_Services.Size(); i++)
	{
		m_Services[i]->cancel();
		m_Services[i]->join();
	}

	// wait for clients to exit and then delete them
	for (int i=0; i<m_ClientList.Size(); i++)
		delete m_ClientList[i];
}

JobServer::~JobServer()
{
	delete m_pScheduler;
	delete g_InitialUpdateSemaphore;
}

// start thread

// add a farm client command string
void JobServer::AddFarmClientCommandline(char *psz, int iJobFlag)
{
	FarmClientInfo info;
	info.iJobFlag = iJobFlag;
	info.sCommandline = psz;
	info.iJobID = -1;

	m_FarmClientInfo.PushBack(info);
}

Connection *gpServerConnection;

// a new client has connected - add it to our list
void JobServer::AddClient(ost::TCPSocket *pSocket)
{
	// create the new client and get a pointer to it
	m_ClientList.PushBack();
	int iNewClient = m_ClientList.Size()-1;
	Connection *pNewClient = new Connection(*pSocket, this, "server");
	m_ClientList[iNewClient] = pNewClient;
	pNewClient->m_iID = iNewClient;

	gpServerConnection = pNewClient;

	// send initial update to client
	Send(pNewClient, m_InitialUpdate.m_pCompressedData, m_InitialUpdate.m_iCompressedSize + 8, SendType_InitialUpdate);
}


#ifndef __GNUC__
// check whether a client is running and start it if it isn't
void JobServer::CheckFarmClientIsRunning(FarmClientInfo &info)
{
	// check whether the client is already running
	if (info.iJobID != -1)
	{
		if (GetJobStatus(info.iJobID, SCREENSAVER_PORT))
			return; // job exists
	}

	FarmClient farmClient(SCREENSAVER_PORT, SCREENSAVER_PORT);

	if (farmClient.FindServer())
	{
		info.iJobID = farmClient.StartJob(info.sCommandline.c_str(), "Job", info.iJobFlag, false);
	}
	else
		info.iJobID = -1;
}

// check farm clients are running, and create it if they aren't
void JobServer::CheckFarmClientIsRunning()
{
	U32 i = GetTickCount();
	if (i < m_iMsSinceLastCheckedClient + 15*1000)
		return; // only check once every 15 seconds
	m_iMsSinceLastCheckedClient = i;

	g_bScreenSaverMode = true;
	g_bSilentMode = true;

	// check that all farm client commandlines are added to the farm
	for (int i=0; i<m_FarmClientInfo.Size(); i++)
	{
		CheckFarmClientIsRunning(m_FarmClientInfo[i]);
	}
}
#endif

// if the job is still running, delete it
void JobServer::DeleteFarmClientJob()
{
#ifndef __GNUC__
	FarmClient farmClient(SCREENSAVER_PORT, SCREENSAVER_PORT);

	if (!farmClient.FindServer())
		return;

	// for each commandline that was added to the farm
	for (int i=0; i<m_FarmClientInfo.Size(); i++)
	{
		if (m_FarmClientInfo[i].iJobID != -1)
		{
			// delete it
			farmClient.KillJob(m_FarmClientInfo[i].iJobID);
		}
	}
#endif
}


// this function is called when a new connection to the server starts
void JobServer::run()
{
	// set up a TCPSocket structure, which monitors incoming connections
	ost::InetHostAddress machine;
	ost::TCPSocket socket( machine, NetworkPort );

	while (!m_QuitThread)
	{
#ifndef __GNUC__
		// optionally check whether our farm job still exists, and recreate it if it doesn't
		if (m_FarmClientInfo.Size())
            CheckFarmClientIsRunning();
#endif

		if (socket.isPendingConnection(1000))
		{
			// if we're already sending too many initial updates, don't make any new connections
			if (g_InitialUpdateSemaphore->wait(10*1000))
			{
				// make a new connection
				// once initial update is sent the semaphore will be released in the connection code
				AddClient(&socket);
			}
			else
			{
				// we are too busy. reject connection
				LOGDEBUG( g_NetLog, "Server too busy; rejecting connection\n");
				socket.reject();
				g_InitialUpdateSemaphore->post();
			}
		}
	}

	LOGDEBUG( g_NetLog, "exiting server\n");

	DeleteFarmClientJob();

	// count total number of jobs processed
	int nTotalJobsCompleted = 0;
	for (int i=0; i<m_ClientList.Size(); i++)
		nTotalJobsCompleted += m_ClientList[i]->m_nJobsCompleted;

	// closing client's sockets will make them exit
	NETMSG_QUIT quitmsg;
	for (int i=0; i<m_ClientList.Size(); i++)
	{
//		m_ClientList[i]->disconnect(); // set this client to disconnect
		LOGDEBUG( g_NetLog, "client '%s' jobs completed %d (%5.2f%%)", m_ClientList[i]->m_pszHostName, m_ClientList[i]->m_nJobsCompleted,
				100.f*(float)m_ClientList[i]->m_nJobsCompleted/(float)nTotalJobsCompleted);
	}
}

// process a message object
void JobServer::ProcessMessage(Connection *pConnection, BaseObject *pMsg)
{
//	g_netdebugManager << "SERVER:received a " << pMsg->getPersistenceID() << " object.\n";

	NETMSG_GENERIC *p = (NETMSG_GENERIC*)pMsg;
	p->Process(pConnection, this);

	delete pMsg;
}

