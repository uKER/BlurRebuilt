
#include "stdafx.h"
#include "./netReplicator_Client.h"
#include "netDebug/netDebug.h"
#include "netstate/gameState.h"
#include "netState/statemanager.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netReplicator
{
	Client::Client(const netTransport::ConnGuid& guid, netTransport::SocketRef socket, int socketPort)
	: m_socket(socket)
	, m_socketPort(socketPort)
	, m_guid(guid)
	{
		LOGINFO(netDebug::g_ReplicatorLog, "<netReplicator::Client(%d)> created.", m_guid.m_guid);
	}

	Client::~Client()
	{
		LOGINFO(netDebug::g_ReplicatorLog, "<netReplicator::Client(%d)> destroyed.", m_guid.m_guid);
	}

	ClientStateRef Client::clientState(int stateId)
	{
		ClientStateRef client;
		m_clientStates.get(stateId, client);
		return client;
	}

	bool Client::enableClientStateUpdates(int stateId, bool onOff)
	{
		ClientStateRef client;
		m_clientStates.get(stateId, client);
		if(client.isNull()) return false;
		return client->enable(onOff);
	}		

	bool Client::addClientState(int stateId)
	{
		if(m_clientStates.containsKey(stateId))
		{
			LOGERROR(netDebug::g_ReplicatorLog, "<netReplicator::Client(%d)> already has state client(%d).", m_guid.m_guid, stateId);
			return false;
		}
		m_clientStates.put(stateId, new ClientState(stateId, m_guid, m_socket, m_socketPort));
		return true;
	}

	bool Client::removeClientState(int stateId)
	{
		m_clientStates.remove(stateId);
		return true;
	}

	bool Client::sendingReliableData()
	{
		bdHashMap<int, ClientStateRef>::Iterator it = m_clientStates.getIterator();
		while(it)
		{
			ClientStateRef client = m_clientStates.getValue(it);
			m_clientStates.next(it);
			if(client->sendingReliableData())
			{
				m_clientStates.releaseIterator(it);
				return true;
			}
		}
		m_clientStates.releaseIterator(it);
		return false;
	}

	bool Client::poll()
	{
		bdHashMap<int, ClientStateRef>::Iterator it = m_clientStates.getIterator();
		while(it)
		{
			ClientStateRef client = m_clientStates.getValue(it);
			m_clientStates.next(it);
			client->poll();
		}
		m_clientStates.releaseIterator(it);
		return true;
	}

	bool Client::receive(const netTransport::Message* msg)
	{
		switch(msg->messageId())
		{
		default:
			{
				static int g_counter = 0;
				if(g_counter++ < 10)
				{
					//LOGWARN(netDebug::g_ReplicatorLog, "<netReplicator::Client(%d)> unknown message type(%d, %s)", m_guid.m_guid, msg->messageId(), msg->debugName());
				}
				return false;
			}
			// handle packet ack
		case netTransport::MESSAGE_ID_PACKET:
			{
				netTransport::MsgPacket* packet = (netTransport::MsgPacket*)msg;
				ClientStateRef c = clientState(packet->m_packetId);
				if(c.isNull())	return true;

				return c->receive(msg);
			}
		case netTransport::MESSAGE_ID_PACKET_ACK:
			{
				netTransport::MsgPacketAck* ack = (netTransport::MsgPacketAck*)msg;
				ClientStateRef c = clientState(ack->m_packetId);
				if(c.isNull())	return true;

				return c->receive(msg);
			}
			// handle new packet data
		case netTransport::MESSAGE_ID_PACKET_FRAGMENT:
			{
				netTransport::MsgPacketFragment* fragment = (netTransport::MsgPacketFragment*)msg;
				ClientStateRef c = clientState(fragment->m_packetId);
				if(c.isNull())	return true;

				return c->receive(msg);
			}
		}
		return false;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

