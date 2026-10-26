
#include "stdafx.h"
#include "netTransport_Connection.h"
#include "netPlatform/netPlatform_Core.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{
	Connection::Connection(	SocketRef socket, unsigned char socketPort,
							bool online, const bdSecurityID& secId, const bdSecurityKey& secKey,
							ConnGuid localGuid, ConnGuid remoteGuid)
	: m_socket(socket)
	, m_socketPort(socketPort)
	, m_state(CONN_STATE_NOT_SET)
	, m_closeReason(CLOSE_REASON_NOT_SET)
	, m_localGuid(localGuid)
	, m_remoteGuid(remoteGuid)
	, m_secKey(secKey)
	, m_secId(secId)
	, m_online(online)
	, m_sendBpS(4.0f * 1024.0f) // 4 K throttle
	, m_recvBpS(4.0f * 1024.0f) // 4 K throttle
	, m_outstreamLogger(new TrafficLogger())
	, m_instreamLogger(new TrafficLogger())
	, m_latency(0.0f)
	, m_localQoS(0.0f)
	, m_remoteQoS(0.0f)
	, m_qosRating(CONN_QOS_UNAVAILABLE)
	{}

	Connection::~Connection()
	{
		setState(CONN_STATE_CLOSED);
	}

	bdAddrHandleRef	Connection::getAddrHandle() 
	{ 
		return m_secHandle; 
	}

	void Connection::setForwardHandle(bdAddrHandleRef forwardHandle)
	{
		m_forwardHandle = forwardHandle;
	}

	bdCommonAddrRef Connection::secAddr()
	{
		return m_secAddr;
	}

	bool Connection::reconnect()
	{
		if(m_secAddr.isNull())
			return false;

		char buff[128];
		netPlatform::Core::instance()->getAddrInfo(m_secAddr, buff, sizeof(buff));
		LOGWARN(Debugging::g_logger, "<Connection::reconnect()> attempting reconnection with conn(%d) addr(%s)", m_remoteGuid.m_guid, buff);

		if(!Debugging::dbg_useAddressMap)
		{
			// unregister address
			netPlatform::Core::instance()->unregisterAddress(m_secHandle);
			m_secHandle = NULL;
			
			// re-register address
			m_secHandle = netPlatform::Core::instance()->registerAddress(m_secAddr, m_secId);

			// re-connect address
			if(m_remoteGuid > m_localGuid)
			{
				// connect address handle
				if(!netPlatform::Core::instance()->connectAddress(m_secAddr, m_secHandle))
				{
					LOGERROR(Debugging::g_logger, "<Connection::connect> failed to register host handle!");
					return false;
				}		
			}
		}
		else
		{
			// register address
			m_socket->addressMap()->unregisterAddress(m_secHandle);
			m_secHandle = NULL;

			// re-register address
			m_secHandle = m_socket->addressMap()->registerAddress(m_online, m_connectAddr, m_secId, m_secKey);

			// re-connect address
			if(m_remoteGuid > m_localGuid)
			{
				if(!m_socket->addressMap()->connectAddress(m_secHandle))
				{
					LOGERROR(Debugging::g_logger, "<Connection::connect> failed to register host handle!");
					return false;
				}
			}
		}
		return true;
	}

	bool Connection::migrate(const bdSecurityID& secId, const bdSecurityKey& secKey)
	{
		if(m_secHandle.notNull())
		{
			//netDebug::logWarn(netDebug::g_TransportLog, "<Connection::migrate()> handle already set.");
		}

		m_secId = secId;
		m_secKey = secKey;
		return true;
	}
		
	bool Connection::peer(const Addr& addr)
	{
		destroy();

		m_connectAddr = addr;
		m_secAddr = new bdCommonAddr;
		m_connectAddr.toAddr(m_secAddr);
		
		if(!Debugging::dbg_useAddressMap)
		{
			// register address
			m_secHandle = netPlatform::Core::instance()->registerAddress(m_secAddr, m_secId);
		}
		else
		{
			// register address
			m_secHandle = m_socket->addressMap()->registerAddress(m_online, m_connectAddr, m_secId, m_secKey);
		}

		// buggered
		if(m_secHandle.isNull())
		{
			LOGERROR(Debugging::g_logger, "<Connection::peer> failed to register peer handle!");
			m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
			setState(CONN_STATE_CLOSED);
			return false;
		}

		// connect address (one-way).
		if(m_remoteGuid > m_localGuid)
		{
			if(!Debugging::dbg_useAddressMap)
			{
				// connect address handle
				if(!netPlatform::Core::instance()->connectAddress(m_secAddr, m_secHandle))
				{
					LOGERROR(Debugging::g_logger, "<Connection::connect> failed to register host handle!");
					m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
					setState(CONN_STATE_CLOSED);
					return false;
				}		
			}
			else
			{
				if(!m_socket->addressMap()->connectAddress(m_secHandle))
				{
					LOGERROR(Debugging::g_logger, "<Connection::connect> failed to register host handle!");
					m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
					setState(CONN_STATE_CLOSED);
					return false;
				}
			}
		}

		m_closeReason = CLOSE_REASON_NOT_SET;
		return setState(CONN_STATE_ESTABLISHED);
	}

	bool Connection::connect(const Addr& addr, netStream* joinParams)
	{
		destroy();

		// connection already hooked to something?!?
		if(m_state != CONN_STATE_NOT_SET)
			return false;

		m_connectKey = netPlatform::Core::instance()->generateNonce();
		m_connectAddr = addr;
		m_localGuid = ConnGuid(CONN_GUID_CLIENT);
		m_remoteGuid = ConnGuid(CONN_GUID_HOST);
	
		m_secAddr = new bdCommonAddr;
		m_connectAddr.toAddr(m_secAddr);

		if(!Debugging::dbg_useAddressMap)
		{
			// register address
			m_secHandle = netPlatform::Core::instance()->registerAddress(m_secAddr, m_secId);

			// buggered
			if(m_secHandle.isNull())
			{
				LOGERROR(Debugging::g_logger, "<Connection::connect> failed to register host handle!");
				m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
				setState(CONN_STATE_CLOSED);
				return false;
			}

			// connect address handle
			if(!netPlatform::Core::instance()->connectAddress(m_secAddr, m_secHandle))
			{
				LOGERROR(Debugging::g_logger, "<Connection::connect> failed to register host handle!");
				m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
				setState(CONN_STATE_CLOSED);
				return false;
			}
		}
		else
		{
			m_secHandle = m_socket->addressMap()->registerAddress(m_online, m_connectAddr, m_secId, m_secKey);

			// buggered
			if(m_secHandle.isNull())
			{
				LOGERROR(Debugging::g_logger, "<Connection::connect> failed to register host handle!");
				m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
				setState(CONN_STATE_CLOSED);
				return false;
			}

			// connect address handle
			if(!m_socket->addressMap()->connectAddress(m_secHandle))
			{
				LOGERROR(Debugging::g_logger, "<Connection::connect> failed to register host handle!");
				m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
				setState(CONN_STATE_CLOSED);
				return false;
			}
		}
	
		m_cachedPacket.reset();

		// set join params
		m_joinParams.clear();
		
		if(joinParams != NULL)
			m_joinParams.copy(joinParams);
			
		if(m_joinParams.getWriteByteCount() > MsgConnectRequest::MAX_PAYLOAD)
		{
			LOGERROR(Debugging::g_logger, "<Connection::connect> join data too large!");
			m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
			setState(CONN_STATE_CLOSED);
			return false;
		}

		// send connect requests
		m_closeReason = CLOSE_REASON_NOT_SET;
		return setState(CONN_STATE_CONNECTING);
	}

	bool Connection::accept()
	{
		// connection not waiting for accept.
		if(m_state != CONN_STATE_PENDING)
			return false;

		m_secAddr = new bdCommonAddr;
		m_connectAddr.toAddr(m_secAddr);


		if(!Debugging::dbg_useAddressMap)
		{
			// register address
			m_secHandle = netPlatform::Core::instance()->registerAddress(m_secAddr, m_secId);
		}
		else
		{
			// register address
			m_secHandle = m_socket->addressMap()->registerAddress(m_online, m_connectAddr, m_secId, m_secKey);
		}

		// buggered
		if(m_secHandle.isNull())
		{
			LOGERROR(Debugging::g_logger, "<Connection::accept()> failed to register client handle!");
			m_closeReason = CLOSE_REASON_SYSTEM_ERROR;
			setState(CONN_STATE_CLOSED);
			return false;
		}

		// send accept message, wait for ping
		m_closeReason = CLOSE_REASON_NOT_SET;
		setState(CONN_STATE_ACCEPTING);
		return true;
	}

	bool Connection::reserve(const Addr& addr)
	{
		destroy();

		m_connectAddr = addr;
		m_connectKey = NET_NONCE_NOT_SET;
				
		// send accept message, wait for ping
		m_closeReason = CLOSE_REASON_NOT_SET;
		return setState(CONN_STATE_RESERVED);
	}

	bool Connection::destroy()
	{
		//m_events.clear();
		m_closeReason = CLOSE_REASON_RESET;
		setState(CONN_STATE_CLOSED);
		setState(CONN_STATE_NOT_SET);
		return true;
	}
		
	bool Connection::close(int reason)
	{
		switch(m_state)
		{
		default:
		case CONN_STATE_RESERVED:
			{
				m_closeReason = reason;
				return setState(CONN_STATE_CLOSED);
			}
		case CONN_STATE_CONNECTING:
		case CONN_STATE_PENDING:
		case CONN_STATE_ACCEPTING:
		case CONN_STATE_ESTABLISHED:
		case CONN_STATE_UNRESPONSIVE:
			{
				m_closeReason = reason;
				return setState(CONN_STATE_CLOSING);
			}
		case CONN_STATE_NOT_SET:
		case CONN_STATE_CLOSING:
		case CONN_STATE_CLOSED:
			{
				// already closed.
				return false;
			}
		}
	}

	bool Connection::updateQoS()
	{
		// check the current upstream load. 
		float currentLoad = m_sendStats.m_bytes[TRAFFIC_TYPE_SOCKET];
		float limit = m_sendBpS;

		// overload (< 0.0f) or spare bandwidth (> 0.0f) in kilobytes. 
		m_localQoS = (limit - currentLoad) / 1024.0f; 

		// setup the rating (only update slowly).
		if(m_qosRatingWatch.ticked(5 * 1024 * 1024))
		{	
			if(!established())
				m_qosRating = CONN_QOS_UNAVAILABLE;

			// check if we must throttle the packet to forward
			m_qosRating = (int)((float)floor(m_localQoS));

			if(m_qosRating < CONN_QOS_VERY_BAD)
				m_qosRating = CONN_QOS_VERY_BAD;

			if(m_qosRating > CONN_QOS_VERY_GOOD)
				m_qosRating = CONN_QOS_VERY_GOOD;
		}
		return true;
	}
		
	bool Connection::updatePing()
	{
		if(!m_msgWatch.started() || m_msgWatch.elapsed(1 * 1000 * 1000))
		{
			updateQoS();

			U64 timestamp = netClock::instance()->now();
			
			MsgConnectPing msg;
			msg.m_timestamp = timestamp;
			msg.m_qos = m_localQoS;

			if(sendMessage(&msg))
			{
				startPing(timestamp);
				m_msgWatch.start();
				return true;
			}
		}
		return false;
	}

	bool Connection::updateState()
	{
		switch(m_state)
		{
		default:
		case CONN_STATE_NOT_SET:
			{
				break;
			}
		case CONN_STATE_CONNECTING:
			{
				if(Debugging::dbg_timeoutEnabled && m_idleWatch.elapsed(10 * 1000 * 1000))
				{
					LOGERROR(Debugging::g_logger, "<Connection::updateState()> host did not respond to our connection request.");
					close(CLOSE_REASON_NOT_RESPONDING);
					break;
				}
				
				if(!m_msgWatch.started() || m_msgWatch.elapsed(500 * 1000))
				{
					MsgConnectRequest msg;
					msg.m_connectKey = m_connectKey;
					msg.m_addr = m_socket->addressMap()->myAddr();
					msg.m_payloadSize = m_joinParams.getWriteByteCount();
					
					if(msg.m_payloadSize > msg.MAX_PAYLOAD)
					{
						LOGERROR(Debugging::g_logger, "<Connection::updateState()> join params too large.");
						break;
					}
					m_joinParams.setReadByteCount(0);
					m_joinParams.readBytes(msg.m_payload, msg.m_payloadSize);
															
					if(sendMessage(&msg))
						m_msgWatch.start();
				}
				break;
			}
		case CONN_STATE_RESERVED:
			{
				if(Debugging::dbg_timeoutEnabled && m_stateWatch.elapsed(60 * 1000 * 1000))
				{
					LOGERROR(Debugging::g_logger, "<Connection::updateState()> reserved connection timed out.");
					close(CLOSE_REASON_KICKED);
					break;
				}
				break;
			}
		case CONN_STATE_PENDING:
			{
				if(Debugging::dbg_timeoutEnabled && m_stateWatch.elapsed(30 * 1000 * 1000))
				{
					LOGERROR(Debugging::g_logger, "<Connection::updateState()> connection being pending for too long.");
					close(CLOSE_REASON_KICKED);
					break;
				}
				break;
			}
		case CONN_STATE_ACCEPTING:
			{
				if(Debugging::dbg_timeoutEnabled && m_stateWatch.elapsed(6 * 1000 * 1000))
				{
					LOGERROR(Debugging::g_logger, "<Connection::updateState()> connection being pending for too long.");
					close(CLOSE_REASON_KICKED);
					break;
				}

				if(!m_msgWatch.started() || m_msgWatch.elapsed(1000 * 1000))
				{
					MsgConnectAccepted msg;
					msg.m_connectKey = m_connectKey;
					msg.m_assignedGuid = m_remoteGuid;
					msg.m_hostGuid = m_localGuid;
					msg.m_payloadSize = 0;
					
					if(sendMessage(&msg))
						m_msgWatch.start();
				}
				break;
			}
		case CONN_STATE_UNRESPONSIVE:
			{
				updatePing();

				// peer not responding?!?
				if(m_idleWatch.elapsedTime() < (3 * 1000 * 1000))
				{
					setState(CONN_STATE_ESTABLISHED);
				}

				// 5 minute unresponsive. retry NAT punch-through (dont want to forward packets forever).
				if(!m_reconnectWatch.started() || m_reconnectWatch.elapsedTime() > (5 * 60 * 1000 * 1000))
				{
					reconnect();
					m_reconnectWatch.start();
				}
				break;
			}
		case CONN_STATE_ESTABLISHED:
			{
				updatePing();
		
				// peer not responding?!?
				if(m_idleWatch.elapsedTime() > (10 * 1000 * 1000))
				{
					if(netTransport::Debugging::dbg_timeoutEnabled)
						setState(CONN_STATE_UNRESPONSIVE);
				}
				break;
			}
		case CONN_STATE_CLOSING:
			{
				// how often we send close messages
				int closeTick = (500 * 1000);
				
				int	addrStatus = netPlatform::Core::instance()->getHandleStatus(m_secHandle);
				bool handleClosed = (addrStatus == netPlatform::ADDRESS_STATUS_LOST);

				// limit on the number of close messages received, after which we can 
				// just kill the connection.
				int recvCloseLimit = 5;
				if (handleClosed || (m_recvCloseCount >= recvCloseLimit))
				{
					setState(CONN_STATE_CLOSED);
					break;
				}
				
				// timeout. The more close messages we receive, the faster we time out.
				int closeTimeout = (closeTick + closeTick / 2) * (recvCloseLimit - m_recvCloseCount);

				// closing timeout.
				if(/*netTransport::Debugging::dbg_timeoutEnabled &&*/ m_stateWatch.elapsed(closeTimeout))
				{
					setState(CONN_STATE_CLOSED);
					break;
				}
				
				if(!m_msgWatch.started() || m_msgWatch.elapsed(closeTick))
				{
					MsgConnectClosing msg;
					msg.m_closeReason = m_closeReason;
					
					if(sendMessage(&msg))
						m_msgWatch.start();
				}
				break;
			}
		case CONN_STATE_CLOSED:
			{
				break;
			}
		}
		return true;
	}

	bool Connection::setState(int state)
	{
		if(state == m_state)
			return false;

		m_msgWatch.start();
		m_stateWatch.start();
		m_state = state;
				
		switch(m_state)
		{
		default:
		case CONN_STATE_NOT_SET:
			{
				break;
			}
		case CONN_STATE_CONNECTING:
			{
				m_idleWatch.start();
				break;
			}
		case CONN_STATE_RESERVED:
			{
				m_idleWatch.start();
				break;
			}
		case CONN_STATE_PENDING:
			{
				m_idleWatch.start();
				
				EventRef e = new Event_ConnPending(guid(), &m_joinParams);
				m_events.addTail(e);
				break;
			}
		case CONN_STATE_ACCEPTING:
			{
				m_idleWatch.start();
				break;
			}
		case CONN_STATE_UNRESPONSIVE:
			{
				// connection not responding?
				m_reconnectWatch.start();
				EventRef e = new Event_ConnUnresponsive(guid());
				m_events.addTail(e);
				break;
			}
		case CONN_STATE_ESTABLISHED:
			{
				m_idleWatch.start();
				
				if(m_state != CONN_STATE_UNRESPONSIVE)
				{					
					EventRef e = new Event_ConnEstablished(guid());
					m_events.addTail(e);
				}
				else
				{
					// send connection recovered?!?
					EventRef e = new Event_ConnRecovered(guid());
					m_events.addTail(e);
				}
				break;
			}
		case CONN_STATE_CLOSING:
			{
				m_recvCloseCount = 0;

				// need handle to be able to send messages
				if(m_secHandle.isNull())
				{
					if(!Debugging::dbg_useAddressMap)
					{
						// register address
						m_secHandle = netPlatform::Core::instance()->registerAddress(m_secAddr, m_secId);
					}
					else
					{
						// register address
						m_secHandle = m_socket->addressMap()->registerAddress(m_online, m_connectAddr, m_secId, m_secKey);
					}
					//LOGWARN(Debugging::g_logger, "<Connection::peer> no address handle!");
					//setState(CONN_STATE_CLOSED);
					//return false;
				}
				break;
			}
		case CONN_STATE_CLOSED:
			{ 
				// unregister address.
				if(m_secHandle.notNull())
				{
					if(!Debugging::dbg_useAddressMap)
					{
						if(!netPlatform::Core::instance()->unregisterAddress(m_secHandle))
						{
							LOGERROR(Debugging::g_logger, "<Connection::setState()> failed to unregister address handle!");
						}
					}
					else
					{
						// unregister address.
						if(!m_socket->addressMap()->unregisterAddress(m_secHandle))
						{
							LOGERROR(Debugging::g_logger, "<Connection::setState()> failed to unregister address handle!");
						}
					}
					m_secHandle = NULL;
				}
				
				// close signal (if we aren't already closed).
				if(m_state > CONN_STATE_NOT_SET && m_state < CONN_STATE_CLOSED)
				{
					EventRef e = new Event_ConnClosed(guid(), m_closeReason);
					m_events.addTail(e);
				}
				break;
			}
		}
		return true;
	}

	EventRef Connection::poll()
	{
		if(m_events.isEmpty())
		{
			m_latency = calculateLatency();

			if(m_trafficWatch.ticked(50 * 1000))
			{
				m_outstreamLogger->poll();
				m_instreamLogger->poll();
		
				m_outstreamLogger->getTraffic((1000 * 1000), m_sendStats);
				m_instreamLogger->getTraffic((1000 * 1000), m_recvStats);
			}
	
			updateState();
		}
		
		if(m_events.isEmpty())
			return NULL;
		
		EventRef e = m_events.getHead();
		m_events.removeHead();

		return e;
	}

	const Connection::Latency* Connection::startPing(U64 timestamp)
	{
		Latency* oldest = &m_latencies[0];

		for(int i = 1; i < MAX_LATENCIES; i ++)
		{
			if(m_latencies[i].m_pingTime < oldest->m_pingTime)
			{
				oldest = &(m_latencies[i]);
			}
		}

		if((oldest->m_pongTime == 0) && (oldest->m_pingTime != 0))
		{
			F32 elapsed = (F32)(netClock::instance()->now() - oldest->m_pingTime) / (1000.0f * 1000.0f);
		
			if(Debugging::dbg_ping && (Debugging::dbg_pingId == m_remoteGuid.m_guid || Debugging::dbg_pingId < 0))
			{
				LOGWARN(Debugging::g_logger, "<Connection(%d)> ping(%llu) lost (%.2f seconds ago)...", m_remoteGuid.m_guid, oldest->m_pingTime, elapsed);
			}
		}

		if(Debugging::dbg_ping && (Debugging::dbg_pingId == m_remoteGuid.m_guid || Debugging::dbg_pingId < 0))
		{
			LOGINFO(Debugging::g_logger, "<Connection(%d)> started ping(%llu)...", m_remoteGuid.m_guid, timestamp);
		}
						
		oldest->m_pingTime = timestamp;
		oldest->m_pongTime = 0;
		return oldest;
	}

	const Connection::Latency* Connection::stopPing(U64 timestamp)
	{
		U64 now = netClock::instance()->now();
		
		if(timestamp > now)
			return NULL;

		for(int i = 1; i < MAX_LATENCIES; i ++)
		{
			if(m_latencies[i].m_pingTime == timestamp)
			{
				m_latencies[i].m_pongTime = now;
				
				if(Debugging::dbg_ping && (Debugging::dbg_pingId == m_remoteGuid.m_guid || Debugging::dbg_pingId < 0))
				{
					LOGINFO(Debugging::g_logger, "<Connection(%d)> new ping(%f secs)...", m_remoteGuid.m_guid, (float)((m_latencies[i].m_pongTime - m_latencies[i].m_pingTime) / (1000.0f * 1000.0f)));
				}			
				return &(m_latencies[i]);
			}
		}

		if(Debugging::dbg_ping && (Debugging::dbg_pingId == m_remoteGuid.m_guid || Debugging::dbg_pingId < 0))
		{
			F32 elapsed = (F32)(netClock::instance()->now() - timestamp) / (1000.0f * 1000.0f);
		
			LOGINFO(Debugging::g_logger, "<Connection(%d)> pong(%llu) not matched (%.2f secs ago)...", m_remoteGuid.m_guid, timestamp, elapsed);
		}		
		return NULL;
	}

	F32 Connection::calculateLatency()
	{
		U64 latency = 0;
		U64 latencycount = 0;

		for(int i = 1; i < MAX_LATENCIES; i ++)
		{
			if(m_latencies[i].m_pongTime < m_latencies[i].m_pingTime)
				continue;

			latency += (m_latencies[i].m_pongTime - m_latencies[i].m_pingTime);
			latencycount++;
		}

		if(latencycount == 0)
			return 16.0f;
		
		F32 latencySecs = (F32)((F64)latency / (F64)(latencycount * 1000 * 1000));
		return latencySecs;
	}
		
	void Connection::flush()
	{
		sendPacket();
	}

	bool Connection::handleMessage(ConnPacket* packet, Message* msg)
	{
		if(!handleMessageInternal(packet, msg))
		{
			return false;
		}

		// packet not forwarded. So the address it
		// came from is indeed our guy on the other side. 
		// update his address.
		if(!packet->m_forwarded)
		{
			// update the secure address
			if(!Debugging::dbg_useAddressMap)
			{
				m_secHandle = packet->m_address;
			}
			else
			{
				m_secHandle = m_socket->addressMap()->updateAddress(m_secHandle, packet->m_address);
			}
		}
		return true;
	}
	
	bool Connection::handleMessageInternal(ConnPacket* packet, Message* msg)
	{
		if(packet->m_port != m_socketPort)
			return false;

		// handle message
		switch(msg->messageId())
		{
		case MESSAGE_ID_CONNECT_REQUEST:
			{
				MsgConnectRequest* request = (MsgConnectRequest*)msg;
					
				if(packet->m_origin.m_guid != CONN_GUID_CLIENT || packet->m_destination.m_guid != CONN_GUID_HOST)
					return false;

				// special reserved slot
				if(m_state == CONN_STATE_RESERVED)
				{
					if(m_connectAddr != request->m_addr)
					{
						LOGERROR(Debugging::g_logger, "<Connection::handleMessage()> mismatch connection request address.");
						return false;
					}
				}
				else if(m_state != CONN_STATE_NOT_SET)
				{
					return false;
				}

				// reset idle watch
				m_idleWatch.start();

				m_connectAddr = request->m_addr;
				m_connectKey = request->m_connectKey;
				m_joinParams.clear();
				m_joinParams.writeBytes(request->m_payload, request->m_payloadSize);
				setState(CONN_STATE_PENDING);
				return true;
			}
		case MESSAGE_ID_CONNECT_ACCEPT:
			{
				MsgConnectAccepted* accepted = (MsgConnectAccepted*)msg;

				if(m_state != CONN_STATE_CONNECTING)
					return false;

				if(!netPlatform::Core::instance()->sameNonce(accepted->m_connectKey, m_connectKey))
					return false;

				// reset idle watch
				m_idleWatch.start();

				m_localGuid = accepted->m_assignedGuid;
				m_remoteGuid = accepted->m_hostGuid;
				m_cachedPacket.reset();
				
				// we've been assigned our guid. signal
				EventRef e0 = new Event_GuidAssigned(m_localGuid, m_remoteGuid);
				m_events.addTail(e0);

				// connection request accepted event
				EventRef e1 = new Event_ConnAccepted(m_localGuid, m_remoteGuid);
				m_events.addTail(e1);

				setState(CONN_STATE_ESTABLISHED);
				return true;
			}
		case MESSAGE_ID_CONNECT_PING:
			{
				MsgConnectPing* ping = (MsgConnectPing*)msg;

				if(m_state != CONN_STATE_ESTABLISHED && 
					m_state != CONN_STATE_ACCEPTING && 
					m_state != CONN_STATE_UNRESPONSIVE)
					return false;

				if(packet->m_origin != m_remoteGuid || packet->m_destination != m_localGuid)
					return false;

				// reset idle watch
				m_idleWatch.start();

				// received first keep-alive, now we can be established
				if(m_state == CONN_STATE_ACCEPTING)
					setState(CONN_STATE_ESTABLISHED);

				// received first keep-alive, now we can be established
				if(m_state == CONN_STATE_UNRESPONSIVE)
					setState(CONN_STATE_ESTABLISHED);

				// how healthy is the connection at the other end.
				m_remoteQoS = ping->m_qos;

				MsgConnectPong reply;
				reply.m_timestamp = ping->m_timestamp;
				
				if(sendMessage(&reply))
				{
					if(Debugging::dbg_ping && (Debugging::dbg_pingId == m_remoteGuid.m_guid || Debugging::dbg_pingId < 0))
					{
						LOGINFO(Debugging::g_logger, "<Connection(%d)> sent pong(%llu)...", m_remoteGuid.m_guid, ping->m_timestamp);
					}
					return true;
				}
				return false;
			}
		case MESSAGE_ID_CONNECT_PONG:
			{
				MsgConnectPong* pong = (MsgConnectPong*)msg;

				if(m_state != CONN_STATE_ESTABLISHED &&
					m_state != CONN_STATE_ACCEPTING &&
					m_state != CONN_STATE_UNRESPONSIVE)
					return false;

				if(packet->m_origin != m_remoteGuid || packet->m_destination != m_localGuid)
					return false;

				// reset idle watch
				m_idleWatch.start();

				const Latency* l = stopPing(pong->m_timestamp);

				// valid reply?
				if(l)
				{
					// send event
					EventRef e = new Event_ConnLatency(guid(), (l->m_pongTime - l->m_pingTime));
					m_events.addTail(e);
					return true;
				}
				return false;
			}
		case MESSAGE_ID_CONNECT_CLOSING:
			{
				MsgConnectClosing* closing = (MsgConnectClosing*)msg;

				//if(!netPlatform::Core::instance()->sameNonce(closing->m_connectKey, m_connectKey))
				//	return false;
				
				// reset idle watch
				m_idleWatch.start();

				// special case for refused connections.
				if(m_state == CONN_STATE_CONNECTING)
				{
					m_localGuid = packet->m_destination;
					m_remoteGuid = packet->m_origin;
					m_cachedPacket.reset();
				}

				if(m_state == CONN_STATE_CLOSING)
				{
					m_recvCloseCount++;
					return true;
				}

				if(packet->m_origin != m_remoteGuid || packet->m_destination != m_localGuid)
					return false;

				if(m_state >= CONN_STATE_CLOSED)
					return false;

				if(m_state < CONN_STATE_CLOSED)
				{
					m_closeReason = closing->m_closeReason;
					setState(CONN_STATE_CLOSING);
				}
				return true;
			}
		default:
			{
				if(packet->m_origin != m_remoteGuid || packet->m_destination != m_localGuid)
					return false;

				// reset idle watch
				m_idleWatch.start();
				return true;
			}
		}
		return false;
	}

	bool Connection::verifyPacket(ConnPacket* packet)
	{
		analysePacket(packet, false, false);
		return true;
	}

	bool Connection::analysePacket(ConnPacket* packet, bool forwarded, bool outgoing)
	{
		TrafficLoggerRef logger = (outgoing)? m_outstreamLogger : m_instreamLogger;
		TrafficSample& sample = (outgoing)? m_sendStats : m_recvStats;
		packet->analyse(m_socket->messageParser(), logger, forwarded, true);

		logger->getTraffic((1000 * 1000), sample);
		return true;
	}

	void Connection::getInboundTraffic(TrafficSample& sample)
	{
		sample = m_recvStats;
		sample.setLimit(m_recvBpS);
	}

	void Connection::getOutboundTraffic(TrafficSample& sample)
	{
		sample = m_sendStats;
		sample.setLimit(m_sendBpS);
	}

	bool Connection::forwardPacket(ConnPacket* packet)
	{
		if(m_state != CONN_STATE_ESTABLISHED)
			return false;

		if(m_secHandle.isNull())
			return false;

		if(!Debugging::dbg_forwardEnabled)
			return false;

		// check if we've throttled the packet.
		float currentLoad = ((float)packet->transmittedBytes()) + m_sendStats.m_bytes[TRAFFIC_TYPE_SOCKET];
		if(currentLoad > m_sendBpS)
			return false;
		
		// should we forward further.
		bool forward = (netPlatform::Core::instance()->getHandleStatus(m_secHandle) == netPlatform::ADDRESS_STATUS_LOST);
		forward |= (m_remoteGuid == ConnGuid(Debugging::dbg_unreachableConnection));
		forward &= m_forwardHandle.notNull();
		packet->m_forwarded = true;
		packet->m_address = forward? m_forwardHandle : m_secHandle;
		
		int packetBytes = m_socket->sendPacket(packet);
		analysePacket(packet, true, true);
		return true;
	}
	
	bool Connection::sendPacket()
	{
		int	addrStatus = netPlatform::Core::instance()->getHandleStatus(m_secHandle);
		bool forward = (addrStatus == netPlatform::ADDRESS_STATUS_LOST) && (Debugging::dbg_forwardEnabled);
		
		// setup packet header
		m_cachedPacket.m_port = m_socketPort;
		m_cachedPacket.m_origin = m_localGuid;
		m_cachedPacket.m_destination = m_remoteGuid;
		m_cachedPacket.m_forwarded = forward || (m_remoteGuid == ConnGuid(Debugging::dbg_unreachableConnection));
		m_cachedPacket.m_address = m_cachedPacket.m_forwarded? m_forwardHandle : m_secHandle;
		
		// address is BS
		if(m_cachedPacket.m_address.isNull())
			return false;
		
		// packet error or empty. nothing to send.
		if(m_cachedPacket.empty() || !m_cachedPacket.ok())
		{
			// clear packet.
			return m_cachedPacket.reset();
		}

		// we have senty a packet recently.
		if(m_sendWatch.started() && !m_sendWatch.elapsed(1 * 1000 * 1000))
		{
			// check if we throttle the packet.
			float currentLoad = ((float)m_cachedPacket.transmittedBytes()) + m_sendStats.m_bytes[TRAFFIC_TYPE_SOCKET];
	
			// throttled. return.
			if(currentLoad > m_sendBpS)
				return false;
		}
	
		// send current packet.
		int packetBytes = m_socket->sendPacket(&m_cachedPacket);
		
		// log packet stats.
		analysePacket(&m_cachedPacket, false, true);

		// reset send watch.
		m_sendWatch.start();

		// clear packet.
		return m_cachedPacket.reset();
	}		

	bool Connection::sendMessage(Message* msg)
	{
		// write message to current packet.
		// if we succeeded, we're done.
		int added = m_cachedPacket.writeMessage(msg);
		return (added > 0);
	}

	const char* Connection::stateToString() const
	{
		switch(m_state)
		{
		default: assert(false); return "UNKNOWN";
		case CONN_STATE_NOT_SET: return "not set";
		case CONN_STATE_CONNECTING: return "connecting";
		case CONN_STATE_RESERVED: return "reserved";
		case CONN_STATE_PENDING: return "pending";
		case CONN_STATE_ACCEPTING: return "accepting";
		case CONN_STATE_ESTABLISHED: return "established";
		case CONN_STATE_UNRESPONSIVE: return "unresponsive";
		case CONN_STATE_CLOSING: return "closing";
		case CONN_STATE_CLOSED: return "closed";
		}
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

