// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdQoS/bdPacket/bdQoSRequestPacket.h>
#include <bdSocket/bdQoS/bdQoS.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

bdQoSRequestPacket::bdQoSRequestPacket(bdUInt32 secid, bdUInt32 id)
: m_type(BD_QOS_REQUEST),
  m_id(id),
  m_secid(secid)
    
{
	m_timestamp = bdPlatformTiming::getHiResTimeStamp();
}

bdQoSRequestPacket::bdQoSRequestPacket() :
m_type(0),
m_timestamp(0),
m_id(0),
m_secid(0)
{
}

bdUByte8 bdQoSRequestPacket::getType() const
{
	return m_type;
}

bdUInt64 bdQoSRequestPacket::getTimestamp() const
{
	return m_timestamp;
}

bdUInt32 bdQoSRequestPacket::getId() const
{
	return m_id;
}

bdUInt32 bdQoSRequestPacket::getSecId() const
{
	return m_secid;
}

bdBool bdQoSRequestPacket::serialize(void *data,
									 const bdUInt size,
									 const bdUInt offset,
									 bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_type);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_timestamp);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_id);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_secid);

	if (!status)
	{
		newOffset = offset;
	}

	return status;
}

bdBool bdQoSRequestPacket::deserialize(const void* data,
									   const bdUInt size,
									   const bdUInt offset,
									   bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_type);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_timestamp);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_id);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_secid);

	if (!status)
	{
		newOffset = offset;
	}

	return status;
}


