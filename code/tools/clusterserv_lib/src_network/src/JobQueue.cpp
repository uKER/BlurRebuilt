#include "Network/stdafx.h"
#include "Network/JobQueue.h"

// first-in-first-out queue with a fixed maximum size
JobQueue::JobQueue()
{
	// set all jobs to 0
	for (int i=0; i<JOB_QUEUE_SIZE; i++)
	{
		m_Jobs[i].pJob = NULL;
		m_Jobs[i].iID  = 0;
	}
	m_iIn = m_iOut = 0;
}

// add a job
void JobQueue::AddJob(int iID, BaseObject *pObj)
{
	m_Jobs[m_iIn].iID  = iID;
	m_Jobs[m_iIn].pJob = pObj;

	if (++m_iIn >= JOB_QUEUE_SIZE)
		m_iIn = 0;
	assert(m_iIn != m_iOut);
}

// get a job
bool JobQueue::GetJob(int *piID, BaseObject **ppObj)
{
	if (m_iIn == m_iOut)
		return false;	// no jobs currently available
	
	*ppObj = m_Jobs[m_iOut].pJob;
	*piID  = m_Jobs[m_iOut].iID;
	if (++m_iOut >= JOB_QUEUE_SIZE)
		m_iOut = 0;

	return true;
}
