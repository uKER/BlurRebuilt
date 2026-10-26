// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdContentUnlock/bdUnlockContentInfo.h>

bdUnlockContentInfo::bdUnlockContentInfo()
:	m_contentKey(0)
{
}

bdUnlockContentInfo::~bdUnlockContentInfo()
{
}

bdBool bdUnlockContentInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt32(m_contentKey);
	ok = ok && buffer->readUInt64(m_hardwareID);
	return ok;
}

bdUInt bdUnlockContentInfo::sizeOf()
{
	return sizeof(bdUnlockContentInfo);
}


