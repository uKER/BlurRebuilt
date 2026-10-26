// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#define BD_LOG_LEVEL "platform/socket"


# include <sys/types.h>
# include <unistd.h>
# include <errno.h>

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>

# include <signal.h>
# include <sys/ioctl.h>   /* _SGIAPI FIO* functions	*/

# include <netinet/in.h>
# include <net/if.h>
#include <stdio.h>
#include <netdb.h>

#define BD_GETHOSTBYNAME_BUFFERSIZE 4096

bdInt BD_CALL bdPlatformSocket::create(const bdBool blocking,
									   const bdBool broadcast)
{
	// create the socket handle
	bdInt handle = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(handle < 0)
	{
		const bdInt error = errno;
		BD_ERR(BD_LOG_LEVEL, "Failed to create socket . Error: %i" ,error );
	}
	else if(true == broadcast)
	{
		BD_INFO(BD_LOG_LEVEL, "Setting broadcast for handle %i. " , handle );
		// try and set the broadcast flag on the socket.
		const bdInt sockBroadcast = 1;
		const bdInt sockoptResult = ::setsockopt(	handle,
													SOL_SOCKET,
													SO_BROADCAST,
													&sockBroadcast,
													sizeof(sockBroadcast));
		// Upon successful completion, setsockopt() returns 0. 
		// Otherwise, -1 is returned and errno is set to indicate the error.
		if(sockoptResult != 0)
		{
			const bdInt error = errno;
			BD_WARN(BD_LOG_LEVEL, "Failed to set socket option for broadcast. Error: %i" ,error );
		}
	}

	//FIXME: Should only allow non-blocking mode in single-threaded environment

	// Set socket blocking mode
	bdULong nonblocking = !blocking;
	const bdInt setOptRes = ::ioctl(handle, FIONBIO, &nonblocking) ;
	if(setOptRes != 0)
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
#ifdef htons
	localAddr.sin_port = htons(port);
#else   
	localAddr.sin_port = ::htons(port);
#endif	

	if ( ::bind( handle, (struct sockaddr *)&localAddr, sizeof(localAddr) )  == BD_INVALID_SOCKET_HANDLE )
	{
		const bdInt error = errno;

		switch (error)
		{
			case EACCES:
			case EADDRINUSE:
				return BD_NET_ADDRESS_IN_USE;
			default:
				::close(handle);
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
#ifdef htons
	recpt.sin_port = htons(port);
#else
	recpt.sin_port = ::htons(port);
#endif

	bdInt sent = ::sendto(handle,
							data,
							length,
							0,
							reinterpret_cast<struct sockaddr *>(&recpt),
							sizeof(recpt));

	if(sent < 0)
	{
		const bdInt error = errno;

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
		m_totalBytesSent += sent;
		m_totalPacketsSent ++;
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

	socklen_t remoteAddrLen = sizeof(struct sockaddr_in);
	bdInt received = ::recvfrom(handle,
								data,
								size,
								0,
								reinterpret_cast<struct sockaddr *>(&remoteAddr),
								&remoteAddrLen);



	if(received < 0)
	{
		const bdInt error = errno;


		switch(error)
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
#ifdef htons
					port = ntohs(remoteAddr.sin_port);
#else
					port = ::ntohs(remoteAddr.sin_port);
#endif
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
#ifdef htons
		port = ntohs(remoteAddr.sin_port);
#else
		port = ::ntohs(remoteAddr.sin_port);
#endif
		addr.m_inAddri = remoteAddr.sin_addr.s_addr;

		m_totalBytesRecvd += received;
		m_totalPacketsRecvd++;
	}

	return received;
}

bdBool BD_CALL bdPlatformSocket::close(bdInt& handle)
{
	bdBool closed = (handle == BD_INVALID_SOCKET_HANDLE);
	if (!closed)
	{
		closed = (::close(handle) == 0);
		handle = BD_INVALID_SOCKET_HANDLE;
	}
	return closed;
}

bdDNSHandle BD_CALL bdPlatformSocket::getDNSHandle(void*, bdInt)
{
	return 0;
}

bdUInt BD_CALL bdPlatformSocket::getHostByName(const bdNChar8* const name,
											   bdInAddr *adresses,
											   bdUInt numAdresses,
											   const bdDNSHandle)
{
	bdInt error;
	bdUInt i = 0;
	struct addrinfo hints, *addr, *addr0;

	// We only support IPV4 right now.
	bdMemset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	error = ::getaddrinfo(name, BD_NULL, &hints, &addr0);
	if (error)
	{
		BD_WARN(BD_LOG_LEVEL, "bdPlatformSocket::getHostByName failed: error=%i", error);
	}
	else
	{
		for (addr = addr0; i < numAdresses && addr != BD_NULL; addr = addr->ai_next, ++i)
		{
			adresses[i].inUn.m_iaddr = reinterpret_cast<struct sockaddr_in *>(addr->ai_addr)->sin_addr.s_addr;
		}
		::freeaddrinfo(addr0);
	}
	
	return i;
}

void BD_CALL bdPlatformSocket::abortGetHostByName(bdDNSHandle)
{
	//TODO: implement this
}
