// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdFacebook/bdFacebook.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>


#define BD_LOG_LEVEL "facebook"
#define TYPE_CHECKED true


bdFacebook::bdFacebook(bdRemoteTaskManager *const remoteTaskManager)
	: m_remoteTaskManager(remoteTaskManager)
{
}

bdFacebook::~bdFacebook()
{
}

bdRemoteTaskRef bdFacebook::registerAccount(const bdNChar8 *const accountName,
											const bdNChar8 *const password)
{
	bdUInt taskSize = BD_TASK_OVERHEAD + 3; // 3 = # parameters + 1 
	taskSize += (bdUInt)bdStrnlen(accountName, BD_FACEBOOK_MAX_ACCOUNT_LENGTH);
	taskSize += (bdUInt)bdStrnlen(password, BD_FACEBOOK_MAX_PASSWORD_LENGTH);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_FACEBOOK_SERVICE, BD_FACEBOOK_REGISTER);

	bdBool ok = true;

	ok = ok && buffer.writeString(accountName, BD_FACEBOOK_MAX_ACCOUNT_LENGTH);
	ok = ok && buffer.writeString(password, BD_FACEBOOK_MAX_PASSWORD_LENGTH);

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

bdRemoteTaskRef bdFacebook::post(const bdNChar8 *const message,
								 const bdNChar8 *const name,
								 const bdNChar8 *const caption,
								 const bdNChar8 *const description,
								 const bdNChar8 *const imageUrl,
								 const bdNChar8 *const imageHref)
{
	bdUInt taskSize = BD_TASK_OVERHEAD + 6; // 6 = # parameters + 1 
	taskSize += message == BD_NULL ? 0 : (bdUInt)bdStrnlen(message, BD_FACEBOOK_MAX_MESSAGE_LENGTH);
	taskSize += name == BD_NULL ? 0 : (bdUInt)bdStrnlen(name, BD_FACEBOOK_MAX_MESSAGE_LENGTH);
	taskSize += caption == BD_NULL ? 0 : (bdUInt)bdStrnlen(caption, BD_FACEBOOK_MAX_MESSAGE_LENGTH);
	taskSize += description == BD_NULL ? 0 : (bdUInt)bdStrnlen(description, BD_FACEBOOK_MAX_DESCRIPTION_LENGTH);
	taskSize += imageUrl == BD_NULL ? 0 : (bdUInt)bdStrnlen(imageUrl, BD_FACEBOOK_MAX_IMAGE_URL_LENGTH);
	taskSize += imageHref == BD_NULL ? 0 : (bdUInt)bdStrnlen(imageHref, BD_FACEBOOK_MAX_IMAGE_URL_LENGTH);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_FACEBOOK_SERVICE, BD_FACEBOOK_POST);

	bdBool ok = true;

	ok = ok && buffer.writeString(message, BD_FACEBOOK_MAX_MESSAGE_LENGTH);
	ok = ok && buffer.writeString(name, BD_FACEBOOK_MAX_MESSAGE_LENGTH);
	ok = ok && buffer.writeString(caption, BD_FACEBOOK_MAX_MESSAGE_LENGTH);
	ok = ok && buffer.writeString(description, BD_FACEBOOK_MAX_DESCRIPTION_LENGTH);
	ok = ok && buffer.writeString(imageUrl, BD_FACEBOOK_MAX_IMAGE_URL_LENGTH);
	ok = ok && buffer.writeString(imageHref, BD_FACEBOOK_MAX_IMAGE_URL_LENGTH);

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

bdRemoteTaskRef bdFacebook::unregisterAccount()
{
	bdUInt taskSize = BD_TASK_OVERHEAD + 1; // 1 = # parameters + 1 

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_FACEBOOK_SERVICE, BD_FACEBOOK_UNREGISTER);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}

	return remoteTask;
}

bdRemoteTaskRef bdFacebook::uploadPhoto(const bdUInt64 fileID,
										const bdNChar8 *const caption)
{
	bdUInt taskSize = BD_TASK_OVERHEAD + 3; // 3 = # parameters + 1
	taskSize += sizeof(bdUInt64);
	taskSize += caption == BD_NULL ? 0 : (bdUInt)bdStrnlen(caption, BD_FACEBOOK_MAX_MESSAGE_LENGTH);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_FACEBOOK_SERVICE, BD_FACEBOOK_UPLOAD_PHOTO);

	bdBool ok = true;

	ok = ok && buffer.writeUInt64(fileID);
	if (caption != BD_NULL)
	{
		ok = ok && buffer.writeString(caption, BD_FACEBOOK_MAX_MESSAGE_LENGTH);
	}

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

bdRemoteTaskRef bdFacebook::isRegistered(bdFacebookIsRegistered *const isRegistered)
{
	bdUInt taskSize = BD_TASK_OVERHEAD + 1; // 1 = # parameters + 1 

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_FACEBOOK_SERVICE, BD_FACEBOOK_IS_REGISTERED);

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
