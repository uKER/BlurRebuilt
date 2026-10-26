#ifndef NETTRANSPORT_CONNPACKET_H
#define NETTRANSPORT_CONNPACKET_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netTransport_Enums.h"
#include "netTransport_Message.h"
#include "netTransport_TrafficLogger.h"
#include "netStream/netArray.h"

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdSocket/bdAddrHandle.h>

namespace netTransport
{
	//================================================================================================================
	//
	// packet format :
	// ---------------
	// ushort(encrypted bytes) + uchar(port) + ushort(origin) + ushort(destination) + ubytes(encrypted payload) + ubytes(unencrypted payload)
	//                         ^                                                                                ^
	//                         |                                                                                |
	//                          ------------------------------ encrypted bytes ---------------------------------
	//
	//================================================================================================================
	BD_REFERENCE(ConnPacket);
	class ConnPacket
	{
	public:
		ConnPacket();

		// read from socket data
		int readFrom(char* data, int dataLen);

		// write to socket data
		int writeTo(char* data, int dataLen);

		// reset packet for sending
		bool reset(int packetSize=PACKET_MTU);

		// analyse content of the packet.
		bool analyse(MessageParserRef messageParser, TrafficLoggerRef logger, bool forwarded, bool detailedAnalysis);

		void setForwarded(bool onOff) { m_forwarded = onOff; }

		bool forwarded() const { return m_forwarded; }
		bool empty() const;
		bool ok() const;
		bool error() const { return !ok(); }
		
		// read one message. return how many bytes read
		Message* readMessage(MessageParserRef messageParser);

		// read one message. return how many bytes written
		int writeMessage(Message* msg);
	
		int transmittedBytes();

	public:
		unsigned char	m_port;
		bdAddrHandleRef m_address;
		bool			m_forwarded;
		ConnGuid		m_origin;
		ConnGuid		m_destination;

	private:
		bool writeEncryptedHeader();
		bool writeUnencryptedHeader();
		bool readEncryptedHeader();
		bool readUnencryptedHeader();
		bool resetEncryptedBuffer(int encryptedCapacity);
		bool resetUnEncryptedBuffer(int unencryptedCapacity);
		int encryptedHeaderSize() const;
		int unencryptedHeaderSize() const;

		int						m_encryptedCapacity;
		int						m_unencryptedCapacity;
		netArray<PACKET_MTU>	m_encrypted;
		netArray<PACKET_MTU>	m_unencrypted;
	};
}
#endif //NETTRANSPORT_CONNPACKET_H