// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_FRIEND_PROPOSAL_H
#define BD_FRIEND_PROPOSAL_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

		
class bdFriendProposal: public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		bdFriendProposal();

		~bdFriendProposal();

		/// Deserialize the friend info from a buffer.
		/// \param buffer[in|out]	The buffer from which the member variables 
		///							are deserialized.
		/// \return 
		///		- True, if deserialization completed successfully.
		///		- False, otherwise.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// The user ID of the player who sent the friendship proposal
		bdOnlineUserID m_userID;

		/// The name of the player who sent the friendship proposal
		bdNChar8 m_userName[BD_MAX_USERNAME_LENGTH];

		bdUByte8 m_attachment[BD_MAX_INVITE_ATTACHMENT_SIZE];

		bdUInt32 m_attachmentSize;
		
};


#endif // BD_FRIEND_PROPOSAL_H
