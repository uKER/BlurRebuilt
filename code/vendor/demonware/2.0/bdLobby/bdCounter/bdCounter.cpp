// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdCounter/bdCounter.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdCounter/bdCounterValue.h>

#define BD_LOG_LEVEL "counters"
#define TYPE_CHECKED true

bdCounter::bdCounter(bdRemoteTaskManager *const remoteTaskManager)
:m_remoteTaskManager(remoteTaskManager)
{
}

bdCounter::~bdCounter()
{
}

bdRemoteTaskRef bdCounter::incrementCounters(bdCounterValue *const counterIncrements,	const bdUInt numEntries)
{
	const bdUInt numParameters = numEntries;
	bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	for (bdUInt i=0; i < numEntries; i++)
	{
		taskSize += counterIncrements[i].sizeOf();
	}

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_COUNTER_SERVICE, BD_COUNTER_INCREMENT_COUNTER);

	bdBool wroteParam = true;
	// write all counterIncrements to the buffer
	for(bdUInt i=0; i < numEntries; i++)
	{
		counterIncrements[i].serialize(buffer);
	}

	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdCounter::getCounterTotals(bdCounterValue results[], const bdUInt numCounterIDs)
{

	const bdUInt numParameters = numCounterIDs;
	bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	for (bdUInt i=0; i < numCounterIDs; i++)
	{
		taskSize += sizeof(bdUInt32);
	}

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_COUNTER_SERVICE, BD_COUNTER_GET_COUNTER_TOTAL);

	bdBool wroteParam = true;

	// write all counterIDs to the buffer
	for(bdUInt i=0; i < numCounterIDs; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt32(results[i].m_counterID);
	}

	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			remoteTask->setTaskResult(results, numCounterIDs);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}
