
#include "stdafx.h"
#include "./netReplicator_Replicator.h"
#include "netDebug/netDebug.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netReplicator
{
	Replicator::Replicator(	const char* debugName, 
							int maxClients, int maxEntities, 
							netTransport::ConnGuid self, 
							netTransport::SocketRef socket, int socketPort)
	: m_debugName(debugName)
	, m_maxClients(maxClients)
	, m_maxEntities(maxEntities)
	, m_self(self)
	, m_socket(socket)
	, m_socketPort(socketPort)
	{
		LOGINFO(netDebug::g_Log, "<netReplicator::Replicator> created replicator('%s') port(%d)", m_debugName, m_socketPort);
	}

	Replicator::~Replicator()
	{
		LOGINFO(netDebug::g_Log, "<netReplicator::Replicator> destroyed replicator('%s') port(%d)", m_debugName, m_socketPort);
	}
	
	StateRef Replicator::state(int stateId)
	{
		StateRef state;
		m_states.get(stateId, state);
		return state;
	}

	bool Replicator::addState(int stateId, const char* debugName, U64 updateTickPeriod)
	{
		if(m_states.containsKey(stateId))
		{
			LOGERROR(netDebug::g_Log, "<netReplicator::Replicator> already has state(%d).", stateId);
			return false;
		}

		StateRef state = new State(stateId, debugName, m_maxClients, m_maxEntities, m_self, updateTickPeriod);

		state->setHost(m_host);

		m_states.put(stateId, state);
		return true;
	}

	bool Replicator::removeState(int stateId)
	{
		m_states.remove(stateId);
		return true;
	}

	bool Replicator::sendingReliableData()
	{
		bdHashMap<int, ClientRef>::Iterator it = m_clients.getIterator();
		while(it)
		{
			ClientRef client = m_clients.getValue(it);
			m_clients.next(it);

			if(client->sendingReliableData())
			{
				m_states.releaseIterator(it);
				return true;
			}
		}
		m_states.releaseIterator(it);
		return false;
	}

	bool Replicator::migrating()
	{
		bdHashMap<int, StateRef>::Iterator it = m_states.getIterator();
		while(it)
		{
			StateRef state = m_states.getValue(it);
			m_states.next(it);
		
			if(state->migrating())
			{
				m_states.releaseIterator(it);
				return true;
			}
		}
		m_states.releaseIterator(it);
		return false;
	}

	int Replicator::listClientsForPromotion(int* clients, int maxClients)
	{
		bdHashMap<int, StateRef>::Iterator it = m_states.getIterator();
		while(it)
		{
			StateRef state = m_states.getValue(it);
			m_states.next(it);

			int clientsCount = state->listClientsForPromotion(clients, maxClients);
			if(clientsCount > 0)
			{
				return clientsCount;
			}
		}
		m_states.releaseIterator(it);
		return 0;
	}
		
	int Replicator::migrationWeight(int hostGuid)
	{
		bdHashMap<int, StateRef>::Iterator it = m_states.getIterator();
		while(it)
		{
			StateRef state = m_states.getValue(it);
			m_states.next(it);

			int stateMigrationWeight = state->migrationWeight(hostGuid);
			
			if(stateMigrationWeight >= 0)
			{
				m_states.releaseIterator(it);
				return stateMigrationWeight;
			}
		}
		m_states.releaseIterator(it);
		return -1;
		
	}

	// clients
	ClientRef Replicator::client(const netTransport::ConnGuid& guid)
	{
		ClientRef client;
		m_clients.get(guid, client);
		return client;
	}

	bool Replicator::addClient(const netTransport::ConnGuid& guid)
	{
		if(m_clients.containsKey(guid))
		{
			LOGERROR(netDebug::g_Log, "<netReplicator::Replicator(%s)> already has client(%d).", m_debugName, guid.m_guid);
			return false;
		}

		// new client to insert
		ClientRef newClient = new Client(guid, m_socket, m_socketPort);

		if(!m_clients.put(guid, newClient))
		{
			assert(false);
			return false;
		}

		// now register clients with game states.
		bdHashMap<int, StateRef>::Iterator it = m_states.getIterator();
		while(it)
		{
			StateRef state = m_states.getValue(it);
			m_states.next(it);

			// register client with that game state.
			newClient->addClientState(state->stateId());
		}
		m_states.releaseIterator(it);
		return true;
	}

	bool Replicator::removeClient(const netTransport::ConnGuid& guid)
	{
		m_clients.remove(guid);
		return true;
	}

	bool Replicator::enableClientStateUpdates(const netTransport::ConnGuid& guid, int stateId, bool onOff)
	{
		ClientRef theclient = client(guid);
		if(theclient.isNull()) return false;
		return theclient->enableClientStateUpdates(stateId, onOff);
	}

	bool Replicator::setHost(const netTransport::ConnGuid& host)
	{
		if(host == m_host)
			return true;

		m_host = host;
		return true;
	}
		
	bool Replicator::poll()
	{
		pollHosts();
		pollStates();
		pollClients();
		return true;
	}

	bool Replicator::pollHosts()
	{
		bdHashMap<int, StateRef>::Iterator it = m_states.getIterator();
		while(it)
		{
			StateRef state = m_states.getValue(it);
			m_states.next(it);
		
			if(state->getHost() != m_host)
			{
				state->setHost(m_host);
			}
		}
		m_states.releaseIterator(it);
		return true;
	}

	bool Replicator::pollStates()
	{
		bdHashMap<int, StateRef>::Iterator it = m_states.getIterator();
		while(it)
		{
			StateRef state = m_states.getValue(it);
			m_states.next(it);
			state->poll();
		}
		m_states.releaseIterator(it);
		return true;
	}

	bool Replicator::pollClients()
	{	
		bdHashMap<int, ClientRef>::Iterator it = m_clients.getIterator();
		while(it)
		{
			ClientRef client = m_clients.getValue(it);
			m_clients.next(it);
			client->poll();
		}
		m_clients.releaseIterator(it);
		return true;
	}

	bool Replicator::receive(const netTransport::ConnGuid& from, const netTransport::Message* msg)
	{
		ClientRef c = client(from);
		if(c.isNull()) return false;
		return c->receive(msg);
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif