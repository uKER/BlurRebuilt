// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdClient.h>

#include <bdConnection/bdConnectionStore.h>
#include <bdConnection/bdConnection.h>
#include <bdConnection/bdMessage/bdReceivedMessage.h>

#include <bdNet/bdNet.h>
#include <bdNet/bdLANDiscovery/bdGameInfo.h>

#include <bdClientServer/bdReplica/bdClientReplicaData.h>
#include <bdClientServer/bdReplica/bdUpdate/bdUpdateListener.h>
#include <bdClientServer/bdPlayer/bdPlayerOperations.h>
#include <bdClientServer/bdPlayer/bdPlayerFactory.h>
#include <bdClientServer/bdPlayer/bdClientPlayerListener.h>

#define BD_LOG_LEVEL "bdClientServer/client"

bdClient::bdClient()
: m_pendingId(0),
  m_commandInterval(0),
  m_commandTimer(0),
  m_extraStepTime(0),
  m_commandGenerator(BD_NULL)
{
}

bdClient::~bdClient()
{
	if (m_connection)
	{
		m_connection->disconnect();
	}
	bdNet::getInstance()->getConnectionStore()->unregisterDispatchInterceptor(this);
	m_commandGenerator = BD_NULL;
}


void bdClient::connect(bdGameInfoRef game)
{
	bdNetImpl *net = bdNet::getInstance();
	bdSecurityKeyMap *keyMap = net->getSocketRouter()->getKeyMap();
	bdConnectionStore *store = net->getConnectionStore();

	keyMap->registerKey(game->getSecurityID(), game->getSecurityKey());

	m_connection = store->create(game->getHostAddr(), game->getSecurityID());
	m_connection->connect();

	net->getConnectionStore()->registerDispatchInterceptor(this);
	m_replicaManager.init(&m_replicaTable);

	// fixme: remove.  
	/// \todo
	/// The following code blocks until the host
	/// connects. This is to prevent data being
	/// send to a not connected connection.
	/// The logic should be changed so that when a onConnect 
	/// call is called any data is then sent.
	while(m_connection->getStatus() != bdConnection::BD_CONNECTED)
	{
		net->sendAll();
		net->receiveAll();
		net->dispatchAll();
		net->pump();
	}
}

void bdClient::join(const bdPlayerRef player)
{
	// find the first free slot in the pending joins array

	// TODO: don't send the join request if BD_MAX_CLIENT_PLAYERS have already joined.

	bdMessageRef joinMsg = new bdMessage(BD_CLIENT_PLAYER_MESSAGE);
	bdBitBufferRef buffer = joinMsg->getPayload();
	buffer->writeRangedUInt32(BD_JOIN, 0, BD_MAX_PLAYER_OPERATION - 1);
	buffer->writeRangedUInt32(m_pendingId, 0, BD_MAX_CLIENT_PLAYERS-1);
	player->setId(m_pendingId);
	m_pendingId++;
	m_pendingJoins.addHead(player);

	player->serialize(buffer);

	m_connection->send(joinMsg, true);

}

void bdClient::leave(const bdPlayerRef player)
{
	bdMessageRef leaveMsg = new bdMessage(BD_CLIENT_PLAYER_MESSAGE);
	
	bdBitBufferRef buffer = leaveMsg->getPayload();
	buffer->writeRangedUInt32(BD_LEAVE, 0, BD_MAX_PLAYER_OPERATION - 1);

	// write the id of the player that's leaving
	buffer->BD_PLAYER_ID_WRITE(player->getId());

	m_connection->send(leaveMsg, true);

	// purge any references to the player
	for(bdUInt32 i=0; i < BD_MAX_CLIENT_PLAYERS; i++)
	{
		if(player == m_players[i])
		{
			m_players[i] = BD_NULL;
			break;
		}
	}
}

void bdClient::registerPlayerListener(bdClientPlayerListener *const listener)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_playerListeners.findFirst(listener, index))
	{
		BD_WARN(BD_LOG_LEVEL,"Same listener registered multiple times.");
	}
#endif // BD_DEBUG

	m_playerListeners.pushBack(listener);
}

void bdClient::unregisterPlayerListener(bdClientPlayerListener *const listener)
{
	m_playerListeners.removeAllKeepOrder(listener);
}

void bdClient::registerUpdateListener(bdUpdateListener *const listener)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_updateListeners.findFirst(listener, index))
	{
		BD_WARN(BD_LOG_LEVEL,"same listener registered multiple times.");
	}
#endif // BD_DEBUG

	m_updateListeners.pushBack(listener);
}

void bdClient::unregisterUpdateListener(bdUpdateListener *const listener)
{
	m_updateListeners.removeAllKeepOrder(listener);
}

void bdClient::setCommandGenerator(bdCommandGenerator *const commandGenerator)
{
	if(m_commandGenerator && commandGenerator)
	{
		BD_WARN(BD_LOG_LEVEL,"command generator already set. Old generator will be replaced.");
	}

	m_commandGenerator = commandGenerator;
}

void bdClient::setCommandInterval(const bdFloat32 interval)
{
	m_commandInterval = interval;
}

bdFloat32 bdClient::getCommandInterval() const
{
	return m_commandInterval;
}

void bdClient::step(const bdFloat32 elapsedTime)
{
	// record the step time
	m_commandTimer += elapsedTime;

	// if command interval has passed generate commands
	if(m_commandTimer >= m_commandInterval)
	{
		const bdFloat32 commandTime = m_commandTimer - m_extraStepTime;
		m_commandTimer -= m_commandInterval;
		m_extraStepTime = m_commandTimer;

		if (!m_commandGenerator)
		{
			BD_ERR(BD_LOG_LEVEL, "No bdCommandGenerator registered.");
		}

		// generate a command for each local player
		for(bdUInt32 i = 0; (i < BD_MAX_CLIENT_PLAYERS) && m_commandGenerator; i++)
		{
			bdPlayerRef player = m_players[i];
			
			// only generate a command if the plater if controlling a replica
			if(player && player->getControlledReplica())
			{
				bdCommandWindow *const commandWindow = player->getClientCommandWindow();
				bdUserCommand *command = commandWindow->getSpareCommand();
				command->setTimeInSeconds(commandTime);

				if(m_commandGenerator->generateCommand(player, command))
				{
					// send the command
					commandWindow->addSpareCommand();	
					bdMessageRef cmdMsg = new bdMessage(BD_USER_COMMAND_MESSAGE);
					bdBitBufferRef buffer = cmdMsg->getPayload();
					buffer->BD_PLAYER_ID_WRITE(player->getId());
					commandWindow->writeCommands(buffer);
					m_connection->send(cmdMsg);
					
					// do any client side prediction
					if(player->testSettings(bdPlayer::BD_DO_CLIENT_SIDE_PREDICTION))
					{
						bdReplicaRef replica = player->getControlledReplica();
						replica->processCommand(command, true);
						replica->step(command->getTimeInSeconds(), true);
					}

				}			
			}
		}
	}
		
	// step the non locally controlled replicas
	bdReplicaTable::Iterator iterator = m_replicaTable.getIterator();
	while(iterator)
	{
		bdReplicaRef replica = m_replicaTable.next(iterator);

		bdBool controlled = false;

		for(bdUInt32 i = 0; i < BD_MAX_CLIENT_PLAYERS; i++)
		{
			bdPlayerRef player = m_players[i];
			if(player && (player->getControlledReplica() == replica))
			{
				controlled = true;
				break;
			}			
		}
		
		if(!controlled)
		{
			replica->step(elapsedTime, true);
		}
	}
}

bdPlayerRef bdClient::getPlayer(const bdPlayerId playerId) const
{
	for(bdUInt32 i = 0; i < BD_MAX_CLIENT_PLAYERS; i++)
	{
		if(m_players[i].notNull() && (m_players[i]->getId() == playerId))
		{
			return m_players[i];
		}
	}

	return BD_NULL;
}


bdClientReplicaManager* bdClient::getReplicaManager() 
{
	return &m_replicaManager;
}


bdBool bdClient::accept(const bdReceivedMessage& receivedMsg) 
{
	bdMessageRef msg = receivedMsg.getMessage();
	const bdMessageType type = msg->getType();

	switch (type)
	{
		case BD_SERVER_REPLICA_MANAGER_MESSAGE:
		{
			m_replicaManager.handle(msg);
			return true;
		}
		case BD_SERVER_UPDATE_MESSAGE:
		{
			handleUpdate(msg);
			return true;
		}
		case BD_SERVER_PLAYER_MESSAGE:
		{
			handlePlayerMessage(msg);
			return true;
		}
		default:
		{
			return false;
		}
	}
}

bdConnectionRef bdClient::getConnection() const
{
	return m_connection;
}

void bdClient::handleJoinReply(bdMessageRef replyMsg)
{
	bdBitBufferRef buffer = replyMsg->getPayload();

	bdBool success = true;
	bdBool ok = buffer->readBool(success);

	bdUInt32 pendingId = 0;
	ok = ok && buffer->readRangedUInt32(pendingId, 0, BD_MAX_CLIENT_PLAYERS-1);

	bdPlayerId playerId = 0;
	if(success)
	{
		ok = ok && buffer->BD_PLAYER_ID_READ(playerId);
	}

	if(!ok)
	{
		BD_WARN(BD_LOG_LEVEL,"malformed reply message.");
		return;
	}
	
	bdLinkedList<bdPlayerRef>::Position pos = m_pendingJoins.getHeadPosition();
	while(pos)
	{
		bdPlayerRef player = m_pendingJoins.getAt(pos);
		if (player->getId() == pendingId)
		{
			m_pendingJoins.removeAt(pos);

			if(success)
			{
				// add to the player list
				bdUInt32 i = 0;
				while(i < BD_MAX_CLIENT_PLAYERS)
				{
					if(!m_players[i])
					{
						m_players[i] = player;
						break;
					}
					i++;
				}

				if(i == BD_MAX_CLIENT_PLAYERS)
				{
					BD_WARN(BD_LOG_LEVEL,"BD_MAX_CLIENT_PLAYERS already present, join request impossible.");
					return;
				}

				// set the id the server gave to the player
				player->setId(playerId);

				// read the state the server gave to the player
				if (!player->deserialize(buffer))
				{
					BD_WARN(BD_LOG_LEVEL,"Deserialization of player failed.");
					return;
				}
			}

			// As this is a local player we will need a command window.
			player->createClientCommandWindow();

			// inform any listeners of the reply
			for(bdUInt32 i = 0; i < m_playerListeners.getSize(); i++)
			{
				m_playerListeners[i]->onClientJoinReply(player, success);
			}
			return;
		}

		// Go to next item in list
		m_pendingJoins.forward(pos);

	}	

	// we didn't find the pending join request
	BD_WARN(BD_LOG_LEVEL,"received unexpected BD_JOIN_REPLY.");	
}

void bdClient::handleUpdate(bdMessageRef updateMsg)
{
	bdBitBufferRef buffer = updateMsg->getPayload();

	bdReplicaId id;
	bdMessageRef ackMsg;
	bdBitBufferRef ackBuffer; 

	while (buffer->BD_REPLICA_ID_READ(id))
	{
		bdReplicaRef replica = m_replicaTable.get(id);
		if(replica)
		{
			
			bdUpdateSeqNum updateSeqNum;
			if(replica->receiveUpdate(buffer, updateSeqNum))
			{
				if(!ackMsg)
				{
					ackMsg = new bdMessage(BD_CLIENT_UPDATE_ACK_MESSAGE);
					ackBuffer = ackMsg->getPayload();
				}

				ackBuffer->BD_REPLICA_ID_WRITE(id);

				// write the update sequence number in to the ack
				ackBuffer->BD_UPDATE_SEQ_NUM_WRITE(updateSeqNum);
	
				replay(replica, buffer);

				// Notify any listeners about the update
				for(bdUInt32 j = 0; j < m_updateListeners.getSize(); j++)
				{
					m_updateListeners[j]->onUpdate(replica);
				}
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "malformed update, skipping any remaining updates.");
				break;
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL,"received update for unknown replica id. Skipping any remaining updates");
			break;
		}
	}

	if(ackMsg)
	{
		m_connection->send(ackMsg);
	}
}

void bdClient::replay(bdReplicaRef replica,
					  bdBitBufferRef buffer)
{
	// if the replica is being controlled by one of our local players
	// and client side prediction is on then replay an commands that
	// the server hasn't seen yet
	if (buffer->testBool())
	{
		// read the last command seen by the server
		bdCommandSeqNum lastCommand = 0;
		const bdBool ok = buffer->BD_COMMAND_SEQ_NUM_READ(lastCommand);

		for(bdUInt32 i = 0; i < BD_MAX_CLIENT_PLAYERS; i++)
		{
			bdPlayerRef player = m_players[i];
			if(player && player->testSettings(bdPlayer::BD_DO_CLIENT_SIDE_PREDICTION) && player->getControlledReplica() == replica)
			{
				if(ok)
				{
					player->replay(lastCommand);
				}
				break;
			}
		}
	}
}

void bdClient::handlePlayerMessage(bdMessageRef playerMsg)
{
	bdBitBufferRef buffer = playerMsg->getPayload();
	bdUInt32 iType;
	bdBool ok = buffer->readRangedUInt32(iType, 0, (BD_MAX_PLAYER_OPERATION-1));

	if(!ok)
	{
		BD_WARN(BD_LOG_LEVEL,"no type");
		return;
	}

	const bdPlayerOperation operation = static_cast<bdPlayerOperation>(iType);

	switch(operation)
	{
		case BD_JOIN:
		{
			// the server has told us about a new player that joined

			// read the players id
			bdPlayerId playerId = 0;
			ok = buffer->BD_PLAYER_ID_READ(playerId);

			if(ok)
			{
				// create an instance of our player class
				bdPlayerRef player = bdPlayerFactory::getInstance()->create();
				player->setId(playerId);
				if (!player->deserialize(buffer))
				{
					BD_WARN(BD_LOG_LEVEL,"Deserialization of player failed.");
				}

				// note that we DON'T create a commandWindow for the player
				// this is a remote player and so will never send commands
				// from this client.

				// inform the listeners of the join
				for(bdUInt32 i = 0; i < m_playerListeners.getSize(); i++)
				{
					m_playerListeners[i]->onClientPlayerJoin(player);
				}
			}
		}
		break;
		case BD_LEAVE:
		{
			bdPlayerId playerId;
			ok = buffer->BD_PLAYER_ID_READ(playerId);

			if(ok)
			{
				for(bdUInt32 j = 0; j < m_playerListeners.getSize(); j++)
				{
					m_playerListeners[j]->onClientPlayerLeave(playerId);
				}
			}
		}
		break;
		case BD_CONTROL_REPLICA:
		{	
			// read the player id
			bdPlayerId playerId = 0;
			ok =  buffer->BD_PLAYER_ID_READ(playerId);

			// read the replica id
			bdReplicaId replicaId = 0;
			ok = ok && buffer->BD_REPLICA_ID_READ(replicaId);

			if(ok)
			{
				// lookup the player and replica
				bdPlayerRef player = getPlayer(playerId);
				bdReplicaRef replica = m_replicaTable.get(replicaId);
				
				if(player && replica)
				{
					bdReplicaRef currentReplica = player->getControlledReplica();
					player->setControlledReplica(replica);
					// inform the listeners of the event
					for(bdUInt32 i = 0; i < m_playerListeners.getSize(); i++)
					{
						m_playerListeners[i]->onClientSetControl(player, currentReplica);
					}
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL,"cannot find player or replica to control.");
				}
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL,"message reading failed, invalid message.");
			}
		}
		break;
		case BD_JOIN_REPLY:
		{					
			handleJoinReply(playerMsg);
		}
		break;
		default:
		{
			BD_WARN(BD_LOG_LEVEL,"cannot handle this bdPlayerOperation on a client.");
		}
	}
}

