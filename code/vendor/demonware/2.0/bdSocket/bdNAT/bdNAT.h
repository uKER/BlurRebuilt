// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_NAT_H
#define BD_NAT_H

#include <bdCore/bdCore.h>

#include <bdSocket/bdPacketInterceptor.h>


// Nat Traversal

#define BD_NAT_TRAV_TYPE_MIN	(10u)
#define BD_NAT_TRAV_TYPE_MAX	(19u)

#define BD_NAT_TRAV_INTRO_REQ		(BD_NAT_TRAV_TYPE_MIN + 0)
#define BD_NAT_TRAV_INTRO_FORWARD	(BD_NAT_TRAV_TYPE_MIN + 1)
#define BD_NAT_TRAV_INTRO_REPLY		(BD_NAT_TRAV_TYPE_MIN + 2)
#define BD_NAT_TRAV_INTRO			(BD_NAT_TRAV_TYPE_MIN + 3)
#define BD_NAT_TRAV_KEEP_ALIVE		(BD_NAT_TRAV_TYPE_MIN + 4)

// Lowest protocol version we understand (used for sending packets).
#define BD_NAT_TRAV_PROTOCOL_VERSION	(0x0002)
// Highest protocol version we understand.
#define BD_NAT_TRAV_PROTOCOL_VERSION_CURRENT (0x0002)

#define BD_NAT_TRAV_HMAC_SIZE (BD_HMAC_SIZE)


// Nat type Discovery

#define BD_NTDP_TYPE_MIN	(20u)
#define BD_NTDP_TYPE_MAX	(29u)

#define BD_NTDP_BINDING_REQUEST		(BD_NTDP_TYPE_MIN + 0)
#define BD_NTDP_BINDING_RESPONSE	(BD_NTDP_TYPE_MIN + 1)

#define BD_NTDP_PROTOCOL_VERSION	(0x0002)
#define BD_NTDP_PROTOCOL_VERSION_CURRENT	(0x0002)


// IP Discovery

#define BD_IP_DISC_TYPE_MIN		(30u)
#define BD_IP_DISC_TYPE_MAX		(39u)

#define BD_IP_DISC_REQ		(BD_IP_DISC_TYPE_MIN + 0)
#define BD_IP_DISC_REP		(BD_IP_DISC_TYPE_MIN + 1)

#define BD_IP_DISC_PROTOCOL_VERSION	(0x0002)
#define BD_IP_DISC_PROTOCOL_VERSION_CURRENT (0x0002)

#define BD_USER_PACKET_TYPE		(101u)

#endif // BD_NAT_H
