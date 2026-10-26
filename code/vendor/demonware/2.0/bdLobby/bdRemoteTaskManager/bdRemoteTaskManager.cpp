// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>

#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyConnection.h>

#define BD_LOG_LEVEL "lobby/remotetaskmanager"


bdRemoteTaskManager::bdRemoteTaskManager(bdLobbyConnectionRef connection,
										 const bdBool useEncryption )
: m_lobbyConnection(connection),
  m_encryptedConnection(useEncryption),
  m_connectionID(0)
{
}

bdRemoteTaskManager::~bdRemoteTaskManager()
{
}

void BD_CALL bdRemoteTaskManager::initTaskBuffer(	bdByteBuffer &buffer,
													const bdUByte8 serviceID,
													const bdUByte8 taskID)
{
	buffer.write(&serviceID, 1);
	buffer.writeUByte8(taskID);
}

bdLobbyErrorCode bdRemoteTaskManager::startTask(bdRemoteTaskRef &newTask,
												bdByteBuffer &queryParams)
{
	bdLobbyErrorCode error = BD_NO_ERROR;
	newTask = new bdRemoteTask();

	// Write no type to indicate the end of the buffer.
	bdBool ok = queryParams.writeNoType();
	
	ok = ok && m_lobbyConnection->send(queryParams.getData(),queryParams.getDataSize(), m_encryptedConnection);
	if(!ok)
	{
		error = BD_SEND_FAILED;
	}

	if(error == BD_NO_ERROR)
	{
		m_tasks.addTail(newTask);
		newTask->start();
	}

	return error;
}

bdLobbyErrorCode bdRemoteTaskManager::startLSGTask(bdRemoteTaskRef &newTask,
												   const bdUByte8 serviceID,
												   const bdUByte8 taskID,
												   const void *const queryParams,
												   const bdUInt queryParamsSize)
{
	// fail safe
	bdLobbyErrorCode error = BD_SEND_FAILED;

	newTask  = new bdRemoteTask();
	if(newTask)
	{
		const bdUInt messageBufferSize = sizeof(serviceID) + sizeof(taskID) + queryParamsSize;
		bdByteBuffer messageBuffer(messageBufferSize);

		bdBool ok = messageBuffer.write(serviceID);
		ok = ok && messageBuffer.write(taskID);
		ok = ok && messageBuffer.write(queryParams, queryParamsSize);

		if(ok && m_lobbyConnection.notNull())
		{
			ok = m_lobbyConnection->send(messageBuffer.getData(), messageBuffer.getMaxReadSize(), m_encryptedConnection);
			if(ok)
			{
				error = BD_NO_ERROR;
				m_tasks.addTail(newTask);
				newTask->start();
			}
		}
	}

	if(newTask && error != BD_NO_ERROR)
	{
		newTask = BD_NULL;
	}

	return error;
}



void bdRemoteTaskManager::handleLSGTaskReply(const bdByteBufferRef buffer)
{
	bdRemoteTaskRef task = m_tasks.getHead();
	// The head of the list can be NULL if the task has been canceled
	if(task.notNull() && task->getStatus() == bdRemoteTask::BD_PENDING)
	{
		task->handleTaskReply(buffer);
	}
	m_tasks.removeHead();
}

void bdRemoteTaskManager::handleTaskReply(const bdByteBufferRef buffer)
{
	bdRemoteTaskRef  task = m_tasks.getHead();
	// The head of the list can be NULL if the task has been canceled
	if(task.notNull() && task->getStatus() == bdRemoteTask::BD_PENDING)
	{
		// get the transactionID 
		bdUInt64 transactionID = 0;
		bdBool ok = buffer->read(transactionID);
		if(ok)
		{
			task->stop(buffer, transactionID);
			BD_INFO(BD_LOG_LEVEL, "Received reply on\n"
								  "connection ID : " BD_UINT_64_FORMAT "\n" 
								  "transaction ID: " BD_UINT_64_FORMAT, m_connectionID, transactionID);
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to retrieve transaction ID");
		}
	}
	m_tasks.removeHead();
}


bdUInt64 bdRemoteTaskManager::getConnectionID() const
{
	return m_connectionID;
}

