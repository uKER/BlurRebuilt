// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdConnectionStatistics.h>

bdConnectionStatistics::bdConnectionStatistics()
{
	reset();
}

void bdConnectionStatistics::reset()
{
	m_bytesSent = 0;
	m_bytesSentPerSecond = 0;
	m_lastBytesSent = 0;
	m_avgBytesSent = 0;

	m_bytesRecv = 0;

	m_avgPacketSentSize = 0;
	m_avgPacketRecvSize = 0;

	m_packetsSent = 0;
	m_packetsRecv = 0;

	m_maxPacketSizeSent = 0;
	m_maxPacketSizeRecv = 0;

	m_minPacketSizeSent = BD_UINT32_MAX;
	m_minPacketSizeRecv = BD_UINT32_MAX;

	m_maxRTT = 0;
	m_minRTT = 0;
	m_avgRTT = 0;
}

void bdConnectionStatistics::addBytesSent(const bdUInt bytes)
{
	m_bytesSent += bytes;
}

void bdConnectionStatistics::addBytesRecv(const bdUInt bytes)
{
	m_bytesRecv += bytes;
}

void bdConnectionStatistics::addPacketSizeSent(const bdUInt bytes)
{
	m_maxPacketSizeSent = BD_MAX(m_maxPacketSizeSent, bytes);
	m_minPacketSizeSent = BD_MIN(m_minPacketSizeSent, bytes);

	if(m_avgPacketSentSize == 0)
	{
		m_avgPacketSentSize = bytes;
	}
	else
	{
		m_avgPacketSentSize = (m_avgPacketSentSize + bytes) / 2;
	}
}

void bdConnectionStatistics::addPacketSizeRecv(const bdUInt bytes)
{
	m_maxPacketSizeRecv = BD_MAX(m_maxPacketSizeRecv, bytes);
	m_minPacketSizeRecv = BD_MIN(m_minPacketSizeRecv, bytes);

	if(m_avgPacketRecvSize == 0)
	{
		m_avgPacketRecvSize = bytes;
	}
	else
	{
		m_avgPacketRecvSize = (m_avgPacketRecvSize + bytes) / 2;
	}
}

void bdConnectionStatistics::addPacketsSent(const bdUInt numPackets)
{
	m_packetsSent += numPackets;
}

void bdConnectionStatistics::addPacketsRecv(const bdUInt numPackets)
{
	m_packetsRecv += numPackets;
}

void bdConnectionStatistics::setLastRTT(const bdFloat32 time)
{
	// first time
	if(m_maxRTT == 0 && m_minRTT == 0)
	{
		m_maxRTT = time;
		m_minRTT = time;
	}

	m_maxRTT = BD_MAX(m_maxRTT, time);	
	m_minRTT = BD_MIN(m_minRTT, time);
	// note the time passed in is already averaged (bdUnicastConnection m_smoothedRTT)
	m_avgRTT = time;
}

bdUInt bdConnectionStatistics::getBytesSent() const
{
	return m_bytesSent;
}

bdUInt bdConnectionStatistics::getBytesSentPerSecond() const
{
	return m_bytesSentPerSecond;
}

bdUInt bdConnectionStatistics::getBytesRecv() const
{
	return m_bytesRecv;
}

bdUInt bdConnectionStatistics::getPacketsSent() const
{
	return m_packetsSent;
}

bdUInt bdConnectionStatistics::getPacketsRecv() const
{
	return m_packetsRecv;
}

bdFloat32 bdConnectionStatistics::getAvgRTT() const
{
	return m_avgRTT;
}

void bdConnectionStatistics::update(const bdFloat32 time)
{
	if (time > 0 )
	{
		const bdUInt bytesSent = (m_bytesSent - m_lastBytesSent);
		m_lastBytesSent = m_bytesSent;
		m_avgBytesSent = (m_avgBytesSent + bytesSent) / 2;
		m_bytesSentPerSecond = static_cast<bdUInt>(m_avgBytesSent / time);
	}
}



