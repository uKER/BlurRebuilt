#include "Network/stdafx.h"
#include "Network/JobScheduler.h"
#include "core\Threads.h"
#include <core/threads/atomic.h>
#include <algorithm>

using namespace std;

/// job
/// initialize job and set sema count to 1
JobScheduler::Job::Job() :
	m_pJob(NULL),
	m_pResult(NULL),
	m_iCost(0),
	m_iID(0),
	m_status(Job_NotStarted),
	m_RefCount(1)
{
}

/// increase the sema count
void JobScheduler::Job::Lock()
{
	MT::AtomicIncrement(&m_RefCount);
}

/// decrease the sema count and delete m_pJob if it reaches 0
void JobScheduler::Job::Release()
{
	if ((MT::AtomicDecrement(&m_RefCount))-1 == 0)
	{
		delete m_pJob;
		m_pJob = NULL;
	}
}

/// job scheduler
JobScheduler::JobScheduler() :
	m_nTotalJobs(0),
	m_nTotalCost(0),
	m_nJobsCompleted(0),
	m_nCostCompleted(0),
	m_nJobsSent(0),
	m_iLastJobStarted(0),
	m_pCallbackFunc(NULL),
	m_pJobCompletedCallback(NULL),
	m_Lock( new ost::Mutex( "JobScheduler" ))
{
}

JobScheduler::~JobScheduler()
{
	assert(AllJobsCompleted());

	delete m_Lock;
	// don't destroy results, because GetResult transfers ownership to the caller
}
	
// add a job to the processing queue
// you can either pass in the job object as pJob, or if pJob == NULL, the scheduler will call the job generation
// callback function to generate the job when it's needed
void JobScheduler::AddJob(int iID, int iCost, BaseObject *pJob)
{
	m_Lock->enter();

	m_Jobs.PushBack();

	Job &newJob = m_Jobs.Back();
	newJob.m_iID	 = iID;
	newJob.m_iCost	 = iCost;
	newJob.m_pJob	 = pJob;
	newJob.m_pResult = NULL;
	newJob.m_status  = Job_NotStarted;
	m_nTotalJobs++;
	m_nTotalCost += iCost;

	m_Lock->leave();
}

// sort jobs, highest cost gets dispatched first
void JobScheduler::SortJobs()
{
	if (m_Jobs.Size())
		sort(&m_Jobs[0], &m_Jobs[m_Jobs.Size()-1]);
}

// return the index of a job that needs doing
int JobScheduler::FindJobThatNeedsDoing()
{
	// look for jobs that haven't been started yet
	for (int i=m_iLastJobStarted; i<m_nTotalJobs; i++)
	{
		if (m_Jobs[i].m_status == Job_NotStarted)
		{
			return i;
		}
	}
	for (int i=0; i<m_iLastJobStarted; i++)
	{
		if (m_Jobs[i].m_status == Job_NotStarted)
		{
			return i;
		}
	}
	// look for jobs that have been started but haven't completed yet
	if (++m_iLastJobStarted >= m_nTotalJobs)
		m_iLastJobStarted = 0;
	for (int i=m_iLastJobStarted; i<m_nTotalJobs; i++)
	{
		if (m_Jobs[i].m_status == Job_InProgress)
		{
			return i;
		}
	}
	for (int i=0; i<m_iLastJobStarted; i++)
	{
		if (m_Jobs[i].m_status == Job_InProgress)
		{
			return i;
		}
	}

	return -1;
}

// get a job to send to the client for processing
bool JobScheduler::GetJob(int *piJob, BaseObject **ppObject)
{
	m_Lock->enter();

	int i = FindJobThatNeedsDoing();
	if (i == -1)
	{
		m_Lock->leave();
		return false;
	}

	m_iLastJobStarted = i;
	if (m_iLastJobStarted >= m_nTotalJobs)
		m_iLastJobStarted = 0;

	if (!m_Jobs[i].m_pJob)
	{
		assert(m_pCallbackFunc); // if this asserts, you forgot to call 'SetGenerateJobCallbackFunction'
		m_Jobs[i].m_pJob = m_pCallbackFunc( m_pCallbackContext, m_Jobs[i].m_iID );
	}

	m_Jobs[i].m_status = Job_InProgress;
	m_Jobs[i].Lock();	// user must call 'release' when done serialising the job
	*ppObject = m_Jobs[i].m_pJob;
	*piJob	  = i;
	m_nJobsSent++;

	m_Lock->leave();

	// if the job we got was a NULL job, call this function again
	if (*ppObject == NULL)
	{
		SetResult(i, NULL, "");
		return GetJob(piJob, ppObject);
	}

	return true;
}

// release the given job, and delete it if neccesary
void JobScheduler::ReleaseJob(int iJob)
{
	m_Jobs[iJob].Release();
}

// returns true if all jobs have been completed
bool JobScheduler::AllJobsCompleted()
{
	return (m_nJobsCompleted >= m_nTotalJobs);
}

// returns the percentage of jobs that have been completed
float JobScheduler::GetPercentageComplete()
{
	return ((float)m_nCostCompleted / (float)m_nTotalCost) * 100.0f;
}

// returns total number of jobs
int JobScheduler::GetNumJobs()
{
	return m_Jobs.Size();
}

// return a job's result
BaseObject *JobScheduler::GetResult(int i)
{
	assert(AllJobsCompleted());
	assert(m_Jobs[i].m_status == Job_Completed);

	return m_Jobs[i].m_pResult;
}

// get the hostname that rendered the job that we just received
const char* JobScheduler::GetCurrentJobHostname()
{
	return m_pszCurrentJobHostname;
}

// set a job's result
void JobScheduler::SetResult(int i, BaseObject *pResult, const char *pszHostname)
{
	assert(m_Jobs[i].m_status != Job_NotStarted);

	m_Lock->enter();

	// check whether we already have the result for this job
	if (m_Jobs[i].m_status == Job_Completed)
	{
		delete pResult;
		m_Lock->leave();
		return;
	}

	// since the job has been completed, we can delete it - just keep the result
	// TODO: fix possible race condition when deleting job while serialising it for sending at the same time
	if (m_Jobs[i].m_pJob)
		m_Jobs[i].Release();

	m_Jobs[i].m_status = Job_Completed;
	m_Jobs[i].m_pResult = pResult;
	m_nJobsCompleted++;
	m_nCostCompleted += m_Jobs[i].m_iCost;

	m_Lock->leave();

	m_pszCurrentJobHostname = pszHostname;

	// call the notification callback function
	if (m_pJobCompletedCallback)
		(*m_pJobCompletedCallback)(m_pJobCompletedCallbackContext, this, &m_Jobs[i]);
}

// cancel all jobs - any clients will be told to exit from now on
void JobScheduler::CancelAllJobs()
{
	m_Lock->enter();

	m_Jobs.Clear();
	m_nTotalJobs = 0;
	m_nJobsCompleted = 0;
	m_nJobsSent = 0;
	m_iLastJobStarted = 0;

	m_Lock->leave();
}

// set job generation callback function
void JobScheduler::SetGenerateJobCallbackFunction( TGenerateJobCallbackFunction *pCallbackFunc, void *pContext )
{
	m_pCallbackFunc = pCallbackFunc;
	m_pCallbackContext = pContext;
}

// set the job completed callback function
void JobScheduler::SetJobCompletedCallbackFunction(JobCompletedCallback *pCallback, void *pContext)
{
	m_pJobCompletedCallback			= pCallback;
	m_pJobCompletedCallbackContext	= pContext;
}

