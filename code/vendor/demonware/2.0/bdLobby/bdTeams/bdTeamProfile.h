// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_TEAM_PROFILE_H
#define BD_TEAM_PROFILE_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

/// Container class for team profiles (public or private profiles).
/// Derive from this class to store title specific attributes.
class bdTeamProfile : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdTeamProfile();

		/// Destructor.
		virtual ~bdTeamProfile();

		/// Serialize the team info for transmission to the MatchMaking+ service.
		/// Override this function in any subclass
		/// \param param[out]	This is returned with the data written to it.
		virtual void serialize(bdByteBuffer &buffer) const;

		/// Deserialize any class members.
		/// Override this function in any subclass
		virtual bdBool deserialize(bdByteBufferRef buffer);

		/// Size of instances of this class. 
		/// Override this function in any subclass
		virtual bdUInt sizeOf();

		/// The team ID
		bdTeamID m_teamID;

};

#endif // BD_TEAM_PROFILE_H
