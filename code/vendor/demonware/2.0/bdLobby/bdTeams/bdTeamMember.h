// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Basic container class for a team member. Is not an 'info' style class. 
//			Doesn't contain (de)serialize fns.

#ifndef BD_TEAM_MEMBER_H
#define BD_TEAM_MEMBER_H

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

// Team Member Status
#define BD_ORDINARY		0
#define BD_ADMIN		1
#define BD_OWNER		2

class bdTeamMember : public bdTaskResult
{
	public:
		
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdTeamMember();

		~bdTeamMember();


		/// Deserialize any class members.
		/// Override this function in any subclass
		virtual bdBool deserialize(bdByteBufferRef buffer);
		
		/// Size of instances of this class. 
		/// Override this function in any subclass
		virtual bdUInt sizeOf();
	
		/// Team ID
		bdTeamID m_teamID;

		/// User ID of the team member
		bdOnlineUserID m_userID;
		
		/// User name of the team member
		bdNChar8 m_userName[BD_MAX_USERNAME_LENGTH];
		
		/// Online status
		bdBool m_isOnline;

		/// Team member status (owner/administrator/ordinary)
		bdUByte8 m_status;
};


#endif // BD_TEAM_MEMBER_H
