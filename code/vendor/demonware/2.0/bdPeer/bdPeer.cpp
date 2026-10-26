// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdPeer.h>
#include <bdPeer/bdPlayer/bdPlayer.h>
#include <bdPeer/bdGame.h>

#include <bdNet/bdLANDiscovery/bdGameInfo.h>

#define BD_LOG_LEVEL "peer"

bdPeer::bdPeer(bdGame *const game,
			   const bdUInt messageLimit,
			   bdSessionHandler *const sessionHandler)
: m_game(game),
  m_session(sessionHandler),
  m_deliveryManager(messageLimit),
  m_replaying(false),
  m_replayPlayerManager(BD_NULL),
  m_replayRound(BD_NULL),
  m_sendInterval(0),
  m_elapsedTime(0)
{
}

bdPeer::~bdPeer()
{
	m_game = BD_NULL;
	m_replayPlayerManager = BD_NULL;
	m_replayRound = BD_NULL;
}

bdSession* bdPeer::getSession()
{
	return &m_session;
}

bdDeliveryManager* bdPeer::getDeliveryManager()
{
	return &m_deliveryManager;
}

bdPlayerManager* bdPeer::getPlayerManager()
{
	bdPlayerManager *const activeManager = m_replaying?m_replayPlayerManager:&m_playerManager;
	return activeManager;
}

bdRound* bdPeer::getRound()
{
	bdRound *const activeRound = m_replaying?m_replayRound:&m_round;
	return activeRound;
}

bdGame* bdPeer::getGame()
{
	return m_game;
}

bdBool bdPeer::connect(const bdGameInfoRef gameInfo,
					   bdBitBuffer *const userData)
{
	bdBool joiningSession = m_session.join(gameInfo->getHostAddr(), gameInfo->getSecurityID(), gameInfo->getSecurityKey(), userData);
	if(joiningSession)
	{
		m_deliveryManager.init(&m_session);
		m_playerManager.init(&m_deliveryManager);

		joiningSession = m_round.init(&m_deliveryManager, &m_playerManager, m_game);
	}

	return joiningSession;
}

void bdPeer::leave()
{
	m_round.quit();
	m_session.leave();
	m_playerManager.quit();
	m_deliveryManager.quit();	
}

bdBool bdPeer::addPlayer(bdPlayer& player)
{
	// can add players if we are in the lobby or in the round
	bdBool canAdd = ((m_round.getStatus() == bdRound::BD_IN_LOBBY) ||
					 (m_round.getStatus() == bdRound::BD_IN_ROUND));
	
	canAdd = canAdd && m_playerManager.add(player);
	return canAdd;
}

bdBool bdPeer::removePlayer(const bdPlayerRef player) const
{
	// can remove players if we are in the lobby or in the round
	bdBool canRemove = ((m_round.getStatus() == bdRound::BD_IN_LOBBY) ||
						(m_round.getStatus() == bdRound::BD_IN_ROUND));
	
	canRemove = canRemove && m_playerManager.remove(player);
	return canRemove;
}

bdBool bdPeer::startRound(const bdUInt32 seed, bdBitBuffer *const userData)
{
	// can begin a round if we are active and in the lobby
	// being active implies we have added a player so peers with no players
	// cannot start the round, unless they are the host
	bdBool startSuccess =	m_deliveryManager.isActive() && 
							((m_round.getStatus() == bdRound::BD_IN_LOBBY) || 
							(m_round.getStatus() == bdRound::BD_IN_ROUND));

	startSuccess = startSuccess && m_round.start(seed, userData);

	return startSuccess;
}

bdBool bdPeer::endRound()
{
	// can end the round is we are in the round and are active
	const bdBool canEnd = m_deliveryManager.isActive() && 
						  (m_round.getStatus() == bdRound::BD_IN_ROUND);
	if(canEnd)
	{
		m_round.end();
	}

	return canEnd;
}

bdBool bdPeer::okToSendCommands() const
{
	const bdBool canSend = (m_round.getStatus() == bdRound::BD_IN_ROUND) &&
							m_deliveryManager.okToSend();
	return canSend;
}

bdBool bdPeer::addCommand(const bdPlayerRef player,
						  bdCommand& command)
{
	// can add commands if we are in a round
	// note that they may not be sent if we are not active
	const bdBool canAdd = (m_round.getStatus() == bdRound::BD_IN_ROUND);

	if(canAdd)
	{
		m_round.addCommand(player, command);
	}

	if(m_replaying)
	{
		BD_WARN(BD_LOG_LEVEL, "Adding command whilst replaying, command will not be sent.");
	}

	return canAdd;
}

void bdPeer::setSendInterval(const bdFloat32 interval)
{
	m_sendInterval = interval;
}

bdBool bdPeer::step(const bdFloat32 deltaTime)
{
	bdBool sent = false;
	if(m_deliveryManager.okToSend())
	{
		if(deltaTime < 0)
		{
			BD_INFO(BD_LOG_LEVEL, "Negative deltaTime passed to bdPeer::step.");
		}

		m_elapsedTime += deltaTime;	

		if(m_elapsedTime >= m_sendInterval)
		{
			if(bdRound::BD_IN_ROUND == m_round.getStatus())
			{
				// Get the game state hash for this tick and sent it to all peers
				bdUInt32 hash = 0;
				if(m_game->getGameStateHash(hash))
				{
					m_round.addHash(hash);
				}
			}
			m_elapsedTime -= m_sendInterval;
			sent = m_deliveryManager.send();
		}		
	}
	return sent;
}

bdBool bdPeer::getReplaying() const
{
	return m_replaying;
}

void bdPeer::setReplaying(const bdBool replaying)
{
	m_replaying = replaying;
}

void bdPeer::setReplayPlayerManager(bdPlayerManager *const replayPlayerManager)
{
	m_replayPlayerManager = replayPlayerManager;	
}

void bdPeer::setReplayRound(bdRound *const replayRound)
{
	m_replayRound = replayRound;
}

