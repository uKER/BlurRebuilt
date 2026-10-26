// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdTeams/bdTeamInfo.h>

#define BD_LOG_LEVEL "teamInfo"

bdTeamInfo::bdTeamInfo()
: m_teamID(0),
  m_ownerUserID(0)
{
}

bdTeamInfo::~bdTeamInfo()
{
}


bdBool bdTeamInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = buffer->readUInt64(m_teamID);
	ok = ok && buffer->readString(m_teamName, BD_MAX_TEAMNAME_LENGTH);
	ok = ok && buffer->readUInt64(m_ownerUserID);
	ok = ok && buffer->readUByte8(m_memberStatus);
	if (!ok)
	{
		BD_ERR(BD_LOG_LEVEL, "Deserialization failed");
	}
	return ok;
}

bdUInt bdTeamInfo::sizeOf()
{
	return sizeof(bdTeamInfo);
}
