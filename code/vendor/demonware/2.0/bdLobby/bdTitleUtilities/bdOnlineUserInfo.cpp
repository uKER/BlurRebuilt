// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdTitleUtilities/bdOnlineUserInfo.h>

bdOnlineUserInfo::bdOnlineUserInfo()
: m_userID(0),
  m_isOnline(false)
{
}

bdOnlineUserInfo::~bdOnlineUserInfo()
{
}


bdBool bdOnlineUserInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = buffer->readUInt64(m_userID);
	ok = ok && buffer->readBool(m_isOnline);	
	return ok;
}

bdUInt bdOnlineUserInfo::sizeOf()
{
	return sizeof(bdOnlineUserInfo);
}
