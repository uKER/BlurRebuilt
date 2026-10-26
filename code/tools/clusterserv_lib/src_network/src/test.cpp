#include "Network/StdAfx.h"
#include "Network/Network.h"
#include "Network/Connection.h"
#include "Network/Server.h"
#include "Network/NetMessages.h"
#include "Network/Connection.h"
#include "Network/JobScheduler.h"
#include "Network/SerializeBin.h"
#include "Network/Debug.h"
#include "Network/ClientInterface.h"
#include "Network/JobScheduler.h"
#include "Network/JobQueue.h"
#include "Network/Client.h"
#include "zlib/compression.h"
#include "Network/test.h"

#ifndef __GNUC__
#include "FarmLibrary/Farmer.h"
#endif

IMPLEMENT_PERSISTENCE(Job,"Job");
IMPLEMENT_PERSISTENCE(Result,"Result");
IMPLEMENT_PERSISTENCE(Client,"Client");

void Client::Initialize() 
{ 
	std::cout << "Running Client::Initialize.\n";
}

BaseObject *Client::Process(BaseObject *pJob) 
{ 
	std::cout << "Running Client::Process\n";

	Result *pResult = new Result();
	return pResult; 
}

void Client::CleanUp()	 
{ 
	std::cout << "Running Client::Cleanup.\n";
}

void test()
{
	// initialize winsock
	g_netdebugManager.SetConsoleEnabledFlag(true);

	JobServer *pServer;
	Client client;

	// optionally specify a commandline that will run on screensaver clients and make them connect to us
	// if we leave this blank, screensaver will not be used
//	pServer->SetFarmClientCommandlineString( "c:/pgr3/tools/lpvsclient.exe /worker jan" );

	// start a server
	// pass in a pointer to the class that will be processing jobs on the client-side
	std::cout << "new jobserver\n";

	pServer = new JobServer( &client );
	std::cout << pServer << " " << pServer->GetJobSchedulerPtr() << "\n";

	std::cout << "press [ENTER] to continue.\n";
	getchar();

	std::cout << pServer->GetJobSchedulerPtr() << "\n";
	Job *pJob = new Job();

	std::cout << "press [ENTER] to continue.\n";
	getchar();

	std::cout << pServer->GetJobSchedulerPtr() << "\n";
	// add a job to process to the server
	// the server will destroy the job once it has been processed
	pServer->GetJobSchedulerPtr()->AddJob(1, 10, pJob);

	std::cout << "press [ENTER] to continue.\n";
	getchar();

	// star a local client, which will connect to the server and process jobs for us
	ost::Thread *pClient = StartClient("127.0.0.1");

	// wait until all jobs have been completed
	pServer->WaitForAllJobsCompleted();
	// deleting the server will make all clients exit
	pServer->Exit();
	delete pServer;

	// wait for our local client to exit
	std::cout << "Waiting for client to finish...\n";
	pClient->join();
	delete pClient;

	std::cout << "finished\n";

	std::cout << "press [ENTER] to continue.\n";
	getchar();
}