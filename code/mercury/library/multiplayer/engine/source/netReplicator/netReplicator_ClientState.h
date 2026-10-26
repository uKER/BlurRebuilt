#ifndef NETREPLICATOR_CLIENTSTATE_H
#define NETREPLICATOR_CLIENTSTATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netTime/netTime.h"
#include "netTransport/netTransport_ConnGuid.h"
#include "netTransport/netTransport_Socket.h"
#include "netTransport/netTransport_SegmentPool.h"
#include "netTransport/netTransport_StreamPacket.h"
#include "netTransport/netTransport_Message.h"

namespace netReplicator
{
	BD_REFERENCE(ClientState);
	class ClientState: public bdReferencable
	{
	public:
		ClientState(int stateId, const netTransport::ConnGuid& self, netTransport::SocketRef socket, int socketPort);
		~ClientState();

		bool enable(bool onOff);
		bool sendingReliableData();	
		bool receive(const netTransport::Message* msg);
		bool poll();
			
		const netTransport::ConnGuid m_guid;
		const int m_socketPort;
		const int m_stateId;
		netTransport::SocketRef m_socket;
		
		netStopwatch m_sendTick;
		bool m_reliable;
		bool m_enabled;
		netStopwatch m_reliableDelay;

		// cached segment allocations.
		netTransport::SegmentPoolRef m_segmentPool;
		netTransport::StreamPacketSendRef m_sendPacket;
		netTransport::StreamPacketReceiveRef m_recvPacket;

		// build new packet and send
		bool sendPacket();
	};
}

#endif //NETREPLICATOR_CLIENTSTATE_H