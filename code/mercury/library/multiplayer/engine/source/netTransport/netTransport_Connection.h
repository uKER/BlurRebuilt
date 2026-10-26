#ifndef NETTRANSPORT_CONNECTION_H
#define NETTRANSPORT_CONNECTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netTransport_Event.h"
#include "netTransport_TrafficLogger.h"
#include "netTransport_Enums.h"
#include "netTransport_Socket.h"
#include "netTime/netTime.h"

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdSocket/bdSecurityKey.h>
#include <bdCore/bdSocket/bdSecurityID.h>

namespace netTransport
{
	BD_REFERENCE(Connection);
	class Connection: public bdReferencable
	{
	public:
	
		Connection(	SocketRef socket, unsigned char socketPort,
					bool online, const bdSecurityID& secId, const bdSecurityKey& secKey,
					ConnGuid localGuid, ConnGuid remoteGuid);

		~Connection();

		// accessors
		ConnGuid remoteGuid() const { return m_remoteGuid; }
		ConnGuid localGuid() const { return m_localGuid; }
		
		bdCommonAddrRef secAddr();
		void setForwardHandle(bdAddrHandleRef forwardHandle);
		bdAddrHandleRef	getAddrHandle();

		int qosRating() const { return m_qosRating; }
		ConnGuid guid() const { return m_remoteGuid; }
		int state() const { return m_state; }
		int closeReason() const { return m_closeReason; }
		const Addr& addr() const { return m_connectAddr; }
		netTime idleTime() { return m_idleWatch.elapsedTime(); }

		const Addr& connectAddr() const { return m_connectAddr; }
		const netNonce& connectKey() const { return m_connectKey; }
		
		bool connecting() const  { return m_state == CONN_STATE_CONNECTING; }
		bool reserved() const  { return m_state == CONN_STATE_RESERVED; }
		bool pending() const  { return m_state == CONN_STATE_PENDING; }
		bool accepting() const  { return m_state == CONN_STATE_ACCEPTING; }
		bool established() const  { return m_state == CONN_STATE_ESTABLISHED; }
		bool unresponsive() const { return m_state == CONN_STATE_UNRESPONSIVE; }
		bool closing() const  { return m_state == CONN_STATE_CLOSING; }
		bool closed() const  { return m_state == CONN_STATE_CLOSED; }
		bool destroyed() const { return m_state == CONN_STATE_NOT_SET; }
		
		// three possible initial state of a connection
		bool peer(const Addr& addr);
		bool reserve(const Addr& addr);
		bool connect(const Addr& addr, netStream* joinParams);
		
		// state transitions
		bool migrate(const bdSecurityID& secId, const bdSecurityKey& secKey);
		bool accept();
		bool close(int reason);
		bool destroy();

		// interface
		EventRef poll();
		void	flush();
		
		bool verifyPacket(ConnPacket* packet);
		bool handleMessage(ConnPacket* packet, Message* msg);
		bool sendMessage(Message* msg);
		bool forwardPacket(ConnPacket* packet);
		bool sendPacket();
		
		// bandwidth management
		void setRates(float sendBpS, float recvBpS) { m_sendBpS = sendBpS; m_recvBpS = recvBpS; }
		void getOutboundTraffic(TrafficSample& report);
		void getInboundTraffic(TrafficSample& report);
		float getLatency() const  { return m_latency; }

		const char* stateToString() const;

	private:
		bool analysePacket(ConnPacket* packet, bool forwarded, bool outgoing);
		bool handleMessageInternal(ConnPacket* packet, Message* msg);
		
		// state transitions
		bool updateQoS();
		bool updateState();
		bool updatePing();
		bool setState(int state);
		bool reconnect();

		// routing
		SocketRef m_socket;
		unsigned char m_socketPort;
		Addr m_connectAddr;
		netNonce m_connectKey;
		ConnGuid m_localGuid;
		ConnGuid m_remoteGuid;
		
		// state
		char m_state;
		int	m_closeReason;
		netStopwatch m_idleWatch;
		netStopwatch m_reconnectWatch;
		netStopwatch m_stateWatch;
		netStopwatch m_msgWatch;
		int m_recvCloseCount;
		netStopwatch m_debugWatch;
		netTime m_sendKeepAliveTime;
				
		// security info
		bool m_online;
		bdSecurityID m_secId;
		bdSecurityKey m_secKey;
		bdAddrHandleRef	m_secHandle;
		bdAddrHandleRef	m_forwardHandle;
		bdCommonAddrRef m_secAddr;

		// connection request payload
		netVector m_joinParams;

		// message cached into a packet
		ConnPacket m_cachedPacket;

		// events returned during polling
		bdLinkedList<EventRef> m_events;

		// bandwidth management
		float m_sendBpS; // throttle the number of bytes we can send
		float m_recvBpS; // the theretical limit of the bytes we can receive
		float m_localQoS;  // basically, throttling state. < 0 means, we've overloaded the connection.
		float m_remoteQoS; // throttle state on the other side. used for determining forwarding handles. 

		TrafficSample m_sendStats;
		TrafficSample m_recvStats;
		netStopwatch  m_sendWatch;
		netStopwatch  m_trafficWatch;

		// signals how good the link is between us and end point
		int m_qosRating;
		netStopwatch m_qosRatingWatch; // only refresh every 5 seconds of so.

		TrafficLoggerRef m_outstreamLogger; // logs outgoing bytes
		TrafficLoggerRef m_instreamLogger; // logs incoming bytes

		struct Latency
		{
			Latency() : m_pingTime(0), m_pongTime(0) {}
			U64 m_pingTime;
			U64 m_pongTime;
		};

		enum { MAX_LATENCIES = 16 };
		Latency m_latencies[MAX_LATENCIES];
		const Latency* startPing(U64 timestamp);
		const Latency* stopPing(U64 timestamp);
		F32 calculateLatency();
		F32 m_latency;
	};
}


#endif //NETTRANSPORT_CONNECTION_H