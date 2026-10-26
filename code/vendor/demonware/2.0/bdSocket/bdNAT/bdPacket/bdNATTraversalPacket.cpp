// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdPacket/bdNATTraversalPacket.h>

bdUInt bdNATTraversalPacket::serializedSize = 0;

bdNATTraversalPacket::bdNATTraversalPacket() :
m_type(0),
m_protocolVersion(0),
m_ident(0)
{
}

bdNATTraversalPacket::bdNATTraversalPacket(const bdUByte8 &type,
										   const bdUInt32 ident,
										   const bdAddr &addrSrc, 
										   const bdAddr &addrDest)
: m_type(type),
  m_protocolVersion(BD_NAT_TRAV_PROTOCOL_VERSION),
  m_ident(ident),
  m_addrSrc(addrSrc),
  m_addrDest(addrDest)
{
}

bdBool bdNATTraversalPacket::serialize(void *data,
									   const bdUInt size,
									   const bdUInt offset,
									   bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_type);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset,
													 newOffset, m_protocolVersion);
	status = status && bdBytePacker::appendBuffer(data, size, newOffset,
												  newOffset, m_hmac, BD_HMAC_SIZE);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_ident);
	status = status && m_addrSrc.serialize(data, size, newOffset, newOffset);
	status = status && m_addrDest.serialize(data, size, newOffset, newOffset);

	if (!status)
	{
		newOffset = offset;
	}

	return status;
}

bdBool bdNATTraversalPacket::deserialize(const void* data,
										 const bdUInt32 size,
										 const bdUInt offset,
										 bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_type);

	status = status && bdBytePacker::removeBasicType(data, size, newOffset,
													 newOffset, m_protocolVersion);


	if(m_protocolVersion >= BD_NAT_TRAV_PROTOCOL_VERSION)
	{
		status = status && bdBytePacker::removeBuffer(data, size, newOffset, 
													newOffset, m_hmac, BD_HMAC_SIZE);
		status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_ident);
		status = status && m_addrSrc.deserialize(data, size, newOffset, newOffset);
		status = status && m_addrDest.deserialize(data, size, newOffset, newOffset);
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

bdUInt bdNATTraversalPacket::getSize() const
{
	if (serializedSize == 0)
	{
		BD_ASSERT(serialize(BD_NULL, BD_UINT16_MAX, 0, serializedSize) && serializedSize > 0, "Size estimate failed.");
	}

	return serializedSize;
}

bdUByte8 bdNATTraversalPacket::getType() const
{
	return m_type;
}

void bdNATTraversalPacket::setType(bdUByte8 type)
{
	m_type = type;
}

bdUInt32 bdNATTraversalPacket::getIdentifier() const
{
	return m_ident;
}

const bdAddr& bdNATTraversalPacket::getAddrDest() const
{
	return m_addrDest;
}

const bdAddr& bdNATTraversalPacket::getAddrSrc() const
{
	return m_addrSrc;
}

const bdUByte8* bdNATTraversalPacket::getHMAC() const
{
	return m_hmac;
}

void bdNATTraversalPacket::setHMAC(const bdUByte8* hmac, const bdUInt size)
{
	BD_ASSERT(size == BD_NAT_TRAV_HMAC_SIZE, "Trying to set HMAC of invalid length.");
	bdMemcpy(m_hmac, hmac, BD_NAT_TRAV_HMAC_SIZE);
}

bdUInt16 bdNATTraversalPacket::getProtocolVersion() const
{
	return m_protocolVersion;
}
