// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>
#include <bdPlatform/bdPlatformSocket/bdPlatformStreamSocket.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

bdUInt64 bdPlatformStreamSocket::m_totalBytesSent = 0;

bdUInt64 bdPlatformStreamSocket::m_totalBytesRecvd = 0;

bdPlatformStreamSocket::bdPlatformStreamSocket()
{
}

bdUInt64 BD_CALL bdPlatformStreamSocket::getBytesReceived()
{
	return m_totalBytesRecvd;
}

bdUInt64 BD_CALL bdPlatformStreamSocket::getBytesSent()
{
	return m_totalBytesSent;
}



