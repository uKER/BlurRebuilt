#ifndef NETREPLICATOR_REPLICATOR_H
#define NETREPLICATOR_REPLICATOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netReplicator/netReplicator_Client.h"
#include "netReplicator/netReplicator_State.h"

namespace netReplicator
{
	BD_REFERENCE(Replicator);
	class Replicator: public bdReferencable
	{
	public:
		Replicator(	const char* debugName, 
					int maxClients, int maxEntities, 
					netTransport::ConnGuid self, 
					netTransport::SocketRef socket, int socketPort);
		~Replicator();

		const char* debugName() const { return m_debugName; }

		// game state management
		bool addState(int stateId, const char* debugName, U64 updateTickPeriod);
		StateRef state(int stateId);
		bool removeState(int stateId);
		int migrationWeight(int hostGuid);
		int listClientsForPromotion(int* clients, int maxClients);			
		bool sendingReliableData();
		bool migrating();

		// state clients management
		ClientRef client(const netTransport::ConnGuid& guid);
		bool addClient(const netTransport::ConnGuid& guid);
		bool removeClient(const netTransport::ConnGuid& guid);
		bool enableClientStateUpdates(const netTransport::ConnGuid& guid, int stateId, bool onOff);
		bdHashMap<int, ClientRef>& clients () { return m_clients; }

		// processing
		bool setHost(const netTransport::ConnGuid& host);
		bool receive(const netTransport::ConnGuid& from, const netTransport::Message* msg);
		bool poll();

		// transport components
		const char* m_debugName;
		const int m_maxClients;
		const int m_maxEntities;
		netTransport::ConnGuid m_self;
		netTransport::SocketRef m_socket;
		int m_socketPort;

		// host cache
		netTransport::ConnGuid m_host;

		// game states, sorted by GameStateIds.
		bdHashMap<int, StateRef> m_states;
		bdHashMap<int, ClientRef> m_clients;

	private:
		bool pollClients();
		bool pollStates();
		bool pollHosts();
	};
}
#endif //NETREPLICATOR_REPLICATOR_H