#ifndef NETREPLICATOR_CLIENT_H
#define NETREPLICATOR_CLIENT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netReplicator/netReplicator_ClientState.h"
#include "netTime/netTime.h"
#include "netTransport/netTransport_ConnGuid.h"
#include "netTransport/netTransport_Socket.h"
#include "netTransport/netTransport_SegmentPool.h"
#include "netTransport/netTransport_StreamPacket.h"
#include "netTransport/netTransport_Message.h"

namespace netReplicator
{
	BD_REFERENCE(Client);
	class Client: public bdReferencable
	{
	public:
		Client(const netTransport::ConnGuid& self, netTransport::SocketRef socket, int socketPort);
		~Client();

		ClientStateRef clientState(int stateId);
		bool addClientState(int stateId);
		bool removeClientState(int stateId);
		bool enableClientStateUpdates(int stateId, bool onOff);	
		bool sendingReliableData();

		bool receive(const netTransport::Message* msg);
		bool poll();
			
		const netTransport::ConnGuid m_guid;
		const int m_socketPort;
		netTransport::SocketRef m_socket;
		bdHashMap<int, ClientStateRef> m_clientStates; // the client/states sending / receiving state updates.
	};
}

#endif //NETREPLICATOR_CLIENT_H