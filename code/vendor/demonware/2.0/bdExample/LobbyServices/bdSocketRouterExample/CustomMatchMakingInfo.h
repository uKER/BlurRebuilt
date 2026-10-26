// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 UNIX OSX IPHONE PS2 PS3 PSP-INFRA WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef BD_CUSTOM_MATCH_MAKING_INFO_H
#define BD_CUSTOM_MATCH_MAKING_INFO_H

#include <bdLobby/bdMatchMaking/bdMatchMakingInfo.h>

#define BD_MAX_SESSIONNAME_LENGTH (16u)

/// This class derives from bdMatchMakingInfo to add custom data attributes.
/// It also overrides the base class serialize/deserialize functions so that the
/// custom attributes are transmitted and received correctly
/// See the Advanced MatchMaking tutorial for more info.
class CustomMatchMakingInfo : public bdMatchMakingInfo
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		CustomMatchMakingInfo();

		~CustomMatchMakingInfo();

		CustomMatchMakingInfo(const bdNChar8 *const sessionName,
			const bdInt levelNum,
			const bdInt gameType);

		bdUWord getSessionName(bdNChar8 *const str) const;
		bdUWord setSessionName(const bdNChar8 *const sessionName);

		bdInt getLevelNum() const;
		void setLevelNum(bdInt levelNum);

		bdInt getGameType() const;
		void setGameType(bdInt gameType);

		/// Serialize all matchmaking info into a buffer.
		/// This is used to transmit this information to the service.
		/// \param param[out]	The buffer into which the member variables are 
		///						serialized.
		virtual void serialize(bdByteBuffer &buffer) const;

		/// Deserialize matchmaking info from a buffer.
		/// This is used to decode matchmaking results received from the service.
		/// \param param[in]	The buffer from which the matchmaking attributes
		///						should be deserialized.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		// Custom fields
		bdNChar8 m_sessionName[BD_MAX_SESSIONNAME_LENGTH+1];
		bdInt m_levelNum;
		bdSecurityKey m_secKey;
};

#endif // BD_CUSTOM_MATCH_MAKING_INFO_H
