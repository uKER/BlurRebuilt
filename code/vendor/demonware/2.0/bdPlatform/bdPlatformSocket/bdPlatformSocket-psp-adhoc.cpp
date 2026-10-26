// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>

#include <stdio.h>
#include <pspnet_error.h>

#include <pspnet_adhoc.h>

bdSocketStatusCode BD_CALL translateError(const bdInt error);

// These macros are also defined in bdCore, but because they are in a higher level
// we need them here
// A macro to get the 'i'th bit in d
#define BD_GET_BIT(d, i) (((d) >> (i)) & 0x1)
// A macro to set the 'i'th bit in d
#define BD_SET_BIT(d, i) ((d) |= (0x1 << (i)))
// A macro to clear the 'i'th bit in d
#define BD_CLEAR_BIT(d, i) ((d) &= ~(0x1 << (i)))

bdInt BD_CALL bdPlatformSocket::create(const bdBool blocking,
									   const bdBool /*broadcast*/)
{
	struct SceNetEtherAddr addr;
	bdMemset(&addr, 0, sizeof(addr));
	bdInt result = sceNetGetLocalEtherAddr(&addr);
	const bdUInt bufferSize = 18;
	bdNChar8 buffer[bufferSize];
	sceNetEtherNtostr(&addr, buffer);

	//TODO: translate the result to BD_??

	if (blocking)
	{
		// The highest order bit in the socket handle is used to store
		// whether blocking or non-blocking IO should be used
		BD_SET_BIT(handle, 31);
	}
	else
	{
		BD_CLEAR_BIT(handle, 31);
	}

	return result;
}

bdSocketStatusCode BD_CALL bdPlatformSocket::bind(bdInt &handle,
												  const bdInAddr addr,
												  const bdUInt16 port)
{
	bdSocketStatusCode result = BD_NET_ERROR;
	const bdInt socketHandle = sceNetAdhocPdpCreate((SceNetEtherAddr*)&addr, port, 2048, 0);
	if (socketHandle >= 0)
	{
		handle = socketHandle;
		result = BD_NET_SUCCESS;
	}

	return result;
}


bdInt BD_CALL bdPlatformSocket::sendTo(const bdInt handle,
									   const bdInAddr addr,
									   const bdUInt16 port,
									   const void *data,
									   const bdUInt length)

{
	if(handle == 0)
	{
		return BD_NET_NOT_BOUND;
	}

	//bdInAddr dest(BD_IN_ADDR_BROADCAST);
	const struct SceNetEtherAddr *destAddr = reinterpret_cast<const struct SceNetEtherAddr*>(&addr);
	const bdInt realHandle = handle & 0x7FFFFFFF;
	bdInt result = sceNetAdhocPdpSend(realHandle,
									  destAddr,
									  port,
									  const_cast<void*>(data),
									  length,
									  100,
									  0);

	if (result == 0)
	{
		// Update the number of bytes sent and ass UDP overhead to it
		m_totalBytesSent += length + BD_UDP_IP_OVERHEAD;
		result = length;
	}
	else
	{
		const bdSocketStatusCode error = translateError(result);
		result = error;
	}

	return result;
}

bdInt BD_CALL bdPlatformSocket::receiveFrom(const bdInt handle,
											bdInAddr &addr,
											bdUInt16 &port,
											void *data,
											const bdUInt size)

{
	bdInt result = -1;
	struct SceNetEtherAddr *fromAddr = reinterpret_cast<struct SceNetEtherAddr*>(&addr);

	bdInt realHandle = handle & 0x7FFFFFFF;
	// check the 'non-blocking'-bit
	if (BD_GET_BIT(handle,31))
	{
		// blocking IO
		result = sceNetAdhocPdpRecv(realHandle, fromAddr, &port, data, (int*)&size, 0, 0);
	}
	else
	{
		// non-blocking IO
		result = sceNetAdhocPdpRecv(realHandle, fromAddr, &port, data, (int*)&size, 0, SCE_NET_ADHOC_F_NONBLOCK);
	}

	if (result == 0)
	{
		// Update the number of bytes received and add UDP overhead to it
		m_totalBytesRecvd += size + BD_UDP_IP_OVERHEAD;
		result = size;
	}
	else
	{
		const bdSocketStatusCode error = translateError(result);
		result = error;
	}

	return result;
}

bdBool BD_CALL bdPlatformSocket::close(bdInt& handle)
{
	// TODO: why does this not check BD_INVALID_SOCKET_HANDLE
	const bdInt realHandle = handle & 0x7FFFFFFF;
	const bdInt closed = (sceNetAdhocPdpDelete(realHandle,0) == 0);
	handle = BD_INVALID_SOCKET_HANDLE;
	return closed;
}

void BD_CALL bdPlatformSocket::unregisterThread()
{
}

bdUInt BD_CALL bdPlatformSocket::getHostByName(const bdNChar8* const name,
											   bdInAddr *adresses,
											   bdUInt numAdresses,
											   const bdDNSHandle)
{
	//BD_WARN("bdPlatformSocket", "PSP DNS does not exist in ad hoc.");
	return 0;
}

void BD_CALL bdPlatformSocket::abortGetHostByName(bdDNSHandle)
{
	//BD_WARN("bdPlatformSocket", "PSP DNS does not exist in ad hoc.");
}

bdSocketStatusCode BD_CALL translateError(const bdInt error)
{
	bdSocketStatusCode translatedError;

	switch (error)
	{
		case SCE_ERROR_NET_ADHOC_WOULD_BLOCK:
		case SCE_ERROR_NET_ADHOC_TIMEOUT:
			translatedError = BD_NET_WOULD_BLOCK;
			break;

		case SCE_ERROR_NET_ADHOC_NOT_INITIALIZED:
			translatedError = BD_NET_SUBSYTEM_ERROR;
			break;

		case SCE_ERROR_NET_ADHOC_INVALID_SOCKET_ID:
			translatedError = BD_NET_INVALID_HANDLE;
			break;

		case SCE_ERROR_NET_ADHOC_NOT_ENOUGH_SPACE:
			translatedError = BD_NET_MSG_SIZE;
			break;

		default:
			translatedError = BD_NET_ERROR;
	}

	return translatedError;
}


