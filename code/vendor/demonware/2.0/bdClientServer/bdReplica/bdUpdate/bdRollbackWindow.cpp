// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdUpdate/bdRollbackWindow.h>

bdRollbackWindow::bdRollbackWindow() :
m_lastServerUpdate(0)
{
	bdCheckUpdateSettings();
}

void bdRollbackWindow::addRollbackState(bdBitBufferRef rollbackState, 
										const bdUpdateSeqNum updateSeqNum)
{	
	const bdUpdateSeqNum newIndex = updateSeqNum & (BD_UPDATE_HISTORY_SIZE-1);

	// clear the states for any updates we missed/dropped
	// we don't really have to do this it's just to keep things tidy
	// and help spot any bugs.
	if(m_lastServerUpdate != updateSeqNum)
	{
		bdUpdateSeqNum nextIndex = (m_lastServerUpdate+1) & (BD_UPDATE_HISTORY_SIZE-1);
		while(nextIndex != newIndex)
		{
			bdRollbackEntry& rollbackEntry = m_rollbackEntries[nextIndex];
			rollbackEntry.m_state = BD_NULL;
			//rollbackEntry.m_updateSeqNum = 0;

			nextIndex = (nextIndex+1) & (BD_UPDATE_HISTORY_SIZE-1);
		}
	}

	// record the new state
	bdRollbackEntry& rollbackEntry = m_rollbackEntries[newIndex];
	rollbackEntry.m_state = rollbackState;
	rollbackEntry.m_updateSeqNum = updateSeqNum;

	// advance the window
	m_lastServerUpdate = updateSeqNum;
}

bdBitBufferRef bdRollbackWindow::getRollbackState(const bdUpdateSeqNum updateSeqNum)
{
	bdBitBufferRef rollBackState;

	// check the rollback state is in the window
	bdBool inWindow = static_cast<bdUpdateSeqNum>(m_lastServerUpdate - updateSeqNum) < BD_UPDATE_HISTORY_SIZE;
	if(inWindow)
	{
		const bdUpdateSeqNum index = updateSeqNum & (BD_UPDATE_HISTORY_SIZE-1);
		bdRollbackEntry& rollbackEntry = m_rollbackEntries[index];
		
		// this should never happen
		BD_ASSERT(rollbackEntry.m_state != BD_NULL, 
			"bdClientDeltaUpdateData::getRollbackState, invalid (BD_NULL) rollback state.");

		// this should never happen
		BD_ASSERT(rollbackEntry.m_updateSeqNum == updateSeqNum, 
			"bdClientDeltaUpdateData::getRollbackState, invalid rollback state. "
			"Update sequence numbers don't match.");

		if(rollbackEntry.m_updateSeqNum == updateSeqNum)
		{
			rollBackState = rollbackEntry.m_state;
		}

	}
/*	else
	{
		bdPrintf("Must be full update %u\n", updateSeqNum);
	}
*/
	return rollBackState;
}

