// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>

#if defined BD_PLATFORM_PS2 && defined BD_PS2_USE_LIBEENET

#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>

#include <eekernel.h>
#include <eenettypes.h>
#include <netinet/in.h>
#include <libeenet.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <eenetctl.h>

#undef bind


bdInt BD_CALL bdPlatformSocket::create(const bdBool blocking,
									   const bdBool broadcast)
{
	// create the socket handle
	bdInt handle = ::sceEENetSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(handle == -1)
	{
		handle = BD_INVALID_SOCKET_HANDLE;
	}
	else if(broadcast)
	{
		// try and set the broadcast flag on the socket.
		const bdInt sockBroadcast = 1;
		const bdInt sockoptResult = ::sceEENetSetsockopt(handle,
														 SOL_SOCKET,
														 SO_BROADCAST,
														 &sockBroadcast,
														 sizeof(sockBroadcast));

		if(sockoptResult != 0)
		{
			handle = BD_INVALID_SOCKET_HANDLE;
		}
	}

	int argp = !blocking;

	const bdBool setOptRes = ::setsockopt(handle, SOL_SOCKET, SO_NBIO, &argp, sizeof(argp)) ;
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
	localAddr.sin_port = ::htons(port);

	if ( ::sceEENetBind(handle, (sceEENetSockaddr*)&localAddr, sizeof(localAddr))  == BD_INVALID_SOCKET_HANDLE )
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

	bdInt sent = ::sceEENetSendto(handle,
									data,
									length,
									0,
									(sceEENetSockaddr*)(&recpt),
									sizeof(recpt));

	if(sent < 0)
	{
		const bdInt error = sceEENetErrno;
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
	bdInt received = 0;


	received = ::sceEENetRecvfrom(handle,
						   data,
						   size,
						   0,
						   (sceEENetSockaddr*)(&remoteAddr),
						   &remoteAddrLen);

	if(received < 0)
	{
		const bdInt error = sceEENetErrno;
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
		sceEENetSocketAbort(handle);
		closed = (sceEENetClose(handle) == 0);
		handle = BD_INVALID_SOCKET_HANDLE;
	}
	return closed;
}


void BD_CALL bdPlatformSocket::unregisterThread()
{
	sceEENetFreeThreadinfo(bdPlatformThread::getThreadHandle());
}

void BD_CALL bdPlatformSocket::unregisterThread(bdInt threadID)
{
	sceEENetFreeThreadinfo(threadID);
}

bdDNSHandle BD_CALL bdPlatformSocket::getDNSHandle(void *, bdInt)
{
	return GetThreadId();
}

bdUInt BD_CALL bdPlatformSocket::getHostByName(const bdNChar8* const name,
											   bdInAddr *adresses,
											   bdUInt numAdresses,
											   bdDNSHandle)
{
	bdUInt numAddrs = 0;

	sceEENetHostent* remoteHost;

	// If the user input is an alpha name for the host, use gethostbyname()
	// If not, get host by addr (assume IPv4)
	if (isalpha(name[0]))
	{
		//host address is a name
		remoteHost = sceEENetGethostbyname(name);
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
        // FIXME: Fix this to correctly return zero addresses when it fails. Use inet_aton
		adresses[0].inUn.m_iaddr = inet_addr(name);
		numAddrs = 1;
	}

	return numAddrs;
}

void BD_CALL bdPlatformSocket::abortGetHostByName(bdDNSHandle handle)
{
	sceEENetThreadAbort(handle);
}

#endif // BD_PLATFORM_PS2 && BD_PS2_USE_LIBEENET
