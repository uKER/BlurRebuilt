// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A NAT type discovery protocol reply packet.

#ifndef BD_NAT_TYPE_DISCOVERY_PACKET_REPLY_H
#define BD_NAT_TYPE_DISCOVERY_PACKET_REPLY_H

#include <bdCore/bdCore.h>

class bdNATTypeDiscoveryPacketReply
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdNATTypeDiscoveryPacketReply();

		bdNATTypeDiscoveryPacketReply(const bdAddr &mappedAddr, const bdAddr &secAddr, const bdUInt16 protocolVersion);

		bdBool serialize(void *data, const bdUInt size, const bdUInt offset, bdUInt &newOffset) const;
		bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt &newOffset);

		bdUByte8 getType() const;
		const bdAddr& getMappedAddr() const;
		const bdAddr& getSecAddr() const;

	protected:

		bdUByte8 m_type;
		bdUInt16 m_protocolVersion;
		bdAddr m_mappedAddr;
		bdAddr m_secAddr;
};

#endif // BD_NAT_TYPE_DISCOVERY_PACKET_REPLY_H

