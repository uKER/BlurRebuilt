// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdPacket/bdDataChunk.h>
#include <bdConnection/bdMessage/bdMessage.h>


bdDataChunk::bdDataChunk()
: bdChunk(BD_CT_DATA),
  m_flags(BD_DC_NO_FLAGS),
  m_seqNum(0)
{
}

bdDataChunk::bdDataChunk(bdMessageRef data,
						 const bdDataFlags flags)
: bdChunk(BD_CT_DATA),
  m_message(data), 
  m_flags((bdUByte8)flags),
  m_seqNum(0)
{
	// set the BD_DC_ENC_DATA and BD_DC_UNENC_DATA flags
	if(m_message.notNull())
	{
		if(m_message->hasPayload() && 
		  (m_message->getPayload()->getNumBitsWritten() > 0))
		{
			m_flags |= BD_DC_ENC_DATA;
		}

		if(m_message->hasUnencryptedPayload() && 
		   //TODO: should be a get num bytes written function 
		  (m_message->getUnencryptedPayload()->getMaxReadSize() > 0))
		{
			m_flags |= BD_DC_UNENC_DATA;
		}
	}
}

bdDataChunk::~bdDataChunk()
{
}


void bdDataChunk::setSequenceNumber(bdSeqNumber seqNum)
{
	m_seqNum = seqNum;
}

bdSeqNumber bdDataChunk::getSequenceNumber() const
{
	return m_seqNum;
}

bdMessageRef bdDataChunk::getMessage() const
{
	return m_message;
}

bdUByte8 bdDataChunk::getFlags() const
{
	return m_flags;
}

bdUInt bdDataChunk::serializeUnencrypted(bdUByte8 *data, 
										 const bdUInt32 size) const
{
	bdUInt bytesWritten = 0;
	if(m_flags & BD_DC_UNENC_DATA)
	{
		BD_ASSERT(m_message->hasUnencryptedPayload(), "BD_DC_UNENC_DATA flag set but no unencrypted payload.");
		
		const bdByteBufferRef unEncPayload = m_message->getUnencryptedPayload();		
		if(unEncPayload.notNull())
		{
			// TODO: use payload->getNumBitsWritten() directly with BD_NUM_BITS_TO_NUM_BYTES?
			const bdUByte8 *const unEncData = unEncPayload->getData();
			//TODO: should be a get num bytes written function 
			const bdUInt unEncDataSize = unEncPayload->getMaxReadSize();

			bdUInt offset = 0;
			if(bdBytePacker::appendBuffer(data, size, offset, offset, unEncData, unEncDataSize))
			{
				bytesWritten = offset;
			}
		}
	}

	return bytesWritten;
}

bdUInt bdDataChunk::getSerializedSize() const
{
	const bdUInt encSerializedSize = serialize(BD_NULL, 0);
	const bdUInt unencSerializedSize = serializeUnencrypted(BD_NULL, 0);
	const bdUInt serializedSize = encSerializedSize + unencSerializedSize;
	return serializedSize;
}

bdUInt bdDataChunk::serialize(bdUByte8 *data, 
							  const bdUInt32 size) const
{
	// WARNING: This code must always return the serialized size of the chunk.
	// Even when data == BD_NULL and size == 0, be careful when editing.

	bdUInt offset = bdChunk::serialize(data, size);
	bdBool ok = true;

	if(m_message.notNull())
	{
		// flags (8)
		ok = bdBytePacker::appendBasicType(data, size, offset, offset, m_flags);

		// message type (8)
		const bdUByte8 type = static_cast<bdUByte8>(m_message->getType());
		ok  = ok && bdBytePacker::appendBasicType(data, size, offset, offset, type);

		// sequence number (16)
		const bdUInt16 seqNum = static_cast<bdUInt16>(m_seqNum);
		ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, seqNum);

		// encrypted payload size (optional 0, 8 or 16)
		bdUInt16 payloadSize = 0; 
		if(m_flags & BD_DC_ENC_DATA)
		{
			BD_ASSERT(m_message->hasPayload(), "BD_DC_ENC_DATA flag set but no payload.");

			const bdBitBufferRef payload = m_message->getPayload();
			if(payload.notNull())
			{
				payloadSize = (bdUInt16)payload->getNumBitsWritten();
				if(payloadSize > 0)
				{
					// TODO: use payload->getNumBitsWritten() directly with BD_NUM_BITS_TO_NUM_BYTES?
					payloadSize = (bdUInt16)payload->getDataSize();
				}
			}
			ok = ok && bdBytePacker::appendEncodedUInt16(data, size, offset, offset, payloadSize);
		}

		// unencrypted payload size (optional 0, 8 or 16)
		if(m_flags & BD_DC_UNENC_DATA)
		{
			BD_ASSERT(m_message->hasUnencryptedPayload(), "BD_DC_UNENC_DATA flag set but no unencrypted payload.");

			const bdByteBufferRef unencPayload = m_message->getUnencryptedPayload();
			bdUInt16 unencPlayloadSize = 0;
			if(unencPayload.notNull())
			{
				//TODO: this should be a get bytes written function
				unencPlayloadSize = (bdUInt16)unencPayload->getMaxReadSize();
			}

			ok = ok && bdBytePacker::appendEncodedUInt16(data, size, offset, offset, unencPlayloadSize);
		}

		// encrypted payload (optional)
		if(payloadSize > 0)
		{
			const bdUByte8 *const payloadData = m_message->getPayload()->getData();
			ok = ok && bdBytePacker::appendBuffer(data, size, offset, offset, payloadData, payloadSize);
		}
	}

	const bdUInt bytesWritten = ok?offset:0;
	return bytesWritten;
}

bdBool bdDataChunk::deserialize(const bdUByte8 *const data,
								const bdUInt size,
								bdUInt &offset)
{
	bdUInt encOffset = 0;
	const bdBool ok = deserialize(data, size, offset, BD_NULL, 0, encOffset);
	return ok;
}

bdBool bdDataChunk::deserialize(const bdUByte8 *const data, 
								const bdUInt size,
								bdUInt &offset,
								const bdUByte8 *const unencData, 
								const bdUInt /*unencSize*/,
								bdUInt &unencOffset)
{
	bdUInt bytesRead = offset;
	bdUInt unencBytesRead = unencOffset;

	bdBool ok = bdChunk::deserialize(data, size, bytesRead);

	// flags
	ok  = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, m_flags);

	// type
	bdUByte8 type8 = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, type8);

	// sequence number
	bdUInt16 seqNum = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, seqNum);
	m_seqNum = seqNum;

	// encrypted payload size (optional 8 or 16)
	bdUInt16 payloadSize = 0; 
	if(m_flags & BD_DC_ENC_DATA)
	{
		ok = ok && bdBytePacker::removeEncodedUInt16(data, size, bytesRead, bytesRead, payloadSize);		
	}

	// unencrypted payload size (optional 8 or 16)
	bdUInt16 unencPayloadSize = 0; 
	if(m_flags & BD_DC_UNENC_DATA)
	{
		ok = ok && bdBytePacker::removeEncodedUInt16(data, size, bytesRead, bytesRead, unencPayloadSize);		
	}

	if(ok)
	{
		const bdMessageType type = static_cast<bdMessageType>(type8);
		m_message = new bdMessage(type, data + bytesRead, payloadSize, true, unencData + unencBytesRead, unencPayloadSize);
		ok = m_message.notNull();
		bytesRead += payloadSize;
		unencBytesRead += unencPayloadSize;

		offset = bytesRead;
		unencOffset = unencBytesRead;
	}

	return ok;
}

