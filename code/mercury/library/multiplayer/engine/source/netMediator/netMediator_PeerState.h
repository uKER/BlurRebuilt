#ifndef NETMEDIATOR_SESSIONSTATE_H
#define NETMEDIATOR_SESSIONSTATE_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netStream/netStream.h"
#include "netTransport/netTransport_Addr.h"
#include "netTransport/netTransport_ConnGuid.h"
#include "netTransport/netTransport_Peer.h"
#include <network/secureKey.h>
#include <system/debug/I_DebugRender.h>

namespace netMediation
{
	enum { MAX_PEERS = 16 };

	/////////////////////////////////////////////////////////////////////////////
	// Class       : PeerState
	//---------------------------------------------------------------------------
	// Created By  : Genesis Network Team
	// Created On  : 16 March 2009
	// Description :	connections in the party (host -> peers).
	//					each members of the party / game will sync their connections
	//					to the host connection graph.
	/////////////////////////////////////////////////////////////////////////////
	BD_REFERENCE(PeerState);
	class PeerState: public netSerialisable, public bdReferencable
	{
	public:
		struct ConnectionList: public netSerialisable 
		{
			ConnectionList();
	
			int m_peerCount;
			netTransport::Peer m_peers[MAX_PEERS];

			bool serialise(class netStream* stream);
			bool deserialise(class netStream* stream);

			const netTransport::Peer* peer(const netTransport::ConnGuid& guid) const;
			const netTransport::Peer* addr(const netTransport::Addr& addr) const;
			int   qosRating(const netTransport::ConnGuid& guid) const;
		};

		struct Session: public netSerialisable 
		{
			Session();

			bool serialise(class netStream* stream);
			bool deserialise(class netStream* stream);

			// current session status
			char						m_name[32];							// our name (mostly debug).
			netTransport::ConnGuid		m_selfGuid;							// our allocated guid.
			netTransport::ConnGuid		m_hostGuid;							// current host attached to us.
			char						m_NATtype;							// our NAT type.

			// electing a new host
			netTransport::ConnGuid		m_electionCandidate;				// who will be our next host.
			int							m_electionWeight;					// how much we weight in the election process.
			bool						m_electionResults;					// all our peers agree with our candidate.
			bool						m_electionUnanimous;				// candidate voted and agreed by everyone.
			
			// session migration info.
			NetPlay::MigrationInfo		m_migrationInfo;					// migration info.
			
			// arbitration session status.
			NetPlay::ArbitrationNonce	m_arbitrationInfo;					// arbitration data.
			char						m_arbitratedPlayerCount;			// number of players arbitrated so far (host info).
		};

		PeerState();
		bool serialise(class netStream* stream);
		bool deserialise(class netStream* stream);
		Session			m_session;		// session data
		ConnectionList	m_connections;	// peers we're connnected to
	};

	/////////////////////////////////////////////////////////////////////////////
	// Class       : PeerState
	//---------------------------------------------------------------------------
	// Created By  : Genesis Network Team
	// Created On  : 16 March 2009
	// Description :	debug render info for session graph rendering.
	/////////////////////////////////////////////////////////////////////////////
	BD_REFERENCE(PeerStateRender);
	class PeerStateRender: public bdReferencable
	{
	public:
		PeerStateRender(const char* name, netTransport::ConnGuid guid, PeerStateRef state, bool established);
		void init(const Maths::HVec2& pos, const Maths::HVec2& dir, float rad);
		
		void render(DbUtil::I_DebugRenderPtr debugRenderPtr, netTransport::ConnGuid selfGuid);
		void renderLink(DbUtil::I_DebugRenderPtr debugRenderPtr, PeerStateRenderRef other, netTransport::ConnGuid selfGuid);
		
		PeerStateRef m_state;
		netTransport::ConnGuid m_guid;

		Maths::HVec2 m_pos;
		Maths::HVec2 m_centre;
		Maths::HVec2 m_dir;
		Maths::HVec2 m_tan;
		Maths::HVec2 m_input;
		Maths::HVec2 m_output;
		float		 m_radius;
		bool		 m_established;
		char		 m_name[48];
	};

}

#endif //NETMEDIATOR_SESSIONSTATE_H