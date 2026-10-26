// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdFacebook/bdFacebookIsRegistered.h>

bdFacebookIsRegistered::bdFacebookIsRegistered()
:m_isRegistered(false)
{
}

void bdFacebookIsRegistered::serialize(bdByteBuffer &buffer) const
{
	buffer.writeBool(m_isRegistered);
}

bdBool bdFacebookIsRegistered::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readBool(m_isRegistered);
	return ok;
}

bdUInt bdFacebookIsRegistered::sizeOf()
{
	return sizeof(bdFacebookIsRegistered);
}



