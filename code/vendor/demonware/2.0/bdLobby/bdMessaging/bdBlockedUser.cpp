// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdMessaging/bdBlockedUser.h>


bdBlockedUser::bdBlockedUser()
{
}

bdBlockedUser::~bdBlockedUser()
{
}

bdBool bdBlockedUser::deserialize(bdByteBufferRef buffer)
{
	bdBool ok;
	ok = buffer->readUInt64(m_userID);
	ok = ok && buffer->readString(m_userName, BD_MAX_USERNAME_LENGTH);
	return ok;
}

bdUInt bdBlockedUser::sizeOf()
{
	return sizeof(bdBlockedUser);
}

