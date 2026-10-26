// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdPacket/bdInitChunk.h>
#include <bdCore/bdUtilities/bdBytePacker.h>

bdInitChunk::bdInitChunk()
: bdChunk(BD_CT_INIT),
  m_initTag(0),
  m_flags(BD_IC_NO_FLAGS),
  m_windowCredit(0)
{
}

bdInitChunk::bdInitChunk(bdUInt32 initTag,
						 const bdWord windowCredit)
: bdChunk(BD_CT_INIT),
  m_initTag(initTag),
  m_flags(BD_IC_NO_FLAGS),
  m_windowCredit(windowCredit)
{
}

bdInitChunk::~bdInitChunk()
{
}

bdUInt32 bdInitChunk::getInitTag() const
{
	return m_initTag;
}

bdInitChunk::bdInitChunkFlags bdInitChunk::getFlags() const
{
	return m_flags;
}

bdWord bdInitChunk::getWindowCredit() const
{
	return m_windowCredit;
}

bdUInt bdInitChunk::serialize(bdUByte8 *data, 
							  const bdUInt size) const
{
	// WARNING: This code must always return the serialized size of the chunk.
	// Even when data == BD_NULL and size == 0, be careful when editing.

	bdUInt offset = 0;
	bdBool ok = true;

	offset = bdChunk::serialize(data, size);

	const bdUByte8 flags = static_cast<bdUByte8>(m_flags);
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, flags);

	bdUInt16 length = 0;
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, length);
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, m_initTag);

	return offset;
}

bdBool bdInitChunk::deserialize(const bdUByte8 *const data, 
								const bdUInt size,
								bdUInt &offset)
{
	bdBool ok = true;
	bdUInt bytesRead = offset;

	if(size - offset > BD_CHUNK_MIN_SIZE)
	{
		ok = bdChunk::deserialize(data, size, bytesRead);	

		bdUByte8 flags = 0;
		ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, flags);
		m_flags = static_cast<bdInitChunkFlags>(flags);

		bdUInt16 length = 0;
		ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, length);
		ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, m_initTag);
	}

	if(ok)
	{
		offset = bytesRead;
	}

	return ok;
}
