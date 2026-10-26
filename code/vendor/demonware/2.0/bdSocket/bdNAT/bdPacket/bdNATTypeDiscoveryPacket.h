// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A NAT type discovery protocol packet.

#ifndef BD_NAT_TYPE_DISCOVERY_PACKET_H
#define BD_NAT_TYPE_DISCOVERY_PACKET_H

#include <bdCore/bdCore.h>

class bdNATTypeDiscoveryPacket
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdNATTypeDiscoveryPacketRequest
		{
			BD_NTDP_SAME_ADDR,
			BD_NTDP_DIFF_PORT,
			BD_NTDP_DIFF_IP,
			BD_NTDP_DIFF_PORT_IP,
			BD_NTDP_INVALID_TYPE
		};

		bdNATTypeDiscoveryPacket(bdNATTypeDiscoveryPacketRequest request);

		bdBool serialize(void *data, const bdUInt size, const bdUInt offset, bdUInt &newOffset) const;
		bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt &newOffset);

		bdUByte8 getType() const;
		bdNATTypeDiscoveryPacketRequest getRequest() const;

		bdUInt16 getProtocolVersion() const;

	protected:
		bdUByte8 m_type;
		bdUInt16 m_protocolVersion;
		bdNATTypeDiscoveryPacketRequest m_request;
};


#endif // BD_NAT_TYPE_DISCOVERY_PACKET_H
