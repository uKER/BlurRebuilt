
#include "stdafx.h"
#include "netTransport_ConnPacket.h"
#include <bdSocket/bdAddrHandle.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

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
	ConnPacket::ConnPacket()
	{
		m_encryptedCapacity = 0;
		m_unencryptedCapacity = 0;		
		m_port = 0;
		m_address = NULL;
		m_origin = netTransport::ConnGuid();
		m_destination = netTransport::ConnGuid();
		m_forwarded = false;
		reset(PACKET_MTU);
	}

	// read from socket data
	int ConnPacket::readFrom(char* data, int dataLen)
	{
		unsigned short encryptedSize;

		// read header
		netBuffer reader(data, dataLen, dataLen, 0);
		reader.readType(encryptedSize);
		
		// reset buffer capacities.
		int unencryptedSize = (dataLen - encryptedSize - sizeof(encryptedSize));
		if(encryptedSize > m_encrypted.getWriteByteLimit() || encryptedSize < encryptedHeaderSize()) return 0;
		if(unencryptedSize > m_unencrypted.getWriteByteLimit() || unencryptedSize < unencryptedHeaderSize()) return 0;
		m_encrypted.clear();
		m_unencrypted.clear();
				
		// read data into buffers.
		if(	!reader.read(&m_encrypted, encryptedSize) || 
			!reader.read(&m_unencrypted, unencryptedSize))
			return false;
		
		// process headers
		if(	!readEncryptedHeader() || 
			!readUnencryptedHeader())
			return false;
		
		// sanity check
		assert(reader.eof());

		return reader.getReadByteCount();
	}

	// write to socket data
	int ConnPacket::writeTo(char* data, int dataLen)
	{
		// nothing to send
		if(empty())
			return 0;
		
		if(!writeEncryptedHeader() || !writeUnencryptedHeader())
		{
			LOGERROR(Debugging::g_logger, "<ConnPacket::write()> failed to write packet headers.");
			return 0;
		}

		// buffer wrapper
		netBuffer writer(data, dataLen, 0, 0);
		
		// count encrypted bytes, including header (minus the counter).
		unsigned short encryptedSize = m_encrypted.getWriteByteCount();
		
		// write packet header
		writer.writeType(encryptedSize);
		
		// write encrypted & unencrypted data.
		writer.writeBytes(m_encrypted.buffer(), m_encrypted.getWriteByteCount());
		writer.writeBytes(m_unencrypted.buffer(), m_unencrypted.getWriteByteCount());
		
		// sanity check
		if(!writer.ok())
			return 0;

		// return how many bytes we wrote.
		int written = writer.getWriteByteCount();
		return written;		
	}

	// reset packet for sending
	bool ConnPacket::reset(int packetSize)
	{
		resetEncryptedBuffer(packetSize);
		resetUnEncryptedBuffer(PACKET_MTU / 2);
		return true;
	}

	bool ConnPacket::ok() const
	{
		return (m_encrypted.ok() && m_unencrypted.ok());
	}

	bool ConnPacket::empty() const
	{
		return (m_encrypted.getWriteByteCount() <= encryptedHeaderSize() && (m_unencrypted.getWriteByteCount() <= unencryptedHeaderSize()));
	}

	int ConnPacket::transmittedBytes()
	{
		// total bytes through socket
		int socketBytes = (m_encrypted.getWriteByteCount() + m_unencrypted.getWriteByteCount() + PACKET_UDP_OVERHEAD);
	
		return socketBytes;
	}

	// analyse content of the packet.
	bool ConnPacket::analyse(MessageParserRef parser, TrafficLoggerRef logger, bool forwarded, bool detailedAnalysis)
	{
		if(logger.isNull())
			return false;

		if(error() || empty())
			return true;

		// total bytes through socket
		int socketBytes = transmittedBytes();
	
		// special forwarded message. 
		// these are sort-of undewsirable stuff.
		if(forwarded)
		{
			logger->log(socketBytes, TRAFFIC_TYPE_FORWARDED);
			logger->log(socketBytes, TRAFFIC_TYPE_SOCKET);
			return true;
		}

		// basic analysis
		logger->log(socketBytes, TRAFFIC_TYPE_SOCKET);
		logger->log((encryptedHeaderSize() + unencryptedHeaderSize() +  PACKET_UDP_OVERHEAD), TRAFFIC_TYPE_HEADER);
		
		// detailed analysis
		if(detailedAnalysis)
		{
			// cache pointers
			int encryptedReadPtr = m_encrypted.getReadByteCount();
			int unencryptedReadPtr = m_unencrypted.getReadByteCount();

			// restart reads from 0
			m_encrypted.setReadByteCount(encryptedHeaderSize());
			m_unencrypted.setReadByteCount(unencryptedHeaderSize());

			// read messages.
			do
			{
				// the message parsed
				Message* msg = readMessage(parser);
				if(msg == NULL) break;

				// analyse message
				unsigned int payloadSize;
				int payloadType;
				unsigned int headerSize;
				msg->getTrafficLog(headerSize, payloadSize, payloadType);
				logger->log(payloadSize, payloadType);
				logger->log(headerSize, TRAFFIC_TYPE_HEADER);
			}
			while(1);

			// reset pointers
			m_encrypted.setReadByteCount(encryptedReadPtr);
			m_unencrypted.setReadByteCount(unencryptedReadPtr);
		}
		return true;
	}
		
	// read one message. return how many bytes read
	Message* ConnPacket::readMessage(MessageParserRef messageParser)
	{
		// still some data to be read
		if(!m_encrypted.eof() && m_encrypted.ok())
		{
			// read data into message instance.
			Message* msg = messageParser->instance(&m_encrypted);
			
			// buggered!
			if(!m_encrypted.ok())
				return NULL;

			return msg;
		}
	
		// still some data to be read
		if(!m_unencrypted.eof() && m_unencrypted.ok())
		{
			// read data into message instance.
			Message* msg = messageParser->instance(&m_unencrypted);
			
			// buggered!
			if(!m_unencrypted.ok())
				return NULL;

			return msg;
		}
		// nothing read (or packet data error).
		return NULL;
	}

	// read one message. return how many bytes written
	int ConnPacket::writeMessage(Message* msg) 
	{
		// get the correct buffer
		netStream* buffer = msg->encrypted()? &m_encrypted : &m_unencrypted;
		netStream* otherbuffer = msg->encrypted()? &m_unencrypted : &m_encrypted;

		// max size of the buffer
		int limit = buffer->writeRemaining(); // how much we have left in the tank.
		limit -= otherbuffer->getWriteByteCount(); // also, consider the other buffer size. Package will be a concatenation of the two, plus the packet overhead.
		if(limit <= 0) return 0;
				
		// throttle message to the limit.
		int bytesRequired = msg->segment(limit);
		if(bytesRequired <= 0) return 0;
		
		// cache buffer pointer in case the message overflows.
		int writePtr = buffer->getWriteByteCount();

		// attempt to write message data.
		if(buffer->error() || !msg->serialise(buffer))
		{
			LOGERROR(Debugging::g_logger, "<ConnPacket::writeMessage()> error(%d) from message(%s), required (%d) bytes, left(%d) bytes.", buffer->status(), msg->debugName(), bytesRequired, limit);
			
			// stream was too small. just ignore we've written the message.
			if(buffer->status() == netStream::STATUS_FAILED_WRITE_OVERFLOW)
			{
				buffer->setWriteByteCount(writePtr);
				buffer->setStatus(netStream::STATUS_OK);
			}
			return 0;
		}

		// number of bytes written.
		int written = (buffer->getWriteByteCount() - writePtr);
		return written;
	}

	int ConnPacket::encryptedHeaderSize() const 
	{
		//	          packetPort		  +    origin			  +    destination
		return sizeof(unsigned char) + sizeof(unsigned char) + sizeof(unsigned char);
	}

	int ConnPacket::unencryptedHeaderSize() const 
	{
		return 0;
	}

	bool ConnPacket::writeEncryptedHeader()
	{
		int ptr = m_encrypted.getWriteByteCount();
		m_encrypted.setWriteByteCount(0);

		unsigned char flag = m_port;
		BIZ_ASSERT_MSG((m_port < 128), "<netTransport::ConnPacket> port overflow.");
		if(m_forwarded) flag |= (1 << 7); else flag &= ~(1 << 7);

		// write encrypted header
		m_encrypted.writeType(flag);
		m_encrypted.writeType(m_origin.m_guid);
		m_encrypted.writeType(m_destination.m_guid);
		m_encrypted.setWriteByteCount(ptr);
		return m_encrypted.ok();
	}

	bool ConnPacket::writeUnencryptedHeader()
	{
		return m_unencrypted.ok();
	}

	bool ConnPacket::readUnencryptedHeader()
	{
		return m_unencrypted.ok();
	}

	bool ConnPacket::readEncryptedHeader()
	{
		unsigned char flags;

		m_encrypted.readType(flags);
		m_encrypted.readType(m_origin.m_guid);
		m_encrypted.readType(m_destination.m_guid);

		// extract info from flags
		m_forwarded = (flags & (1 << 7)) != 0;
		m_port = flags & (~(1 << 7));
		return m_encrypted.ok();
	}


	bool ConnPacket::resetEncryptedBuffer(int encryptedCapacity)
	{
		if(encryptedCapacity > PACKET_MTU)				encryptedCapacity = PACKET_MTU;
		if(encryptedCapacity < encryptedHeaderSize())	encryptedCapacity = encryptedHeaderSize();
		m_encryptedCapacity = encryptedCapacity;

		m_encrypted.clear();
		m_encrypted.setWriteByteLimit(encryptedCapacity);
		m_encrypted.setWriteByteCount(encryptedHeaderSize()); // skip header.
		return writeEncryptedHeader();
	}

	bool ConnPacket::resetUnEncryptedBuffer(int unencryptedCapacity)
	{
		if(unencryptedCapacity > PACKET_MTU)				unencryptedCapacity = PACKET_MTU;
		if(unencryptedCapacity < unencryptedHeaderSize())	unencryptedCapacity = unencryptedHeaderSize();
		m_unencryptedCapacity = unencryptedCapacity;

		m_unencrypted.clear();
		m_unencrypted.setWriteByteLimit(unencryptedCapacity);
		m_unencrypted.setWriteByteCount(unencryptedHeaderSize()); // no header
		return writeUnencryptedHeader();
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

