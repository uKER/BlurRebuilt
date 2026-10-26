// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdProfiles/bdProfiles.h>
#include <bdLobby/bdProfiles/bdProfileInfo.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

#define BD_LOG_LEVEL "profiles"
#define TYPE_CHECKED true

bdProfiles::bdProfiles(bdRemoteTaskManager *const remoteTaskManager)
: m_remoteTaskManager(remoteTaskManager)
{
}

bdProfiles::~bdProfiles()
{
	m_remoteTaskManager = BD_NULL;
}

bdRemoteTaskRef bdProfiles::setPublicInfo(bdProfileInfo *profileInfo)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + profileInfo->sizeOf() + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_PROFILE_MANAGEMENT_SERVICE, BD_PROFILE_SET_PUBLIC_INFO);

	profileInfo->serialize(buffer);
	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}

	return remoteTask;
}

bdRemoteTaskRef bdProfiles::setPrivateInfo(bdProfileInfo *profileInfo)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1; 
	const bdUInt taskSize = BD_TASK_OVERHEAD + profileInfo->sizeOf() + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_PROFILE_MANAGEMENT_SERVICE, BD_PROFILE_SET_PRIVATE_INFO);

	profileInfo->serialize(buffer);
	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}

	return remoteTask;
}

bdRemoteTaskRef bdProfiles::getPublicInfos(	const bdOnlineUserID userIDs[],
											bdProfileInfo publicProfiles[],
											const bdUInt numProfiles)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = numProfiles; 
	const bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(bdOnlineUserID)*numProfiles) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_PROFILE_MANAGEMENT_SERVICE, BD_PROFILE_GET_PUBLIC_INFO);

	bdBool wroteParam = true;
	for (bdUInt i = 0; i < numProfiles; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(userIDs[i]);
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
			remoteTask->setTaskResult(publicProfiles, numProfiles);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdProfiles::getPrivateInfo(bdProfileInfo *const privateProfile)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 0; 
	const bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_PROFILE_MANAGEMENT_SERVICE, BD_PROFILE_GET_PRIVATE_INFO);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}
	else
	{
		remoteTask->setTaskResult(privateProfile, 1);
	}
	
	return remoteTask;
}


bdRemoteTaskRef bdProfiles::deleteProfile()
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 0; 
	const bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_PROFILE_MANAGEMENT_SERVICE, BD_PROFILE_DELETE);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}

	return remoteTask;
}

