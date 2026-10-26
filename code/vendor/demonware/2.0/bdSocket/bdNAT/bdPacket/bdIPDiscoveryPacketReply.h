// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An IP discovery protocol reply packet.

#ifndef BD_IP_DISCOVERY_PACKET_REPLY_H
#define BD_IP_DISCOVERY_PACKET_REPLY_H

#include <bdCore/bdCore.h>

class bdIPDiscoveryPacketReply
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdIPDiscoveryPacketReply(const bdAddr& addr, bdUInt16 protocolVersion);
		bdIPDiscoveryPacketReply();

		bdBool serialize(void *data, const bdUInt size, const bdUInt offset, bdUInt &newOffset) const;
		bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt &newOffset);

		bdUByte8 getType() const;

		const bdAddr& getAddr() const;

		bdUInt16 getProtocolVersion() const;

	protected:

		bdUByte8 m_type;
		bdUInt16 m_protocolVersion;
		bdAddr m_addr;
};


#endif // BD_IP_DISCOVERY_PACKET_REPLY_H
