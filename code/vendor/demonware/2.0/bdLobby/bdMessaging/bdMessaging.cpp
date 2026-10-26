// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdMessaging/bdMessaging.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdMessaging/bdMailHeader.h>
#include <bdLobby/bdMessaging/bdMailBody.h>
#include <bdLobby/bdMessaging/bdNotification.h>
#include <bdLobby/bdMessaging/bdChannelInfo.h>
#include <bdLobby/bdMessaging/bdChannelMember.h>
#include <bdLobby/bdMessaging/bdBlockedUser.h>
#include <bdLobby/bdMessaging/bdLobbyMessageType.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

#define BD_LOG_LEVEL "messaging"
#define TYPE_CHECKED true

bdRemoteTaskRef bdMessaging::getMailHeaders(bdMailHeader mailHeaders[],
											const bdUInt startIndex,
											const bdUInt numMails)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD +  sizeof(bdUInt32)+  sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MESSAGING_SERVICE, BD_MESSAGING_GET_MAIL_HEADERS);

	bdBool wroteParam = buffer.writeUInt32(startIndex);
	wroteParam = wroteParam && buffer.writeUInt32(numMails);

	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(mailHeaders, numMails);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
	
	return task;
}

bdRemoteTaskRef bdMessaging::getMails(	const bdUInt64 mailIDs[], 
										const bdUInt numMailIDs,
										bdMailBody mails[],
										const bdBool deleteOnRead)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = numMailIDs + 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD +  (sizeof(bdUInt64)* numMailIDs) +  sizeof(bdBool) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MESSAGING_SERVICE, BD_MESSAGING_GET_MAILS);

	bdBool wroteParam = buffer.writeBool(deleteOnRead);
	for(bdUInt i = 0; i < numMailIDs; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(mailIDs[i]);
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
			task->setTaskResult(mails, numMailIDs);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
		
	return task;
}

bdRemoteTaskRef bdMessaging::deleteMail(const bdUInt64 mailIDs[], 
										const bdUInt numMailIDs)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = numMailIDs;
	const bdUInt taskSize = BD_TASK_OVERHEAD +  (sizeof(bdUInt64)* numMailIDs) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MESSAGING_SERVICE, BD_MESSAGING_DELETE_MAIL);

	bdBool wroteParam = true;
	for(bdUInt i = 0; i < numMailIDs; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(mailIDs[i]);
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

bdRemoteTaskRef bdMessaging::sendMail(	const void *const message, 
										const bdUInt messageSize, 
										const bdOnlineUserID userIDs[], 
										const bdUInt numUserIDs)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1 + numUserIDs ;
	const bdUInt actualMessageSize = BD_MIN(messageSize , BD_MAXIMUM_CUSTOM_MSG_SIZE);
	if (actualMessageSize < messageSize)
	{
		BD_ERR(BD_LOG_LEVEL, "Exceeded maximum mail size, data will be truncated.");
	}
	const bdUInt taskSize = BD_TASK_OVERHEAD + actualMessageSize + (sizeof(bdOnlineUserID)*numUserIDs) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MESSAGING_SERVICE, BD_MESSAGING_SEND_MAIL);

	bdBool wroteParam = buffer.writeBlob(message, actualMessageSize);
	for(bdUInt i = 0; i < numUserIDs; i++)
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

bdRemoteTaskRef bdMessaging::getTeamMailHeaders(	const bdTeamID teamID,
													bdMailHeader mailHeaders[],
													const bdUInt startIndex,
													const bdUInt numMails)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD +  sizeof(bdTeamID) + sizeof(bdUInt32) +  sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MESSAGING_SERVICE, BD_MESSAGING_GET_TEAM_MAIL_HEADERS);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	wroteParam = wroteParam && buffer.writeUInt32(startIndex);
	wroteParam = wroteParam && buffer.writeUInt32(numMails);

	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(mailHeaders, numMails);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
	
	return task;
}

bdRemoteTaskRef bdMessaging::getTeamMails(	const bdTeamID teamID,
											const bdUInt64 mailIDs[],
											const bdUInt numMailIDs,
											bdMailBody mails[],
											const bdBool deleteOnRead)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = numMailIDs + 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID) + BD_TYPED_ARRAY_HEADER_SIZE +
							(sizeof(bdUInt64)* numMailIDs) +  sizeof(bdBool) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MESSAGING_SERVICE, BD_MESSAGING_GET_TEAM_MAILS);

	bdBool wroteParam = buffer.writeUInt64(teamID);

	wroteParam = wroteParam && buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER64_TYPE, numMailIDs, sizeof(bdUInt64));
	for(bdUInt i = 0; i < numMailIDs; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(mailIDs[i]);
	}
	buffer.writeArrayEnd();
	wroteParam = wroteParam && buffer.writeBool(deleteOnRead);

	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(mails, numMailIDs);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
		
	return task;

}


bdRemoteTaskRef bdMessaging::deleteTeamMail(	const bdTeamID teamID,
												const bdUInt64 mailIDs[], 
												const bdUInt numMailIDs)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = numMailIDs + 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID) + BD_TYPED_ARRAY_HEADER_SIZE +
							(sizeof(bdUInt64)* numMailIDs) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MESSAGING_SERVICE, BD_MESSAGING_DELETE_TEAM_MAIL);

	bdBool wroteParam = buffer.writeUInt64(teamID);

	wroteParam = wroteParam && buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER64_TYPE, numMailIDs, sizeof(bdUInt64));
	for(bdUInt i = 0; i < numMailIDs; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(mailIDs[i]);
	}
	buffer.writeArrayEnd();

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

bdRemoteTaskRef bdMessaging::sendTeamMail(	const void *const message, 
											const bdUInt messageSize, 
											const bdTeamID teamID)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2 ;
	const bdUInt actualMessageSize = BD_MIN(messageSize , BD_MAXIMUM_CUSTOM_MSG_SIZE);
	if (actualMessageSize < messageSize)
	{
		BD_ERR(BD_LOG_LEVEL, "Exceeded maximum mail size, data will be truncated.");
	}
	const bdUInt taskSize = BD_TASK_OVERHEAD + actualMessageSize + sizeof(bdUInt64) + sizeof(bdTeamID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MESSAGING_SERVICE, BD_MESSAGING_SEND_TEAM_MAIL);

	bdBool wroteParam = buffer.writeBlob(message, actualMessageSize);
	wroteParam = wroteParam && buffer.writeUInt64(teamID);

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

bdRemoteTaskRef bdMessaging::getNotifications(	bdNotification notifications[],
												const bdUInt startIndex,
												const bdUInt numNotifications,
												const bdBool deleteOnRead)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt32) + sizeof(bdUInt32) + sizeof(bdBool) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MESSAGING_SERVICE, BD_MESSAGING_GET_NOTIFICATIONS);

	bdBool wroteParam = buffer.writeUInt32(startIndex);
	wroteParam = wroteParam && buffer.writeUInt32(numNotifications);
	wroteParam = wroteParam && buffer.writeBool(deleteOnRead);

	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(notifications, numNotifications);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdMessaging::deleteNotifications(const bdUInt64 notificationIDs[], 
												 const bdUInt numNotifications)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = numNotifications;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(bdUInt64)*numNotifications) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_MESSAGING_SERVICE, BD_MESSAGING_DELETE_NOTIFICATIONS);
	
	bdBool wroteParam = true;
	for(bdUInt i = 0; i < numNotifications; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(notificationIDs[i]);
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

bdRemoteTaskRef bdMessaging::createChatChannel(bdChannelInfo *const channelInfo)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 0;
	const bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CHANNEL_CHAT_SERVICE, BD_CHANNEL_CHAT_CREATE);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}
	else
	{
		const bdUInt numResults = 1;
		task->setTaskResult(channelInfo, numResults);
	}

	return task;
}

bdRemoteTaskRef bdMessaging::subscribeToChannel(const bdUInt64 channelID)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt64) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CHANNEL_CHAT_SERVICE, BD_CHANNEL_CHAT_SUBSCRIBE);

	bdBool wroteParam = buffer.writeUInt64(channelID);
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

bdRemoteTaskRef bdMessaging::unsubscribeFromChannel(const bdUInt64 channelID)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt64) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CHANNEL_CHAT_SERVICE, BD_CHANNEL_CHAT_UNSUBSCRIBE);

	bdBool wroteParam = buffer.writeUInt64(channelID);
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

bdRemoteTaskRef bdMessaging::getChannelList(bdChannelInfo channelInfos[],
											const bdUInt maxNumChannels)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CHANNEL_CHAT_SERVICE, BD_CHANNEL_CHAT_DISCOVER);

	bdBool wroteParam = buffer.writeUInt32(maxNumChannels);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(channelInfos, maxNumChannels);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdMessaging::sendToChannel(	const bdUInt64 channelID,
											const void *const message,
											const bdUInt messageSize)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt64) + messageSize + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CHANNEL_CHAT_SERVICE, BD_CHANNEL_CHAT_SEND);

	bdBool wroteParam = buffer.writeUInt64(channelID);
	wroteParam = wroteParam && buffer.writeBlob(message,messageSize);
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

bdRemoteTaskRef bdMessaging::whisperToChannelMember(const bdUInt64 channelID,
													const bdOnlineUserID userID,
													const void *const message,
													const bdUInt messageSize)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt64) + sizeof(bdOnlineUserID) + messageSize + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CHANNEL_CHAT_SERVICE, BD_CHANNEL_CHAT_WHISPER);

	bdBool wroteParam = buffer.writeUInt64(channelID);
	wroteParam = wroteParam && buffer.writeUInt64(userID);
	wroteParam = wroteParam && buffer.writeBlob(message, messageSize);

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

bdRemoteTaskRef bdMessaging::getChannelMembers(	const bdUInt64 channelID,
												bdChannelMember channelMembers[],
												const bdUInt maxNumMembers)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt64) + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CHANNEL_CHAT_SERVICE, BD_CHANNEL_CHAT_GET_MEMBERS);

	bdBool wroteParam = buffer.writeUInt64(channelID);
	wroteParam = wroteParam && buffer.writeUInt32(maxNumMembers);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(channelMembers, maxNumMembers);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdMessaging::setBlockStatus(const bdOnlineUserID userID,
                                            const bdBool blocked)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID) + sizeof(bdBool) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CHANNEL_CHAT_SERVICE, BD_MESSAGING_SET_BLOCKED_STATUS);

	bdBool wroteParam = buffer.writeUInt64(userID);
	wroteParam = wroteParam && buffer.writeBool(blocked);
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

bdRemoteTaskRef bdMessaging::getBlockedList(bdBlockedUser blockedUsers[],
											const bdUInt startIndex,
	                                        const bdUInt maxNumUsers)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt32) + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CHANNEL_CHAT_SERVICE, BD_MESSAGING_GET_BLOCKED_LIST);

	bdBool wroteParam = buffer.writeUInt32(startIndex);
	wroteParam = wroteParam && buffer.writeUInt32(maxNumUsers);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(blockedUsers, maxNumUsers);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}


bdMessaging::bdMessaging(bdRemoteTaskManager *const remoteTaskManager)
{
	m_remoteTaskManager = remoteTaskManager;
}

bdMessaging::~bdMessaging()
{
}



