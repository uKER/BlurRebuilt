// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdServerReplicaData.h>

#include <bdClientServer/bdReplica/bdUpdate/bdUpdateWindow.h>

#define BD_LOG_LEVEL "bdClientServer/serverreplicadata"

bdServerReplicaData::bdServerReplicaData()
: m_updateWindow(BD_NULL),
  m_nextUpdateNum(0)
{
}

bdServerReplicaData::~bdServerReplicaData()
{
	if (m_updateWindow)
	{
		delete m_updateWindow;
	}
}

bdUpdateWindow* bdServerReplicaData::createUpdateWindow()
{
	if(!m_updateWindow)
	{
		m_updateWindow = new bdUpdateWindow;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Replica data already has a bdUpdateWindow.");
	}

	return m_updateWindow;
}

bdUpdateWindow* bdServerReplicaData::getUpdateWindow()
{
	return m_updateWindow;
}

void bdServerReplicaData::addUpdate(const bdUInt32 dirtyFlags)
{
	if(m_updateWindow)
	{
		m_updateWindow->add(dirtyFlags);
	}
	m_nextUpdateNum++;
}

bdUpdateSeqNum bdServerReplicaData::getUpdateNum() const
{
	// if we have a window the replica is using delta updates
	// so get the sequence number from the window.
	bdUpdateSeqNum updateNum = m_nextUpdateNum - 1;
	if (m_updateWindow)
	{
		updateNum =  m_updateWindow->getUpdateNum();
	}

	return updateNum;
}

void bdServerReplicaData::addConnection(bdConnectionRef connection,
									    const bdUpdateSeqNum firstAck)
{
	// add to the hosts set
	if (!m_connections.insert(connection))
	{
		BD_WARN(BD_LOG_LEVEL,"Host already added to replicated hosts set.");
	}

	// add to the last ack map
	if(!m_lastAckMap.put(connection, firstAck))
	{
		BD_WARN(BD_LOG_LEVEL,"Host already added to last ack map.");

		// update the entry if it already exists
		updateAckMap(connection, firstAck);
	}
}

void bdServerReplicaData::removeConnection(bdConnectionRef connection)
{
	// remove from the hosts set
	if(!m_connections.remove(connection))
	{
		BD_WARN(BD_LOG_LEVEL,"Host not present in hosts set.");
	}

	// remove from the last ack map
	if(!m_lastAckMap.remove(connection))
	{
		BD_WARN(BD_LOG_LEVEL,"Host not present in last ack map.");
	}
}

void bdServerReplicaData::updateAckMap(bdConnectionRef connection,
									   const bdUpdateSeqNum lastAck)
{
	const bdLastAckMap::Iterator iter = m_lastAckMap.getIterator(connection);
	if(iter)
	{
		bdUpdateSeqNum &previousAck = m_lastAckMap.getValue(iter);
		previousAck = lastAck;
#ifdef BD_DEBUG
		m_lastAckMap.releaseIterator(iter);
#endif // BD_DEBUG
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"cannot find host in last ack map");
	}
}

bdBool bdServerReplicaData::getLastAck(bdConnectionRef connection,
									   bdUpdateSeqNum& lastAck) const
{
	return m_lastAckMap.get(connection, lastAck);
}

const bdSet<bdConnectionRef>& bdServerReplicaData::getConnectionSet() const
{
	return m_connections;
}
