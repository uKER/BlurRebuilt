#include "SystemPch.h"
#include <System/Artery/BkSocket.h>
#include <System/Debug/Debug.h>

#include <sys/select.h>
#include <sys/time.h>

#ifdef PLATFORM_LINUX

#include <asm/socket.h>
#include <sys/socket.h>
#include <sys/termios.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>

#define socketclose close

#define SYS_NET_EISCONN		EISCONN
#define SYS_NET_EALREADY	EALREADY
#define SYS_NET_EINVAL		EINVAL
#define SYS_NET_ENOTCONN	ENOTCONN
#define SYS_NET_EWOULDBLOCK	EWOULDBLOCK

#define sys_net_errno errno

#endif

BkSocket::BkSocket() :
	_socket(-1)
{
}

BkSocket::~BkSocket()
{
	Close();
}

bool BkSocket::Connect(const BkSockAddr& sockAddr)
{
	Close();

	if ((_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return false;

	if (connect(_socket, sockAddr, sockAddr.Length()) != 0)
	{
		Close();
		return false;
	}

	const int kTrue = 1;

#if defined(PLATFORM_PS3)
	if (setsockopt(_socket, SOL_SOCKET, SO_NBIO, (char*)&kTrue, sizeof(kTrue)) != 0)
	{
		Close();
		return false;
	}
#else
	//u_long opts = 1;
	//ioctlsocket(_socket,FIONBIO,&opts);
#endif

	return true;
}

bool BkSocket::Listen(const BkSockAddr& sockAddr)
{
	Close();

	if ((_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return false;

	if (bind(_socket, sockAddr, sockAddr.Length()) != 0)
	{
		Close();
		return false;
	}

	if (listen(_socket, 5) != 0)
	{
		Close();
		return false;
	}

	const int kTrue = 1;

#if defined(PLATFORM_PS3)
	if (setsockopt(_socket, SOL_SOCKET, SO_NBIO, (char*)&kTrue, sizeof(kTrue)) != 0)
	{
		Close();
		return false;
	}
#else
	//u_long opts = 1;
	//ioctlsocket(_socket,FIONBIO,&opts);
#endif

	return true;
}

bool BkSocket::Accept(BkSocket& socket)
{
	if (_socket < 0)
		return false;

	int rawSocket = accept(_socket, 0, 0);

	if (rawSocket < 0)
	{
		/*switch(sys_net_errno)
		{
			case SYS_NET_EINTR:			DebugLn("*** EINTR!");			break;
			case SYS_NET_EBADF:			DebugLn("*** EBADF!");			break;
			case SYS_NET_EWOULDBLOCK:	DebugLn("*** EWOULDBLOCK!");	break;
			case SYS_NET_EFAULT:		DebugLn("*** EFAULT!");			break;
			case SYS_NET_EINVAL:		DebugLn("*** EINVAL!");			break;
			case SYS_NET_EOPNOTSUPP:	DebugLn("*** EOPNOTSUPP!");		break;
			case SYS_NET_ECONNABORTED:	DebugLn("*** EOPNOTSUPP!");		break;
			default:					DebugLn("*** ?????!");			break;
		}*/

		return false;
	}

	if (socket.IsOpen())
		socket.Close();

	socket._socket = rawSocket;

	const int kTrue = 1;

#if defined(PLATFORM_PS3)
	if (setsockopt(socket._socket, SOL_SOCKET, SO_NBIO, (char*)&kTrue, sizeof(kTrue)) != 0)
	{
		Close();
		return false;
	}
#else
	//u_long opts = 1;
	//ioctlsocket(_socket,FIONBIO,&opts);
#endif

	return true;
}

bool BkSocket::Broadcast(int port)
{
	Close();

	if ((_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		return false;

	const int kTrue = 1;

	if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char*)&kTrue, sizeof(kTrue)) != 0)
	{
		Close();
		return false;
	}

	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&kTrue, sizeof(kTrue)) != 0)
	{
		Close();
		return false;
	}

#if defined(PLATFORM_PS3)
	if (setsockopt(_socket, SOL_SOCKET, SO_NBIO, (char*)&kTrue, sizeof(kTrue)) != 0)
	{
		Close();
		return false;
	}
#else
	//u_long opts = 1;
	//ioctlsocket(_socket,FIONBIO,&opts);
#endif

	BkSockAddr sockAddr(INADDR_ANY, port);

	if (bind(_socket, sockAddr, sockAddr.Length()) != 0)
	{
		Close();
		return false;
	}

	return true;
}

void BkSocket::Close()
{
	if (_socket >= 0)
	{
		socketclose(_socket);
		_socket = -1;
	}
}

int BkSocket::TestConnection(BkSockAddr& sockAddr)
{
	if (_socket < 0)
		return -1;

	if (recv(_socket, 0, 0, 0) >= 0)
		return 1;

	switch(sys_net_errno)
	{
		case SYS_NET_EWOULDBLOCK:
		return 1;
	}

	fd_set fwrite;
	fd_set fexcept;

	FD_ZERO(&fwrite);
	FD_ZERO(&fexcept);

	FD_SET(_socket, &fwrite);
	FD_SET(_socket, &fexcept);

	timeval tv;

	tv.tv_sec	= 0;
	tv.tv_usec	= 0;

	int result = socketselect(_socket + 1, 0, &fwrite, &fexcept, &tv);

	if (result == 0)
		return 0;
	else if (result < 0)
		return -1;

	if (FD_ISSET(_socket, &fexcept))
		return -1;

	if (FD_ISSET(_socket, &fwrite))
		return 1;

	return 0;
}

int BkSocket::Receive(void* data, int maxLength)
{
	if (_socket < 0)
		return -1;

	int recvLength = recv(_socket, (char*)data, maxLength, 0);

	if (recvLength == 0)
		return -1;

	if (recvLength < 0)
	{
		switch(sys_net_errno)
		{
			case SYS_NET_EWOULDBLOCK:	return 0;
			default:					return -1;
		}
	}

	return recvLength;
}

int BkSocket::Send(const void* data, int length)
{
	if (_socket < 0)
		return -1;

	int sendLength = send(_socket, (const char*)data, length, 0);

	if (sendLength == 0)
		return -1;

	if (sendLength < 0)
	{
		switch(sys_net_errno)
		{
			case SYS_NET_EWOULDBLOCK:	return 0;
			default:					return -1;
		}
	}

	return sendLength;
}

int BkSocket::ReceiveFrom(void* data, int maxLength, BkSockAddr& address)
{
	if (_socket < 0)
		return -1;

	socklen_t addrLength = address.Length();
	int recvLength = recvfrom(_socket, (char*)data, maxLength, 0, address, &addrLength);

	if ((recvLength == 0) || ((int)addrLength != address.Length()))
		return -1;

	if (recvLength < 0)
	{
		switch(sys_net_errno)
		{
			case SYS_NET_EWOULDBLOCK:	return 0;
			default:					return -1;
		}
	}

	return recvLength;
}

int BkSocket::SendTo(const void* data, int length, const BkSockAddr& address)
{
	if (_socket < 0)
		return -1;

	int sendLength = sendto(_socket, (const char*)data, length, 0, address, address.Length());

	if (sendLength == 0)
		return -1;

	if (sendLength < 0)
	{
		switch(sys_net_errno)
		{
			case SYS_NET_EWOULDBLOCK:	return 0;
			default:					return -1;
		}
	}

	return sendLength;
}

void BkSocket::Attach(BkSocket& socket)
{
	if (&socket != this)
	{
		Close();

		_socket			= socket._socket;
		socket._socket	= -1;
	}
}

bool CrossConnect(BkSocket& a, BkSocket& b)
{
	a.Close();
	b.Close();

	BkSocket l;

	if (!l.Listen(BkSockAddr(INADDR_LOOPBACK, 0)))
		return false;

	BkSockAddr addr;
	socklen_t addrLen = addr.Length();

	if (getsockname(l.Handle(), addr, &addrLen) != 0)
		return false;

	if (!a.Connect(addr))
		return false;

	if (!l.Accept(b))
	{
		a.Close();
		return false;
	}

	return true;
}
