	// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdKeyArchive/bdKeyArchive.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdKeyArchive/bdKeyValuePair.h>

#define BD_LOG_LEVEL "key archive"
#define TYPE_CHECKED true

bdKeyArchive::bdKeyArchive(bdRemoteTaskManager *const remoteTaskManager)
: m_remoteTaskManager(remoteTaskManager)
{
}

bdKeyArchive::~bdKeyArchive()
{
	m_remoteTaskManager = BD_NULL;
}

bdRemoteTaskRef bdKeyArchive::write(const bdEntityID entityID,
									const bdBool isTeam,
									const bdKeyValuePair values[],
									const bdUInt numValues)
{
	bdRemoteTaskRef task = BD_NULL;
	
	// first iteration: call serialize on an 0-length bytebuffer to count the number of bytes needed
	bdByteBuffer buffer(0, TYPE_CHECKED);
	buffer.writeUInt64(entityID);
	buffer.writeBool(isTeam);
	for(bdUInt i=0; i<numValues; i++)
	{
		// We don't need to write values that haven't changed
		if (values[i].m_value != 0 || values[i].m_updateType != bdKeyValuePair::BD_ADD)
		{
			buffer.writeUInt16(values[i].m_index);
			buffer.writeInt64(values[i].m_value);
			bdUByte8 updateType = static_cast<bdUByte8>(values[i].m_updateType);
			buffer.writeUByte8(updateType);
		}
	}
	buffer.write(BD_NULL, BD_TASK_OVERHEAD);

	// this allocates the actual amount of bytes needed
	buffer.allocateBuffer();

	bdRemoteTaskManager::initTaskBuffer(buffer, BD_KEY_ARCHIVE_SERVICE, BD_KEYARCHIVE_WRITE);

	bdBool wroteParam = buffer.writeUInt64(entityID);
	wroteParam = wroteParam && buffer.writeBool(isTeam);
	for(bdUInt i=0; i<numValues; i++)
	{
		// We don't need to write values that haven't changed
		if (values[i].m_value != 0 || values[i].m_updateType != bdKeyValuePair::BD_ADD)
		{
			wroteParam = wroteParam && buffer.writeUInt16(values[i].m_index);
			wroteParam = wroteParam && buffer.writeInt64(values[i].m_value);
			bdUByte8 updateType = static_cast<bdUByte8>(values[i].m_updateType);
			wroteParam = wroteParam && buffer.writeUByte8(updateType);
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


bdRemoteTaskRef bdKeyArchive::read(	const bdEntityID entityID,
									const bdBool isTeam,
									const bdBool readDedicated,
									bdKeyValuePair values[],
									const bdUInt numValues)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3 + numValues;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdEntityID) + sizeof(bdBool) + sizeof(bdBool) + (sizeof(bdUInt16)*numValues) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_KEY_ARCHIVE_SERVICE, BD_KEYARCHIVE_READ);

	bdBool wroteParam = buffer.writeUInt64(entityID);
	wroteParam = wroteParam && buffer.writeBool(isTeam);
	wroteParam = wroteParam && buffer.writeBool(readDedicated);
	for (bdUInt i =0; i<numValues; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt16(values[i].m_index);
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
			task->setTaskResult(values, numValues);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdKeyArchive::readAll(	const bdEntityID entityID,
										const bdBool isTeam,
										const bdBool readDedicated,
										bdKeyValuePair values[],
										const bdUInt numValues)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdEntityID) + sizeof(bdBool) + sizeof(bdBool) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_KEY_ARCHIVE_SERVICE, BD_KEYARCHIVE_READ_ALL);

	bdBool wroteParam = buffer.writeUInt64(entityID);
	wroteParam = wroteParam && buffer.writeBool(isTeam);
	wroteParam = wroteParam && buffer.writeBool(readDedicated);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(values, numValues);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}



