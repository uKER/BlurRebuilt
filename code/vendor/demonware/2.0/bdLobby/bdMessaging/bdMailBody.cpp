// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include<bdLobby/bdMessaging/bdMailBody.h>

bdMailBody::bdMailBody()
:m_mailBody(BD_NULL),
 m_size(0),
 m_mailID(0)
{
}

bdMailBody::bdMailBody(	void *mailBody,
						const bdUInt size)
:m_mailBody(mailBody),
 m_size(size),
 m_mailID(0)
{
}

bdMailBody::~bdMailBody()
{
}

bdBool bdMailBody::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = buffer->readUInt64(m_mailID);
	ok = ok && buffer->readBlob(static_cast<bdUByte8*>(m_mailBody), m_size);
	return ok;
}

bdUInt bdMailBody::sizeOf()
{
	return sizeof(bdMailBody);
}

