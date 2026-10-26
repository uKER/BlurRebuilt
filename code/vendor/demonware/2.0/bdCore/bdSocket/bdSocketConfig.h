// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Configuration options for sockets.

#ifndef BD_SOCKET_CONFIG_H
#define BD_SOCKET_CONFIG_H

#include <bdPlatform/bdPlatformConfig/bdPlatformConfig.h>
#if defined BD_COMPILE_RELAY_SOCKET_ROUTER
#include <bdSocket/bdRelay/bdRelayPacket.h>
#endif

#if defined BD_PLATFORM_XBOX
// The real UDP port used by the XBox network stack.
#	define BD_XBOX_PORT (3074)
#endif

/// The maximum size of a Datagram. A Datagram larger than this will not
/// be sent from the network engine.
/// This value is low enough to be less than most networks
/// MTU size.
#define BD_MAX_DATAGRAM_SIZE (1288)

/// The max payload that can be sent/received through calls to sendTo
/// and recvFrom.
#define BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE ( 1264 )

#if defined BD_COMPILE_RELAY_SOCKET_ROUTER
#define BD_MAX_PAYLOAD_SIZE ( BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE - BD_RELAY_MAX_OVERHEAD )
#else
#define BD_MAX_PAYLOAD_SIZE ( BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE )
#endif

#endif // BD_SOCKET_CONFIG_H
