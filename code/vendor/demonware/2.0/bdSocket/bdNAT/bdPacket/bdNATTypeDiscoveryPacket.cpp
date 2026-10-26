// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdPacket/bdNATTypeDiscoveryPacket.h>
#include <bdSocket/bdNAT/bdNAT.h>


bdNATTypeDiscoveryPacket::bdNATTypeDiscoveryPacket(
	bdNATTypeDiscoveryPacketRequest request)
: m_type(BD_NTDP_BINDING_REQUEST),
m_protocolVersion(BD_NTDP_PROTOCOL_VERSION),
m_request(request)
{
}

bdBool bdNATTypeDiscoveryPacket::serialize(void *data,
										   const bdUInt size,
										   const bdUInt offset,
										   bdUInt &newOffset) const
{
	// TODO: remove paddings once protocol version 1 is phased out.
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_type);
#if (BD_NTDP_PROTOCOL_VERSION == 0x01)
	status = status && bdBytePacker::skipBytes(data, size, newOffset, newOffset, 1); // v.1 pad
#endif // protocol version 1
	status = status && bdBytePacker::appendBasicType(data, size, newOffset,
			newOffset, m_protocolVersion);

	// 1 byte is more than enough for request type
	status = status && bdBytePacker::appendBasicType(data, size, newOffset,
			newOffset, static_cast<bdByte8>(m_request));

#if (BD_NTDP_PROTOCOL_VERSION == 0x01)
	const bdUByte8 zero = 0;
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, zero); // v.1 pad
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, zero); // v.1 pad
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, zero); // v.1 pad
#endif // protocol version 1

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdBool bdNATTypeDiscoveryPacket::deserialize(const void* data,
											 const bdUInt size,
											 const bdUInt offset,
											 bdUInt &newOffset)
{
	// TODO: remove paddings once protocol version 1 is phased out.
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::removeBasicType(data, size, newOffset,
			newOffset, m_type);
	// Attempt to treat this as protocol version 2.
	status = status && bdBytePacker::removeBasicType(data, size, newOffset,
			newOffset, m_protocolVersion);
	if (m_protocolVersion != BD_NTDP_PROTOCOL_VERSION_CURRENT)
	{
		// Protocol didn't come out as current version . Assume is version 1.
		status = status && bdBytePacker::rewindBytes(data, size, newOffset,
			newOffset, sizeof(m_protocolVersion));
		status = status && bdBytePacker::skipBytes(data, size, newOffset,
			newOffset, 1); // v.1 padding
		status = status && bdBytePacker::removeBasicType(data, size, newOffset,
			newOffset, m_protocolVersion);
		if (m_protocolVersion != 0x01)
		{
			// Is not version 1. Give up.
			newOffset = offset;
			return false;
		}
	}

	bdByte8 tmpByte = -1;
	status = status && bdBytePacker::removeBasicType(data, size, newOffset,	newOffset, tmpByte);
	m_request = static_cast<bdNATTypeDiscoveryPacketRequest>(tmpByte);

	if (m_protocolVersion == 0x01)
	{
		bdUByte8 zero;
		status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, zero); // v.1 pad
		status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, zero); // v.1 pad
		status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, zero); // v.1 pad
	}

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdUByte8 bdNATTypeDiscoveryPacket::getType() const
{
	return m_type;
}

bdNATTypeDiscoveryPacket::bdNATTypeDiscoveryPacketRequest bdNATTypeDiscoveryPacket::getRequest() const
{
	return m_request;
}

bdUInt16 bdNATTypeDiscoveryPacket::getProtocolVersion() const
{
	return m_protocolVersion;
}

