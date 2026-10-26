// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdSession/bdSession.h>

#include <bdConnection/bdConnectionStore.h>

#include <bdNet/bdNet.h>

#include <bdPeer/bdSession/bdSessionHandler.h>
#include <bdPeer/bdSession/bdSessionListener.h>
#include <bdPeer/bdSession/bdSessionInterceptor.h>
#include <bdSocket/bdAddressMap.h>
#include <bdConnection/bdMessage/bdMessageTypes.h>
#include <bdConnection/bdMessage/bdMessage.h>
#include <bdPeer/bdSession/bdSessionDataFactory.h>
#include <bdCore/bdSocket/bdCommonAddrInfo.h>

#define BD_LOG_LEVEL "bdPeer/session"

#define BD_FIRE_LISTENER(l)								\
do														\
{														\
	for(bdUInt i = 0; i < m_listeners.getSize(); i++)	\
	{													\
		m_listeners[i]->l;								\
	}													\
}														\
while(g_assertFalse)

bdSession::bdSession(bdSessionHandler *const sessionHandler)
: m_handler(sessionHandler),
  m_role(BD_SESSION_PEER),
  m_status(BD_SESSION_NOT_CONNECTED),
  m_localPeerIndex(0),
  m_registeredKey(false)
#ifdef BD_BUFFER_MESSAGES_WHEN_NOT_READY
  ,m_messageBuffer(BD_MESSAGE_BUFFER_SIZE)
#endif // BD_BUFFER_MESSAGES_WHEN_NOT_READY
{
}

bdSession::~bdSession()
{
	cleanup();
}

void bdSession::registerInterceptor(bdSessionInterceptor *const interceptor)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_interceptors.findFirst(interceptor, index))
	{
		BD_WARN(BD_LOG_LEVEL, "Same listener registered multiple times.");
	}
#endif // BD_DEBUG

	m_interceptors.pushBack(interceptor);
}

void bdSession::unregisterInterceptor(bdSessionInterceptor *const interceptor)
{
	m_interceptors.removeAllKeepOrder(interceptor);
}

bdBool bdSession::join(bdCommonAddrRef hostAddr,
					   const bdSecurityID &secID,
					   const bdSecurityKey &secKey,
					   bdBitBuffer *const userData)
{
	bdBool ok = readyToConnect();

	if(ok)
	{
		bdNetImpl *const net = bdNet::getInstance();
		if(hostAddr->isLoopback())
		{
			ok = ok && connectToLocalHost(secID);
			if(ok)
			{
				m_localConnection = m_hostConnection;
				m_hostConnection->registerListener(this);
				setRole(BD_SESSION_HOST);
				setStatus(BD_SESSION_CONNECTING_TO_PEERS);
			}
		}
		else
		{
			ok = ok && connectToRemoteHost(hostAddr, secID);
			ok = ok && connectToLocalPeer(secID);
			ok = ok && createJoinRequest(userData);

			if(ok)
			{
				m_hostConnection->registerListener(this);
				m_localConnection->registerListener(this);
				setRole(BD_SESSION_PEER);
				setStatus(BD_SESSION_CONNECTING_TO_HOST);
			}
		}

		if(ok)
		{
			net->getConnectionStore()->registerDispatchInterceptor(this);

			// record the security ID and key
			m_secID = secID;
			m_secKey = secKey;
			m_gameSecID = secID;
			m_gameSecKey = secKey;
			m_registeredKey = false;
		}
		else
		{
			cleanup();
		}
	}

	return ok;
}

void bdSession::leaveGraceful()
{
	if(m_status != BD_SESSION_NOT_CONNECTED && m_status != BD_SESSION_SHUTTING_DOWN)
	{
		BD_INFO(BD_LOG_LEVEL, "Leaving session - graceful");

		// Set the session to not ready
		setStatus(BD_SESSION_NOT_READY);

		// shutdown any connections
		cleanupGraceful();

		// set the status to shutting down
		setStatus(BD_SESSION_SHUTTING_DOWN);
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Called while the session was shutting down or not connected");
	}
}

void bdSession::leave()
{
	if(m_status != BD_SESSION_NOT_CONNECTED)
	{
		BD_INFO(BD_LOG_LEVEL, "Leaving session");

		// Set the session to not ready
		setStatus(BD_SESSION_NOT_READY);

		// shutdown any connections
		cleanup();

		// set the status back to not connected.
		setStatus(BD_SESSION_NOT_CONNECTED);
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Called while the session was not connected");
	}
}

void  bdSession::registerListener(bdSessionListener *const listener)
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

void bdSession::unregisterListener(bdSessionListener *const listener)
{
	m_listeners.removeAllKeepOrder(listener);
}

bdSession::bdSessionStatus bdSession::getStatus() const
{
	return m_status;
}

bdSession::bdSessionRole bdSession::getRole() const
{
	return m_role;
}

bdUInt bdSession::getNumPeers() const
{
	return m_peers.getSize();
}

bdConnectionRef bdSession::getConnection(const bdUInt index) const
{
	return m_peers[index]->getConnection();
}

void bdSession::setHost(const bdUInt index)
{
	m_hostConnection = m_peers[index]->getConnection();

	if(index == m_localPeerIndex)
	{
		const bdBool alreadyHost = (m_role == BD_SESSION_HOST);
		if(!alreadyHost)
		{
			setRole(BD_SESSION_HOST);
			BD_INFO(BD_LOG_LEVEL, "Promoted to host.");
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Already host.");
		}
	}
	else
	{
		bdNChar8 hostAddrStr[BD_ADDR_STRING_SIZE];
		bdAddressMap::addrToString(m_hostConnection->getAddressHandle(), hostAddrStr, sizeof(hostAddrStr));
		BD_INFO(BD_LOG_LEVEL, "Peer %u (%s) promoted to host.", index, hostAddrStr);
	}
}

bdConnectionRef bdSession::getHost() const
{
	return m_hostConnection;
}

#ifndef BD_BUFFER_MESSAGES_WHEN_NOT_READY

bdBool bdSession::send(bdMessageRef msg,
					   const bdBool reliable)
{
//	BD_ASSERT(m_status == BD_SESSION_READY, "bdSession::send, "
//		"session members are undefined.");

	// TODO : Decide if the appropriate behavior in case of message sending failure
	//        is to:
	//        1) Stop and not try and send the message to the other connections.
	//        2) Attempt to send the message to all remaining connections [current].
	bdBool messagesSent = true;
	// try to send to all peers
	for(bdUInt i = 0; i < m_peers.getSize(); i++)
	{
		const bdConnectionRef connection = m_peers[i]->getConnection();
		messagesSent = connection->send(msg, reliable) && messagesSent;
	}
	return messagesSent;
}

#else // BD_BUFFER_MESSAGES_WHEN_NOT_READY

bdBool bdSession::send(bdMessageRef msg,
					   const bdBool reliable)
{
	bdBufferedMessage bufferedMessage(msg, bdBufferedMessage::BD_TO_SPECIFIC, reliable);
	for(bdUInt i = 0; i < m_peers.getSize(); i++)
	{
		bdBool messageSent = false;
		const bdConnectionRef connection = m_peers[i]->getConnection();

		if(m_status == BD_SESSION_READY)
		{
			messageSent = connection->send(msg, reliable);
		}

		// if the send failed store the connection for buffering
		if((!messageSent) && reliable)
		{
			bufferedMessage.addConnection(connection);
		}
	}

	// if any of the sends failed (and it's reliable) try to buffer the message
	bdBool allMessagesSent = (bufferedMessage.getConnections().getSize() == 0);
	if(!allMessagesSent)
	{
		BD_INFO(BD_LOG_LEVEL, "Unable to send reliable message to all peers, so buffering.");
		allMessagesSent = addToBuffer(bufferedMessage);
	}
//	else
	{
//		BD_INFO(BD_LOG_LEVEL, "Did not send message to session, session not ready.");
	}

	if(!allMessagesSent)
	{
		BD_INFO(BD_LOG_LEVEL, "Sending message to all peers failed.");
	}

	return allMessagesSent;
}

#endif // BD_BUFFER_MESSAGES_WHEN_NOT_READY

#ifndef BD_BUFFER_MESSAGES_WHEN_NOT_READY

bdBool bdSession::send(bdConnectionRef peerConnection,
					   bdMessageRef msg,
					   const bdBool reliable)
{
#ifdef BD_DEBUG
	bdUInt index;
	const bdBool found = getPeerIndex(peerConnection, index);
	BD_ASSERT(found, "bdSession::send, cannot find connection in session.");
#endif // BD_DEBUG

	const bdBool messageSent = peerConnection->send(msg, reliable);
	return messageSent;
}

#else // BD_BUFFER_MESSAGES_WHEN_NOT_READY

bdBool bdSession::send(bdConnectionRef peerConnection,
					   bdMessageRef msg,
					   const bdBool reliable)
{
#ifdef BD_DEBUG
	bdUInt index;
	const bdBool found = getPeerIndex(peerConnection, index);
	BD_ASSERT(found, "bdSession::send, cannot find connection in session.");
#endif // BD_DEBUG

	bdBool messageSent = peerConnection->send(msg, reliable);
	if((!messageSent) && reliable)
	{
		BD_INFO(BD_LOG_LEVEL, "Unable to send reliable message, buffering it.");

		bdBufferedMessage bufferedMessage(msg, bdBufferedMessage::BD_TO_SPECIFIC, reliable);
		bufferedMessage.addConnection(peerConnection);
		messageSent = addToBuffer(bufferedMessage);
	}

	return messageSent;
}

#endif // BD_BUFFER_MESSAGES_WHEN_NOT_READY

#ifndef BD_BUFFER_MESSAGES_WHEN_NOT_READY

bdBool bdSession::sendHost(bdMessageRef msg,
						   const bdBool reliable)
{
	bdBool messageSent = false;
	//if(m_status == BD_SESSION_READY)
	if ( m_hostConnection.notNull() )
	{
		messageSent = m_hostConnection->send(msg, reliable);
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "No host connection.");
	}


	if(!messageSent)
	{
		BD_INFO(BD_LOG_LEVEL, "Sending message to peer failed.");
	}
	return messageSent;
}

#else //BD_BUFFER_MESSAGES_WHEN_NOT_READY

bdBool bdSession::sendHost(bdMessageRef msg,
						   const bdBool reliable)
{
	bdBool messageSent = false;
	if(m_status == BD_SESSION_READY && m_hostConnection.notNull())
	{
		messageSent = m_hostConnection->send(msg, reliable);
	}

	if((!messageSent) && reliable)
	{
		BD_INFO(BD_LOG_LEVEL, "Unable to send reliable message to host, buffering it.");

		bdBufferedMessage bufferedMessage(msg, bdBufferedMessage::BD_TO_HOST, reliable);
		messageSent = addToBuffer(bufferedMessage);
	}

	if(!messageSent)
	{
		BD_INFO(BD_LOG_LEVEL, "Sending message to host failed.");
	}

	return messageSent;
}

#endif // BD_BUFFER_MESSAGES_WHEN_NOT_READY

bdBool bdSession::sendUpdatePeerList()
{
	bdBool sent = false;
	if(m_role == BD_SESSION_HOST)
	{
		// create the resolution message
		const bdMessageRef msg = new bdMessage(BD_UPDATE_PEER_LIST);
		if(msg.notNull())
		{
			const bdBitBufferRef buffer = msg->getPayload();
			if(buffer.notNull())
			{
				const bdUInt numPeers = m_peers.getSize();
				buffer->writeUInt32(numPeers);

				// write the common addrs
				for(bdUInt i=0; i < numPeers; i++)
				{
					const bdSessionDataRef peerData = m_peers[i];

					bdConnectionRef connection;
					if(peerData.notNull())
					{
						connection = peerData->getConnection();
					}

					if(connection.notNull())
					{
						const bdCommonAddrRef addr = connection->getAddress();
						if(addr.notNull())
						{
							//write the address
							addr->serialize(buffer);
						}
					}
				}

				// send the message to all peers (no need to send it to yourself)
				bdBool sentToAll = true;
				for(bdUInt i=0; i < numPeers; i++)
				{
					if(i != m_localPeerIndex)
					{
						const bdSessionDataRef peerData = m_peers[i];
						bdConnectionRef connection;
						if(peerData.notNull())
						{
							connection = peerData->getConnection();
						}

						if(connection.notNull())
						{
							sentToAll = connection->send(msg, true) && sentToAll;
						}
					}
				}

				sent = sentToAll;
			}
		}
	}

	return sent;
}

bdBool bdSession::getPeerIndex(const bdConnectionRef connection,
							   bdUInt& index) const
{
	const bdUInt numPeers = m_peers.getSize();
	bdBool found = false;
	for(bdUInt i=0; !found && (i < numPeers); i++)
	{
		/// todo bdReference == operator review..
		if(m_peers[i]->getConnection() == connection)
		{
			index = i;
			found = true;
		}
	}
	return found;
}

bdUInt bdSession::getLocalPeerIndex() const
{
	return m_localPeerIndex;
}

bdBool bdSession::readyToConnect() const
{
	const bdBool readyToConnect = (m_status == BD_SESSION_NOT_CONNECTED);
	if(!readyToConnect)
	{
		BD_WARN(BD_LOG_LEVEL, "Session status must be BD_SESSION_NOT_CONNECTED to connect.");
	}

	return readyToConnect;
}

bdBool bdSession::connectToLocalHost(const bdSecurityID& secID)
{
	const bdNetImpl *const net = bdNet::getInstance();
	const bdCommonAddrRef localAddr = net->getLocalCommonAddr();
    const bdBool connecting = startConnect(m_hostConnection, localAddr, secID, "local host");
	return connecting;
}

bdBool bdSession::connectToRemoteHost(const bdCommonAddrRef hostAddr,
									  const bdSecurityID& secID)
{
    const bdBool connecting = startConnect(m_hostConnection, hostAddr, secID, "remote host");
	return connecting;
}

bdBool bdSession::connectToLocalPeer(const bdSecurityID& secID)
{
	const bdNetImpl *const net = bdNet::getInstance();
	const bdCommonAddrRef localAddr = net->getLocalCommonAddr();
    const bdBool connecting = startConnect(m_localConnection, localAddr, secID, "local peer");
	return connecting;
}

bdBool bdSession::startConnect(bdConnectionRef& connection,
							   const bdCommonAddrRef addr,
							   const bdSecurityID& secID,
							   const bdNChar8 *const connectionDesc)
{
	bdBool connecting = false;
	if(!connection)
	{
		bdNetImpl *const net = bdNet::getInstance();
		bdConnectionStore *const store = net->getConnectionStore();

		connection = store->create(addr, secID);
		if(connection)
		{
			connecting = connection->connect();
			if (connecting)
			{
				BD_INFO(BD_LOG_LEVEL, "Connecting to %s.", connectionDesc);
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to initialize %s connection.", connectionDesc);
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to create %s connection.", connectionDesc);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL "%s connection already exists.", connectionDesc);
	}

	return connecting;
}

bdBool bdSession::createJoinRequest(bdBitBuffer *const userData)
{
	bdBool createdJoinRequest = false;

	if(m_sessionJoinRequest.notNull())
	{
		BD_WARN(BD_LOG_LEVEL, "Join request already exists, old request discarded.");
	}

	// create a join request message
	m_sessionJoinRequest = new bdMessage(BD_SESSION_JOIN_REQ);
	if(m_sessionJoinRequest)
	{
		createdJoinRequest = true;
		// copy the user data into the message
		if(userData)
		{
			const bdBitBufferRef buffer = m_sessionJoinRequest->getPayload();
			createdJoinRequest = buffer->append(*userData);

			if(!createdJoinRequest)
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to append used data to join request");
				m_sessionJoinRequest = BD_NULL;
			}
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to allocate join request message.");
	}

	return createdJoinRequest;
}

void bdSession::checkSessionConsistency()
{
	// Check if m_peers is consistent
	bdBool consistent = true;

	bdUInt numPeers = m_peers.getSize();
	for(bdUInt i=1; consistent && (i < numPeers); i++)
	{
		consistent = (m_peers[0]->equals(m_peers[i]));
	}

	// Set status appropriately
	if(consistent)
	{
		setStatus(BD_SESSION_READY);
		BD_INFO(BD_LOG_LEVEL, "Session status: BD_SESSION_READY");

		// if the joining peer is now in the session the join is finished
		numPeers = m_peers.getSize();
		for(bdUInt i = 0; m_joiningPeer && (i < numPeers); i++)
		{
			if(m_joiningPeer == m_peers[i]->getConnection())
			{
				BD_FIRE_LISTENER(onSessionConnect(m_joiningPeer));
				m_joiningPeer = BD_NULL;
			}
		}
		processPendingJoinRequest();
	}
	else
	{
		setStatus(BD_SESSION_NOT_READY);
		BD_INFO(BD_LOG_LEVEL, "Session status: BD_SESSION_NOT_READY");
	}

//	bdNChar8 sessionInfo[1024];
//	bdSessionInfo::getInfo(this, sessionInfo, sizeof(sessionInfo));
//	BD_INFO(BD_LOG_LEVEL, sessionInfo);
}

void bdSession::doLocalHash()
{
	const bdUInt numPeers = m_peers.getSize();
	if(numPeers > m_localPeerIndex)
	{
		m_peers[m_localPeerIndex]->calculateLocalHash(m_peers);
	}
	else
	{
		// FIXME: find out why this happens and fix it
		BD_ERR(BD_LOG_LEVEL, "Local index is %u but there are only %u peers.", m_localPeerIndex, numPeers);
	}
}

void bdSession::sendConsistencyUpdates()
{
	const bdUInt numPeers = m_peers.getSize();
	if(numPeers > m_localPeerIndex)
	{
		bdMessageRef consistencyMessage = new bdMessage(BD_SESSION_CONSISTENCY_MESSAGE);
		bdBitBufferRef bitbuffer = consistencyMessage->getPayload();

		// Serialize our local bdSessionData into the message...
		m_peers[m_localPeerIndex]->serialize(bitbuffer);

		for(bdUInt i=0; i < numPeers; i++)
		{
			if(i != m_localPeerIndex)
			{
				m_peers[i]->getConnection()->send(consistencyMessage, true);
			}
		}
	}
	else
	{
		// FIXME: find out why this happens and fix it
		BD_ERR(BD_LOG_LEVEL, "Local index is %u but there are only %u peers.", m_localPeerIndex, numPeers);
	}
}

void bdSession::setStatus(const bdSessionStatus status)
{
	if(status != m_status)
	{
		BD_FIRE_LISTENER(onSessionStatusChange(m_status, status));
		m_status = status;
	}

#ifdef BD_BUFFER_MESSAGES_WHEN_NOT_READY
	if(status == BD_SESSION_READY)
	{
		sendBufferedMessages();
	}
#endif // BD_BUFFER_MESSAGES_WHEN_NOT_READY
}

void bdSession::setRole(const bdSessionRole role)
{
	if(role != m_role)
	{
		BD_FIRE_LISTENER(onSessionRoleUpdate(role));
		m_role = role;
	}
}

void bdSession::onConnect(bdConnectionRef connection)
{
	bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
	bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, sizeof(addrStr));

	if (m_status == BD_SESSION_CONNECTING_TO_PEERS)
	{
		bdUInt index = 0;
		if(m_pendingConnections.findFirst(connection, index))
		{
			m_pendingConnections.removeAt(index);

			bdUInt peerIndex;
			getPeerIndex(connection, peerIndex);
			BD_INFO(BD_LOG_LEVEL, "Connected to peer %u (%s).", peerIndex, addrStr);
		}

		if(m_pendingConnections.isEmpty())
		{
			// do we need a local loopback connection?
			bdSessionDataRef data = bdSessionDataFactory::getInstance()->create();
			data->setConnection(m_localConnection);
			m_peers.pushBack(data);

			m_localPeerIndex = m_peers.getSize() - 1;

			doLocalHash();
			checkSessionConsistency();
			sendConsistencyUpdates();

			BD_INFO(BD_LOG_LEVEL, "Connected to all peers. Local peer index is %u.", m_localPeerIndex);

			BD_FIRE_LISTENER(onSessionConnectSuccess());
		}
	}
	/// todo bdReference == operator review..
	else if (m_status == BD_SESSION_CONNECTING_TO_HOST && connection == m_hostConnection )
	{
		setStatus(BD_SESSION_CONNECTED_TO_HOST);

		BD_INFO(BD_LOG_LEVEL, "Connected to host (%s), sending join request.", addrStr);
		BD_ASSERT(m_sessionJoinRequest.notNull(), "The peer should not send a null message, indicates logic failure in bdPeer");

		m_hostConnection->send(m_sessionJoinRequest, true);
		m_sessionJoinRequest = BD_NULL;
	}


}

void bdSession::onConnectFailed(bdConnectionRef connection)
{
	// is this the host connection
	/// todo bdReference == operator review..
	const bdBool hostConnectionFailed = m_hostConnection == connection;

	// is this a peer connection
	bdUInt index = 0;
	const bdBool peerConnectionFailed = m_pendingConnections.findFirst(connection, index);

	// is it a session connection
	const bdBool sessionConnectionFailed = peerConnectionFailed || hostConnectionFailed;

	if(sessionConnectionFailed)
	{
		switch(m_status)
		{
			case BD_SESSION_CONNECTING_TO_HOST:
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to connect to host.");
				m_sessionJoinRequest = BD_NULL;
				setStatus(BD_SESSION_CONNECTING_TO_HOST_FAILED);
			}
			break;
			case BD_SESSION_CONNECTING_TO_PEERS:
			{
				// this error message prints the index from m_pendingConnections
				// fixme -- print addr instead...
				BD_WARN(BD_LOG_LEVEL, "Failed to connect to a peer %u.", index);
				setStatus(BD_SESSION_CONNECTING_TO_PEERS_FAILED);
			}
			break;
			default:
			{
				BD_ERR(BD_LOG_LEVEL, "Connecting failed while not connecting to host or peers.");
				setStatus(BD_SESSION_NOT_CONNECTED);
			}
		}

		BD_FIRE_LISTENER(onSessionConnectFail());
		cleanup();
	}
}

void bdSession::onDisconnect(bdConnectionRef connection)
{
	// is the disconnected peer the session host?
	/// todo bdReference == operator review..
	if((connection == m_hostConnection) && (m_status == BD_SESSION_CONNECTED_TO_HOST))
	{
		// if we had issued a join request and not yet heard back
		// then we can't promote anyone as we don't have a list of peers
		// yet, so just fail.
		BD_INFO(BD_LOG_LEVEL, "Host disconnected, while waiting for peer list.");
		BD_FIRE_LISTENER(onSessionConnectFail());
	}

	// has the disconnected peer joined the session?
	bdBool removed = false;
	const bdUInt numPeers = m_peers.getSize();

	for(bdUInt i = 0; !removed && (i < numPeers); i++)
	{
		const bdSessionDataRef peerData = m_peers[i];
		/// todo bdReference == operator review..
		if(peerData->getConnection() == connection)
		{
			bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
			bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, sizeof(addrStr));
			BD_INFO(BD_LOG_LEVEL, "Peer %u (%s) disconnected.", i, addrStr);

			removed = true;

			// someone has left so we must be inconsistent
			// this is set to ensure the onStatusChanged callback is fired
			// otherwise it will not be fired if we are the only peer left
			// TODO: Remove this...it's only there so host migration that is
			// triggered on a session status change works for the final peer.
			// A better solution if to flag that migration needs to happen when
			// the host disconnects, and check the flag when the status flips
			// to ready.
			setStatus(BD_SESSION_NOT_READY);
			m_peers.removeSection(i, i+1);

			// reassign the local host index if required
			if(m_localPeerIndex > i)
			{
				m_localPeerIndex--;
			}
			doLocalHash();
			sendConsistencyUpdates();
			checkSessionConsistency();

			BD_FIRE_LISTENER(onSessionDisconnect(connection));
		}
	}

	// is the disconnected peer someone we are trying to connect to?
	// if so it from the pending list
	// TODO: can this actually happen, can we get an onDisconnect for someone
	// we have not connected to yet?
	bdUInt index = 0;
	if(m_pendingConnections.findFirst(connection, index))
	{
		m_pendingConnections.removeAt(index);
	}

	// has the disconnected peer sent us a message that is in the pending joins list?
	bdLinkedList<bdReceivedMessage>::Position pos = m_pendingJoinRequests.getHeadPosition();
	while(pos)
	{
		const bdReceivedMessage& joinRequest = m_pendingJoinRequests.getAt(pos);
		bdConnectionRef sender = joinRequest.getConnection();

		/// todo bdReference == operator review.. to bdPeer code.
		if(connection == sender)
		{
			bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
			bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, sizeof(addrStr));
			BD_INFO(BD_LOG_LEVEL, "Queued joining peer disconnected (%s).", addrStr);

			m_pendingJoinRequests.removeAt(pos);
		}
		else
		{
			m_pendingJoinRequests.forward(pos);
		}
	}

	// is the disconnected peer in the process of joining?
	/// todo bdReference == operator review.. to bdPeer code.
	if(connection == m_joiningPeer)
	{
		bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
		bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, sizeof(addrStr));
		BD_INFO(BD_LOG_LEVEL, "Joining peer disconnected (%s).", addrStr);

		m_joiningPeer = BD_NULL;

		// process any pending join requests
		processPendingJoinRequest();
	}

	// don't listen to the connection any longer
	connection->unregisterListener(this);
}

bdBool bdSession::accept(const bdReceivedMessage& receivedMsg)
{
	bdMessageRef message = receivedMsg.getMessage();
	const bdMessageType type = message->getType();
	bdBool accepted = true;

	switch (type)
	{
		case BD_SESSION_JOIN_REQ:
		{
			handleJoinRequest(receivedMsg);
		}
		break;
		case BD_SESSION_JOIN_REPLY:
		{
			handleJoinReply(receivedMsg);
		}
		break;
		case BD_SESSION_CONSISTENCY_MESSAGE:
		{
			handleConsistencyMessage(receivedMsg);
		}
		break;
		case BD_UPDATE_PEER_LIST:
		{
			handleUpdatePeerList(receivedMsg);
		}
		break;
		default:
		{
			// is the message from a member of this session?
			bdUInt index;
			accepted = false;
			const bdConnectionRef connection = receivedMsg.getConnection();
			if(getPeerIndex(connection, index))
			{
				// dispatch message to any interceptors
				for(bdUInt i = 0; !accepted && (i < m_interceptors.getSize()); i++)
				{
					accepted = m_interceptors[i]->acceptSession(receivedMsg);
				}
			}
		}
	}

	return accepted;
}

void bdSession::handleJoinRequest(const bdReceivedMessage& joinRequest)
{
	if(m_role == BD_SESSION_HOST)
	{
		const bdConnectionRef connection = joinRequest.getConnection();
		connection->registerListener(this);

		bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
		bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, sizeof(addrStr));
		BD_INFO(BD_LOG_LEVEL, "Received join request (%s).", addrStr);

		if (!m_joiningPeer && (m_status == BD_SESSION_READY))
		{
			processJoinRequest(joinRequest);
		}
		else
		{
			const bdNChar8 *const notReady = "Session not ready";
			const bdNChar8 *const inProgress = "Join in progress";
			const bdNChar8 *const message = m_joiningPeer.notNull()?inProgress:notReady;
			BD_INFO(BD_LOG_LEVEL, "%s queuing join (%s).", message, addrStr);

			// Add the join request into a list of pending joins and
			// process the list when we the state flips back to BD_SESSION_READY
			m_pendingJoinRequests.addTail(joinRequest);
		}
	}
	else
	{
		// we are not a host so can't deal with the join request.
		BD_WARN(BD_LOG_LEVEL, "Non-host received join request.");
	}
}

void bdSession::handleJoinReply(const bdReceivedMessage& joinReply)
{
	const bdConnectionRef connection = joinReply.getConnection();

	/// todo bdReference == operator review.. to bdPeer code.
	if((m_status != BD_SESSION_CONNECTED_TO_HOST) &&
	   (connection != m_hostConnection))
	{
		return;
	}

	const bdMessageRef message = joinReply.getMessage();
	const bdBitBufferRef buffer = message->getPayload();
	bdBool cleanUp = false;
	bdBool joinAllowed = false;
	bdBool ok = buffer->readBool(joinAllowed);

	if(ok)
	{
		if(joinAllowed)
		{
			bdNetImpl *const net = bdNet::getInstance();

			bdSecurityID gameSecID;
			bdSecurityKey gameSecKey;

			ok = ok && buffer->readBits(gameSecID.ab, BD_SECURITY_ID_LENGTH << 3);
			ok = ok && buffer->readBits(gameSecKey.ab, BD_SECURITY_KEY_LENGTH << 3);

			// read out the number of peers
			bdUByte8 numPeers = 0;
			ok = ok && buffer->readUByte8(numPeers);

			bdArray<bdCommonAddrRef> peers(numPeers);
			bdCommonAddrRef localAddr = net->getLocalCommonAddr();

			// read out the peer data
			for(bdUInt i = 0; ok && (i < numPeers); i++)
			{
				bdCommonAddrRef addr = new bdCommonAddr;
				ok = addr->deserialize(localAddr, buffer);

				if(ok)
				{
					peers.pushBack(addr);
				}
			}

			if(ok)
			{
				// register the game sec id and key if they are new
				if((bdMemcmp(gameSecID.ab, m_secID.ab, BD_SECURITY_ID_LENGTH) != 0) ||
				   (bdMemcmp(gameSecKey.ab, m_secKey.ab, BD_SECURITY_KEY_LENGTH) != 0))
				{
					// add the key to the sec key map
					m_registeredKey = net->getSocketRouter()->getKeyMap()->registerKey(gameSecID, gameSecKey);
					if(m_registeredKey)
					{
						// record the game sec key and id
						m_gameSecID = gameSecID;
						m_gameSecKey = gameSecKey;
					}
					else
					{
						BD_WARN(BD_LOG_LEVEL, "Failed to register game security key.");
					}
				}
			}

			if(ok)
			{
				BD_INFO(BD_LOG_LEVEL, "Join allowed. Session has %u peers.", numPeers);

				// everything was read out correctly so it's safe to proceed
				BD_FIRE_LISTENER(onSessionJoinAccepted());
				setStatus(BD_SESSION_CONNECTING_TO_PEERS);

				// create a connection to each peer
				bdCommonAddrRef hostAddr = m_hostConnection->getAddress();

				m_peers.ensureCapacity(numPeers);
				m_pendingConnections.ensureCapacity(numPeers-1);

				bdBool connectionsOk = true;

				for(bdUInt i = 0; connectionsOk && (i < numPeers); i++)
				{
					bdCommonAddrRef peerAddr = peers[i];
					if(*peerAddr == *hostAddr)
					{
						// we are already connected to the host
						bdSessionDataRef data = bdSessionDataFactory::getInstance()->create();
						data->setConnection(m_hostConnection);
						m_peers.pushBack(data);
					}
					else
					{
						bdNChar8 connectionDesc[16];
						bdSnprintf(connectionDesc, sizeof(connectionDesc), "peer %u", i);

						bdConnectionRef peerConnection;
						const bdBool connecting = startConnect(peerConnection, peerAddr, m_gameSecID, connectionDesc);
						if(connecting)
						{
							peerConnection->registerListener(this);
							bdSessionDataRef data = bdSessionDataFactory::getInstance()->create();
							data->setConnection(peerConnection);
							m_peers.pushBack(data);
							m_pendingConnections.pushBack(peerConnection);
						}
						else
						{
							BD_FIRE_LISTENER(onSessionConnectFail());
							connectionsOk = false;
							cleanUp = true;
						}
					}
				}

				// call onConnect here in case the peers contains just the host
				if(connectionsOk)
				{
					onConnect(m_hostConnection);
				}
			}
		}
		else
		{
			// join was refused
			BD_INFO(BD_LOG_LEVEL, "Join refused.");
			BD_FIRE_LISTENER(onSessionJoinRefused(buffer));
			cleanUp = true;
		}
	}

	if(!ok)
	{
		// message parsing failed
		BD_WARN(BD_LOG_LEVEL, "Malformed join reply.");
		cleanUp = true;
	}

	if(cleanUp)
	{
		setStatus(BD_SESSION_NOT_CONNECTED);
		cleanup();
	}
}

void bdSession::handleConsistencyMessage(const bdReceivedMessage& consistencyMsg)
{
	bdBool ok = false;
	if(m_localConnection.isNull())
	{
		return;
	}

	// parse the consistency message
	const bdMessageRef message = consistencyMsg.getMessage();
	const bdBitBufferRef bitbuffer = message->getPayload();

	// is the consistency message from a known peer
	bdUInt index;
	const bdConnectionRef connection = consistencyMsg.getConnection();
	if(getPeerIndex(connection, index))
	{
		// message is from a known peer
		ok = m_peers[index]->deserialize(bitbuffer);
	}
	else
	{
		// message is from a new peer
		/// todo bdReference == operator review.. to bdPeer code.
		if((m_role == BD_SESSION_HOST) && (m_joiningPeer != connection))
		{
			/// todo bdReference == operator review.. to bdPeer code.
			BD_ASSERT(m_joiningPeer == connection, "bdSession::handleConsistencyMessage, "
				"unknown new peer attempting to join.");
		}
		else
		{
			/// todo bdReference == operator review.. to bdPeer code.
			if(m_joiningPeer != connection)
			{
				connection->registerListener(this);
			}


			bdSessionDataRef data = bdSessionDataFactory::getInstance()->create();
			data->setConnection(connection);
			ok = data->deserialize(bitbuffer);
			if(ok)
			{
				m_peers.pushBack(data);

				doLocalHash();
				sendConsistencyUpdates();

				m_joiningPeer = connection;
			}
		}
	}
	if(ok)
	{
		checkSessionConsistency();
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "Malformed consistency message.");
	}
}

void bdSession::handleUpdatePeerList(const bdReceivedMessage& updatePeerListMsg)
{
	const bdConnectionRef connection = updatePeerListMsg.getConnection();
	const bdMessageRef msg = updatePeerListMsg.getMessage();

	bdBitBufferRef buffer;
	if(msg.notNull())
	{
		buffer = msg->getPayload();
	}

	if(buffer.notNull() && (connection == m_hostConnection))
	{
		// read out the number of peers
		bdUInt32 numPeers = 0;
		bdBool ok = buffer->readUInt32(numPeers);

		bdArray<bdConnectionRef> hostConnections(numPeers);
		bdBool forceLeave = false;

		const bdCommonAddrRef localAddr = bdNet::getInstance()->getLocalCommonAddr();

		for(bdUInt i=0; ok && (i < numPeers); i++)
		{
			bdCommonAddr addr;
			ok = addr.deserialize(localAddr, buffer);

			if(ok)
			{
				// find the matching connection
				const bdUInt numSessionPeers = m_peers.getSize();
				bdConnectionRef sessionConnection;
				bdBool found = false;
				for(bdUInt j=0; !found && (j < numSessionPeers); j++)
				{
					const bdSessionDataRef peerData = m_peers[j];
					if(peerData.notNull())
					{
						sessionConnection = peerData->getConnection();
						if(sessionConnection.notNull())
						{
							found = (*(sessionConnection->getAddress()) == addr);
						}
					}
				}

				if(found)
				{
					// the host is connected to this guy and so am I
					hostConnections.pushBack(sessionConnection);
				}
				else
				{
					// I don't have a connection that host has, so disconnect
					bdNChar8 addrStr[BD_COMMON_ADDR_INFO_SIZE];
					bdCommonAddrInfo::getBriefInfo(addr, addrStr, sizeof(addrStr));
					BD_INFO(BD_LOG_LEVEL, "Host is connected to %s, I am not. Calling onForceLeave.", addrStr);
					BD_FIRE_LISTENER(onForceLeave());
					forceLeave = true;
					ok = false;
				}
			}
		}

		// am I connected to any other peers
		if(!forceLeave)
		{
			bdArray<bdConnectionRef> toDisconnect;
			const bdUInt numSessionPeers = m_peers.getSize();
			for(bdUInt i = 0; i < numSessionPeers; i++)
			{
				const bdSessionDataRef peerData = m_peers[i];
				bdConnectionRef sessionConnection;

				if(peerData.notNull())
				{
					sessionConnection = peerData->getConnection();
				}

				// is the host also connected to this peer
				bdUInt index = 0;
				if(sessionConnection.notNull() && !hostConnections.findFirst(sessionConnection, index))
				{
					// the host is not connected to this peer add it to the disconnect list
					// dont disconnect here, as doing so will fire callbacks that will
					// change the size of m_peers
					bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
					bdMemset(addrStr, 0, sizeof(addrStr));
					bdAddressMap::addrToString(sessionConnection->getAddressHandle(), addrStr, sizeof(addrStr));
					BD_INFO(BD_LOG_LEVEL, "Connection not known to host disconnecting peer %u: %s.", i, addrStr);

					toDisconnect.pushBack(sessionConnection);
				}
			}

			const bdUInt numDisconnections = toDisconnect.getSize();
			for(bdUInt i = 0; i < numDisconnections; i++)
			{
				const bdConnectionRef connection = toDisconnect[i];
				if(connection.notNull())
				{
					connection->disconnect();
				}
			}
		}
	}
}

void bdSession::processJoinRequest(const bdReceivedMessage& joinRequest)
{
	BD_ASSERT(m_role == BD_SESSION_HOST, "bdSession::processJoinRequest, must be host.");
	BD_ASSERT(m_status == BD_SESSION_READY, "bdSession::processJoinRequest, session not ready.");
	BD_ASSERT(!m_joiningPeer, "bdSession::processJoinRequest, join already in progress.");

	const bdMessageRef message = joinRequest.getMessage();
	const bdConnectionRef connection = joinRequest.getConnection();

	bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
	bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, sizeof(addrStr));
	BD_INFO(BD_LOG_LEVEL, "Processing join request (%s).", addrStr);

	bdBool joinAccepted = true;
	bdBitBuffer userData;

	// determine if we can allow the player to join based on the number of players
	// todo This needs to be re-factored...
	const bdUInt mPeersSize = m_peers.getSize();
	const bdUByte8 numPeers = static_cast<bdUByte8>(mPeersSize);
	if(mPeersSize > numPeers)
	{
		joinAccepted = false;
	}

	if(m_handler && joinAccepted)
	{
		bdBitBufferRef buffer = message->getPayload();
		joinAccepted = m_handler->onSessionConnectRequest(buffer,
														  &userData);
	}

	bdMessageRef joinReply = new bdMessage(BD_SESSION_JOIN_REPLY);
	bdBitBufferRef buffer = joinReply->getPayload();
	buffer->writeBool(joinAccepted);

	if(joinAccepted)
	{
		buffer->writeBits(m_gameSecID.ab, BD_SECURITY_ID_LENGTH << 3);
		buffer->writeBits(m_gameSecKey.ab, BD_SECURITY_KEY_LENGTH << 3);

		// request has been accepted
		buffer->writeUByte8(numPeers);

		BD_INFO(BD_LOG_LEVEL, "Join allowed (%s).", addrStr);
		BD_INFO(BD_LOG_LEVEL, "Existing peers (%u):", numPeers);

		for(bdUInt i=0; i < numPeers; i++)
		{
			bdConnectionRef connection = m_peers[i]->getConnection();
			bdCommonAddrRef addr = connection->getAddress();
			addr->serialize(buffer);

			bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, sizeof(addrStr));
			BD_INFO(BD_LOG_LEVEL, "Peer %u: (%s).", i, addrStr);
		}
		m_joiningPeer = connection;
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "Join denied (%s).", addrStr);
		// request has been denied
		// don't listen for disconnects as we don't care anymore
		connection->unregisterListener(this);

		// write the user data to the message buffer
		//buffer->writeBits(userData.getData(), userData.getWritePosition());
		buffer->append(userData);
	}

	connection->send(joinReply, true);
}

void bdSession::processPendingJoinRequest()
{
	if ((m_role == BD_SESSION_HOST) &&
		!m_joiningPeer &&
		(m_status == BD_SESSION_READY) &&
		m_pendingJoinRequests.getHeadPosition())
	{
		BD_INFO(BD_LOG_LEVEL, "Processing queued join request.");
		const bdReceivedMessage& joinRequest = m_pendingJoinRequests.getHead();
		processJoinRequest(joinRequest);
		m_pendingJoinRequests.removeHead();
	}
}

void bdSession::cleanup()
{
	// close host connection
	if(m_hostConnection.notNull())
	{
		m_hostConnection->unregisterListener(this);
		m_hostConnection->disconnect();
		m_hostConnection = BD_NULL;
	}

	// close local loopback connection
	if(m_localConnection.notNull())
	{
		m_localConnection->unregisterListener(this);
		m_localConnection->disconnect();
		m_localConnection = BD_NULL;
	}

	// close joining peer connection
	if(m_joiningPeer.notNull())
	{
		m_joiningPeer->unregisterListener(this);
		m_joiningPeer->disconnect();
		m_joiningPeer = BD_NULL;
	}

	bdUInt i;

	// close all peer connections
	const bdUInt numPeers = m_peers.getSize();
	for(i = 0; i < numPeers; i++)
	{
		const bdSessionDataRef peerData = m_peers[i];
		const bdConnectionRef connection = peerData->getConnection();
		connection->unregisterListener(this);
		connection->disconnect();
	}

	// drop the references to all connections
	m_peers.clear();

	m_role = BD_SESSION_PEER;
	m_status = BD_SESSION_NOT_CONNECTED;
	m_localPeerIndex = 0;

	// close all pending connections
	const bdUInt numPendingConnections = m_pendingConnections.getSize();
	for(i = 0; i < numPendingConnections; i++)
	{
		bdConnectionRef connection = m_pendingConnections[i];
		connection->unregisterListener(this);
		connection->disconnect();
	}

	// drop the references to all pending connections
	m_pendingConnections.clear();

	// clear any pending joins
	m_pendingJoinRequests.clear();

	// reset the security id
	m_secID = bdSecurityID();

	// reset the security key
	m_secKey = bdSecurityKey();

	// drop the reference to the join request
	m_sessionJoinRequest = BD_NULL;

	// unregister the game security id and key
	if(m_registeredKey)
	{
		bdNet::getInstance()->getSocketRouter()->getKeyMap()->unregisterKey(m_gameSecID);
		m_registeredKey = false;
	}

	// reset game security id
	m_gameSecID = bdSecurityID();

	// reset game security key
	m_gameSecKey = bdSecurityKey();

	// unregister as a dispatch interceptor
	bdNet::getInstance()->getConnectionStore()->unregisterDispatchInterceptor(this);

#ifdef BD_BUFFER_MESSAGES_WHEN_NOT_READY
	// clear buffered messages
	m_messageBuffer.clear();
#endif // BD_BUFFER_MESSAGES_WHEN_NOT_READY

}


void bdSession::cleanupGraceful()
{
	// close host connection
	if(m_hostConnection.notNull())
	{
		m_hostConnection->unregisterListener(this);
		m_hostConnection->disconnect();
	}

	// close local loopback connection
	if(m_localConnection.notNull())
	{
		m_localConnection->unregisterListener(this);
		m_localConnection->disconnect();
	}

	// close joining peer connection
	if(m_joiningPeer.notNull())
	{
		m_joiningPeer->unregisterListener(this);
		m_joiningPeer->disconnect();
	}

	bdUInt i;

	// close all peer connections
	const bdUInt numPeers = m_peers.getSize();
	for(i = 0; i < numPeers; i++)
	{
		const bdSessionDataRef peerData = m_peers[i];
		const bdConnectionRef connection = peerData->getConnection();
		connection->unregisterListener(this);
		connection->disconnect();
	}

	// close all pending connections
	const bdUInt numPendingConnections = m_pendingConnections.getSize();
	for(i = 0; i < numPendingConnections; i++)
	{
		bdConnectionRef connection = m_pendingConnections[i];
		connection->unregisterListener(this);
		connection->disconnect();
	}

	// clear any pending joins
	m_pendingJoinRequests.clear();

	// drop the reference to the join request
	m_sessionJoinRequest = BD_NULL;

	// unregister as a dispatch interceptor
	bdNet::getInstance()->getConnectionStore()->unregisterDispatchInterceptor(this);

#ifdef BD_BUFFER_MESSAGES_WHEN_NOT_READY
	// clear buffered messages
	m_messageBuffer.clear();
#endif // BD_BUFFER_MESSAGES_WHEN_NOT_READY

}

void bdSession::pumpShutdown()
{
	bdBool allShutdown = true;

	// Check if all peer connections are fully disconnected
	for(bdUInt i = 0; i < m_peers.getSize() && allShutdown; i++)
	{		
		const bdConnectionRef connection = m_peers[i]->getConnection();

		if(connection->getStatus() != bdConnection::BD_DISCONNECTED)
		{
			allShutdown = false;
		}
	}

	// Check if all pending connections are fully disconnected
	for(bdUInt i = 0; i < m_pendingConnections.getSize() && allShutdown; i++)
	{		
		if(m_pendingConnections[i]->getStatus() != bdConnection::BD_DISCONNECTED)
		{
			allShutdown = false;
		}
	}

	// Check host connection is fully disconnected
	if(allShutdown && m_hostConnection.notNull() && m_hostConnection->getStatus() != bdConnection::BD_DISCONNECTED)
	{
		allShutdown = false;
	}

	// Check local loopback connection is fully disconnected
	if(allShutdown && m_localConnection.notNull() && m_localConnection->getStatus() != bdConnection::BD_DISCONNECTED)
	{
		allShutdown = false;
	}

	// Check joining peer connection is fully disconnected
	if(allShutdown && m_joiningPeer.notNull() && m_joiningPeer->getStatus() != bdConnection::BD_DISCONNECTED)
	{
		allShutdown = false;
	}

	if(allShutdown){
		// drop the references to all pending connections
		m_pendingConnections.clear();

		// drop the references to all connections
		m_peers.clear();

		// clear host connection
		m_hostConnection = BD_NULL;

		// clear local loopback conection
		m_localConnection = BD_NULL;

		// clear joining peer connection
		m_joiningPeer = BD_NULL;


		BD_INFO(BD_LOG_LEVEL, "All connections disconnected, marking session as not connected.");
		m_status = BD_SESSION_NOT_CONNECTED;
		m_role = BD_SESSION_PEER;
		m_localPeerIndex = 0;

		// reset the security id
		m_secID = bdSecurityID();

		// reset the security key
		m_secKey = bdSecurityKey();

		// reset game security id
		m_gameSecID = bdSecurityID();

		// reset game security key
		m_gameSecKey = bdSecurityKey();

	}
}


#ifdef BD_BUFFER_MESSAGES_WHEN_NOT_READY

bdSession::bdBufferedMessage::bdBufferedMessage()
: m_destination(BD_TO_UNKNOWN),
  m_reliable(false)
{
}

bdSession::bdBufferedMessage::bdBufferedMessage(const bdMessageRef message,
												const bdDestination destination,
												const bdBool reliable)
: m_message(message),
  m_destination(destination),
  m_reliable(reliable)
{
	BD_ASSERT(reliable, "Should not be buffering unreliable messages.");
}

bdSession::bdBufferedMessage::bdDestination bdSession::bdBufferedMessage::getDestination() const
{
	return m_destination;
}

bdMessageRef bdSession::bdBufferedMessage::getMessage() const
{
	return m_message;
}

bdBool bdSession::bdBufferedMessage::getReliable() const
{
	return m_reliable;
}

void bdSession::bdBufferedMessage::addConnection(const bdConnectionRef connection)
{
	// we old want to buffer reliable messages
	if(m_reliable)
	{
		m_sendTo.pushBack(connection);
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Attempted to buffer unreliable message. Message not buffered.");
	}
}

bdArray<bdConnectionRef>& bdSession::bdBufferedMessage::getConnections()
{
	return m_sendTo;
}

bdBool bdSession::addToBuffer(const bdBufferedMessage& message)
{
//	BD_ASSERT(m_status =! BD_SESSION_READY, "Should not be buffering messages "
//		"if the session status is BD_SESSION_READY");

	// is there room in the buffer
	const bdBool addedToBuffer = m_messageBuffer.getSize() < BD_MESSAGE_BUFFER_SIZE;
	if(addedToBuffer)
	{
		m_messageBuffer.pushBack(message);
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "bdSession send buffer is full.");
	}

	return addedToBuffer;
}

void bdSession::sendBufferedMessages()
{
	for(bdUInt i = 0; i < m_messageBuffer.getSize(); i++)
	{
		if(send(m_messageBuffer[i]))
		{
			m_messageBuffer.removeSection(i, i+1);
			i--;
		}
	}
}

bdBool bdSession::send(bdBufferedMessage& message)
{
	const bdBufferedMessage::bdDestination destination = message.getDestination();
	bdBool messagesSent = false;
	switch (destination)
	{
		case bdBufferedMessage::BD_TO_SPECIFIC:
		{
			bdArray<bdConnectionRef>& connections = message.getConnections();
			for(bdUInt i = 0; i < connections.getSize(); i++)
			{
				const bdConnectionRef connection = connections[i];
				const bdBool sent = connection->send(message.getMessage(), message.getReliable());
				if(sent)
				{
					connections.removeAt(i);
					i--;
				}
			}
			messagesSent = (connections.getSize() == 0);
		}
		break;
		case bdBufferedMessage::BD_TO_HOST:
		{
			messagesSent = m_hostConnection->send(message.getMessage(), message.getReliable());
		}
		break;
		default:
		{
			BD_ASSERT(false, "Unknown destination type.");
		}
	}

	return messagesSent;
}

#endif // BD_BUFFER_MESSAGES_WHEN_NOT_READY

bdArray<bdSessionDataRef>& bdSession::getSessionData()
{
	return m_peers;
}

