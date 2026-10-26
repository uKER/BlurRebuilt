// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdPlatformStreamSocket.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netex/errno.h>
#include <netex/net.h>
#include <sys/poll.h>

#define BD_LOG_LEVEL "platform/streamsocket"

#define BD_NET_TCP_BACKLOG (5u)

bdInt BD_CALL bdPlatformStreamSocket::create(const bdBool blocking)
{
	// create the socket handle
	bdInt handle = ::socket(AF_INET, SOCK_STREAM, 0);

	if (handle < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "Failed to create socket res: %i sys_net_errno: %i" , handle, sys_net_errno);
		handle = BD_INVALID_SOCKET_HANDLE;
	}
	else
	{
		bdInt nonblocking = blocking ? 0 : 1;
		const bdInt result = setsockopt(handle, SOL_SOCKET, SO_NBIO, &nonblocking, sizeof(nonblocking));

		if (result < 0)
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to set socket option res: %i sys_net_errno: %i" , result, sys_net_errno);
			handle = BD_INVALID_SOCKET_HANDLE;
		}
	}

	return handle;
}

bdSocketStatusCode BD_CALL bdPlatformStreamSocket::bind(const bdInt handle,
														const bdInAddr addr,
														const bdUInt16 port)
{
	if(handle < 0)
	{
		return BD_NET_INVALID_HANDLE;
	}

	struct sockaddr_in localAddr;
	// Fill in address for binding locally
	bdMemset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = addr.m_inAddri;
	localAddr.sin_port = htons(port);

	if (::bind( handle, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0)
	{
		// couldn't bind
		::socketclose(handle);

		return BD_NET_ERROR;
	}

	if (::listen(handle, BD_NET_TCP_BACKLOG) < 0)
	{
		switch (sys_net_errno)
		{
			case SYS_NET_EBADF: // Bad handle
			case SYS_NET_EOPNOTSUPP: // Socket type cannot accept.
				// Should never happen.
				::socketclose(handle);
				return BD_NET_SUBSYTEM_ERROR;
				break;
			default:
				return BD_NET_ERROR;
		}
	}

	return BD_NET_SUCCESS;
}

bdSocketStatusCode BD_CALL bdPlatformStreamSocket::connect(const bdInt handle,
														   const bdInAddr addr,
														   const bdUInt16 port)
{
	struct sockaddr_in remoteAddr;
	// Fill in address for binding locally
	bdMemset(&remoteAddr, 0, sizeof(remoteAddr));
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_addr.s_addr = addr.m_inAddri;
	remoteAddr.sin_port = htons(port);

	const bdInt retval = ::connect(handle, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));

	if(retval < 0)
	{
		switch (sys_net_errno)
		{
			case SYS_NET_EINTR: // Block canceled by abort
				return BD_NET_BLOCKING_CALL_CANCELED;
				break;
			case SYS_NET_EBADF: // Bad handle
				return BD_NET_INVALID_HANDLE;
				break;
			case SYS_NET_EACCES: // Tried to broadcast when not allowed
				return BD_NET_ADDRESS_INVALID;
				break;
			case SYS_NET_EAFNOSUPPORT: // Address family not supported.
			case SYS_NET_EINVAL: // Invalid argument
				return BD_NET_ADDRESS_INVALID;
				break;
			case SYS_NET_EINPROGRESS: // Connection being established.
			case SYS_NET_EALREADY: // Connection is being established already.
				return BD_NET_WOULD_BLOCK;
				break;
			case SYS_NET_EADDRINUSE: // Address already in use.
				// TODO: this should never happen. Assert.
				return BD_NET_SUBSYTEM_ERROR;
				break;
			case SYS_NET_EISCONN: // Already connected
				// TODO: should return a better error.
				return BD_NET_ERROR;
				break;
			case SYS_NET_ETIMEDOUT: // Timed out
			case SYS_NET_ECONNREFUSED: // Connection refused.
				return BD_NET_CONNECTION_RESET;
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
	struct sockaddr_in remoteAddr;
	bdMemset(&remoteAddr, 0, sizeof(remoteAddr));

	bdInt remoteAddrLen = sizeof(remoteAddr);

	bdInt sd = ::accept(handle,
						reinterpret_cast<sockaddr*>(&remoteAddr),
						reinterpret_cast<socklen_t*>(&remoteAddrLen));

	if(sd > 0)
	{
		port = ntohs(remoteAddr.sin_port);
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
	if(handle < 0)
	{
		return BD_NET_INVALID_HANDLE;
	}

	bdInt sent = ::send(handle,
						reinterpret_cast<const bdNChar8*>(data),
						length,
						0);

	if(sent < 0)
	{
		switch (sys_net_errno)
		{
			case SYS_NET_EINTR: // Block canceled by abort
				return BD_NET_BLOCKING_CALL_CANCELED;
				break;
			case SYS_NET_EBADF: // Bad handle
				return BD_NET_INVALID_HANDLE;
				break;
			case SYS_NET_EAGAIN: // Timeout or TCP resend occurred.
				return BD_NET_WOULD_BLOCK;
			case SYS_NET_EPIPE: // Other side closed writing pipe.
			case SYS_NET_EINVAL: // Invalid argument
			// TODO: unreachable & reset should be ignored?
			case SYS_NET_ENETUNREACH: // Unreachable
			case SYS_NET_ECONNREFUSED: // connection reset
				return BD_NET_CONNECTION_RESET;
				break;
			case SYS_NET_ENOTCONN: // Not connected
				// Check exception.
				return BD_NET_NOT_CONNECTED;
				break;
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
	if(handle < 0)
	{
		return BD_NET_INVALID_HANDLE;
	}

	//struct sockaddr_in remoteAddr;
	//bdMemset(&remoteAddr, 0, sizeof(remoteAddr));

	bdInt received = ::recv(handle,
							reinterpret_cast<bdNChar8*>(data),
							size,
							0);

	if (received == 0)
	{
		return BD_NET_CONNECTION_CLOSED;
	}
	else if(received < 0)
	{
		switch(sys_net_errno)
		{
			case SYS_NET_EINTR: // Block canceled by abort
				return BD_NET_BLOCKING_CALL_CANCELED;
				break;
			case SYS_NET_EBADF: // Bad handle
				return BD_NET_INVALID_HANDLE;
				break;
			case SYS_NET_EWOULDBLOCK: // Would block
				return BD_NET_WOULD_BLOCK;
				break;
			case SYS_NET_EINVAL: // Invalid argument
			case SYS_NET_ECONNABORTED: // Connection aborted
				return BD_NET_CONNECTION_RESET;
				break;
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
	if (handle > 0)
	{
		ok = (::socketclose(handle) == 0);
	}
	return ok;
}

void BD_CALL bdPlatformStreamSocket::unregisterThread()
{
}

bdBool BD_CALL bdPlatformStreamSocket::isWritable(const bdInt handle)
{

	struct pollfd pfd[1];
	pfd[0].fd = handle;
	pfd[0].events = POLLWRNORM;

	const bdInt res = ::socketpoll(pfd, 1, 0);

	if ((res > 0) && (pfd[0].revents == pfd[0].events))
	{
		return true;
	}
	return false;
}

