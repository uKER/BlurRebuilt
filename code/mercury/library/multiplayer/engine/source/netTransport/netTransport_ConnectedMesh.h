#ifndef NETTRANSPORT_CONNECTEDMESH_H
#define NETTRANSPORT_CONNECTEDMESH_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netTransport_Addr.h"
#include "netTransport_Connection.h"
#include "netTransport_ConnGuid.h"
#include "netTransport_Event.h"
#include "netTransport_LANDiscovery.h"
#include "netTransport_Peer.h"
#include "netTransport_Socket.h"
#include "netStream/netStream.h"

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

namespace netTransport
{
	BD_REFERENCE(RefusedConnection);
	BD_REFERENCE(ConnectedMesh);
	class ConnectedMesh: public SocketHandler, public bdReferencable
	{
	public:
		enum 
		{ 
			STATE_NOT_SET=-1,
			STATE_CONNECTING, 
			STATE_HOSTING,
			STATE_PEERING, 
			STATE_CLOSING,
			STATE_CLOSED, 
		};

		ConnectedMesh(bool online, int maxConnections, U32 titleId, SocketRef socket, unsigned char socketPort);
		virtual ~ConnectedMesh();

		bool host(const bdSecurityID& secId, const bdSecurityKey& secKey);
		bool join(const bdSecurityID& secId, const bdSecurityKey& secKey, bdCommonAddrRef secAddr, netStream* joinParams=NULL);
		bool setHost(const ConnGuid& hostGuid);
		bool setSecurity(const bdSecurityID& secId, const bdSecurityKey& secKey);
		bool accept(const ConnGuid& clientGuid);
		bool close(const ConnGuid& clientGuid, int reason);
		ConnGuid reserve(const Addr& addr);
		bool close();
		
		EventRef poll();
		bool sendMessage(Message* msg, ConnGuid to);		

		bool advertise(bdGameInfoRef info);
		bool advertising() const;

		bool setRates(float sendBpS, float recvBpS);
		void getOutboundTraffic(TrafficSample& report);
		void getInboundTraffic(TrafficSample& report);
		bool getLatency(float& latency, int& latencyCount);
			
		// peers
		bool syncPeers(const Peer* peers, int peerCount);
		bool getPeers(Peer* peers, int& peerCount, int maxPeers);
		bool setForwarder(const ConnGuid& peer, const ConnGuid& forwarder);
						
		const ConnGuid&	selfGuid() const { return m_selfGuid; }
		const ConnGuid&	hostGuid() const { return m_hostGuid; }
		
		bool connecting() const { return m_state == STATE_CONNECTING; }
		bool peering() const { return m_state == STATE_PEERING; }
		bool hosting() const { return m_state == STATE_HOSTING; }
		bool lostHost();
		bool closing() const { return m_state == STATE_CLOSING; }
		bool closed() const { return m_state == STATE_CLOSED; }
				
		ConnectionRef hostConnection() { return findConnection(hostGuid()); }
		ConnectionRef findConnection(const ConnGuid& guid);
		ConnectionRef findConnection(const Addr& addr);
		int	countConnections();
		bdHashMap<int, ConnectionRef>& connections() { return m_connections; }
				
		// state
		unsigned char port() const { return m_socketPort; }
		int	state() const { return m_state; }
		int closeReason() const	{ return m_closeReason; }
		
	protected:
		virtual bool verifyPacket(ConnPacket* packet);
		virtual bool handleMessage(ConnPacket* packet, Message* msg);

	private:
		ConnectionRef findConnection(const Addr& connectAddr, const netNonce& connectKey);
	
		bool removeConnection(ConnectionRef conn);
		bool pollConnection(ConnectionRef conn);
		void resetConnections();
		bool setState(int state);
		bool updateState();
		bool updateAdvertise();
		bool updateThrottling();
		bool updateForwarder();
		
	private:
		// packet transport
		SocketRef m_socket;
		const unsigned char m_socketPort;
		const U32 m_titleId;
		const int m_maxConnections;
		
		// fluff
		const bool				m_online;
		LANDiscoveryServerRef	m_discoveryServer;
		bdSecurityID			m_secId;
		bdSecurityKey			m_secKey;
		
		// self and host guid
		ConnGuid m_hostGuid;
		ConnGuid m_selfGuid;
		ConnGuidGenerator m_guidGenerator;

		// state
		bdInt m_state;
		int m_closeReason;
		bdGameInfoRef m_advertiseInfo;
		netStopwatch m_stateWatch;
		
		// bandwidth cap.
		float m_sendBpS;
		float m_recvBpS;

		// temporary connection.
		ConnectionRef m_connector;
		
		// connected mesh
		bdHashMap<int, ConnectionRef> m_connections;

		// events returned during polling
		bdLinkedList<EventRef> m_events;

	private:
		bool addRefusedConnection(const Addr& connectAddr, const netNonce& connectKey, int reason);
		bool updateRefusedConnections();
		bool clearRefusedConnection();
		RefusedConnectionRef findRefusedConnection(const Addr& connectAddr, const netNonce& connectKey);
		bdLinkedList<RefusedConnectionRef> m_refused;
	};
};
#endif //NETTRANSPORT_CONNECTEDMESH_H