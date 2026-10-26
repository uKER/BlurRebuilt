// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdUtilities/bdBytePacker.h>
#include <bdConnection/bdPacket/bdPacket.h>
#include <bdConnection/bdPacket/bdDataChunk.h>
#include <bdConnection/bdPacket/bdInitChunk.h>
#include <bdConnection/bdPacket/bdInitAckChunk.h>
#include <bdConnection/bdPacket/bdSAckChunk.h>
#include <bdConnection/bdPacket/bdHeartbeatChunk.h>
#include <bdConnection/bdPacket/bdHeartbeatAckChunk.h>
#include <bdConnection/bdPacket/bdCookieEchoChunk.h>
#include <bdConnection/bdPacket/bdCookieAckChunk.h>
#include <bdConnection/bdPacket/bdShutdownChunk.h>
#include <bdConnection/bdPacket/bdShutdownAckChunk.h>
#include <bdConnection/bdPacket/bdShutdownCompleteChunk.h>
#include <bdConnection/bdMessage/bdMessage.h>

#define BD_MAX_DESERIALIZE_ITERATIONS	(100u)


#define BD_LOG_LEVEL	"bdConnection/packet"

bdPacket::bdPacket()
: m_tag(0),
  m_bufferSize(0),
  m_currentSize(0)
{
}

bdPacket::bdPacket(const bdUInt32 tag,
				   const bdUInt bufferSize)
: m_tag(tag),
  m_bufferSize(bufferSize - sizeof(m_tag) - BD_SOCKET_SIZE_FIELD_SIZE),
  m_currentSize(sizeof(m_tag) + BD_SOCKET_SIZE_FIELD_SIZE)
{
}

/// Destructor.
bdPacket::~bdPacket()
{
}

bdUInt bdPacket::serialize(bdUByte8 *outBuffer, 
						   const bdUInt outSize)
{
	// skip the first BD_SOCKET_SIZE_FIELD_SIZE bytes
	bdUInt outOffset = BD_SOCKET_SIZE_FIELD_SIZE;	
	
	// write verification tag
	bdBool ok = bdBytePacker::appendBasicType(outBuffer, outSize, outOffset, outOffset, m_tag);

	// write the chunks encrypted data
	bdLinkedList<bdChunkRef>::Position chunkPos = m_chunks.getHeadPosition();
	bdUInt bytesRemaining = outSize - outOffset;
	bdUInt encDataSize = outOffset - BD_SOCKET_SIZE_FIELD_SIZE;

	while(chunkPos && ok)
	{			
		const bdChunkRef chunk = m_chunks.forward(chunkPos);

		// is there enough room for this chunk in the output buffer
		// including any unencrypted data
		const bdUInt serializedNumBytes = chunk->getSerializedSize();
		if(bytesRemaining >= serializedNumBytes)
		{
			bytesRemaining -= serializedNumBytes;
			//TODO: this should return a bool to indicate success or failure
			const bdUInt bytesWritten = chunk->serialize(outBuffer + outOffset, outSize - outOffset);
			encDataSize += bytesWritten;
			outOffset += bytesWritten;
		}
		else
		{
			ok = false;
			BD_WARN(BD_LOG_LEVEL, "Buffer not big enough for all chunks.");
		}
	}

	// write the chunks unencrypted data
	// this should always succeed as the space has been reserved above
	chunkPos = m_chunks.getHeadPosition();
	while(chunkPos && ok)
	{
		const bdChunkRef chunk = m_chunks.forward(chunkPos);

		if(chunk->getType() == BD_CT_DATA)
		{
			bdDataChunkRef dataChunk = reference_cast<bdDataChunk>(chunk);
			// note bdDataChunk::serializeUnencrypted does nothing if there is no unencrypted data
			//TODO: this should return a bool to indicate success or failure
			const bdUInt bytesWritten = dataChunk->serializeUnencrypted(outBuffer + outOffset, outSize - outOffset);
			outOffset += bytesWritten;
		}
	}

	bdUInt totalBytesWritten = 0;
	if(ok)
	{
		// write the size of the encrypted section
		const bdUInt16 encDataSize16 = static_cast<bdUInt16>(encDataSize);
		if(encDataSize16 == encDataSize)
		{
			bdUInt newOffset = 0;
			ok = bdBytePacker::appendBasicType(outBuffer, outSize, 0, newOffset, encDataSize16);
			if(ok)
			{
				totalBytesWritten = outOffset;
			}
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL , "Encrypted data section size too big to fit in bdUInt16.");
		}
	}

	BD_ASSERT(m_currentSize == totalBytesWritten, "Packet size miscalculation.");

	return totalBytesWritten;
}

bdBool bdPacket::deserialize(const bdUByte8 *inData, 
							 const bdUInt inSize)
{
	bdBool ok = (inData != BD_NULL) && (inSize > BD_BASIC_PACKET_OVERHEAD);
	

	// read the size of the encrypted part of the packet
	bdUInt16 encSize = 0;
	bdUInt encOffset = 0;
	
	ok = ok && bdBytePacker::removeBasicType(inData, inSize, encOffset, encOffset, encSize);
	
	// get a pointer to the encrypted data 
	const bdUByte8 *const encData = inData + encOffset;
	
	// get a pointer to the unencrypted data
	const bdUByte8 *const unencData = encData + encSize;
	const bdUInt unencSize = inSize - (encSize + encOffset);
	bdUInt unencOffset = 0;

	// reset the offset
	encOffset = 0;

	// read out the verification tag.
	ok  = ok && bdBytePacker::removeBasicType(encData, encSize, encOffset, encOffset, m_tag);

	// deserialize the chunks.
	const bdUInt maxIt = BD_MAX_DESERIALIZE_ITERATIONS;
	bdUInt iterations = 0;

	//TODO: what if there is only unenc data in the packet? should we be testing "ok" here?
	while((encOffset < encSize) && ok && (iterations < maxIt))
	{
		// read the chunk type
		// TODO: should return a bool bdChunk::getType
		const bdChunkTypes type = bdChunk::getType(encData + encOffset, encSize - encOffset);	

		// TODO: remove all this newing it's not required, just handle the chunks as they are read
		bdChunkRef chunk;
		switch(type)
		{
			case BD_CT_PADDING:
				encOffset += 1;
				break;
			case BD_CT_DATA:
				chunk = new bdDataChunk;
				break;
			case BD_CT_SACK: 
				chunk = new bdSAckChunk;
				break;
			case BD_CT_HEARTBEAT: 
				chunk = new bdHeartbeatChunk;
				break;
			case BD_CT_HEARTBEAT_ACK: 
				chunk = new bdHeartbeatAckChunk;
				break;
			case BD_CT_INIT:
				chunk = new bdInitChunk;
				break;
			case BD_CT_INIT_ACK:
				chunk = new bdInitAckChunk;
				break;
			case BD_CT_ERROR: 
				BD_WARN(BD_LOG_LEVEL, "Not implemented! (BD_CT_ERROR)");
				/// xxx: fixme
				break;
			case BD_CT_COOKIE_ECHO: 
				chunk = new bdCookieEchoChunk;
				break;
			case BD_CT_COOKIE_ACK:
				chunk = new bdCookieAckChunk;
				break;
			case BD_CT_ABORT: 
				BD_WARN(BD_LOG_LEVEL, "Not implemented! (BD_CT_ABORT)");
				/// xxx: fixme
				break;
			case BD_CT_SHUTDOWN: 
				chunk = new bdShutdownChunk;
				break;
			case BD_CT_SHUTDOWN_ACK: 
				chunk = new bdShutdownAckChunk;
				break;
			case BD_CT_SHUTDOWN_COMPLETE:
				chunk = new bdShutdownCompleteChunk;
				break;
			default:
				ok = false;
				BD_WARN(BD_LOG_LEVEL, "unknown chunk type.");
				break;
		}

		if(chunk && ok)
		{
			if(type == BD_CT_DATA)
			{
				const bdDataChunkRef dataChunk = reference_cast<bdDataChunk>(chunk);
				ok = ok && dataChunk->deserialize(encData, encSize, encOffset, unencData, unencSize, unencOffset);
			}
			else
			{
				ok = ok && chunk->deserialize(encData, encSize, encOffset);
			}

			if(ok)
			{
				m_chunks.addTail(chunk);
			}	
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Chunk deserialization failed.");
			}
		}
		iterations++;
	}

	return ok;
}

bdUInt32 bdPacket::getVerificationTag() const
{
	return m_tag;
}


bdBool bdPacket::addChunk(bdChunkRef chunk)
{
	BD_ASSERT(chunk.notNull(), "Chunk should never be null.");

	// fail safe
	bdBool added = false;

	// get the serialized size of the chunk 
	bdUInt chunkSize = chunk->getSerializedSize();

	const bdUInt newPacketSize = m_currentSize + chunkSize;
	if(newPacketSize <= m_bufferSize)
	{
		m_chunks.addTail(chunk);
		m_currentSize = newPacketSize;
		added = true;
	}

	return added;
}

bdBool bdPacket::getNextChunk(bdChunkRef &chunk)
{
	bdBool result = false;
	
	if(!m_chunks.isEmpty())
	{
		chunk = m_chunks.getHead();
		m_chunks.removeHead();

		result = true;
	}

	return result;
}

bdBool bdPacket::isEmpty() const
{
	return m_chunks.isEmpty();
}

// protected:


