// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An IP discovery protocol packet.

#ifndef BD_IP_DISCOVERY_PACKET_H
#define BD_IP_DISCOVERY_PACKET_H

#include <bdCore/bdCore.h>

class bdIPDiscoveryPacket
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdIPDiscoveryPacket();

		bdBool serialize(void *data, const bdUInt size, const bdUInt offset, bdUInt &newOffset) const;
		bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt &newOffset);

		bdUByte8 getType() const;

		bdUInt16 getProtocolVersion() const;

	protected:

		bdUByte8 m_type;
		bdUInt16 m_protocolVersion;
};


#endif // BD_IP_DISCOVERY_PACKET_H


