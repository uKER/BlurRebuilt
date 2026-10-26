// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdMessaging/bdNotification.h>
#include <bdLobby/bdMessaging/bdLobbyMessageType.h>


bdNotification::bdNotification()
{
}

bdNotification::~bdNotification()
{
}

bdBool bdNotification::deserialize(bdByteBufferRef buffer)
{
	bdUInt32 notificationType;

	bdBool ok = buffer->readUInt32(notificationType);
	m_type = static_cast<bdNotificationType>(notificationType);
	ok = ok &&  buffer->readUInt32(m_timeStamp);
	ok = ok &&  buffer->readUInt64(m_ID);
	ok = buffer->readUInt64(m_sourceUser);
	ok = ok && buffer->readString(m_sourceUserName, BD_MAX_USERNAME_LENGTH);
	ok = buffer->readUInt64(m_affectedUser);
	ok = ok && buffer->readString(m_affectedUserName, BD_MAX_USERNAME_LENGTH);
	ok = buffer->readUInt64(m_teamID);
	ok = ok && buffer->readString(m_teamName, BD_MAX_USERNAME_LENGTH);
	
	return ok;
}


bdUInt bdNotification::sizeOf()
{
	return sizeof(bdNotification);
}
