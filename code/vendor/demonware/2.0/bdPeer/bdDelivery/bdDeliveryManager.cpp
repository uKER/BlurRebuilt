// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdDelivery/bdDeliveryManager.h>

#include <bdConnection/bdMessage/bdMessage.h>

#include <bdNet/bdNet.h>

#include <bdPeer/bdDelivery/bdDeliveryMessage.h>
#include <bdPeer/bdDelivery/bdDeliveryManagerListener.h>
#include <bdPeer/bdDelivery/bdDeliveryLogger.h>
#include <bdPeer/bdDelivery/bdDeliveryInitializer.h>

#define BD_LOG_LEVEL "peer/deliverymanager"

#define BD_DELIVERY_RECOVERY_SIZE (m_messageLimit)
#define BD_DELIVERY_WINDOW_NUM_MESSAGES ((2*m_messageLimit)+1)
#define BD_DELIVERY_WINDOW_SIZE (BD_DELIVERY_RECOVERY_SIZE + BD_DELIVERY_WINDOW_NUM_MESSAGES)

#define BD_FIRE_LISTENER(l)								\
do														\
{														\
	for(bdUInt i = 0; i < m_listeners.getSize(); i++)	\
	{													\
		m_listeners[i]->l;								\
	}													\
}														\
while(g_assertFalse)

bdDeliveryManager::bdDeliveryManager(const bdUInt messageLimit)
: m_session(BD_NULL),
  m_status(BD_DELIVERY_NOT_JOINED),
  m_messageLimit(messageLimit),
  m_outgoingMsgNum(0),
  m_dispatchNum(0)
{
	BD_ASSERT(messageLimit > 0, "bdDeliveryManager::bdDeliveryManager, "
		"message limit cannot be zero.");

	// create the message window
	m_messageWindow = new bdMessageBatch[BD_DELIVERY_WINDOW_SIZE];
}

bdDeliveryManager::~bdDeliveryManager()
{
	quit();

	if(m_messageWindow)
	{
		delete [] m_messageWindow;
	}
	m_session = BD_NULL;
}

void bdDeliveryManager::init(bdSession *const session)
{
	BD_ASSERT(session != BD_NULL, "bdDeliveryManager::init, "
		"session cannot be null.");

	m_session = session;
	m_status = BD_DELIVERY_NOT_JOINED;
	m_outgoingMsgNum = 0;
	m_dispatchNum = 0;
	
	// register as a session interceptor, so we can dispatch upwards
	m_session->registerInterceptor(this);

	// register as s session listener so we can react to host changes
	m_session->registerListener(this);

	// register as a delivery interceptor as we are going to send ourselves messages
	registerInterceptor(this);
}

void bdDeliveryManager::quit()
{
	if(m_session)
	{
		m_session->unregisterInterceptor(this);
		m_session->unregisterListener(this);
	}
	unregisterInterceptor(this);

	m_session = BD_NULL;
	m_status = BD_DELIVERY_NOT_JOINED;
	m_outgoingMsgNum = 0;
	m_dispatchNum = 0;
	m_outgoingMessage = BD_NULL;
		
	m_activePeers.clear();
	m_joinedPeers.clear();
	m_pendingJoins.clear();
	m_pendingDeactivations.clear();

	if(m_messageWindow)
	{
		// clear the message window
		for(bdUInt i = 0; i < BD_DELIVERY_WINDOW_SIZE; i++)
		{
			m_messageWindow[i].clear();
		}		
	}
}

void bdDeliveryManager::registerInterceptor(bdDeliveryInterceptor *const interceptor)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_interceptors.findFirst(interceptor, index))
	{
		BD_WARN(BD_LOG_LEVEL, "Same interceptor registered multiple times.");
	}
#endif // BD_DEBUG

	m_interceptors.pushBack(interceptor);
}

void bdDeliveryManager::unregisterInterceptor(bdDeliveryInterceptor *const interceptor)
{
	m_interceptors.removeAllKeepOrder(interceptor);
}

void bdDeliveryManager::registerListener(bdDeliveryManagerListener *const listener)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_listeners.findFirst(listener, index))
	{
		BD_WARN(BD_LOG_LEVEL, "Same listener registered multiple times.");
	}
#endif // BD_DEBUG

	m_listeners.pushBack(listener);
}
		
void bdDeliveryManager::unregisterListener(bdDeliveryManagerListener *const listener)
{
	m_listeners.removeAllKeepOrder(listener);
}

void bdDeliveryManager::registerLogger(bdDeliveryLogger *const logger)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_loggers.findFirst(logger, index))
	{
		BD_WARN(BD_LOG_LEVEL,"Same logger registered multiple times.");
	}
#endif // BD_DEBUG

	m_loggers.pushBack(logger);
}

void bdDeliveryManager::unregisterLogger(bdDeliveryLogger *const logger)
{
	m_loggers.removeAllKeepOrder(logger);
}

void bdDeliveryManager::registerInitializer(bdDeliveryInitializer *const initializer)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_initializers.findFirst(initializer, index))
	{
		BD_WARN(BD_LOG_LEVEL, "Same initializer registered multiple times.");
	}
#endif // BD_DEBUG

	m_initializers.pushBack(initializer);
}

void bdDeliveryManager::unregisterInitializer(bdDeliveryInitializer *const initializer)
{
	m_initializers.removeAllKeepOrder(initializer);
}

bdBool bdDeliveryManager::isActive() const
{
	const bdBool active = (m_status == BD_DELIVERY_ACTIVE);
	return active;
}

bdBool bdDeliveryManager::activate() const
{	
	bdBool sent = false;
	if(m_status == BD_DELIVERY_JOINED)
	{
		// send an activate request to the host
		bdMessageRef activateReq = new bdMessage(BD_DELIVERY_MGR_ACTIVATE_REQUEST);
		sent = m_session->sendHost(activateReq);		
	}

	if(m_status == BD_DELIVERY_ACTIVE)
	{
		BD_WARN(BD_LOG_LEVEL,"Already active.");
	}

	if(m_status != BD_DELIVERY_JOINED)
	{
		// should only issue activation requests if we are fully joined. If issue 
		// such a request when not ready we may not be synced with the other 
		// peers and may miss our own activation.
		BD_WARN(BD_LOG_LEVEL,"cannot send activate request while not fully joined.");
	}

	return sent;
}

void bdDeliveryManager::deactivate()
{
	BD_INFO(BD_LOG_LEVEL, "Sending deactivate message");
	if((m_session->getRole() != bdSession::BD_SESSION_HOST) &&
	   (m_status == BD_DELIVERY_ACTIVE))
	{
		bdMessage deactivateMessage(BD_DELIVERY_MGR_DEACTIVATE_MSG);
		addMessage(deactivateMessage);
	}

	if(m_status != BD_DELIVERY_ACTIVE)
	{
		BD_WARN(BD_LOG_LEVEL, "Not active.");
	}

	if(m_session->getRole() == bdSession::BD_SESSION_HOST)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot deactivate host.");
	}
}

void bdDeliveryManager::addMessage(bdMessage& msg)
{
	// message format [messageNum, 1, type, size, data, ... 0]
	if(!m_outgoingMessage)
	{
		createOutgoingMessage();
	}

	bdBitBufferRef outgoingBuffer = m_outgoingMessage->getPayload();
	
	const bdMessageType type = msg.getType();
	
	outgoingBuffer->writeBool(true);
	outgoingBuffer->writeRangedUInt32(type, 0, BD_DELIVERY_MGR_MAX_MESSAGE_TYPE);

	// copy the batch message buffer
	bdBitBufferRef buffer = msg.getPayload();	
	const bdUByte8 *const data = buffer->getData();
	const bdUInt numBits = buffer->getWritePosition();
	const bdUInt16 numBits16 = static_cast<bdUInt16>(numBits);			

	// write num bits as 16 bit, but check the conversion.
	BD_ASSERT(numBits == numBits16, "bdDeliveryManager::addMessage, "
		"numBits is too big.");

	outgoingBuffer->writeUInt16(numBits16);
	outgoingBuffer->writeBits(data, numBits);

}

bdBool bdDeliveryManager::okToSend() const
{

	const bdBool canSend = ((m_outgoingMsgNum - m_dispatchNum) < m_messageLimit) && 
						   (m_status == BD_DELIVERY_ACTIVE) &&
						   (m_session->getStatus() == bdSession::BD_SESSION_READY);
	return canSend;
}

bdBool bdDeliveryManager::send()
{
	bdBool canSend = okToSend();

	if(canSend)
	{
		bdLinkedList<bdPendingJoin>::Position pos = m_pendingJoins.getHeadPosition();
		while(pos)
		{
			bdLinkedList<bdPendingJoin>::Position oldPos = pos;
			const bdPendingJoin& pendingJoin = m_pendingJoins.forward(pos);
			if(pendingJoin.m_joinAt == m_outgoingMsgNum)
			{
				sendInitData(pendingJoin.m_connection);
				m_joinedPeers.pushBack(pendingJoin.m_connection);
				m_pendingJoins.removeAt(oldPos); 
			}
		}

		if(!m_outgoingMessage)
		{
			createOutgoingMessage();
		}

		bdBitBufferRef outgoingBuffer  = m_outgoingMessage->getPayload();

		// write a null terminator
		outgoingBuffer->writeBool(false);

		// write the out going message num
		// be sure to leave the first bit intact
		outgoingBuffer->resetWritePosition();
		outgoingBuffer->writeUInt32(m_outgoingMsgNum);
		
		// send to all joined peers
		canSend = sendToJoined(m_outgoingMessage);

		m_outgoingMessage = BD_NULL;
		m_outgoingMsgNum++;
	}
	
	return canSend;
}

bdSession* bdDeliveryManager::getSession()
{
	return m_session;
}

bdBool bdDeliveryManager::acceptSession(const bdReceivedMessage& receivedMsg)
{
	const bdMessageRef message = receivedMsg.getMessage();
	const bdMessageType type = message->getType();
	bdBool accepted = true;

	switch(type)
	{	
		case BD_DELIVERY_MGR_PENDING_JOIN:
		{
			handlePendingJoin(receivedMsg);
		}
		break;
		case BD_DELIVERY_MGR_INIT_DATA:
		{
			handleInitData(receivedMsg);
		}
		break;
		case BD_DELIVERY_MGR_ACTIVATE_REQUEST:
		{
			handleActivateRequest(receivedMsg);
		}
		break;
		case BD_DELIVERY_MGR_BATCH_MESSAGE:
		{
			handleBatchMessage(receivedMsg);			
		}	
		break;
		case BD_DELIVERY_MGR_RECOVERY_MESSAGE:
		{
			handleRecoveryMessage(receivedMsg);
		}
		break;
		case BD_DELIVERY_MGR_INIT_COMPLETE:
		{
			handleInitComplete(receivedMsg);
		}
		break;
		default:
		{
			accepted = false;
		}
	}

	return accepted;
}

bdBool bdDeliveryManager::acceptDelivery(const bdDeliveryMessage& receivedMsg)
{
	const bdMessageRef message = receivedMsg.getMessage();
	const bdMessageType type = message->getType();
	bdBool accepted = true;

	switch(type)
	{
		case BD_DELIVERY_MGR_ACTIVATE_MSG:
		{
			handleActivate(message);
		}
		break;
		case BD_DELIVERY_MGR_DEACTIVATE_MSG:
		{
			const bdCommonAddrRef aboutAddr = receivedMsg.getAboutAddr();
			handleDeactivate(aboutAddr);
		}
		break;
		default:
		{
			accepted = false;
		}		
	}

	return accepted;
}

void bdDeliveryManager::onSessionConnectSuccess()
{
	// successfully connected to the session 
	if(m_session->getRole() == bdSession::BD_SESSION_HOST)
	{
		// if we are the host we must be active
		m_status = BD_DELIVERY_ACTIVE;

		bdCommonAddrRef localAddr = bdNet::getInstance()->getLocalCommonAddr();
		const bdConnectionRef connection = findSessionConnection(*localAddr);		
		if(connection.notNull())
		{			
			m_joinedPeers.pushBack(connection);
			m_activePeers.pushBack(connection->getAddress());
		}
	}
}

void bdDeliveryManager::onSessionConnect(const bdConnectionRef connection)
{
	if(m_session->getRole() == bdSession::BD_SESSION_HOST)
	{
		// a new peer has requested to join
		// pick a safe point in the future for them to join
		// this is a point at which they will be guaranteed to receive a full
		// set of messages (i.e. a message from each active peer).	
		const bdUInt32 joinAt = m_outgoingMsgNum + m_messageLimit;
		const bdCommonAddrRef addr = connection->getAddress();

		// send a message to all peers informing them of the pending join
		const bdMessageRef pendingJoinMsg = new bdMessage(BD_DELIVERY_MGR_PENDING_JOIN);
		const bdBitBufferRef buffer = pendingJoinMsg->getPayload();		
		addr->serialize(buffer);
		buffer->writeUInt32(joinAt);

		// send the pending join to all joined peers
		sendToJoined(pendingJoinMsg);

		// send the pending join to all joining peers
		m_session->send(connection, pendingJoinMsg);

		// send the pending join to peers whose join is pending
		bdLinkedList<bdPendingJoin>::Position pos = m_pendingJoins.getHeadPosition();
		while(pos)
		{
			const bdPendingJoin& pendingJoin = m_pendingJoins.forward(pos);
			m_session->send(pendingJoin.m_connection, pendingJoinMsg);
		}
	}
}

void bdDeliveryManager::onSessionDisconnect(const bdConnectionRef connection)
{
	// if there is a pending join for this connection remove it
	bdLinkedList<bdPendingJoin>::Position pos = m_pendingJoins.getHeadPosition();
	while(pos)
	{
		bdLinkedList<bdPendingJoin>::Position oldPos = pos;
		const bdPendingJoin& pendingJoin = m_pendingJoins.forward(pos);
		/// todo bdReference == operator review.. to bdPeer code.
		if(pendingJoin.m_connection == connection)
		{
			m_pendingJoins.removeAt(oldPos);
		}
	}

	// if we have joined schedule the disconnected peers deactivation
	// and send recovery messages
	if(m_status >= BD_DELIVERY_JOINED)
	{
		const bdCommonAddrRef addr = connection->getAddress();

		// clean any messages from the disconnected peer from the window
		cleanWindow(addr);

		BD_INFO(BD_LOG_LEVEL, "Adding addr to pending deactivations");
		m_pendingDeactivations.pushBack(addr);

		const bdUInt numPeers = m_session->getNumPeers();
		// is there anyone else in the session (other than us and the disconnecting host)?
		if(numPeers > 1)
		{
			const bdMessageRef recoveryMessage = createRecoveryMessage();

			// send the recovery message to everyone else in the session
			for(bdUInt i = 0; i < numPeers; i++)
			{
				const bdConnectionRef peerConnection = m_session->getConnection(i);
				const bdBool isLoopback = peerConnection->getAddress()->isLoopback();

				/// todo bdReference == operator review.. to bdPeer code.
				if((peerConnection != connection) && !isLoopback)
				{
					m_session->send(peerConnection, recoveryMessage);
				}		
				// As pending deactivations are only processed when the session status
				// changes to ready we will need to call it manually when the session
				// is already ready.
				if(m_session->getStatus() == bdSession::BD_SESSION_READY)
				{
					processPendingDeactivations();
				}
			}
		}
		else
		{
			processPendingDeactivations();
		}
	}

	// remove the peer from the joined list
	m_joinedPeers.removeAll(connection);
}

void bdDeliveryManager::onSessionStatusChange(const bdSession::bdSessionStatus previous,
											  const bdSession::bdSessionStatus current)
{
	if((previous == bdSession::BD_SESSION_NOT_READY) &&
	   (current == bdSession::BD_SESSION_READY))
	{
		processPendingDeactivations();
	}
}

void bdDeliveryManager::handlePendingJoin(const bdReceivedMessage& pendingJoin)
{
	// the host has scheduled the addition of a new peer
	const   bdMessageRef message = pendingJoin.getMessage();
	const bdBitBufferRef buffer = message->getPayload();
	
	const bdCommonAddrRef localAddr = bdNet::getInstance()->getLocalCommonAddr();
	bdCommonAddr joinAddr;
	bdUInt32 joinAt = 0;

	bdBool ok = joinAddr.deserialize(localAddr, buffer);
	ok = ok && buffer->readUInt32(joinAt);

	if(ok)
	{
		if(joinAddr.isLoopback())
		{
			// we are the joining peer, so set the dispatch number ready to receive
			// messages
			m_dispatchNum = joinAt;
			m_status = BD_DELIVERY_JOINING;
		}
		else
		{
			// look up the connection from the session
			const bdConnectionRef connection = findSessionConnection(joinAddr);
			if(connection.notNull())
			{
				// found a matching connection so record the pending join
				const bdPendingJoin pendingJoin(connection, joinAt);
				m_pendingJoins.addTail(pendingJoin);
			}		
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Malformed pending join message");
	}
}

void bdDeliveryManager::handleInitData(const bdReceivedMessage& initData)
{
	BD_INFO(BD_LOG_LEVEL, "Handling Init Data for bdDeliveryManager status: %u", m_status);
	if(m_status == BD_DELIVERY_JOINING)
	{
		// Host Addr Not Set Bug.. DemonWare RT# 1039
		// Fixme -- this should be fixed as part of a bdPeer refactor..
		bdCommonAddrRef hostAddrAsPerInitData = initData.getConnection()->getAddress();
		bdCommonAddrRef hostAddrAsPerSession = m_session->getHost()->getAddress();
		if(hostAddrAsPerInitData != hostAddrAsPerSession)
		{
			BD_WARN(BD_LOG_LEVEL, "Session's host common addr differs from sender of init data");
			bdUInt newHostIndex = 0;
			m_session->getPeerIndex(initData.getConnection(), newHostIndex);
			m_session->setHost(newHostIndex);
		}


		const bdMessageRef message = initData.getMessage();
		const bdBitBufferRef buffer = message->getPayload();

		bdCommonAddrRef localAddr = bdNet::getInstance()->getLocalCommonAddr();

		bdArray<bdConnectionRef> joinedPeers;
		bdArray<bdCommonAddrRef> activePeers;		

		// read the number of active and joined peers
		bdUByte8 numActiveAndJoinedPeers;
		bdBool ok = buffer->readUByte8(numActiveAndJoinedPeers);

		// read the peer addresses that are active and joined
		bdUInt i;
		for(i=0; ok && (i < numActiveAndJoinedPeers); i++)
		{
			bdCommonAddr addr;
			ok = addr.deserialize(localAddr, buffer);
			
			if(ok)
			{
				const bdConnectionRef connection = findSessionConnection(addr);
				if(connection.notNull())
				{	
					joinedPeers.pushBack(connection);
					activePeers.pushBack(connection->getAddress());					
				}
			}
		}

		// read the number of only active (not joined)
		bdUByte8 numActivePeers = 0;		
		ok = ok && buffer->readUByte8(numActivePeers);

		// read the active (not joined) peer addresses
		for(i=0; ok && (i < numActivePeers); i++)
		{
			bdCommonAddr addr;
			ok = addr.deserialize(localAddr, buffer);
			
			if(ok)
			{				
				const bdConnectionRef connection = findSessionConnection(addr);
				if(connection.notNull())
				{
					activePeers.pushBack(connection->getAddress());
				}
			}
		}

		// read the number of only joined (not active)
		bdUByte8 numJoinedPeers = 0;		
		ok = ok && buffer->readUByte8(numJoinedPeers);

		// read the joined (not active) peer addresses
		for(i=0; ok && (i < numJoinedPeers); i++)
		{
			bdCommonAddr addr;
			ok = addr.deserialize(localAddr, buffer);
			
			if(ok)
			{
				const bdConnectionRef connection = findSessionConnection(addr);
				if(connection.notNull())
				{
					joinedPeers.pushBack(connection);
				}
			}
		}

		if(ok)
		{
			m_activePeers = activePeers;
			m_joinedPeers = joinedPeers;
			m_status = BD_DELIVERY_JOINED;

			// add ourselves to the joined peers list		
			const bdConnectionRef connection = findSessionConnection(*localAddr);		
			if(connection.notNull())
			{			
				m_joinedPeers.pushBack(connection);
			}
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Init data received while in unexpected state: %u", m_status);
	}
}

void bdDeliveryManager::handleActivateRequest(const bdReceivedMessage& activateRequest)
{
	if(m_session->getRole() == bdSession::BD_SESSION_HOST)
	{
		const bdConnectionRef connection = activateRequest.getConnection();
		
		// check the peer has joined
		bdUInt index;
		if(m_joinedPeers.findFirst(connection, index))
		{
			// is the sender already active?		
			const bdCommonAddrRef addr = connection->getAddress();
			if(!m_activePeers.findFirst(addr, index))
			{
				bdMessage activateMessage(BD_DELIVERY_MGR_ACTIVATE_MSG);
				const bdBitBufferRef buffer = activateMessage.getPayload();	
				
				addr->serialize(buffer);
				addMessage(activateMessage);
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL,"Received activate request already active peer.");	
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Received activate request from unknown peer.");
		}		
	}
}

void bdDeliveryManager::handleBatchMessage(const bdReceivedMessage& batchMessage)
{	
	const bdMessageRef message = batchMessage.getMessage();
	const bdCommonAddrRef addr = batchMessage.getConnection()->getAddress();	
	const bdDeliveryMessage deliveryMessage(message, addr, addr);
	addToWindow(deliveryMessage);
}

void bdDeliveryManager::handleRecoveryMessage(const bdReceivedMessage& recoveryMessage)
{
		
	const bdCommonAddrRef localAddr = bdNet::getInstance()->getLocalCommonAddr();
	const bdCommonAddrRef fromAddr = recoveryMessage.getConnection()->getAddress();
	const bdBitBufferRef buffer = recoveryMessage.getMessage()->getPayload();

	// read out the address table
	bdUByte8 numAddresses;
	bdBool ok = buffer->readUByte8(numAddresses);

	bdArray<bdCommonAddrRef> addresses(numAddresses);

	bdUInt i;
	for(i = 0; ok && (i < numAddresses); i++)
	{
		bdCommonAddrRef address = new bdCommonAddr;
		ok = address->deserialize(localAddr, buffer);
		if(ok)
		{
			addresses.pushBack(address);
		}
	}

	bdArray<bdDeliveryMessage> recoveryMessages;

	bdBool haveData = false;
	ok = ok && buffer->readBool(haveData);

	while(ok && haveData)
	{
		// read the address index
		bdUInt32 addrIndex = 0;
		ok = ok && buffer->readRangedUInt32(addrIndex, 0, numAddresses);

		// read the data size
		bdUInt16 numBits = 0;
		ok = ok && buffer->readUInt16(numBits);

		// peek at the message num
		const bdUInt32 payloadReadPos = buffer->getReadPosition();
		buffer->resetReadPosition();
		// find the offset of a bit buffer payload (the number of bits of header data eg the type check flag)
		// TODO: add a bdBitBuffer::getNumHeaderBits() and use it instead
		const bdUInt32 offset =  buffer->getReadPosition();
		buffer->setReadPosition(payloadReadPos + offset);
		bdUInt32 messageNum = 0;
		ok = ok && buffer->readUInt32(messageNum);
		buffer->setReadPosition(payloadReadPos);

		// read the data
		bdUByte8 payload[BD_MAX_PAYLOAD_SIZE];
		ok = ok && buffer->readBits(payload, numBits);

		// read the continue flag
		ok = ok && buffer->readBool(haveData);

		if(ok && (messageNum >= m_dispatchNum))
		{
			const bdMessageRef batchMessage = new bdMessage(BD_DELIVERY_MGR_BATCH_MESSAGE, payload, BD_NUM_BITS_TO_NUM_BYTES(numBits), true);

			const bdDeliveryMessage deliveryMessage(batchMessage, fromAddr, addresses[addrIndex]);
			recoveryMessages.pushBack(deliveryMessage);
		}
	}

	// if the parsing was ok then add all the messages
	for(i=0; ok && (i < recoveryMessages.getSize()); i++)
	{			
		addToWindow(recoveryMessages[i]);
	}
	
	if(!ok)
	{
		BD_WARN(BD_LOG_LEVEL," Malformed recovery message.");
	}

}

void bdDeliveryManager::handleInitComplete(const bdReceivedMessage&)
{
	BD_FIRE_LISTENER(onDeliveryInitCompleted());
}

void bdDeliveryManager::handleActivate(const bdMessageRef activateMessage)
{
	const bdBitBufferRef buffer = activateMessage->getPayload();

	const bdCommonAddrRef localAddr = bdNet::getInstance()->getLocalCommonAddr();

	bdCommonAddr about;
	const bdBool ok = about.deserialize(localAddr, buffer);

	if(ok)
	{
		const bdConnectionRef connection = findSessionConnection(about);

		// check the peer has joined
		bdUInt index;
		if(connection.notNull() && m_joinedPeers.findFirst(connection, index))
		{
			// check the peer is not already active
			const bdCommonAddrRef aboutAddr = connection->getAddress();
			if(!m_activePeers.findFirst(aboutAddr, index))
			{
				m_activePeers.pushBack(aboutAddr);

				// if this activate is about us then we'd better 
				// activate so we can start sending messages
				if(aboutAddr->isLoopback())
				{
					m_status = BD_DELIVERY_ACTIVE;
					m_outgoingMsgNum = m_dispatchNum+1;
				}

				BD_FIRE_LISTENER(onDeliveryActivate(aboutAddr));
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL,"Received activate for already active peer.");
			}		
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL," Received activate for unknown peer.");
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Malformed activate message");
	}
}

void bdDeliveryManager::handleDeactivate(const bdCommonAddrRef addr)
{
	BD_INFO(BD_LOG_LEVEL, "Handling deactivate on tick : %u", m_dispatchNum);
	// remove peer from the active array
	bdUInt index = 0;
	const bdBool found = m_activePeers.findFirst(addr, index);
	if(found)
	{
		m_activePeers.removeAt(index);		
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Peer not found in list of active peers");
	}

	// if we are deactivating change our state so we don't send messages
	if(addr->isLoopback())
	{
		m_status = BD_DELIVERY_JOINED;
	}

	// select a new host if necessary
	// the order of m_activePeers may not be synchronized across all peers
	// so use the session to iterate the active peers.	
	/// todo bdReference == operator review.. to bdPeer code.
	bdBool hostSet = !(addr == m_session->getHost()->getAddress());
	const bdUInt numPeers = m_session->getNumPeers();
	for(bdUInt i=0; !hostSet && (i < numPeers); i++)
	{
		const bdConnectionRef connection = m_session->getConnection(i);
		const bdCommonAddrRef peerAddr = connection->getAddress();

		// is this session member active?
		if(m_activePeers.findFirst(peerAddr, index))
		{
			m_session->setHost(i);
			hostSet = true;
		}
	}

	if(!hostSet)
	{
		BD_WARN(BD_LOG_LEVEL,"Unable to promote a new host, no active peers.");

		BD_FIRE_LISTENER(onDeliveryHostMigrationFailed());
	}

	if(found)
	{
		BD_FIRE_LISTENER(onDeliveryDeactivate(addr));
	}
}

void bdDeliveryManager::addToWindow(const bdDeliveryMessage& deliveryMessage)
{
	// First try to add any messages that have been buffered while joining
	// was in progress
	bdLinkedList<bdDeliveryMessage>::Position pos = m_joiningBuffer.getHeadPosition();
	while (pos)
	{
		bdDeliveryMessage& message = m_joiningBuffer.getAt(pos);
		const bdBool added = tryAddToWindow(message);
		if(added)
		{
			m_joiningBuffer.removeAt(pos);
		}
		else
		{
			m_joiningBuffer.forward(pos);
		}
	}

	// Now try to add the latest message
	const bdBool added = tryAddToWindow(deliveryMessage);
	if(!added && (m_status != BD_DELIVERY_ACTIVE))
	{
		m_joiningBuffer.addTail(deliveryMessage);
		BD_INFO(BD_LOG_LEVEL, "Buffered: %u", m_joiningBuffer.getSize());
	}	

}

bdBool bdDeliveryManager::tryAddToWindow(const bdDeliveryMessage& deliveryMessage)
{
	bdBool added = false;
	const bdBitBufferRef buffer = deliveryMessage.getMessage()->getPayload();

	// read the message number
	bdUInt32 messageNum = 0;
	buffer->resetReadPosition();
	const bdBool ok = buffer->readUInt32(messageNum);

	if(ok)
	{
		// if m_status >= BD_DELIVERY_JOINING the m_dispatchNum has been set
		// so we can safely check the message number is in range
		if(m_status >= BD_DELIVERY_JOINING)
		{
			if((messageNum >= m_dispatchNum) && (messageNum < m_dispatchNum + BD_DELIVERY_WINDOW_NUM_MESSAGES))
			{
				const bdUInt windowIndex = messageNum % BD_DELIVERY_WINDOW_SIZE;
				bdMessageBatch& batch = m_messageWindow[windowIndex];				
				batch.pushBack(deliveryMessage);
				added = true;

				// only try to process the window if this message could have filled the
				// current batch, and we have fully joined the delivery (otherwise we
				// won't know who the active peers are and so won't know if the batch is full)
				if((messageNum == m_dispatchNum) && (m_status >= BD_DELIVERY_JOINED))
				{
					processWindow();

					// remove any old pending joins 
					// must do this as if we are not active m_pendingJoins will grow unchecked
					bdLinkedList<bdPendingJoin>::Position pos = m_pendingJoins.getHeadPosition();
					while(pos)
					{
						bdLinkedList<bdPendingJoin>::Position oldPos = pos;
						const bdPendingJoin& pendingJoin = m_pendingJoins.forward(pos);
						if(pendingJoin.m_joinAt < m_dispatchNum)
						{
							m_joinedPeers.pushBack(pendingJoin.m_connection);
							m_pendingJoins.removeAt(oldPos); 
						}
					}
				}
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Out of range message received: msg num: %u , dsp num : %u del win num msgs: %u", messageNum, m_dispatchNum, BD_DELIVERY_WINDOW_NUM_MESSAGES);
			}
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Malformed delivery message");
	}

	return added;
}

void bdDeliveryManager::processWindow()
{
	// is the current batch full/ready to be dispatched
	// do this in a loop in case changes made by the processing of messages cause the
	// next batch to become full (typically through deactivation).
	bdBool batchDispatched;
	do
	{
		batchDispatched = false;

		const bdUInt currentBatchIndex = m_dispatchNum % BD_DELIVERY_WINDOW_SIZE;
		bdMessageBatch& currentBatch = m_messageWindow[currentBatchIndex];

		const bdUInt numActivePeers = m_activePeers.getSize();
		const bdUInt numMessages = currentBatch.getSize();

		// have we received a message from all active peers?
		if(numActivePeers && (numMessages >= numActivePeers))
		{
			// use a set to ensure consistent dispatching order on all peers
			bdSet<bdDeliveryMessage> dispatchSet;

			for(bdUInt i = 0; i < numMessages; i++)
			{
				bdDeliveryMessage& deliveryMessage = currentBatch[i];
				bdCommonAddrRef aboutAddr = deliveryMessage.getAboutAddr();

				// check this message is about an active peer, if this came 
				// from a recovery message aboutAddr will not be in m_activePeers 
				// so we have to search for it using the data rather than just 
				// the pointer/ref
				const bdCommonAddrRef addr = findActiveAddr(*aboutAddr);
				if(addr.notNull())
				{
					deliveryMessage.setAboutAddr(addr);
					dispatchSet.insert(deliveryMessage);
				}
			}

			if(numActivePeers == dispatchSet.getSize())
			{
				bdSet<bdDeliveryMessage>::Iterator iter = dispatchSet.getIterator();

				BD_FIRE_LISTENER(onDeliveryStart());
				bdUInt batchIndex = 0;
			
				while(iter)
				{					
					const bdDeliveryMessage& deliveryMessage = dispatchSet.next(iter);					
					const bdBitBufferRef buffer = deliveryMessage.getMessage()->getPayload();
					buffer->resetReadPosition();

					// check the message number
					bdUInt32 messageNum = 0;
					const bdBool ok = buffer->readUInt32(messageNum);
					BD_ASSERT(ok && (messageNum == m_dispatchNum), "bdDeliveryManager::dispatch, "
						"message is in wrong batch.");					

					// read out individual messages
					bdMessageRef dispatchMessage;
					while((dispatchMessage = getBatchMessage(buffer)).notNull())
					{
						const bdDeliveryMessage toDispatch(dispatchMessage, deliveryMessage.getFromAddr(), deliveryMessage.getAboutAddr());
						dispatch(toDispatch);
						
						// send the message to any loggers
						for(bdUInt j = 0; j < m_loggers.getSize(); j++)
						{
							m_loggers[j]->logMessage(toDispatch);
						}
					}					

					// ensure only the dispatched messages are left in the batch
					currentBatch.set(batchIndex++, deliveryMessage);
				}

				BD_FIRE_LISTENER(onDeliveryEnd());

				// remove any non dispatched messages from the batch
				currentBatch.popBack(numMessages - numActivePeers);

				// clear the oldest entry
				const bdUInt recoveryIndex = getRecoveryIndex();
				bdMessageBatch &oldRecovery = m_messageWindow[recoveryIndex];
				oldRecovery.clear();

				m_dispatchNum++;				
				batchDispatched = true;
			}
		}
	}
	while (batchDispatched);
}

bdMessageRef bdDeliveryManager::getBatchMessage(const bdBitBufferRef buffer)
{
	bdMessageRef message;

	bdBool haveMessage = false;
	bdBool ok = buffer->readBool(haveMessage);
	if(haveMessage && ok)
	{
		bdUInt32 type32;
		ok = buffer->readRangedUInt32(type32, 0, BD_DELIVERY_MGR_MAX_MESSAGE_TYPE);
		const bdMessageType type = static_cast<bdMessageType>(type32);

		bdUInt16 numBits = 0;
		ok = ok && buffer->readUInt16(numBits);

		bdUByte8 payload[BD_MAX_PAYLOAD_SIZE];
		ok = ok && buffer->readBits(payload, numBits);

		if(ok)
		{
			message = new bdMessage(type, payload, BD_NUM_BITS_TO_NUM_BYTES(numBits), true);
		}
	}

	if(!ok)
	{
		BD_WARN(BD_LOG_LEVEL,"Malformed batch message.");
	}

	return message;
}

void bdDeliveryManager::dispatch(const bdDeliveryMessage& message) const
{
	bdBool intercepted = false;
	for(bdUInt i = 0; !intercepted && (i < m_interceptors.getSize()); i++)
	{								
		intercepted = m_interceptors[i]->acceptDelivery(message);
	}

	if(!intercepted)
	{
		BD_ERR(BD_LOG_LEVEL, "No interceptor registered for message type.");
	}
}

bdUInt bdDeliveryManager::getRecoveryIndex() const
{
	const bdUInt currentBatchIndex = m_dispatchNum % BD_DELIVERY_WINDOW_SIZE;
	const bdUInt recoveryIndex = (currentBatchIndex + BD_DELIVERY_WINDOW_NUM_MESSAGES) % BD_DELIVERY_WINDOW_SIZE;
	return recoveryIndex;
}

void bdDeliveryManager::cleanWindow(const bdCommonAddrRef addr)
{
	// remove any buffers that are from addr and are yet to be processed/dispatched.
	// be careful not to remove messages from the recovery section of the window.
	for(bdUInt i = 0; i < BD_DELIVERY_WINDOW_NUM_MESSAGES; i++)
	{
		const bdUInt index = (i + m_dispatchNum) % BD_DELIVERY_WINDOW_SIZE;

		bdMessageBatch &batch = m_messageWindow[index];
		
		for(bdUInt j=0; j < batch.getSize(); j++)
		{
			bdDeliveryMessage& deliveryMessage = batch[j];

			/// todo bdReference == operator review.. to bdPeer code.
			if(deliveryMessage.getFromAddr() == addr)
			{
				batch.removeAt(j);

				// Need to recheck the same slot.
				j--;
			}
		}
	}
}

void bdDeliveryManager::processPendingDeactivations()
{
	// process any pending deactivations
	const bdUInt numDeactivations = m_pendingDeactivations.getSize();

	for(bdUInt i = 0; i < numDeactivations; i++)
	{
		// schedule deactivation after the last message about this peer
		const bdCommonAddrRef toDeactivate = m_pendingDeactivations[i];		
		bdBool haveMessage = true;

		bdUInt j;
		for(j = 0; haveMessage && (j < BD_DELIVERY_WINDOW_NUM_MESSAGES); j++)
		{
			const bdUInt batchIndex = (j + m_dispatchNum) % BD_DELIVERY_WINDOW_SIZE;
			haveMessage = findMessage(batchIndex, toDeactivate);
		}

		if(!haveMessage)
		{			
			const bdCommonAddrRef localAddr = bdNet::getInstance()->getLocalCommonAddr();			

			const bdUInt32 messageNum = m_dispatchNum + j - 1;

			// create a deactivate message
			const bdMessageRef message = new bdMessage(BD_DELIVERY_MGR_BATCH_MESSAGE);
			const bdBitBufferRef buffer = message->getPayload();
			
			buffer->writeUInt32(messageNum);
			buffer->writeBool(true);
			buffer->writeRangedUInt32(BD_DELIVERY_MGR_DEACTIVATE_MSG, 0, BD_DELIVERY_MGR_MAX_MESSAGE_TYPE);
			buffer->writeUInt16(0);
			buffer->writeBool(false);

			const bdDeliveryMessage deliveryMessage(message, localAddr, toDeactivate);
			addToWindow(deliveryMessage);
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Couldn't find empty slot in window for deactivate message.");
		}
	}

	m_pendingDeactivations.clear();
}

void bdDeliveryManager::createOutgoingMessage()
{
	m_outgoingMessage = new bdMessage(BD_DELIVERY_MGR_BATCH_MESSAGE);
	const bdBitBufferRef buffer = m_outgoingMessage->getPayload();	
	buffer->writeUInt32(0);
}

bdConnectionRef bdDeliveryManager::findSessionConnection(const bdCommonAddr& addr) const
{
	bdConnectionRef found;	
	const bdUInt numPeers = m_session->getNumPeers();
	for(bdUInt i=0; !found && (i < numPeers); i++)
	{
		const bdConnectionRef connection = m_session->getConnection(i);
		bdCommonAddrRef connectionAddr = connection->getAddress();
		if(addr == *connectionAddr)
		{
			found = connection;
		}
	}
	if(found.isNull())
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to find address in session.");
	}
	return found;
}

bdCommonAddrRef bdDeliveryManager::findActiveAddr(const bdCommonAddr& addr) const
{
	bdCommonAddrRef found;
	
	// look in the active peers
	const bdUInt numActivePeers = m_activePeers.getSize();
	for(bdUInt i=0; !found && (i < numActivePeers); i++)
	{
		bdCommonAddrRef peerAddr = m_activePeers[i];
		if(addr == *peerAddr)
		{
			found = peerAddr;
		}
	}

	return found;
}

bdBool bdDeliveryManager::findMessage(const bdUInt index,
									  const bdCommonAddrRef fromAddr) const
{
	bdBool found = false;
	const bdMessageBatch& batch = m_messageWindow[index];
	const bdUInt numMessages = batch.getSize();

	for(bdUInt i = 0; !found && (i < numMessages); i++)
	{
		const bdDeliveryMessage& deliveryMessage = batch[i];

		/// todo bdReference == operator review.. to bdPeer code.
		if(deliveryMessage.getFromAddr() == fromAddr)
		{
			found = true;
		}
	}

	return found;
}

bdMessageRef bdDeliveryManager::createRecoveryMessage() const
{
	// to cope with multiple/cascading disconnects we send the entire recovery window			
	// message format [numAddrs, addr ..., 1, addrIndex, numBits, data ... 0]

	const bdMessageRef recoveryMessage = new bdMessage(BD_DELIVERY_MGR_RECOVERY_MESSAGE);
	const bdBitBufferRef buffer = recoveryMessage->getPayload();

	const bdUInt recoveryIndex = getRecoveryIndex();

	// recovery messages can become quite large so optimize the message layout
	// using an address table at the start of the message
	bdArray<bdCommonAddrRef> addresses;

	bdUInt i;
	for(i=0; i < BD_DELIVERY_RECOVERY_SIZE; i++)
	{
		const bdUInt index = (recoveryIndex + i) % BD_DELIVERY_WINDOW_SIZE;
		const bdMessageBatch &recoveryBatch = m_messageWindow[index];				
		const bdUInt batchSize = recoveryBatch.getSize();

		for(bdUInt j=0; j < batchSize; j++)
		{
			const bdDeliveryMessage& message = recoveryBatch[j];
			const bdCommonAddrRef about = message.getAboutAddr();

			bdUInt addrIndex;
			if(!addresses.findFirst(about, addrIndex))
			{
				addresses.pushBack(about);
			}
		}
	}

	// write the address table
	const bdUInt numAddresses = addresses.getSize();
	const bdUByte8 numAddresses8 = static_cast<bdUByte8>(numAddresses);
	BD_ASSERT(numAddresses == numAddresses8, "bdDeliveryManager::createRecoveryMessage, "
		"more than 255 addresses in recovery message.");

	buffer->writeUByte8(numAddresses8);

	for(i=0; i < numAddresses; i++)
	{
		addresses[i]->serialize(buffer);
	}

	// now write the recovery data
	for(i=0; i < BD_DELIVERY_RECOVERY_SIZE; i++)
	{
		const bdUInt index = (recoveryIndex + i) % BD_DELIVERY_WINDOW_SIZE;
		const bdMessageBatch &recoveryBatch = m_messageWindow[index];				
		const bdUInt batchSize = recoveryBatch.getSize();

		for(bdUInt j=0; j < batchSize; j++)
		{
			const bdDeliveryMessage& message = recoveryBatch[j];
			const bdCommonAddrRef about = message.getAboutAddr();

			// look up the about address index
			bdUInt addrIndex = 0;
			if (addresses.findFirst(about, addrIndex))
			{
				const bdBitBufferRef srcBuffer = message.getMessage()->getPayload();
				const bdUInt numBits = srcBuffer->getWritePosition();
				const bdUInt16 numBits16 = static_cast<bdUInt16>(numBits);

				// write num bits as 16 bit, but check the conversion.
				BD_ASSERT(numBits == numBits16, "bdDeliveryManager::createRecoveryMessage, "
					"numBits is too big.");

				buffer->writeBool(true);
				buffer->writeRangedUInt32(addrIndex, 0, numAddresses);
				buffer->writeUInt16(numBits16);
				buffer->writeBits(srcBuffer->getData(), numBits);
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Address lookup failed.");
			}
		}
	}

	// write a null terminator
	buffer->writeBool(false);

	return recoveryMessage;
}

void bdDeliveryManager::sendInitData(const bdConnectionRef connection)
{
	// The host is responsible for send all init data.
	// This makes the system deterministic.
	if(m_session->getRole() == bdSession::BD_SESSION_HOST)
	{
		const bdMessageRef joinData = new bdMessage(BD_DELIVERY_MGR_INIT_DATA);
		const bdBitBufferRef buffer = joinData->getPayload();

		// todo set a BD_MAX_NUM_PEERS to remove need for allocations
		bdArray<bdCommonAddrRef> active(m_activePeers);
		bdArray<bdConnectionRef> joined(m_joinedPeers);
		const bdUInt numActivePeers = active.getSize();
		const bdUInt numJoinedPeers = joined.getSize();
		bdArray<bdCommonAddrRef> both(BD_MAX(numActivePeers, numJoinedPeers));

		// find the peers that are active and joined
		bdUInt i;
		for(i=0; i < active.getSize(); i++)
		{
			const bdCommonAddrRef activeAddr = active[i];

			bdBool found = false;
			for(bdUInt j=0; !found && (j < joined.getSize()); j++)
			{
				const bdConnectionRef joinedConnection = joined[j];
				const bdCommonAddrRef joinedAddr = joinedConnection->getAddress();

				/// todo bdReference == operator review.. to bdPeer code.
				if(activeAddr == joinedAddr)
				{
					found = true;
					both.pushBack(activeAddr);
					active.removeAt(i);
					i--;
					joined.removeAt(j);					
				}
			}
		}
		
		// write the peers that are active and joined
		const bdUInt numBoth = both.getSize();
		const bdUByte8 numBoth8 = static_cast<bdUByte8>(numBoth);
		buffer->writeUByte8(numBoth8);
		for(i=0; i < numBoth; i++)
		{
			both[i]->serialize(buffer);
		}

		// write the peers that are active only
		const bdUInt numActive = active.getSize();
		const bdUByte8 numActive8 = static_cast<bdUByte8>(numActive);
		buffer->writeUByte8(numActive8);
		for(i=0; i < numActive; i++)
		{
			active[i]->serialize(buffer);
		}

		// write the peers that are joined only
		const bdUInt numJoined = joined.getSize();
		const bdUByte8 numJoined8 = static_cast<bdUByte8>(numJoined);
		buffer->writeUByte8(numJoined8);
		for(i=0; i < numJoined; i++)
		{
			const bdCommonAddrRef joinedAddr = joined[i]->getAddress();
			joinedAddr->serialize(buffer);
		}

		m_session->send(connection, joinData);
	
		const bdUInt numInitializers = m_initializers.getSize();
		for(i=0; i < numInitializers; i++)
		{
			m_initializers[i]->sendInitData(connection);
		}	

		// Send a message to let the joining instance know that all init data has
		// been sent. The ordered messaging system will ensure that this message
		// is received after all the above messages.
		const bdMessageRef deliveryInitCompletedMessage = new bdMessage(BD_DELIVERY_MGR_INIT_COMPLETE);
		m_session->send(connection, deliveryInitCompletedMessage);


	}
}

bdBool bdDeliveryManager::sendToJoined(const bdMessageRef message) const
{
	bdBool sent = true;
	for(bdUInt i=0; i < m_joinedPeers.getSize(); i++)
	{
		sent = m_session->send(m_joinedPeers[i], message) && sent;		
	}
	return sent;
}

bdDeliveryManager::bdPendingJoin::bdPendingJoin(const bdConnectionRef connection,
												const bdUInt32 joinAt)
: m_connection(connection),
  m_joinAt(joinAt)												 
{
}

bdDeliveryManager::bdPendingJoin::~bdPendingJoin()
{
}
