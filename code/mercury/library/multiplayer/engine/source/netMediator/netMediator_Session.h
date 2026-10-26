#ifndef NETMEDIATOR_SESSION_H
#define NETMEDIATOR_SESSION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netEnums/netEnums.h"
#include "netMediator/netMediator_PeerState.h"
#include "netReplicator/netReplicator_Replicator.h"
#include "netSession/netSession.h"
#include "netStream/netStream.h"
#include "netTime/netTime.h"
#include "netTransport/netTransport_ConnectedMesh.h"
#include "netTransport/netTransport_Socket.h"
#include "netTransport/netTransport_TrafficAnalyser.h"

#include <misc_shared/messaging/MessageHandler.h>
#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/Messaging/MessageSubscriberSystem.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <system/debug/I_DebugRender.h>

BD_REFERENCE(netSessionCreateData);
BD_REFERENCE(netSessionJoinData);
BD_REFERENCE(netEnumeratedSession);
BD_REFERENCE(netSessionSearch);
BD_REFERENCE(netInvite);
BD_REFERENCE(netSession);

namespace netMediation
{
	BD_REFERENCE(Session);
	class Session: public Messaging::C_MessageHandler, public bdReferencable
	{
	public:
		enum
		{
			STATE_NOT_SET=-1,
			STATE_CREATING,
			STATE_JOINING,
			STATE_CONNECTED,
			STATE_DISCONNECTED,
			STATE_LEAVING,
			STATE_LEFT,
			STATE_COUNT,
		};

		enum
		{
			STATUS_NOT_SET = -1,
			STATUS_OK = netTransport::CLOSE_REASON_NOT_SET,
			STATUS_ERROR = netTransport::CLOSE_REASON_COUNT + 1000,
		};

		Session(const char* debugName, int maxClients, int maxEntities, netTransport::SocketRef socket, unsigned char socketPort);
		virtual ~Session();

		// state
		virtual bool connecting		() { return m_state == STATE_CREATING || m_state == STATE_JOINING; }
		virtual bool connected		() { return m_state == STATE_CONNECTED;	}
		virtual bool disconnected	() { return m_state == STATE_DISCONNECTED; }
		virtual bool left			() { return m_state == STATE_LEFT;	}
		virtual bool error			() { return m_status != netTransport::CLOSE_REASON_NOT_SET;	}
		virtual bool ok				() { return m_status == netTransport::CLOSE_REASON_NOT_SET; }
		
		int	 NATtype() const { return m_NATtype; }
		int  status	() const { return m_status; }
		int  stateId() const { return m_state; }
		const char* debugName() const { return m_debugName; }
		void getBandwidthLoad(float& in, float& out);
		void getBandwidthSpare(float& in, float& out);
		void getBandwidthCap(float& in, float& out);
		void setBandwidthCap(float in, float out);
		
		// session interface
		virtual void render	(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h);
		virtual bool join	(netSessionJoinDataRef   sessionJoinData,	netStreamRef gameParams);
		virtual bool create	(netSessionCreateDataRef sessionCreateData, netStreamRef gameParams);
		virtual void leave	(bool fast);
		virtual void poll	();
		virtual void cleanup();
		virtual bool MessageHandler(const Messaging::C_MessageBase* pMessage);

		
		// transport interface
		virtual bool receive(const netTransport::ConnGuid& from, const netTransport::Message* msg);
		
		// component factory interface
		bool factoryCreate(int type, bool registerFactory=true);
		bool factoryDestroy(int type);
		bool factorySerialise(int type, class netStream* stream);
		bool factoryDeserialise(int type, class netStream* stream);

		// accessors
		netReplicator::ReplicatorRef replicator() { return m_replicator; }
		netSessionRef session();
		netEnumeratedSessionRef  sessionInfo();
		netTransport::ConnectedMeshRef mesh() { return m_connectedMesh; }
		PeerStateRef peer(const netTransport::ConnGuid& guid);
		bdHashMap<int, PeerStateRef>& peers() { return m_peers; }
		
		bool setPresence(bool onOff);
		bool isPresence() const;

		bool hosting() const;
		bool peering() const;

		static const char* stateToString(int state);

	protected:
		//------------------------------
		// Entity factory interface
		//------------------------------
		virtual bool entityCreate(int type, const netTransport::ConnGuid& guid)=0;
		virtual bool entityDestroy(int type, const netTransport::ConnGuid& guid)=0;
		virtual bool entitySerialise(int type, const netTransport::ConnGuid& guid, class netStream* stream)=0;
		virtual bool entityDeserialise(int type, const netTransport::ConnGuid& guid, class netStream* stream)=0;
		virtual netTransport::ConnGuid entityClientGuid(int type)=0;

		//------------------------------
		// replicator client and entity management
		//------------------------------
		bool createClient(const netTransport::ConnGuid& guid);
		bool destroyClient(const netTransport::ConnGuid& guid);
		virtual bool createClientEntities(const netTransport::ConnGuid& guid)=0;
		virtual bool destroyClientEntities(const netTransport::ConnGuid& guid)=0;
		
		int  setStatus(int status);
		int  setState(int state);
		netEnumeratedSessionRef createGame(netSessionCreateDataRef sessionCreateData);	

		virtual bool processEvent(netTransport::EventRef ev);		
		virtual void renderGraph(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h);
		virtual void renderTraffic(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h);
		virtual const char* renderGraphNodeName(const netTransport::ConnGuid& guid, char* buff, int len);
		
	public:
		// status
		const char* m_debugName;
		const int m_maxClients;
		const int m_maxEntities;

		int m_status;
		int m_step;
		int	m_state;
		bool m_online;

		// transport
		const unsigned char m_socketPort;
		netTransport::SocketRef m_socket;
		netTransport::ConnectedMeshRef m_connectedMesh;
		netStreamRef m_gameParams;

		// session
		netSessionRef			m_session;
		netEnumeratedSessionRef m_sessionInfo;	// advertised info.
		netSessionQoSRef		m_sessionQoS;	// qos we want to use.
		netSessionStateRef		m_sessionState;	// session state we want to achieve (local machine, session->sessionState() is the state on servers).
		bool					m_presence;		// are we dictating for presence.
		int						m_NATtype;		// NAT type of the session (basically the most restrictive peer).
		
		//--------------------------------------------------
		// replication and synching
		//--------------------------------------------------
	public:
		// replicates distributed object states between all members of the session.
		netReplicator::ReplicatorRef m_replicator;
		
		// state of session of each peer
		bdHashMap<int, PeerStateRef> m_peers;	// session state of peers
		PeerStateRef	m_syncHost;				// session state to sync with
		
		// record network traffic
		netTransport::TrafficLogRef	m_outTrafficLogger;
		netTransport::TrafficLogRef	m_inTrafficLogger;
		float m_sendCapKBpS;
		float m_receiveCapKBpS;
		float m_sendLoadKBpS;
		float m_receiveLoadKBpS;

		bool debugOutputMigration();

		// components update
		void updateNATtype();
		void updateForwarders();
		void updateTraffic();
		void pollConnections();
		void updateState();
		void updateSelf();

		// synchronise our session state with our host.
		void synchronise();
		void syncConnections();
		void syncMigration();
		void syncClients();
		void syncPresence();
		bool promotePeer();

		bool renderMigration(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h);
		bool debugOutputMigration(PeerStateRef data, char* buffer, int len);
	
		virtual PeerStateRef electMigrationCandidate(PeerStateRef a, PeerStateRef b);
		virtual bool kickOutOfSyncClients();
		virtual netTransport::ConnGuid findForwarder(const netTransport::ConnGuid& target, const netTransport::Peer* myPeers, int myPeerCount);

		// the complete graph to render
		void updateRenderGraph(float x, float y, float w, float h);
		bdHashMap<int, PeerStateRenderRef> m_renderGraph;
	};
}
#endif //NETMEDIATOR_SESSION_H