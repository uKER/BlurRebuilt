
#include "stdafx.h"
#include "netTransport_Message.h"
#include "netStream\netStreamSizeCalculator.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{
	Message::Message(int messageId, bool encrypted, const char* debugName)
	: m_messageId(messageId)
	, m_encrypted(encrypted)
	, m_debugName(debugName)
	{}

	Message::~Message()
	{}

	int Message::segment(int limit)
	{
		netStreamSizeCalculator_NEW calc(limit, true);
		
		if(!serialise(&calc))
			return 0;

		return calc.getWriteByteCount();
	}
	
	bool Message::serialise(netStream* str)
	{
		str->writeType(m_messageId);
		
		if(!writeParams(str))
			return false;

		return str->ok();
	}

	bool Message::deserialise(netStream* str)
	{
		unsigned char messageId;
		str->readType(messageId);
		assert(messageId == m_messageId);
		return readParams(str);
	}

	void Message::getTrafficLog(unsigned int& headerSize, unsigned int& payloadSize, int&  payloadType)
	{
		headerSize  = 0;
		payloadSize = 0;
		payloadType = TRAFFIC_TYPE_NOT_SET;

		if(!logParams(headerSize, payloadSize, payloadType))
		{
			headerSize = serialiseSize();
		}
		else
		{
			headerSize += sizeof(unsigned char);
			return;
		}
	}		

	MsgConnectRequest::MsgConnectRequest()
	: Message(MESSAGE_ID_CONNECT_REQUEST, true, "msg.ConnRequest")
	{
		m_payloadSize = 0;
	}

	bool MsgConnectRequest::logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType)
	{
		headerSize = 0;
		payloadSize = BD_COMMON_ADDR_SERIALIZED_SIZE + sizeof(m_connectKey) + sizeof(m_payloadSize) + m_payloadSize;
		payloadType = TRAFFIC_TYPE_SYSTEM;
		return true;
	}
	
	bool MsgConnectRequest::writeParams(netStream* str)
	{
		assert(m_payloadSize <= MAX_PAYLOAD);

		str->writeBytes(m_addr.m_buffer, BD_COMMON_ADDR_SERIALIZED_SIZE);
		str->writeType(m_connectKey);
		str->writeType(m_payloadSize);
		str->writeBytes(m_payload, m_payloadSize);
		return str->ok();		
	}

	bool MsgConnectRequest::readParams(netStream* str)
	{
		str->readBytes(m_addr.m_buffer, BD_COMMON_ADDR_SERIALIZED_SIZE);
		str->readType(m_connectKey);
		str->readType(m_payloadSize);
		if(str->error() || m_payloadSize > MAX_PAYLOAD)
		{
			m_payloadSize = 0;
			return false;
		}
		str->readBytes(m_payload, m_payloadSize);
		return str->ok();
	}


	MsgConnectAccepted::MsgConnectAccepted()
	: Message(MESSAGE_ID_CONNECT_ACCEPT, true, "msg.ConnAccepted")
	{
		m_payloadSize = 0;
	}

	bool MsgConnectAccepted::logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType)
	{
		headerSize = 0;
		payloadSize = sizeof(m_connectKey) + sizeof(m_assignedGuid.m_guid) + sizeof(m_hostGuid.m_guid) + sizeof(m_payloadSize) + m_payloadSize;
		payloadType = TRAFFIC_TYPE_SYSTEM;
		return true;
	}
	
	bool MsgConnectAccepted::writeParams(netStream* str)
	{
		assert(m_payloadSize <= MAX_PAYLOAD);
		
		str->writeType(m_connectKey);
		str->writeType(m_assignedGuid.m_guid);
		str->writeType(m_hostGuid.m_guid);
		str->writeType(m_payloadSize);
		str->writeBytes(m_payload, m_payloadSize);
		return str->ok();
	}

	bool MsgConnectAccepted::readParams(netStream* str)
	{
		str->readType(m_connectKey);
		str->readType(m_assignedGuid.m_guid);
		str->readType(m_hostGuid.m_guid);
		str->readType(m_payloadSize);
		if(str->error() || m_payloadSize > MAX_PAYLOAD)
		{
			m_payloadSize = 0;
			return false;
		}
		str->readBytes(m_payload, m_payloadSize);
		return str->ok();
	}
	
	MsgConnectPing::MsgConnectPing()
	: Message(MESSAGE_ID_CONNECT_PING, true, "msg.ConnPing")
	{
		m_timestamp = 0;
		m_qos = 0.0f;
	}

	bool MsgConnectPing::logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType)
	{
		headerSize = 0;
		payloadSize = sizeof(m_timestamp) + sizeof(m_qos);
		payloadType = TRAFFIC_TYPE_SYSTEM;
		return true;
	}

	bool MsgConnectPing::writeParams(netStream* str)
	{
		str->writeType(m_timestamp);
		str->writeType(m_qos);
		return str->ok();
	}

	bool MsgConnectPing::readParams(netStream* str)
	{
		str->readType(m_timestamp);
		str->readType(m_qos);
		return str->ok();
	}
	
	MsgConnectPong::MsgConnectPong()
	: Message(MESSAGE_ID_CONNECT_PONG, true, "msg.ConnPong")
	{
		m_timestamp = 0;
	}

	bool MsgConnectPong::logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType)
	{
		headerSize = 0;
		payloadSize = sizeof(m_timestamp);
		payloadType = TRAFFIC_TYPE_SYSTEM;
		return true;
	}

	bool MsgConnectPong::writeParams(netStream* str)
	{
		str->writeType(m_timestamp);
		return str->ok();
	}

	bool MsgConnectPong::readParams(netStream* str)
	{
		str->readType(m_timestamp);
		return str->ok();
	}
	
	MsgConnectClosing::MsgConnectClosing()
	: Message(MESSAGE_ID_CONNECT_CLOSING, true, "msg.ConnClosing")
	{
		m_closeReason = CLOSE_REASON_NOT_SET;
	}

	bool MsgConnectClosing::logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType)
	{
		headerSize = 0;
		payloadSize = sizeof(m_closeReason);
		payloadType = TRAFFIC_TYPE_SYSTEM;
		return true;
	}

	bool MsgConnectClosing::writeParams(netStream* str)
	{
		str->writeType(m_closeReason);
		return str->ok();
	}

	bool MsgConnectClosing::readParams(netStream* str)
	{
		str->readType(m_closeReason);
		return str->ok();
	}

	MsgVoiceFragments::MsgVoiceFragments()
	: Message(MESSAGE_ID_VOICE_FRAGMENTS, false, "msg.VoiceFragments")
	{
		m_userId = 0;
		m_fragmentSize = 0;
		m_maxFragments = 0;
		m_minFragments = 0;
		m_fragmentsTransmitted = 0;
	}

	bool MsgVoiceFragments::logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType)
	{
		headerSize = 0;
		payloadSize = sizeof(m_userId) + sizeof(m_fragmentSize) + sizeof(m_fragmentsTransmitted) + m_fragmentSize * m_fragmentsTransmitted;
		payloadType = TRAFFIC_TYPE_VOICE;
		return true;
	}
	
	bool MsgVoiceFragments::writeParams(netStream* str)
	{
		int overhead = sizeof(m_userId) + sizeof(m_fragmentSize) + sizeof(m_fragmentsTransmitted);
		int limit = str->getWriteByteLimit() - str->getWriteByteCount();
		limit -= overhead; // message headers we'll be packing in
		if(limit <= 0) return false;

		if(limit > MAX_PAYLOAD)
			limit = MAX_PAYLOAD;

		int fragments = limit / m_fragmentSize;
		
		if(fragments > m_maxFragments)
			fragments = m_maxFragments;
		
		if(fragments < m_minFragments) 
			return false;
		
		m_fragmentsTransmitted = fragments;

		int payloadSize = m_fragmentSize * fragments;

		assert(payloadSize >= 0 && payloadSize <= MAX_PAYLOAD);
		assert(m_userId != 0);
		assert(m_maxFragments <= 255);
		assert(m_minFragments >= 1);

		// three byte header
		str->writeType(m_userId);
		str->writeType(m_fragmentSize);
		str->writeType(m_fragmentsTransmitted);
		str->writeBytes(m_payload, payloadSize);
		return str->ok();
	}

	bool MsgVoiceFragments::readParams(netStream* str)
	{
		str->readType(m_userId);
		str->readType(m_fragmentSize);
		str->readType(m_fragmentsTransmitted);

		int payloadSize = m_fragmentSize * m_fragmentsTransmitted;
		
		if(str->error() || payloadSize > MAX_PAYLOAD)
		{
			m_fragmentsTransmitted = 0;
			return false;
		}
		str->readBytes(m_payload, payloadSize);
		return str->ok();
	}	

	/////////////////////////////////////////////////////////////////////////////
	// Class       : MsgPacket
	//---------------------------------------------------------------------------
	// Created By  : Genesis Network Team
	// Created On  : 21 April 2009
	// Description : packet chunk, sent as a single blob
	/////////////////////////////////////////////////////////////////////////////
	MsgPacket::MsgPacket()
	: Message(MESSAGE_ID_PACKET, true, "msg.Packet")
	, m_packetId(0)
	, m_payloadSize(0)
	{}

	bool MsgPacket::logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType)
	{
		headerSize = sizeof(m_packetId) + sizeof(m_payloadSize);
		payloadSize = m_payloadSize;
		payloadType = TRAFFIC_TYPE_CUSTOM;
		return true;
	}

	bool MsgPacket::writeParams(netStream* str)
	{
		assert(m_payloadSize <= MAX_PAYLOAD);
		str->writeType(m_packetId);
		str->writeType(m_payloadSize);
		str->writeBytes(m_payload, m_payloadSize);
		return str->ok();		
	}

	bool MsgPacket::readParams(netStream* str)
	{
		str->readType(m_packetId);
		str->readType(m_payloadSize);
		if(str->error() || m_payloadSize > MAX_PAYLOAD)
		{
			m_payloadSize = 0;
			return false;
		}
		str->readBytes(m_payload, m_payloadSize);
		return str->ok();
	}

	MsgPacketFragment::MsgPacketFragment(unsigned int pointer, unsigned int size, bool reliable)
	: Message(MESSAGE_ID_PACKET_FRAGMENT, true, "msg.PacketFragment")
	{
		m_timestamp = 0;
		m_reliable = reliable;
		m_packetId = 0;
		m_packetSqn = 0;
		m_packetSize = 0;
		m_packetPointer = pointer;
		m_minPayload = 0;
		m_maxPayload = 0;
		m_payloadSize = size;
	}

	bool MsgPacketFragment::logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType)
	{
		headerSize = sizeof(m_reliable) + sizeof(m_packetId) + sizeof(m_packetSqn) + sizeof(m_packetSize) + sizeof(m_packetPointer) + sizeof(m_payloadSize);
		payloadSize = m_payloadSize;
		payloadType = m_reliable? TRAFFIC_TYPE_RELIABLE : TRAFFIC_TYPE_CUSTOM;
		return true;
	}

	bool MsgPacketFragment::writeParams(netStream* str)
	{
		int overhead = sizeof(m_reliable) + sizeof(m_packetId) + sizeof(m_packetSqn) + sizeof(m_packetSize) + sizeof(m_packetPointer) + sizeof(m_payloadSize);
		int limit = (str->getWriteByteLimit() - str->getWriteByteCount());
		limit -= overhead; // message headers we'll be packing in

		if(limit < m_minPayload) 
			return false;

		if(limit > m_maxPayload)
			limit = m_maxPayload;

		m_payloadSize = limit;

		str->writeType(m_reliable);
		str->writeType(m_packetId);
		str->writeType(m_packetSqn);
		str->writeType(m_packetSize);
		str->writeType(m_packetPointer);
		str->writeType(m_payloadSize);
		str->writeBytes(m_payload, m_payloadSize);
		return str->ok();
	}

	bool MsgPacketFragment::readParams(netStream* str)
	{
		str->readType(m_reliable);
		str->readType(m_packetId);
		str->readType(m_packetSqn);
		str->readType(m_packetSize);
		str->readType(m_packetPointer);
		str->readType(m_payloadSize);
	
		if(str->error() || m_payloadSize > MAX_PAYLOAD)
		{
			m_minPayload = m_maxPayload = m_payloadSize = 0;
			return false;
		}
		str->readBytes(m_payload, m_payloadSize);

		m_minPayload = m_maxPayload = m_payloadSize;
				
		return str->ok();
	}

	MsgPacketAck::MsgPacketAck()
	: Message(MESSAGE_ID_PACKET_ACK, true, "msg.PacketAck")
	{
		m_packetId = 0;
		m_packetSqn = 0;
		m_packetPointer = 0;
		m_payloadSize = 0;
	}
	
	bool MsgPacketAck::logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType)
	{
		headerSize = sizeof(m_packetId) + sizeof(m_packetSqn) + sizeof(m_packetPointer) + sizeof(m_payloadSize);
		payloadSize = 0;
		payloadType = TRAFFIC_TYPE_RELIABLE;
		return true;
	}

	bool MsgPacketAck::writeParams(netStream* str)
	{
		str->writeType(m_packetId);
		str->writeType(m_packetSqn);
		str->writeType(m_packetPointer);
		str->writeType(m_payloadSize);
		return str->ok();	
	}

	bool MsgPacketAck::readParams(netStream* str)
	{
		str->readType(m_packetId);
		str->readType(m_packetSqn);
		str->readType(m_packetPointer);
		str->readType(m_payloadSize);
		return str->ok();	
	}

	MessageParser::MessageParser()
	{}

	MessageParser::~MessageParser()
	{}

	// grab the message instance given some data
	Message* MessageParser::instance(netStream* str)
	{
		if(!str->ok()) return NULL;
		int readPtr = str->getReadByteCount();
		
		unsigned char messageId;
		str->readType(messageId);
		if(str->error()) return NULL;

		str->setReadByteCount(readPtr);
		
		Message* msg = genericInstance(messageId, str);
		if(msg != NULL)
			return msg;

		msg = customInstance(messageId, str);
		if(msg != NULL)
			return msg;

		LOGERROR(Debugging::g_logger, "<MessageParser::customInstance> unhandled message type (%d)", messageId);
		return NULL;
	}

	Message* MessageParser::genericInstance(unsigned char messageId, netStream* str)
	{
		Message* msg = NULL;

		switch(messageId)
		{
		default:
			{
				return NULL;
			}
		case MESSAGE_ID_CONNECT_REQUEST:
			{
				msg = &m_connRequestInstance;
				break;
			}
		case MESSAGE_ID_CONNECT_ACCEPT:
			{
				msg = &m_connAcceptInstance;
				break;
			}
		case MESSAGE_ID_CONNECT_PING:
			{
				msg = &m_connPingInstance;
				break;
			}
		case MESSAGE_ID_CONNECT_PONG:
			{
				msg = &m_connPongInstance;
				break;
			}
		case MESSAGE_ID_CONNECT_CLOSING:
			{
				msg = &m_connClosingInstance;
				break;
			}
		case MESSAGE_ID_VOICE_FRAGMENTS:
			{
				msg = &m_voiceFragmentsInstance;
				break;
			}
		case MESSAGE_ID_PACKET:
			{
				msg = &m_packet;
				break;
			}
		case MESSAGE_ID_PACKET_FRAGMENT:
			{
				msg = &m_packetFragment;
				break;
			}
		case MESSAGE_ID_PACKET_ACK:
			{
				msg = &m_packetAck;
				break;
			}
		}

		if(msg == NULL)
			return NULL;

		if(!msg->deserialise(str))
			return NULL;

		return msg;
	}
	
	Message* MessageParser::customInstance(unsigned char messageId, netStream* str)
	{
		return NULL;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

