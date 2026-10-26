#include "stdafx.h"
#include "./netTransport_Message.h"
#include "./netTransport_StreamPacket.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Core.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{
#if !defined( CFG_GOLD )
	Debug::Int dbg_dropPacket("Multiplayer.Transport.PacketDropPercent", 0, Debug::Range(0, 100));
#endif

	StreamPacketSend::StreamPacketSend(SegmentPoolRef segmentPool, unsigned char packetId, SocketRef socket, int socketPort, ConnGuid guid)
	: SegmentStream(segmentPool)
	, m_socket(socket)
	, m_socketPort(socketPort)
	, m_guid(guid)
	, m_packetSqn(0)
	, m_packetId(packetId)
	, m_packetReliable(false)
	{}

	bool StreamPacketSend::logMessage(const Message* msg)
	{
		if(m_socketPort != netTransport::Debugging::dbg_logPacketSocketPort && netTransport::Debugging::dbg_logPacketSocketPort >= 0)
			return false;
		
		if(m_guid.m_guid != netTransport::Debugging::dbg_logPacketDestination && netTransport::Debugging::dbg_logPacketDestination >= 0)
			return false;

		int messageId = msg->messageId();

		switch(messageId)
		{
		default:
			return false;
		case MESSAGE_ID_PACKET_ACK:
		case MESSAGE_ID_PACKET_FRAGMENT:
			if(!netTransport::Debugging::dbg_logPacketReliable)
				return false;
			break;
		case MESSAGE_ID_PACKET:
			if(!netTransport::Debugging::dbg_logPacketUnreliable)
				return false;
			break;
		}
		
		switch(messageId)
		{
		default:
			{
				return false;
			}
		case MESSAGE_ID_PACKET_ACK:
			{
				const MsgPacketAck* ack = (const MsgPacketAck*)msg;

				LOGINFO(netDebug::g_SendPacketLog, "<StreamPacketSend(%d, %d)> packet(%d, %d) receive ack(%d, %d)", 
							m_socketPort,
							m_guid.m_guid,
							ack->m_packetId, 
							ack->m_packetSqn, 
							ack->m_packetPointer,
							ack->m_payloadSize);
				break;
			}
		case MESSAGE_ID_PACKET_FRAGMENT:
			{
				const MsgPacketFragment* frag = (const MsgPacketFragment*)msg;

				LOGINFO(netDebug::g_SendPacketLog, "<StreamPacketSend(%d, %d)> packet(%d, %d) send %s fragment %s(%d bytes), payload(%d bytes).", 
										m_socketPort,
										m_guid.m_guid,
										frag->m_packetId, 
										frag->m_packetSqn, 
										frag->m_reliable? "reliable" : "unreliable",
										(!frag->m_packetPointer)? "length" : "ptr", 
										(!frag->m_packetPointer)? frag->m_packetSize : frag->m_packetPointer, 
										frag->m_payloadSize);
				break;
			}
		case MESSAGE_ID_PACKET:
			{
				const MsgPacket* pack = (const MsgPacket*)msg;

				LOGINFO(netDebug::g_SendPacketLog, "<StreamPacketSend(%d, %d)> packet(%d) sent length(%d bytes).", 
								m_socketPort,
								m_guid.m_guid,
								pack->m_packetId, 
								pack->m_payloadSize);
				break;
			}
		}
		return true;
	}

	// start a new transmission.
	bool StreamPacketSend::transmissionStart(unsigned int sqn, bool reliable)
	{
		if(error())
			return false;

		// reset read pointer. 
		setReadByteCount(0);

		// clear fragment queue.
		m_fragments.clear();
		
		// empty packet. no need to do any real work.
		if(getWriteByteCount() == 0)
			return true;
		
		m_packetSqn = sqn;
		m_packetReliable = reliable;
		setMode(MODE_READ | MODE_WRITE);
		
		// started a new packet
		if(Debugging::dbg_logPacketReliable || Debugging::dbg_logPacketUnreliable)
		{
			LOGINFO(netDebug::g_SendPacketLog, "<StreamPacketSend(%d, %d)> starting packet(%d, %d) length(%d bytes), reliable(%s)", 
						m_socketPort,
						m_guid.m_guid,
						m_packetId, 
						m_packetSqn, 
						getWriteByteCount(),
						m_packetReliable? "true" : "false");
		}

		// try a quick sneaky transmit (for small frequent updates).
		if(transmitSinglePacket())
			return true;
	
		// else, slice data into fragments, and cache them for transmission.
		return buildFragments();
	}

	bool StreamPacketSend::transmitSinglePacket()
	{
		// packet needs a ack.
		if(m_packetReliable)
			return false;

		// packet too large to fit in one MTU.
		if(getWriteByteCount() > MsgPacket::MAX_PAYLOAD)
			return false;

		// build quick packet.
		setReadByteCount(0);
		
		MsgPacket packet;
		packet.m_packetId = m_packetId;
		packet.m_payloadSize = getWriteByteCount();

		// gah!
		if(!readBytes(packet.m_payload, getWriteByteCount()))
			return false;

		// attempt to send the packet. 
		if(!m_socket->send(&packet, m_socketPort, m_guid))
			return false;

		// log our transmission.
		logMessage(&packet);

		return true;

	}

	// the stream needs to be sliced up.
	bool StreamPacketSend::buildFragments()
	{
		// reset read pointer.
		setReadByteCount(0);

		// while we have not finish the segmentation...
		while(!eof())
		{
			// build one fragment packet
			int position = getReadByteCount();
			int size = (readRemaining() > netTransport::MsgPacketFragment::MAX_PAYLOAD)? netTransport::MsgPacketFragment::MAX_PAYLOAD : readRemaining();

			netTransport::MsgPacketFragmentRef fragment = new netTransport::MsgPacketFragment(position, size, m_packetReliable);

			if(!readBytes(fragment->m_payload, size))
				return false;

			// setup fragment info
			fragment->m_packetId	 = m_packetId;
			fragment->m_packetSqn	 = m_packetSqn;
			fragment->m_packetSize	 = getWriteByteCount();
			fragment->m_payloadSize  = size;
			fragment->m_maxPayload	 = size;
			fragment->m_minPayload	 = size;
			fragment->m_timestamp	 = netClock::instance()->now();
			
			// queue fragment to the list of stuff that needs to be transmitted.
			m_fragments.addTail(fragment);
		}
		return true;
	}

	// removed aknowledged fragment from the resend queue.
	bool StreamPacketSend::ackFragment(const MsgPacketAck* ack)
	{
		bdLinkedList<netTransport::MsgPacketFragmentRef>::Position pos = m_fragments.getHeadPosition();
		while(pos)
		{
			netTransport::MsgPacketFragmentRef fragment = m_fragments.getAt(pos);
			
			// this is our fragment.
			if(fragment->m_packetSqn == ack->m_packetSqn &&
				fragment->m_packetId == ack->m_packetId && 
				fragment->m_packetPointer == ack->m_packetPointer)
			{
				// sanity check. Make sure other side received what we originally transmitted.
				if(fragment->m_payloadSize != ack->m_payloadSize)
				{
					LOGERROR(netDebug::g_SendPacketLog, "<StreamPacketSend(%d, %d)> acked fragment(%d) mismatch in size(%d instead of %d)", 
								m_socketPort,
								m_guid.m_guid,
								fragment->m_packetPointer, 
								ack->m_payloadSize, 
								fragment->m_payloadSize);
				}
				// we can remove that from the resend queue.
				else
				{
					m_fragments.removeAt(pos);
					return true;
				}
			}
			// check next fragment in queue.
			m_fragments.forward(pos);
		}
		return true;
	}

	bool StreamPacketSend::transmit()
	{
		// nothing to transmit. 
		if(transmissionFinished())
			return false;

		// find fragment to send (one that has been sent the earliest).
		bdLinkedList<netTransport::MsgPacketFragmentRef>::Position pos = m_fragments.getHeadPosition();
		bdLinkedList<netTransport::MsgPacketFragmentRef>::Position posToSend = NULL;
		netTransport::MsgPacketFragmentRef fragmentToSend;
		
		while(pos)
		{
			netTransport::MsgPacketFragmentRef fragment = m_fragments.getAt(pos);
			
			if(fragmentToSend.isNull() || (fragment->m_timestamp < fragmentToSend->m_timestamp))
			{
				fragmentToSend = fragment;
				posToSend = pos;
			}
			m_fragments.forward(pos);
		}

		// send/resend the earliest fragment
		bool sent = m_socket->send(fragmentToSend, m_socketPort, m_guid);
		
		// completely throttled. nothing that could be sent.
		if(!sent)
			return false;

		// time-stamp the fragment
		fragmentToSend->m_timestamp = netClock::instance()->now();

		// if not reliable, remove from the list as soon as we send it.
		if(!fragmentToSend->m_reliable)
		{
			m_fragments.removeAt(posToSend);
		}
	
		logMessage(fragmentToSend);
		
		return ok();
	}
	
	// receive an ack.
	bool StreamPacketSend::receive(const Message* msg)
	{
		if(msg->messageId() != MESSAGE_ID_PACKET_ACK)
			return false;

		const MsgPacketAck* ack = (const MsgPacketAck*)msg;

		// wrong packet Id. skip.
		if(ack->m_packetId != m_packetId)
			return false;
		
		// remove agknowledged fragment from the list.
		ackFragment(ack);

		logMessage(msg);
		
		return true;
	}

	// we have finish transmitting stuff.
	bool StreamPacketSend::transmissionFinished()
	{
		// break if error detected
		if(error())
			return true;

		// no more fragments to transmit.
		if(!m_fragments.isEmpty())
			return false;

		setMode(MODE_READ | MODE_WRITE);
		return true;
	}

	StreamPacketReceive::StreamPacketReceive(SegmentPoolRef segmentPool, int maxSize, unsigned char packetId, SocketRef socket, int socketPort, ConnGuid guid)
	: SegmentStream(segmentPool)
	, m_socket(socket)
	, m_socketPort(socketPort)
	, m_guid(guid)
	, m_packetSqn(0)
	, m_packetId(packetId)
	, m_packetLimit(maxSize)
	, m_packetSize(0)
	{
		// reset tjhe packet state to 0.
		reset(0);
	}

	// reset the curent packet state. 
	bool StreamPacketReceive::reset(unsigned int sqn)
	{
		clear();
		setMode(MODE_READ | MODE_WRITE);
		
		// theoretical packet size limit.
		m_packetSize = m_packetLimit;
		setWriteByteLimit(m_packetSize);
		m_packetSqn = sqn;
	
		if(Debugging::dbg_logPacketReliable || Debugging::dbg_logPacketUnreliable)
		{			
			LOGINFO(netDebug::g_RecvPacketLog, "<StreamPacketRecv(%d, %d)> reset packet(%d, %d).", m_socketPort, m_guid.m_guid, m_packetId, m_packetSqn);
		}		
		return true;
	}

	// receive and process a message.
	bool StreamPacketReceive::receive(const Message* msg)
	{
		if(msg->messageId() == MESSAGE_ID_PACKET_FRAGMENT)
			return receiveFragment((const MsgPacketFragment*) msg);

		if(msg->messageId() == MESSAGE_ID_PACKET)
			return receiveSinglePacket((const MsgPacket*) msg);

		return true;
	}

	// single packet burst transmission.
	bool StreamPacketReceive::receiveSinglePacket(const MsgPacket* packet)
	{
		// wrong packet Id.
		if(packet->m_packetId != m_packetId)
			return false;
	
		// reset cuurent packet data.
		clear();

		// extract data.
		if(!writeBytes(packet->m_payload, packet->m_payloadSize))
		{
			clear();
			return false;
		}

		// total packet size
		m_packetSize = packet->m_payloadSize;
		setWriteByteLimit(packet->m_payloadSize);
		
		// log transmission
		logMessage(packet);

		return true;
	}
	
	// receive part of a packet
	bool StreamPacketReceive::receiveFragment(const MsgPacketFragment* fragment)
	{
		// wrong packet Id
		if(fragment->m_packetId != m_packetId)
			return false;
		
		// always return an ack so other side can flush its fragment queue.
		if(fragment->m_reliable)
			ackSegment(fragment);

		// old packet. ignore.
		if(fragment->m_packetSqn < m_packetSqn)
			return true;

		// new packet. reset stream.
		if(fragment->m_packetSqn > m_packetSqn)
			reset(fragment->m_packetSqn);


		// tuning to Sanity FM.
		BIZ_ASSERT(fragment->m_packetPointer >= 0);
		
		// if the pointer is 0, we have a starting packet
		// the packet length is valid
		if(fragment->m_packetPointer == 0)
		{
			BIZ_ASSERT_MSG((fragment->m_packetSize <= (unsigned int) m_packetLimit), "packet very large size(%d kB), limit(%d kB).", fragment->m_packetSize, m_packetLimit);
			m_packetSize = fragment->m_packetSize;
			setWriteByteLimit(m_packetSize);
		}

		// reset write pointer.
		setWriteByteCount(fragment->m_packetPointer);
		
		// write message data
		if(!writeBytes(fragment->m_payload, fragment->m_payloadSize))
			return false;
		
		// log transmission
		logMessage(fragment);
	
		return ok();
	}

	bool StreamPacketReceive::ackSegment(const MsgPacketFragment* fragment)
	{
		// fragment not reliable. no need to send an ack.
		if(!fragment->m_reliable)
			return true;

		// setup ack.
		MsgPacketAck msg;
		msg.m_packetId		= fragment->m_packetId;
		msg.m_packetSqn		= fragment->m_packetSqn;
		msg.m_packetPointer = fragment->m_packetPointer;
		msg.m_payloadSize	= fragment->m_payloadSize;
		
		// send ack.
		if(!m_socket->send(&msg, m_socketPort, m_guid))
			return false;

		// log transmission.
		logMessage(&msg);

		return ok();
	}
	
	bool StreamPacketReceive::complete()
	{
		if(m_packetSize == 0)
			return false;

		// we have holes in the packet.
		if(!SegmentStream::completed(m_packetSize))
			return false;

		setMode(MODE_READ | MODE_WRITE);
		setReadByteCount(0);
		setWriteByteCount(m_packetSize);

		if(Debugging::dbg_logPacketReliable || Debugging::dbg_logPacketUnreliable)
		{			
			LOGINFO(netDebug::g_RecvPacketLog, "<StreamPacketRecv(%d, %d)> packet(%d, %d) complete length(%d).", m_socketPort, m_guid.m_guid, m_packetId, m_packetSqn, m_packetSize);
		}		

		return true;
	}

	bool StreamPacketReceive::logMessage(const Message* msg)
	{
		int messageId = msg->messageId();

		if(m_socketPort != netTransport::Debugging::dbg_logPacketSocketPort && netTransport::Debugging::dbg_logPacketSocketPort >= 0)
			return false;
		
		if(m_guid.m_guid != netTransport::Debugging::dbg_logPacketOrigin && netTransport::Debugging::dbg_logPacketOrigin >= 0)
			return false;

		switch(messageId)
		{
		default:
			return false;
		case MESSAGE_ID_PACKET_ACK:
		case MESSAGE_ID_PACKET_FRAGMENT:
			if(!netTransport::Debugging::dbg_logPacketReliable)
				return false;
			break;
		case MESSAGE_ID_PACKET:
			if(!netTransport::Debugging::dbg_logPacketUnreliable)
				return false;
			break;
		}

		switch(messageId)
		{
		default:
			{
				return false;
			}
		case MESSAGE_ID_PACKET_ACK:
			{
				const MsgPacketAck* ack = (const MsgPacketAck*)msg;

				LOGINFO(netDebug::g_RecvPacketLog, "<StreamPacketRecv(%d, %d)> packet(%d, %d) send ack(%d, %d)", 
						m_socketPort,
						m_guid.m_guid,
						ack->m_packetId, 
						ack->m_packetSqn, 
						ack->m_packetPointer,
						ack->m_payloadSize);
				break;
			}
		case MESSAGE_ID_PACKET_FRAGMENT:
			{
				const MsgPacketFragment* frag = (const MsgPacketFragment*)msg;
		
				LOGINFO(netDebug::g_RecvPacketLog, "<StreamPacketRecv(%d, %d)> packet(%d, %d) received %s fragment %s(%d bytes), payload(%d bytes).", 
							m_socketPort,
							m_guid.m_guid,
							frag->m_packetId, 
							frag->m_packetSqn, 
							frag->m_reliable? "reliable" : "unreliable",
							(!frag->m_packetPointer)? "length" : "ptr", 
							(!frag->m_packetPointer)? frag->m_packetSize : frag->m_packetPointer, 
							frag->m_payloadSize);

				break;
			}
		case MESSAGE_ID_PACKET:
			{
				const MsgPacket* pack = (const MsgPacket*)msg;
				LOGINFO(netDebug::g_SendPacketLog, "<StreamPacketSend(%d, %d)> packet(%d) received length(%d bytes).", 
							m_socketPort,
							m_guid.m_guid,
							pack->m_packetId, 
							pack->m_payloadSize);
				break;
			}
		}
		return true;
	}		
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

