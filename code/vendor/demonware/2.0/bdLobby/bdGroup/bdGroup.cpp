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
#include <bdLobby/bdGroup/bdGroup.h>

#define BD_LOG_LEVEL "groups"
#define TYPE_CHECKED true

bdGroup::bdGroup(bdRemoteTaskManager *const remoteTaskManager)
:m_remoteTaskManager(remoteTaskManager)
{
}

bdGroup::~bdGroup()
{
}

bdRemoteTaskRef bdGroup::setGroups(const bdUInt32 *const groupIDs,
								   const bdUInt numEntries)
{
	const bdUInt numParameters = numEntries;
	bdUInt taskSize = BD_TASK_OVERHEAD + numParameters + numEntries*sizeof(bdUInt32);
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_GROUP_SERVICE, BD_SET_GROUPS);

	bdBool wroteParam = true;
	wroteParam = wroteParam && buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER32_TYPE, numEntries, sizeof(bdUInt32));
	for(bdUInt i=0; i< numEntries; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt32(groupIDs[i]);
	}
	buffer.writeArrayEnd();

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

bdRemoteTaskRef bdGroup::setGroupsForEntity(const bdEntityID entityID,
											const bdUInt32 *const groupIDs,
											const bdUInt numEntries)
{
	const bdUInt numParameters = numEntries + 1;
	bdUInt taskSize = BD_TASK_OVERHEAD + numParameters + sizeof(bdEntityID) + numEntries*sizeof(bdUInt32);
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_GROUP_SERVICE, BD_SET_ENTITY_GROUPS);

	// Write the entity Id to the buffer.
	bdBool wroteParam = buffer.writeUInt64(entityID);

	// write all groupIDs to the buffer
	wroteParam = wroteParam && buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER32_TYPE, numEntries, sizeof(bdUInt32));
	for(bdUInt i=0; i< numEntries; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt32(groupIDs[i]);
	}
	buffer.writeArrayEnd();

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


bdRemoteTaskRef bdGroup::getEntityGroups(const bdEntityID entityID,
										 bdGroupID results[],
										 const bdUInt numEntries)
{

	const bdUInt numParameters = 1;
	bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_GROUP_SERVICE, BD_GET_ENTITY_GROUPS);

	// Write the entity Id to the buffer.
	const bdBool wroteParam = buffer.writeUInt64(entityID);

	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			remoteTask->setTaskResult(results, numEntries);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdGroup::getGroupCounts(const bdUInt32 *const groupIDs,
										const bdUInt numGroupIDs,
										bdGroupCount results[],
										const bdUInt numEntries)
{

	const bdUInt numParameters = numGroupIDs;
	bdUInt taskSize = BD_TASK_OVERHEAD + numParameters + numGroupIDs*sizeof(bdUInt32);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_GROUP_SERVICE, BD_GET_GROUP_COUNTS);

	bdBool wroteParam = true;
	// write all groupIDs to the buffer

	wroteParam = wroteParam && buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER32_TYPE, numGroupIDs, sizeof(bdUInt32));
	for(bdUInt i=0; i< numGroupIDs; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt32(groupIDs[i]);
	}
	buffer.writeArrayEnd();

	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			remoteTask->setTaskResult(results, numEntries);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

/*
bdRemoteTaskRef (	);
*/
