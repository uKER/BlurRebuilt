// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdTwitter/bdTwitter.h>


#define BD_LOG_LEVEL "twitter"
#define TYPE_CHECKED true


bdTwitter::bdTwitter(bdRemoteTaskManager *const remoteTaskManager)
	: m_remoteTaskManager(remoteTaskManager)
{
}

bdTwitter::~bdTwitter()
{
}

bdRemoteTaskRef bdTwitter::registerAccount(const bdNChar8 *const accountName,
										   const bdNChar8 *const password)
{
	bdUInt taskSize = BD_TASK_OVERHEAD + 3; // 3 = # parameters + 1 
	taskSize += (bdUInt)bdStrnlen(accountName, BD_TWITTER_MAX_ACCOUNT_LENGTH);
	taskSize += (bdUInt)bdStrnlen(password, BD_TWITTER_MAX_PASSWORD_LENGTH);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_TWITTER_SERVICE, BD_TWITTER_REGISTER);

	bdBool ok = true;

	ok = ok && buffer.writeString(accountName, BD_TWITTER_MAX_ACCOUNT_LENGTH);
	ok = ok && buffer.writeString(password, BD_TWITTER_MAX_PASSWORD_LENGTH);

	if (ok)
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

bdRemoteTaskRef bdTwitter::post(const bdNChar8 *const message)
{
	bdUInt taskSize = BD_TASK_OVERHEAD + 2; // 2 = # parameters + 1 
	taskSize += (bdUInt)bdStrnlen(message, BD_TWITTER_MAX_MESSAGE_LENGTH);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_TWITTER_SERVICE, BD_TWITTER_POST);

	bdBool ok = true;

	ok = ok && buffer.writeString(message, BD_TWITTER_MAX_MESSAGE_LENGTH);

	if (ok)
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

bdRemoteTaskRef bdTwitter::unregisterAccount()
{
	bdUInt taskSize = BD_TASK_OVERHEAD + 1; // 1 = # parameters + 1 

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_TWITTER_SERVICE, BD_TWITTER_UNREGISTER);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}

	return remoteTask;
}

bdRemoteTaskRef bdTwitter::isRegistered(bdTwitterIsRegistered *const isRegistered)
{
	bdUInt taskSize = BD_TASK_OVERHEAD + 1; // 1 = # parameters + 1 

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_TWITTER_SERVICE, BD_TWITTER_IS_REGISTERED);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}
	else
	{
		remoteTask->setTaskResult(isRegistered, 1);
	}

	return remoteTask;
}
