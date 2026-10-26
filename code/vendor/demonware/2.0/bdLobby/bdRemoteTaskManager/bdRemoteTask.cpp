// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

#define BD_LOG_LEVEL "remote task"

/// Default timeout in seconds for remote tasks 
/// a value of 0 or less means an infinite timeout
#define BD_DEFAULT_REMOTE_TASK_TIMEOUT (0.0f)

bdRemoteTask::bdRemoteTask()
: m_timeout(BD_DEFAULT_REMOTE_TASK_TIMEOUT),
  m_status(BD_EMPTY),
  m_taskResult(BD_NULL),
  m_taskResultList(BD_NULL),
  m_numResults(0),
  m_maxNumResults(0),
  m_totalNumResults(0),
  m_transactionID(0),
  m_errorCode(BD_NO_ERROR)
{
	m_timer.reset();
}

bdRemoteTask::~bdRemoteTask()
{
}


bdRemoteTask::bdStatus bdRemoteTask::getStatus()
{
	// if pending check the timeout
	if(m_status == BD_PENDING)
	{
		const bdFloat32 elapsedTime = m_timer.getElapsedTimeInSeconds();
		if((m_timeout > 0) && (elapsedTime >= m_timeout))
		{
			m_status = BD_TIMED_OUT;
			m_byteResults = BD_NULL;
			BD_INFO(BD_LOG_LEVEL, "Remote task timed out after %.3fs.", m_timeout);
		}
	}
	
	return m_status;
}


bdUInt bdRemoteTask::getNumResults() const
{
	return m_numResults;
}

bdUInt bdRemoteTask::getTotalNumResults() const
{
	return m_totalNumResults;
}

void bdRemoteTask::cancelTask()
{
	m_status = BD_CANCELLED;
}

bdByteBufferRef bdRemoteTask::getByteResults() const
{
	return m_byteResults;
}

bdFloat32 bdRemoteTask::getTimeout() const
{
	return m_timeout;
}

void bdRemoteTask::setTimeout(const bdFloat32 timeout)
{
	m_timeout = timeout;
}

bdUInt64 bdRemoteTask::getTransactionID() const
{
	return m_transactionID;
}

bdLobbyErrorCode bdRemoteTask::getErrorCode() const
{
	return m_errorCode;
}

void bdRemoteTask::setTaskResult(	bdTaskResult *result,
									const bdUInt numResults)
{
	m_taskResult = result;
	m_maxNumResults = numResults;
}

void bdRemoteTask::setTaskResultList(bdTaskResult **result,
                                     const bdUInt numResults)
{
	m_taskResultList = result;
	m_maxNumResults = numResults;
}

bdTaskResult *bdRemoteTask::getTaskResult()
{
	return m_taskResult;
}

void bdRemoteTask::handleTaskReply(const bdByteBufferRef buffer)
{
	bdBool ok;
	m_status = BD_FAILED;
	bdUInt64 transactionID = 0;
	ok = buffer->readUInt64(transactionID);
	if(ok)
	{
		m_transactionID = transactionID;
		BD_INFO(BD_LOG_LEVEL, "Received reply on\n"
							  "transaction ID: " BD_UINT_64_FORMAT, transactionID);
	}
	bdUInt32 taskErrorCode = BD_MAX_ERROR_CODE;
	ok = buffer->readUInt32(taskErrorCode);
	if (ok)
	{
		m_errorCode = static_cast<bdLobbyErrorCode>(taskErrorCode);
		if (m_errorCode == BD_NO_ERROR)
		{
			bdUByte8 operation;
			ok = buffer->readUByte8(operation);
			if(ok)
			{
				// set task to done regardless of numresults
				m_status = BD_DONE;

				bdUInt32 numReplies = 0;
				ok = buffer->readUInt32(numReplies);
				
				if (ok && (numReplies > 0))
				{
					m_numResults = numReplies;

					ok = buffer->readUInt32(m_totalNumResults);
					if (ok && m_taskResult)
					{
						bdUInt resultSize = m_taskResult->sizeOf();
						bdByte8 *taskResult = reinterpret_cast<bdByte8*>(m_taskResult);
						
						bdUInt numResultsToRead = BD_MIN(m_numResults, m_maxNumResults);
						for (bdUInt i =0; i < numResultsToRead; i++)
						{
							((bdTaskResult*)taskResult)->deserialize(buffer);
							taskResult += resultSize;
						}
					}
					else if (ok && m_taskResultList)
					{
						bdUInt numResultsToRead = BD_MIN(m_numResults, m_maxNumResults);
						for (bdUInt i =0; i < numResultsToRead; i++)
						{
							m_taskResultList[i]->deserialize(buffer);
						}
					}
					else
					{
						BD_WARN(BD_LOG_LEVEL, "Could not handle task result");
					}
					
				}
			}
		}
		else
		{
			m_status = BD_FAILED;
#if defined BD_DEBUG
			BD_WARN(BD_LOG_LEVEL, "Transaction ID " BD_UINT_64_FORMAT " failed.", m_transactionID);
#endif // BD_DEBUG
		}
	}
}


void bdRemoteTask::start(const bdFloat32 timeout)
{
	m_timer.start();
	m_timeout = timeout;
	m_status = BD_PENDING;
	m_byteResults = BD_NULL;
}


void bdRemoteTask::stop(const bdByteBufferRef results, 
						const bdUInt64 id)
{	
	m_status = BD_DONE;
	m_byteResults = results;
	m_transactionID = id;
}
