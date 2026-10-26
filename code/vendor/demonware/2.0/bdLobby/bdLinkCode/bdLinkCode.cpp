// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdLinkCode/bdLink.h>
#include <bdLobby/bdLinkCode/bdLinkCode.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>


#define BD_LOG_LEVEL "linkcode"
#define TYPE_CHECKED true


bdLinkCode::bdLinkCode(bdRemoteTaskManager *const remoteTaskManager)
	: m_remoteTaskManager(remoteTaskManager)
{
}

bdLinkCode::~bdLinkCode()
{
}

bdRemoteTaskRef bdLinkCode::getLinkCodes(const bdUInt64 entityID,
										 const bdUInt32 *const categories,
										 bdLink *const links,
										 const bdUInt numCategories)
{
	bdUInt taskSize = BD_TASK_OVERHEAD + 1 + sizeof(bdUInt64) + numCategories + (numCategories * sizeof(bdUInt32));

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_LINKCODE_SERVICE, BD_LINKCODE_GET_LINK_CODES);

	bdBool ok = true;

	ok = ok && buffer.writeUInt64(entityID);
	for (bdUInt i = 0; ok && i < numCategories; ++i)
	{
		ok = ok && buffer.writeUInt32(categories[i]);
	}

	if (ok)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult == BD_NO_ERROR)
		{
			remoteTask->setTaskResult(links, numCategories);
		}
		else
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

bdRemoteTaskRef bdLinkCode::getEntityIDsFromLinkCodes(const bdUInt32 category,
													  const bdNChar8 *const *const linkCodes,
													  bdLink *const links,
													  const bdUInt numLinkCodes)
{
	bdUInt taskSize = BD_TASK_OVERHEAD + 1 + sizeof(bdUInt32) + numLinkCodes;
	for (bdUInt i = 0; i < numLinkCodes; ++i)
	{
		taskSize += (bdUInt)bdStrnlen(linkCodes[i], BD_MAX_LINK_CODE_LENGTH);
	}

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskRef remoteTask = BD_NULL;
	m_remoteTaskManager->initTaskBuffer(buffer, BD_LINKCODE_SERVICE, BD_LINKCODE_GET_ENTITY_IDS_FROM_LINK_CODES);

	bdBool ok = true;

	ok = ok && buffer.writeUInt32(category);
	for (bdUInt i = 0; ok && i < numLinkCodes; ++i)
	{
		ok = ok && buffer.writeString(linkCodes[i], BD_MAX_LINK_CODE_LENGTH);
	}

	if (ok)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult == BD_NO_ERROR)
		{
			remoteTask->setTaskResult(links, numLinkCodes);
		}
		else
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
