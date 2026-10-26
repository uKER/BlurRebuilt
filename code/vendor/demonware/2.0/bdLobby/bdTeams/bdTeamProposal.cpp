// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdTeams/bdTeamProposal.h>

bdTeamProposal::bdTeamProposal()
{
}

bdTeamProposal::~bdTeamProposal()
{
}


bdBool bdTeamProposal::deserialize(bdByteBufferRef buffer)
{
	bdBool ok;
	ok = buffer->readUInt64(m_userID);
	ok = ok && buffer->readUInt64(m_teamID);
	ok = ok && buffer->readString(m_userName, BD_MAX_USERNAME_LENGTH);
	ok = ok && buffer->readString(m_teamName, BD_MAX_TEAMNAME_LENGTH);
	bdUInt maxSize = BD_MAX_INVITE_ATTACHMENT_SIZE;
	ok = ok && buffer->readBlob(m_attachment, maxSize);
	m_attachmentSize = maxSize;
	return ok;
}
		
bdUInt bdTeamProposal::sizeOf()
{
	return sizeof(bdTeamProposal);
}

