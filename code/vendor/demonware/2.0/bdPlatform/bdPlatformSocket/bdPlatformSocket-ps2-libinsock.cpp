// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>

#if defined BD_PLATFORM_PS2 && defined BD_PS2_USE_LIBINSOCK

#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>

#  include <libinsck.h>
#  include <errno.h>
#  include <assert.h>

#  undef bind

// Note: non-blocking IO is not supported by the socket library. Although
// the equivalent sceInsockSetsockopt()-function exists, it doesn't have any effect.
// Non-blocking IO is achieved by polling the network stack for available data.
// The highest order-bit in the socket handle is used to indicate whether
// blocking or non-blocking IO should be used.


// These macros are also defined in bdCore, but because they are in a higher level
// we need them here
// A macro to get the 'i'th bit in d
#define BD_GET_BIT(d, i) (((d) >> (i)) & 0x1)
// A macro to set the 'i'th bit in d
#define BD_SET_BIT(d, i) ((d) |= (0x1 << (i)))
// A macro to clear the 'i'th bit in d
#define BD_CLEAR_BIT(d, i) ((d) &= ~(0x1 << (i)))

bdInt BD_CALL bdPlatformSocket::create(const bdBool blocking,
									   const bdBool broadcast)
{
	// create the socket handle
	bdInt handle = ::sceInsockSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

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

	return handle;
}

bdSocketStatusCode BD_CALL bdPlatformSocket::bind(bdInt &handle,
												  const bdInAddr addr,
												  const bdUInt16 port)
{
	if(handle == BD_INVALID_SOCKET_HANDLE)
	{
		return BD_NET_INVALID_HANDLE;
	}

	struct sockaddr_in localAddr;
	// Fill in address for binding locally
	bdMemset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = addr.m_inAddri;
	localAddr.sin_port = ::htons(port);

	bdInt realHandle = handle & 0x7FFFFFFF;
	if ( ::sceInsockBind(realHandle, (sceInsockSockaddr*)&localAddr, sizeof(localAddr))  == BD_INVALID_SOCKET_HANDLE )
	{
		// couldn't bind
		return BD_NET_ERROR;
	}

	return BD_NET_SUCCESS;
}

bdInt BD_CALL bdPlatformSocket::sendTo(const bdInt handle,
									   const bdInAddr addr,
									   const bdUInt16 port,
									   const void *data,
									   const bdUInt length)
{
	if(handle == BD_INVALID_SOCKET_HANDLE)
	{
		return BD_NET_INVALID_HANDLE;
	}

	struct sockaddr_in recpt;

	bdMemset(&recpt, 0, sizeof(recpt));
	recpt.sin_family = AF_INET;
	recpt.sin_addr.s_addr = addr.m_inAddri;
	recpt.sin_port = ::htons(port);

	bdInt realHandle = handle & 0x7FFFFFFF;
	bdInt sent = ::sceInsockSendto(realHandle,
									data,
									length,
									0,
									(sceInsockSockaddr*)(&recpt),
									sizeof(recpt));

	if(sent < 0)
	{
		const bdInt error = sceInsockErrno;
		switch(error)
		{
			case EWOULDBLOCK:
				return BD_NET_WOULD_BLOCK;
			case EINVAL:
				return BD_NET_ADDRESS_INVALID;
			case ECONNRESET:
			case ETIMEDOUT:
			case ENETUNREACH:
			case ECONNABORTED:
			case EHOSTUNREACH:
			case ENETDOWN:
				return BD_NET_CONNECTION_RESET;
			case EMSGSIZE:
				return BD_NET_MSG_SIZE;
			default:
				return BD_NET_ERROR;
		}
	}
	else
	{
		m_totalBytesSent += sent + BD_UDP_IP_OVERHEAD;
	}

	return sent;
}

bdInt BD_CALL bdPlatformSocket::receiveFrom(const bdInt handle,
											bdInAddr &addr,
											bdUInt16 &port,
											void *data,
											const bdUInt size)
{
	if(handle == BD_INVALID_SOCKET_HANDLE)
	{
		return BD_NET_INVALID_HANDLE;
	}

	struct sockaddr_in remoteAddr;
	bdMemset(&remoteAddr, 0, sizeof(remoteAddr));
	bdUInt remoteAddrLen = sizeof(struct sockaddr_in);

	bdInt realHandle = handle & 0x7FFFFFFF;
	bdInt received;

	// check the 'non-blocking'-bit
	if (BD_GET_BIT(handle,31))
	{
		received = ::sceInsockRecvfrom(realHandle,
									   data,
									   size,
									   0,
									   (sceInsockSockaddr*)(&remoteAddr),
									   &remoteAddrLen);
	}
	else
	{
		// do non-blocking IO
		sceInetPollFd_t fds[1];
		fds[0].cid = handle;
		fds[0].events = sceINET_POLLIN;
		int dataAvail = sceInsockPoll (fds, 1, 0);

		if (dataAvail)
		{
			received = ::sceInsockRecvfrom(realHandle,
											data,
											size,
											0,
											(sceInsockSockaddr*)(&remoteAddr),
											&remoteAddrLen);
		}
		else
		{
			return BD_NET_WOULD_BLOCK;
		}
	}

	if(received < 0)
	{
		const bdInt errorCode  = sceInsockErrno;

		switch(errorCode)
		{
			case EWOULDBLOCK:
				return BD_NET_WOULD_BLOCK;
				break;
			case ECONNRESET:
			case ETIMEDOUT:
			case ENETUNREACH:
			case ECONNABORTED:
			case EHOSTUNREACH:
			case ENETDOWN:
				{
					port = ::ntohs(remoteAddr.sin_port);
					addr.m_inAddri = remoteAddr.sin_addr.s_addr;
					return BD_NET_CONNECTION_RESET;
				}
				break;
			case EMSGSIZE:
				return BD_NET_MSG_SIZE;
				break;
			case EINTR:
				return BD_NET_BLOCKING_CALL_CANCELED;
				break;
			case EINVAL:
				// This indicates the socket has not yet been bound (either
				// through bind or a call to sendTo) and we therefore can't
				// receive.
				// This happens as we have scheduled a packet to be sent (causing
				// a bind) but this thread has run before the send thread.
				// return a
				return BD_NET_NOT_BOUND;
				break;
			default:
				return BD_NET_ERROR;
		}
	}
	else
	{
		port = ::ntohs(remoteAddr.sin_port);
		addr.m_inAddri = remoteAddr.sin_addr.s_addr;
		m_totalBytesRecvd += received + BD_UDP_IP_OVERHEAD;
	}

	return received;
}

bdBool BD_CALL bdPlatformSocket::close(bdInt& handle)
{
	bdBool closed = (handle == BD_INVALID_SOCKET_HANDLE);
	if (!closed)
	{
		const bdInt realHandle = handle & 0x7FFFFFFF;
		::sceInsockAbort(realHandle, 0);
		closed = (::sceInsockShutdown(realHandle, SHUT_RDWR) == 0);
		handle = BD_INVALID_SOCKET_HANDLE;
	}
	return closed;
}


void BD_CALL bdPlatformSocket::unregisterThread()
{
	bdThreadHandle threadID = bdPlatformThread::getThreadHandle();
	sceInsockTerminate(threadID);
}

void BD_CALL bdPlatformSocket::unregisterThread(bdInt threadID)
{
	sceInsockTerminate(threadID);
}

bdDNSHandle BD_CALL bdPlatformSocket::getDNSHandle(void *, bdInt)
{
	// libinsock does not allow multiple name lookups
	return 0;
}

bdUInt BD_CALL bdPlatformSocket::getHostByName(const bdNChar8* const name,
											   bdInAddr *adresses,
											   bdUInt numAdresses,
											   bdDNSHandle)
{
	bdUInt numAddrs = 0;

	sceInsockHostent* remoteHost;

	// If the user input is an alpha name for the host, use gethostbyname()
	// If not, get host by addr (assume IPv4)
	if (isalpha(name[0]))
	{
		//host address is a name
		remoteHost = sceInsockGethostbyname(name);

		if(remoteHost)
		{
			bdUInt index = 0;
			while (index < numAdresses && remoteHost->h_addr_list[index])
			{
				struct in_addr inAddr;
				bdMemcpy(&inAddr, remoteHost->h_addr_list[index], sizeof(struct in_addr));
				adresses[index].inUn.m_iaddr = inAddr.s_addr;
				index ++;
			}
			numAddrs = index;
		}
	}
	else
	{
		adresses[0].inUn.m_iaddr = inet_addr(name);
		numAddrs = 1;
	}

	return numAddrs;
}

void BD_CALL bdPlatformSocket::abortGetHostByName(bdDNSHandle)
{
	sceInsockAbortGethostbyname(BD_NULL);
}

#endif // BD_PLATFORM_PS2 && BD_PS2_USE_LIBINSOCK
