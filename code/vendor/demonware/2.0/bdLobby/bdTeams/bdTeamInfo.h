// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_TEAM_INFO_H
#define BD_TEAM_INFO_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

class bdTeamInfo : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdTeamInfo();

		/// Destructor.
		virtual ~bdTeamInfo();

		/// Deserialize any class members.
		/// Override this function in any subclass
		virtual bdBool deserialize(bdByteBufferRef buffer);

		/// Size of instances of this class. 
		/// Override this function in any subclass
		virtual bdUInt sizeOf();

		/// The team ID
		bdTeamID m_teamID;

		/// The owner ID
		bdOnlineUserID m_ownerUserID;
		
		/// The team name
		bdNChar8 m_teamName[BD_MAX_TEAMNAME_LENGTH];

		/// The member status (owner/administrator/ordinary)
		bdUByte8 m_memberStatus;
};

#endif // BD_TEAM_INFO_H
