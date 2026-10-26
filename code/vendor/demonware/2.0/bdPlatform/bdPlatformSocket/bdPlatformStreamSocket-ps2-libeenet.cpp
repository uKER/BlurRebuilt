// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformSocket/bdPlatformStreamSocket.h>
#if defined BD_PLATFORM_PS2 && defined BD_PS2_USE_LIBEENET

#define sceEENetDisableSocketSymbolAliases
#include <eekernel.h>
#include <eenettypes.h>
#include <netinet/in.h>
#include <libeenet.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <eenetctl.h>


bdInt BD_CALL bdPlatformStreamSocket::create()
{
	// create the socket handle
	const bdInt handle = ::sceEENetSocket(SCE_EENET_AF_INET, SCE_EENET_SOCK_STREAM, SCE_EENET_IPPROTO_TCP);
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

	sceEENetSockaddrIn localAddr;
	// Fill in address for binding locally
	bdMemset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = SCE_EENET_AF_INET;
	localAddr.sin_addr.s_addr = addr.m_inAddri;
	localAddr.sin_port = sceEENetHtons(port);

	if ( ::sceEENetBind( handle, reinterpret_cast<sceEENetSockaddr*>(&localAddr), sizeof(localAddr) )  == BD_INVALID_SOCKET_HANDLE )
	{
		// couldn't bind
		return BD_NET_ERROR;
	}

	::sceEENetListen(handle, 5);

	return BD_NET_SUCCESS;
}

bdSocketStatusCode BD_CALL bdPlatformStreamSocket::connect(const bdInt handle,
														   const bdInAddr addr,
														   const bdUInt16 port)
{
	sceEENetSockaddrIn remoteAddr;
	// Fill in address for binding locally
	bdMemset(&remoteAddr, 0, sizeof(remoteAddr));
	remoteAddr.sin_family = SCE_EENET_AF_INET;
	remoteAddr.sin_addr.s_addr = addr.m_inAddri;
	remoteAddr.sin_port = sceEENetHtons(port);

	const bdInt retval = ::sceEENetConnect(handle, reinterpret_cast<sceEENetSockaddr*>(&remoteAddr), sizeof(remoteAddr));

	if(retval < 0)
	{
		switch(errno)
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
	sceEENetSockaddrIn remoteAddr;
	bdMemset(&remoteAddr, 0, sizeof(remoteAddr));

	sceEENetSocklen_t remoteAddrLen = sizeof(struct sceEENetSockaddrIn);

	bdInt sd = static_cast<bdInt>(::sceEENetAccept(handle, reinterpret_cast<sceEENetSockaddr*>(&remoteAddr), &remoteAddrLen));

	if(sd != BD_INVALID_SOCKET_HANDLE)
	{
		port = sceEENetNtohs(remoteAddr.sin_port);
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

	const bdInt sent = ::sceEENetSend(handle, data, length, 0);

	if(sent < 0)
	{
		switch(errno)
		{
			case ENOTCONN:
				if(checkSocketException(handle))
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
	// do non-blocking IO
	sceEENetPollfd fds[1];
	fds[0].fd = handle;
	fds[0].events = SCE_EENET_POLLIN;
	const bdInt dataAvail = sceEENetPoll (fds, 1, 0);
	bdInt received = 0;

	if (dataAvail)
	{
		received = ::sceEENetRecv(handle, data, size, 0);
	}
	else
	{
		return BD_NET_WOULD_BLOCK;
	}

	if(received < 0)
	{
		switch(sceEENetErrno)
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
	bdInt res = 0;
	if (handle != BD_INVALID_SOCKET_HANDLE)
	{
		::sceEENetSocketAbort(handle);
		res = ::sceEENetShutdown(handle, SCE_EENET_SHUT_RDWR);
	}
	return (res == 0);
}

void BD_CALL bdPlatformStreamSocket::unregisterThread()
{
}


bdBool BD_CALL bdPlatformStreamSocket::checkSocketException(const bdInt handle)
{
	timeval zero = {0, 0};
	sceEENetFdSet fdset;
	sceEENetFD_ZERO(&fdset);
	sceEENetFD_SET(handle, &fdset);
	const bdInt res = ::sceEENetSelect(1, BD_NULL, BD_NULL, &fdset, &zero);

	// there is a problem if an error occured or
	// the socket is in fdset
	const bdBool haveException = (res == -1) || sceEENetFD_ISSET(handle, &fdset);
	return haveException;
}

bdBool BD_CALL bdPlatformStreamSocket::isWritable(const bdInt handle)
{
	return true;
}

#endif // BD_PLATFORM_PS2 && BD_PS2_USE_LIBEENET
