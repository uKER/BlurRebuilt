	// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdGameLog.h>

#include <bdPeer/bdPeer.h>
#include <bdPeer/bdPlayer/bdPlayer.h>
#include <bdPeer/bdGame.h>

#define BD_LOG_LEVEL "peer/gamelog"

bdGameLog::bdGameLog()
: m_peer(BD_NULL),
  m_numMessages(0),
  m_numEmptyFrames(0),
  m_numEmptyFramesPlayed(0),
  m_counterIndex(0),
  m_messageIndex(0)
{	
}

bdGameLog::~bdGameLog()
{
	m_peer = BD_NULL;
}

void bdGameLog::start(bdPeer *const peer)
{
	m_peer = peer;	

	m_buffer = new bdBitBuffer;

	// get the player managers serializable state
	bdPlayerManager *const playerManager = m_peer->getPlayerManager();
	playerManager->serialize(m_buffer);

	// copy any listeners from the player manager
	m_playerManager.m_listeners = playerManager->m_listeners;

	// clear any existing players from the player manager
	m_playerManager.m_players.clear();

	// get the rounds serializable state
	bdRound *const round = m_peer->getRound();
	round->serialize(m_buffer);

	// set the round to use the replay player manager
	m_round.m_playerManager = &m_playerManager;

	// get the game state
	bdGame *const game = m_peer->getGame();
	game->serialize(m_buffer);

	m_peer->getDeliveryManager()->registerLogger(this);
	m_peer->getDeliveryManager()->registerListener(this);

	// clear any previously recorded data
	m_messages.clear();
	m_messageCount.clear();
	m_emptyFrameCount.clear();
	m_numMessages = 0;
	m_numEmptyFrames = 0;
}

void bdGameLog::stop()
{
	if(m_peer)
	{
		m_messageCount.pushBack(m_numMessages);
		m_emptyFrameCount.pushBack(m_numEmptyFrames);

		m_peer->getDeliveryManager()->unregisterLogger(this);
		m_peer->getDeliveryManager()->unregisterListener(this);
	}
}

bdBool bdGameLog::startReplay(bdGame *const game)
{
	// setup the initial state
	bdBool ok = false;

	if(m_buffer)
	{
		m_buffer->resetReadPosition();
		ok = m_playerManager.deserialize(m_buffer);	
		ok = ok && m_round.deserialize(m_buffer);
		ok = ok && game->deserialize(m_buffer);

		m_round.m_game = game;

		m_numEmptyFramesPlayed = 0;
		m_counterIndex = 0;
		m_messageIndex = 0;

		m_peer->setReplayPlayerManager(&m_playerManager);
		m_peer->setReplayRound(&m_round);
//		m_peer->setReplaying(true);

		if(!ok)
		{
			BD_ERR(BD_LOG_LEVEL, "Initial state deserialization failed.");
		}
	}

	return ok;
}

void bdGameLog::stopReplay()
{
	m_peer->setReplayPlayerManager(BD_NULL);
	m_peer->setReplayRound(BD_NULL);
	m_peer->setReplaying(false);
}

bdBool bdGameLog::issueMessages()
{
	bdBool issuedMessages = false;
	if(m_counterIndex < m_emptyFrameCount.getSize())
	{
		issuedMessages = true;

		const bdBool replaying = m_peer->getReplaying();
		m_peer->setReplaying(true);

		const bdUInt32 numEmptyFrames = m_emptyFrameCount[m_counterIndex];
		if(m_numEmptyFramesPlayed < numEmptyFrames)
		{
			// empty frame don't issue any messages
			m_numEmptyFramesPlayed++;
		}
		else
		{
			// issue messages
			const bdUInt numMessages = m_messageCount[m_counterIndex];
			for(bdUInt i = 0; i < numMessages; i++)
			{
				const bdDeliveryMessage& deliveryMessage = m_messages[m_messageIndex];

				deliveryMessage.getMessage()->getPayload()->resetReadPosition();
				
				bdBool accepted = m_playerManager.acceptDelivery(deliveryMessage);
				accepted = !accepted && m_round.acceptDelivery(deliveryMessage);
				
				m_messageIndex++;
			}

			m_numEmptyFramesPlayed = 0;
			m_counterIndex++;
		}

		m_round.onDeliveryEnd();

		m_peer->setReplaying(replaying);
	}

	return issuedMessages;
}

bdPlayerManager* bdGameLog::getPlayerManager()
{
	return &m_playerManager;
}

bdRound* bdGameLog::getRound()
{
	return &m_round;
}

void bdGameLog::logMessage(const bdDeliveryMessage& receivedMsg)
{
	m_messages.pushBack(receivedMsg);
	m_numMessages++;
}

void bdGameLog::onDeliveryEnd()
{	
	if(m_numMessages == 0)
	{
		m_numEmptyFrames++;
	}
	else
	{
		m_messageCount.pushBack(m_numMessages);
		m_emptyFrameCount.pushBack(m_numEmptyFrames);
		m_numEmptyFrames = 0;
		m_numMessages = 0;
	}
}
