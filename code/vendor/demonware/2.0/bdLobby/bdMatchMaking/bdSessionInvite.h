// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// Purpose: A bdSessionInvite contains information about an outstanding invite
// to join an online session.

#ifndef BD_SESSION_INVITE_H
#define BD_SESSION_INVITE_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdMatchMaking/bdSessionID.h>


/// A bdSessionInvite contains information about an outstanding invite
/// to join an online session.
class bdSessionInvite : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

 		/// Default Constructor
		bdSessionInvite();

		/// Virtual Destructor
		virtual ~bdSessionInvite();

		/// Deserialize the entire invite from a buffer.
		/// \param buffer[in|out]	The buffer from which the member variables
		///							are deserialized.
		/// \return
		///		- True, if deserialization completed successfully.
		///		- False, otherwise.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// The user ID of the sender
		bdOnlineUserID m_senderID;

		/// The user name of the sender
		bdNChar8 m_senderName [BD_MAX_USERNAME_LENGTH];

		/// The session ID of the session
		bdSessionID m_sessionID;

		/// Pointer to an optional attachment.
		bdUByte8 m_attachment[BD_MAX_INVITE_ATTACHMENT_SIZE];

		/// Size of the attachment in bytes.
		bdUInt m_attachmentSize;

};

#endif // BD_SESSION_INVITE_H
