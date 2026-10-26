// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdStorage/bdStorage.h>
#include <bdLobby/bdStorage/bdGetFilesResult.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdStorage/bdFileData.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

#define BD_LOG_LEVEL "storage"
#define TYPE_CHECKED true

bdStorage::bdStorage(bdRemoteTaskManager *const remoteTaskManager)
: m_remoteTaskManager(remoteTaskManager)
{
}

bdStorage::~bdStorage()
{
	m_remoteTaskManager = BD_NULL;
}

bdRemoteTaskRef bdStorage::uploadFile(	const bdNChar8 *const fileName,
										const void *const fileData,
										const bdUInt fileSize,
										const bdFileInfo::bdVisibility fileVisibility,
										const bdOnlineUserID ownerID,
										bdFileInfo *const fileInfo)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + bdUInt(bdStrnlen(fileName, BD_MAX_FILE_NAME_LENGTH)) + fileSize + sizeof(bdFileInfo::bdVisibility) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STORAGE_SERVICE, BD_STORAGE_UPLOAD_FILE);

	bdBool wroteParam = buffer.writeString(fileName, BD_MAX_FILE_NAME_LENGTH);
	wroteParam = wroteParam && buffer.writeBool(fileVisibility == bdFileInfo::BD_VISIBLE_PRIVATE);
	wroteParam = wroteParam && buffer.writeBlob(fileData, fileSize);
	if (ownerID != 0)
	{
		wroteParam = wroteParam && buffer.writeUInt64(ownerID);
	}

	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else if (fileInfo != BD_NULL)
		{
			const bdUInt numResults = 1;
			remoteTask->setTaskResult(fileInfo, numResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdStorage::removeFile(const bdNChar8 *const fileName,
									  const bdOnlineUserID ownerID)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + bdUInt(bdStrnlen(fileName, BD_MAX_FILE_NAME_LENGTH)) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STORAGE_SERVICE, BD_STORAGE_REMOVE_FILE);

	bdBool wroteParam = buffer.writeString(fileName, BD_MAX_FILE_NAME_LENGTH);
	if (ownerID != 0)
	{
		wroteParam = wroteParam && buffer.writeUInt64(ownerID);
	}

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

bdRemoteTaskRef bdStorage::getFile(	const bdNChar8 *fileName,
									bdFileData *const fileData,
									const bdOnlineUserID ownerUID)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + bdUInt(bdStrnlen(fileName, BD_MAX_FILE_NAME_LENGTH) + sizeof(bdOnlineUserID)) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STORAGE_SERVICE, BD_STORAGE_GET_FILE);

	bdBool wroteParam = buffer.writeString(fileName, BD_MAX_FILE_NAME_LENGTH);
	wroteParam = wroteParam && buffer.writeUInt64(ownerUID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			const bdUInt numResults = 1;
			remoteTask->setTaskResult(fileData, numResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdStorage::getFileByID(const bdUInt64 fileID,
                                       bdFileData *const fileData)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt64) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STORAGE_SERVICE, BD_STORAGE_GET_FILE_BY_ID);

	bdBool wroteParam = buffer.writeUInt64(fileID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			const bdUInt numResults = 1;
			remoteTask->setTaskResult(fileData, numResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
	
	return remoteTask;
}

bdRemoteTaskRef bdStorage::updateFile(const bdUInt64 fileID,
									  const void *fileData,
									  bdUInt fileSize)
{
	bdRemoteTaskRef remoteTask = BD_NULL;

	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt64) + fileSize;
	bdByteBuffer buffer(taskSize, true);

	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STORAGE_SERVICE, BD_STORAGE_UPDATE_FILE);

	buffer.writeUInt64(fileID);
	buffer.writeBlob(fileData, fileSize);

	m_remoteTaskManager->startTask(remoteTask, buffer);

	return remoteTask;
}

bdRemoteTaskRef bdStorage::getFilesByID(const bdUInt32 numFileIDs,
										const bdUInt64 fileIDs[],
										bdGetFilesResult filesResult[])
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + BD_TYPED_ARRAY_HEADER_SIZE + sizeof(bdUInt64)*numFileIDs;

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STORAGE_SERVICE, BD_STORAGE_GET_FILES_BY_ID);

	buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER64_TYPE, numFileIDs, sizeof(bdUInt64));
	for(bdUInt i=0; i< numFileIDs; i++)
	{
		buffer.writeUInt64(fileIDs[i]);
	}
	buffer.writeArrayEnd();

	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(filesResult, 1);
	return remoteTask;
}

bdRemoteTaskRef bdStorage::listFilesByOwner(const bdOnlineUserID ownerID,
											const bdUInt32 startDate,
											bdFileInfo fileList[],
											const bdUInt16 maxNumResults,
											const bdUInt16 offset,
											const bdNChar8 *const fileName)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 5;
	bdUInt taskSize = BD_TASK_OVERHEAD +sizeof(bdOnlineUserID) + sizeof(bdUInt32) +sizeof(bdUInt16) +sizeof(bdUInt16) + numParameters;
	if (fileName)
	{
		taskSize += bdUInt(bdStrnlen(fileName, BD_MAX_FILE_NAME_LENGTH));
	}
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STORAGE_SERVICE, BD_STORAGE_LISTBYOWNER);

	bdBool wroteParam = buffer.writeUInt64(ownerID);
	wroteParam = wroteParam && buffer.writeUInt32(startDate);
	wroteParam = wroteParam && buffer.writeUInt16(maxNumResults);
	wroteParam = wroteParam && buffer.writeUInt16(offset);
	if (fileName)
	{
		wroteParam = wroteParam && buffer.writeString(fileName, BD_MAX_FILE_NAME_LENGTH);
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
			remoteTask->setTaskResult(fileList, maxNumResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
	
	return remoteTask;
}

bdRemoteTaskRef bdStorage::listAllPublisherFiles(	const bdUInt32 startDate,
													bdFileInfo fileList[],
													const bdUInt16 maxNumResults,
													const bdUInt16 offset,
													const bdNChar8 *const fileName)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 4;
	bdUInt taskSize = BD_TASK_OVERHEAD +sizeof(bdUInt32)  +sizeof(bdUInt16)  +sizeof(bdUInt16) + numParameters;
	if (fileName)
	{
		taskSize += bdUInt(bdStrnlen(fileName, BD_MAX_FILE_NAME_LENGTH));
	}
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STORAGE_SERVICE, BD_STORAGE_LISTPUBLISHERFILES);

	bdBool wroteParam = buffer.writeUInt32(startDate);
	wroteParam = wroteParam && buffer.writeUInt16(maxNumResults);
	wroteParam = wroteParam && buffer.writeUInt16(offset);
	if (fileName)
	{
		wroteParam = wroteParam && buffer.writeString(fileName, BD_MAX_FILE_NAME_LENGTH);
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
			remoteTask->setTaskResult(fileList, maxNumResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
	
	return remoteTask;
}

bdRemoteTaskRef bdStorage::getPublisherFile(const bdNChar8 *const fileName, 
											bdFileData *const fileData)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + bdUInt(bdStrnlen(fileName, BD_MAX_FILE_NAME_LENGTH)) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STORAGE_SERVICE, BD_STORAGE_GET_PUBLISHER_FILE);

	bdBool wroteParam = buffer.writeString(fileName, BD_MAX_FILE_NAME_LENGTH);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			const bdUInt numResults = 1;
			remoteTask->setTaskResult(fileData, numResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
	
	return remoteTask;
}


