// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_FRIEND_INFO_H
#define BD_FRIEND_INFO_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

/// Container class to store information about a player's friend
class bdFriendInfo : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		bdFriendInfo();

		~bdFriendInfo();

		/// Deserialize the friend info from a buffer.
		/// \param buffer[in|out]	The buffer from which the member variables 
		///							are deserialized.
		/// \return 
		///		- True, if deserialization completed successfully.
		///		- False, otherwise.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		/// Inherited from bdTaskResult
		virtual bdUInt sizeOf();

		/// The friend's user ID
		bdOnlineUserID m_userID;

		/// The friend's user name
		bdNChar8 m_userName[BD_MAX_USERNAME_LENGTH];

		/// Status flag to indicate the online status
		bdBool m_isOnline;

		/// The friend's group ID
		bdUByte8 m_groupID;
};


#endif // BD_FRIEND_INFO_H
