// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdPacket/bdNATTypeDiscoveryPacketReply.h>
#include <bdSocket/bdNAT/bdNAT.h>

bdNATTypeDiscoveryPacketReply::bdNATTypeDiscoveryPacketReply() :
m_type(0),
m_protocolVersion(0)
{
}

bdNATTypeDiscoveryPacketReply::bdNATTypeDiscoveryPacketReply(const bdAddr &mappedAddr,
							  const bdAddr &secAddr,
							  const bdUInt16 protocolVersion)
: m_type(BD_NTDP_BINDING_RESPONSE),
m_protocolVersion(protocolVersion),
m_mappedAddr(mappedAddr),
m_secAddr(secAddr)
{
}

bdBool bdNATTypeDiscoveryPacketReply::serialize(void *data,
												const bdUInt size,
												const bdUInt offset,
												bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_type);
	if (m_protocolVersion == 0x01)
	{
		status = status && bdBytePacker::skipBytes(data, size, newOffset, newOffset, 1); // v.1 pad
	}
	status = status && bdBytePacker::appendBasicType(data, size, newOffset,
		newOffset, m_protocolVersion);

	status = status && m_mappedAddr.serialize(data, size, newOffset, newOffset);
	if (m_protocolVersion == 0x01)
	{
		status = status && bdBytePacker::skipBytes(data, size, newOffset, newOffset, 2); // v.1 pad
	}

	status = status && m_secAddr.serialize(data, size, newOffset, newOffset);
	if (m_protocolVersion == 0x01)
	{
		status = status && bdBytePacker::skipBytes(data, size, newOffset, newOffset, 2); // v.1 pad
	}

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdBool bdNATTypeDiscoveryPacketReply::deserialize(const void* data,
												  const bdUInt size,
												  const bdUInt offset,
												  bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_type);

	if( m_type >= BD_NTDP_TYPE_MIN && m_type <= BD_NTDP_TYPE_MAX )
	{
		// Attempt to treat this as protocol version 2
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

		status = status && m_mappedAddr.deserialize(data, size, newOffset, newOffset);
		if (m_protocolVersion == 0x01)
		{
			status = status && bdBytePacker::skipBytes(data, size, newOffset,
				newOffset, 2); // v.1 padding
		}
		status = status && m_secAddr.deserialize(data, size, newOffset, newOffset);
	}
	else
	{
		status = false;		
	}

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdUByte8 bdNATTypeDiscoveryPacketReply::getType() const
{
	return m_type;
}

const bdAddr& bdNATTypeDiscoveryPacketReply::getMappedAddr() const
{
	return m_mappedAddr;
}

const bdAddr& bdNATTypeDiscoveryPacketReply::getSecAddr() const
{
	return m_secAddr;
}
