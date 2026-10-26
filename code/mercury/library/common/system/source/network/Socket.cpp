#include "SystemPch.h"

#include <system/network/platform.h>

#include <system/network/address.h>
#include <system/network/socket.h>
#include <core/utility/coredebug.h>

namespace Network
{

/// create a socket
Socket::Socket(int domain, int type, int protocol)
{
	m_socket = socket(domain, type, protocol);

	int opt = 1;
#ifdef SO_NOSIGPIPE
	setsockopt(so, SOL_SOCKET, SO_NOSIGPIPE, (char *)&opt, sizeof(opt));
#endif
}

/// Initialize using an exisiting SOCKET structure
Socket::Socket(SOCKET s)
{
	m_socket = s;
}

Socket::~Socket()
{
}

// connect to a specified address + port
bool Socket::Connect(const Address &ia, U32 port)
{
	struct sockaddr_in addr;

#if xPLATFORM_PC	// this code is supposed to speed up connection time but might time-out too early. disabled it for now
	// make socket non-blocking
	unsigned long flag = 1;
	ioctlsocket(m_socket, FIONBIO, &flag);
#endif

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
#ifndef PLATFORM_PS3	// [Markc] to get PS3 version compiling
	addr.sin_addr = ia.GetAddress();
#endif
	addr.sin_port = htons(port);

#if xPLATFORM_PC
	int r = ::connect(m_socket, (struct sockaddr *)&addr, (socklen_t)sizeof(addr));
	if (r == 0)
		return true;
	if (WSAGetLastError() == WSAEWOULDBLOCK)
	{
		if (IsInputPending(50))
		{
			return ::connect(m_socket, (struct sockaddr *)&addr, (socklen_t)sizeof(addr)) == 0;
		}
		return false;
	}
	else
	{
		return (r == 0);
	}

#else
	return (::connect(m_socket, (struct sockaddr *)&addr, (socklen_t)sizeof(addr)) == 0);
#endif
}


/// get associated SOCKET structure
SOCKET Socket::GetSocket()
{
	return m_socket;
}

/// wait for input on the socket
bool Socket::IsInputPending(U32 timeout)
{
	int status;

	struct timeval tv;
	struct timeval *tvp = &tv;
	fd_set grp;

	if (timeout == (U32)-1)
		tvp = NULL;
	else
	{
		tv.tv_usec = (timeout % 1000) * 1000;
		tv.tv_sec = timeout / 1000;
	}

	FD_ZERO(&grp);
	SOCKET sosave = m_socket;
	FD_SET(sosave, &grp);

#if defined( PLATFORM_PS3 )
	status = socketselect((int)m_socket + 1, &grp, NULL, NULL, tvp);
#else
	status = select((int)m_socket + 1, &grp, NULL, NULL, tvp);
#endif

	if(status < 1)
		return false;
	if(FD_ISSET(m_socket, &grp))
		return true;
	return false;
}

/// send data to socket
U32 Socket::Send(const void *buf, U64 len)
{
	return (U32) ::send(m_socket, (const char *)buf, (int) len, 0);
};

/// receive data from socket
U32 Socket::Receive(void *buf, U64 len)
{
	return (U32) ::recv(m_socket, (char *)buf, (int) len, 0);
}

/// peek
U32 Socket::Peek(void *buf, U64 len)
{
	return (U32) ::recv(m_socket, (char *)buf, (int) len, MSG_PEEK);
}


void Socket::Close()
{
#if defined( COMPILER_VC )
	shutdown(m_socket, SD_SEND);
	closesocket(m_socket);

#else
//	close(m_socket);
#endif
}

/// create an unconnected TCP socket
TCPSocket::TCPSocket() :
	Socket(AF_INET, SOCK_STREAM, 0)
{
}

TCPSocket::~TCPSocket()
{
	Close();
}

/// Initialize using an exisiting SOCKET structure
TCPSocket::TCPSocket(SOCKET s) :
	Socket(s)
{
}

/// bind to a port and listen for incoming connections (server)
TCPSocket::TCPSocket(U16 port, unsigned backlog) :
Socket(AF_INET, SOCK_STREAM, 0)
{
	LocalHost hostaddr;

	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
//	addr.sin_addr = hostaddr.GetAddress();
	addr.sin_port = htons(port);

#if defined( PLATFORM_PS3)
#define SOMAXCONN 5
#endif
	if (backlog == (unsigned)-1)
		backlog = SOMAXCONN;


	int opt = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, (socklen_t)sizeof(opt));

	bind(m_socket, (struct sockaddr *)&addr, sizeof(addr));

	listen(m_socket, backlog);
}

}
