// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>

#include <bdLobby/bdTags/bdTags.h>
#include <bdLobby/bdTags/bdTagsArray.h>
#include <bdLobby/bdTags/bdTagQueryParams.h>

// Tags operations
#define BD_TAGS_GET_TAGS_FOR_ENTITYIDS			(1u)
#define BD_TAGS_SET_TAGS_FOR_ENTITYID			(2u)
#define BD_TAGS_REMOVE_TAGS_FOR_ENTITYID		(3u)
#define BD_TAGS_REMOVE_ALL_TAGS_FOR_ENTITYID	(4u)
#define BD_TAGS_SEARCH_BY_TAGS					(5u)
#define BD_TAGS_CUSTOM_QUERY					(6u)

#define TYPE_CHECKED true

bdTags::bdTags(bdRemoteTaskManager *const remoteTaskManager)
: m_remoteTaskManager(remoteTaskManager)
{
}

bdTags::~bdTags()
{
	m_remoteTaskManager = BD_NULL;
}

bdRemoteTaskRef bdTags::getTagsForEntityIDs(const bdUInt32 numEntityIDs,
											const bdEntityID entityIDs[],
											bdTagsArray tagResults[])
{
	// TODO: possibly use an array class here for the entityIDs?
	bdRemoteTaskRef task = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + BD_TYPED_ARRAY_HEADER_SIZE + (sizeof(bdUInt64)*numEntityIDs);
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TAG_SERVICE, BD_TAGS_GET_TAGS_FOR_ENTITYIDS);
	
	buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER64_TYPE, numEntityIDs, sizeof(bdUInt64));
	for(bdUInt i=0; i< numEntityIDs; i++)
	{
		buffer.writeUInt64(entityIDs[i]);
	}
	buffer.writeArrayEnd();
	
	m_remoteTaskManager->startTask(task, buffer);

	task->setTaskResult(tagResults, numEntityIDs);

	return task;
}

bdRemoteTaskRef bdTags::setTagsForEntityID(const bdEntityID entityID,
										   const bdUInt numTags,
										   const bdTag tags[])
{
	bdRemoteTaskRef task = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdEntityID) + BD_TYPED_ARRAY_HEADER_SIZE + (sizeof(bdUInt32)*numTags*2);
	
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TAG_SERVICE, BD_TAGS_SET_TAGS_FOR_ENTITYID);

	buffer.writeUInt64(entityID);
	buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER32_TYPE, numTags*2, sizeof(bdUInt32));
	for(bdUInt i=0; i< numTags; i++)
	{
		tags[i].serialize(buffer);
	}
	buffer.writeArrayEnd();
	
	m_remoteTaskManager->startTask(task, buffer);

	return task;
}

bdRemoteTaskRef bdTags::removeTagsForEntityID(const bdEntityID entityID,
											  const bdUInt numTags,
											  const bdTag tags[])
{
	bdRemoteTaskRef task = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdEntityID) + BD_TYPED_ARRAY_HEADER_SIZE + (sizeof(bdUInt32)*numTags*2);
	
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TAG_SERVICE, BD_TAGS_REMOVE_TAGS_FOR_ENTITYID);
	
	buffer.writeUInt64(entityID);
	buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER32_TYPE, numTags*2, sizeof(bdUInt32));
	for(bdUInt i=0; i< numTags; i++)
	{
		tags[i].serialize(buffer);
	}
	buffer.writeArrayEnd();

	m_remoteTaskManager->startTask(task ,buffer);

	return task;
}

bdRemoteTaskRef bdTags::removeAllTagsForEntityID(const bdEntityID entityID)
{
	bdRemoteTaskRef task = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdEntityID);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TAG_SERVICE, BD_TAGS_REMOVE_ALL_TAGS_FOR_ENTITYID);

	buffer.writeUInt64(entityID);

	m_remoteTaskManager->startTask(task ,buffer);

	return task;
}


bdRemoteTaskRef bdTags::searchByTags(const bdUInt startIndex,
                                     const bdUInt maxNumResults,
									 const bdUInt numTags,
									 const bdTag tags[],
									 bdFileID results[])
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD +  sizeof(startIndex) + sizeof(maxNumResults) + BD_TYPED_ARRAY_HEADER_SIZE + (sizeof(bdUInt32)*numTags*2);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TAG_SERVICE, BD_TAGS_SEARCH_BY_TAGS);

	buffer.writeUInt32(startIndex);
	buffer.writeUInt32(maxNumResults);

	buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER32_TYPE, numTags*2, sizeof(bdUInt32));
	for(bdUInt i=0; i< numTags; i++)
	{
		tags[i].serialize(buffer);
	}
	buffer.writeArrayEnd();
	
	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(results, maxNumResults);
	return remoteTask;
}
