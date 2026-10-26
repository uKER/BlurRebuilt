#include "CServ/ClusterServHeaders.h"
#ifdef WIN32
#include <crtdbg.h>
#endif

#define NUM_TEST_JOBS 10
#define JOB_SIZE (1024*200)
#define RESULT_SIZE (1024*200)

#include "Network/test.h"

IMPLEMENT_PERSISTENCE(Job,"Job");
IMPLEMENT_PERSISTENCE(Result,"Result");
IMPLEMENT_PERSISTENCE(Client,"Client");

void MemMirrorOn(void);

void Client::Initialize() 
{ 
//	std::cout << "Running Client::Initialize.\n";
}

BaseObject *Client::Process(BaseObject *pJob) 
{ 
//	std::cout << "Running Client::Process\n";

	Result *pResult = new Result();
	return pResult; 
}

void Client::CleanUp()	 
{ 
	float f = 10;
	printf("%5.2f\n", sinf(f));
//	std::cout << "Running Client::Cleanup.\n";
}

void Test()
{
	JobServer *pServer;
	ost::Thread *pClient;
	Client client;

	std::cout << "Starting test\n";

	// optionally specify a commandline that will run on screensaver clients and make them connect to us
	// if we leave this blank, screensaver will not be used
//	pServer->SetFarmClientCommandlineString( "c:/pgr3/tools/lpvsclient.exe /worker jan" );

	// start a server
	// pass in a pointer to the class that will be processing jobs on the client-side
	pServer = new JobServer( &client );

	// add a job to process to the server
	// the server will destroy the job once it has been processed
	for (int i=0; i<NUM_TEST_JOBS; i++)
	{
		Job *pJob = new Job();
		pServer->GetJobSchedulerPtr()->AddJob(i, 10, pJob);
	}

	std::cout << "press [ENTER] to continue.\n";
	getchar();

	pClient = StartClient();

	// wait until all jobs have been completed
	std::cout << "wait all jobs comp\n";
	pServer->WaitForAllJobsCompleted();
	std::cout << "all jobs completed\n";

	// retrieve results
	for (int i=0; i<NUM_TEST_JOBS; i++)
	{
		Result *pResult = (Result*)pServer->GetJobSchedulerPtr()->GetResult(i);
		delete pResult;
	}

	// deleting the server will make all clients exit
	pServer->Exit();	// make the server thead quit out
	delete pServer;		// delete the service thread

	// wait for our local client to exit
	std::cout << "Waiting for client to finish...\n";
	pClient->join();	// the client's socket will have disconnected so wait for it to automatically exit
	delete pClient;		// delete the service thread

	std::cout << "press [ENTER] to continue.\n";
	getchar();
}

void Server()
{
	JobServer *pServer;
	Client client;

	// start a server
	// pass in a pointer to the class that will be processing jobs on the client-side
	pServer = new JobServer( &client );

	// add a job to process to the server
	// the server will destroy the job once it has been processed
	Job *pJob = new Job();
	pServer->GetJobSchedulerPtr()->AddJob(1, 10, pJob);

	std::cout << "Server is active\n";

	// wait until all jobs have been completed
	std::cout << "wait all jobs comp\n";
	pServer->WaitForAllJobsCompleted();
	std::cout << "all jobs completed\n";

	// deleting the server will make all clients exit
	pServer->Exit();
	delete pServer;
}

void Client(char *pszTarget)
{
	std::cout << "client: connecting to " << pszTarget << "\n";

	// star a local client, which will connect to the server and process jobs for us
	ost::Thread *pClient = StartClient(pszTarget);

	std::cout << "Client is active\n";

	// give the thread a chance to start
	ost::Thread::sleep(1000);

	getchar();

	// wait for our local client to exit
	delete pClient;
}

int main( int argc , char *argv[] )
{
	bool bVerbose = false;

#if defined(WIN32) && defined(_DEBUG) && !defined(AMD64)
	MemMirrorOn();
	// turn on memory leak reporting
	_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );
#endif

	if (bVerbose)
	{
		g_netdebugManager.Create( "c:/netlog.txt" );
		g_netdebugManager.SetConsoleEnabledFlag(true);
		g_netdebugManager.SetLogFileEnabledFlag(true);
	}

	if (argc <= 1)
		Test();
	else if (!stricmp(argv[1], "server"))
		Server();
	else
		Client(argv[1]);

	if (bVerbose)
		g_netdebugManager.Destroy();
}
