
#include "stdafx.h"
#include "./netReplicator_ClientState.h"
#include "netDebug/netDebug.h"
#include "netstate/gameState.h"
#include "netState/statemanager.h"
#include "netTransport/netTransport_Message.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netReplicator
{
	ClientState::ClientState(int stateId, const netTransport::ConnGuid& guid, netTransport::SocketRef socket, int socketPort)
	: m_stateId(stateId)
	, m_socket(socket)
	, m_socketPort(socketPort)
	, m_guid(guid)
	, m_reliable(true)
	, m_enabled(true)
	{
		m_segmentPool = new netTransport::SegmentPool(32); 
		m_sendPacket = new netTransport::StreamPacketSend(m_segmentPool, m_stateId, m_socket, m_socketPort, m_guid);
		m_recvPacket = new netTransport::StreamPacketReceive(m_segmentPool, (256 * 1024), m_stateId, m_socket, m_socketPort, m_guid);

		C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);
		selfState->SetClientActive(m_guid.m_guid, true);
	
		LOGINFO(netDebug::g_ReplicatorLog, "<netReplicator::ClientState(%d, %d)> created.", m_guid.m_guid, m_stateId);
	}

	ClientState::~ClientState()
	{
		C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);
		if(selfState != NULL)
		{
			// close client communications			
			selfState->SetClientActive(m_guid.m_guid, false);

			// only for server
			if(selfState->IsServer())
			{
				// remove entities attached to that client
				selfState->RequestDeleteClientsEntities(m_guid.m_guid);
			}
		}	
		LOGINFO(netDebug::g_ReplicatorLog, "<netReplicator::ClientState(%d, %d)> destroyed.", m_guid.m_guid, m_stateId);
	}

	bool ClientState::poll()
	{
		sendPacket();
		return true;
	}

	bool ClientState::enable(bool onOff)
	{
		m_enabled = onOff;
		return true;
	}

	bool ClientState::sendingReliableData()
	{
		return m_sendPacket->transmissionReliable() && !m_sendPacket->transmissionFinished();
	}

	bool ClientState::sendPacket()
	{	
		// the packet has been transmitted. Time to build a new one.
		C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);
		
		// send what's left to send.
		m_sendPacket->transmit();
		
		// the packet is not finished sending fragments (or waiting aknowledgement).
		if(!m_sendPacket->transmissionFinished())
			return false;

		// wait for tick for next packet.
		bool ticked = selfState->NetworkTickInThisFrame();
		if(!ticked)
			return false;

		// block state update after a reliable delivery, to avoid sending another
		// full state again (we need the client to aknowledge the transmitted state first).
		if(m_sendPacket->transmissionReliable())
		{
			// restart the delay timer
			if(!m_reliableDelay.started())
				m_reliableDelay.start();
		
			// has the client aknowledged uour state, or we've waited long enough
			if(selfState->OutboundAck(m_guid.m_guid) < selfState->SequenceNumber() &&  !m_reliableDelay.elapsed(1 * 1000 * 1000))
				return false;
		
			// stop delay timer, ready for next reliable packet.
			m_reliableDelay.stop();
		}
		
		// dont generate new packets
		if(!m_enabled) 
			return false;
				
		// build packet with game state.
		m_sendPacket->clear();
		m_sendPacket->setMode(netStream::MODE_WRITE);
		if(!selfState->StateToPacket(m_guid.m_guid, m_sendPacket))
		{
			//LOGERROR(netDebug::g_ReplicatorLog, "<netReplicator::ClientState(%d, %d)> failed to create client packet.", m_guid.m_guid, m_stateId);
			m_sendPacket->clear();
			return false;
		}

		// if packet greater than 2K bytes, force send reliably.
		m_reliable = m_sendPacket->getWriteByteCount() > (2 * 1024);

		// start transmitting the state packet.
		if(!m_sendPacket->transmissionStart(m_sendPacket->sqn() + 1, m_reliable))
		{
			m_sendPacket->clear();
			return false;
		}
							
		// reset reliable flag?
		m_reliable = false;
		return true;
	}

	bool ClientState::receive(const netTransport::Message* msg)
	{
		switch(msg->messageId())
		{
		default:
			{
				LOGFATAL(netDebug::g_ReplicatorLog, "<netReplicator::ClientState(%d, %d)> unknown message type(%d, %s)", m_guid.m_guid, m_stateId, msg->messageId(), msg->debugName());
				return false;
			}
			// new state update. dont use the stream, just handle the data directly, it'll be all right.
		case netTransport::MESSAGE_ID_PACKET:
			{
				netTransport::MsgPacket* packet = (netTransport::MsgPacket*)msg;

				m_recvPacket->logMessage(packet);

				netBuffer buffer(packet->m_payload, packet->m_payloadSize, packet->m_payloadSize, 0);

				C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);

				selfState->PacketToState(m_guid.m_guid, &buffer);
			
				return true;
			}
			// packet is segmented, handle it carefully
		case netTransport::MESSAGE_ID_PACKET_FRAGMENT:	// a packet has been fragmented
		{
				m_recvPacket->receive(msg);

				//----------------------------------------------
				// received a complete packet, and not read yet.
				//----------------------------------------------
				if(m_recvPacket->complete())
				{
					// handle packet
					C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);

					selfState->PacketToState(m_guid.m_guid, m_recvPacket);

					m_recvPacket->reset(m_recvPacket->sqn());
				}
				return true;
			}
		// handle packet ack
		case netTransport::MESSAGE_ID_PACKET_ACK:
			{
				// receive acks
				m_sendPacket->receive(msg);
				return true;
			}
		}
		return false;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

