#ifndef NETTRANSPORT_STREAMPACKET_H
#define NETTRANSPORT_STREAMPACKET_H

#ifdef _MSC_VER
#pragma once
#endif


#include <netTransport/netTransport_ConnGuid.h>
#include <netTransport/netTransport_SegmentStream.h>
#include <netTransport/netTransport_Socket.h>
#include <netTime/netTime.h>

namespace netTransport
{
	BD_REFERENCE(MsgPacket);
	BD_REFERENCE(MsgPacketFragment);
	BD_REFERENCE(StreamPacketSend);
	class StreamPacketSend: public SegmentStream
	{
	public:
		StreamPacketSend(SegmentPoolRef segmentPool, unsigned char packetId, SocketRef socket, int socketPort, ConnGuid guid);

		bool transmissionReliable() { return m_packetReliable; }
		bool transmissionStart(unsigned int sqn, bool reliable);
		bool transmit();
		bool transmissionFinished();
		
		bool receive(const Message* msg);
		unsigned int sqn() const { return m_packetSqn; }

	private:
		SocketRef		m_socket;
		int				m_socketPort;
		ConnGuid		m_guid;
		
		unsigned char	m_packetId;
		unsigned int	m_packetSqn;
		unsigned int	m_packetPointer;
		bool			m_packetReliable;

		
		// packet split into fragments.
		bdLinkedList<netTransport::MsgPacketFragmentRef> m_fragments;
		
		bool transmitSinglePacket();
		bool buildFragments();
		bool ackFragment(const MsgPacketAck* ack);

		bool logMessage(const Message* msg);
		
	};

	BD_REFERENCE(StreamPacketReceive);
	class StreamPacketReceive: public SegmentStream
	{
	public:
		StreamPacketReceive(SegmentPoolRef segmentPool, int packetMaxSize, unsigned char packetId, SocketRef socket, int socketPort, ConnGuid guid);
		bool reset(unsigned int sqn);
		bool receive(const Message* msg);
		bool complete();
		bool sendAck(unsigned int fragmentPosition, unsigned int fragmentSize);
		unsigned int sqn() const  { return m_packetSqn; }
		
		const int		m_packetLimit;
		SocketRef		m_socket;
		int				m_socketPort;
		ConnGuid		m_guid;
		
		unsigned char	m_packetId;
		unsigned int	m_packetSize;
		unsigned int	m_packetSqn;

		bool logMessage(const Message* msg);
		bool receiveSinglePacket(const MsgPacket* packet);
		bool receiveFragment(const MsgPacketFragment* fragment);
		bool ackSegment(const MsgPacketFragment* fragment);
		
	};
}

#endif //NETTRANSPORT_STREAMPACKET_H