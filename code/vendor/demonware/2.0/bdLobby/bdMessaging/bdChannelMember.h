// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Describes a subscriber of a chat channel.

#ifndef BD_CHANNEL_MEMBER_H
#define BD_CHANNEL_MEMBER_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>


class bdChannelMember : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdChannelMember();

		~bdChannelMember();

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		bdUInt64 m_userID;
		bdNChar8 m_userName[BD_MAX_USERNAME_LENGTH];
};


#endif // BD_CHANNEL_MEMBER_H
