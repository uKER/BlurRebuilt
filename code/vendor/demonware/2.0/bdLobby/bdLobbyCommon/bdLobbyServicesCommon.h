// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Type definitions and constants relating to the lobby services.

#ifndef BD_LOBBY_SERVICES_COMMON_H
#define BD_LOBBY_SERVICES_COMMON_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyErrorCode.h>

// Title identifier
typedef bdUInt32 bdTitleID;

// An entity id constitutes either a user or a team id.
typedef bdUInt64 bdEntityID;

// Uniquely identifies an online user
typedef bdEntityID bdOnlineUserID;

// Uniquely identifies a team. 
typedef bdEntityID bdTeamID;

// LeaderBoard identifier
typedef bdUInt32 bdLeaderBoardID;


#define BD_MAX_USERNAME_LENGTH		(64u)
#define BD_MAX_GROUPNAME_LENGTH		(32u)
#define BD_MAX_FILE_NAME_LENGTH 	(128u)
#define BD_MAX_TEAMNAME_LENGTH		(64u)
#define BD_MAX_NUM_TEAM_MEMBERS		(50u)
#define BD_MAXIMUM_CUSTOM_MSG_SIZE 	(1024u)
#define BD_MAX_CHAT_MESSAGE_SIZE 	(1024u)
#define BD_MAX_RICH_PRESENCE_UPDATE (1024u)
#define BD_MAX_INVITE_ATTACHMENT_SIZE (1024u)
#define BD_MAX_LOCATION_LENGTH		(256u)
#define BD_MAX_URL_SIZE				(512u)
#define BD_MAX_SERVER_URL_SIZE		(128u)
#define BD_MAX_THUMB_DATA_SIZE		(1024*8)
#define BD_MAX_LINK_CODE_LENGTH		(64u)

// Service IDs
#define BD_TEAM_MANAGEMENT_SERVICE					(3u)
#define BD_STATS_SERVICE							(4u)
#define BD_MESSAGING_SERVICE						(6u)
#define BD_AUTH_SERVICE_ID							(7u)
#define BD_PROFILE_MANAGEMENT_SERVICE				(8u)
#define BD_FRIENDS_MANAGEMENT_SERVICE				(9u)
#define BD_STORAGE_SERVICE							(10u)
#define BD_CHANNEL_CHAT_SERVICE						(11u)
#define BD_TITLE_UTILITIES_SERVICE				    (12u)
#define BD_KEY_ARCHIVE_SERVICE						(15u)
#define BD_LSG_SERVICE								(18u)
#define BD_ARBITRATION_SERVICE						(19u)
#define BD_MATCHMAKING_SERVICE						(21u)
#define BD_FILE_STATS_SERVICE						(22u)
#define BD_COUNTER_SERVICE						    (23u)
#define BD_CONTENT_UNLOCK_SERVICE					(26u)
#define BD_DML_SERVICE								(27u)
#define BD_GROUP_SERVICE						    (28u)
#define BD_TWITTER_SERVICE							(35u)
#define BD_FACEBOOK_SERVICE							(36u)
#define BD_APPLE_PUSH_NOTIFICATION_SERVICE			(37u)
#define BD_STREAMING_SERVICE2						(50u)
#define BD_TAG_SERVICE								(52u)
#define BD_REPORT_CONTENT_SERVICE					(53u)
#define BD_VOTERANK_SERVICE							(55u)
#define BD_LINKCODE_SERVICE							(57u)

// Task Reply Message
#define BD_LOBBY_SERVICE_TASK_REPLY		(1u)
#define BD_LOBBY_SERVICE_PUSH_MESSAGE	(2u)
#define BD_LOBBY_ERROR_RESULT			(3u)
#define BD_LOBBY_CONNECTION_ID          (4u)
#define BD_LSG_SERVICE_TASK_REPLY		(5u)

#endif // BD_LOBBY_SERVICES_COMMON_H
