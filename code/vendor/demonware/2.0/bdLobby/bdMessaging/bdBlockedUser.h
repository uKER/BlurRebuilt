// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_BLOCKED_USER_H
#define BD_BLOCKED_USER_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>


class bdBlockedUser : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdBlockedUser();

		~bdBlockedUser();

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		bdUInt64 m_userID;
		bdNChar8 m_userName[BD_MAX_USERNAME_LENGTH];
};


#endif // BD_BLOCKED_USER_H
