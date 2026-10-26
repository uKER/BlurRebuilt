// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdServer.h>

#include <bdConnection/bdConnection.h>
#include <bdConnection/bdUnicastConnection.h>
#include <bdConnection/bdConnectionStore.h>
#include <bdConnection/bdMessage/bdReceivedMessage.h>

#include <bdNet/bdNet.h>
#include <bdNet/bdLANDiscovery/bdGameInfo.h>

#include <bdClientServer/bdClientSettings.h>
#include <bdClientServer/bdPlayer/bdServerPlayerListener.h>
#include <bdClientServer/bdPlayer/bdInterest/bdFullInterest.h>

#include <bdClientServer/bdCommand/bdCommandFactory.h>
#include <bdClientServer/bdReplica/bdServerReplicaData.h>
#include <bdClientServer/bdPlayer/bdPlayerOperations.h>
#include <bdClientServer/bdPlayer/bdPlayerFactory.h>

#define BD_LOG_LEVEL "bdClientServer/server"

bdPlayerId bdServer::m_maxPlayerId = 0;

bdServer::bdServer()
: m_updateInterval(0.032f),
  m_processTime(0),
  m_interest(BD_NULL)
{
	
}

bdServer::~bdServer()
{
	bdNet::getInstance()->getConnectionStore()->unregisterDispatchInterceptor(this);

	// We may have registered with bdConnections as a listener so make sure we 
	// unregister ourselves
	for (bdUInt i = 0 ; i < m_connections.getSize(); i++)
	{
		m_connections[i]->unregisterListener(this);
	}
	if(m_interest)
	{
		delete m_interest;
	}

}

void bdServer::init(bdGameInfoRef gameInfo)
{
	bdNetImpl *net = bdNet::getInstance();
	
	// General initialization
	net->getConnectionStore()->registerDispatchInterceptor(this);
	m_replicaManager.init(&m_replicaTable, &m_connections);
	m_interest = new bdFullInterest(&m_replicaTable);
	net->getSocketRouter()->getKeyMap()->registerKey(gameInfo->getSecurityID(), gameInfo->getSecurityKey());

}


void bdServer::registerPlayerListener(bdServerPlayerListener *const listener)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_playerListeners.findFirst(listener, index))
	{
		BD_WARN(BD_LOG_LEVEL,"same listener registered multiple times.");
	}
#endif // BD_DEBUG

	m_playerListeners.pushBack(listener);
}

void bdServer::unregisterPlayerListener(bdServerPlayerListener *const listener)
{
	m_playerListeners.removeAllKeepOrder(listener);
}

void bdServer::setUpdateInterval(const bdFloat32 secs)
{
	m_updateInterval = secs;
}

const bdServerReplicaManager* bdServer::getReplicaManager() const
{
	return &m_replicaManager;
}

void bdServer::setInterest(bdInterestInterface *const interest)
{
	if(interest)
	{
		if(m_interest)
		{
			delete m_interest;
		}
		m_interest = interest;
	}	
}

bdServerReplicaManager* bdServer::getReplicaManager()
{
	return &m_replicaManager;
}

void bdServer::setControlledReplica(bdPlayerRef player,
									bdReplicaRef replica)
{
	bdConnectionRef connection = player->getServerData()->getConnection();

	// send a message to the players host to change replica
	bdMessageRef controlMsg = new bdMessage(BD_SERVER_PLAYER_MESSAGE);

	bdBitBufferRef buffer = controlMsg->getPayload();

	buffer->writeRangedUInt32(BD_CONTROL_REPLICA, 0, BD_MAX_PLAYER_OPERATION-1);

	// write the player id to the message
	buffer->BD_PLAYER_ID_WRITE(player->getId());

	// write the replica id to the message
	buffer->BD_REPLICA_ID_WRITE(replica->getId());

	connection->send(controlMsg, true);
	// do the change over

	// remove the currently controlled replica from the control map
	bdReplicaRef currentReplica = player->getControlledReplica();
	if(currentReplica)
	{
		if(!m_controlMap.remove(currentReplica))
		{
			BD_WARN(BD_LOG_LEVEL,"cannot find currently controlled replica in m_controlMap");
		}		
	}

	// remove the current replica from the control map
	bdPlayerRef currentPlayer;
	m_controlMap.remove(replica, currentPlayer);
	if(currentPlayer)
	{
		// ensure that only one player is controlling this replica
		currentPlayer->setControlledReplica(BD_NULL);
	}

	// add the new player/replica pair to the control map
	if(!m_controlMap.put(replica, player))
	{
		BD_WARN(BD_LOG_LEVEL,"replica is already controlled by a bdPlayer");
	}

	// clear all commands 
	player->getServerData()->resetCommands();
	player->setControlledReplica(replica);
}

bdPlayerRef bdServer::getControllingPlayer(bdReplicaRef replica)
{
	bdPlayerRef player;
	m_controlMap.get(replica, player);
	return player;
}

void bdServer::step(const bdFloat32 time)
{
	// are we going to send updates this frame?
	bdBool updateFrame = false;

	// record the step time
	m_processTime += time;


	if(m_processTime >= m_updateInterval)
	{
		updateFrame = true;
		m_processTime -= m_updateInterval;		
	}

	updateReplicas(time, updateFrame); 
}
 
void bdServer::step()
{
	updateReplicas(0.f, true);
}

bdPlayerRef bdServer::getPlayer(const bdPlayerId playerId) const
{
	// TODO: make this lookup O(1)
	for(bdUInt32 i = 0; i < m_players.getSize(); i++)
	{
		if(m_players[i].notNull() && (m_players[i]->getId() == playerId))
		{
			return m_players[i];
		}
	}

	return BD_NULL;
}

bdBool bdServer::accept(const bdReceivedMessage& receivedMsg)

{
	bdMessageRef msg = receivedMsg.getMessage();
	const bdMessageType type = msg->getType();
	switch (type)
	{
		case BD_CLIENT_PLAYER_MESSAGE:
		{
			handlePlayerMessage(receivedMsg);
			return true;
		}
		case BD_CLIENT_UPDATE_ACK_MESSAGE:
		{
			handleUpdateAck(receivedMsg);
			return true;
		}
		case BD_USER_COMMAND_MESSAGE:
		{
			// look up the player that sent this command
			bdPlayerId playerId;
			msg->getPayload()->BD_PLAYER_ID_READ(playerId);

			bdPlayerRef player = getPlayer(playerId);

			if (player.notNull())
			{				
				handleUserCommandMessage(player, msg);				
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL,"player lookup failed.");
			}

			return true;
		}
		default:
		{
			return false;
		}
	}
}

void bdServer::onDisconnect(bdConnectionRef connection)
{
	for(bdUInt32 i = 0; i < m_players.getSize(); i++)
	{
		if(m_players[i]->getServerData()->getConnection() == connection)
		{
			removePlayer(m_players[i]);
			// Need to recheck the same slot
			i--;
		}
	}

	connection->unregisterListener(this);
}

void bdServer::handlePlayerMessage(const bdReceivedMessage& receivedMsg)
{	
	bdMessageRef msg = receivedMsg.getMessage();
	bdBitBufferRef buffer = msg->getPayload();
	bdUInt32 iType;
	if(!buffer->readRangedUInt32(iType, 0, (BD_MAX_PLAYER_OPERATION-1)))
	{
		BD_WARN(BD_LOG_LEVEL,"no type");
		return;
	}

	const bdPlayerOperation operation = static_cast<bdPlayerOperation>(iType);

	switch(operation)
	{
		case BD_JOIN:
		{	
			bdConnectionRef connection = receivedMsg.getConnection();
			bdBool found = false;
			for(bdUInt32 i = 0; i < m_connections.getSize(); i++)
			{
				if(m_connections[i] == connection)
				{
					found = true;
					break;
				}
			}

			// this message is from a new host so register for disconnects
			
			if(!found)
			{				
				connection->registerListener(this);
			}
			
			
			handlePlayerJoin(receivedMsg);
		}
		break;
		case BD_LEAVE:
		{
			handlePlayerLeave(msg);			
		}
		break;
		default:
		{
			BD_WARN(BD_LOG_LEVEL,"cannot handle this bdPlayerOperation on a server.");
		}
	}
}

void bdServer::handlePlayerJoin(const bdReceivedMessage& receivedMsg)
{
	bdMessageRef msg = receivedMsg.getMessage();
	bdBitBufferRef requestBuffer = msg->getPayload();

	bdUInt32 requestId;
	if(!requestBuffer->readRangedUInt32(requestId, 0, BD_MAX_CLIENT_PLAYERS-1))
	{
		BD_WARN(BD_LOG_LEVEL,"no request id");
		return;
	}

	// set up the reply
	//bdMessageRef replyMsg = bdReplyManager::setupReply(joinRequest, BD_SERVER_PLAYER_MESSAGE);
	bdMessageRef replyMsg = new bdMessage(BD_SERVER_PLAYER_MESSAGE);

	bdBitBufferRef replyBuffer = replyMsg->getPayload();

	bdBool canJoin = true;

	// inform the listeners of the join request
	bdConnectionRef connection = receivedMsg.getConnection();
	bdUInt32 i = 0;
	for(i= 0; canJoin && (i < m_playerListeners.getSize()); i++)
	{
		canJoin &= m_playerListeners[i]->onServerPlayerJoinRequest(this, connection);
	}

	replyBuffer->writeRangedUInt32(BD_JOIN_REPLY, 0, BD_MAX_PLAYER_OPERATION-1);
	replyBuffer->writeBool(canJoin);
	replyBuffer->writeRangedUInt32(requestId, 0, BD_MAX_CLIENT_PLAYERS-1);
	
	bdPlayerRef player;

	if(canJoin)
	{
		// create a player
		player = bdPlayerFactory::getInstance()->create();
		bdServerPlayerData *const serverData = player->createServerData();
		serverData->setConnection(connection);
		if (!player->deserialize(requestBuffer))
		{
			BD_WARN(BD_LOG_LEVEL,"Deserialization of player failed.");
		}

		// TODO: this should use a proper id manager
		const bdPlayerId id = ++m_maxPlayerId;
		player->setId(id);

		// give the listeners a chance to edit the player setup
		for(i = 0; i < m_playerListeners.getSize(); i++)
		{
			m_playerListeners[i]->onServerSetupPlayer(this, player);
		}

		// maintain the hosts set and inform other players of the join
		bdBool isNewHost = true;
		bdMessageRef joinedMessage = createJoinMessage(player);

		// add the player id to the reply
		replyBuffer->BD_PLAYER_ID_WRITE(id);	
		player->serialize(replyBuffer);	

		for(i = 0; i < m_connections.getSize(); i++)
		{
			if (m_connections[i] == connection)
			{
				isNewHost = false;
			}
			m_connections[i]->send(joinedMessage, true);
		}
		
		// Todo: maybe we should add the connection in the onConnect() callback?
		if(isNewHost)
		{
			m_connections.pushBack(connection);
		}

	}
	
	// Send the reply to the source host of the request
	connection->send(replyMsg, true);	

	if (canJoin)
	{
		// inform the listeners about the join
		for(i = 0; i < m_playerListeners.getSize(); i++)
		{
			m_playerListeners[i]->onServerPlayerJoin(this, player);
		}

		// inform the player of all existing players
		for(i = 0; i < m_players.getSize(); i++)
		{
			bdMessageRef playerInfoMsg = new bdMessage(BD_SERVER_PLAYER_MESSAGE);
			bdBitBufferRef buffer = playerInfoMsg->getPayload();
			buffer->writeRangedUInt32(BD_JOIN, 0, BD_MAX_PLAYER_OPERATION-1);

			// write the player id and serialize
			buffer->BD_PLAYER_ID_WRITE(m_players[i]->getId());
			m_players[i]->serialize(buffer);
			
			connection->send(playerInfoMsg, true);
		}
		
		// finally add to the player list
		m_players.pushBack(player);
	}
}

void bdServer::handlePlayerLeave(bdMessageRef leaveMsg)
{	
	bdBitBufferRef buffer = leaveMsg->getPayload();

	// read the id of the player that's leaving
	bdPlayerId playerId = 0;
	const bdBool ok = buffer->BD_PLAYER_ID_READ(playerId);

	if(ok)
	{
		bdPlayerRef player = getPlayer(playerId);
		if(player)
		{
			removePlayer(player);
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL,"received player leave message for non-existent player.");
		}		
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"invalid message.");
	}
}

void bdServer::handleUpdateAck(const bdReceivedMessage& receivedMsg)
{
	bdReplicaId id;
	bdMessageRef ackMsg = receivedMsg.getMessage();
	bdBitBufferRef buffer = ackMsg->getPayload();
	while (buffer->BD_REPLICA_ID_READ(id))
	{
		bdUpdateSeqNum lastAck;
		// read the update sequence numbers frome the ack
		if(!buffer->BD_UPDATE_SEQ_NUM_READ(lastAck))
		{
			BD_WARN(BD_LOG_LEVEL,"cannot read ack value from update ack message");
			return;
		}

		bdReplicaRef replica = m_replicaTable.get(id);
		
		if(replica)
		{
			bdConnectionRef connection = receivedMsg.getConnection();
//			bdPrintf("-> Ack(%u)\n", lastAck);
			bdServerReplicaData *const serverData = replica->getServerData();
			serverData->updateAckMap(connection, lastAck);
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL,"received update ack for replica that does not exist on the server.");
		}
	}
}

void bdServer::handleUserCommandMessage(bdPlayerRef player,
										bdMessageRef commandMsg)
{
	bdReplicaRef controlledReplica = player->getControlledReplica();
	if(controlledReplica)
	{
		bdBitBufferRef buffer = commandMsg->getPayload();
		bdCommandSeqNum commandSeqNum = 0;
		bdBool ok = buffer->BD_COMMAND_SEQ_NUM_READ(commandSeqNum);
		if(!ok)
		{
			BD_WARN(BD_LOG_LEVEL,"malformed command.");
			return;
		}

		bdServerPlayerData *const playerData = player->getServerData();
		const bdCommandSeqNum expectedCommandSeqNum = playerData->getCommandSeqNum()+1;

		bdCommandSeqNum droppedPackets = commandSeqNum - expectedCommandSeqNum;

		playerData->setCommandSeqNum(commandSeqNum);

		bdUserCommand** commands = playerData->getCommands();

		// if more than BD_COMMAND_REDUNDANCY packets were dropped
		// use the last packet we know about to predict up to the
		// commands we have received.
		if(droppedPackets > BD_COMMAND_REDUNDANCY)
		{
			BD_WARN(BD_LOG_LEVEL,"more than BD_COMMAND_REDUNDANCY packets lost");
		}
		while(droppedPackets > BD_COMMAND_REDUNDANCY)
		{			
			controlledReplica->applyCommand(commands[0], false);
			controlledReplica->extrapolate(commands[0]->getTimeInSeconds(), false);		
			droppedPackets--;
		}
		
		// deserialize the commands
		
		// currentCommand is initially empty and will be filled incrementally with the 
		// delta compressed commands from the message
		bdUserCommand *const currentCommand = bdCommandFactory::getInstance()->create();
		currentCommand->clear();

		// emptyCommand is always empty and is used to force a full serialization of
		// the received command.
		bdUserCommand *const emptyCommand = bdCommandFactory::getInstance()->create();
		emptyCommand->clear();

		bdCommandSeqNum i;
		for(i = 0; ok && (i <= BD_COMMAND_REDUNDANCY); i++)
		{			
			ok = ok && currentCommand->deserialize(buffer);
		
			// force a full serialization of the command
			bdBitBufferRef cmdBuf = new bdBitBuffer;
			currentCommand->serialize(cmdBuf, emptyCommand);

			const bdUInt32 index = BD_COMMAND_REDUNDANCY - i;
			
			// store it in the command buffer
			commands[index]->clear();
			ok = ok && commands[index]->deserialize(cmdBuf);

		}

		// apply any commands from the packet that we haven't seen before.
		for(i = 0; ok && (i <= BD_COMMAND_REDUNDANCY); i++)
		{
			const bdUInt32 cmdIndex = BD_COMMAND_REDUNDANCY - i;
			if (droppedPackets >= cmdIndex)
			{
				controlledReplica->applyCommand(commands[cmdIndex], true);
				controlledReplica->extrapolate(commands[cmdIndex]->getTimeInSeconds(),false);
			}
		}

		if(!ok)
		{
			BD_ERR(BD_LOG_LEVEL, "command deserialize failed.");
		}
	

		delete currentCommand;
		delete emptyCommand;
	}
}

void bdServer::updateReplicas(const bdFloat32 time,
							  const bdBool sendUpdates)
{
	// inform any interest implementation that we are beginning an send cycle.
	if(sendUpdates)
	{
		m_interest->beginUpdate();
	}
	
	bdReplicaTable::Iterator replicaIterator = m_replicaTable.getIterator();
	while(replicaIterator)
	{
		// extrapolate any server controlled replicas
		bdReplicaRef replica = m_replicaTable.next(replicaIterator);
		if(!m_controlMap.containsKey(replica))
		{
			replica->extrapolate(time, false);
		}
		
		const bdUInt32 dirtyFlags = replica->getDirtyFlags();
		if(sendUpdates && dirtyFlags)
		{
			bdServerReplicaData *const serverData = replica->getServerData();
			serverData->addUpdate(dirtyFlags);
			replica->clearDirtyFlags();
		}
	}

	// create an update message for each host
	const bdUInt32 numHosts = m_connections.getSize();
	const bdUInt32 numPlayers = m_players.getSize();
	for(bdUInt32 i = 0; (i < numHosts) && sendUpdates; i++)
	{
		bdSet<bdReplicaRef> interestSet;
		//const bdAddr& host = m_playerHosts[i];

		// Congestion control: check if we have exceeded the bandwidth limit to the
		// host that we are going to update
		bdConnectionRef connection = m_connections[i];
		if(!connection->getAddress()->isLoopback())
		{
			const bdUInt32 transmissionRate = connection->getTransmissionRate();
			const bdUnicastConnectionRef remoteConnection = reference_cast<bdUnicastConnection>(connection);
			const bdUInt32 bytesPerSecond = remoteConnection->getStats().getBytesSentPerSecond();
			if ((transmissionRate > 0) && (bytesPerSecond > transmissionRate))
			{
				break;
			}
		}

		// create an interest set for this host
		for(bdUInt32 j = 0; j < numPlayers; j++)
		{
			bdPlayerRef player = m_players[j];
			// Todo: should make this O(1)
			if(player->getServerData()->getConnection() == connection)
			{
				m_interest->getInterestSet(player, interestSet);
			}
		}

		// create an update message for this host
		if(interestSet.getSize() > 0)
		{
			bdMessageRef updateMsg = new bdMessage(BD_SERVER_UPDATE_MESSAGE);
			bdBitBufferRef buffer = updateMsg->getPayload();
			bdSet<bdReplicaRef>::Iterator iter = interestSet.getIterator();
			bdBool sendUpdateMsg = false;
			while(iter)
			{	
				bdReplicaRef replica = interestSet.next(iter);

				// create an update for this replica				
				const bdBool haveUpdate = replica->createUpdate(connection, buffer);

				// if the replica is controlled by this host append the
				// last seen command number to the message
				bdPlayerRef player;
				if(haveUpdate) 
				{
					bdBool writeCmdNum = false;
					if (m_controlMap.get(replica, player))
					{
						const bdServerPlayerData *const playerData = player->getServerData();
						if(playerData->getConnection() == connection)
						{							
							writeCmdNum = true;
							const bdCommandSeqNum lastSeqNum = playerData->getCommandSeqNum();
							buffer->writeBool(true);
							buffer->BD_COMMAND_SEQ_NUM_WRITE(lastSeqNum);
						}
					}
					if (!writeCmdNum)
					{
						buffer->writeBool(false);
					}
				}

				sendUpdateMsg = sendUpdateMsg || haveUpdate;
			}

			if(sendUpdateMsg)
			{
				connection->send(updateMsg);
			}
		}
	}
}

void bdServer::removePlayer(bdPlayerRef player)
{
	bdUInt32 index = 0;
	
	if (m_players.findFirst(player, index))
	{
		// remove from the player array
		m_players.removeAt(index);

		// remove from the control map
		bdReplicaRef controlledReplica = player->getControlledReplica();
		if(controlledReplica)
		{
			if(!m_controlMap.remove(controlledReplica))
			{
				BD_ERR(BD_LOG_LEVEL, "player was controlling a replica but was not in the control map.");
			}
		}		
		
		bdBool removeHost = true;		
		bdUInt32 i;

		// see if there are any other players on the same host
		bdConnectionRef connection = player->getServerData()->getConnection();
		const bdUInt32 numPlayers = m_players.getSize();
		for(i = 0; i < numPlayers; i++)
		{
			if(m_players[i]->getServerData()->getConnection() == connection)
			{
				removeHost = false;
				break;
			}
		}

		if (removeHost)
		{
			m_connections.removeAll(connection);
		 	m_replicaManager.removeConnection(connection);
		}

		// inform the listeners about the leave
		const bdUInt32 numListeners = m_playerListeners.getSize();
		for(i = 0; i < numListeners; i++)
		{
			m_playerListeners[i]->onServerPlayerLeave(this, player);
		}

		// inform other hosts about the leave
		bdMessageRef leftMsg = createLeaveMessage(player);
		const bdUInt32 numHosts = m_connections.getSize();
		for(i = 0; i < numHosts; i++)
		{
			if(m_connections[i] != connection)
			{
				m_connections[i]->send(leftMsg, true);
			}
		}
	}
}

bdMessageRef bdServer::createJoinMessage(bdPlayerRef player) const
{
	bdMessageRef joinedMsg = new bdMessage(BD_SERVER_PLAYER_MESSAGE);

	bdBitBufferRef buffer = joinedMsg->getPayload();
	buffer->writeRangedUInt32(BD_JOIN, 0, BD_MAX_PLAYER_OPERATION-1);
	buffer->BD_PLAYER_ID_WRITE(player->getId());
	player->serialize(buffer);

	return joinedMsg;
}

bdMessageRef bdServer::createLeaveMessage(bdPlayerRef player) const
{
	bdMessageRef leftMsg = new bdMessage(BD_SERVER_PLAYER_MESSAGE);

	bdBitBufferRef buffer = leftMsg->getPayload();
	buffer->writeRangedUInt32(BD_LEAVE, 0, BD_MAX_PLAYER_OPERATION-1);
	buffer->BD_PLAYER_ID_WRITE(player->getId());
	
	return leftMsg;
}

bdReplicaTable* bdServer::getReplicaTable()
{
	return &m_replicaTable;
}


