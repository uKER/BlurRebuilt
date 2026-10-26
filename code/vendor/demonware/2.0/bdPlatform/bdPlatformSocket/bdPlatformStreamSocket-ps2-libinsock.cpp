// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdPlatformStreamSocket.h>

#if (defined BD_PLATFORM_PS2) && (defined BD_PS2_USE_LIBINSOCK)

#define sceInsockDisableSocketSymbolAliases
#include <libinsck.h>
#include <errno.h>

bdInt BD_CALL bdPlatformStreamSocket::create(const bdBool blocking)
{
	// create the socket handle
	bdInt handle = ::sceInsockSocket(SCE_INSOCK_AF_INET, SCE_INSOCK_SOCK_STREAM, SCE_INSOCK_IPPROTO_TCP);

	int argp = !blocking;
	cosnt bdBool setOptRes = ::sceInsockSetsockopt(handle, SCE_INSOCK_SOL_SOCKET, SCE_INSOCK_SO_NBIO, &argp, sizeof(argp)) ;
	if(setOptRes != 0)
	{
		handle = BD_INVALID_SOCKET_HANDLE;
	}

	return handle;


}

bdSocketStatusCode BD_CALL bdPlatformStreamSocket::bind(const bdInt handle,
														const bdInAddr addr,
														const bdUInt16 port)
{
	if(handle == BD_INVALID_SOCKET_HANDLE)
	{
		return BD_NET_INVALID_HANDLE;
	}

	sceInsockSockaddrIn localAddr;
	// Fill in address for binding locally
	bdMemset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = SCE_INSOCK_AF_INET;
	localAddr.sin_addr.s_addr = addr.m_inAddri;
	localAddr.sin_port = sceInsockHtons(port);

	if ( ::sceInsockBind( handle, reinterpret_cast<sceInsockSockaddr*>(&localAddr), sizeof(localAddr) )  == BD_INVALID_SOCKET_HANDLE )
	{
		// couldn't bind
		return BD_NET_ERROR;
	}

	::sceInsockListen(handle, 5);

	return BD_NET_SUCCESS;
}

bdSocketStatusCode BD_CALL bdPlatformStreamSocket::connect(const bdInt handle,
														   const bdInAddr addr,
														   const bdUInt16 port)
{
	sceInsockSockaddrIn remoteAddr;
	// Fill in address for binding locally
	bdMemset(&remoteAddr, 0, sizeof(remoteAddr));
	remoteAddr.sin_family = SCE_INSOCK_AF_INET;
	remoteAddr.sin_addr.s_addr = addr.m_inAddri;
	remoteAddr.sin_port = sceInsockHtons(port);

	const bdInt retval = ::sceInsockConnect(handle, reinterpret_cast<sceInsockSockaddr*>(&remoteAddr), sizeof(remoteAddr));

	if(retval < 0)
	{
		switch(sceInsockErrno)
		{
			case EINPROGRESS:
			case EWOULDBLOCK:
				return BD_NET_WOULD_BLOCK;
				break;
			case ECONNRESET:
			case ETIMEDOUT:
			case ENETUNREACH:
			case ECONNABORTED:
			case EHOSTUNREACH:
			case ENETDOWN:
				return BD_NET_CONNECTION_RESET;
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
	return BD_NET_SUCCESS;
}

bdBool BD_CALL bdPlatformStreamSocket::accept(const bdInt handle,
											  bdInt &newHandle,
											  bdInAddr &addr,
											  bdUInt16 &port)
{
	sceInsockSockaddrIn remoteAddr;
	bdMemset(&remoteAddr, 0, sizeof(remoteAddr));

	sceInsockSocklen_t remoteAddrLen = sizeof(sceInsockSockaddrIn);

	const bdInt sd = static_cast<bdInt>(::sceInsockAccept(handle, reinterpret_cast<sceInsockSockaddr*>(&remoteAddr), &remoteAddrLen));

	if(sd != BD_INVALID_SOCKET_HANDLE)
	{
		port = sceInsockNtohs(remoteAddr.sin_port);
		addr.m_inAddri = remoteAddr.sin_addr.s_addr;

		newHandle = sd;

		return true;
	}

	return false;
}


bdInt BD_CALL bdPlatformStreamSocket::send(const bdInt handle,
										   const void *const data,
										   const bdUInt length)
{
	if(handle == BD_INVALID_SOCKET_HANDLE)
	{
		return BD_NET_INVALID_HANDLE;
	}

	const bdInt sent = ::sceInsockSend(handle, data, length, 0);

	if(sent < 0)
	{
		switch(sceInsockErrno)
		{
			case ECONNRESET:
			case ETIMEDOUT:
			case ENETUNREACH:
			case ECONNABORTED:
			case EHOSTUNREACH:
			case ENETDOWN:
				return BD_NET_CONNECTION_RESET;
				break;
			case EWOULDBLOCK:
			//case EAGAIN:
				return BD_NET_WOULD_BLOCK;
			break;
			case ENOTCONN:
				if (checkSocketException(handle))
				{
					return BD_NET_CONNECTION_RESET;
				}
				return BD_NET_NOT_CONNECTED;
			default:
				return BD_NET_ERROR;
		}
	}
	else
	{
		m_totalBytesSent += sent;
	}

	return sent;
}


bdInt BD_CALL bdPlatformStreamSocket::receive(const bdInt handle,
											  void *const data,
											  const bdUInt size)
{
	if(handle == BD_INVALID_SOCKET_HANDLE)
	{
		return BD_NET_INVALID_HANDLE;
	}


	bdInt received = 0;
	received = ::sceInsockRecv(handle, data, size, 0);

	if(received < 0)
	{
		switch(sceInsockErrno)
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
				return BD_NET_CONNECTION_RESET;
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
			case ENOTCONN:
				if (checkSocketException(handle))
				{
					return BD_NET_CONNECTION_RESET;
				}
				return BD_NET_NOT_CONNECTED;
			default:
				return BD_NET_ERROR;
		}
	}
	else
	{
		m_totalBytesRecvd += received;
	}

	return received;
}

bdBool BD_CALL bdPlatformStreamSocket::close(const bdInt handle)
{
	bdBool ok = true;
	if (handle != BD_INVALID_SOCKET_HANDLE)
	{
		::sceInsockAbort(handle, 0);
		bdInt res = ::sceInsockShutdown(handle, SCE_INSOCK_SHUT_RDWR);
		// 128 means "socket not connected", which can happen
		ok = ((res == 0) || (sceInsockErrno == 128));
	}
	return ok;
}

void BD_CALL bdPlatformStreamSocket::unregisterThread()
{
}

bdBool BD_CALL bdPlatformStreamSocket::checkSocketException(const bdInt handle)
{
	// pulled from: sce\ee\sample\inet\libinsck\echo_server_single

	sceInetPollFd_t poll;
	poll.cid = handle;
	poll.events = sceINET_POLLERR;
	poll.revents = 0;

	const bdInt res = sceInsockPoll(&poll, 1, 0);

	// there is a problem if an error (-1) or
	// a socket exception occured
	const bdBool haveException = (res == -1) || (poll.revents & sceINET_POLLERR);
	return haveException;
}

bdBool BD_CALL bdPlatformStreamSocket::isWritable(const bdInt handle)
{
	return true;
}

#endif (defined BD_PLATFORM_PS2) && (defined BD_PS2_USE_LIBINSOCK)
