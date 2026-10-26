#ifndef NETREPLICATOR_STATE_H
#define NETREPLICATOR_STATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include "netTime/netTime.h"
#include "netTransport/netTransport_ConnGuid.h"

namespace netReplicator
{
	BD_REFERENCE(State);
	class State: public bdReferencable
	{
	public:
		State(	int stateId, const char* debugName, 
				int maxClients, int maxEntities, 
				const netTransport::ConnGuid& self,
				U64 updateTickPeriod);
		~State();

		int listClientsForPromotion(int* clients, int maxClients);	
		bool migrating();
		int migrationWeight(int hostGuid);
		int stateId() const { return m_stateId; }
		void setUpdateTickPeriod(U64 periodInMicroseconds) { m_updateTickPeriod = periodInMicroseconds; }
		bool poll();

		const char* debugName() const { return m_debugName; }
		
		void setHost(const netTransport::ConnGuid& hostGuid);
		netTransport::ConnGuid getHost();
		
		const char* m_debugName;
		const int m_stateId;
		const netTransport::ConnGuid m_self;

		// ticks network updates
		netStopwatch m_updateTick;
		
		// ticks network updates
		U64 m_updateTickPeriod;
	};
}
#endif //NETREPLICATOR_STATE_H