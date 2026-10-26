// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdDML/bdDML.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

#define BD_LOG_LEVEL "dml"
#define TYPE_CHECKED true

bdDML::bdDML(bdRemoteTaskManager *const remoteTaskManager)
:m_remoteTaskManager(remoteTaskManager)
{
}

bdDML::~bdDML()
{
}

bdRemoteTaskRef bdDML::recordIP(const bdUInt32 ipAddr)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(ipAddr)) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_DML_SERVICE, BD_RECORD_IP);

	const bdBool wroteIpAddr = buffer.writeUInt32(ipAddr);
	if(true == wroteIpAddr)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write ip addr %u into buffer" , ipAddr);
	}

	return remoteTask;
}

