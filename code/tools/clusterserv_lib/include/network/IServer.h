#ifndef __ISERVER_H__
#define __ISERVER_H__

namespace Progress
{
class I_ProgressBar;
};

// common interface for cluster servers
// used by the normal cluster server and the dummy "null" server
class JobScheduler;
class IServer
{
public:
	virtual ~IServer() {};


	// get pointer to job scheduler
	JobScheduler *GetJobSchedulerPtr();

	// wait till all jobs have been completed (prints a progress bar)
	void WaitForAllJobsCompleted(Progress::I_ProgressBar *pProgress);

	// set a flag which will make the server thread exit
	virtual void Exit() = 0;

protected:
	JobScheduler		*m_pScheduler;	// needs to be initialized / freed by the inheriting class
};

#endif
