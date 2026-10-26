// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdFriends/bdFriends.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdFriends/bdFriendInfo.h>
#include <bdLobby/bdFriends/bdFriendProposal.h>
#include <bdLobby/bdFriends/bdGroupInfo.h>

#define BD_LOG_LEVEL	("lobby/friends")
#define TYPE_CHECKED true

bdFriends::bdFriends(bdRemoteTaskManager *const remoteTaskManager)
: m_remoteTaskManager(remoteTaskManager)
{
	
}

bdFriends::~bdFriends()
{
	m_remoteTaskManager = BD_NULL;
}

bdRemoteTaskRef bdFriends::proposeFriendship(	const bdOnlineUserID friendID,
												const void *const attachment,
												const bdUInt attachmentSize)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID) + attachmentSize + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_PROPOSE_FRIENDSHIP);

	bdBool wroteParam = buffer.writeUInt64(friendID);
	if ( attachment != BD_NULL )
	{
		wroteParam = wroteParam && buffer.writeBlob(attachment, attachmentSize);
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

bdRemoteTaskRef bdFriends::proposeFriendships(	bdFriendProposal proposals[],
												const bdUInt numProposals )
{
	bdRemoteTaskRef task = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD;
	const bdUInt numParameters = 2;
	for( bdUInt i=0; i < numProposals; i++ )
	{
		taskSize += sizeof(bdOnlineUserID) + proposals[i].m_attachmentSize + numParameters;
	}
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_PROPOSE_FRIENDSHIPS);

	bdBool wroteParam = true;
	for( bdUInt i=0; i < numProposals && wroteParam; i++ )
	{
		wroteParam = wroteParam && buffer.writeUInt64(proposals[i].m_userID);
		if ( proposals[i].m_attachmentSize != 0 )
		{
			wroteParam = wroteParam && buffer.writeBlob(proposals[i].m_attachment, 
								proposals[i].m_attachmentSize);
		}
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

bdRemoteTaskRef bdFriends::withdrawProposal(const bdOnlineUserID friendID)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_WITHDRAW_PROPOSAL);

	bdBool wroteParam = buffer.writeUInt64(friendID);
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

bdRemoteTaskRef bdFriends::withdrawProposals(	bdOnlineUserID friendIDs[],
												const bdUInt numFriends )
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (numFriends * (sizeof(bdOnlineUserID) + numParameters));
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_WITHDRAW_PROPOSALS);

	bdBool wroteParam = true;
	for( bdUInt i=0; i < numFriends && wroteParam; i++ )
	{
		wroteParam = wroteParam && buffer.writeUInt64(friendIDs[i]);
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

bdRemoteTaskRef bdFriends::getOutgoingProposals(bdFriendProposal proposedFriends[],
												const bdUInt maxNumProposals)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_GET_OUTGOING_PROPOSALS);

	bdBool wroteParam = buffer.writeUInt32(maxNumProposals);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(proposedFriends, maxNumProposals);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdFriends::getIncomingProposals(bdFriendProposal incomingProposals[],
												const bdUInt maxNumProposals)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_GET_INCOMING_PROPOSALS);

	bdBool wroteParam = buffer.writeUInt32(maxNumProposals);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(incomingProposals, maxNumProposals);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdFriends::acceptProposal(const bdOnlineUserID proposer)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_ACCEPT_PROPOSAL);

	bdBool wroteParam = buffer.writeUInt64(proposer);
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

bdRemoteTaskRef bdFriends::acceptProposals(	bdOnlineUserID proposers[],	
											const bdUInt numProposers )
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (numProposers * ( sizeof(bdOnlineUserID) + numParameters ));
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_ACCEPT_PROPOSALS);

	bdBool wroteParam = true;
	for( bdUInt i=0; i < numProposers && wroteParam; i++ )
	{
		wroteParam = wroteParam && buffer.writeUInt64(proposers[i]);
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

bdRemoteTaskRef bdFriends::rejectProposal(const bdOnlineUserID proposer)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_REJECT_PROPOSAL);

	bdBool wroteParam = buffer.writeUInt64(proposer);
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

bdRemoteTaskRef bdFriends::rejectProposals(	bdOnlineUserID proposers[], 
											const bdUInt numProposers)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (numProposers * ( sizeof(bdOnlineUserID) + numParameters));
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_REJECT_PROPOSALS);

	bdBool wroteParam = true;
	for( bdUInt i=0; i < numProposers && wroteParam; i++ )
	{
		wroteParam = wroteParam && buffer.writeUInt64(proposers[i]);
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

bdRemoteTaskRef bdFriends::removeFriend(const bdOnlineUserID friendID)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_REVOKE_FRIENDSHIP);

	bdBool wroteParam = buffer.writeUInt64(friendID);
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

bdRemoteTaskRef bdFriends::removeFriends(bdOnlineUserID friendIDs[], 
										const bdUInt numFriends)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (numFriends * (sizeof(bdOnlineUserID) + numParameters));
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_REVOKE_FRIENDSHIPS);

	bdBool wroteParam = true;
	for( bdUInt i=0; i < numFriends && wroteParam; i++ )
	{
		wroteParam = wroteParam && buffer.writeUInt64(friendIDs[i]);
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

bdRemoteTaskRef bdFriends::removeAllFriends()
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 0;
	const bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_REMOVE_ALL_FRIENDS);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}

	return task;
}

bdRemoteTaskRef bdFriends::getFriends(	bdFriendInfo friendInfos[],
										const bdUInt maxNumFriends)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_GET_FRIENDS);

	bdBool wroteParam = buffer.writeUInt32(maxNumFriends);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(friendInfos, maxNumFriends);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdFriends::setFriendGroup(	const bdOnlineUserID friendID,
											const bdUByte8 groupID)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID) + sizeof(bdUByte8) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_SET_GROUP);

	bdBool wroteParam = buffer.writeUInt64(friendID);
	wroteParam = wroteParam && buffer.writeUByte8(groupID);
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

bdRemoteTaskRef bdFriends::setGroupName(bdGroupInfo *groupInfo)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + groupInfo->sizeOf() + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_SET_GROUP_NAME);

	groupInfo->serialize(buffer);
	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}

	return task;
}

bdRemoteTaskRef bdFriends::getGroupNames(	bdGroupInfo groupInfos[],
											const bdUInt maxNumGroups)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_GET_GROUPS);

	bdBool wroteParam = buffer.writeUInt32(maxNumGroups);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(groupInfos, maxNumGroups);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}


bdRemoteTaskRef bdFriends::deleteGroupName(const bdUByte8 groupID)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUByte8) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_DELETE_GROUP_NAME);

	bdBool wroteParam = buffer.writeUByte8(groupID);
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

bdRemoteTaskRef bdFriends::sendInstantMessage(	const bdOnlineUserID friendUID, 
												const void *const message, 
												const bdUInt msgSize)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt actualMsgSize = BD_MIN(msgSize , BD_MAX_CHAT_MESSAGE_SIZE);
	if (actualMsgSize < msgSize)
	{
		BD_ERR(BD_LOG_LEVEL, "Exceeded maximum instant message size, data will be truncated.");
	}
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID) + actualMsgSize + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_SEND_INSTANT_MESSAGE);

	bdBool wroteParam = buffer.writeUInt64(friendUID);
	wroteParam = wroteParam && buffer.writeBlob(message, actualMsgSize);
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

bdRemoteTaskRef bdFriends::setRichPresence(const void *const data, 
										   const bdUInt dataSize)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt actualDataSize = BD_MIN(dataSize , BD_MAX_RICH_PRESENCE_UPDATE);
	if (actualDataSize < dataSize)
	{
		BD_ERR(BD_LOG_LEVEL, "Exceeded maximum rich presence update size, data will be truncated.");
	}
	const bdUInt taskSize = BD_TASK_OVERHEAD + actualDataSize + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_SET_RICH_PRESENCE);

	bdBool wroteParam = buffer.writeBlob(data, actualDataSize);
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

bdRemoteTaskRef bdFriends::getRichPresence(const bdOnlineUserID friendUID, bdRichPresence *richPresence)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID) + numParameters;
	
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FRIENDS_MANAGEMENT_SERVICE, BD_FRIEND_GET_RICH_PRESENCE);

	bdBool wroteParam = buffer.writeUInt64(friendUID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(richPresence, 1);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

