#ifndef NETTRANSPORT_EVENT_H
#define NETTRANSPORT_EVENT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netTransport_Enums.h"
#include "netTransport_ConnGuid.h"
#include "netStream/netVector.h"

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

namespace netTransport
{
	enum
	{
		EVENT_ID_NOT_SET=-1,
		EVENT_ID_CONNECT_PENDING, // connection waiting for accept
		EVENT_ID_CONNECT_ACCEPTED, // connection request has been accepted by host
		EVENT_ID_CONNECT_ESTABLISHED, // connection is now established
		EVENT_ID_CONNECT_UNRESPONSIVE, // connection became unresponsive
		EVENT_ID_CONNECT_RECOVERED, // connection has finally responded
		EVENT_ID_CONNECT_LATENCY, // latency report
		EVENT_ID_CONNECT_CLOSED, // connection closed
		EVENT_ID_CONNECT_DESTROYED, // connection destroyed
		EVENT_ID_GUID_ASSIGNED, // host assigned us guids
		EVENT_ID_MESH_HOSTING, // we're hosting the session
		EVENT_ID_MESH_PEERING, // we're peering in the session
		EVENT_ID_MESH_CLOSED, // we've closed our links to the session
		EVENT_ID_COUNT,
	};

	BD_REFERENCE(Event);
	class Event: public bdReferencable
	{
	public:
		Event(int eventId, const char* debugName)
		: m_eventId(eventId)
		, m_debugName(debugName)
		{}
		virtual ~Event() 
		{}
		
		const char* debugName() const { return m_debugName; }
		const char* m_debugName;
		const int m_eventId;
	};

	BD_REFERENCE(Event_Connection);
	class Event_Connection: public Event
	{
	public:
		Event_Connection(int id, const char* debugName, ConnGuid guid)
		: Event(id, debugName)
		, m_guid(guid)
		{}
		const ConnGuid m_guid; 
	};


	BD_REFERENCE(Event_ConnPending);
	class Event_ConnPending: public Event_Connection
	{
	public:
		Event_ConnPending(ConnGuid guid, netStream* joinParams)
		: Event_Connection(EVENT_ID_CONNECT_PENDING, "ConnPending", guid)
		, m_joinParams(joinParams->getWriteByteCount())
		{
			m_joinParams.copy(joinParams);
		}

		netVector m_joinParams;
	};

	BD_REFERENCE(Event_ConnAccepted);
	class Event_ConnAccepted: public Event_Connection
	{
	public:
		Event_ConnAccepted(ConnGuid assignedGuid, ConnGuid hostGuid)
		: Event_Connection(EVENT_ID_CONNECT_ACCEPTED, "ConnAccepted", assignedGuid)
		, m_assignedGuid(assignedGuid)
		, m_hostGuid(hostGuid)
		{}
		
		const ConnGuid m_assignedGuid; 
		const ConnGuid m_hostGuid; 
	};

	BD_REFERENCE(Event_ConnEstablished);
	class Event_ConnEstablished: public Event_Connection
	{
	public:
		Event_ConnEstablished(ConnGuid guid)
		: Event_Connection(EVENT_ID_CONNECT_ESTABLISHED, "ConnEstablished", guid)
		{}
	};

	BD_REFERENCE(Event_ConnUnresponsive);
	class Event_ConnUnresponsive: public Event_Connection
	{
	public:
		Event_ConnUnresponsive(ConnGuid guid)
		: Event_Connection(EVENT_ID_CONNECT_UNRESPONSIVE, "ConnUnresponsive", guid)
		{}
	};

	BD_REFERENCE(Event_ConnRecovered);
	class Event_ConnRecovered: public Event_Connection
	{
	public:
		Event_ConnRecovered(ConnGuid guid)
		: Event_Connection(EVENT_ID_CONNECT_RECOVERED, "ConnRecovered", guid)
		{}
	};

	BD_REFERENCE(Event_ConnLatency);
	class Event_ConnLatency: public Event_Connection
	{
	public:
		Event_ConnLatency(ConnGuid guid, U64 latency)
		: Event_Connection(EVENT_ID_CONNECT_LATENCY, "ConnLatency", guid)
		, m_latency(latency)
		{}
		const U64 m_latency;
	};

	BD_REFERENCE(Event_ConnClosed);
	class Event_ConnClosed: public Event_Connection
	{
	public:
		Event_ConnClosed(ConnGuid guid, int reason)
		: Event_Connection(EVENT_ID_CONNECT_CLOSED, "ConnClosed", guid)
		, m_reason(reason)
		{}
		const int m_reason;
	};

	BD_REFERENCE(Event_GuidAssigned);
	class Event_GuidAssigned: public Event
	{
	public:
		Event_GuidAssigned(ConnGuid selfGuid, ConnGuid hostGuid)
		: Event(EVENT_ID_GUID_ASSIGNED, "GuidAssigned")
		, m_selfGuid(selfGuid)
		, m_hostGuid(hostGuid)
		{}

		const ConnGuid m_selfGuid; 
		const ConnGuid m_hostGuid; 
	};
	

	BD_REFERENCE(Event_MeshHosting);
	class Event_MeshHosting: public Event
	{
	public:
		Event_MeshHosting(ConnGuid selfGuid)
		: Event(EVENT_ID_MESH_HOSTING, "MeshHosting")
		, m_selfGuid(selfGuid)
		{}

		const ConnGuid m_selfGuid; 
	};
	
	BD_REFERENCE(Event_MeshPeering);
	class Event_MeshPeering: public Event
	{
	public:
		Event_MeshPeering(ConnGuid selfGuid, ConnGuid hostGuid)
		: Event(EVENT_ID_MESH_PEERING, "MeshPeering")
		, m_selfGuid(selfGuid)
		, m_hostGuid(hostGuid)
		{}

		const ConnGuid m_selfGuid; 
		const ConnGuid m_hostGuid; 
	};

	BD_REFERENCE(Event_MeshClosed);
	class Event_MeshClosed: public Event
	{
	public:
		Event_MeshClosed(int reason)
		: Event(EVENT_ID_MESH_CLOSED, "MeshClosed")
		, m_reason(reason)
		{}
		const int m_reason;
	};
}

#endif //NETTRANSPORT_EVENT_H