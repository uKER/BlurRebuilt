// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdUpdate/bdUpdateWindow.h>

bdUpdateWindow::bdUpdateWindow()
: m_nextUpdateNum(0)
{
	bdCheckUpdateSettings();
	// add any other initialisation stuff into reset
	bdMemset(m_dirtyFlags, BD_NULL, sizeof(bdUInt32)*BD_UPDATE_HISTORY_SIZE);
}

void bdUpdateWindow::add(const bdUInt32 dirtyFlags)
{
	const bdUpdateSeqNum nextIndex = m_nextUpdateNum & (BD_UPDATE_HISTORY_SIZE-1);
	m_dirtyFlags[nextIndex] = dirtyFlags;
	m_nextUpdateNum++;
}

bdBool bdUpdateWindow::inWindow(const bdUpdateSeqNum updateNum) const
{
	return static_cast<bdUpdateSeqNum>((m_nextUpdateNum - 1) - updateNum) < BD_UPDATE_HISTORY_SIZE;
}

bdUpdateSeqNum bdUpdateWindow::getUpdatesSince(const bdUpdateSeqNum updateNum) const
{
	bdUpdateSeqNum updatesSince = 0;
	if(inWindow(updateNum))
	{
		updatesSince = static_cast<bdUpdateSeqNum>((m_nextUpdateNum - 1) - updateNum);
	}

	return updatesSince;
}

bdUInt32 bdUpdateWindow::get(const bdUpdateSeqNum updateNum) const
{
	bdUInt32 result = 0;
	if (inWindow(updateNum))
	{
		const bdUpdateSeqNum updateIndex = updateNum & (BD_UPDATE_HISTORY_SIZE-1);
		result = m_dirtyFlags[updateIndex];
	}

	return result;
}


bdUpdateSeqNum bdUpdateWindow::getUpdateNum() const
{
	return m_nextUpdateNum - 1;
}
