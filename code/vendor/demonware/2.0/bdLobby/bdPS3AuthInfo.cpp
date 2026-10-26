// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3 UNIX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdPS3AuthInfo.h>

bdPS3AuthInfo::bdPS3AuthInfo()
: m_AVChatAvailability(false),
  m_ContentRatingEnabled(false),
  m_age(0)
{
	bdMemset(&m_region, BD_NULL, sizeof(m_region));
	bdMemset(&m_domain, BD_NULL, sizeof(m_domain));
}

bdPS3AuthInfo::~bdPS3AuthInfo()
{
}

bdBool bdPS3AuthInfo::deserialize(const void *buffer, bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool result = true;

	result = result && bdBytePacker::removeBuffer(buffer, bufferSize, offset, offset, reinterpret_cast<bdUByte8*>(&m_region), sizeof(m_region));
	result = result && bdBytePacker::removeBuffer(buffer, bufferSize, offset, offset, reinterpret_cast<bdUByte8*>(&m_domain), sizeof(m_domain));
	result = result && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_AVChatAvailability);
	result = result && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_ContentRatingEnabled);
	result = result && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_age);

	return result;
}

bdUInt bdPS3AuthInfo::serialize(void *buffer, bdUInt bufferSize)
{
	//
	bdUInt offset = 0;
	bdBool result = true;
	result = result && bdBytePacker::appendBuffer(buffer, bufferSize, offset, offset, reinterpret_cast<bdUByte8*>(&m_region), sizeof(m_region));
	result = result && bdBytePacker::appendBuffer(buffer, bufferSize, offset, offset, reinterpret_cast<bdUByte8*>(&m_domain), sizeof(m_domain));
	result = result && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_AVChatAvailability);
	result = result && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_ContentRatingEnabled);
	result = result && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_age);

	if(!result)
	{
		offset = 0;
	}
	return offset;
}
