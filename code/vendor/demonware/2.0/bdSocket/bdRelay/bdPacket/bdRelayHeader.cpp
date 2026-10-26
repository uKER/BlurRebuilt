// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdRelay/bdPacket/bdRelayHeader.h>

bdRelayHeader::bdRelayHeader() :
m_type(BD_RELAY_INVALID_TYPE),
m_version(BD_RELAY_VERSION_CURRENT)
{
}

bdRelayHeader::~bdRelayHeader()
{
}

bdRelayHeader::bdRelayHeader(bdUInt type)
: m_version(BD_RELAY_VERSION_CURRENT)
{
	m_type = static_cast<bdUByte8>(type);
}

bdBool bdRelayHeader::serializeHeader(void *data,
									   const bdUInt size,
									   const bdUInt offset,
									   bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_type);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_version);

	if (!status)
	{
		newOffset = offset;
	}

	return status;
}

bdBool bdRelayHeader::deserializeHeader(const void* data,
										 const bdUInt size,
										 const bdUInt offset,
										 bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_type);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_version);

	if (!status)
	{
		newOffset = offset;
	}

	return status;
}

bdUByte8 bdRelayHeader::getType() const
{
	return m_type;
}


