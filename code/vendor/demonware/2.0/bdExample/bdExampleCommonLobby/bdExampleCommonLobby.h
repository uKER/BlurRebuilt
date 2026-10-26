// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_EXAMPLE_COMMON_LOBBY_H
#define BD_EXAMPLE_COMMON_LOBBY_H

#include <bdExample/bdExampleCommon/bdExampleCommon.h>

// auth
#if !defined BD_PLATFORM_XENON &&  !defined BD_PLATFORM_XBOX
#include <bdLobby/bdAuthService.h>
#include <bdLobby/bdLobbyCommon/bdAuthCommon.h>
#endif

#include <bdLobby/bdAuthTicket.h>

// lobby
#include <bdLobby/bdLobbyService.h>

// remote tasks
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

// matchmaking
#include <bdLobby/bdMatchMaking/bdMatchMaking.h>
#include <bdLobby/bdMatchMaking/bdMatchMakingInfo.h>

// stats
#include <bdLobby/bdStats/bdStats.h>

// profiles
#include <bdLobby/bdProfiles/bdProfiles.h>
#include <bdLobby/bdTeams/bdTeams.h>

// messaging
#include <bdLobby/bdMessaging/bdMessaging.h>
#include <bdLobby/bdMessaging/bdLobbyMessageType.h>
#include <bdLobby/bdMessaging/bdChannelMember.h>




// channel chat
#include <bdLobby/bdMessaging/bdChannelInfo.h>

// utilities
#include <bdExample/bdExampleCommonLobby/PrintableList.h>

#define BD_LOG_LEVEL "example"

#define START_KEY	BD_BUTTON1
#define START_TEXT	"Start: " BD_BUTTON1_DESC

#define SKIP_KEY	BD_BUTTON2
#define SKIP_TEXT	"Skip: " BD_BUTTON2_DESC

#define AUTH_ADDRESS	"sbox200-auth.mmp3.demonware.net"
#define AUTH_PORT		(3074)

#define LSG_ADDRESS		"sbox200ng.lsg-imt.mmp3.demonware.net"
#define LSG_PORT		(3074)

#define DNS_LOOKUP_TIMEOUT (5.0f)

// Task status values used in examples
enum TaskStatus
{
	READY,
	PENDING,
	DONE,
	FAILED,
	SKIPPED
};

// Safe string handling

void BD_CALL IndexStrings(const bdNChar8 *const strings[], 
						  const bdUInt numStrings, 
						  const bdUInt index, 
						  bdString& dest);

// Lobby error code to string translation

void BD_CALL LobbyErrorCodeToString(const bdLobbyErrorCode code, 
									bdNChar8 *const buffer,
									const bdUWord bufferSize);

// Lobby message to string translation

//void BD_CALL LobbyMessageTypeToString(const bdLobbyMessageType type,
//									  bdNChar8 *const buffer,
//									  const bdUWord bufferSize);

#ifndef BD_PLATFORM_XENON
// DNS lookups
TaskStatus BD_CALL StartGetHostByName(const bdNChar8 *const domainNameStr,
									  bdGetHostByName **getHostByName,
									  const bdInetAddr resolvedAddr);

TaskStatus BD_CALL PumpGetHostByName(bdGetHostByName **getHostByName,
									 bdInetAddr &resolvedAddr);

#endif //#ifndef BD_PLATFORM_XENON

// Remote tasks

TaskStatus BD_CALL CheckRemoteTaskStatus(bdRemoteTaskRef remoteTask);

void BD_CALL CloseRemoteTask(bdRemoteTaskRef remoteTask);


// Common print functions

void BD_CALL printInfo(const bdChannelInfo& channelInfo,
					   bdNChar8 *buf,
					   const bdUInt length);

void BD_CALL printInfo(const bdChannelMember& memberInfo,
					   bdNChar8 *buf,
					   const bdUInt length);

#endif // BD_EXAMPLE_COMMON_LOBBY_H
