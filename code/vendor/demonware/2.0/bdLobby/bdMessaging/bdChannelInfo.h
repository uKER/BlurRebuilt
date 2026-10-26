// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Describes a chat channel. 

#ifndef BD_CHANNEL_INFO_H
#define BD_CHANNEL_INFO_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

/// Describes a chat channel.
class bdChannelInfo : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdChannelInfo();

		~bdChannelInfo();

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// The chat channel ID
		bdUInt64 m_channelID;

		/// Number of players currently subscribe to the chat channel
		bdUInt32 m_numSubscribers;

		/// Maximum number of players that are allowed in the chat channel
		bdUInt32 m_maxSubscribers;
};

#endif // BD_CHANNEL_INFO_H
