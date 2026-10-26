// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "bdRichPresence.h"

bdRichPresence::bdRichPresence()
	:m_dataSize(0),
	m_data(BD_NULL)
{
}

bdRichPresence::bdRichPresence(void *richPresence, 
							   const bdUInt size )
	:m_dataSize(size),
	m_data(richPresence)
{
}

bdRichPresence::~bdRichPresence()
{

}

bdBool bdRichPresence::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = buffer->readBlob(static_cast<bdUByte8*>(m_data), m_dataSize);
	return ok;
}

bdUInt bdRichPresence::sizeOf()
{
	return sizeof(bdRichPresence);
}
