// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_ONLINE_USER_INFO_H
#define BD_ONLINE_USER_INFO_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

/// Container class to store information about whether a user is online or not
class bdOnlineUserInfo : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		bdOnlineUserInfo();

		~bdOnlineUserInfo();

		/// Deserialize the online user info from a buffer.
		/// \param buffer[in|out]	The buffer from which the member variables 
		///							are deserialized.
		/// \return 
		///		- True, if deserialization completed successfully.
		///		- False, otherwise.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		/// Inherited from bdTaskResult
		virtual bdUInt sizeOf();

		/// The user's user ID
		bdOnlineUserID m_userID;

		/// Status flag to indicate the online status
		bdBool m_isOnline;
};


#endif // BD_ONLINE_USER_INFO_H
