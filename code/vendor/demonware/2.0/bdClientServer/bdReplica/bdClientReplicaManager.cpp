// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdClientReplicaManager.h>
#include <bdClientServer/bdReplica/bdReplicaTable.h>
#include <bdClientServer/bdReplica/bdReplicaFactory.h>
#include <bdClientServer/bdReplica/bdClientReplicaListener.h>
#include <bdClientServer/bdReplica/bdReplicaManagerOperations.h>
#include <bdClientServer/bdReplica/bdClientReplicaData.h>
#include <bdClientServer/bdReplica/bdUpdate/bdRollbackWindow.h>

#include <bdConnection/bdMessage/bdMessage.h>

#define BD_LOG_LEVEL "bdClientServer/clientreplicamanager"

bdClientReplicaManager::bdClientReplicaManager()
: m_replicaTable(BD_NULL)
{
}

void bdClientReplicaManager::init(bdReplicaTable *const replicaTable)
{
	m_replicaTable = replicaTable;
}

void bdClientReplicaManager::registerReplicaListener(bdClientReplicaListener *const listener)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_listeners.findFirst(listener, index))
	{
		BD_WARN(BD_LOG_LEVEL,"same listener registered multiple times.");
	}
#endif // BD_DEBUG

	m_listeners.pushBack(listener);
}

void bdClientReplicaManager::unregisterReplicaListener(bdClientReplicaListener *const listener)
{
	m_listeners.removeAllKeepOrder(listener);
}

void bdClientReplicaManager::handle(bdMessageRef msg)
{

	bdBitBufferRef buffer = msg->getPayload();

	bdUInt32 iType;
	while (buffer->readRangedUInt32(iType,	0, (BD_MAX_REPLICA_MANAGER_OPERATION - 1)))
	{
	
		const bdReplicaManagerOperation operation = static_cast<bdReplicaManagerOperation>(iType);

		switch(operation)
		{
			case BD_CREATE:
			{
				handleCreateSecondary(msg);
			}
			break;
			case BD_DESTROY:
			{
				handleDestroySecondary(msg);
			}
			break;
			case BD_NOOP:
			{
				return;
			}
			default:
			{
				BD_WARN(BD_LOG_LEVEL,"invalid type.");
			}
		}
	}
}

void bdClientReplicaManager::handleCreateSecondary(bdMessageRef msg)
{
	bdReplicaFactoryImpl *const factory  = bdReplicaFactory::getInstance();
	
	bdBitBufferRef buffer = msg->getPayload();

	bdUInt32 objectType = 0;
	if(!buffer->readRangedUInt32(objectType, 0, factory->getMaxType()))
	{
		bdPrintf("Creating object of type %u\n", objectType);
		BD_WARN(BD_LOG_LEVEL,"no type.");
		return;
	}

	bdReplicaRef replica = factory->create(objectType);
	if (replica)
	{	
		// read the replica id
		bdReplicaId replicaId = 0;
		if(!buffer->BD_REPLICA_ID_READ(replicaId))
		{
			BD_ERR(BD_LOG_LEVEL, "no replica id.");
			return;
		}
		replica->setId(replicaId);

		// notify the application about the new replica.
		for(bdUInt32 i = 0; i < m_listeners.getSize(); i++)
		{
			m_listeners[i]->onClientCreate(replica);
		}
		
		// create the secondary data
		bdClientReplicaData *const clientData = replica->createClientData();

		// read the infrequently changed replica state
		replica->readReplicaInfo(buffer);

		// read the frequently changed replica state
		if (!replica->deserialize(buffer))
		{
			BD_ERR(BD_LOG_LEVEL, " Deserialization of replica failed.");
			return;
		}		
		
		if (replica->testSettings(bdReplica::BD_DO_DELTA_COMPRESSION))
		{
			bdUpdateSeqNum updateSeqNum;
			// read the update sequence number			
			if(buffer->BD_UPDATE_SEQ_NUM_READ(updateSeqNum))
			{
				// store this initial state so we can revert back to it
				bdBitBufferRef initialState = new bdBitBuffer;
				replica->writeFullState(initialState);
				bdRollbackWindow *const rollbackWindow = clientData->getRollbackWindow();
				rollbackWindow->addRollbackState(initialState, updateSeqNum);
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Cannot read update sequence number from create measage.");
			}
		}

		// add to the id to replica lookup table
		m_replicaTable->add(replica);

		// notify the application that the replica is now initialized.
		for(bdUInt32 j = 0; j < m_listeners.getSize(); j++)
		{
			m_listeners[j]->onClientInitialize(replica);
		}
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "secondary replica creation failed");
	}
}

void bdClientReplicaManager::handleDestroySecondary(bdMessageRef msg)
{
	bdBitBufferRef buffer = msg->getPayload();

	bdReplicaId replicaId = 0;
	if(!buffer->BD_REPLICA_ID_READ(replicaId))
	{
		BD_WARN(BD_LOG_LEVEL,"no id");
		return;	
	}
	bdReplicaRef replica = m_replicaTable->get(replicaId);

	if (replica)
	{
		m_replicaTable->remove(replicaId);

		// Notify the application about the impending deletion.
		for(bdUInt32 i = 0; i < m_listeners.getSize(); i++)
		{
			m_listeners[i]->onClientDelete(replica);
		}

		if (replica->getRefCount() != 1)
		{
			BD_WARN(BD_LOG_LEVEL,"Cannot delete replica, as external references are still held.");
		}
	}
}



