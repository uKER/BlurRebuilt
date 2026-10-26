#include "Network/StdAfx.h"

#include <core/containers/string.h>
#include "Network/IServer.h"
#include "Network/JobScheduler.h"
#include <system\misc\progress.h>

// get pointer to job scheduler
JobScheduler *IServer::GetJobSchedulerPtr()
{
	return m_pScheduler;
}

// wait till all jobs have been completed
void IServer::WaitForAllJobsCompleted(Progress::I_ProgressBar *pProgress)
{
	Progress::UpdateSimple progress(pProgress, "Rendering PVS");

	// wait until all the jobs have been done
	int nDots = 0;
	while (!GetJobSchedulerPtr()->AllJobsCompleted())
	{
		float fProgress = GetJobSchedulerPtr()->GetPercentageComplete();

		char buffer[512];
		sprintf(buffer, "\r%5.2f%% completed", fProgress);
		if (pProgress)
			progress.SetProgress(fProgress);
		std::cout << buffer;
		for (int i=0; i<nDots; i++)
			std::cout << ".";
		std::cout << "   " << std::flush;
		nDots = (nDots+1)&3;

		ost::Thread::sleep(400);
	}
}
