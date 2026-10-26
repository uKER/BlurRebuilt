// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdPacket/bdChunk.h>

bdChunk::bdChunk(const bdChunkTypes type)
: m_type(type)
{
}

bdChunk::~bdChunk()
{
}

bdChunkTypes bdChunk::getType() const
{
	return m_type;
}

bdChunkTypes bdChunk::getType(const bdUByte8 *const data, 
							  const bdUInt size)
{
	bdChunkTypes type = BD_CT_INVALID;
	bdBool result = true;
	bdUInt offset = 0;

	bdUByte8 tmp = static_cast<bdUByte8>(type);
	result = result && bdBytePacker::removeBasicType(data, size, offset, offset, tmp);
	if (result)
	{
		type = static_cast<bdChunkTypes>(tmp);
	}

	return type;
}


bdUInt bdChunk::serialize(bdUByte8 *data, 
						  const bdUInt32 size) const
{
	// WARNING: This code must always return the serialized size of the chunk.
	// Even when data == BD_NULL and size == 0, be careful when editing.

	bdUInt offset = 0;
	const bdUByte8 type = static_cast<bdUByte8>(m_type);
	bdBytePacker::appendBasicType(data, size, offset, offset, type);
	return offset;
}

bdBool bdChunk::deserialize(const bdUByte8 *const data, 
							const bdUInt size,
							bdUInt &offset)
{
	bdBool result = size - offset >= BD_CHUNK_MIN_SIZE;	

	bdUByte8 tmp = 0;
	result = result && bdBytePacker::removeBasicType(data, size, offset, offset, tmp);	
	if (result)
	{
		m_type = static_cast<bdChunkTypes>(tmp);
	}

	return result;
}


bdUInt bdChunk::getSerializedSize() const
{
	const bdUInt serializedSize = serialize(BD_NULL, 0);
	return serializedSize;
}

bdBool bdChunk::isControl() const
{
	return m_type != BD_CT_DATA;
}

