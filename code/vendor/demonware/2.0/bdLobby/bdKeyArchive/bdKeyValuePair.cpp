// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdKeyArchive/bdKeyValuePair.h>


bdKeyValuePair::bdKeyValuePair()
: m_updateType(BD_ADD),
  m_index(0),
  m_value(0)
{
}


bdBool bdKeyValuePair::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;

	ok = ok && buffer->readUInt16(m_index);
	ok = ok && buffer->readInt64(m_value);
	return ok;
}

bdUInt bdKeyValuePair::sizeOf()
{
	return sizeof(bdKeyValuePair);
}



