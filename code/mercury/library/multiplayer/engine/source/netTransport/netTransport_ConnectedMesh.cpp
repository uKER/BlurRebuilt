
#include "stdafx.h"
#include "netDebug\netDebug.h"
#include "netTransport_ConnectedMesh.h"
#include "netPlatform/netPlatform_Core.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif


namespace netTransport
{
	ConnectedMesh::ConnectedMesh(bool online, int maxConnections, U32 titleId, SocketRef socket, unsigned char socketPort)
	: m_socket(socket)
	, m_online(online)
	, m_socketPort(socketPort)
	, m_titleId(titleId)
	, m_sendBpS(16 * 1024)
	, m_recvBpS(16 * 1024)
	, m_maxConnections(maxConnections)
	, m_guidGenerator(ConnGuid(0), ConnGuid(m_maxConnections-1))
	{
		socket->registerHandler(socketPort, this);

		// self and host guid
		m_hostGuid = ConnGuid(CONN_GUID_INVALID);
		m_selfGuid = ConnGuid(CONN_GUID_INVALID);
		
		// state
		m_state = STATE_NOT_SET;
		m_closeReason = CLOSE_REASON_NOT_SET;
	}

	ConnectedMesh::~ConnectedMesh()
	{
		resetConnections();
		m_socket->unregisterHandler(m_socketPort, this);
	}

	bool ConnectedMesh::host(const bdSecurityID& secId, const bdSecurityKey& secKey)
	{
		resetConnections();

		m_guidGenerator.reset();

		m_secId = secId;
		m_secKey = secKey;
		
		if(!Debugging::dbg_useAddressMap)
		{
			if(!netPlatform::Core::instance()->registerKey(m_online, m_secId, m_secKey))
			{
				m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
				return setState(STATE_CLOSED);
			}
		}
		else
		{
			if(!m_socket->addressMap()->registerKey(m_online, m_secId, m_secKey))
			{
				m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
				return setState(STATE_CLOSED);
			}
		}

		m_hostGuid = m_guidGenerator.generate();
		m_selfGuid = m_hostGuid;
		assert(m_hostGuid != ConnGuid(CONN_GUID_INVALID));

		// we've been assigned our guid. signal
		Event_GuidAssignedRef e = new Event_GuidAssigned(m_selfGuid, m_hostGuid);
		m_events.addTail(e);

		m_closeReason = CLOSE_REASON_NOT_SET;
		return setState(STATE_HOSTING);
	}

	bool ConnectedMesh::join(const bdSecurityID& secId, const bdSecurityKey& secKey, bdCommonAddrRef secAddr, netStream* joinParams)
	{
		resetConnections();

		m_guidGenerator.reset();
		
		m_secId = secId;
		m_secKey = secKey;

		if(!Debugging::dbg_useAddressMap)
		{
			if(!netPlatform::Core::instance()->registerKey(m_online, m_secId, m_secKey))
			{
				m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
				return setState(STATE_CLOSED);
			}
		}
		else
		{
			if(!m_socket->addressMap()->registerKey(m_online, m_secId, m_secKey))
			{
				m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
				return setState(STATE_CLOSED);
			}
		}

		m_hostGuid = ConnGuid(CONN_GUID_HOST);
		m_selfGuid = ConnGuid(CONN_GUID_CLIENT);

		m_connector = new Connection(	m_socket, m_socketPort, 
										m_online, m_secId, m_secKey,
										m_selfGuid, m_hostGuid);
		Addr hostAddr;
		hostAddr.fromAddr(secAddr);
		if(!m_connector->connect(hostAddr, joinParams))
		{
			m_connector->destroy();
		}
		m_closeReason = CLOSE_REASON_NOT_SET;
		return setState(STATE_CONNECTING);
	}

	bool ConnectedMesh::setSecurity(const bdSecurityID& secId, const bdSecurityKey& secKey)
	{
		// no change
		if(	netPlatform::Core::instance()->sameSecurityId(secId, m_secId) && 
			netPlatform::Core::instance()->sameSecurityKey(secKey, m_secKey))
		{
			return true;
		}

		if(!Debugging::dbg_useAddressMap)
		{
			// new security registration.
			m_secId = secId;
			m_secKey = secKey;

			// register new id / key pair.
			if(!netPlatform::Core::instance()->registerKey(m_online, m_secId, m_secKey))
			{
				LOGERROR(Debugging::g_logger, "<netTransport::ConnectedMesh::migrate()> failed key registration.");
				return false;
			}
		}
		else
		{
			// remove old key NULL reference
			m_socket->addressMap()->unregisterKey(m_secId, m_secKey);

			// new security registration.
			m_secId = secId;
			m_secKey = secKey;

			// add new key NULL reference
			if(!m_socket->addressMap()->registerKey(m_online, m_secId, m_secKey))
			{
				LOGERROR(Debugging::g_logger, "<netTransport::ConnectedMesh::migrate()> failed key registration.");
				return false;
			}
		}

		// update advertise info.
		if(m_advertiseInfo.notNull())
		{
			m_advertiseInfo->setSecurityID(secId);
			m_advertiseInfo->setSecurityKey(secKey);
		}

		// migrate connections (that haven't got a handle yet) with the new key registration.
		bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
		while(it)
		{
			ConnectionRef conn = m_connections.getValue(it);
			m_connections.next(it);

			// change security credentials if necessary.
			conn->migrate(secId, secKey);
		}
		m_connections.releaseIterator(it);
		return true;				
	}
		
	bool ConnectedMesh::setHost(const ConnGuid& hostGuid)
	{
		if(hostGuid < ConnGuid(CONN_GUID_ASSIGNED_FIRST) || hostGuid > ConnGuid(CONN_GUID_ASSIGNED_LAST))
			return false;

		// no change in host
		if(m_hostGuid == hostGuid)
			return false;

		switch(m_state)
		{
		default:
		case STATE_NOT_SET:
		case STATE_CONNECTING:
			{
				close();
				return false;
			}
		case STATE_HOSTING:
		case STATE_PEERING:
			{
				// new host
				m_hostGuid = hostGuid;

				// we are not the new host
				if(m_hostGuid != m_selfGuid)
				{
					return setState(STATE_PEERING);
				}
				// we are now the new host
				else
				{
					return setState(STATE_HOSTING);
				}
			}
		case STATE_CLOSING:
		case STATE_CLOSED:
			{
				// close handshake with the new host
				m_hostGuid = hostGuid;
				return true;
			}
		}
		return false;
	}

	bool ConnectedMesh::accept(const ConnGuid& clientGuid)
	{
		// only for the host
		if(m_state != STATE_HOSTING)
			return false;

		ConnectionRef conn = findConnection(clientGuid);
		if(conn.isNull()) return false;

		return conn->accept();
	}

	bool ConnectedMesh::close(const ConnGuid& clientGuid, int reason)
	{
		// only for the host. peers cant close connections willfully.
		if(m_state != STATE_HOSTING)
			return false;

		ConnectionRef conn = findConnection(clientGuid);
		if(conn.isNull()) return false;

		return conn->close(reason);
	}

	ConnGuid ConnectedMesh::reserve(const Addr& addr)
	{
		// only for the host. 
		if(!hosting())
			return ConnGuid();

		// find a connection with that address.
		ConnectionRef conn = findConnection(addr);
		if(conn.notNull())
		{
			// connection already in the process of connecting to us
			if(conn->pending() || conn->reserved())
			{
				return conn->guid();
			}

			// need to reset the connection state to reserved.
			// if the link is established, the connection is already reserved.
			if(!conn->established() && !conn->unresponsive())
			{
				conn->destroy();
				conn->reserve(addr);
			}
			return conn->guid();
		}

		// create a new connection to reserve the slot.
		ConnGuid newGuid = m_guidGenerator.generate();

		if(newGuid.isNull())
			return ConnGuid();

		if(findConnection(newGuid))
		{
			LOGFATAL(Debugging::g_logger, "<netTransport::ConnectedMesh::reserve()> already have conn(%d)?!?.", newGuid.m_guid);
			return false;
		}

		conn = new Connection(	m_socket, m_socketPort, 
								m_online, m_secId, m_secKey,
								m_selfGuid, newGuid);

		if(!conn->reserve(addr))
			return ConnGuid();

		m_connections.put(newGuid, conn);
		return newGuid;
	}

	// destroy all connections.		
	void ConnectedMesh::resetConnections()
	{
		m_discoveryServer = NULL;

		// host connection for peers.
		if(m_connector.notNull())
		{
			m_connector->destroy();

			// update connection
			pollConnection(m_connector);

			m_connector = NULL;
		}

		// client / peer connections
		bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
		while(it)
		{
			ConnectionRef conn = m_connections.getValue(it);
			
			conn->destroy();
			
			// update connection
			pollConnection(conn);

			m_connections.remove(it);

			// signal the guid is now history
			m_guidGenerator.release(conn->guid());
		}
		m_connections.releaseIterator(it);
	}

	// close gracefully.
	bool ConnectedMesh::close()
	{
		if(m_state >= STATE_CLOSING)
			return false;
		
		if(m_closeReason == CLOSE_REASON_NOT_SET)
		{
			m_closeReason = CLOSE_REASON_SHUTDOWN;
		}

		bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
		while(it)
		{
			ConnectionRef conn = m_connections.getValue(it);
			m_connections.next(it);
			conn->close(m_closeReason);
		}
		m_connections.releaseIterator(it);
		
		if(m_connector.notNull())
		{
			m_connector->close(m_closeReason);
		}

		setState(STATE_CLOSING);
		return true;
	}

	// stop advertising our presence. 
	// Also, kick all pending connections.
	bool ConnectedMesh::advertise(bdGameInfoRef info)
	{
		m_advertiseInfo = info;
		return true;
	}

	bool ConnectedMesh::getPeers(Peer* peers, int& peerCount, int maxPeers)
	{
		if(!hosting() && !peering())
			return false;

		peerCount = 0;
		
		// ourselves
		peers[peerCount].m_guid  = m_selfGuid;
		peers[peerCount].m_qos   = CONN_QOS_VERY_GOOD;
		peers[peerCount].m_addr  = m_socket->addressMap()->myAddr();
		peers[peerCount].m_state = CONN_STATE_ESTABLISHED;
		peerCount++;

		bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
		while(it)
		{
			ConnectionRef conn = m_connections.getValue(it);
			m_connections.next(it);
			
			if(peerCount >= maxPeers)
			{
				// sanity check.
				assert(false);
				break;
			}
			peers[peerCount].m_guid  = conn->guid();
			peers[peerCount].m_addr  = conn->addr();
			peers[peerCount].m_qos   = conn->qosRating();
			peers[peerCount].m_state = conn->state();
			peerCount++;
		}
		m_connections.releaseIterator(it);

		// sort peers for consistency checks.
		Peer::sortPeers(peers, peerCount);
		return true;
	}
		
	
	// synchronise our connections with a list of peers
	bool ConnectedMesh::syncPeers(const Peer* peers, int peerCount)
	{
		if(m_state != STATE_PEERING)
			return false;

		bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
		while(it)
		{
			ConnectionRef conn = m_connections.getValue(it);
			m_connections.next(it);
	
			if(conn->guid() == m_hostGuid || conn->guid() == m_selfGuid)
				continue;
				
			const Peer* peer = Peer::findPeer(conn->guid(), peers, peerCount);

			// peer not in the list anymore, or not linked. remove him.
			if(!peer || peer->closed() || peer->destroyed())
			{
				// ka-boom!
				conn->destroy();
			}

			// connection closing on host. close locally too.
			if(peer && peer->closing())
			{
				conn->close(CLOSE_REASON_CLOSED_BY_HOST);
			}
		}
		m_connections.releaseIterator(it);

		// register new peers
		for(int i = 0; i < peerCount; i ++)
		{
			// not interested self and current host
			if(peers[i].m_guid == m_hostGuid || peers[i].m_guid == m_selfGuid)
				continue;

			// link not ok. ignore.
			if(!peers[i].established() /*&& !peers[i].unresponsive()*/)
				continue;

			ConnectionRef conn;
			m_connections.get(peers[i].m_guid, conn);

			// we already have a connection with that guid.
			if(conn.notNull())
			{
				// but the address is different? destroy connection.
				// it will get re-established later.
				if(conn->addr() != peers[i].m_addr)
				{
					conn->destroy();
					continue;
				}
			}
			else
			{
				// signal the guid is in use
				m_guidGenerator.reserve(peers[i].m_guid);

				// create a new connection with that guid.
				ConnectionRef newConn = new Connection(	m_socket, m_socketPort, 
														m_online, m_secId, m_secKey,
														m_selfGuid, peers[i].m_guid);
				// put in the list.
				if(newConn->peer(peers[i].m_addr))
				{
					m_connections.put(peers[i].m_guid, newConn);
				}
			}
		}
		return true;
	}

	bool ConnectedMesh::lostHost() 
	{ 
		if(m_state != STATE_PEERING)
			return false;
		
		ConnectionRef hostConn = hostConnection();
		
		if(hostConn.isNull())
			return true;
		
		if(hostConn->closed() || hostConn->destroyed())
			return true;

		if(Debugging::dbg_timeoutEnabled && hostConn->idleTime() > (20 * 1000 * 1000))
			return true; 

		return false;
	}
		
	bool ConnectedMesh::pollConnection(ConnectionRef conn)
	{
		// event notifications
		EventRef e = conn->poll();
		while(e.notNull())
		{
			m_events.addTail(e);
			e = conn->poll();
		}
		conn->flush();
		return true;
	}
				
	// find a connection attached to an address
	ConnectionRef ConnectedMesh::findConnection(const Addr& connectAddr, const netNonce& connectKey)
	{
		bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
		while(it)
		{
			ConnectionRef conn = m_connections.getValue(it);
			m_connections.next(it);

			// same public address.
			if(	conn->addr() == connectAddr)
			{
				// special case for reserved connections.
				if(conn->reserved())
				{
					if(conn->connectKey() != NET_NONCE_NOT_SET)
					{
						LOGERROR(Debugging::g_logger, "<netTransport::ConnectedMesh::findConnection()> connection found and reserved, but connect key is SET.");
					}					
					// same address, connection reserved, that's the one.
					m_connections.releaseIterator(it);
					return conn;
				}	
				// same address, same key, not reserved, that's the one.
				if(conn->connectKey() == connectKey)
				{
					m_connections.releaseIterator(it);
					return conn;
				}
			}
		}
		m_connections.releaseIterator(it);
		return NULL;			
	}
				
	// find a connection attached to an address
	ConnectionRef ConnectedMesh::findConnection(const Addr& addr)
	{
		bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
		while(it)
		{
			ConnectionRef conn = m_connections.getValue(it);
			m_connections.next(it);

			if(conn->addr() == addr)
			{
				m_connections.releaseIterator(it);
				return conn;
			}
		}
		m_connections.releaseIterator(it);
		return NULL;			
	}
		
	// find a connection attached to a guid
	ConnectionRef ConnectedMesh::findConnection(const ConnGuid& guid)
	{
		ConnectionRef conn;
		m_connections.get(guid, conn);
		return conn;
	}

	// number of connections we have.
	int	ConnectedMesh::countConnections()
	{
		return m_connections.getSize();
	}
	
	bool ConnectedMesh::setState(int state)
	{
		// state already set.
		if(state == m_state)
			return false;

		switch(state)
		{
		default:
		case STATE_NOT_SET:
			{
				return false;
			}
		case STATE_CONNECTING:
			{			
				break;
			}
		case STATE_HOSTING:
			{
				m_guidGenerator.reserve(m_selfGuid);

				// this wil restart the discovery server (in case of migration).
				m_discoveryServer = NULL;

				// signal the assigned guids
				EventRef e = new Event_MeshHosting(m_selfGuid);
				m_events.addTail(e);
				break;
			}
		case STATE_PEERING:
			{
				clearRefusedConnection();

				m_guidGenerator.reserve(m_hostGuid);
				m_guidGenerator.reserve(m_selfGuid);
		
				// signal the assigned guids
				EventRef e = new Event_MeshPeering(m_selfGuid, m_hostGuid);
				m_events.addTail(e);
				break;
			}
		case STATE_CLOSING:
			{
				break;
			}
		case STATE_CLOSED:
			{
				// clear connections.
				resetConnections();

				if(!Debugging::dbg_useAddressMap)
				{
					// unregister security key.
					netPlatform::Core::instance()->unregisterKey(m_online, m_secId, m_secKey);
				}
				else
				{
					// remove old key NULL reference.
					m_socket->addressMap()->unregisterKey(m_secId, m_secKey);
				}
		
				// signal
				EventRef e = new Event_MeshClosed(m_closeReason);
				m_events.addTail(e);
				break;
			}
		}

		m_state = state;
		m_stateWatch.start();
		return true;
	}

	bool ConnectedMesh::advertising() const
	{
		if(m_online)
		{
			return m_advertiseInfo.notNull();
		}
		else
		{
			return (m_discoveryServer.notNull() && m_discoveryServer->getStatus() == bdLANDiscoveryServer::BD_PENDING);
		}
	}

	bool ConnectedMesh::updateAdvertise()
	{
		if(m_online)
			return true;

		if(m_advertiseInfo.notNull())
		{
			if(hosting())
			{
				m_advertiseInfo->setHostAddr(netPlatform::Core::instance()->myCommonAddr());
			}
			else
			{
				ConnectionRef host = hostConnection();
				
				if(host.notNull() && host->secAddr().notNull())
				{
					m_advertiseInfo->setHostAddr(host->secAddr());
				}
			}
		}

		bool shouldAdvertise = (m_state == STATE_HOSTING) && m_advertiseInfo.notNull();

		if(advertising())
		{
			m_discoveryServer->update();
		}

		if(shouldAdvertise == advertising())
			return true;

		if(advertising() && !shouldAdvertise)
		{
			m_discoveryServer->stop();

			// advertising stopped. kick pending connections.
			bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
			while(it)
			{
				ConnectionRef conn = m_connections.getValue(it);
				m_connections.next(it);

				if(conn->connecting() || conn->pending() || conn->reserved() || conn->accepting())
				{
					conn->close(CLOSE_REASON_UNAVAILABLE);
				}
			}
			m_connections.releaseIterator(it);
		}

		if(!advertising() && shouldAdvertise)
		{
			bdCommonAddrRef localAddr = netPlatform::Core::instance()->myCommonAddr();
			if(localAddr.isNull())
				return true;

			if(m_discoveryServer == NULL)
				m_discoveryServer = new LANDiscoveryServer();

			m_advertiseInfo->setSecurityID(m_secId);
			m_advertiseInfo->setSecurityKey(m_secKey);
			m_advertiseInfo->setHostAddr(localAddr);
			m_advertiseInfo->setTitleID(m_titleId);
			m_discoveryServer->start(m_advertiseInfo);
		}
		return true;
	}

	bool ConnectedMesh::updateState()
	{
		switch(m_state)
		{
		default:
		case STATE_NOT_SET:
			{
				return false;
			}
		case STATE_CONNECTING:
			{
				// err.... temporary connection to host borked. 
				if(m_connector.isNull() || m_connector->closed() || m_connector->destroyed())
				{
					if(m_connector.notNull())
						m_closeReason = m_connector->closeReason();
					else
						m_closeReason = CLOSE_REASON_UNAVAILABLE;
					
					setState(STATE_CLOSED);
					return false;
				}

				// update connection
				pollConnection(m_connector);

				if(m_connector->established())
				{
					assert(m_connector->guid() == m_hostGuid);
					assert(m_connector->guid() != ConnGuid(CONN_GUID_HOST));
					assert(m_connector->guid() != ConnGuid(CONN_GUID_INVALID));
					assert(!m_connections.containsKey(m_connector->guid()));

					m_connections.put(m_connector->guid(), m_connector);
					m_connector = NULL;

					setState(STATE_PEERING);
					return true;
				}
				return true;
			}
		case STATE_HOSTING:
			{
				updateRefusedConnections();
			
				// update all clients
				bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
				while(it)
				{
					ConnectionRef conn = m_connections.getValue(it);

					// update connection
					pollConnection(conn);

					// established connection timed out.
					// (other states have special timeout values).
					if(Debugging::dbg_timeoutEnabled)
					{
						// connection timed out. give it the boot.
						if((conn->established() || conn->unresponsive()) && conn->idleTime() > (6 * 1000 * 1000))
						{
							conn->close(CLOSE_REASON_NOT_RESPONDING);
						}
					}

					// connection is now dead.
					if(conn->closed() || conn->destroyed())
					{
						// signal the guid is now history
						m_guidGenerator.release(conn->guid());

						// remove connection
						m_connections.remove(it);
					}
					// else, go to next
					else
					{
						m_connections.next(it);
					}
				}
				m_connections.releaseIterator(it);
				return true;
			}
		case STATE_PEERING:
			{
				// collect peer events
				bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
				while(it)
				{
					ConnectionRef conn = m_connections.getValue(it);
				
					// update connection
					pollConnection(conn);

					// connection is now dead.
					// dont remove closed connections, that will be synched with the host.
					// this allows for smoother mogration.
					if(/*conn->closed() ||*/ conn->destroyed())
					{
						// signal the guid is now history
						m_guidGenerator.release(conn->guid());

						// remove connection
						m_connections.remove(it);
					}
					// else, go to next
					else
					{
						m_connections.next(it);
					}
				}
				m_connections.releaseIterator(it);

				// no migration. quit.
				if(lostHost() && !Debugging::dbg_migrationEnabled)
				{
					m_closeReason = CLOSE_REASON_SHUTDOWN;
					setState(STATE_CLOSED);
				}
				return true;
			}
		case STATE_CLOSING:
			{
				if(m_connector.notNull())
				{
					// update connection
					pollConnection(m_connector);
				
					// connection dead.
					if(m_connector->closed() || m_connector->destroyed())
					{
						// signal the guid is now history
						m_guidGenerator.release(m_connector->guid());
						m_connector = NULL;
					}
				}

				// wait for client connections to close (and peers?).
				bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
				while(it)
				{
					ConnectionRef conn = m_connections.getValue(it);

					// update connection
					pollConnection(conn);

					// connection dead.
					if(conn->closed() || conn->destroyed())
					{
						// signal the guid is now history
						m_guidGenerator.release(conn->guid());

						// remove connection
						m_connections.remove(it);
					}
					// check next
					else
					{
						m_connections.next(it);
					}
				}
				m_connections.releaseIterator(it);

				if(m_connections.isEmpty() && m_connector.isNull())
				{
					setState(STATE_CLOSED);
					return true;
				}
				return true;
			}
		case STATE_CLOSED:
			{
				return true;
			}
		}
	}

	bool ConnectedMesh::verifyPacket(ConnPacket* packet)
	{
		if(packet->m_port != m_socketPort)
			return false;

		if(m_state == STATE_NOT_SET)
		{
			return false;
		}

		if(m_state == STATE_CONNECTING)
		{
			if(packet->m_forwarded)
			{
				LOGWARN(Debugging::g_logger, "<netTransport::ConnectedMesh::verifyPacket()> packet forwarded and we are connecting?");
				return false;
			}
			if(m_connector.isNull())
				return false;

			return m_connector->verifyPacket(packet);		
		}

		// we are the target. 
		if(packet->m_destination == m_selfGuid)
		{
			// find out who's been talking to us.
			ConnectionRef target = findConnection(packet->m_origin);
			
			// unknown. dont bother.
			if(target.isNull())
				return false;

			// check packet content.
			return target->verifyPacket(packet);
		}

		// meh. buggered.
		if(packet->m_destination.isNull() || packet->m_origin.isNull())
			return false;
		
		// adressed to unidentified host
		if(packet->m_destination == ConnGuid(CONN_GUID_HOST))
		{
			// should be comming from a unidentified clients
			if(packet->m_origin != ConnGuid(CONN_GUID_CLIENT))
				return false;

			// only if we are hosting, and also accepting connections.
			return hosting() && advertising();
		}

		// reply from host?
		if(packet->m_destination == ConnGuid(CONN_GUID_CLIENT))
			return m_connector.notNull();
		
		// not a valid guid.
		if(!packet->m_destination.assigned() || !packet->m_origin.assigned())
			return false;
		
		// this must be a forwarded packet. 
		if(!packet->m_forwarded)
		{
			LOGWARN(Debugging::g_logger, "<netTransport::ConnectedMesh::verifyPacket()> packet not addressed to us(%d instead of %d), and yet it is not forwarded???", m_selfGuid.m_guid, packet->m_destination.m_guid);
			packet->m_forwarded = true;
		}

		// find the target of the packet.
		ConnectionRef target = findConnection(packet->m_destination);
		
		// target found, but we cant talk to it. No point forwarding the packet.
		if(target.isNull() || target->unresponsive())
			return false;

		// add packet to forward queue.
		target->forwardPacket(packet);
		return false; // dont parse packet messages.
	}

	bool ConnectedMesh::sendMessage(Message* msg, ConnGuid to)
	{
		if(msg->messageId() < MESSAGE_ID_VOICE_FRAGMENTS)
		{
			LOGERROR(Debugging::g_logger, "<netTransport::ConnectedMesh::sendMessage()> can only send custom/voice data.");
			return false;
		}

		switch(m_state)
		{
		default:
			{
				//LOGWARN(Debugging::g_logger, "<netTransport::ConnectedMesh::sendMessage()> not hosting or peering. cant send custom/voice data.");
				return false;
			}
		case STATE_HOSTING:
		case STATE_PEERING:
			{
				if(to.broadcast())
				{
					bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
					while(it)
					{
						ConnectionRef conn = m_connections.getValue(it);
						m_connections.next(it);
						conn->sendMessage(msg);
					}
					m_connections.releaseIterator(it);
					return true;
				}

				ConnectionRef connection = findConnection(to);
				if(connection.isNull())
				{
					LOGERROR(Debugging::g_logger, "<netTransport::ConnectedMesh::sendMessage()> peer(%d) not found.", to.m_guid);
					return false;
				}
				return connection->sendMessage(msg);
			}
		}
		return false;
	}
		
	bool ConnectedMesh::handleMessage(ConnPacket* packet, Message* msg)
	{
		if(packet->m_port != m_socketPort)
			return false;

		switch(msg->messageId())
		{
		case MESSAGE_ID_CONNECT_REQUEST:
			{
				MsgConnectRequest* request = (MsgConnectRequest*)msg;
				
				if(m_state != STATE_HOSTING)
					return addRefusedConnection(request->m_addr, request->m_connectKey, CLOSE_REASON_REFUSED);

				if(packet->m_destination != ConnGuid(CONN_GUID_HOST))
					return addRefusedConnection(request->m_addr, request->m_connectKey, CLOSE_REASON_REFUSED);

				if(packet->m_origin != ConnGuid(CONN_GUID_CLIENT))
					return addRefusedConnection(request->m_addr, request->m_connectKey, CLOSE_REASON_REFUSED);

				RefusedConnectionRef refusedConn = findRefusedConnection(request->m_addr, request->m_connectKey);
				if(refusedConn.notNull()) 
					return true;

				//ConnectionRef conn = findConnection(request->m_addr);
				ConnectionRef conn = findConnection(request->m_addr, request->m_connectKey);

				if(conn.isNull())
				{
					ConnGuid guid = m_guidGenerator.generate();

					if(guid.isNull()) 
						return addRefusedConnection(request->m_addr, request->m_connectKey, CLOSE_REASON_FULL);

					conn = new Connection(m_socket, m_socketPort, m_online, m_secId, m_secKey, m_selfGuid, guid);

					// sanity check
					assert(!m_connections.containsKey(guid));

					m_connections.put(conn->guid(), conn);
				}
				return conn->handleMessage(packet, msg);
			}
		case MESSAGE_ID_CONNECT_ACCEPT:
			{
				if(m_state != STATE_CONNECTING)
					return false;

				if(m_connector.isNull())
					return false;

				if(!m_connector->handleMessage(packet, msg))
					return false;

				m_selfGuid = m_connector->localGuid();
				m_hostGuid = m_connector->remoteGuid();					
				return true;
			}
		case MESSAGE_ID_CONNECT_CLOSING:
			{
				if(m_state == STATE_CONNECTING)
				{
					if(m_connector.notNull())
						return m_connector->handleMessage(packet, msg);

					return false;
				}
				// FALL THROUGH
				// FALL THROUGH
				// FALL THROUGH
			}
		case MESSAGE_ID_CONNECT_PING:
		case MESSAGE_ID_CONNECT_PONG:
		default:
			{
				if(packet->m_destination != m_selfGuid)
					return false;

				if(m_state == STATE_CONNECTING)
				{
					if(m_connector.notNull())
						return m_connector->handleMessage(packet, msg);
				}

				// find the target client of the packet.
				ConnectionRef target = findConnection(packet->m_origin);

				if(target.notNull())
					return target->handleMessage(packet, msg);
				
				// nothing...
				return false;
			}
		}
		return false;
	}

	EventRef ConnectedMesh::poll()
	{
		if(m_events.isEmpty())
		{
			updateState();
			updateAdvertise();
			updateThrottling();
			}
		
		if(m_events.isEmpty())
			return NULL;

		EventRef e = m_events.getHead();
		m_events.removeHead();
		return e;
	}	

	bool ConnectedMesh::getLatency(float& latency, int& latencyCount)
	{
		latency = 0.0f;
		latencyCount = 0;

		bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
		while(it)
		{
			ConnectionRef conn = m_connections.getValue(it);
			m_connections.next(it);
			latency += conn->getLatency();
			latencyCount++;
		}
		m_connections.releaseIterator(it);
		return true;
	}

	void ConnectedMesh::getInboundTraffic(TrafficSample& report)
	{
		report = TrafficSample();
		
		bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
		while(it)
		{
			ConnectionRef conn = m_connections.getValue(it);
			m_connections.next(it);

			TrafficSample s;
			conn->getInboundTraffic(s);
			report += s;
		}
		m_connections.releaseIterator(it);

		// normalise connection reports into bytes / sec.
		report.normalise();
	}
	
	void ConnectedMesh::getOutboundTraffic(TrafficSample& report)
	{
		report = TrafficSample();

		bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
		while(it)
		{
			ConnectionRef conn = m_connections.getValue(it);
			m_connections.next(it);

			TrafficSample s;
			conn->getOutboundTraffic(s);
			report += s;
		}
		m_connections.releaseIterator(it);

		// normalise connection reports into bytes / sec.
		report.normalise();
	}
	
	bool ConnectedMesh::setForwarder(const ConnGuid& peer, const ConnGuid& forwarder)
	{
		ConnectionRef peerconn = findConnection(peer);
		ConnectionRef forwarderconn = findConnection(forwarder);
		
		if(peerconn.isNull())
			return false;

		if(forwarder == peer)
		{
			LOGERROR(Debugging::g_logger, "<netTransport::ConnectedMesh::setForwarder()> forwarder is target.");
			peerconn->setForwardHandle(NULL);
			return false;
		}

		if(forwarder == m_selfGuid)
		{
			LOGERROR(Debugging::g_logger, "<netTransport::ConnectedMesh::setForwarder()> forwarder is self.");
			peerconn->setForwardHandle(NULL);
			return false;
		}

		if(forwarderconn.isNull())
		{
			peerconn->setForwardHandle(NULL);
			return false;
		}

		if(!forwarderconn->established())
		{
			LOGERROR(Debugging::g_logger, "<netTransport::ConnectedMesh::setForwarder()> forwarder connection not established.");
			peerconn->setForwardHandle(NULL);
			return false;
		}

		bdAddrHandleRef handle = forwarderconn->getAddrHandle();

		int	addrStatus = netPlatform::Core::instance()->getHandleStatus(handle);
		
		if(addrStatus != netPlatform::ADDRESS_STATUS_CONNECTED)
		{
			LOGERROR(Debugging::g_logger, "<netTransport::ConnectedMesh::setForwarder()> forwarder handle not in CONNECTED state.");
			peerconn->setForwardHandle(NULL);
			return false;
		}

		peerconn->setForwardHandle(handle);
		return true;
	}

	bool ConnectedMesh::updateThrottling()
	{
		int count = countConnections();

		bdHashMap<int, ConnectionRef>::Iterator it = m_connections.getIterator();
		while(it)
		{
			ConnectionRef conn = m_connections.getValue(it);
			m_connections.next(it);
			conn->setRates(m_sendBpS / count, m_recvBpS / count);
		}
		m_connections.releaseIterator(it);
		return true;
	}

	bool ConnectedMesh::setRates(float sendBpS, float recvBpS)
	{
		// minimum acceptable!
		m_sendBpS = sendBpS;
		m_recvBpS = recvBpS;
		return true;
	}


	BD_REFERENCE(RefusedConnection);
	class RefusedConnection: public bdReferencable
	{
	public:
		RefusedConnection(SocketRef socket, unsigned char socketPort,
						  bool online, const bdSecurityID& secId, const bdSecurityKey& secKey,
						  const Addr& connAddr, const netNonce& connKey, int reason)
		{
			m_socket = socket;
			m_socketPort = socketPort;
			m_online = online;
			m_secId = secId;
			m_secKey = secKey;
			m_connAddr = connAddr;
			m_connKey = connKey;
			m_reason = reason;
			m_msgCount = 0;
			m_secAddr = new bdCommonAddr();
			m_connAddr.toAddr(m_secAddr);

			if(!Debugging::dbg_useAddressMap)
			{
				// register address
				m_connHandle = netPlatform::Core::instance()->registerAddress(m_secAddr, m_secId);
			}
			else
			{
				// register address
				m_connHandle = m_socket->addressMap()->registerAddress(m_online, m_connAddr, m_secId, m_secKey);
			}
		}

		bool update()
		{
			if(m_msgCount > 10)
				return false;

			if(!m_msgTick.ticked(1 * 1000 * 1000))
				return true;

			MsgConnectClosing msg;
			msg.m_closeReason = m_reason;
			
			m_packet.reset();
			m_packet.writeMessage(&msg);
			m_packet.m_port = m_socketPort;
			m_packet.m_origin = ConnGuid(CONN_GUID_HOST);
			m_packet.m_destination = ConnGuid(CONN_GUID_CLIENT);
			m_packet.m_forwarded = false;
			m_packet.m_address = m_connHandle;
			m_socket->sendPacket(&m_packet);
			m_msgCount++;
			return true;
		}

		~RefusedConnection()
		{
			// unregister address
			if(!Debugging::dbg_useAddressMap)
			{
				netPlatform::Core::instance()->unregisterAddress(m_connHandle);
			}
			else
			{
				m_socket->addressMap()->unregisterAddress(m_connHandle);
			}
			m_connHandle = NULL;
		}
	public:
		int	m_reason;
		netStopwatch m_msgTick;
		int m_msgCount;
		
		SocketRef m_socket;
		unsigned char m_socketPort;
		bool m_online;
		bdSecurityID m_secId;
		bdSecurityKey m_secKey;
		bdCommonAddrRef m_secAddr;
		bdAddrHandleRef	m_connHandle;
		Addr m_connAddr;
		netNonce m_connKey;
		ConnPacket m_packet;
	};

	RefusedConnectionRef ConnectedMesh::findRefusedConnection(const Addr& connectAddr, const netNonce& connectKey)
	{
		// update refused connections.
		bdLinkedList<RefusedConnectionRef>::Position pos = m_refused.getHeadPosition();
		while(pos)
		{
			RefusedConnectionRef conn = m_refused.forward(pos);

			// same public address, same connection key.
			if(	conn->m_connAddr == connectAddr && conn->m_connKey == connectKey)
				return conn;
		}
		return NULL;
	}

	bool ConnectedMesh::addRefusedConnection(const Addr& connectAddr, const netNonce& connectKey, int reason)
	{
		RefusedConnectionRef conn = findRefusedConnection(connectAddr, connectKey);
		if(conn.notNull()) return false;
		if(m_refused.getSize() > 8) return false;

		conn = new RefusedConnection(m_socket, m_socketPort, 
									 m_online, m_secId, m_secKey,
									 connectAddr, connectKey, reason);
		
		m_refused.addTail(conn);
		return true;
	}

	bool ConnectedMesh::clearRefusedConnection()
	{
		m_refused.clear();
		return true;
	}

	bool ConnectedMesh::updateRefusedConnections()
	{
		// update refused connections.
		bdLinkedList<RefusedConnectionRef>::Position pos = m_refused.getHeadPosition();
		while(pos)
		{
			RefusedConnectionRef conn = m_refused.getAt(pos);
			
			if(!conn->update())
			{
				m_refused.removeAt(pos);
			}
			else
			{
				m_refused.forward(pos);
			}
		}
		return true;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

