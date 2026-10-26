#include "StdAfx.h"

#include "Lux\Lux.h"
#include "Lux\Core.h"
#include "CServ/ClusterServHeaders.h"
#include "Lux\GlobalIllumination\GlobalIlluminationNet.h"
#include <system\misc\progress.h>

using namespace LUX;

void InitServerPaths(RunServerStruct &net)
{
	// initialize to 0
	net.pszTmpFilename1[0] = 0;
	net.pszTmpFilename2[0] = 0;

	char pWindowsPath[MAX_PATH]; GetTempPathA(MAX_PATH, pWindowsPath); // [Blur2PC] local temp folder; the farm share is gone

#ifdef __GNUC__
	// no farm job spawning for linux version
#else
	// -- Windows version
	if (net.clientExePath != "")
	{
		// copy the file to a temporary folder on the network
		if (GetTempFileName(pWindowsPath, "lpvs_Win_", 0, net.pszTmpFilename1) == 0)
		{
			LOGWARN( g_Log, "Could not access server ('%s')\n", pWindowsPath);
		}
		else
		{
			if (CopyFile(net.clientExePath.c_str(), net.pszTmpFilename1, false))
			{
			}
			else
			{
				LOGWARN( g_Log, "Couldn't find remote client exe '%s'.\n", net.clientExePath.c_str());
			}
		}
	}
	// -- G5 version
	if (net.clientElfPath != "")
	{
		// copy the file to a temporary folder on the network
		if (GetTempFileName(pWindowsPath, "lpvs_G5_", 0, net.pszTmpFilename2) == 0)
		{
			LOGWARN( g_Log, "Could not access server ('%s')\n", pWindowsPath);
		}
		else
		{
			if (CopyFile(net.clientElfPath.c_str(), net.pszTmpFilename2, false))
			{
			}
			else
			{
				LOGWARN( g_Log, "Couldn't find remote client exe '%s'.\n", net.clientElfPath.c_str());
			}
		}
	}
#endif
}

void DeleteServerPaths(RunServerStruct &net)
{
#if defined(PLATFORM_WINDOWS)
	for (int i=0; i<2; i++)
	{
		char *psz = i?net.pszTmpFilename2:net.pszTmpFilename1;
		if (psz[0] == 0)
			continue;
		DWORD rtn = 0;
		int iTime = 5;
		while (!rtn && --iTime>0)
		{
			rtn = DeleteFile(psz);
			MT::Thread::Sleep(1000);
		}
	}
#endif
}

bool RunServer(RunServerStruct &net, Progress::I_ProgressBar *pProgress)
{
	JobScheduler *pScheduler = net.pServer->GetJobSchedulerPtr();

	Progress::UpdateSimple progress(pProgress, "Rendering");

	pScheduler->SortJobs();

	float fProgress;

	// TODO: wait here until the server has been started properly

	// start a farm job - make them connect to us
	if (net.bUseNetworkClients)
	{
#ifdef __GNUC__
		// no farm job spawning for linux version
#else
		// get host as a dotted IP address
		char buffer[512];
		gethostname(buffer, 512);
		ost::IPV4Address localhost = ost::InetHostAddress( buffer );
		const char *hostname = inet_ntoa(localhost.getAddress());

		char buffer2[512];

		// -- Windows version
		if (net.pszTmpFilename1[0] != 0)
		{
			sprintf(buffer2, "%s /worker %s", net.pszTmpFilename1, hostname);

			// add windows commandline
			net.pServer->AddFarmClientCommandline( buffer2, CLUSTERSERV_FLAG_WINDOWS);
		}
/*
		// disabling the G5's

		// -- G5 version
		if (net.pszTmpFilename2[0] != 0)
		{
			char *pLinuxPath =   "/mnt/farm/tmp/";

			sprintf(buffer2, "%s%s /worker %s", pLinuxPath, strrchr(net.pszTmpFilename2, '\\')+1, hostname);

			// add linux commandline
			net.pServer->AddFarmClientCommandline( buffer2, CLUSTERSERV_FLAG_LINUX);
		}
*/
#endif
	}

	// start server
	net.pServer->start();

	// start client
	net.pClient = 0;
	net.pClient = StartClient();

	LOGINFO( g_Log, "Rendering...\n");

	// wait until all the jobs have been done
	int nDots = 0;
	while (!pScheduler->AllJobsCompleted())
	{
		ost::Thread::sleep(400);
		fProgress = pScheduler->GetPercentageComplete();
		progress.SetProgress(fProgress);
		if (net.pLux->DoProgressCallback(fProgress,"Rendering lighting"))
		{
			pScheduler->CancelAllJobs();
			return false;
		}

//		printf("\r%5.2f%% completed - avg %d local lights per poly", fProgress, gnLightsCount/gnLightsCX);
		char buffer[512];
		sprintf(buffer, "\r%5.2f%% completed", fProgress);
		std::cout << buffer;
		for (int i=0; i<nDots; i++)
			std::cout << ".";
		std::cout << std::flush;
		printf("   ");
		nDots = (nDots+1)&3;
		// TODO: display a progress bar using m_nJobsCompleted
	}

	LOGINFO( g_Log, "Rendering completed.\n");

	net.pServer->Exit();

	return true;
}
