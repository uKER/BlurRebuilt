// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdMatchMaking/bdSessionID.h>

bdSessionID::bdSessionID()
: m_sessionID()
{
	bdMemset(m_sessionID.ab, 0, sizeof(m_sessionID.ab));
}

bdSessionID::bdSessionID( const bdSessionID &other )
{
	bdMemcpy(m_sessionID.ab, other.m_sessionID.ab, BD_SECURITY_ID_LENGTH);
}

bdBool bdSessionID::deserialize(bdByteBufferRef buffer)
{
	bdBool ok;
	// read securityID
	bdUInt32 blobLength = BD_SECURITY_ID_LENGTH;
	ok = buffer->readBlob(m_sessionID.ab, blobLength);

	return ok;
}

void bdSessionID::serialize(bdByteBuffer &buffer) const
{
	
	bdUInt32 blobLength = BD_SECURITY_ID_LENGTH;
	buffer.writeBlob(m_sessionID.ab, blobLength);
}

bdUInt bdSessionID::sizeOf()
{
	return sizeof(bdSessionID);
}


