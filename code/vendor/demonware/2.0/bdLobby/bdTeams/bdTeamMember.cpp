// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdTeams/bdTeamMember.h>

bdTeamMember::bdTeamMember()
: m_userID(0),
  m_isOnline(false),
  m_status(BD_ORDINARY)
{
}

bdTeamMember::~bdTeamMember()
{
}




bdBool bdTeamMember::deserialize(bdByteBufferRef buffer)
{
	bdBool ok;
	ok = buffer->readUInt64(m_teamID);
	ok = buffer->readUInt64(m_userID);
	ok = ok && buffer->readString(m_userName, BD_MAX_USERNAME_LENGTH);
	ok = ok && buffer->readBool(m_isOnline);
	ok = ok && buffer->readUByte8(m_status);
	
	return ok;
}

bdUInt bdTeamMember::sizeOf()
{
	return sizeof(bdTeamMember);
}

