// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A window containing a history of full bdReplica states.

#ifndef BD_ROLLBACK_WINDOW_H
#define BD_ROLLBACK_WINDOW_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdReplica/bdUpdate/bdUpdateSettings.h>

/// A window containing a history of full bdReplica states.
/// On a client each bdReplica that uses delta compressed update messages
/// is augmented with this data, which can be accessed via
/// bdReplica::getClientData()->getRollbackWindow().
/// The bdReplica states held in the window are used to rollback the
/// bdReplica to the state a delta compressed update is created with
/// respect to.
/// The window is kept in-sync with the corresponding bdUpdateWindow
/// on the server.
class bdRollbackWindow
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdRollbackWindow();

		struct bdRollbackEntry
		{
			BD_DECLARE_NEW_AND_DELETE_OPERATORS

			bdUpdateSeqNum m_updateSeqNum;
			bdBitBufferRef m_state;
		};

		void addRollbackState(bdBitBufferRef rollbackState,
							  const bdUpdateSeqNum updateSeqNum);

		bdBitBufferRef getRollbackState(const bdUpdateSeqNum updateSeqNum);

	protected:

		bdUpdateSeqNum m_lastServerUpdate;

		bdRollbackEntry m_rollbackEntries[BD_UPDATE_HISTORY_SIZE];
};

#endif // BD_ROLLBACK_WINDOW_H
