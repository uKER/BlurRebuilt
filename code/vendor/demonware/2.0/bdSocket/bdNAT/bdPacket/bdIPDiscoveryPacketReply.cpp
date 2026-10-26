// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdPacket/bdIPDiscoveryPacketReply.h>
#include <bdSocket/bdNAT/bdNAT.h>

bdIPDiscoveryPacketReply::bdIPDiscoveryPacketReply(const bdAddr &addr, bdUInt16 protocolVersion)
: m_type(BD_IP_DISC_REP),
m_protocolVersion(protocolVersion),
m_addr(addr)
{
}
 
bdIPDiscoveryPacketReply::bdIPDiscoveryPacketReply()
: m_type(0), 
m_protocolVersion(0)
{
}

bdBool bdIPDiscoveryPacketReply::serialize(void *data,
										   const bdUInt size,
										   const bdUInt offset,
										   bdUInt &newOffset) const
{
	// TODO: remove paddings once protocol version 1 is phased out.
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_type);
//#if (BD_IP_DISC_PROTOCOL_VERSION == 0x01)
	if (m_protocolVersion == 0x01)
	{
		status = status && bdBytePacker::skipBytes(data, size, newOffset, newOffset, 1);
	}
//#endif // protocol version 1
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_protocolVersion);

	status = status && m_addr.serialize(data, size, newOffset, newOffset);
//#if (BD_IP_DISC_PROTOCOL_VERSION == 0x01)
	if (m_protocolVersion == 0x01)
	{
		status = status && bdBytePacker::skipBytes(data, size, newOffset, newOffset, 2);
	}
//#endif // protocol version 1

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdBool bdIPDiscoveryPacketReply::deserialize(const void* data,
											 const bdUInt size,
											 const bdUInt offset,
											 bdUInt &newOffset)
{
	// TODO: remove paddings once protocol version 1 is phased out.
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_type);

	if( m_type >= BD_IP_DISC_TYPE_MIN && m_type <= BD_IP_DISC_TYPE_MAX )
	{
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

		status = status && m_addr.deserialize(data, size, newOffset, newOffset);
		if (m_protocolVersion == 0x01)
		{
			status = status && bdBytePacker::skipBytes(data, size, newOffset, newOffset, 2);
		}
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

bdUByte8 bdIPDiscoveryPacketReply::getType() const
{
	return m_type;
}

const bdAddr& bdIPDiscoveryPacketReply::getAddr() const
{
	return m_addr;
}

bdUInt16 bdIPDiscoveryPacketReply::getProtocolVersion() const
{
	return m_protocolVersion;
}
