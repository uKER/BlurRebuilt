// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdTwitter/bdTwitterIsRegistered.h>

bdTwitterIsRegistered::bdTwitterIsRegistered()
:m_isRegistered(false)
{
}

void bdTwitterIsRegistered::serialize(bdByteBuffer &buffer) const
{
	buffer.writeBool(m_isRegistered);
}

bdBool bdTwitterIsRegistered::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readBool(m_isRegistered);
	return ok;
}

bdUInt bdTwitterIsRegistered::sizeOf()
{
	return sizeof(bdTwitterIsRegistered);
}



