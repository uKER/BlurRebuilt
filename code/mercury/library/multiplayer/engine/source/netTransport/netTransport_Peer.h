#ifndef NETTRANSPORT_PEER_H
#define NETTRANSPORT_PEER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netTransport_Addr.h"
#include "netTransport_ConnGuid.h"
#include "netTransport_Enums.h"

#include "netStream/netStream.h"
#include "netStream/netSerialise.h"

namespace netTransport
{
	class Peer: public netSerialisable
	{
	public:
		// peer list utilities
		static void sortPeers(Peer* peers, int peerCount);
		static const Peer* findPeer(const Addr& addr, const Peer* peers, int peerCount);
		static const Peer* findPeer(const ConnGuid& guid, const Peer* peers, int peerCount);
		
	public:
		Peer()
		{
			m_state = CONN_STATE_NOT_SET;
			m_qos = CONN_QOS_UNAVAILABLE;
		}

		int qos() const { return m_qos; }

		bool connecting() const  { return m_state == CONN_STATE_CONNECTING; }
		bool pending() const  { return m_state == CONN_STATE_PENDING; }
		bool accepting() const  { return m_state == CONN_STATE_ACCEPTING; }
		bool established() const  { return m_state == CONN_STATE_ESTABLISHED; }
		bool unresponsive() const { return m_state == CONN_STATE_UNRESPONSIVE; }
		bool closing() const  { return m_state == CONN_STATE_CLOSING; }
		bool closed() const  { return m_state == CONN_STATE_CLOSED; }
		bool destroyed() const { return m_state == CONN_STATE_NOT_SET; }
		
		bool serialise(netStream* stream) 
		{ 
			stream->writeType(m_state);
			stream->writeType(m_qos);
			m_guid.serialise(stream);
			m_addr.serialise(stream);
			return stream->ok();
		}

		bool deserialise(netStream* stream) 
		{ 
			stream->readType(m_state);
			stream->readType(m_qos);
			m_guid.deserialise(stream);
			m_addr.deserialise(stream);
			return stream->ok();
		}

		char			m_state;
		char			m_qos;
		ConnGuid		m_guid;
		Addr			m_addr;
	};
}

#endif //NETTRANSPORT_PEER_H