// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdPlayer/bdPlayerManager.h>

#include <bdCore/bdUtilities/bdSort.h>

#include <bdConnection/bdMessage/bdMessage.h>

#include <bdNet/bdNet.h>

#include <bdPeer/bdDelivery/bdDeliveryManager.h>
#include <bdPeer/bdDelivery/bdDeliveryMessage.h>
#include <bdPeer/bdPlayer/bdPlayerManagerListener.h>
#include <bdPeer/bdPlayer/bdPlayerFactory.h>

#define BD_LOG_LEVEL "bdPlayerManager"

#define BD_FIRE_LISTENER(l)								\
do														\
{														\
	for(bdUInt listenerIndex = 0; listenerIndex < m_listeners.getSize(); listenerIndex++)	\
	{													\
		m_listeners[listenerIndex]->l;								\
	}													\
}														\
while(g_assertFalse)

bdPlayerManager::bdPlayerManager()
:  m_deliveryManager(BD_NULL)
{
}

bdPlayerManager::~bdPlayerManager()
{
	quit();
}

void bdPlayerManager::init(bdDeliveryManager *const deliveryManager)
{	
	BD_ASSERT(deliveryManager != BD_NULL, "bdPlayerManager::init, "
		"deliveryManager cannot be null");

	m_deliveryManager = deliveryManager;

	m_deliveryManager->registerListener(this);
	m_deliveryManager->registerInterceptor(this);
	m_deliveryManager->registerInitializer(this);
	m_deliveryManager->getSession()->registerInterceptor(this);
}

void bdPlayerManager::quit()
{
	m_players.clear();
//	m_listeners.clear();

	if(m_deliveryManager)
	{
		m_deliveryManager->unregisterListener(this);
		m_deliveryManager->unregisterInterceptor(this);
		m_deliveryManager->unregisterInitializer(this);
		m_deliveryManager->getSession()->unregisterInterceptor(this);
		m_deliveryManager = BD_NULL;
	}	
}

bdBool bdPlayerManager::add(bdPlayer& player)
{
	// must be active in order to add a player
	bdBool active = m_deliveryManager->isActive();
	if(!active)
	{
		active = m_deliveryManager->activate();
	}

	if(active)
	{
		// create a player join message
		bdMessage joinMessage(BD_PLAYER_MGR_JOIN_MSG);
		const bdBitBufferRef buffer = joinMessage.getPayload();
		player.serialize(buffer);
		m_deliveryManager->addMessage(joinMessage);
	}

	return active;
}

bdBool bdPlayerManager::remove(const bdPlayerRef player) const
{	
	// must be active in order to remove a player
	bdBool active = m_deliveryManager->isActive();
	if(!active)
	{
		active = m_deliveryManager->activate();
	}

	if(active)
	{
		// create a player leave message
		bdMessage leaveMessage(BD_PLAYER_MGR_LEAVE_MSG);
		const bdBitBufferRef buffer = leaveMessage.getPayload();
		const bdPlayerId playerId = player->getId();
		buffer->BD_PLAYER_ID_WRITE(playerId);
		m_deliveryManager->addMessage(leaveMessage);
	}

	return active;
}

bdUInt bdPlayerManager::getNumPlayers() const
{
	return m_players.getSize();
}

bdPlayerRef bdPlayerManager::getPlayer(const bdUInt i) const
{
	const bdPlayerRef player = m_players[i];
	return player;
}

void bdPlayerManager::registerListener(bdPlayerManagerListener *const listener)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_listeners.findFirst(listener, index))
	{
		BD_WARN(BD_LOG_LEVEL,"Same listener registered multiple times.");
	}
#endif // BD_DEBUG

	m_listeners.pushBack(listener);
}
		
void bdPlayerManager::unregisterListener(bdPlayerManagerListener *const listener)
{
	m_listeners.removeAllKeepOrder(listener);
}

void bdPlayerManager::serialize(bdBitBufferRef buffer)
{
	const bdUInt numPlayers = m_players.getSize();
	const bdUByte8 numPlayers8 = static_cast<bdUByte8>(numPlayers);

	BD_ASSERT(numPlayers == numPlayers8, "bdPlayerManager::serialize, too many players.");

	// write the number of players
	// todo: this should be a ranged write
	buffer->writeUByte8(numPlayers8);

	for(bdUInt i = 0; i < numPlayers; i++)
	{
		const bdPlayerRef player = m_players[i];
		buffer->BD_PLAYER_ID_WRITE(player->getId());
		player->getAddr()->serialize(buffer);
		player->serialize(buffer);
	}
}

bdBool bdPlayerManager::deserialize(bdBitBufferRef buffer)
{
	m_players.clear();

	// read the number of players
	// todo: this should be a ranged read
	bdUByte8 numPlayers;
	bdBool ok = buffer->readUByte8(numPlayers);

	bdCommonAddrRef localAddr = bdNet::getInstance()->getLocalCommonAddr();

	for(bdUInt i = 0; ok && (i < numPlayers); i++)
	{
		const bdPlayerRef newPlayer = bdPlayerFactory::getInstance()->create();
		const bdCommonAddrRef addr = new bdCommonAddr;

		bdPlayerId id;
		ok = buffer->BD_PLAYER_ID_READ(id);
		ok = ok && addr->deserialize(localAddr, buffer);
		ok = ok && newPlayer->deserialize(buffer);

		if(ok)
		{
			newPlayer->setId(id);
			newPlayer->setAddr(addr);
			m_players.pushBack(newPlayer);			
		}
	}

	if(!ok)
	{
		BD_WARN(BD_LOG_LEVEL,"Deserialization failed.");
	}

	return ok;
}

void bdPlayerManager::onDeliveryDeactivate(const bdCommonAddrRef addr)
{
	BD_INFO(BD_LOG_LEVEL, "Handling delivery deactivate callback");
	// a peer has deactivated so we must remove all it's players
	for(bdUInt i = 0; i < m_players.getSize(); i++)
	{
		const bdPlayerRef player = m_players[i];
		if(player->getAddr()->getHash() == addr->getHash())
		{
			BD_FIRE_LISTENER(onPlayerLeave(player));
			m_players.removeSection(i, i+1);
			// need to recheck the current index
			i--;
		}
	}
}

bdBool bdPlayerManager::acceptDelivery(const bdDeliveryMessage& receivedMsg)
{
	const bdMessageRef message = receivedMsg.getMessage();
	const bdMessageType type = message->getType();
	bdBool intercepted = true;

	switch(type)
	{
		case BD_PLAYER_MGR_JOIN_MSG:
		{
			handleAdd(receivedMsg);
		}
		break;
		case BD_PLAYER_MGR_LEAVE_MSG:
		{
			handleRemove(receivedMsg);
		}
		break;
		default:
		{
			intercepted = false;
		}
	}

	return intercepted;
}

void bdPlayerManager::handleAdd(const bdDeliveryMessage& addMsg)
{
	const bdPlayerRef player = bdPlayerFactory::getInstance()->create();

	const bdBitBufferRef buffer = addMsg.getMessage()->getPayload();
	const bdBool ok = player->deserialize(buffer);
	if(ok)
	{
		bdPlayerId maxPlayerId = 0;
		const bdUInt numPlayers = m_players.getSize();
		for(bdUInt i = 0; i < numPlayers; i++)
		{
			const bdPlayerId playerId = m_players[i]->getId();
			maxPlayerId = BD_MAX(maxPlayerId, playerId);
		}
		
		player->setId(maxPlayerId+1);
		player->setAddr(addMsg.getAboutAddr());
		m_players.pushBack(player);

		bdCombSort(m_players.begin(), m_players.getSize(), bdPlayerSort());
		
		BD_INFO(BD_LOG_LEVEL, "Player with ID %u added", player->getId());
		BD_FIRE_LISTENER(onPlayerJoin(player));

	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Malformed player add message.");
	}
}

void bdPlayerManager::handleRemove(const bdDeliveryMessage& removeMsg)
{
	const bdBitBufferRef buffer = removeMsg.getMessage()->getPayload();
	bdPlayerId leaveId;
	const bdBool ok = buffer->BD_PLAYER_ID_READ(leaveId);
	if(ok)
	{
		bdPlayerRef foundPlayer;
		bdUInt numPlayers = m_players.getSize();
		for(bdUInt i = 0; !foundPlayer && (i < numPlayers); i++)
		{
			const bdPlayerRef player = m_players[i];
			if(leaveId == player->getId())
			{
				foundPlayer = player;		
				BD_FIRE_LISTENER(onPlayerLeave(player));
				//m_players.removeAt(i);
				m_players.removeSection(i, i+1);
				BD_INFO(BD_LOG_LEVEL, "Player with ID %u removed", player->getId());

			}
		}

		if(!foundPlayer)
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to find player.");
		}

		// if this is the last local player then deactivate
		// note && m_deliveryManager in case of replay when m_deliveryManager == BD_NULL
		if(foundPlayer && foundPlayer->isLocal() && m_deliveryManager)
		{			
			bdBool morePlayers = false;
			numPlayers = m_players.getSize();
			for(bdUInt playerIndex = 0; !morePlayers && (playerIndex < numPlayers); playerIndex++)
			{
				morePlayers = m_players[playerIndex]->isLocal();
			}

			if(!morePlayers)
			{
				m_deliveryManager->deactivate();
			}
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Malformed player remove message.");
	}
}

void bdPlayerManager::sendInitData(const bdConnectionRef connection)
{
	/// TODO : This code can be improved :
	///       look into how the data could be separated so that if the size
	///       was greater than max message size...
	bdSession *const session = m_deliveryManager->getSession();
	if(session->getRole() == bdSession::BD_SESSION_HOST)
	{
		// send details about all players to the connecting peer
		const bdUInt numPlayers = m_players.getSize();

		const bdMessageRef initData = new bdMessage(BD_PLAYER_MGR_INIT_DATA);
		const bdBitBufferRef buffer = initData->getPayload();

		// TODO: use less that 8 bits (set a max players)
		bdUByte8 playerCount = 0;
		const bdUInt writePosition = buffer->getWritePosition();
		// Write a temporary player count for now, we will rewind to 
		// here after we have retrieved the data for all players
		// and write the actual value.
		// We do this because some players may no longer have a valid
		// connection in which case we will exclude them from the list.
		buffer->writeUByte8(playerCount);

		// send info about all our existing players
		for(bdUInt i = 0; i < numPlayers; i++)
		{
			const bdPlayerRef player = m_players[i];
			
			
			//must also send the connection info for each player...
			bdCommonAddrRef addr = player->getAddr();
			bdUInt32 index = 0;
			bdBool found = false;
			
			// need to find the peer index for this player.
			for(bdUInt peerIndex = 0; peerIndex < session->getNumPeers(); peerIndex++)
			{
				if(addr->getHash() == session->getConnection(peerIndex)->getAddress()->getHash())
				{
					index = peerIndex;
					found = true;
					break;
				}
			}
			
			if(found)
			{
				// TODO: use less that 8 bits (set a max peers)
				buffer->BD_PLAYER_ID_WRITE(player->getId());
				player->serialize(buffer);
				buffer->writeUInt32(index);
				playerCount++;
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Peer address not found in session.");
			}
		}
		
		// We will now rewind and write the actual number of players with 
		// valid data.
		buffer->setWritePosition(writePosition);
		buffer->writeUByte8(playerCount);

		m_deliveryManager->getSession()->send(connection, initData);
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "This ought only be called on the session host.");
	}

}

bdBool bdPlayerManager::acceptSession(const bdReceivedMessage& receivedMsg)
{
	const bdMessageRef message = receivedMsg.getMessage();
	const bdMessageType type = message->getType();
	bdBool accepted = true;

	switch(type)
	{	
		case BD_PLAYER_MGR_INIT_DATA:
		{
			handleInitData(receivedMsg);
		}
		break;
		default:
		{
			accepted = false;
		}
	}

	return accepted;
}

void bdPlayerManager::handleInitData(const bdReceivedMessage& initData)
{
	/// TODO : This code can be improved
	///  cf. sendInitData
	BD_INFO(BD_LOG_LEVEL, "Handling Init Data for bdPlayerManager");

	const bdMessageRef message = initData.getMessage();
	const bdBitBufferRef buffer = message->getPayload();

	bdArray<bdPlayerRef> players;

	bdUByte8 numPlayers;
	bdBool ok = buffer->readUByte8(numPlayers);

	for(bdUInt i = 0; ok && (i < numPlayers); i++)
	{
		const bdPlayerRef player = bdPlayerFactory::getInstance()->create();

		bdPlayerId id;
		ok = buffer->BD_PLAYER_ID_READ(id);
		ok = ok && player->deserialize(buffer);	

		bdUInt32 index = 0;
		ok = ok && buffer->readUInt32(index);
		bdCommonAddrRef addrRef;
		if(ok)
		{
			// use the peer index to find the correct common addr.
			addrRef = m_deliveryManager->getSession()->getConnection(index)->getAddress();
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to read peer index from init data.");
		}
		if(ok)
		{
			player->setId(id);
			player->setAddr(addrRef);
			players.pushBack(player);
		}
	}

	// if the parsing went ok, add the new players
	if(ok)
	{
		const bdUInt numNewPlayers = players.getSize();
		for(bdUInt i = 0; i < numNewPlayers; i++)
		{
			const bdPlayerRef newPlayer = players[i];

			// check the new player doesn't already exist
			const bdUInt numCurrPlayers = m_players.getSize();
			bdBool exists = false;
			for(bdUInt j = 0; !exists && (j < numCurrPlayers); j++)
			{
				exists = (newPlayer->getId() == m_players[j]->getId());
			}

			if(!exists)
			{
				m_players.pushBack(newPlayer);
				BD_INFO(BD_LOG_LEVEL, "Player with ID %u added", newPlayer->getId());
				BD_FIRE_LISTENER(onPlayerJoin(newPlayer));
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Player id already taken.");
			}
		}

		bdCombSort(m_players.begin(), m_players.getSize(), bdPlayerSort());
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "Malformed player info message");
	}
}
