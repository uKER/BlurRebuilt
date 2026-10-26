// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#include <pspnet_error.h>


#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_resolver.h>
#include <pspnet/sys/socket.h>
#include <assert.h>

#define RESOLVER_TIMEOUT (5 * 1000 * 1000)
#define RESOLVER_RETRY 3



#include <stdio.h>


bdInt BD_CALL bdPlatformSocket::create(const bdBool blocking,
									   const bdBool broadcast)
{

	// create the socket handle
	bdInt handle = sceNetInetSocket(SCE_NET_INET_AF_INET, SCE_NET_INET_SOCK_DGRAM, 0);

	bdUInt timeout = 10 * 1000;

	bdInt ret = sceNetInetSetsockopt(	handle,
										SCE_NET_INET_SOL_SOCKET,
										SCE_NET_INET_SO_SNDTIMEO,
										&timeout,
										sizeof(timeout));
	assert(ret >= 0);

	if(broadcast)
	{
		bdUInt broadcastSet = 1;
		ret = sceNetInetSetsockopt(	handle,
									SCE_NET_INET_SOL_SOCKET,
									SCE_NET_INET_SO_ONESBCAST,
									&broadcastSet,
									sizeof(broadcastSet));
	}

	if(ret < 0)
	{
		/* Error handling */
	}

	// Set socket blocking mode
	bdInt nonblocking = !blocking;
	const bdInt result = sceNetInetSetsockopt(handle, 
										SCE_NET_INET_SOL_SOCKET,
										SCE_NET_INET_SO_NBIO, 
										&nonblocking, 
										sizeof(nonblocking));

	if (result != 0)
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

	struct SceNetInetSockaddrIn localAddr;
	// Fill in address for binding locally
	bdMemset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = SCE_NET_INET_AF_INET;
	localAddr.sin_addr.s_addr = 0;

	// sin_port is in network byte order.
	localAddr.sin_port = sceNetHtons(port);

	if ( sceNetInetBind( handle, (struct SceNetInetSockaddr *)&localAddr, sizeof(localAddr) )  == BD_INVALID_SOCKET_HANDLE )
	{
		// couldn't bind
		sceNetInetClose(handle);

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

	struct SceNetInetSockaddrIn recpt;

	bdMemset(&recpt, 0, sizeof(recpt));
	recpt.sin_family = SCE_NET_INET_AF_INET;
	recpt.sin_addr.s_addr = addr.m_inAddri;

	// sin_port is in network byte order.
	recpt.sin_port = sceNetHtons(port);

	bdInt sent = sceNetInetSendto(	handle,
									reinterpret_cast<const bdNChar8 *>(data),
									length,
									0,
									reinterpret_cast<struct SceNetInetSockaddr *>(&recpt),
									sizeof(recpt));

	if(sent < 0)
	{
		const bdInt lastErrno = sceNetInetGetErrno();
		switch(lastErrno)
		{
			case 11:	//EINPROGRESS:
			case 119:	//EAGAIN:
				{
					return BD_NET_WOULD_BLOCK;
				}
			default:
				return BD_NET_ERROR;
		}

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

	struct SceNetInetSockaddrIn remoteAddr;
	bdMemset(&remoteAddr, 0, sizeof(remoteAddr));

	SceNetInetSocklen_t remoteAddrLen = sizeof(struct SceNetInetSockaddrIn);
	bdInt received = sceNetInetRecvfrom(handle,
								reinterpret_cast<bdNChar8 *>(data),
								size,
								SCE_NET_INET_MSG_DONTWAIT ,
								reinterpret_cast<struct SceNetInetSockaddr *>(&remoteAddr),
								&remoteAddrLen);



	if(received < 0)
	{
		const bdInt lastErrno = sceNetInetGetErrno();
		switch(lastErrno)
		{
			// this will do for now...
			case 11:
			{
				return BD_NET_WOULD_BLOCK;
			}
			default:
				return BD_NET_ERROR;
		}
	}
	else
	{
		port = sceNetNtohs(remoteAddr.sin_port);
		addr.m_inAddri = remoteAddr.sin_addr.s_addr;
	}

	return received;
}

bdBool BD_CALL bdPlatformSocket::close(bdInt& handle)
{
	bdBool closed = (handle == BD_INVALID_SOCKET_HANDLE);
	if (!closed)
	{
		closed = (sceNetInetClose(handle) == 0);
		handle = BD_INVALID_SOCKET_HANDLE;
	}
	return closed;
}

void BD_CALL bdPlatformSocket::unregisterThread()
{
}

bdDNSHandle BD_CALL bdPlatformSocket::getDNSHandle(void *buf ,bdInt bufLen)
{
	bdDNSHandle handle = 0;
	bdInt ret = sceNetResolverCreate(&handle, buf, bufLen);
	if(ret < 0)
	{
		return 0;
	}
	return handle;
}

bdUInt BD_CALL bdPlatformSocket::getHostByName(const bdNChar8* const name,
											   bdInAddr *adresses,
											   bdUInt numAdresses,
											   const bdDNSHandle handle)
{

	bdInAddr addr;

	bdInt ret = sceNetResolverStartNtoA(handle, name, (SceNetInetInAddr*)&addr, RESOLVER_TIMEOUT, RESOLVER_RETRY);
	if(ret < 0)
	{		
		return 0;
	}
	sceNetResolverDelete(handle);
	adresses[0] = addr;
	// seems like PSP can only ever return one IP address
	return 1;
}

void BD_CALL bdPlatformSocket::abortGetHostByName(bdDNSHandle handle)
{
	sceNetResolverStop(handle);
}
