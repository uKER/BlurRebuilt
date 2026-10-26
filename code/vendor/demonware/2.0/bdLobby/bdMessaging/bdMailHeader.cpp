// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include<bdLobby/bdMessaging/bdMailHeader.h>

bdMailHeader::bdMailHeader()
{
}

bdMailHeader::~bdMailHeader()
{
}

bdBool bdMailHeader::deserialize(bdByteBufferRef buffer)
{
	bdBool ok;
	ok = buffer->readUInt64(m_senderID);
	ok = ok && buffer->readString(m_senderName, BD_MAX_USERNAME_LENGTH);
	ok = ok && buffer->readUInt64(m_mailID);
	ok = ok && buffer->readUInt32(m_timeStamp);
	ok = ok && buffer->readUInt32(m_size);
	return ok;
}

bdUInt bdMailHeader::sizeOf()
{
	return sizeof(bdMailHeader);
}

