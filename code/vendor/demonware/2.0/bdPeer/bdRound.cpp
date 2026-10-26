// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdRound.h>

#include <bdConnection/bdConnection.h>
#include <bdConnection/bdMessage/bdMessage.h>

#include <bdPeer/bdSession/bdSession.h>
#include <bdPeer/bdDelivery/bdDeliveryManager.h>
#include <bdPeer/bdDelivery/bdDeliveryMessage.h>
#include <bdPeer/bdPlayer/bdPlayerManager.h>
#include <bdPeer/bdPlayer/bdPlayer.h>
#include <bdPeer/bdCommand/bdCommand.h>
#include <bdPeer/bdCommand/bdCommandFactory.h>
#include <bdPeer/bdGame.h>

#define BD_LOG_LEVEL "bdRound"

bdRound::bdRound()
: m_deliveryManager(BD_NULL),
  m_playerManager(BD_NULL),
  m_game(BD_NULL),
#ifdef BD_DEBUG
  m_randomSeeded(false),
#endif // BD_DEBUG
  m_random(1000),
  m_status(BD_ROUND_UNINITIALIZED),
  m_hashSet(false),
  m_gameStateHash(0)
{
}

bdRound::~bdRound()
{
	quit();
}

bdBool bdRound::init(bdDeliveryManager *const deliveryManager,
				   bdPlayerManager *const playerManager,
				   bdGame *const game)
{
	bdBool result = false;
	if(BD_ROUND_UNINITIALIZED == m_status)
	{
		BD_ASSERT(deliveryManager != BD_NULL, "bdRound::init, "
			"deliveryManager cannot be null");

		BD_ASSERT(playerManager != BD_NULL, "bdRound::init, "
			"playerManager cannot be null");

		BD_ASSERT(game != BD_NULL, "bdRound::init, "
			"game cannot be null");

		m_deliveryManager = deliveryManager;
		m_playerManager = playerManager;
		m_game = game;

	#ifdef BD_DEBUG
		m_randomSeeded = false;
	#endif // BD_DEBUG

		m_random.setSeed(1000);
		m_status = BD_WAITING;

		m_deliveryManager->registerListener(this);
		m_deliveryManager->registerInterceptor(this);
		m_deliveryManager->registerInitializer(this);
		m_deliveryManager->getSession()->registerInterceptor(this);

		if (m_deliveryManager->getSession()->getRole() == bdSession::BD_SESSION_HOST)
		{
			m_status = BD_IN_LOBBY;
		}
		result = true;
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "m_status : % This should only be called when m_status is uninitialized", m_status);
	}
	return result;
}

void bdRound::quit()
{
	// Todo
	// should we have a check here so that quit is only called after init has been called ?
	if(m_deliveryManager)
	{
		m_deliveryManager->unregisterListener(this);
		m_deliveryManager->unregisterInterceptor(this);
		m_deliveryManager->unregisterInitializer(this);
		m_deliveryManager->getSession()->unregisterInterceptor(this);
		m_deliveryManager = BD_NULL;
	}

	m_playerManager = BD_NULL;
	m_game = BD_NULL;

#ifdef BD_DEBUG
	m_randomSeeded = false;
#endif // BD_DEBUG

	m_random.setSeed(1000);
	m_status = BD_ROUND_UNINITIALIZED;
}

bdRound::bdRoundStatus bdRound::getStatus() const
{
	return m_status;
}

bdRandom* bdRound::getRandom()
{
#ifdef BD_DEBUG
	if(!m_randomSeeded)
	{
		BD_WARN(BD_LOG_LEVEL,"Generator many not be deterministically seeded.");
	}
#endif // BD_DEBUG

	if(BD_IN_ROUND != m_status)
	{
		BD_ERR(BD_LOG_LEVEL, "The random number generator is only deterministic when a round is in progress");
	}

	return &m_random;
}

bdBool bdRound::start(const bdUInt32 seed,
					bdBitBuffer *const userData)
{
	bdBool result = false;
	if(m_status == BD_IN_LOBBY)
	{
		bdMessage startMessage(BD_ROUND_START_MSG);
		const bdBitBufferRef buffer = startMessage.getPayload();
		buffer->writeUInt32(seed);
		
		// copy the user data into the message
		bdBool appendSuccess = true;
		if(userData)
		{
			appendSuccess = buffer->append(*userData);
		}

		if(appendSuccess)
		{		
			m_deliveryManager->addMessage(startMessage);
			result = true;
		}
	}
	return result;
}

bdBool bdRound::end()
{
	bdBool result = false;
	if(m_status == BD_IN_ROUND)
	{
		bdMessage endMessage(BD_ROUND_END_MSG);
		m_deliveryManager->addMessage(endMessage);
		result = true;
	}
	return result;
}

void bdRound::addCommand(const bdPlayerRef player,
						 bdCommand& command)
{
	if(m_status != BD_IN_ROUND)
	{
		BD_WARN(BD_LOG_LEVEL,"Adding commands before round has started.");
	}

	bdMessage commandMessage(BD_COMMAND_MSG);
	const bdBitBufferRef buffer = commandMessage.getPayload();
	buffer->BD_PLAYER_ID_WRITE(player->getId());
	command.serialize(buffer);

	m_deliveryManager->addMessage(commandMessage);
}

void bdRound::serialize(bdBitBufferRef buffer)
{
	// write the status
	buffer->writeRangedUInt32(m_status, 0, BD_MAX_ROUND_STATUS-1);

	// write the random number seed
	const bdUInt seed = m_random.getSeed();
	const bdUInt32 seed32 = static_cast<bdUInt32>(seed);

	BD_ASSERT(seed == seed32, "bdRound::serialize, cannot write seed as 32 bit.");

	buffer->writeRangedUInt32(seed32, 0, BD_MAX_RANDOM);
}

bdBool bdRound::deserialize(bdBitBufferRef buffer)
{
	// read the status
	bdUInt32 status;
	bdBool ok = buffer->readRangedUInt32(status, 0, BD_MAX_ROUND_STATUS-1);

	// read the random number seed
	bdUInt32 seed = 0;
	ok = ok && buffer->readRangedUInt32(seed, 0, BD_MAX_RANDOM);

	if(ok)
	{
		m_status = static_cast<bdRoundStatus>(status);
		m_random.setSeed(seed);
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Deserialization failed.");
	}

	return ok;
}

bdBool bdRound::acceptDelivery(const bdDeliveryMessage& receivedMsg)
{
	const bdMessageRef message = receivedMsg.getMessage();
	const bdMessageType type = message->getType();
	bdBool intercepted = true;

	switch(type)
	{
		case BD_ROUND_START_MSG:
		{
			handleStart(receivedMsg);
		}
		break;
		case BD_ROUND_END_MSG:
		{
			handleEnd(receivedMsg);
		}
		break;
		case BD_COMMAND_MSG:
		{
			handleCommand(receivedMsg);
		}
		break;
		case BD_GAME_STATE_HASH_MSG:
		{
			handleHash(receivedMsg);
		}
		break;
		default:
		{
			intercepted = false;
		}
	}

	return intercepted;
}

void bdRound::onDeliveryEnd()
{
	if(m_status == BD_IN_ROUND)
	{
		m_game->tick();
	}
	m_hashSet = false;
}

void bdRound::handleStart(const bdDeliveryMessage& startMsg)
{	
	if(m_status == BD_IN_LOBBY)
	{
		const bdMessageRef message = startMsg.getMessage();
		const bdBitBufferRef buffer = message->getPayload();
		bdUInt32 seed = 0;
		const bdBool ok = buffer->readUInt32(seed);
		if(ok)
		{
			m_random.setSeed(seed);
#ifdef BD_DEBUG
			m_randomSeeded = true;
#endif // BD_DEBUG

			// WARNING: swapping these two lines is liable to break
			// peer to peer round replays.
			m_game->onRoundStart(buffer);
			m_status = BD_IN_ROUND;
		}
	}
}

void bdRound::handleEnd(const bdDeliveryMessage&)
{
	if(m_status == BD_IN_ROUND)
	{
		m_game->onRoundEnd();
	}
	m_status = BD_IN_LOBBY;
#ifdef BD_DEBUG
	m_randomSeeded = false;
#endif // BD_DEBUG

}

void bdRound::handleCommand(const bdDeliveryMessage& commandMsg)
{
	if(m_status == BD_IN_ROUND)
	{
		const bdBitBufferRef buffer = commandMsg.getMessage()->getPayload();

		bdPlayerId id;
		bdBool ok = buffer->BD_PLAYER_ID_READ(id);

		const bdCommandRef command = bdCommandFactory::getInstance()->create();
		ok = ok && command->deserialize(buffer);

		// find the player 		
		bdBool found = false;
		const bdUInt numPlayers = m_playerManager->getNumPlayers();
		for(bdUInt i=0; !found && (i < numPlayers); i++)
		{			
			const bdPlayerRef player = m_playerManager->getPlayer(i);			
			if(player->getId() == id)
			{				
				found = true;
				m_game->processCommand(player, command);
			}
		}

		if(!ok)
		{
			BD_WARN(BD_LOG_LEVEL,"Malformed command.");
		}
		
		if(!found)
		{
			BD_WARN(BD_LOG_LEVEL, "Player lookup failed.");
		}
	}
}

void bdRound::handleHash(const bdDeliveryMessage& commandMsg)
{
	if(m_status == BD_IN_ROUND)
	{
		const bdBitBufferRef buffer = commandMsg.getMessage()->getPayload();

		bdUInt32 hash32 = 0;
		bdBool ok = buffer->readUInt32(hash32);
		if(ok)
		{
			if(!m_hashSet)
			{
				m_gameStateHash = hash32;
				m_hashSet = true;
			}

			// If this assert is hit it indicates the hash being supplied to bdPeer
			// through the bdGame interface differs between peers.
			// see bdGame::getGameStateHash() for more info..
			BD_ASSERT(m_gameStateHash == hash32, "Game State Hashes Have Diverged.");

		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to read hash from game state hash message");
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Game state hash messages should only be sent while a round is in progress ");
	}
}

void bdRound::sendInitData(const bdConnectionRef connection)
{
	bdSession *const session = m_deliveryManager->getSession();
	if(session->getRole() == bdSession::BD_SESSION_HOST)
	{
		const bdMessageRef initData = new bdMessage(BD_ROUND_INIT_DATA);
		const bdBitBufferRef buffer = initData->getPayload();

		buffer->writeRangedUInt32(m_status, 0, BD_MAX_ROUND_STATUS-1);
		
		const bdUInt seed = m_random.getSeed();
		const bdUInt32 seed32 = static_cast<bdUInt32>(seed);
		buffer->writeUInt32(seed32);

		const bdBool sendSuccess = session->send(connection, initData);
		if(!sendSuccess)
		{
			BD_ERR(BD_LOG_LEVEL, "Sending of init data failed");
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "This ought only be called on the session host.");
	}
}

bdBool bdRound::acceptSession(const bdReceivedMessage& receivedMsg)
{
	const bdMessageRef message = receivedMsg.getMessage();
	const bdMessageType type = message->getType();
	bdBool accepted = true;

	switch(type)
	{	
		case BD_ROUND_INIT_DATA:
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

void bdRound::handleInitData(const bdReceivedMessage& initData)
{
	BD_INFO(BD_LOG_LEVEL, "Handling Init Data for bdRound status: %u", m_status);
	if(m_status == BD_WAITING)
	{
		const bdMessageRef message = initData.getMessage();
		const bdBitBufferRef buffer = message->getPayload();
		bdUInt32 status32 = 0;
		bdBool ok = buffer->readRangedUInt32(status32, 0, BD_MAX_ROUND_STATUS-1);
		if(ok)
		{
			const bdRoundStatus hostStatus = static_cast<bdRoundStatus>(status32);
			switch(hostStatus)
			{
				case BD_IN_LOBBY:
				{
					m_status = BD_IN_LOBBY;
				}
				break;
				case BD_IN_ROUND:
				{
					if(m_game->allowLateJoins())
					{
						m_status = BD_UPDATING;
					}
					else if(m_game->allowStateBasedMidRoundJoins())
					{
						m_status = BD_UPDATING;
					}
					else
					{
						m_status = BD_WAITING;
					}
				}
				break;
				default:
				{
					BD_ERR(BD_LOG_LEVEL, "Host is in unexpected state.");
				}
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Malformed init message.");
		}

		if(ok)
		{
			// read out the seed for the random number generator...
			bdUInt32 seed32 = 0;
			ok = buffer->readUInt32(seed32);
			if(ok)
			{
				bdUInt seed = static_cast<bdUInt>(seed32);
				m_random.setSeed(seed);
#ifdef BD_DEBUG
				m_randomSeeded = true;
#endif //#ifdef BD_DEBUG
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Failed to read random seed from message.");
			}
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Host has unexpected status.");
	}
}

void bdRound::onDeliveryInitCompleted()
{
	if(BD_UPDATING == m_status)
	{
		m_status = BD_IN_ROUND;
		m_game->onRoundJoined();
	}
	else if(BD_WAITING == m_status || BD_IN_LOBBY == m_status)
	{
		BD_INFO(BD_LOG_LEVEL, "Delivery initialization complete: waiting for next round to start.");
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "Callback received while status was %u, should be BD_UPDATING : %u", m_status, BD_UPDATING);
	}
}

void bdRound::addHash(const bdUInt32 hash) const 
{
	const bdUInt32 hash32 = static_cast<bdUInt32>(hash);
	bdMessage hashMessage(BD_GAME_STATE_HASH_MSG);
	hashMessage.getPayload()->writeUInt32(hash32);
	m_deliveryManager->addMessage(hashMessage);
}

