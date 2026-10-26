// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformSocket/bdPlatformStreamSocket.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#define BD_LOG_LEVEL "platform stream socket"

#if defined BD_PLATFORM_WIN32

#if !defined (_WIN32_WINNT)
#	define _WIN32_WINNT 0x400
#endif

# if !defined (_INC_WINDOWS)     // Already include windows.h ?
// Must define strict before including windows.h !
#  if !defined (STRICT)
#   define STRICT 1
#  endif // STRICT
#  if !defined (WIN32_LEAN_AND_MEAN)
#   define WIN32_LEAN_AND_MEAN
#  endif // !defined (WIN32_LEAN_AND_MEAN)
# endif // !defined (_INC_WINDOWS)

// winsock will include windows.h if it hasn't been included all ready

# pragma warning( push )
# pragma warning( disable: 4127) // warning in windows headers
#  include <winsock2.h>
# pragma warning( pop )

# if defined (_MSC_VER)
#  pragma comment(lib, "ws2_32.lib")
# endif // _MSC_VER

#else // xbox/xenon


#endif // defined BD_PLATFORM_WIN32

bdInt BD_CALL bdPlatformStreamSocket::create(const bdBool blocking)
{
	// create the socket handle
	bdInt handle = static_cast<bdInt>(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

	// Set socket blocking mode
	bdULong nonblocking = !blocking;
	const bdInt setOptRes = ::ioctlsocket(handle, FIONBIO, &nonblocking);
	if (setOptRes != 0)
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

	struct sockaddr_in localAddr;
	// Fill in address for binding locally
	bdMemset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = addr.m_inAddri;
	localAddr.sin_port = htons(port);

	if ( ::bind( handle, (struct sockaddr *)&localAddr, sizeof(localAddr) )  == BD_INVALID_SOCKET_HANDLE )
	{
		// couldn't bind
		::closesocket(handle);

		return BD_NET_ERROR;
	}

	::listen(handle, SOMAXCONN);

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
		const bdInt error = ::WSAGetLastError();

		switch(error)
		{
			case WSAEWOULDBLOCK:
				return BD_NET_WOULD_BLOCK;
			case WSAECONNRESET:
			case WSAETIMEDOUT:
			case WSAENETUNREACH:
			case WSAECONNABORTED:
			case WSAEHOSTUNREACH:
			case WSAENETDOWN:
				{
					return BD_NET_CONNECTION_RESET;
				}
			case WSAEMSGSIZE:
				return BD_NET_MSG_SIZE;
			case WSAEINTR:
				return BD_NET_BLOCKING_CALL_CANCELED;
			case WSAEINVAL:
				// This indicates the socket has not yet been bound (either
				// through bind or a call to sendTo) and we therefore can't
				// receive.
				// This happens as we have scheduled a packet to be sent (causing
				// a bind) but this thread has run before the send thread.
				// return a
				return BD_NET_NOT_BOUND;
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

	bdInt remoteAddrLen = sizeof(struct sockaddr_in);

	bdInt sd = static_cast<bdInt>(::accept(handle, reinterpret_cast<sockaddr*>(&remoteAddr), &remoteAddrLen));

	if(sd != BD_INVALID_SOCKET_HANDLE)
	{
		port = ntohs(remoteAddr.sin_port);
		addr.m_inAddri = remoteAddr.sin_addr.s_addr;

		newHandle = sd;

		return true;
	}

	return false;
}


bdInt BD_CALL bdPlatformStreamSocket::send(const bdInt handle,
										   const void *data,
										   const bdUInt length)
{
	if(handle == BD_INVALID_SOCKET_HANDLE)
	{
		return BD_NET_INVALID_HANDLE;
	}

#if defined BD_PLATFORM_XENON
	// On Xenon, attempt to send on a not-yet-connected socket results
	// in warnings on the console. Doing select() for writability
	// helps.
	bdInt sent;
	if (isWritable(handle))
	{
		sent = ::send(handle,
							reinterpret_cast<const bdNChar8 *>(data),
							length,
							0);
	}
	else
	{
		if (checkSocketException(handle))
		{
			return BD_NET_CONNECTION_RESET;
		}
		return BD_NET_NOT_CONNECTED;
	}
#else // Not Xenon
    bdInt sent = ::send(handle,
							reinterpret_cast<const bdNChar8 *>(data),
							length,
							0);
#endif // Not Xenon

	if(sent < 0)
	{
		const bdInt error = ::WSAGetLastError();
		switch(error)
		{
			case WSAEINTR:
				return BD_NET_BLOCKING_CALL_CANCELED;
			case WSAEWOULDBLOCK:
				return BD_NET_WOULD_BLOCK;
			case WSAEADDRNOTAVAIL:
			case WSAEINVAL:
			case WSAEDESTADDRREQ:
			case WSAEAFNOSUPPORT:
				return BD_NET_ADDRESS_INVALID;
			case WSAECONNRESET:
			case WSAETIMEDOUT:
			case WSAENETUNREACH:
			case WSAECONNABORTED:
			case WSAEHOSTUNREACH:
			case WSAENETDOWN:
			case WSAECONNREFUSED:
				return BD_NET_CONNECTION_RESET;
			case WSAEMSGSIZE:
				return BD_NET_MSG_SIZE;
			case WSAENOTCONN:
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
											  void *data,
											  const bdUInt size)
{
	if(handle == BD_INVALID_SOCKET_HANDLE)
	{
		return BD_NET_INVALID_HANDLE;
	}

#if defined BD_PLATFORM_XENON
	// On Xenon, attempt to receive on a not-yet-connected socket results
	// in warnings on the console. Doing select() for readability
	// helps.
	bdInt received;
	if (isWritable(handle))
	{
		received = ::recv(handle,
			reinterpret_cast<bdNChar8 *>(data),
			size,
			0);
	}
	else
	{
		if (checkSocketException(handle))
		{
			return BD_NET_CONNECTION_RESET;
		}
		return BD_NET_NOT_CONNECTED;
	}
#else // Not Xenon
	bdInt received = ::recv(handle,
					  reinterpret_cast<bdNChar8 *>(data),
					  size,
					  0);
#endif // Not Xenon
	//struct sockaddr_in remoteAddr;
	//bdMemset(&remoteAddr, 0, sizeof(remoteAddr));

	//bdInt received = ::recv(handle,
	//							reinterpret_cast<bdNChar8 *>(data),
	//							size,
	//							0);

	if(received < 0)
	{
		const bdInt error = ::WSAGetLastError();

		switch(error)
		{
			case WSAEWOULDBLOCK:
				return BD_NET_WOULD_BLOCK;
			case WSAECONNRESET:
			case WSAETIMEDOUT:
			case WSAENETUNREACH:
			case WSAECONNABORTED:
			case WSAEHOSTUNREACH:
			case WSAENETDOWN:
				{
					return BD_NET_CONNECTION_RESET;
				}
			case WSAENOTCONN:
				if (checkSocketException(handle))
				{
					return BD_NET_CONNECTION_RESET;
				}
				return BD_NET_NOT_CONNECTED;
			case WSAEMSGSIZE:
				return BD_NET_MSG_SIZE;
			case WSAEINTR:
				return BD_NET_BLOCKING_CALL_CANCELED;
			case WSAEINVAL:
				// This indicates the socket has not yet been bound (either
				// through bind or a call to sendTo) and we therefore can't
				// receive.
				// This happens as we have scheduled a packet to be sent (causing
				// a bind) but this thread has run before the send thread.
				// return a
				return BD_NET_NOT_BOUND;
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
		const int closeResult = ::closesocket(handle);
		if(0 == closeResult)
		{
			ok = true;
		}
		else if(SOCKET_ERROR == closeResult)
		{
			const int errorCode = WSAGetLastError();
			BD_WARN(BD_LOG_LEVEL, "Failed to close socket Error %i." , errorCode);
			ok = false;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to close socket. Unknown Error Code %i." , closeResult);
			ok = false;
		}
	}
	return ok;
}

void BD_CALL bdPlatformStreamSocket::unregisterThread()
{
}

bdBool BD_CALL bdPlatformStreamSocket::checkSocketException(const bdInt handle)
{
	const timeval zero = {0,0};
	fd_set fdset;
	FD_ZERO(&fdset);

#pragma warning( push )
#pragma warning( disable : 4127 ) // conditional expression is constant
	FD_SET(static_cast<SOCKET>(handle), &fdset);
#pragma warning( pop )

	// returns the total number of socket handles contained in the fd_set structures,
	// zero if the time limit expired, or SOCKET_ERROR
	const bdInt res = ::select(0, BD_NULL, BD_NULL, &fdset, &zero);

	// there is a problem if an error occurred or
	// the socket is in fdset
	const bdBool haveException = (res == SOCKET_ERROR) || FD_ISSET(handle, &fdset);

	return haveException;
}

bdBool BD_CALL bdPlatformStreamSocket::isWritable(const bdInt handle)
{
	const timeval zero = {0, 0};
	fd_set fdset;
	FD_ZERO(&fdset);

#pragma warning( push )
#pragma warning( disable : 4127 ) // conditional expression is constant
	FD_SET(static_cast<SOCKET>(handle), &fdset);
#pragma warning( pop )

	const bdInt res = ::select(0, BD_NULL, &fdset, BD_NULL, &zero);

	// is writeable if the socket is in fdset and
	// there was no errors
	const bdBool writeable = FD_ISSET(handle, &fdset) && (res != SOCKET_ERROR);
	return writeable;
}

#if defined(BD_PLATFORM_WIN32)
bdBool BD_CALL bdPlatformStreamSocket::getSocketAddr(const bdInt handle,
													 bdInAddr& socketAddr)
{
	struct sockaddr_in retrievedAddr;
	int length = sizeof(retrievedAddr);

	if (::getsockname(handle, (struct sockaddr *) &retrievedAddr, &length) == -1)
	{
		return false;
	}

	socketAddr = retrievedAddr.sin_addr.s_addr;
	return true;
}
#endif





