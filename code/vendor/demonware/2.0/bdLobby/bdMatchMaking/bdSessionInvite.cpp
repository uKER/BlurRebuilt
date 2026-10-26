// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdMatchMaking/bdSessionInvite.h>

#define BD_LOG_LEVEL "session invite"

bdSessionInvite::bdSessionInvite()
 :m_senderID(0),
  m_attachmentSize(0)
{
}

bdSessionInvite::~bdSessionInvite()
{
}

bdBool bdSessionInvite::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;

	ok = ok && buffer->readUInt64(m_senderID);
	ok = ok && buffer->readString(m_senderName, BD_MAX_USERNAME_LENGTH);

	// TODO, should we check that the id is of the expected length ?
	bdUInt length =  BD_SECURITY_ID_LENGTH;
	ok = ok && buffer->readBlob(&m_sessionID.m_sessionID.ab[0], length);

	m_attachmentSize = BD_MAX_INVITE_ATTACHMENT_SIZE;
	ok = ok && buffer->readBlob(m_attachment, m_attachmentSize);

	if (!ok)
	{
		BD_ERR(BD_LOG_LEVEL, "Deserialization failed");
	}
	return ok;
}

bdUInt bdSessionInvite::sizeOf()
{
	return sizeof(bdSessionInvite);
}

