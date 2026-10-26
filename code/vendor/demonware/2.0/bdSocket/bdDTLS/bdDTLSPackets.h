// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_DTLS_PACKETS_H
#define BD_DTLS_PACKETS_H

#include <bdCore/bdCore.h>

#define BD_DTLS_VERSION		(2u)
#define BD_DTLS_VERSION_CURRENT (2u)
#define BD_DTLS_TYPE_MIN	(1u)
#define BD_DTLS_TYPE_MAX	(9u)
#define BD_DTLS_HMAC_SIZE	(8u)

enum bdDTLSPacketTypes
{
	BD_DTLS_INIT = BD_DTLS_TYPE_MIN,
	BD_DTLS_INIT_ACK,
	BD_DTLS_COOKIE_ECHO,
	BD_DTLS_COOKIE_ACK,
	BD_DTLS_ERROR,
	BD_DTLS_DATA
};


#endif // BD_DTLS_PACKETS_H
