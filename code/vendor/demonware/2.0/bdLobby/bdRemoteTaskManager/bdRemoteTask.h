// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A handle to keep track of a Matchmaking+ remote task.

#ifndef BD_REMOTE_TASK_H
#define BD_REMOTE_TASK_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyErrorCode.h>

class bdTaskResult;

#define BD_TASK_OVERHEAD 64

/// A handle to keep track of a Matchmaking+ remote task.
/// A remote task handles the communication to the LSG and processes any
/// available results. The game code should pump the status of the remote task
/// until it has entered a 'complete' state.
class bdRemoteTask : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdStatus
		{
			BD_EMPTY,
			BD_PENDING,
			BD_DONE,
			BD_FAILED,
			BD_TIMED_OUT,
			BD_CANCELLED,
			BD_MAX_STATUS
		};

		/// Gets the current status of the task
		/// \return The task's status
		bdStatus getStatus();	

		/// Gets the number of result sets available for a completed task.
		/// This number is always less or equal to the number of results that were
		/// specified when creating the task.
		/// \return The number of available result sets
		bdUInt getNumResults() const;

		/// Returns the maximum number of results that were available for a completed
		/// task. This number may be greater or equal to the number of results that
		/// were specified when creating the task.
		/// \return The maximum number of available result sets
		bdUInt getTotalNumResults() const;

		/// Cancels a running task. 
		/// This sets the status of the tasks to BD_CANCELED, all results that
		/// are sent from the server for this task will be discarded.
		/// Note that canceling a task does not prevent that task from being executed on the server, 
		/// it simply indicates that the client is not interested in any results for this task.
		void cancelTask();

		/// Gets a reference to the internal byte buffer that is used to store the task results.
		/// \return A reference to the internal byte buffer
		bdByteBufferRef getByteResults() const;

		/// Gets the current task timeout value
		/// \return The current task timeout value in seconds
		bdFloat32 getTimeout() const;

		/// Sets the task timeout value
		void setTimeout(const bdFloat32 timeout);

		/// \return the transaction ID of the task (or 0 if status!=BD_DONE)
		bdUInt64 getTransactionID() const;

		/// Gets the error code of the last operation.
		/// \return The error code of the last operation.
		bdLobbyErrorCode getErrorCode() const;

		/// Sets the internal member for storing the task result
		void setTaskResult(	bdTaskResult *result, 
							const bdUInt numResults);

		/// Sets the internal member for storing the task result
		void setTaskResultList(	bdTaskResult **result, 
							    const bdUInt numResults);

		/// Gets a pointer to the task result.
		bdTaskResult *getTaskResult();

		/// Handles the reply for a given task.
		void handleTaskReply(const bdByteBufferRef buffer);

		/// Constructor
		bdRemoteTask();

		/// Destructor.
		~bdRemoteTask();

	protected:

		friend class bdRemoteTaskManager;
		friend class bdContentStreaming;

		void start(const bdFloat32 timeout=0);

		void stop(	const bdByteBufferRef results, 
					const bdUInt64 id);


	protected:

		bdStopwatch m_timer;

		bdFloat32 m_timeout;

		bdStatus m_status;

		bdByteBufferRef m_byteResults;

		bdTaskResult *m_taskResult;

		bdTaskResult **m_taskResultList;

		/// Actual number of results returned by the server
		bdUInt m_numResults;

		/// Maximum number of results that were requested for this task
		bdUInt m_maxNumResults;

		/// Total number of results that were available for this task
		bdUInt m_totalNumResults;

		/// The transaction ID for this taks
		bdUInt64 m_transactionID;

		/// error code store for look up
		bdLobbyErrorCode m_errorCode;

};

BD_REFERENCE(bdRemoteTask);

#endif // BD_REMOTE_TASK_H
