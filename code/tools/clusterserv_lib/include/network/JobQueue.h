#pragma once

// very simple queue
#define JOB_QUEUE_SIZE	35

struct JobQueueElement
{
    BaseObject *pJob;
	int		iID;
};

class JobQueue
{
public:

			JobQueue();
	void	AddJob(int iID, BaseObject *pObj);
	bool	GetJob(int *, BaseObject **);

    JobQueueElement	m_Jobs[JOB_QUEUE_SIZE];
	int				m_iIn;
	int				m_iOut;
};
