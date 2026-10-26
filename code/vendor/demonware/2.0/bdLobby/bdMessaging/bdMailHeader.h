// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_MAIL_HEADER_H
#define BD_MAIL_HEADER_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

/// Contains common entries of a mail message
class bdMailHeader : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdMailHeader();

		virtual ~bdMailHeader();

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// The user ID of the mail sender.
		bdOnlineUserID m_senderID;

		/// The user name of the mail sender.
		bdNChar8 m_senderName[BD_MAX_USERNAME_LENGTH];

		/// The mail ID
		bdUInt64 m_mailID;

		/// Time stamp when the message was created
		bdUInt32 m_timeStamp;

		/// The size of the message in bytes
		bdUInt m_size;
};


#endif // BD_MAIL_HEADER_H
