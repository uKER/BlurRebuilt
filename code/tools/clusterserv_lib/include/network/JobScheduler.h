#ifndef __NETWORK_JOBSCHEDULER_H__
#define __NETWORK_JOBSCHEDULER_H__

class JobScheduler
{
public:
	// local typedefs
	// the status of a job
	enum JobStatus
	{
		Job_NotStarted,
		Job_InProgress,
		Job_Completed,
	};

	// defines a job
	class Job
	{
	public:
					/// initialize job and set sema count to 1
					Job();
					/// increase the sema count
		void		Lock();
					/// decrease the sema count and delete m_pJob if it reaches 0
		void		Release();

		BaseObject	*m_pJob;
		BaseObject	*m_pResult;
		int			m_iCost;
		int			m_iID;
		JobStatus	m_status;
		int			m_RefCount;	// to prevent deleting a job that is being serialised

		bool operator<(const Job &e) const { return m_iCost > e.m_iCost; }
	};

	// job generation callback function type
	typedef BaseObject *TGenerateJobCallbackFunction(void *pContext, int iID);
	// job completion callback function type
	typedef void JobCompletedCallback(void *pContext, JobScheduler *pScheduler, JobScheduler::Job *pJob);

	// function definitions
				JobScheduler();
				~JobScheduler();

	// add a job to the processing queue
	void		AddJob(int iID, int iCost, BaseObject *pJob = NULL);
	// retrieve the result of a processed job
	BaseObject	*GetResult(int i);
	// returns true if all jobs have been completed
	bool		AllJobsCompleted();
	// returns the percentage of jobs that have been completed
	float		GetPercentageComplete();
	// returns the number of jobs that have been added
	int			GetNumJobs();
	// set the callback function, which is called when a job needs to be generated
	void		SetGenerateJobCallbackFunction( TGenerateJobCallbackFunction *pCallbackFunc, void *pContext );
	// set the callback function, which is called when a job has been completed
	void		SetJobCompletedCallbackFunction(JobCompletedCallback *pCallback, void *pContext);
	// cancels all jobs
	void		CancelAllJobs();
	// sort jobs by cost
	void		SortJobs();
	// release the lock on a job (after serialising it)
	void		ReleaseJob(int iJob);
	// get the hostname that rendered the job that we just received (to be called from job callback function)
	const char* GetCurrentJobHostname();

protected:
	// allow networking code to retrieve jobs and set results
	friend class NETMSG_JOB_RESULT;
	friend class NETMSG_REQUEST_JOB;
	friend class NullServer;

	bool		GetJob(int *piJob, BaseObject **ppObject);
	void		SetResult(int i, BaseObject *pResult, const char *pszHostname);
private:
	// list of jobs
	typedef Ut::UtVector< Ut::DynamicVector<Job> > JobList;

	JobList		m_Jobs;

	int			m_nTotalJobs;
	int			m_nTotalCost;
	int			m_nJobsSent;
	int			m_nJobsCompleted;
	int			m_nCostCompleted;
	int			m_iLastJobStarted;

	int			FindJobThatNeedsDoing();

	TGenerateJobCallbackFunction	*m_pCallbackFunc;
	void							*m_pCallbackContext;

	JobCompletedCallback	*m_pJobCompletedCallback;
	void					*m_pJobCompletedCallbackContext;

	ost::Mutex *m_Lock;

	const char *m_pszCurrentJobHostname;
};

#endif
