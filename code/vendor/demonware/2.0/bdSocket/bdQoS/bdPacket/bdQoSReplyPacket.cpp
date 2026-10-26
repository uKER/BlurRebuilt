// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdQoS/bdPacket/bdQoSReplyPacket.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>
#include <bdSocket/bdQoS/bdQoS.h>

#define BD_LOG_LEVEL "bdSocket/qos"

bdUInt bdQoSReplyPacket::headerSize = 0;

bdQoSReplyPacket::bdQoSReplyPacket()
: m_type(BD_QOS_REPLY),
  m_id(0),
  m_timestamp(0),
  m_enabledMode(true),
  m_data(BD_NULL),
  m_dataSize(0),
  m_serializedSize(0)
{
}

bdQoSReplyPacket::bdQoSReplyPacket(bdUInt32 id, bdUInt64 timestamp)
: m_type(BD_QOS_REPLY),
  m_id(id),
  m_timestamp(timestamp),
  m_enabledMode(true),
  m_data(BD_NULL),
  m_dataSize(0),
  m_serializedSize(0)
{
}

bdQoSReplyPacket::~bdQoSReplyPacket()
{
	if (m_data)
	{
		bdDeallocate<bdUByte8>(m_data);
		m_data = BD_NULL;
		m_dataSize = 0;
	}
}

bdBool bdQoSReplyPacket::setData(const bdUByte8 *data, bdUInt dataSize)
{
	if (dataSize <= (BD_MAX_DATAGRAM_SIZE - getHeaderSize()))
	{
		if (m_data != BD_NULL)
		{
			bdDeallocate<bdUByte8>(m_data);
			m_data = BD_NULL;
			m_dataSize = 0;
			m_serializedSize = 0;
		}

		if (dataSize > 0)
		{
			m_data = bdAllocate<bdUByte8>(dataSize);

			if (m_data == BD_NULL)
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to allocate data buffer.");
				return false;
			}

			m_dataSize = dataSize;
			bdMemcpy(m_data, data, dataSize);
		}

		return true;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Attempted to set more data then available.");
		return false;
	}
}

void bdQoSReplyPacket::setId(bdUInt32 id)
{
	m_id = id;
}

void bdQoSReplyPacket::setTimestamp(bdUInt64 timestamp)
{
	m_timestamp = timestamp;
}

void bdQoSReplyPacket::setEnabled(bdBool mode)
{
	m_enabledMode = mode;
	m_serializedSize = 0;
}

bdUByte8 bdQoSReplyPacket::getType() const
{
	return m_type;
}

bdFloat32 bdQoSReplyPacket::getElapsedTime() const
{
	return bdPlatformTiming::getElapsedTime(m_timestamp, bdPlatformTiming::getHiResTimeStamp());
}

bdUInt32 bdQoSReplyPacket::getId() const
{
	return m_id;
}

bdUByte8* bdQoSReplyPacket::getData()
{
	return m_data;
}

bdUInt32 bdQoSReplyPacket::getDataSize() const
{
	return m_dataSize;
}

bdBool bdQoSReplyPacket::getEnabled() const
{
	return m_enabledMode;
}

bdBool bdQoSReplyPacket::serialize(void *data, const bdUInt size, const bdUInt offset, bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_type);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_id);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_timestamp);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_enabledMode);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_dataSize);
	if (m_dataSize > 0)
	{
		status = status && bdBytePacker::appendBuffer(data, size, newOffset, newOffset, m_data, m_dataSize);
	}

	if (!status)
	{
		newOffset = offset;
	}

	return status;
}

bdBool bdQoSReplyPacket::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_type);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_id);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_timestamp);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_enabledMode);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_dataSize);

	if (size - newOffset != m_dataSize)
	{
		BD_WARN(BD_LOG_LEVEL, "Received a packet with invalid dataSize!");
		status = false;
	}

	if (status && m_data != BD_NULL)
	{
		bdDeallocate<bdUByte8>(m_data);
		m_data = BD_NULL;
	}

	if (status && m_dataSize > 0)
	{
		m_data = bdAllocate<bdUByte8>(m_dataSize);
		if (m_data == BD_NULL)
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to allocate data buffer.");
			status = false;
		}
		status = status && bdBytePacker::removeBuffer(data, size, newOffset, newOffset, m_data, m_dataSize);
	}

	if (!status)
	{
		newOffset = offset;
	}

	return status;
}

bdUInt bdQoSReplyPacket::getHeaderSize()
{
	if (headerSize == 0)
	{
		const bdUInt oldDataSize = m_dataSize;
		m_dataSize = 0;
		BD_ASSERT(serialize(BD_NULL, BD_UINT16_MAX, 0, headerSize), "Size measurement failed.");
		m_dataSize = oldDataSize;
	}
	return headerSize;
}

bdUInt bdQoSReplyPacket::getSerializedSize()
{
	BD_ASSERT(bdQoSReplyPacket::serialize(BD_NULL, BD_UINT16_MAX, 0, m_serializedSize),
		"Size measurement failed.");
	return m_serializedSize;
}


