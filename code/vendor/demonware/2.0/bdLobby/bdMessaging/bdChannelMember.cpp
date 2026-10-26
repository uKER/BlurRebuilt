// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include<bdLobby/bdMessaging/bdChannelMember.h>


bdChannelMember::bdChannelMember()
{
}

bdChannelMember::~bdChannelMember()
{
}

bdBool bdChannelMember::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt64(m_userID);
	ok = ok && buffer->readString(m_userName, BD_MAX_USERNAME_LENGTH);
	return ok;
}

bdUInt bdChannelMember::sizeOf()
{
	return sizeof(bdChannelMember);
}



