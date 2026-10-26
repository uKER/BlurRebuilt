// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdTitleUtilities/bdTitleUtilities.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdTitleUtilities/bdTitleStats.h>
#include <bdLobby/bdTitleUtilities/bdVerifyString.h>
#include <bdLobby/bdTitleUtilities/bdTimeStamp.h>

#define BD_LOG_LEVEL "title utilities"
#define TYPE_CHECKED true

bdTitleUtilities::bdTitleUtilities(bdRemoteTaskManager *const remoteTaskManager)
:m_remoteTaskManager(remoteTaskManager)
{
}

bdTitleUtilities::~bdTitleUtilities()
{
}

bdRemoteTaskRef bdTitleUtilities::verifyString(	const bdNChar8 *const str, 
												const bdUInt length, 
												bdVerifyString *verified)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + ((bdUInt)(bdStrnlen(str, length))) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TITLE_UTILITIES_SERVICE, BD_VERIFY_STRING);

	const bdBool wroteString = buffer.writeString(str, length);
	if(true == wroteString)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			remoteTask->setTaskResult(verified, 1);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write string %s into buffer" , str);
	}
	return remoteTask;
}

bdRemoteTaskRef bdTitleUtilities::recordEvent(const bdNChar8 *const eventStr, const bdUInt length, bdUInt category)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + ((bdUInt)(bdStrnlen(eventStr, length))) + sizeof(category) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TITLE_UTILITIES_SERVICE, BD_RECORD_EVENT);

	const bdBool wroteString = buffer.writeString(eventStr, length);
	buffer.writeUInt32(category);
	if(true == wroteString)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write string %s into buffer" , eventStr);
	}

	return remoteTask;
}

bdRemoteTaskRef bdTitleUtilities::recordEventBin(const void *eventBlob, const bdUInt length, bdUInt category)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + length + sizeof(category);
	bdByteBuffer buffer(taskSize, true);

	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TITLE_UTILITIES_SERVICE, BD_RECORD_EVENT_BIN);

	buffer.writeBlob(eventBlob, length);
	buffer.writeUInt32(category);
	m_remoteTaskManager->startTask(remoteTask, buffer);

	return remoteTask;
}

bdRemoteTaskRef bdTitleUtilities::getTitleStats(bdTitleStats *titleStats)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt taskSize = BD_TASK_OVERHEAD;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TITLE_UTILITIES_SERVICE, BD_GET_TITLE_STATS);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}
	else
	{
		remoteTask->setTaskResult(titleStats, 1);
	}

	return remoteTask;
}

bdRemoteTaskRef bdTitleUtilities::areUsersOnline(bdOnlineUserInfo results[], const bdUInt numUsers)
{

	const bdUInt numParameters = numUsers;
	bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	for (bdUInt i=0; i < numUsers; i++)
	{
		taskSize += sizeof(bdUInt64);
	}

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TITLE_UTILITIES_SERVICE, BD_ARE_USERS_ONLINE);

	bdBool wroteParam = true;

	// write all user IDs to the buffer
	for(bdUInt i=0; i < numUsers; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(results[i].m_userID);
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
			remoteTask->setTaskResult(results, numUsers);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTitleUtilities::getServerTime(bdTimeStamp *timeStamp)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt taskSize = BD_TASK_OVERHEAD;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TITLE_UTILITIES_SERVICE, BD_GET_SERVER_TIME);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);

	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}
	else
	{
		remoteTask->setTaskResult(timeStamp, 1);
	}

	return remoteTask;
}
