// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_NOTIFICATION_H
#define BD_NOTIFICATION_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

#include <bdLobby/bdMessaging/bdLobbyMessageType.h>

class bdNotification : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdNotification();

		virtual ~bdNotification();

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();
 
		/// The notification type
		bdNotificationType m_type;

		/// Time stamp when the notification was created
		bdUInt32 m_timeStamp;

		/// A unique notification ID
		bdUInt64 m_ID;

		/// User ID of the relevant user that caused the notification
		bdOnlineUserID m_sourceUser;

		/// User name of the user that caused the notification
		bdNChar8 m_sourceUserName[BD_MAX_USERNAME_LENGTH];

		/// User ID/Name of the player that the notification was about 
		bdOnlineUserID m_affectedUser;
		bdNChar8 m_affectedUserName[BD_MAX_USERNAME_LENGTH];

		/// Relevant team ID in case the notification was pertaining to a team
		bdTeamID m_teamID;

		/// Team name in case the notification was pertaining to a team
		bdNChar8 m_teamName[BD_MAX_TEAMNAME_LENGTH];
		
};



#endif // BD_NOTIFICATION_H
