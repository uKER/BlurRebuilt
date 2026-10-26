// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef BD_REMOTE_TASK_MANAGER_H
#define BD_REMOTE_TASK_MANAGER_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyErrorCode.h>

BD_REFERENCE(bdLobbyConnection);
BD_REFERENCE(bdRemoteTask);


/// Internal helper class used by the various services to perform remote tasks.
class bdRemoteTaskManager 
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdRemoteTaskManager(bdLobbyConnectionRef connection,
							const bdBool useEncryption = true);

		virtual ~bdRemoteTaskManager();

		static void BD_CALL initTaskBuffer(	bdByteBuffer &buffer, 
											const bdUByte8 serviceID, 	
											const bdUByte8 taskID);

		bdLobbyErrorCode startTask(bdRemoteTaskRef &newTask,
									bdByteBuffer &queryParams);

		bdLobbyErrorCode startLSGTask(bdRemoteTaskRef &newTask,
									  const bdUByte8 serviceID,
									  const bdUByte8 taskID,
									  const void *const queryParams,
									  const bdUInt queryParamsSize);

	
		/// \return the connection ID. This will be 0 if the LSG has yet to respond with a connection ID.
		bdUInt64 getConnectionID() const;

	protected:		

		void handleTaskReply(const bdByteBufferRef buffer);

		void handleLSGTaskReply(const bdByteBufferRef buffer);
	
	protected:		

		friend class bdLobbyService;

		bdLinkedList<bdRemoteTaskRef> m_tasks;

		bdLobbyConnectionRef m_lobbyConnection;

		bdBool m_encryptedConnection;

		bdUInt64 m_connectionID;
};

#endif // BD_REMOTE_TASK_MANAGER_H
