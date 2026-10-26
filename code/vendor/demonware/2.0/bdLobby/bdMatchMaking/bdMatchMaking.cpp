// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdMatchMaking/bdMatchMaking.h>

#include <bdLobby/bdMatchMaking/bdMatchMakingInfo.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdMatchMaking/bdSessionParams.h>
#include <bdLobby/bdMatchMaking/bdPerformanceValue.h>
#include <bdLobby/bdMatchMaking/bdSessionInvite.h>

#define BD_LOG_LEVEL "matchmaking"
#define TYPE_CHECKED true

bdMatchMaking::bdMatchMaking(bdRemoteTaskManager *const remoteTaskManager)
: m_remoteTaskManager(remoteTaskManager)
{
}

bdMatchMaking::~bdMatchMaking()
{
	m_remoteTaskManager = BD_NULL;
}

bdRemoteTaskRef bdMatchMaking::createSession(	bdMatchMakingInfo *sessionInfo,
												bdSessionID *sessionCredentials)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD +  sessionInfo->sizeOf() + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MATCHMAKING_SERVICE, BD_MM_CREATE_SESSION);

	sessionInfo->serialize(buffer);
	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task ,buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}
	else
	{
		// result will contain the security key/id of the session created on the server
		const bdUInt numResults = 1;
		task->setTaskResult(sessionCredentials, numResults);
	}

	return task;
}


bdRemoteTaskRef bdMatchMaking::updateSession(	const bdSessionID& sessionID,
												bdMatchMakingInfo *const sessionInfo)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdSessionID) + sessionInfo->sizeOf() + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MATCHMAKING_SERVICE, BD_MM_UPDATE_SESSION);

	sessionID.serialize(buffer);
	sessionInfo->serialize(buffer);
	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task ,buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}

	return task;
}


bdRemoteTaskRef bdMatchMaking::deleteSession(const bdSessionID& sessionID)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdSessionID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MATCHMAKING_SERVICE, BD_MM_DELETE_SESSION);

	sessionID.serialize(buffer);
	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}

	return task;
}

bdRemoteTaskRef bdMatchMaking::findSessionFromID(	const bdSessionID& sessionID,
													bdMatchMakingInfo *const sessionInfo)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdSessionID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MATCHMAKING_SERVICE, BD_MM_FIND_SESSION_BY_ID);

	sessionID.serialize(buffer);
	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}
	else
	{
		const bdUInt numResults = 1;
		task->setTaskResult(sessionInfo, numResults);
	}

	return task;
}

bdRemoteTaskRef bdMatchMaking::findSessions(const bdUInt queryID,
											const bdUInt startIndex,
											const bdUInt maxNumResults,
											bdSessionParams *sessionParams,
											bdMatchMakingInfo results[])
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 4;
	const bdUInt taskSize = BD_TASK_OVERHEAD +  sizeof(bdUInt32) + sizeof(bdUInt32) + sizeof(bdUInt32) + sessionParams->sizeOf() + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MATCHMAKING_SERVICE, BD_MM_FIND_SESSIONS);

	bdBool wroteParam = buffer.writeUInt32(queryID);
	wroteParam = wroteParam && buffer.writeUInt32(startIndex);
	wroteParam = wroteParam && buffer.writeUInt32(maxNumResults);
	sessionParams->serialize(buffer);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(results, maxNumResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdMatchMaking::inviteToSession(	const bdSessionID& sessionID,
												const bdOnlineUserID userIDs[],
												const bdUInt numUsers,
												const void *const attachment,
												const bdUInt attachmentSize)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2 + numUsers ;
	const bdUInt actualAttachmentSize = BD_MIN(attachmentSize , BD_MAX_INVITE_ATTACHMENT_SIZE);
	if (actualAttachmentSize < attachmentSize)
	{
		BD_ERR(BD_LOG_LEVEL, "Exceeded maximum invite attachment size, data will be truncated.");
	}
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdSessionID) + actualAttachmentSize + (sizeof(bdOnlineUserID)*numUsers) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MATCHMAKING_SERVICE, BD_MM_INVITE_TO_SESSION);

	sessionID.serialize(buffer);
	bdBool wroteParam = buffer.writeBlob(attachment, actualAttachmentSize);
	for(bdUInt i = 0; i < numUsers; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(userIDs[i]);
	}
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
	
	return task;
}

bdRemoteTaskRef bdMatchMaking::notifyJoin(const bdSessionID& sessionID,
										  const bdOnlineUserID* users,
										  const bdUInt numUsers)
{
	return startNotifyTask(sessionID, users, numUsers, BD_MM_NOTIFY_JOIN);
}

bdRemoteTaskRef bdMatchMaking::notifyLeave(const bdSessionID& sessionID,
										   const bdOnlineUserID* users,
										   const bdUInt numUsers)
{
	return startNotifyTask(sessionID, users, numUsers, BD_MM_NOTIFY_LEAVE);
}

bdRemoteTaskRef bdMatchMaking::submitPerformance(const bdUInt gameType, 
												 bdPerformanceValue *performanceValues, 
												 const bdUInt numValues)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1 + numValues;
	bdUInt taskSize = BD_TASK_OVERHEAD +  sizeof(bdUInt32) + numParameters;
	for (bdUInt i=0; i<numValues; i++)
	{
		taskSize += performanceValues[i].sizeOf();
	}
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MATCHMAKING_SERVICE, BD_MM_SUBMIT_PERFORMANCE);

	bdBool wroteParam = buffer.writeUInt32(gameType);
	for(bdUInt i=0; i<numValues; i++)
	{
		performanceValues[i].serialize(buffer);
	}
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;

}

bdRemoteTaskRef bdMatchMaking::getPerformanceValues(const bdEntityID *const entityIDs, 
													const bdUInt numEntityIDs, 
													const bdUInt gameType,
													bdPerformanceValue results[])
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1 + numEntityIDs;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt32) + (sizeof(bdUInt64)*numEntityIDs) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MATCHMAKING_SERVICE, BD_MM_GET_PERFORMANCE);

	bdBool wroteParam = buffer.writeUInt32(gameType);
	for(bdUInt i=0; i<numEntityIDs; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(entityIDs[i]);
	}
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(results, numEntityIDs);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdMatchMaking::getSessionInvites(	const bdUInt32 startIndex,
													const bdUInt32 maxNumResults,
													bdSessionInvite results[])
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + + sizeof(bdUInt32) + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MATCHMAKING_SERVICE, BD_MM_GET_SESSION_INVITES);

	bdBool wroteParam = buffer.writeUInt32(startIndex);
	wroteParam = wroteParam && buffer.writeUInt32(maxNumResults);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(results, maxNumResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdMatchMaking::startNotifyTask(	const bdSessionID& sessionID,
												const bdOnlineUserID* users,
												const bdUInt numUsers,
												const bdUByte8 taskType)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1 + numUsers;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdSessionID) + (sizeof(bdOnlineUserID)*numUsers) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MATCHMAKING_SERVICE, taskType);

	sessionID.serialize(buffer);
	bdBool wroteParam = true;
	for(bdUInt i=0; i < numUsers; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(users[i]);
	}
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

