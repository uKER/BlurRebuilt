// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdFriends/bdFriendProposal.h>

bdFriendProposal::bdFriendProposal()
: m_userID(0),
  m_attachmentSize(0)
{
	bdMemset(m_userName, 0, BD_MAX_USERNAME_LENGTH);
}

bdFriendProposal::~bdFriendProposal()
{
}


bdBool bdFriendProposal::deserialize(bdByteBufferRef buffer)
{
	bdBool ok;
	ok = buffer->readUInt64(m_userID);
	ok = ok && buffer->readString(m_userName, BD_MAX_USERNAME_LENGTH);
	bdUInt maxSize = BD_MAX_INVITE_ATTACHMENT_SIZE;
	ok = ok && buffer->readBlob(m_attachment, maxSize);
	m_attachmentSize = maxSize;
	return ok;
}

bdUInt bdFriendProposal::sizeOf()
{
	return sizeof(bdFriendProposal);
}
