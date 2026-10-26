// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdTeams/bdTeamProfile.h>


bdTeamProfile::bdTeamProfile()
{
}

bdTeamProfile::~bdTeamProfile()
{
}

void bdTeamProfile::serialize(bdByteBuffer&)const
{
}

bdBool bdTeamProfile::deserialize(bdByteBufferRef buffer)
{
	bdBool ok;
	ok = buffer->readUInt64(m_teamID);
	return ok;
}

bdUInt bdTeamProfile::sizeOf()
{
	return sizeof(bdTeamProfile);
}

