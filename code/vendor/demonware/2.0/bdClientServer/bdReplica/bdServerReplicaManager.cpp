// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdServerReplicaManager.h>

#include <bdConnection/bdConnection.h>
#include <bdConnection/bdConnectionStore.h>

#include <bdClientServer/bdReplica/bdReplicaTable.h>
#include <bdClientServer/bdReplica/bdServerReplicaData.h>
#include <bdClientServer/bdReplica/bdReplicaManagerOperations.h>
#include <bdClientServer/bdReplica/bdReplicaFactory.h>

#define BD_LOG_LEVEL "bdClientServer/serverreplicamanager"

bdServerReplicaManager::bdServerReplicaManager()
: m_replicaTable(BD_NULL),
  m_connections(BD_NULL)
{
}

void bdServerReplicaManager::init(bdReplicaTable *const replicaTable,
								  bdArray<bdConnectionRef> *const connections)
{
	m_replicaTable = replicaTable;
	m_connections = connections;
}

void bdServerReplicaManager::add(bdReplicaRef replica)
{
	replica->createServerData();

	// allocate a new ID from the ID service.
	const bdReplicaId replicaId = m_idManager.bind();	
	replica->setId(replicaId);

	// add into the server replica table
	if (!m_replicaTable->add(replica))
	{
		BD_WARN(BD_LOG_LEVEL,"Replica is already in the servers replica table.");
		return;	
	}

	// call initialize as the final step of the replica
	// being created.
	replica->initialize();

	refresh();
}

void bdServerReplicaManager::remove(bdReplicaRef replica)
{
	BD_ASSERT(replica && replica->getServerData(), "bdServerReplicaManager::remove, "
		"replica and/or primary data not set.");
	
	bdServerReplicaData *const serverData = replica->getServerData();

	const bdSet<bdConnectionRef> &connectionSet = serverData->getConnectionSet();
	bdSet<bdConnectionRef>::Iterator iter = connectionSet.getIterator();
	// Send a DESTROY_SECONDARY message to all host that have a replica of this object.
	while(iter)
	{
		bdMessageRef msg = new bdMessage(BD_SERVER_REPLICA_MANAGER_MESSAGE);
		//const bdAddr& host = hostSet.getAt(iter);
		bdConnectionRef connection = connectionSet.getAt(iter);
		connectionSet.next(iter);
		createDereplicateMessage(replica, msg);
		// Write a BD_NOOP to indicate the end of the message
		bdBitBufferRef buffer = msg->getPayload();
		buffer->writeRangedUInt32(BD_NOOP, 0, BD_MAX_REPLICA_MANAGER_OPERATION-1);
		//msg->send(host);
		connection->send(msg, true);
	}

	// remove from replica table
	if(!m_replicaTable->remove(replica))
	{
		BD_WARN(BD_LOG_LEVEL,"Could not remove replica from bdPrimaryReplicaTable, no entry found");
	}

	// free the objects ID in the idManager
	m_idManager.unbind(replica->getId());

}

void bdServerReplicaManager::refresh(bdReplicaRef replica, bdHashMap<bdConnectionRef, bdMessageRef> &replicationMsg)
{
	BD_ASSERT(replica && replica->getServerData(),"bdServerReplicaManager::removeAll, "
		"replica and/or primary data not set.");

	bdSet<bdConnectionRef> newRelevantHosts;	

	// TODO: this should get it's set of host from the list of current players
	// rather than the connection level which is too low.
//	bdConnectionStoreImpl *const connectionStore = bdConnectionStore::getInstance();
//	bdConnectionStoreImpl::Iterator iterator = connectionStore->getIterator();
//	while(iterator)
//	{
//		bdConnection *const connection = connectionStore->next(iterator);
//		newRelevantHosts.insert(connection->getHost());
//	}
	for (bdUInt i = 0; i < m_connections->getSize(); i++)
	{
		newRelevantHosts.insert((*m_connections)[i]);
	}

	// get the set of hosts on which this replica exists
	const bdSet<bdConnectionRef> &currentRelevantHosts = replica->getServerData()->getConnectionSet();

	// get the diffs of the 2 sets of hosts
	bdSet<bdConnectionRef> toDereplicate;
	bdSet<bdConnectionRef> toReplicate;
	bdSet<bdConnectionRef>::compare(currentRelevantHosts, newRelevantHosts, toDereplicate, toReplicate);

	// issue the dereplication messages
	bdSet<bdConnectionRef>::Iterator dereplicateIter = toDereplicate.getIterator();
	while(dereplicateIter)
	{
		bdConnectionRef connection = toDereplicate.next(dereplicateIter);
		bdMessageRef msg;
		if (!replicationMsg.get(connection, msg))
		{
			msg = new bdMessage(BD_SERVER_REPLICA_MANAGER_MESSAGE);
			//msg->setIsReliable();
			replicationMsg.put(connection, msg);
		}
		dereplicate(replica, connection, msg);
	}

	// issue the replication messages
	bdSet<bdConnectionRef>::Iterator replicateIter = toReplicate.getIterator();
	while(replicateIter)
	{
		bdConnectionRef connection = toReplicate.next(replicateIter);
		bdMessageRef msg;
		if  (!replicationMsg.get(connection, msg))
		{
			msg = new bdMessage(BD_SERVER_REPLICA_MANAGER_MESSAGE);
			//msg->setIsReliable();
			replicationMsg.put(connection, msg);
		}
		replicate(replica, connection, msg);

	}
}

void bdServerReplicaManager::refresh()
{
	bdReplicaTable::Iterator iter = m_replicaTable->getIterator();
	// This hashmap stores a message per host. We use this in order to accumulate
	// multiple replication messages that are sent to the same host during the
	// refresh phase.
	bdHashMap<bdConnectionRef, bdMessageRef> replicationMsg;
	while(iter)
	{
		bdReplicaRef replica = m_replicaTable->next(iter);
		refresh(replica, replicationMsg);
	}
	bdHashMap<bdConnectionRef, bdMessageRef>::Iterator msgIter = replicationMsg.getIterator();
	while(msgIter)
	{
		bdMessageRef msg = replicationMsg.getValue(msgIter);

		// Write a BD_NOOP to indicate the end of the message
		bdBitBufferRef buffer = msg->getPayload();
		buffer->writeRangedUInt32(BD_NOOP, 0, BD_MAX_REPLICA_MANAGER_OPERATION-1);
		replicationMsg.getKey(msgIter)->send(msg, true);
		replicationMsg.next(msgIter);
	}
}

void bdServerReplicaManager::replicate(bdReplicaRef replica,
									   bdConnectionRef connection,
									   bdMessageRef msg) const
{

	// Send a replicate message to the new host
	bdBitBufferRef buffer = msg->getPayload();
	buffer->writeRangedUInt32(BD_CREATE, 0, BD_MAX_REPLICA_MANAGER_OPERATION-1);
	buffer->writeRangedUInt32(replica->getType(), 0, bdReplicaFactory::getInstance()->getMaxType());

	buffer->BD_REPLICA_ID_WRITE(replica->getId());

	replica->writeReplicaInfo(buffer);
	replica->writeFullState(buffer);	
	
	bdServerReplicaData *const serverData = replica->getServerData();
	const bdUpdateSeqNum mostRecentUpdate = serverData->getUpdateNum();
	if (replica->testSettings(bdReplica::BD_DO_DELTA_COMPRESSION))
	{
		// write the update sequence number
		buffer->BD_UPDATE_SEQ_NUM_WRITE(mostRecentUpdate);
	}
	// add the host to the replicas server data
	serverData->addConnection(connection, mostRecentUpdate);
}

void bdServerReplicaManager::dereplicate(bdReplicaRef replica,
										 bdConnectionRef connection,
										 bdMessageRef msg) const
{
	createDereplicateMessage(replica, msg);
	bdServerReplicaData *const serverData = replica->getServerData();
	serverData->removeConnection(connection);
}

void bdServerReplicaManager::createDereplicateMessage(bdReplicaRef replica,
													  bdMessageRef msg) const
{
	bdBitBufferRef buffer = msg->getPayload();
	buffer->writeRangedUInt32(BD_DESTROY, 0, BD_MAX_REPLICA_MANAGER_OPERATION-1);

	buffer->BD_REPLICA_ID_WRITE(replica->getId());
}

void bdServerReplicaManager::removeConnection(bdConnectionRef connection)
{
	bdReplicaTable::Iterator iter = m_replicaTable->getIterator();
	while(iter)
	{
		const bdReplicaRef replica = m_replicaTable->next(iter);
		bdServerReplicaData *const serverData = replica->getServerData();
		serverData->removeConnection(connection);
	}
}

