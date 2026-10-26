// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdFriends/bdFriendInfo.h>

bdFriendInfo::bdFriendInfo()
: m_userID(0),
  m_isOnline(false),
  m_groupID(0)
{
	bdMemset(m_userName, 0, BD_MAX_USERNAME_LENGTH);
}

bdFriendInfo::~bdFriendInfo()
{
}


bdBool bdFriendInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok;
	ok = buffer->readUInt64(m_userID);
	ok = ok && buffer->readString(m_userName, BD_MAX_USERNAME_LENGTH);
	ok = ok && buffer->readBool(m_isOnline);
	ok = ok && buffer->readUByte8(m_groupID);
	
	return ok;
}

bdUInt bdFriendInfo::sizeOf()
{
	return sizeof(bdFriendInfo);
}
