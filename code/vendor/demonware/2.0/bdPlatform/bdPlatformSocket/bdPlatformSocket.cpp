// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>

bdUInt64 bdPlatformSocket::m_totalBytesSent = 0;
bdUInt64 bdPlatformSocket::m_totalBytesRecvd = 0;
bdUInt64 bdPlatformSocket::m_totalPacketsSent = 0;
bdUInt64 bdPlatformSocket::m_totalPacketsRecvd = 0;

bdUInt64 BD_CALL bdPlatformSocket::getBytesSent()
{
	return m_totalBytesSent;
}

bdUInt64 BD_CALL bdPlatformSocket::getBytesReceived()
{
	return m_totalBytesRecvd;
}

bdUInt64 BD_CALL bdPlatformSocket::getPacketsSent()
{
	return m_totalPacketsSent;
}

bdUInt64 BD_CALL bdPlatformSocket::getPacketsRecvd()
{
	return m_totalPacketsRecvd;
}

bdPlatformSocket::bdPlatformSocket()
{
}

