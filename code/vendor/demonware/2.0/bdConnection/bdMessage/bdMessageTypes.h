// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_MESSAGE_TYPES_H
#define BD_MESSAGE_TYPES_H

#define BD_MESSAGE								(0u)
#define	BD_CONNECTION_DISCONNECT				(1u)
#define	BD_CONNECTION_PING						(2u)
#define BD_CONNECTION_SET_PARAM					(3u)

#define	BD_SERVER_UPDATE_MESSAGE				(4u)
#define BD_SERVER_REPLICA_MANAGER_MESSAGE		(5u)
#define BD_SERVER_PLAYER_MESSAGE				(6u)

#define BD_USER_COMMAND_MESSAGE					(7u)

#define BD_CLIENT_PLAYER_MESSAGE				(8u)
#define BD_CLIENT_UPDATE_ACK_MESSAGE			(9u)

#define BD_SESSION_JOIN_REQ						(10u)
#define BD_SESSION_JOIN_REPLY					(11u)
#define BD_SESSION_CONSISTENCY_MESSAGE			(12u)
#define BD_UPDATE_PEER_LIST						(13u)

#define BD_DELIVERY_MGR_ACTIVATE_REQUEST		(14u)
#define BD_DELIVERY_MGR_BATCH_MESSAGE			(15u)
#define BD_DELIVERY_MGR_RECOVERY_MESSAGE		(16u)
#define BD_DELIVERY_MGR_PENDING_JOIN			(17u)
#define BD_DELIVERY_MGR_INIT_DATA				(18u)
#define BD_DELIVERY_MGR_INIT_COMPLETE			(19u)
												
#define BD_PLAYER_MGR_INIT_DATA					(20u)
#define BD_ROUND_INIT_DATA						(21u)
												
#define BD_STREAM_CREATE						(22u)
#define BD_STREAM_OUT_STREAM_CLOSE				(23u)
#define BD_STREAM_IN_STREAM_FINISHED			(24u)
#define BD_STREAM_IN_STREAM_CLOSE				(25u)
#define BD_STREAM_DATA							(26u)
												
#define BD_DISCOVERY_BROADCAST					(27u)
#define BD_DISCOVERY_REPLY						(28u)
												
#define BD_USER_MESSAGE_TYPE					(29u)

typedef bdUByte8 bdMessageType;

#endif // BD_MESSAGE_TYPES_H
