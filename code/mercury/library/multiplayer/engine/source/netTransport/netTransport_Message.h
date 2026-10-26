#ifndef MULTIPLAYER_MESSAGE_H
#define MULTIPLAYER_MESSAGE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "./netTransport_Addr.h"
#include "./netTransport_ConnGuid.h"
#include "./netTransport_Enums.h"
#include "netEnums/netEnums.h"
#include "netStream/netSerialise.h"
#include "netStream/netStream.h"
#include "netTime/netTime.h"

namespace netTransport
{
	enum
	{
		MESSAGE_ID_NOT_SET,
		MESSAGE_ID_CONNECT_REQUEST,				// request to connect to a host, with added custom data blob
		MESSAGE_ID_CONNECT_ACCEPT,				// response to a conect request, with added custom data blob
		MESSAGE_ID_CONNECT_PING,				// ping request to calculate round-trip latency
		MESSAGE_ID_CONNECT_PONG,				// reply to a  ping request
		MESSAGE_ID_CONNECT_CLOSING,				// connection is closing
		MESSAGE_ID_VOICE_FRAGMENTS,				// some voice data
		MESSAGE_ID_PACKET,						// packet data 
		MESSAGE_ID_PACKET_FRAGMENT,				// a packet has been fragmented, and expects acknowledgement
		MESSAGE_ID_PACKET_ACK,					// a ack for a reliable fragmented packet.
		MESSAGE_ID_CUSTOM_TYPES,				// custom types can be defined after
		
	};

	class Message: public netSerialisable
	{
	public:
		Message(int messageId, bool encrypted, const char* debugName);
		virtual ~Message();

		void getTrafficLog(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType);
		int segment(int limit); // try to make the message fit within a limited number of bytes.
		bool serialise(netStream* str);
		bool deserialise(netStream* str);
		
		unsigned char messageId() const { return m_messageId; }
		bool encrypted() const { return m_encrypted; }
		const char* debugName() const { return m_debugName; }
		
	protected:
		virtual bool logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType)=0;
		virtual bool writeParams(netStream* str)=0;
		virtual bool readParams(netStream* str)=0;
		
	private:
		const char* m_debugName;
		const unsigned char m_messageId;
		const bool m_encrypted;
	};

	class MsgConnectRequest: public Message
	{ 
	public:
		MsgConnectRequest();
		virtual bool logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType);
		virtual bool writeParams(netStream* str);
		virtual bool readParams(netStream* str);
		
		netNonce m_connectKey; 
		netTransport::Addr m_addr; 
		unsigned short m_payloadSize;
		
		enum { MAX_PAYLOAD = (PACKET_MTU - 100) };
		char m_payload[MAX_PAYLOAD];
	};

	class MsgConnectAccepted: public Message
	{ 
	public:
		MsgConnectAccepted();
		virtual bool logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType);
		virtual bool writeParams(netStream* str);
		virtual bool readParams(netStream* str);
		
		netNonce m_connectKey; 
		ConnGuid m_assignedGuid; 
		ConnGuid m_hostGuid; 

		unsigned short m_payloadSize; 

		enum { MAX_PAYLOAD = (PACKET_MTU - 100) };
		char m_payload[MAX_PAYLOAD];
	};

	class MsgConnectPing: public Message
	{ 
	public:
		MsgConnectPing();
		virtual bool logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType);
		virtual bool writeParams(netStream* str);
		virtual bool readParams(netStream* str);
		
		U64 m_timestamp;
		F32 m_qos;
	};

	class MsgConnectPong: public Message
	{ 
	public:
		MsgConnectPong();
		virtual bool logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType);
		virtual bool writeParams(netStream* str);
		virtual bool readParams(netStream* str);
		
		U64 m_timestamp; 
	};

	class MsgConnectClosing: public Message
	{ 
	public:
		MsgConnectClosing();
		virtual bool logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType);
		virtual bool writeParams(netStream* str);
		virtual bool readParams(netStream* str);
		
		int m_closeReason; 
	};
	
	class MsgVoiceFragments: public Message
	{ 
	public:
		MsgVoiceFragments();
		virtual bool logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType);
		virtual bool writeParams(netStream* str);
		virtual bool readParams(netStream* str);
		
		U64				m_userId;		// who's speaking 
		unsigned char	m_fragmentSize;	// size of a fragment 
		int				m_maxFragments;	// max number of fragments.
		int				m_minFragments;	// min number of fragments to send.

		// the size of the data we've packed
		unsigned char m_fragmentsTransmitted;	// [return value] how much is in the payload.
		
		enum { MAX_PAYLOAD = (PACKET_MTU - 100) };
		char m_payload[MAX_PAYLOAD];			// the fragment data.
	};

	/////////////////////////////////////////////////////////////////////////////
	// Class       : MsgPacket
	//---------------------------------------------------------------------------
	// Created By  : Genesis Network Team
	// Created On  : 21 April 2009
	// Description : packet chunk, sent as a single blob
	/////////////////////////////////////////////////////////////////////////////
	BD_REFERENCE(MsgPacket);
	class MsgPacket: public Message
	{
	public:
		MsgPacket();
		virtual bool logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType);
		virtual bool writeParams(netStream* str);
		virtual bool readParams(netStream* str);

		unsigned char	m_packetId;
		unsigned short	m_payloadSize;
		
		enum { MAX_PAYLOAD = (PACKET_MTU - 200) };
		char m_payload[MAX_PAYLOAD];
	};

	/////////////////////////////////////////////////////////////////////////////
	// Class       : MsgPacketFragment
	//---------------------------------------------------------------------------
	// Created By  : Genesis Network Team
	// Created On  : 21 April 2009
	// Description : reliable packet fragment
	/////////////////////////////////////////////////////////////////////////////
	BD_REFERENCE(MsgPacketFragment);
	class MsgPacketFragment: public Message, public bdReferencable
	{
	public:
		MsgPacketFragment(unsigned int pointer=0, unsigned int size=0, bool reliable=false);
		virtual bool logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType);
		virtual bool writeParams(netStream* str);
		virtual bool readParams(netStream* str);

		bool			m_reliable;
		unsigned char	m_packetId;
		unsigned int	m_packetSqn;
		unsigned int	m_packetSize;
		unsigned int	m_packetPointer;
		
		unsigned short	m_minPayload;
		unsigned short	m_maxPayload;
		unsigned short	m_payloadSize;
		U64				m_timestamp;
		
		enum { MAX_PAYLOAD = 700 };
		char m_payload[MAX_PAYLOAD];
	};

	/////////////////////////////////////////////////////////////////////////////
	// Class       : MsgPacketAck
	//---------------------------------------------------------------------------
	// Created By  : Genesis Network Team
	// Created On  : 21 April 2009
	// Description : ack for a reliable packet fragment
	/////////////////////////////////////////////////////////////////////////////
	class MsgPacketAck: public Message
	{
	public:
		MsgPacketAck();
		virtual bool logParams(unsigned int& headerSize, unsigned int& payloadSize, int& payloadType);
		virtual bool writeParams(netStream* str);
		virtual bool readParams(netStream* str);

		unsigned char	m_packetId;
		unsigned int	m_packetSqn;
		unsigned int	m_packetPointer;
		unsigned short	m_payloadSize;
	};

	/////////////////////////////////////////////////////////////////////////////
	// Class       : MessageParser
	//---------------------------------------------------------------------------
	// Created By  : Genesis Network Team
	// Created On  : 07 March 2009
	// Description : takes binary streams and generate messages.
	//               this is intended for reading straight out of the socket, 
	//               and forwarding the message as a 'temporary' placeholder.
	/////////////////////////////////////////////////////////////////////////////
	BD_REFERENCE(MessageParser);
	class MessageParser: public bdReferencable
	{
	public:
		MessageParser();
		~MessageParser();

		// grab the message instance given some data
		Message* instance(netStream* str);

	protected:
		Message* customInstance(unsigned char messageId, netStream* str);
		Message* genericInstance(unsigned char messageId, netStream* str);

		MsgConnectRequest				m_connRequestInstance;
		MsgConnectAccepted				m_connAcceptInstance;
		MsgConnectPing					m_connPingInstance;
		MsgConnectPong					m_connPongInstance;
		MsgConnectClosing				m_connClosingInstance;
		MsgVoiceFragments				m_voiceFragmentsInstance;
		MsgPacket						m_packet;
		MsgPacketFragment				m_packetFragment;
		MsgPacketAck					m_packetAck;
	};
}

#endif //MULTIPLAYER_MESSAGE_H