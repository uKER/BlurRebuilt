// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netex/errno.h>
#include <netex/net.h>

#define BD_LOG_LEVEL "platform/socket"

bdInt BD_CALL bdPlatformSocket::create(const bdBool blocking,
									   const bdBool broadcast)
{

	// create the socket handle
	bdInt handle = ::socket(AF_INET, SOCK_DGRAM, 0);

	if (handle < 0)
	{
		handle = -1;
	}

	if(handle > 0 && broadcast)
	{
		// try and set the broadcast flag on the socket.
		const bdInt sockBroadcast = 1;
		::setsockopt(handle, SOL_SOCKET, SO_BROADCAST, &sockBroadcast, sizeof(sockBroadcast));

	}

	bdInt nonblocking = blocking ? 0 : 1;
	const bdInt result = setsockopt(handle, SOL_SOCKET, SO_NBIO, &nonblocking, sizeof(nonblocking));
	if (result < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "Failed to set blocking mode with result %i, sys_net_errno : %i" ,result,  sys_net_errno);
		handle = BD_INVALID_SOCKET_HANDLE;
	}

	return handle;
}

bdSocketStatusCode BD_CALL bdPlatformSocket::bind(bdInt &handle,
												  const bdInAddr addr,
												  const bdUInt16 port)
{
	if(handle < 0)
	{
		return BD_NET_INVALID_HANDLE;
	}

	sockaddr_in localAddr;
	// Fill in address for binding locally
	bdMemset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = addr.m_inAddri;
	localAddr.sin_port = htons(port);

	if ( ::bind( handle, (struct sockaddr *)&localAddr, sizeof(localAddr) ) < 0 )
	{
		// couldn't bind
		::socketclose(handle);

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
	if(handle < 0)
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
		switch(sys_net_errno)
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
			case SYS_NET_EAGAIN: // Timeout
				return BD_NET_WOULD_BLOCK;
				break;
			case SYS_NET_EINVAL: // Invalid argument
				return BD_NET_ADDRESS_INVALID;
				break;
			case SYS_NET_EPIPE: // Writing pipe was closed.
				return BD_NET_CONNECTION_CLOSED;
				break;
			case SYS_NET_ENETUNREACH: // Destination unreachable
			case SYS_NET_EHOSTDOWN:
				return BD_NET_CONNECTION_RESET;
				break;
			case SYS_NET_ENOTCONN: // Not connected.
				return BD_NET_NOT_CONNECTED;
				break;
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
	if(handle < 0)
	{
		return BD_NET_INVALID_HANDLE;
	}

	struct sockaddr_in remoteAddr;
	bdMemset(&remoteAddr, 0, sizeof(remoteAddr));

	socklen_t remoteAddrLen = sizeof(struct sockaddr_in);
	bdInt received = ::recvfrom(handle,
								reinterpret_cast<bdNChar8*>(data),
								size,
								0,
								reinterpret_cast<struct sockaddr *>(&remoteAddr),
								&remoteAddrLen);



	if(received < 0)
	{
		switch(sys_net_errno)
		{
			case SYS_NET_EINTR: // Block canceled by abort
				return BD_NET_BLOCKING_CALL_CANCELED;
				break;
			case SYS_NET_EBADF: // Bad handle
				return BD_NET_INVALID_HANDLE;
				break;
			case SYS_NET_EAGAIN: // Timeout
				return BD_NET_WOULD_BLOCK;
				break;
			case SYS_NET_EINVAL: // Invalid argument
				//// This indicates the socket has not yet been bound (either
				//// through bind or a call to sendTo) and we therefore can't
				//// receive.
				//// This happens as we have scheduled a packet to be sent (causing
				//// a bind) but this thread has run before the send thread.
				//// return a
				return BD_NET_ADDRESS_INVALID;
				break;
			case SYS_NET_ENOTCONN: // Not connected.
				return BD_NET_NOT_CONNECTED;
				break;
			default:
				return BD_NET_ERROR;
		}
	}
	else
	{
		port = ntohs(remoteAddr.sin_port);
		addr.m_inAddri = remoteAddr.sin_addr.s_addr;
	}

	return received;
}

bdBool BD_CALL bdPlatformSocket::close(bdInt& handle)
{
	// TODO: why does this not check BD_INVALID_SOCKET_HANDLE
	bdBool closed = (handle <= 0);
	if (!closed)
	{
		closed = (::socketclose(handle) == 0);
		handle = BD_INVALID_SOCKET_HANDLE;
	}
	return closed;
}

void BD_CALL bdPlatformSocket::unregisterThread()
{
	bdInt ret = 0;
	ret = sys_net_free_thread_context(0, SYS_NET_THREAD_SELF);
	if (ret < 0)
	{
		printf("sys_net_free_thread_context returned %i\n", ret);
	}

}

bdDNSHandle BD_CALL bdPlatformSocket::getDNSHandle(void *, bdInt)
{
	bdDNSHandle getHostnameHandle;
	sys_ppu_thread_get_id(&getHostnameHandle);
	return getHostnameHandle;
}


bdUInt BD_CALL bdPlatformSocket::getHostByName(const bdNChar8* const name,
											   bdInAddr *adresses,
											   bdUInt numAdresses,
											   bdDNSHandle )
{


	const hostent* remoteHost = ::gethostbyname(name);
	bdUInt numAddrs = 0;

	if (remoteHost)
	{
		bdUInt i;
		for (i=0; i < numAdresses && remoteHost->h_addr_list[i]; i++)
		{
			in_addr inAddr;
			bdMemcpy(&inAddr, remoteHost->h_addr_list[i], sizeof(inAddr));
			adresses[i].inUn.m_iaddr = inAddr.s_addr;
		}
		numAddrs = i;
	}

	return numAddrs;
}

void BD_CALL bdPlatformSocket::abortGetHostByName(bdDNSHandle handle)
{
	sys_net_abort_resolver(handle, 0);
}

