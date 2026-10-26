#include "SystemPch.h"

#if defined(PLATFORM_PS3) || defined(PLATFORM_LINUX)

#include "BkSocket.cpp"

#else

#include <System/Artery/WsSocket.h>

WsSocket::WsSocket() :
	_socket(INVALID_SOCKET),
	_event(WSA_INVALID_EVENT)
{
}

WsSocket::~WsSocket()
{
	Close();
}

bool WsSocket::Connect(const WsSockAddr& sockAddr)
{
	Close();

	if ((_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		return false;

	if ((_event = CreateEvent(0, FALSE, FALSE, 0)) == WSA_INVALID_EVENT)
	{
		Close();
		return false;
	}

	if (WSAEventSelect(_socket, _event, FD_CONNECT | FD_WRITE | FD_READ | FD_CLOSE) == SOCKET_ERROR)
	{
		Close();
		return false;
	}

	if (connect(_socket, sockAddr, sockAddr.Length()) == SOCKET_ERROR)
	{
		switch(WSAGetLastError())
		{
			case WSAEWOULDBLOCK:
			return true;

			default:
				Close();
			return false;
		}
	}

	return true;
}

bool WsSocket::Listen(const WsSockAddr& sockAddr)
{
	Close();

	if ((_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		return false;

	const BOOL kTrue = TRUE;

	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&kTrue, sizeof(kTrue)) == SOCKET_ERROR)
	{
		Close();
		return false;
	}

	if (bind(_socket, sockAddr, sockAddr.Length()) == SOCKET_ERROR)
	{
		Close();
		return false;
	}

	if (listen(_socket, 5) == SOCKET_ERROR)
	{
		Close();
		return false;
	}

	if ((_event = CreateEvent(0, FALSE, FALSE, 0)) == WSA_INVALID_EVENT)
	{
		Close();
		return false;
	}

	if (WSAEventSelect(_socket, _event, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
	{
		Close();
		return false;
	}

	return true;
}

bool WsSocket::Accept(WsSocket& socket)
{
	if (_socket == INVALID_SOCKET)
		return false;

	SOCKET rawSocket = accept(_socket, 0, 0);

	if (rawSocket == INVALID_SOCKET)
		return false;

	if (socket.IsOpen())
		socket.Close();

	socket._socket = rawSocket;

	if ((socket._event = CreateEvent(0, FALSE, FALSE, 0)) == WSA_INVALID_EVENT)
	{
		socket.Close();
		return false;
	}

	if (WSAEventSelect(socket._socket, socket._event, FD_WRITE | FD_READ | FD_CLOSE) == SOCKET_ERROR)
	{
		socket.Close();
		return false;
	}

	return true;
}

bool WsSocket::Broadcast(int port)
{
	Close();

	if ((_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
		return false;

	const BOOL kTrue = TRUE;

	if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char*)&kTrue, sizeof(kTrue)) == SOCKET_ERROR)
	{
		Close();
		return false;
	}

	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&kTrue, sizeof(kTrue)) == SOCKET_ERROR)
	{
		Close();
		return false;
	}

	WsSockAddr sockAddr(INADDR_ANY, port);

	if (bind(_socket, sockAddr, sockAddr.Length()) == SOCKET_ERROR)
	{
		Close();
		return false;
	}

	if ((_event = CreateEvent(0, FALSE, FALSE, 0)) == WSA_INVALID_EVENT)
	{
		Close();
		return false;
	}

	if (WSAEventSelect(_socket, _event, FD_READ | FD_CLOSE) == SOCKET_ERROR)
	{
		Close();
		return false;
	}

	return true;
}

void WsSocket::Close()
{
	if (_socket != INVALID_SOCKET)
	{
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}

	if (_event != WSA_INVALID_EVENT)
	{
		WSACloseEvent(_event);
		_event = WSA_INVALID_EVENT;
	}
}

#define SKTDBG(x)
//#define SKTDBG(x) DebugLn(x)

int WsSocket::TestConnection(WsSockAddr& sockAddr)
{
	if (_socket == INVALID_SOCKET)
		return -1;

	if (recv(_socket, 0, 0, 0) >= 0)
		return 1;

	switch(WSAGetLastError())
	{
		case WSAEWOULDBLOCK:
		return 1;
	}

	if (connect(_socket, 0, 0) == 0)
		return 1;

#ifdef PLATFORM_PC
	switch(WSAGetLastError())
	{
		case WSAEISCONN:		SKTDBG("EISCONN");		return 1;

		case WSAEALREADY:		SKTDBG("EALREADY");		return 0;
		case WSAEWOULDBLOCK:	SKTDBG("EWOULDBLOCK");	return 0;
		case WSAEINVAL:			SKTDBG("EINVAL");		return 0;

		case WSAENOTCONN:		SKTDBG("ENOTCONN");		return -1;
		default:				SKTDBG("EUNKNOWN!");	return -1;
	}
#else
	switch(WSAGetLastError())
	{
		case WSAEISCONN:		SKTDBG("EISCONN");		return 1;

		case WSAEALREADY:		SKTDBG("EALREADY");		return 0;

		case WSAEWOULDBLOCK:	SKTDBG("EWOULDBLOCK");	return -1;
		case WSAEINVAL:			SKTDBG("EINVAL");		return -1;
		case WSAENOTCONN:		SKTDBG("ENOTCONN");		return -1;
		default:				SKTDBG("EUNKNOWN!");	return -1;
	}
#endif
}

int WsSocket::Receive(void* data, int maxLength)
{
	if (_socket == INVALID_SOCKET)
		return -1;

	int recvLength = recv(_socket, (char*)data, maxLength, 0);

	if (recvLength == 0)
		return -1;

	if (recvLength < 0)
	{
		switch(WSAGetLastError())
		{
			case WSAEWOULDBLOCK:	return 0;
			default:				return -1;
		}
	}

	return recvLength;
}

int WsSocket::Send(const void* data, int length)
{
	if (_socket == INVALID_SOCKET)
		return -1;

	int sendLength = send(_socket, (const char*)data, length, 0);

	if (sendLength == 0)
		return -1;

	if (sendLength < 0)
	{
		switch(WSAGetLastError())
		{
			case WSAEWOULDBLOCK:	return 0;
			default:				return -1;
		}
	}

	return sendLength;
}

int WsSocket::ReceiveFrom(void* data, int maxLength, WsSockAddr& address)
{
	if (_socket == INVALID_SOCKET)
		return -1;

	int addrLength = address.Length();
	int recvLength = recvfrom(_socket, (char*)data, maxLength, 0, address, &addrLength);

	if ((recvLength == 0) || (addrLength != address.Length()))
		return -1;

	if (recvLength < 0)
	{
		switch(WSAGetLastError())
		{
			case WSAEWOULDBLOCK:	return 0;
			default:				return -1;
		}
	}

	return recvLength;
}

int WsSocket::SendTo(const void* data, int length, const WsSockAddr& address)
{
	if (_socket == INVALID_SOCKET)
		return -1;

	int sendLength = sendto(_socket, (const char*)data, length, 0, address, address.Length());

	if (sendLength == 0)
		return -1;

	if (sendLength < 0)
	{
		switch(WSAGetLastError())
		{
			case WSAEWOULDBLOCK:	return 0;
			default:				return -1;
		}
	}

	return sendLength;
}

void WsSocket::Attach(WsSocket& socket)
{
	if (&socket != this)
	{
		Close();

		_socket			= socket._socket;
		socket._socket	= INVALID_SOCKET;

		_event			= socket._event;
		socket._event	= WSA_INVALID_EVENT;
	}
}

#endif
