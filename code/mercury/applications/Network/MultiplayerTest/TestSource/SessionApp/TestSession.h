#ifndef TESTSESSION_H
#define TESTSESSION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <netMediator/netMediator_Session.h>
#include <netTransport/netTransport_Socket.h>

namespace Test
{
	struct Player: public netSerialisable
	{
		Player();
		virtual bool serialise(class netStream* stream);
		virtual bool deserialise(class netStream* stream);

		char m_name[100];
		U64  m_timestamp;
	};
	
	// test host state
	BD_REFERENCE(HostState);
	class HostState: public netSerialisable, public bdReferencable
	{
	public:
		HostState();
		virtual ~HostState();
		virtual bool serialise(class netStream* stream);
		virtual bool deserialise(class netStream* stream);

		enum { MAX_PLAYERS = 16 };
		Player m_players[MAX_PLAYERS];
	};

	// test peer state
	BD_REFERENCE(PeerState);
	class PeerState: public netMediation::PeerState
	{
	public:
		PeerState();
		virtual ~PeerState();
		virtual bool serialise(class netStream* stream);
		virtual bool deserialise(class netStream* stream);
		Player m_player;
	};

	BD_REFERENCE(Session);
	class Session: public netMediation::Session, public netTransport::SocketListener
	{
	public:
		static Session* instance();
		Session();
		~Session();

		void poll();
		bool processEvent(netTransport::EventRef ev);		
		bool recv(netTransport::Message* msg, int socketPort, netTransport::ConnGuid from);
		void render(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h);
		
		virtual bool entityCreate(int type, const netTransport::ConnGuid& guid);
		virtual bool entityDestroy(int type, const netTransport::ConnGuid& guid);
		virtual bool entitySerialise(int type, const netTransport::ConnGuid& guid, class netStream* stream);
		virtual bool entityDeserialise(int type, const netTransport::ConnGuid& guid, class netStream* stream);
		virtual netTransport::ConnGuid entityClientGuid(int type);
							
		//------------------------------
		// create entites attached to client
		//------------------------------
		bool createClientEntities(const netTransport::ConnGuid& guid);
		bool destroyClientEntities(const netTransport::ConnGuid& guid);

		virtual const char* renderGraphNodeName(const netTransport::ConnGuid& guid, char* buff, int len);

	private:
		void syncPeerState();
		void syncHostState();
		netStopwatch m_updateTick;
		HostStateRef m_hostState;
	};
}

#endif //TESTSESSION_H