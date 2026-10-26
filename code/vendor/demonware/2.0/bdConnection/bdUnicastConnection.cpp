// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdUnicastConnection.h>

#include <bdSocket/bdAddressMap.h>

#include <bdConnection/bdMessage/bdMessage.h>
#include <bdConnection/bdWindow/bdReliableSendWindow.h>
#include <bdConnection/bdWindow/bdReliableReceiveWindow.h>
#include <bdConnection/bdConnectionListener.h>

#include <bdConnection/bdPacket/bdSAckChunk.h>
#include <bdConnection/bdPacket/bdDefaultPacket.h>
#include <bdConnection/bdPacket/bdInitChunk.h>
#include <bdConnection/bdPacket/bdInitAckChunk.h>
#include <bdConnection/bdPacket/bdCookie.h>
#include <bdConnection/bdPacket/bdCookieEchoChunk.h>
#include <bdConnection/bdPacket/bdCookieAckChunk.h>
#include <bdConnection/bdPacket/bdHeartbeatChunk.h>
#include <bdConnection/bdPacket/bdHeartbeatAckChunk.h>
#include <bdConnection/bdPacket/bdShutdownChunk.h>
#include <bdConnection/bdPacket/bdShutdownAckChunk.h>
#include <bdConnection/bdPacket/bdShutdownCompleteChunk.h>

// State transition diagram for Unicast Connection states.
//    
//                           -----          -------- (frm any state)
//                         /       \      /  rcv ABORT      [ABORT]
//        rcv INIT        |         |    |   ----------  or ----------
//        --------------- |         v    v   delete TCB     snd ABORT
//        generate Cookie  \    +---------+                 delete TCB
//        snd INIT ACK       ---|  CLOSED |
//                              +---------+
//                               /      \      [ASSOCIATE]
//                              /        \     ---------------
//                             |          |    create TCB
//                             |          |    snd INIT
//                             |          |    strt init timer
//              rcv valid      |          |
//            COOKIE  ECHO     |          v
//        (1) ---------------- |      +------------+
//            create TCB       |      | COOKIE-WAIT| (2)
//            snd COOKIE ACK   |      +------------+
//                             |          |
//                             |          |    rcv INIT ACK
//                             |          |    -----------------
//                             |          |    snd COOKIE ECHO
//                             |          |    stop init timer
//                             |          |    strt cookie timer
//                             |          v
//                             |      +--------------+
//                             |      | COOKIE-ECHOED| (3)
//                             |      +--------------+
//                             |          |
//                             |          |    rcv COOKIE ACK
//                             |          |    -----------------
//                             |          |    stop cookie timer
//                             v          v
//                           +---------------+
//                           |  ESTABLISHED  |
//                           +---------------+
//                                    |
//                                    |
//                           /--------+--------\        . 
//       [SHUTDOWN]         /                   \       .
//       -------------------|                   |
//       check outstanding  |                   |
//       DATA chunks        |                   |
//                          v                   |
//                     +---------+              |
//                     |SHUTDOWN-|              | rcv SHUTDOWN/check
//                     |PENDING  |              | outstanding DATA
//                     +---------+              | chunks
//                          |                   |------------------
//     No more outstanding  |                   |
//     ---------------------|                   |
//     snd SHUTDOWN         |                   |
//     strt shutdown timer  |                   |
//                          v                   v
//                     +---------+        +-----------+
//                 (4) |SHUTDOWN-|        | SHUTDOWN- |  (5,6)
//                     |SENT     |        | RECEIVED  |
//                     +---------+        +-----------+
//                          |  \                |
//    (A) rcv SHUTDOWN ACK  |   \               |
//    ----------------------|    \              |
//    stop shutdown timer   |     \rcv:SHUTDOWN |
//    send SHUTDOWN COMPLETE|      \  (B)       |
//    delete TCB            |       \           |
//                          |        \          | No more outstanding
//                          |         \         |-----------------
//                          |          \        | send SHUTDOWN ACK
//    (B)rcv SHUTDOWN       |           \       | strt shutdown timer
//    ----------------------|            \      |
//    send SHUTDOWN ACK     |             \     |
//    start shutdown timer  |              \    |
//    move to SHUTDOWN-     |               \   |
//    ACK-SENT              |                |  |
//                          |                v  |
//                          |             +-----------+
//                          |             | SHUTDOWN- | (7)
//                          |             | ACK-SENT  |
//                          |             +----------+-
//                          |                   | (C)rcv SHUTDOWN COMPLETE
//                          |                   |-----------------
//                          |                   | stop shutdown timer
//                          |                   | delete TCB
//                          |                   |
//                          |                   | (D)rcv SHUTDOWN ACK
//                          |                   |--------------
//                          |                   | stop shutdown timer
//                          |                   | send SHUTDOWN COMPLETE
//                          |                   | delete TCB
//                          |                   |
//                          \    +---------+    /
//                           \-->| CLOSED  |<--/
//                               +---------+

#define BD_UC_INIT_TIMEOUT		(.5f)
#define BD_UC_COOKIE_TIMEOUT	(.5f)
#define BD_UC_SHUTDOWN_TIMEOUT	(.5f)
#define BD_UC_SHUTDOWN_GUARD	(5.f)

#if defined BD_DEBUG
#	define BD_UC_INIT_RESENDS		(200)
#	define BD_UC_COOKIE_RESENDS		(200)
#else
#	define BD_UC_INIT_RESENDS		(5)
#	define BD_UC_COOKIE_RESENDS		(5)
#endif

#define BD_UC_SHUTDOWN_RESENDS	(5)

// we need to keep sending keep alives in debug mode in order to 
// get NAT devices open.
#define BD_KEEP_ALIVE_TIMEOUT (BD_MIN(BD_ALIVE_PERIOD / 5.f, 10.f))

// This is not valid if the timeout can be programatically 
// altered..
// BD_KEEP_ALIVE_TIMEOUT needs to be less than BD_SOCKET_TIMEOUT. Otherwise,
// unexpected connection drops may occur.
//BD_COMPILE_ASSERT(BD_KEEP_ALIVE_TIMEOUT < BD_SOCKET_TIMEOUT, 
//				  g_alivePeriodLessThanSocketTimeout);

#define BD_UC_RTO_ALPHA (0.125f)
#define BD_UC_RTO_BETA  (0.25f)
/// Clock granularity is not worst than 100us
#define BD_UC_CLOCK_GRANULARITY (100)

#define BD_LOG_LEVEL	"bdConnection/connections"

bdBool bdUnicastConnection::connect()
{
	bdBool result = false;
	
	if(m_state == BD_UC_CLOSED)
	{
		result = bdConnection::connect();

		// send an init chunk
		result = result && sendInit();

		if(true == result)
		{
			// change the state. we are now waiting for the
			// other side to send us a cookie.
			// (we are waiting on a bdInitAckChunk chunk)
			m_state = BD_UC_COOKIE_WAIT;
			result = true;
		}
	}
	
	if(!result)
	{
		BD_WARN(BD_LOG_LEVEL, "failed to connect!");
	}

	return result;
}

void bdUnicastConnection::disconnect()
{
	switch(m_state)
	{
		case BD_UC_COOKIE_WAIT:
		case BD_UC_COOKIE_ECHOED:
			close();
			break;
		case BD_UC_ESTABLISHED:
			m_state = BD_UC_SHUTDOWN_PENDING;
			m_shutdownGuard.start();
			break;
		default: // we're in a disconnecting/closed state already
			break;
	}
}

bdBool bdUnicastConnection::send(const bdMessageRef message,
								 const bdBool reliable)
{

	bdBool sent = false;

	if(m_state == BD_UC_ESTABLISHED)
	{
		bdUInt payLoadSize = 0;

		if(message->hasPayload())
		{
			payLoadSize += message->getPayload()->getDataSize();
		}

		if(message->hasUnencryptedPayload())
		{
			payLoadSize += message->getUnencryptedPayload()->getDataSize();
		}

		if(payLoadSize <= BD_MAX_MESSAGE_SIZE)
		{
			bdDataChunk::bdDataFlags flags = bdDataChunk::BD_DC_NO_FLAGS;

			if(!reliable)
			{
				flags = bdDataChunk::BD_DC_UNRELIABLE;
			}

			bdDataChunkRef chunk = new bdDataChunk(message, flags);

			if(reliable)
			{
				if (!m_reliableSendWindow)
				{
					m_reliableSendWindow = new bdReliableSendWindow;
				}
				sent = m_reliableSendWindow->add(chunk);
				if(false == sent)
				{
					BD_WARN(BD_LOG_LEVEL, "Failed to add message to reliable send window.");
				}
			}
			else
			{
				m_unreliableSendWindow.add(chunk);
				sent = true;		
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Message size > BD_MAX_MESSAGE_SIZE (%u > %u).", payLoadSize , BD_MAX_MESSAGE_SIZE);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "connection not established.");
	}


	return sent;
}

bdConnection::Status bdUnicastConnection::getStatus() const
{
	Status status = BD_DISCONNECTED;

	switch(m_state)
	{
		case BD_UC_CLOSED:
			status = BD_DISCONNECTED;
			break;
		case BD_UC_COOKIE_WAIT:
		case BD_UC_COOKIE_ECHOED:
			status = BD_CONNECTING;
			break;
		case BD_UC_ESTABLISHED:
			status = BD_CONNECTED;
			break;
		case BD_UC_SHUTDOWN_PENDING:
		case BD_UC_SHUTDOWN_SENT:
		case BD_UC_SHUTDOWN_RECEIVED:
		case BD_UC_SHUTDOWN_ACK_SENT:
			status = BD_DISCONNECTING;
			break;
		default:
			BD_WARN(BD_LOG_LEVEL, "Unknown state");
	}

	return status;
}

void bdUnicastConnection::close()
{
	if(m_state == BD_UC_CLOSED)
	{
		// already closed. don't call listeners.
	}
	else if(m_state < BD_UC_ESTABLISHED)
	{
		// the connection failed to connect.
		callListenersConnect(false);
	}
	else if (m_state == BD_UC_ESTABLISHED)
	{
		callListenersDisconnect();
	}
	else
	{
		// this happens when an ordered shutdown is completed.
		callListenersDisconnect();
	}

	m_state = BD_UC_CLOSED;
}


bdConnectionStatistics& bdUnicastConnection::getStats()
{
	return m_stats;
}


// protected:

bdUnicastConnection::bdUnicastConnection(bdCommonAddrRef dest,
										 bdAddressMap *addrMap)
: bdConnection(dest),
  m_reliableSendWindow(BD_NULL),
  m_reliableRecvWindow(BD_NULL),
  m_state(BD_UC_CLOSED),
  m_addrMap(addrMap)
{
	if(BD_NULL == m_addrMap)
	{
		BD_ERR(BD_LOG_LEVEL, "Addr map pointer not initialised.");
	}
	reset();
}

bdUnicastConnection::bdUnicastConnection(bdAddressMap *addrMap)
: m_reliableSendWindow(BD_NULL),
  m_reliableRecvWindow(BD_NULL),
  m_state(BD_UC_CLOSED),
  m_addrMap(addrMap)
{
	if(BD_NULL == m_addrMap)
	{
		BD_ERR(BD_LOG_LEVEL, "Addr map pointer not initialised.");
	}
	reset();
}

bdUnicastConnection::~bdUnicastConnection()
{
	delete m_reliableRecvWindow;
	delete m_reliableSendWindow;
}



bdBool bdUnicastConnection::receive(const bdUByte8 * buffer, 
									const bdUInt32 bufferSize)
{
	// Check the shutdown guard to make sure we haven't been stuck in the
	// shutdown state for too long
	if(m_shutdownGuard.getElapsedTimeInSeconds() > BD_UC_SHUTDOWN_GUARD)
	{
		close();
		return false;
	}

	if(bufferSize == 0)		
	{
		BD_WARN(BD_LOG_LEVEL, "Received zero size packet passed.");
		return false;
	}

	bdBool dataHandled = false;
	bdBool result = false;

	bdPacket packet;

	// 
	// Deserialize the packet and check if it is valid.
	// 
	bdBool valid = packet.deserialize(buffer, bufferSize);
	
	m_stats.addBytesRecv(bufferSize + BD_UDP_IP_OVERHEAD);

	// only record packet stats for valid packets
	if(valid)
	{
		m_stats.addPacketsRecv(1);
		m_stats.addPacketSizeRecv(bufferSize);
	}

	//
	// Process control chunks
	//

	if(valid)
	{
		bdChunkRef chunk;
		packet.getNextChunk(chunk);

		if(chunk && chunk->isControl())
		{
			const bdChunkTypes type = chunk->getType();

			switch(type)
			{
				case BD_CT_INIT:
					result = handleInit(chunk);
					break;
				case BD_CT_INIT_ACK:
					result = handleInitAck(chunk, packet.getVerificationTag());
					break;
				case BD_CT_COOKIE_ECHO:
					result = handleCookieEcho(chunk, packet.getVerificationTag());
					break;
				case BD_CT_COOKIE_ACK:
					result  = handleCookieAck(chunk, packet.getVerificationTag());
					break;
				default:
					break;
			}

			if(result)
			{
				chunk =  BD_NULL;
			}
		}
		
		//
		// Process data and other control chunks
		//
		if(m_localTag == packet.getVerificationTag())
		{
			do
			{
				if(chunk)
				{
					const bdChunkTypes type = chunk->getType();

					switch(type)
					{
						case BD_CT_DATA:
							result = handleData(chunk);
							dataHandled = dataHandled || result;
							break;
						case BD_CT_SACK:
							result = handleSAck(chunk);
							break;
						case BD_CT_HEARTBEAT:
							result = handleHeartbeat(chunk);
							break;
						case BD_CT_HEARTBEAT_ACK:
							result = handleHeartbeatAck(chunk);
							break;
						case BD_CT_SHUTDOWN:
							result = handleShutdown(chunk);
							break;
						case BD_CT_SHUTDOWN_ACK:
							result = handleShutdownAck(chunk);
							break;
						case BD_CT_SHUTDOWN_COMPLETE:
							result = handleShutdownComplete(chunk);
							break;
						default:
							//BD_WARN(BD_LOG_LEVEL, "Unknown chunk type");
							break;
					}
				}
			}
			while(packet.getNextChunk(chunk));
		}

		if(result)
		{
			m_receiveTimer.start();
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "bdUnicastConnection::receive(): Invalid packet received.");
	}

	// Section 9.2 - SCTP Implementer's Guide 
	//While in SHUTDOWN-SENT state, the SHUTDOWN sender MUST immediately
	//respond to each received packet containing one or more DATA chunk(s)
	//with a SHUTDOWN chunk
	if(m_state == BD_UC_SHUTDOWN_SENT)
	{
		if(!sendShutdown())
		{
			close();
		}
	}

	// fixme: implement
	//		/*if (message->getType() == BD_CONNECTION_SET_PARAM)
	//		{
	//		message->getPayload()->readUInt32(m_maxTransmissionRate);
	//		message->getPayload()->setReadPosition(0); // use resetReadPosition
	//		}*/

	return result;
}


bdUInt bdUnicastConnection::getDataToSend(bdUByte8 *const buffer,
										  const bdUInt bufferSize)
{
	bdPacket packet(m_peerTag, bufferSize);

	// Check the shutdown guard to make sure we haven't been stuck in the
	// shutdown state for too long
	if(m_shutdownGuard.getElapsedTimeInSeconds() > BD_UC_SHUTDOWN_GUARD)
	{
		close();
		return 0;
	}
	
	//
	// First check the timers to see if control chunks need to be resent.
	//
	if(m_outQueue.isEmpty())
	{

		if(m_initTimer.getElapsedTimeInSeconds() > BD_UC_INIT_TIMEOUT)
		{
			if(!sendInit()) 
			{
				// if we can't send something weird is wrong.
				// close connection.
				close();
			}
		}
		else if(m_cookieTimer.getElapsedTimeInSeconds() > BD_UC_COOKIE_TIMEOUT)
		{
			if(!sendCookieEcho(m_initAckChunk))
			{
				// if we can't send something weird is wrong.
				// close connection.
				close();
			}
		}
		else if(m_shutdownTimer.getElapsedTimeInSeconds() > BD_UC_SHUTDOWN_TIMEOUT)
		{
			switch(m_state)
			{
				case BD_UC_SHUTDOWN_SENT:
					if(!sendShutdown())
					{
						// if we can't send something weird is wrong.
						// close connection.
						close();
					}
					break;
				case BD_UC_SHUTDOWN_RECEIVED:
				case BD_UC_SHUTDOWN_ACK_SENT:
					if(!sendShutdownAck())
					{
						// if we can't send something weird is wrong.
						// close connection.
						close();
					}
					break;
				default:
					break;
			}
		}

	}

	// Check for control chunks.
	while(!m_outQueue.isEmpty())
	{
		bdControlChunkStore &chunkStore = m_outQueue.peek();
		
		if(chunkStore.m_lone)
		{
			// anything we've already added to the packet 
			// needs to be put back in the queue.
			bdChunkRef chunk;
			while(packet.getNextChunk(chunk))
			{
				m_outQueue.enqueue(chunk);
			}

			bdUInt32 tagToUse = m_peerTag;
			// INIT ACK chunk must be sent with the peer tag defined within it
			if (chunkStore.m_chunk->getType() == BD_CT_INIT_ACK)
			{
				tagToUse = (static_cast<bdInitAckChunkRef>(chunkStore.m_chunk))->getPeerTag();
			}

			// create a new packet to send the lone chunk.
			bdPacket lpacket(tagToUse, bufferSize);
			lpacket.addChunk(chunkStore.m_chunk);
			m_outQueue.dequeue();
			// ARGH: FIXME : mid-function returns are evil [and lazy].............  
			return lpacket.serialize(buffer, bufferSize);
		}
		else
		{
			if(packet.addChunk(chunkStore.m_chunk))
			{
				m_outQueue.dequeue();
			}
			else
			{
				break;
			}
		}
	}


	//
	// Now look for anything else that needs to be sent.
	//
	const bdFloat32 lastReceived = m_receiveTimer.getElapsedTimeInSeconds();
	switch(m_state)
	{
		case BD_UC_ESTABLISHED:
		case BD_UC_SHUTDOWN_SENT:
		case BD_UC_SHUTDOWN_PENDING:
		case BD_UC_SHUTDOWN_RECEIVED:
			{
				// check to see if the connection has timed out. the connection times
				// out if it hasn't received any data in BD_ALIVE_PERIOD seconds.
				if (lastReceived > BD_ALIVE_PERIOD)
				{
					BD_WARN(BD_LOG_LEVEL, "bdUnicastConnection: Connection timed out.");
					close();
				}
				else
				{
					// find data to send:

					// look through reliable windows
					if(m_reliableRecvWindow)
					{
						m_reliableRecvWindow->getDataToSend(packet);
					}
					if(m_reliableSendWindow)
					{
						m_reliableSendWindow->getDataToSend(packet);
					}

					// get data from unreliable window.
					m_unreliableSendWindow.getDataToSend(packet);
				}
			}
			break;
		default:
			break;

	}

	//
	// Check to see if we are closing
	//
	switch(m_state)
	{
		case BD_UC_SHUTDOWN_PENDING:
			if(packet.isEmpty() && windowsEmpty())
			{
				// if we have sent everything to send
				if(sendShutdown())
				{
					m_state = BD_UC_SHUTDOWN_SENT;
				}
				else
				{
					close();
				}
			}
			break;
		case BD_UC_SHUTDOWN_RECEIVED:
			if(packet.isEmpty() && windowsEmpty())
			{
				if(sendShutdownAck())
				{
					m_state = BD_UC_SHUTDOWN_ACK_SENT;
				}
				else
				{
					close();
				}
			}
			break;
		default:
			break;
	}

	// 
	// Check to see if we need to do keep alive
	//
	switch(m_state)
	{
		case BD_UC_ESTABLISHED:
			// if we have no data to send check to see if we need to send a keep alive.
			if (packet.isEmpty())
			{
				const bdFloat32 lastSent = m_sendTimer.getElapsedTimeInSeconds();

				// send a keep alive (heartbeat) if we haven't sent anything 
				// in BD_KEEP_ALIVE_TIMEOUT seconds.
				if (lastSent > BD_KEEP_ALIVE_TIMEOUT)
				{
					BD_INFO(BD_LOG_LEVEL, "Sending Heartbeat. Last send %.2fs ago.", lastSent);

					bdHeartbeatChunkRef chunk = new bdHeartbeatChunk;

					packet.addChunk(chunk);
				}
			}
			break;
		default:
			break;
	}


	bdUInt dataToSend = 0;
	if(!packet.isEmpty())
	{
		dataToSend = packet.serialize(buffer, bufferSize);

		if(dataToSend > bufferSize)
		{
			BD_ERR(BD_LOG_LEVEL, "buffer overflow!");
		}

		m_sendTimer.start();
	}

	if(dataToSend > 0)
	{		
		m_stats.addBytesSent(BD_UDP_IP_OVERHEAD + dataToSend);
		m_stats.addPacketsSent(1);
		m_stats.addPacketSizeSent(dataToSend);
	}


	return dataToSend;
}


// protected:



bdBool bdUnicastConnection::getMessageToDispatch(bdMessageRef &message)
{
	bdBool result = false;

	bdMessageRef toDispatch;

	if(m_reliableRecvWindow)
	{
		bdDataChunkRef chunk = m_reliableRecvWindow->getNextToRead();
		if(chunk)
		{
			toDispatch = chunk->getMessage();
			result = true;
		}
	}

	if (!result)
	{
		bdDataChunkRef chunk = m_unreliableReceiveWindow.getNextToRead();
		if(chunk)
		{
			toDispatch = chunk->getMessage();
			result = true;
		}
	}

	if(result)
	{
		message = toDispatch;
	}

	return result;
}

bdBool bdUnicastConnection::handleData(bdChunkRef &chunk)
{
	bdBool result = false;

	bdDataChunkRef dchunk = reference_cast<bdDataChunk>(chunk);

	if(dchunk->getFlags() & bdDataChunk::BD_DC_UNRELIABLE)
	{
		result = m_unreliableReceiveWindow.add(dchunk);
	}
	else
	{
		if(!m_reliableRecvWindow)
		{
			m_reliableRecvWindow = new bdReliableReceiveWindow;
		}
		result = m_reliableRecvWindow->add(dchunk);

		if(!result)
		{
			BD_WARN(BD_LOG_LEVEL, "receive window full.");
		}
	}

	return result;
}

bdBool bdUnicastConnection::handleSAck(bdChunkRef &chunk)
{
	bdBool result = false;

	bdSAckChunkRef schunk = reference_cast<bdSAckChunk>(chunk);
	bdFloat32 rtt;

	if(m_reliableSendWindow)
	{
		result = m_reliableSendWindow->handleAck(schunk, rtt);

		if (result)
		{
			if(rtt > 0)
			{
				// RTO calculation:
				bdFloat32 rto = 0;

				//RTO    - Retransmission Time-out
				//RTT    - Round-trip Time
				//RTTVAR - Round-trip Time Variation
				//SRTT   - Smoothed RTT
				if(m_smoothedRTT || m_RTTVariation)
				{
					//When a new RTT measurement R' is made, set
					//
					//       RTTVAR <- (1 - RTO.Beta) * RTTVAR + RTO.Beta * |SRTT - R'| SRTT
					//       <- (1 - RTO.Alpha) * SRTT + RTO.Alpha * R'
					//
					//       Note: The value of SRTT used in the update to RTTVAR is its value
					//       before updating SRTT itself using the second assignment.
					//
					//       After the computation, update RTO <- SRTT + 4 * RTTVAR.
					bdFloat32 absRTT = m_smoothedRTT > rtt ? (m_smoothedRTT - rtt) : (rtt - m_smoothedRTT);
					m_RTTVariation = ((1.f - BD_UC_RTO_BETA) * m_RTTVariation) +
									  (BD_UC_RTO_BETA * absRTT);

					m_smoothedRTT = ((1.f - BD_UC_RTO_ALPHA) * m_smoothedRTT) +
									 (BD_UC_RTO_ALPHA * rtt);
				}
				else
				{
					//When the first RTT measurement R is made, set SRTT <- R, RTTVAR
					//	<- R/2, and RTO <- SRTT + 4 * RTTVAR.
					m_smoothedRTT = rtt;
					m_RTTVariation = rtt / 2;	
				}

				//BD_INFO(BD_LOG_LEVEL, "rtt %f, srtt %f, rttvar %f", rtt, m_smoothedRTT, m_RTTVariation);
				m_stats.setLastRTT(m_smoothedRTT);	

				//Whenever RTTVAR is computed, if RTTVAR = 0, then 
				//adjust RTTVAR <- G, where G is the CLOCK GRANULARITY.
				// fixme: this will never be zero. it's a float!
				if (m_RTTVariation == 0) 
				{
					m_RTTVariation = BD_UC_CLOCK_GRANULARITY;
				}

				// After the computation, update RTO <- SRTT + (MULTIPLIER * RTTVAR).
				rto = m_smoothedRTT + (BD_RT0_MULTIPLIER * m_RTTVariation);
				
				if(rto < BD_UC_RTO_MIN)
				{
					//Whenever RTO is computed, if it is less than RTO.Min seconds then
					//it is rounded up to RTO.Min seconds.  The reason for this rule is
					//that RTOs that do not have a high minimum value are susceptible
					//to unnecessary timeouts.
					rto = BD_UC_RTO_MIN;
				}
				else if (rto > BD_UC_RTO_MAX)
				{
					//A maximum value may be placed on RTO provided it is at least
					//RTO.max seconds.
					rto = BD_UC_RTO_MAX;
				}

				m_reliableSendWindow->setTimeoutPeriod(rto);
			}
		}
		else
		{
			// if we can't handle the ack that means the other end has sent us
			// invalid data and so the windows will be in an invalid state.
			// disconnect() should succeed but if it can't because the windows
			// are stopping an ordered shutdown the connection will timeout and
			// close.
			BD_WARN(BD_LOG_LEVEL, "Unable to handle ack. disconnecting connection.");
			disconnect();
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "invalid stream id.");
	}

	return result;
}

bdBool bdUnicastConnection::handleInit(bdChunkRef &chunk)
{
	bdBool result = false;

	bdInitChunkRef ichunk = reference_cast<bdInitChunk>(chunk);

	bdSecurityID id;
	if(BD_NULL == m_addrMap)
	{
		BD_ERR(BD_LOG_LEVEL, "Addr map pointer not initialised.");
	}
	else
	{
		m_addrMap->addrToCommonAddr(m_addrHandle, m_addr, id);
		if(ichunk->getInitTag())
		{
			BD_INFO(BD_LOG_LEVEL, "uc::handling init: m_localTag: %d", m_localTag);
			result = sendInitAck(chunk);
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "handleInit: invalid init tag (%u)", ichunk->getInitTag());
			result = false;
		}
	}

	return result;
}

bdBool bdUnicastConnection::handleInitAck(bdChunkRef &chunk, const bdUInt32 vtag)
{
	bdBool result = false;
	
	//If an INIT ACK is received by an endpoint in any state other than the
	//COOKIE-WAIT state, the endpoint should discard the INIT ACK chunk.
	//An unexpected INIT ACK usually indicates the processing of an old or
	//duplicated INIT chunk.
	if(m_state == BD_UC_COOKIE_WAIT)
	{
		if(vtag == m_localTag)
		{
			bdInitAckChunkRef iachunk = reference_cast<bdInitAckChunk>(chunk);

			m_peerTag = iachunk->getInitTag();

			if(m_peerTag)
			{
				BD_INFO(BD_LOG_LEVEL, "uc::handling init ack: m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);

				if(sendCookieEcho(iachunk))
				{
					m_initTimer.reset();
					m_state = BD_UC_COOKIE_ECHOED;

					result = true;
				}
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "handleInitAck: invalid init tag (%u)", m_peerTag);
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Invalid verification tag on init ack.");
		}
	}

	return result;
}

bdBool bdUnicastConnection::handleCookieEcho(bdChunkRef &chunk, const bdUInt32)
{
	bdBool result = false;

	bdCookieEchoChunkRef cchunk = reference_cast<bdCookieEchoChunk>(chunk);

	bdCookieRef cookie;

	if(cchunk->getCookie(cookie))
	{
		// fixme: should verify time-stamp.
		const bdUInt32 localTag = cookie->getLocalTag();
		const bdUInt32 peerTag = cookie->getPeerTag();
		const bdUInt32 localTieTag = cookie->getLocalTieTag();
		const bdUInt32 peerTieTag = cookie->getPeerTieTag();

		switch(m_state)
		{
			case BD_UC_CLOSED:
				{
					// expected state.
					bdSecurityID id;
				if(BD_NULL == m_addrMap)
				{
					BD_ERR(BD_LOG_LEVEL, "Addr map pointer not initialised.");
				}
				else
				{
					m_addrMap->addrToCommonAddr(m_addrHandle, m_addr, id);
					
					m_localTag = localTag;
					m_peerTag = peerTag;
					BD_INFO(BD_LOG_LEVEL, "uc::handling cookie echo: m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);
					if(sendCookieAck())
					{				
						m_state = BD_UC_ESTABLISHED;
						callListenersConnect(true);
						result = true;
					}
				}
					break;
				}
			default:
				// unexpected state.
				//+------------+------------+---------------+--------------+-------------+
				//|  Local Tag | Peer's Tag | Local-Tie-Tag |Peer's-Tie-Tag|   Action/   |
				//|            |            |               |              | Description |
				//+------------+------------+---------------+--------------+-------------+
				//|    X       |     X      |      M        |      M       |     (A)     |
				//+------------+------------+---------------+--------------+-------------+
				//|    M       |     X      |      A        |      A       |     (B)     |
				//+------------+------------+---------------+--------------+-------------+
				//|    M       |     0      |      A        |      A       |     (B)     |
				//+------------+------------+---------------+--------------+-------------+
				//|    X       |     M      |      0        |      0       |     (C)     |
				//+------------+------------+---------------+--------------+-------------+
				//|    M       |     M      |      A        |      A       |     (D)     |
				//+======================================================================+
				//|       Table 2: Handling of a COOKIE ECHO when a TCB exists           |
				//+======================================================================+
				//
				//   Legend:
				//
				//      X - Tag does not match the existing TCB
				//      M - Tag matches the existing TCB.
				//      0 - No Tie-Tag in Cookie (unknown).
				//      A - All cases, i.e. M, X or 0.
				//

				if(
					m_localTag != localTag && m_peerTag != peerTag
					&&
					m_localTag == localTieTag && m_peerTag == peerTieTag
					)
				{
					//A) In this case, the peer may have restarted.  When the endpoint
					//   recognizes this potential 'restart', the existing session is
					//   treated the same as if it received an ABORT followed by a new
					//   COOKIE ECHO with the following exceptions:

					//   -  Any SCTP DATA Chunks MAY be retained (this is an implementation
					//      specific option).

					//   -  A notification of RESTART SHOULD be sent to the ULP instead of
					//      a "COMMUNICATION LOST" notification.

					//   All the congestion control parameters (e.g., cwnd, ssthresh)
					//   related to this peer MUST be reset to their initial values (see
					//   Section 6.2.1).

					//   After this the endpoint shall enter the ESTABLISHED state.

					//If the endpoint is in the SHUTDOWN-ACK-SENT state and recognizes
					//the peer has restarted (Action A), it MUST NOT setup a new
					//association but instead resend the SHUTDOWN ACK and send an ERROR
					//chunk with a "Cookie Received while Shutting Down" error cause to
					//its peer.

					reset();

					if(sendCookieAck())
					{
						BD_INFO(BD_LOG_LEVEL, "uc::handling cookie echo: m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);
						bdSecurityID id;
						if(BD_NULL == m_addrMap)
						{
							BD_ERR(BD_LOG_LEVEL, "Addr map pointer not initialised.");
						}
						else
						{
							m_addrMap->addrToCommonAddr(m_addrHandle, m_addr, id);

						m_localTag = localTag;
						m_peerTag = peerTag;

						m_state = BD_UC_ESTABLISHED;

						callListenersReconnect();
						result = true;
					}
					}

					// todo - handle SHUTDOWN-ACK-SENT case
				}
				else if(
						(m_localTag == localTag && m_peerTag != peerTag)
						||
						(m_localTag == localTag && !peerTieTag)
						)
				{
					//B) In this case, both sides may be attempting to start an association
					//   at about the same time but the peer endpoint started its INIT
					//   after responding to the local endpoint's INIT.  Thus it may have
					//   picked a new Verification Tag not being aware of the previous Tag
					//   it had sent this endpoint.  The endpoint should stay in or enter
					//   the ESTABLISHED state but it MUST update its peer's Verification
					//   Tag from the State Cookie, stop any init or cookie timers that may
					//   running and send a COOKIE ACK.

					if(m_state != BD_UC_ESTABLISHED)
					{
						m_state = BD_UC_ESTABLISHED;
						callListenersConnect(true);
					}
					m_peerTag = peerTag;

					m_initTimer.reset();
					m_cookieTimer.reset();

					result = sendCookieAck();
				}
				else if(m_localTag != localTag && m_peerTag == peerTag)
				{
					//C) In this case, the local endpoint's cookie has arrived late.
					//   Before it arrived, the local endpoint sent an INIT and received an
					//   INIT-ACK and finally sent a COOKIE ECHO with the peer's same tag
					//   but a new tag of its own.  The cookie should be silently
					//   discarded.  The endpoint SHOULD NOT change states and should leave
					//   any timers running.
				}
				else if(m_localTag == localTag && m_peerTag == peerTag)
				{
					//D) When both local and remote tags match the endpoint should always
					//	enter the ESTABLISHED state, if it has not already done so. It
					//	should stop any init or cookie timers that may be running and send
					//	a COOKIE ACK.

					if(m_state != BD_UC_ESTABLISHED)
					{
						// fixme: what if we're shutting down?
						m_state = BD_UC_ESTABLISHED;
						callListenersConnect(true);
					}

					m_initTimer.reset();
					m_cookieTimer.reset();
					result = sendCookieAck();
				}
				else
				{
					//   any case not shown in above table, the cookie should be
					//   silently discarded.

					result = true;
				}
				break;
		}
	}

	return result;
}

bdBool bdUnicastConnection::handleCookieAck(bdChunkRef &, const bdUInt32 vtag)
{
	bdBool result = false;

	if(vtag == m_localTag)
	{
		//At any state other than COOKIE-ECHOED, an endpoint should silently
		//discard a received COOKIE ACK chunk.
		if(m_state == BD_UC_COOKIE_ECHOED)
		{
			BD_INFO(BD_LOG_LEVEL, "uc::handling cookie ack: m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);

			m_state = BD_UC_ESTABLISHED;
			callListenersConnect(true);

			m_cookieTimer.reset();

			result = true;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Cookie ack received in invalid state: %u", m_state);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Invalid verification tag on cookie ack. (%u)", vtag);
	}

	return result;
}

bdBool bdUnicastConnection::handleHeartbeat(bdChunkRef &chunk)
{
	m_receiveTimer.start();

	return sendHeartbeatAck(chunk);
}

bdBool bdUnicastConnection::handleHeartbeatAck(bdChunkRef &)
{
	m_receiveTimer.start();

	return true;
}

bdBool bdUnicastConnection::handleShutdown(bdChunkRef &)
{
	bdBool result = false;

	switch(m_state)
	{
		case BD_UC_ESTABLISHED:
			BD_INFO(BD_LOG_LEVEL, "uc::handling shutdown (a)");
			m_state = BD_UC_SHUTDOWN_RECEIVED;
			result = true;
			break;
		case BD_UC_SHUTDOWN_SENT:
			BD_INFO(BD_LOG_LEVEL, "uc::handling shutdown (b)");
			m_state = BD_UC_SHUTDOWN_RECEIVED;
			result = sendShutdownAck();
			break;
		default:
			BD_WARN(BD_LOG_LEVEL, "uc::handling shutdown (c) - unexpected (%u)!", m_state);
			break;
	}

	if(result)
	{
		BD_INFO(BD_LOG_LEVEL, "uc::handling shutdown.");
		m_shutdownGuard.start();
	}
	
	return true;
}

bdBool bdUnicastConnection::handleShutdownAck(bdChunkRef &)
{
	bdBool result = false;

	switch(m_state)
	{
		case BD_UC_SHUTDOWN_SENT:
			BD_INFO(BD_LOG_LEVEL, "uc::handling shutdown ack (a)");
			sendShutdownComplete();
			close();
			m_shutdownTimer.reset();
			result = true;
			break;
		case BD_UC_SHUTDOWN_ACK_SENT:
			BD_INFO(BD_LOG_LEVEL, "uc::handling shutdown ack (b)");
			sendShutdownComplete();
			close();
			m_shutdownTimer.reset();
			result = true;
			break;
		default:
			BD_INFO(BD_LOG_LEVEL, "uc::handling shutdown ack (c) - unexpected (%u).", m_state);
			break;
	}

	return result;
}

bdBool bdUnicastConnection::handleShutdownComplete(bdChunkRef &)
{
	bdBool result = false;
	
	switch(m_state)
	{
		case BD_UC_SHUTDOWN_ACK_SENT:
			BD_INFO(BD_LOG_LEVEL, "uc::handling shutdown complete (a)");
			m_shutdownTimer.reset();
			close();
			result = true;
			break;
		default:
			BD_WARN(BD_LOG_LEVEL, "uc::handling shutdown complete (b) - unexpected!");
			break;
	}

	return result;
}


bdBool bdUnicastConnection::sendInit()
{
	bdBool result = m_state == BD_UC_CLOSED || m_state == BD_UC_COOKIE_WAIT;

	BD_ASSERT(result, "invalid state to send init from.");

	if(m_initResends++ < BD_UC_INIT_RESENDS)
	{
		m_initTimer.start();

		bdInitChunkRef chunk = new bdInitChunk(m_localTag, BD_DEFAULT_RECEIVE_WINDOW_CREDIT);

		m_outQueue.enqueue(bdControlChunkStore(chunk));

		BD_INFO(BD_LOG_LEVEL, "uc::sending init: m_localTag: %d", m_localTag);
	}
	else
	{
		result = false;
	}

	return result;
}


bdBool bdUnicastConnection::sendInitAck(bdInitChunkRef chunk)
{
	bdBool result = true;

	bdUInt32 peerTag = chunk->getInitTag();
	
	bdUInt32 localTag = 0;
	bdUInt32 peerTieTag = 0;
	bdUInt32 localTieTag = 0;

	switch(m_state)
	{
		case BD_UC_CLOSED:
		case BD_UC_SHUTDOWN_ACK_SENT:
			// expected state.
			m_peerTag = peerTag;
			localTag = m_localTag;
			break;
		case BD_UC_COOKIE_ECHOED:
			//For an endpoint that is in the COOKIE-ECHOED state it MUST populate
			//its Tie-Tags with the Tag information of itself and its peer.
			peerTieTag = m_peerTag;
			localTieTag = m_localTag;
			//break; continue into next case:
		case BD_UC_COOKIE_WAIT:	
			//This usually indicates an initialization collision, i.e., each
			//endpoint is attempting, at about the same time, to establish an
			//association with the other endpoint.
			//Upon receipt of an INIT in the COOKIE-WAIT or COOKIE-ECHOED state, an
			//endpoint MUST respond with an INIT ACK using the same parameters it
			//sent in its original INIT chunk (including its Initiation Tag,
			//unchanged).  These original parameters are combined with those from
			//the newly received INIT chunk.  The endpoint shall also generate a
			//State Cookie with the INIT ACK.  The endpoint uses the parameters
			//sent in its INIT to calculate the State Cookie.
			localTag = m_localTag;
			break;
		case BD_UC_ESTABLISHED:
		case BD_UC_SHUTDOWN_PENDING:
		case BD_UC_SHUTDOWN_RECEIVED:
		case BD_UC_SHUTDOWN_SENT:
			//Unless otherwise stated, upon reception of an unexpected INIT for
			//this association, the endpoint shall generate an INIT ACK with a
			//State Cookie.  In the outbound INIT ACK the endpoint MUST copy its
			//current Verification Tag and peer's Verification Tag into a reserved
			//place within the state cookie.  We shall refer to these locations as
			//the Peer's-Tie-Tag and the Local-Tie-Tag.  The outbound SCTP packet
			//containing this INIT ACK MUST carry a Verification Tag value equal to
			//the Initiation Tag found in the unexpected INIT.  And the INIT ACK
			//MUST contain a new Initiation Tag. Other parameters for the endpoint 
			//SHOULD be copied from the
			//existing parameters of the association into the INIT ACK and cookie.
			localTag = bdTrulyRandom::getInstance()->getRandomUInt();
			peerTieTag = m_peerTag;
			localTieTag = m_localTag;
			break;
		default:
			result = false;
			break;
	};
	
	if(result)
	{
		bdCookieRef cookie = new bdCookie(localTag, peerTag, localTieTag, peerTieTag);

		bdInitAckChunkRef iachunk = new bdInitAckChunk(localTag, cookie,
				BD_DEFAULT_RECEIVE_WINDOW_CREDIT, peerTag);
		m_outQueue.enqueue(bdControlChunkStore(iachunk, true));

		BD_INFO(BD_LOG_LEVEL, "uc::sending init ack: m_localTag/localTag/m_peerTag: %d/%d/%d", m_localTag, localTag, m_peerTag);
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "bdUnicastConnection::sendInitAck(): Failed to send init ack.");
	}

	return result;
}

bdBool bdUnicastConnection::sendCookieEcho(bdInitAckChunkRef chunk)
{
	bdBool result = true;

	m_initAckChunk = chunk;

	if(m_cookieResends++ < BD_UC_COOKIE_RESENDS)
	{
		bdByteBufferRef cookie;
		if(chunk->getCookie(cookie))
		{
			m_cookieTimer.start();

			bdCookieEchoChunkRef cookieEcho = new bdCookieEchoChunk(cookie);

			m_outQueue.enqueue(bdControlChunkStore(cookieEcho));

			BD_INFO(BD_LOG_LEVEL, "uc::sending cookie echo: m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);
		}
	}
	else
	{
		result = false;
	}

	return result;
}

bdBool bdUnicastConnection::sendCookieAck()
{
	bdBool result = true;

	bdCookieAckChunkRef chunk = new bdCookieAckChunk;

	m_outQueue.enqueue(bdControlChunkStore(chunk));

	BD_INFO(BD_LOG_LEVEL, "uc::sending cookie ack: m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);

	return result;
}

bdBool bdUnicastConnection::sendHeartbeat(bdInitChunkRef)
{
	bdHeartbeatChunkRef hchunk = new bdHeartbeatChunk;

	m_outQueue.enqueue(bdControlChunkStore(hchunk, false));

	return true;
}

bdBool bdUnicastConnection::sendHeartbeatAck(bdInitChunkRef)
{
	// we only need to ack the heartbeat if there is nothing already
	// going out.
	if(m_outQueue.isEmpty() && windowsEmpty())
	{
		bdHeartbeatAckChunkRef hchunk = new bdHeartbeatAckChunk;
		m_outQueue.enqueue(bdControlChunkStore(hchunk, false));
	}

	return true;
}

bdBool bdUnicastConnection::sendShutdown()
{
	bdBool result = true;

	if(m_shutdownResends++ < BD_UC_SHUTDOWN_RESENDS)
	{
		BD_INFO(BD_LOG_LEVEL, "uc::sending shutdown (%u/%u)", m_shutdownResends, BD_UC_SHUTDOWN_RESENDS);
		bdChunkRef chunk = new bdShutdownChunk;
		m_outQueue.enqueue(bdControlChunkStore(chunk));

		m_shutdownTimer.start();
	}
	else
	{
		result = false;
	}

	return result;
}

bdBool bdUnicastConnection::sendShutdownAck()
{
	bdBool result = true;

	if(m_shutdownResends++ < BD_UC_SHUTDOWN_RESENDS)
	{
		BD_INFO(BD_LOG_LEVEL, "uc::sending shutdown ack (%u/%u)", m_shutdownResends, BD_UC_SHUTDOWN_RESENDS);
		bdChunkRef chunk = new bdShutdownAckChunk;
		m_outQueue.enqueue(bdControlChunkStore(chunk));

		m_shutdownTimer.start();
	}
	else
	{
		result = false;
	}

	return result;
}

bdBool bdUnicastConnection::sendShutdownComplete()
{
	bdBool result = true;

	BD_INFO(BD_LOG_LEVEL, "uc::sending shutdown complete");

	bdChunkRef chunk = new bdShutdownCompleteChunk;
	m_outQueue.enqueue(bdControlChunkStore(chunk));


	return result;
}


void bdUnicastConnection::reset()
{
	delete m_reliableRecvWindow;
	delete m_reliableSendWindow;

	m_reliableRecvWindow = BD_NULL;
	m_reliableSendWindow = BD_NULL;

	while(!m_outQueue.isEmpty())
	{
		m_outQueue.dequeue();
	}

	m_unreliableReceiveWindow.reset();
	m_unreliableSendWindow.reset();

	m_sendTimer.start();
	m_receiveTimer.start();

	m_initResends = 0;
	m_cookieResends = 0;
	m_shutdownResends = 0;

	m_localTag = bdTrulyRandom::getInstance()->getRandomUInt();

	m_peerTag = 0;

	m_smoothedRTT = 0;
	m_RTTVariation = 0;
}

void bdUnicastConnection::callListenersConnect(const bdBool success)
{
	for(bdUInt i = 0; i < m_listeners.getSize(); i++)
	{
		if(success)
		{
			m_listeners[i]->onConnect(this);
		}
		else
		{
			m_listeners[i]->onConnectFailed(this);
		}
	}
}

void bdUnicastConnection::callListenersDisconnect()
{
	for(bdUInt i = 0; i < m_listeners.getSize(); i++)
	{
		m_listeners[i]->onDisconnect(this);
	}
}

void bdUnicastConnection::callListenersReconnect()
{
	for(bdUInt i = 0; i < m_listeners.getSize(); i++)
	{
		m_listeners[i]->onReconnect(this);
	}
}

bdBool bdUnicastConnection::windowsEmpty() const
{
	bdBool empty = true;
	if(m_reliableSendWindow && !m_reliableSendWindow->isEmpty())
	{
		empty = false;
	}
	return empty;
}

