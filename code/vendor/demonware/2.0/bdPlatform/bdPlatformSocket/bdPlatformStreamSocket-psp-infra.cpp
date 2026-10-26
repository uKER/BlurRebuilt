// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformSocket/bdPlatformStreamSocket.h>
#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#include <pspnet_error.h>


#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_resolver.h>
#include <pspnet/sys/socket.h>
#include <pspnet/sys/time.h>
#include <pspnet/sys/select.h>

bdInt BD_CALL bdPlatformStreamSocket::create(const bdBool blocking)
{

	// create the socket handle
	bdInt handle = sceNetInetSocket(SCE_NET_INET_AF_INET, SCE_NET_INET_SOCK_STREAM, 0);

	bdUInt timeout = 1000 * 1000;

	bdInt ret = sceNetInetSetsockopt(	handle,
										SCE_NET_INET_SOL_SOCKET,
										SCE_NET_INET_SO_SNDTIMEO,
										&timeout,
										sizeof(timeout));


	// Set socket blocking mode
	bdInt nonblocking = !blocking;
	const bdInt setOptRes = sceNetInetSetsockopt(handle,
										SCE_NET_INET_SOL_SOCKET,
								    	SCE_NET_INET_SO_NBIO,
								    	&nonblocking,
								    	sizeof(nonblocking));
	if (0 != setOptRes)
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
	sceNetInetListen(handle, 5);
	return BD_NET_SUCCESS;
}

bdSocketStatusCode BD_CALL bdPlatformStreamSocket::connect(const bdInt handle,
														   const bdInAddr addr,
														   const bdUInt16 port)
{
	struct SceNetInetSockaddrIn remoteAddr;
	// Fill in address for binding locally
	bdMemset(&remoteAddr, 0, sizeof(remoteAddr));
	remoteAddr.sin_family = SCE_NET_INET_AF_INET;
	remoteAddr.sin_addr.s_addr = addr.m_inAddri;

	// sin_port is in network byte order.
	remoteAddr.sin_port = sceNetHtons(port);


	const bdInt retval = ::sceNetInetConnect(handle, (SceNetInetSockaddr*)&remoteAddr, sizeof(remoteAddr));

	if(retval < 0)
	{
		const bdInt error = sceNetInetGetErrno();

		switch(error)
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

	return BD_NET_SUCCESS;
}

bdBool BD_CALL bdPlatformStreamSocket::accept(const bdInt handle,
											  bdInt &newHandle,
											  bdInAddr &addr,
											  bdUInt16 &port)
{
	struct SceNetInetSockaddrIn remoteAddr;
	// Fill in address for binding locally
	bdMemset(&remoteAddr, 0, sizeof(remoteAddr));
	remoteAddr.sin_family = SCE_NET_INET_AF_INET;
	remoteAddr.sin_addr.s_addr = addr.m_inAddri;

	// sin_port is in network byte order.
	remoteAddr.sin_port = sceNetHtons(port);

	SceNetInetSocklen_t remoteAddrLen = sizeof(struct SceNetInetSockaddrIn);

	bdInt sd = static_cast<bdInt>(::sceNetInetAccept(handle, reinterpret_cast<SceNetInetSockaddr*>(&remoteAddr), &remoteAddrLen));

	if(sd != BD_INVALID_SOCKET_HANDLE)
	{
		port = sceNetNtohs(remoteAddr.sin_port);
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


	bdInt sent = sceNetInetSend(	handle,
									data,
									length,
									0);

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
			case 128:
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

	bdInt received = sceNetInetRecv(handle,
								data,
								size,
								SCE_NET_INET_MSG_DONTWAIT);


	if(received < 0)
	{
		const bdInt lastErrno = sceNetInetGetErrno();
		switch(lastErrno)
		{
			case 11:	//EINPROGRESS:
			case 119:	//EAGAIN:
			{
				return BD_NET_WOULD_BLOCK;
			}
			case 128:
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
		ok = (sceNetInetClose(handle) == 0);
	}
	return ok;
}

void BD_CALL bdPlatformStreamSocket::unregisterThread()
{
}

bdBool BD_CALL bdPlatformStreamSocket::checkSocketException(const bdInt handle)
{
	struct SceNetInetTimeval zero;
	zero.tv_sec = 0;
	zero.tv_usec = 0;
	struct SceNetInetFdSet fdset;
	SceNetInetFD_ZERO(&fdset);
	SceNetInetFD_SET(handle, &fdset);
	const bdInt res = ::sceNetInetSelect(2, BD_NULL, BD_NULL, &fdset, &zero);

	if ((res == 1) && SceNetInetFD_ISSET(handle, &fdset))
	{
		return true;
	}
	return false;
}

bdBool BD_CALL bdPlatformStreamSocket::isWritable(const bdInt handle)
{
	struct SceNetInetTimeval zero;
	zero.tv_sec = 0;
	zero.tv_usec = 100;

	struct SceNetInetFdSet fdset;
	SceNetInetFD_ZERO(&fdset);
	SceNetInetFD_SET(handle, &fdset);

	const bdInt res = ::sceNetInetSelect(2, BD_NULL, &fdset, BD_NULL, &zero);

	// is writeable if the socket is in fdset and
	// there was no errors
	if (SceNetInetFD_ISSET(handle, &fdset))
	{
		return true;
	}
	return false;
}

