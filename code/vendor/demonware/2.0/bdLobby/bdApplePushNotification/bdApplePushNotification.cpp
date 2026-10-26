// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdApplePushNotification/bdApplePushNotification.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>


#define BD_LOG_LEVEL "applepushnotifications"
#define TYPE_CHECKED true


bdApplePushNotification::bdApplePushNotification(bdRemoteTaskManager *const remoteTaskManager)
: m_remoteTaskManager(remoteTaskManager)
{
}

bdApplePushNotification::~bdApplePushNotification()
{
}

bdRemoteTaskRef bdApplePushNotification::setDeviceToken(const bdNChar8 *const deviceToken, const bdUInt numBytes)
{
	const bdUInt numParameters = 1;
	bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	taskSize += (bdUInt)bdStrnlen(deviceToken, numBytes + 1); // extra char for null termination
	
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_APPLE_PUSH_NOTIFICATION_SERVICE, BD_APPLE_PUSH_NOTIFICATION_SET_DEVICE_TOKEN);
	
	bdBool ok = true;
	
	ok = ok && buffer.writeString(deviceToken, numBytes + 1);
	
	if (ok)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if (startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to start task: Error %i", startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to write param into buffer");
	}
	
	return remoteTask;
}

bdRemoteTaskRef bdApplePushNotification::sendPushNotification(const bdNChar8 *const payload,
															  const bdEntityID *const userIDs,
															  const bdUInt numEntries)
{
	const bdUInt numParameters = numEntries + 1;
	bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	taskSize += (bdUInt)bdStrnlen(payload, BD_APPLE_PUSH_NOTIFICATION_MAX_PAYLOAD_LENGTH);
	taskSize += sizeof(bdEntityID) + numEntries*sizeof(bdEntityID);
	
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_APPLE_PUSH_NOTIFICATION_SERVICE, BD_APPLE_PUSH_NOTIFICATION_SEND_PUSH_NOTIFICATION);
	
	bdBool ok = true;
	
	// write the payload to the buffer.
	ok = ok && buffer.writeString(payload, BD_APPLE_PUSH_NOTIFICATION_MAX_PAYLOAD_LENGTH);
	
	// write all the userIDs to the buffer
	ok = ok && buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER64_TYPE, numEntries, sizeof(bdUInt64));
	for(bdUInt i = 0; i < numEntries; i++)
	{
		ok = ok && buffer.writeUInt64(userIDs[i]);
	}
	buffer.writeArrayEnd();
	
	if(ok)
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
