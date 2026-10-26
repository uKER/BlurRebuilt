// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdPacket/bdIPDiscoveryPacket.h>
#include <bdSocket/bdNAT/bdNAT.h>

bdIPDiscoveryPacket::bdIPDiscoveryPacket()
: m_type(BD_IP_DISC_REQ),
m_protocolVersion(BD_IP_DISC_PROTOCOL_VERSION)
{
}

bdBool bdIPDiscoveryPacket::serialize(void *data,
									  const bdUInt size,
									  const bdUInt offset,
									  bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_type);
#if (BD_IP_DISC_PROTOCOL_VERSION == 0x01)
	status = status && bdBytePacker::skipBytes(data, size, newOffset, newOffset, 1);
#endif // protocol version 1
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_protocolVersion);

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdBool bdIPDiscoveryPacket::deserialize(const void* data,
										const bdUInt size,
										const bdUInt offset,
										bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_type);
	// Attempt to treat this as protocol version 2
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_protocolVersion);
	if (m_protocolVersion != BD_IP_DISC_PROTOCOL_VERSION_CURRENT)
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

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdUByte8 bdIPDiscoveryPacket::getType() const
{
	return m_type;
}

bdUInt16 bdIPDiscoveryPacket::getProtocolVersion() const
{
	return m_protocolVersion;
}


