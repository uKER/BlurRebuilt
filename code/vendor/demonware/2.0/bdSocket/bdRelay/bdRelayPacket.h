// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Encapsulates a relay packet, with serialization and deserialization functions

#ifndef BD_RELAY_PACKET_H
#define BD_RELAY_PACKET_H

#define BD_DTLS_PACKET_FIELD_SIZE (2u)
#define BD_RELAY_PACKET_HEADER_SIZE (2u)
#define BD_RELAY_VERSION_CURRENT (2u)
#define BD_RELAY_MAX_OVERHEAD ( BD_RELAY_PACKET_HEADER_SIZE + 2 + (2 * BD_COMMON_ADDR_SERIALIZED_SIZE ) + 2)

#define BD_RELAY_TYPE_MIN	(50u)
#define BD_RELAY_TYPE_MAX	(59u)
#define BD_RELAY_INVALID_TYPE (0u)

enum bdRelayPacketType
{
	BD_DIRECT = BD_RELAY_TYPE_MIN,
	BD_RELAY,
	BD_RELAY_FORWARD,
	BD_RELAY_FINAL,
	BD_RELAY_ROUTE_ADD,
	BD_RELAY_ROUTE_REMOVE,
	BD_RELAYER_INFO_REQUEST,
	BD_RELAYER_INFO_REPLY,
}; // enum bdRelayPacketTypes

class routePacket
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	bdNChar8* packetData;
	bdUInt packetLength;
};

#endif // BD_RELAY_PACKET_H
