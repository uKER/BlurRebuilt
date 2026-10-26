// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#define BD_LOG_LEVEL "platform/socket"

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


bdInt BD_CALL bdPlatformSocket::create(const bdBool blocking,
									   const bdBool broadcast)
{

	// create the socket handle
#if defined BD_PLATFORM_WIN32
	bdInt handle = static_cast<bdInt>(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));


#else
	bdInt handle;

	if(broadcast)
	{
		 handle = static_cast<bdInt>(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
	}
	else
	{
		handle = static_cast<bdInt>(::socket(AF_INET, SOCK_DGRAM, IPPROTO_VDP));
	}
#endif

	if(broadcast)
	{
		// try and set the broadcast flag on the socket.
		const bdUInt32 sockBroadcast = 1;
		const bdInt sockoptResult = ::setsockopt(
													handle,
													SOL_SOCKET,
													SO_BROADCAST,
													reinterpret_cast<const bdNChar8*>(&sockBroadcast),
													sizeof(sockBroadcast)
												);
		if (sockoptResult != 0)
		{
			return BD_INVALID_SOCKET_HANDLE;
		}
	}

	//FIXME:  Only allow non-blocking mode in single-threaded environment!

	// Set socket blocking mode
	bdULong nonblocking = !blocking;
	const bdInt setOptRes = ::ioctlsocket(handle, FIONBIO, &nonblocking);
	if (setOptRes != 0)
	{
		handle = BD_INVALID_SOCKET_HANDLE;
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
	localAddr.sin_port = htons(port);

	if ( ::bind( handle, (struct sockaddr *)&localAddr, sizeof(localAddr) )  == BD_INVALID_SOCKET_HANDLE )
	{
		const bdInt error = ::WSAGetLastError();

		// couldn't bind
		::closesocket(handle);

		switch(error)
		{
			case WSAEACCES:
			case WSAEADDRINUSE:
			case WSAEADDRNOTAVAIL:
				return BD_NET_ADDRESS_IN_USE;
			default:
				return BD_NET_ERROR;
		}
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
	recpt.sin_port = htons(port);

	bdInt sent = ::sendto(handle,
							reinterpret_cast<const bdNChar8 *>(data),
							length,
							0,
							reinterpret_cast<struct sockaddr *>(&recpt),
							sizeof(recpt));

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
			{
				BD_INFO(BD_LOG_LEVEL, "send: Received connection reset : %i" ,error);
				return BD_NET_CONNECTION_RESET;
			}
			case WSAEMSGSIZE:
				return BD_NET_MSG_SIZE;
			default:
				return BD_NET_ERROR;
		}
	}
	else
	{
		m_totalBytesSent += sent + BD_UDP_IP_OVERHEAD;
		m_totalPacketsSent++;
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

	bdInt remoteAddrLen = sizeof(struct sockaddr_in);
	bdInt received = ::recvfrom(handle,
								reinterpret_cast<bdNChar8 *>(data),
								size,
								0,
								reinterpret_cast<struct sockaddr *>(&remoteAddr),
								&remoteAddrLen);



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
					BD_INFO(BD_LOG_LEVEL, "recv: Received connection reset : %i" ,error);
					port = ntohs(remoteAddr.sin_port);
					addr.m_inAddri = remoteAddr.sin_addr.s_addr;
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
	else
	{
		port = ntohs(remoteAddr.sin_port);
		addr.m_inAddri = remoteAddr.sin_addr.s_addr;

		m_totalBytesRecvd += received + BD_UDP_IP_OVERHEAD;
		m_totalPacketsRecvd++;
	}

	return received;
}

bdBool BD_CALL bdPlatformSocket::close(bdInt& handle)
{
	bdBool closed = (handle == BD_INVALID_SOCKET_HANDLE);
	if (!closed)
	{
		const int closeResult = ::closesocket(handle);
		closed = (closeResult == 0);
		handle = BD_INVALID_SOCKET_HANDLE;
	}
	return closed;
}

void BD_CALL bdPlatformSocket::unregisterThread()
{
}

bdUInt BD_CALL bdPlatformSocket::getHostByName(const bdNChar8* const name,
											   bdInAddr *adresses,
											   bdUInt numAdresses,
											   bdDNSHandle)
{
	bdUInt numAddrs = 0;

// there is no way to look up hostnames on xbox.
#if defined BD_PLATFORM_WIN32

	hostent* remoteHost = BD_NULL;

	// If the user input is an alpha name for the host, use gethostbyname()
	// If not, get host by addr (assume IPv4)
	if (isalpha(name[0]))
	{
		//host address is a name
		remoteHost = gethostbyname(name);
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
		if (adresses[0].inUn.m_iaddr == INADDR_NONE)
		{
			// FIXME: this is not strictly right :)
			static const bdNChar8 *magicCase = "255.255.255.255";
			if (bdStrcmp(magicCase, name) == 0)
			{
				numAddrs = 1;
			}
			else
			{
				numAddrs = 0;
			}
		}
		else
		{
			numAddrs = 1;
		}
	}
#else
	// if on xbox/xenon the following lines avoid "unreferenced formal parameter" warning
	name;
	adresses;
	numAdresses;
#endif // defined BD_PLATFORM_WIN32

	return numAddrs;
}


void BD_CALL bdPlatformSocket::abortGetHostByName(bdDNSHandle)
{

}

#ifdef BD_PLATFORM_WIN32
void BD_CALL bdPlatformSocket::setExclusiveAccess(bdInt handle)
{

	// this call is not available on win9x/Me
	// http://blogs.msdn.com/wndp/archive/2005/08/03/Anthony_Jones.aspx

	// This call does not work with non-admin accounts on XP pre SP2
	// or on win2000 pre SP4
	// http://support.microsoft.com/?id=870562

	const bdUInt32 sockExclusiveAddrUse = 1;
	::setsockopt(	handle,
					SOL_SOCKET,
					SO_EXCLUSIVEADDRUSE,
					reinterpret_cast<const bdNChar8*>(&sockExclusiveAddrUse),
					sizeof(sockExclusiveAddrUse));
	
}
#endif //#ifdef BD_PLATFORM_WIN32
