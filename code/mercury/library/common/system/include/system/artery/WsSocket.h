#ifndef SYSTEM_ARTERY_WSSOCKET_H
#define SYSTEM_ARTERY_WSSOCKET_H

#ifndef PLATFORM_PS3

#ifdef _MSC_VER
#pragma once
#endif

//
// WsSockAddr
//

class WsSockAddr
{
public:
	inline WsSockAddr()
	{
		ZeroMemory(&_sockAddr, sizeof(_sockAddr));

		_sockAddr.sin_family = AF_INET;
	}

	inline WsSockAddr(IN_ADDR addr, int port = 0)
	{
		ZeroMemory(&_sockAddr.sin_zero, sizeof(_sockAddr.sin_zero));

		_sockAddr.sin_family	= AF_INET;
		_sockAddr.sin_port		= htons((unsigned short)port);
		_sockAddr.sin_addr		= addr;
	}

	inline WsSockAddr(unsigned int addr, int port = 0)
	{
		ZeroMemory(&_sockAddr.sin_zero, sizeof(_sockAddr.sin_zero));

		_sockAddr.sin_family			= AF_INET;
		_sockAddr.sin_port				= htons((unsigned short)port);
		_sockAddr.sin_addr.S_un.S_addr	= htonl(addr);
	}

public:
	inline operator SOCKADDR*()
	{
		return (SOCKADDR*)&_sockAddr;
	}

	inline operator const SOCKADDR*() const
	{
		return (const SOCKADDR*)&_sockAddr;
	}

public:
	inline bool operator==(const WsSockAddr& rhs) const
	{
		return (
				(_sockAddr.sin_addr.s_addr == rhs._sockAddr.sin_addr.s_addr) &&
				(_sockAddr.sin_port == rhs._sockAddr.sin_port)
			);
	}

public:
	inline IN_ADDR Addr() const
	{
		return _sockAddr.sin_addr;
	}

	inline int Port() const
	{
		return ntohs(_sockAddr.sin_port);
	}

	inline int Length() const
	{
		return sizeof(_sockAddr);
	}

private:
	SOCKADDR_IN _sockAddr;
};

//
// WsSocket
//

class WsSocket
{
public:
	WsSocket();
	~WsSocket();

public:
	bool Connect(const WsSockAddr& sockAddr);
	bool Listen(const WsSockAddr& sockAddr);
	bool Accept(WsSocket& socket);
	bool Broadcast(int port);
	void Close();

public:
	int TestConnection(WsSockAddr& sockAddr);

public:
	int Receive(void* data, int maxLength);
	int Send(const void* data, int length);

public:
	int ReceiveFrom(void* data, int maxLength, WsSockAddr& address);
	int SendTo(const void* data, int length, const WsSockAddr& address);

public:
	void Attach(WsSocket& socket);

public:
	inline bool IsOpen() const
	{
		return _socket != INVALID_SOCKET;
	}

	inline WSAEVENT Event() const
	{
		return _event;
	}

private:
	SOCKET		_socket;
	WSAEVENT	_event;
};

#else
class WsSockAddr
{
};	
#endif

#endif // SYSTEM_WSSOCKET_H
