// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdDTLS/bdPacket/bdDTLSHeader.h>

bdUInt bdDTLSHeader::serializedSize = 0;

bdDTLSHeader::bdDTLSHeader() :
m_type(0),
m_version(0),
m_vtag(0),
m_counter(0)
{
}

bdDTLSHeader::~bdDTLSHeader()
{
}

bdDTLSHeader::bdDTLSHeader(bdDTLSPacketTypes type,
						   bdUInt16 vtag,
						   bdUInt16 counter)
: m_version(BD_DTLS_VERSION),
m_vtag(vtag),
m_counter(counter)
{
	m_type = static_cast<bdUByte8>(type);
}

bdBool bdDTLSHeader::serialize(void *data,
							   const bdUInt size,
							   const bdUInt offset,
							   bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_type);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_version);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_vtag);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_counter);

	if (!status)
	{
		newOffset = offset;
	}

	return status;
}

bdBool bdDTLSHeader::deserialize(const void* data,
								 const bdUInt size,
								 const bdUInt offset,
								 bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_type);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_version);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_vtag);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_counter);

	if (!status)
	{
		newOffset = status;
	}

	return status;
}

bdUByte8 bdDTLSHeader::getType() const
{
	return m_type;
}

bdUInt16 bdDTLSHeader::getVtag() const
{
	return m_vtag;
}

bdUInt16 bdDTLSHeader::getCounter() const
{
	return m_counter;
}

bdUInt bdDTLSHeader::getSerializedSize() const
{
	if (serializedSize == 0)
	{
		BD_ASSERT(bdDTLSHeader::serialize(BD_NULL, BD_UINT16_MAX, 0, serializedSize),
			"Size measurment failed.");
	}
	return serializedSize;
}


