// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdFriends/bdGroupInfo.h>

bdGroupInfo::bdGroupInfo()
: m_groupID(0)
{
	bdMemset(m_groupName, 0, BD_MAX_GROUPNAME_LENGTH);
}

bdGroupInfo::~bdGroupInfo()
{
}

void bdGroupInfo::serialize(bdByteBuffer &buffer)const
{
	buffer.writeUByte8(m_groupID);
	buffer.writeString(m_groupName, bdStrnlen(m_groupName,BD_MAX_GROUPNAME_LENGTH));
}

bdBool bdGroupInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok;
	ok = buffer->readUByte8(m_groupID);
	ok = ok && buffer->readString(m_groupName, BD_MAX_GROUPNAME_LENGTH);
	return ok;
}

bdUInt bdGroupInfo::sizeOf()
{
	return sizeof(bdGroupInfo);
}
