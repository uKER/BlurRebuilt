// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdDTLS/bdPacket/bdDTLSInit.h>

#include <bdCore/bdSocket/bdSecurityInfo.h>

#define BD_LOG_LEVEL "bdDTLSinit/foo"

bdDTLSInit::bdDTLSInit() :
m_initTag(0)
{
}

bdDTLSInit::bdDTLSInit(bdUInt16 initTag, 
					   const bdSecurityID& secID)
: bdDTLSHeader(BD_DTLS_INIT, 0, 0),
  m_initTag(initTag),
  m_secID(secID)
{
}

bdDTLSInit::~bdDTLSInit()
{
}

bdBool bdDTLSInit::serialize(void *data,
							 const bdUInt size,
							 const bdUInt offset,
							 bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdDTLSHeader::serialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_initTag);
	status = status && bdBytePacker::appendBuffer(data, size, newOffset, newOffset, m_secID.ab, sizeof(m_secID.ab));

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdBool bdDTLSInit::deserialize(const void* data,
							   const bdUInt size,
							   const bdUInt offset,
							   bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdDTLSHeader::deserialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_initTag);
	status = status && bdBytePacker::removeBuffer(data, size, newOffset, newOffset, m_secID.ab, sizeof(m_secID.ab));

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdUInt16 bdDTLSInit::getInitTag() const
{
	return m_initTag;
}

void bdDTLSInit::getSecID(bdSecurityID& secID) const
{
	secID = m_secID;
}


