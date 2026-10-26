// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
#include <bdLobby/bdContentUnlock/bdContentUnlock.h>
#include <bdLobby/bdLobbyCommon/bdAuthCommon.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

bdContentUnlock::bdContentUnlock(bdRemoteTaskManager *const remoteTaskManager)
: m_remoteTaskManager(remoteTaskManager)
{
	
}

bdContentUnlock::~bdContentUnlock()
{
	m_remoteTaskManager = BD_NULL;
}

bdRemoteTaskRef bdContentUnlock::listContentByLicenseCode(	const bdNChar8* licenseCode,
															bdUnlockContentInfo *content )
{
	bdUInt64 unlockKey = bdAuthUtility::getLicenseID(licenseCode);
	return listContent(unlockKey, content);
}

bdRemoteTaskRef bdContentUnlock::listContent(	const bdUInt64 unlockKey,
												bdUnlockContentInfo *content )
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(unlockKey);
	bdByteBuffer buffer(taskSize, true);

	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CONTENT_UNLOCK_SERVICE, BD_CONTENT_UNLOCK_LIST_CONTENT_BY_KEY);

	buffer.writeUInt64(unlockKey);

	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(content, 1);

	return remoteTask;	
}

bdRemoteTaskRef bdContentUnlock::unlockContentByLicenseCode(const bdNChar8* licenseCode,
															bdUnlockContentInfo *unlockedContent,
															const bdUInt64 hardwareID)

{
	bdUInt64 unlockKey = bdAuthUtility::getLicenseID(licenseCode);
	return unlockContent(unlockKey, unlockedContent, hardwareID);
}


bdRemoteTaskRef bdContentUnlock::unlockContent(const bdUInt64 unlockKey,
												bdUnlockContentInfo *unlockedContent,
												const bdUInt64 hardwareID)

{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(unlockKey) + sizeof(hardwareID);
	bdByteBuffer buffer(taskSize, true);

	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CONTENT_UNLOCK_SERVICE, BD_CONTENT_UNLOCK_UNLOCK_CONTENT);

	buffer.writeUInt64(unlockKey);
	buffer.writeUInt64(hardwareID);

	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(unlockedContent, 1);

	return remoteTask;
}

bdRemoteTaskRef bdContentUnlock::listUnlockedContent(bdUnlockContentInfo contentList[], bdUInt16 maxNumResults)
{
	bdRemoteTaskRef remoteTask = BD_NULL;

	bdUInt taskSize = BD_TASK_OVERHEAD;
	bdByteBuffer buffer(taskSize, true);

	bdRemoteTaskManager::initTaskBuffer(buffer, BD_CONTENT_UNLOCK_SERVICE, BD_CONTENT_UNLOCK_LIST_UNLOCKED_CONTENT);
	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(contentList, maxNumResults);

	return remoteTask;
}
