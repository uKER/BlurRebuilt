// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdReplica.h>

#include <bdClientServer/bdReplica/bdServerReplicaData.h>
#include <bdClientServer/bdReplica/bdClientReplicaData.h>
#include <bdClientServer/bdReplica/bdInterpolationData.h>
#include <bdClientServer/bdCommand/bdCommandWindow.h>
#include <bdClientServer/bdReplica/bdUpdate/bdRollbackWindow.h>

#define BD_LOG_LEVEL "bdClientServer/replica"

bdReplica::bdReplica()
: m_id(0),
  m_settings(BD_MAX_REPLICA_SETTING-1),
  m_serverData(BD_NULL),
  m_clientData(BD_NULL)
{
	m_type = BD_REPLICA;
}

bdReplica::~bdReplica()
{
	if(m_serverData)
	{
		delete m_serverData;		
	}

	if(m_clientData)
	{
		delete m_clientData;		
	}
}

void bdReplica::setId(const bdReplicaId id)
{
	m_id = id;
}

const bdReplicaId& bdReplica::getId() const
{
	if(!m_id)
	{
		BD_WARN(BD_LOG_LEVEL,"replica has invalid id.");
	}
	return m_id;
}

bdReplicaType bdReplica::getType() const
{
	return m_type;
}

void bdReplica::initialize()
{
}

void bdReplica::processCommand(bdUserCommand *const command,
							   const bdBool newCommand)
{
	// apply the command locally
	applyCommand(command, newCommand);
	
	// apply the command to the interpolation target, if one exists
	if(m_clientData &&
	   m_clientData->getInterpolationData() &&
	   m_clientData->getInterpolationData()->getTarget() &&
	   m_clientData->getInterpolationData()->getActive())	
	{
		bdInterpolationInterface *const target = m_clientData->getInterpolationData()->getTarget();
		target->applyCommand(command, newCommand);
	}
}

void bdReplica::step(const bdFloat32 secs,
					 const bdBool doExtrapolation)
{
	if (doExtrapolation)
	{
		// extrapolate the replica 
		extrapolate(secs, false);
	}

	// extrapolate the interpolation target (if any)
	bdInterpolationData *const interpolationData = m_clientData->getInterpolationData();
	if(interpolationData && interpolationData->getActive())
	{
		bdInterpolationInterface *const target = interpolationData->getTarget();
		const bdFloat32 interpolationRatio = interpolationData->getNextInterpolationStep(secs);

		if (doExtrapolation)
		{
			target->extrapolate(secs, false);
		}

		// interpolate towards the target
		if(interpolationRatio)
		{
			interpolate(target, interpolationRatio);
		}
	}
}

void bdReplica::writeReplicaInfo(bdBitBufferRef)
{
}

void bdReplica::readReplicaInfo(bdBitBufferRef)
{
}

bdServerReplicaData* bdReplica::createServerData()
{
	if (!m_serverData)
	{
		m_serverData = new bdServerReplicaData;
		if(testSettings(BD_DO_DELTA_COMPRESSION))
		{
			m_serverData->createUpdateWindow();
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"replica already has bdServerReplicaData.");
	}
	return m_serverData;
}
 
bdServerReplicaData* bdReplica::getServerData()
{
	return m_serverData;
}

bdClientReplicaData* bdReplica::createClientData()
{
	if (!m_clientData)
	{
		m_clientData = new bdClientReplicaData();
		if (testSettings(BD_DO_DELTA_COMPRESSION))
		{
			m_clientData->createRollbackWindow();			
		}
	}
	return m_clientData;
}

bdClientReplicaData* bdReplica::getClientData()
{
	return m_clientData;
}

bdBool bdReplica::createUpdate(bdConnectionRef connection,
							   bdBitBufferRef buffer)
{
	// create an update about this object for the specified host

	// is this replica using delta updates?
	if(testSettings(BD_DO_DELTA_COMPRESSION))
	{
		// delta update
		return createDeltaUpdate(connection, buffer);
	}
	else
	{
		// non delta update
		return createNonDeltaUpdate(connection, buffer);
	}
}

bdBool bdReplica::createDeltaUpdate(bdConnectionRef connection,
									bdBitBufferRef buffer)
{
	BD_ASSERT(m_serverData != BD_NULL, "bdReplica::createDeltaUpdate, "
		"replica must have valid bdServerReplicaData.");

	bdUpdateWindow *const updateWindow = m_serverData->getUpdateWindow();

	BD_ASSERT(updateWindow != BD_NULL, "bdReplica::createDeltaUpdate, "
		"replica must have valid bdUpdateWindow.");

	// get the last acknowledged sequence number from the host
	// and the number of updates since
	bdUpdateSeqNum lastAck;
	bdUpdateSeqNum numUpdates = 0;
	bdBool insideWindow = false;
	bdUInt32 combinedDirtyFlags = 0;

	if(m_serverData->getLastAck(connection, lastAck))
	{
		// is the lastAck still inside the updateWindow
		if(updateWindow->inWindow(lastAck))
		{
			insideWindow = true;
			numUpdates = updateWindow->getUpdatesSince(lastAck);

			// if there are no new updates just return
			// NOTE: This isn't quite correct as sequence nums wrap.
			// The client could be very far out of date, requiring
			// and update and this condition may still be true, the
			// only way around this is to increase the range of the
			// sequence numbers. Hopefully it'll get another update
			// sometime soon.
			if(numUpdates == 0)
			{
				return false;
			}

			// Aggregate all the dirty flags in the window together.
			for (bdUByte8 i = 0; i < numUpdates; i++)
			{
				combinedDirtyFlags |= updateWindow->get(lastAck+1+i);
			}			
		}	
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"cannot find lastAck for replica from given host.");
	}

	if(!insideWindow)
	{
		lastAck = updateWindow->getUpdateNum();
		numUpdates = 0;
	}

	// write the replica id
	buffer->BD_REPLICA_ID_WRITE(m_id);
	// write the last seen update number
	buffer->BD_UPDATE_SEQ_NUM_WRITE(lastAck);
	// write the number of updates in this message
	buffer->writeRangedUInt32(numUpdates, 0, BD_UPDATE_HISTORY_SIZE);

	if(insideWindow)
	{
		//bdPrintf("<- Update %u - %u\n", lastAck, static_cast<bdUByte8>(lastAck + numUpdates));

		// write the interpolation break flag.
		const bdBool interpolationBreak = combinedDirtyFlags & (bdUInt32)BD_INTERPOLATION_BREAK?true:false;
		buffer->writeBool(interpolationBreak);

		const bdUInt32 backupDirtyFlags = m_dirtyFlags;
		m_dirtyFlags = combinedDirtyFlags;
		// serialize all the dirty fields
		serialize(buffer);
		m_dirtyFlags = backupDirtyFlags;
	}
	else
	{
		//bdPrintf("<- Full update %u\n", lastAck);

		// write the interpolation break flag.
		buffer->writeBool(true);
		writeFullState(buffer);
	}

	return true;
}

bdBool bdReplica::createNonDeltaUpdate(bdConnectionRef connection,
									   bdBitBufferRef buffer)
{
	BD_ASSERT(m_serverData != BD_NULL, "bdReplica::createNonDeltaUpdate, "
		"replica must have valid bdServerReplicaData.");

	const bdUpdateSeqNum updateNum = m_serverData->getUpdateNum();

	bdUpdateSeqNum lastAck; 
	if(m_serverData->getLastAck(connection, lastAck))
	{
		// already acked that latest update so just return
		// NOTE: This isn't quite correct as sequence nums wrap.
		// The client could be very far out of date, requiring
		// and update and this condition may still be true, the
		// only way around this is to increase the range of the
		// sequence numbers.Hopefully it'll get another update
		// sometime soon.
		if(lastAck == updateNum)
		{
			return false;
		}
	}

	buffer->BD_REPLICA_ID_WRITE(m_id);
	buffer->BD_UPDATE_SEQ_NUM_WRITE(updateNum);
	buffer->writeBool(testDirtyFlags(static_cast<bdUInt32>(BD_INTERPOLATION_BREAK)));
	serialize(buffer);
	return true;
}

bdBool bdReplica::receiveUpdate(bdBitBufferRef buffer,
								bdUpdateSeqNum& updateSeqNum)
{

	bdInterpolationInterface *target = this;
	bdInterpolationData *const interpolationData = m_clientData->getInterpolationData();
	if(interpolationData && interpolationData->getActive() && interpolationData->getTarget())
	{
		target = interpolationData->getTarget();
		interpolationData->startInterpolation();
	}

	bdBool ok = true;
	bdBool interpolationBreak = false;

	// is this replica using delta updates?
	if(testSettings(BD_DO_DELTA_COMPRESSION))
	{
		// delta update
		ok =  receiveDeltaUpdate(buffer, target, updateSeqNum, interpolationBreak);
	}
	else
	{
		// non delta update
		ok = receiveNonDeltaUpdate(buffer, target, updateSeqNum, interpolationBreak);
	}

	if(interpolationData && interpolationBreak)
	{
		interpolationData->endInterpolation();
	}

	return ok;
}

bdBool bdReplica::receiveDeltaUpdate(bdBitBufferRef buffer,									 
									 bdInterpolationInterface *const target,
									 bdUpdateSeqNum& updateSeqNum,
									 bdBool& interpolationBreak)
{
	BD_ASSERT(m_clientData != BD_NULL, "bdReplica::receiveDeltaUpdate, "
		"replica must have valid client data.");

	bdBool ok = true;

	bdUpdateSeqNum fromStateSeqNum = 0;
	bdUInt32 numUpdates = 0;
	
	// read the sequence number of the from state
	ok = ok && buffer->BD_UPDATE_SEQ_NUM_READ(fromStateSeqNum);
	// read the number of updates in the message
	ok = ok && buffer->readRangedUInt32(numUpdates, 0, BD_UPDATE_HISTORY_SIZE);
	// read the interpolation break flag
	ok = ok && buffer->readBool(interpolationBreak);

	if(!ok)
	{
		BD_WARN(BD_LOG_LEVEL,"malformed update message.");
	}


	if(ok)
	{
		bdRollbackWindow *const rollbackWindow = m_clientData->getRollbackWindow();
		bdBitBufferRef fromState = rollbackWindow->getRollbackState(fromStateSeqNum);
		
		if(fromState)
		{
			const bdUInt32 readPos = fromState->getReadPosition();
			if(!target->deserialize(fromState))
			{
				BD_ERR(BD_LOG_LEVEL, "Deserialization of rollback state failed.");
				ok = false;
			}
			fromState->setReadPosition(readPos);
		}

		// apply the update
		if(ok)
		{
			if(!target->deserialize(buffer))
			{
				BD_ERR(BD_LOG_LEVEL, "deserialization of update state failed.");	
				ok = false;
			}
		}

		if(ok)
		{
			updateSeqNum = fromStateSeqNum + static_cast<bdUpdateSeqNum>(numUpdates);

			// save the new state for future roll back
			bdBitBufferRef newState = new bdBitBuffer;
			target->writeFullState(newState);
			rollbackWindow->addRollbackState(newState, updateSeqNum);

			//	bdPrintf("-> Update %u - %u\n", fromStateSeqNum, updateSeqNum /*fromStateSeqNum + numUpdates*/);
		}
	}
	return ok;
}

bdBool bdReplica::receiveNonDeltaUpdate(bdBitBufferRef buffer,
										bdInterpolationInterface *const target,
										bdUpdateSeqNum& updateSeqNum,
										bdBool& interpolationBreak) const
{
	bdBool ok = true;
	bdUpdateSeqNum updateNum = 0;
	
	ok = ok && buffer->BD_UPDATE_SEQ_NUM_READ(updateNum);
	ok = ok && buffer->readBool(interpolationBreak);
	ok = ok && target->deserialize(buffer);

	updateSeqNum = updateNum;
	return ok;
}

void bdReplica::setSettings(const bdUInt32 settings)
{
	m_settings |= settings;
}

void bdReplica::clearSettings(const bdUInt32 settings)
{
	m_settings &= ~(settings);
}

bdBool bdReplica::testSettings(const bdUInt32 settings) const
{
	return (m_settings & settings) == settings;
}

void bdReplica::serialize(bdBitBufferRef)
{
}

bdBool bdReplica::deserialize(bdBitBufferRef)
{
	return true;
}

void bdReplica::interpolate(bdInterpolationInterface *const, 
							const bdFloat32)
{
}


