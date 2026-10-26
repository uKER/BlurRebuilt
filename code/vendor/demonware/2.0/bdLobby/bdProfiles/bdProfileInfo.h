// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// Purpose: This wraps the basic info for a single user profile.

#ifndef BD_PROFILES_INFO_H
#define BD_PROFILES_INFO_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

/// This wraps the basic info for a single user profile
/// This class should be derived from and its serialize/deserialize functions
/// should be overridden so that title-specific profile info can be transmitted
/// to and retrieved from the MatchMaking+ service.
class bdProfileInfo : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdProfileInfo();

		/// Destructor.
		virtual ~bdProfileInfo();

		/// Serialize the profile info for transmission to the MatchMaking+ service.
		/// \param param[out]	This is returned with the data written to it.
		virtual void serialize(bdByteBuffer &buffer)const;

		/// Deserialize the stats from a buffer.
		/// \param param[in]	This buffer contains stats transmitted by the
		///						MatchMaking+ service.
		/// \return
		///		- True, the buffer was de-serialized correctly.
		///		- False, there was an error de-serializing the buffer.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		/// Returns the size in bytes of an instance of bdProfileInfo
		/// Override this function in any subclass.
		/// \return The size in bytes
		virtual bdUInt sizeOf();

		/// Get the entity ID associated with this profile. This can either be
		/// a user ID or a team ID.
		/// \return	The entityID.
		bdEntityID getEntityID() const;

		/// The user/team id
		bdEntityID m_entityID;

};

#endif // BD_PROFILES_INFO_H
