// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdStats/bdSessionNonce.h>


bdSessionNonce::bdSessionNonce()
: m_nonce(0)
{
}

bdSessionNonce::~bdSessionNonce()
{
}


void bdSessionNonce::serialize(bdByteBuffer &buffer)const
{
	buffer.writeUInt64(m_nonce);
}

bdBool bdSessionNonce::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = buffer->readUInt64(m_nonce);
	return ok;
}

bdUInt bdSessionNonce::sizeOf()
{
	return sizeof(bdSessionNonce);
}

