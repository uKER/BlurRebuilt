#ifndef SYSTEM_ARTERY_BKSOCKET_H
#define SYSTEM_ARTERY_BKSOCKET_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sys/socket.h>
#include <netinet/in.h>

#if defined(PLATFORM_PS3)
#include <netex/errno.h>
#endif

//
// BkSockAddr
//

class BkSockAddr
{
public:
	inline BkSockAddr()
	{
		memset(&_sockAddr, 0, sizeof(_sockAddr));

		_sockAddr.sin_family = AF_INET;
	}

	inline BkSockAddr(in_addr addr, int port = 0)
	{
		memset(&_sockAddr.sin_zero, 0, sizeof(_sockAddr.sin_zero));

		_sockAddr.sin_family	= AF_INET;
		_sockAddr.sin_port		= htons(port);
		_sockAddr.sin_addr		= addr;
	}

	inline BkSockAddr(unsigned int addr, int port = 0)
	{
		memset(&_sockAddr.sin_zero, 0, sizeof(_sockAddr.sin_zero));

		_sockAddr.sin_family		= AF_INET;
		_sockAddr.sin_port			= htons(port);
		_sockAddr.sin_addr.s_addr	= htonl(addr);
	}

public:
	inline operator sockaddr*()
	{
		return (sockaddr*)&_sockAddr;
	}

	inline operator const sockaddr*() const
	{
		return (const sockaddr*)&_sockAddr;
	}

public:
	inline bool operator==(const BkSockAddr& rhs) const
	{
		return (
				(_sockAddr.sin_addr.s_addr == rhs._sockAddr.sin_addr.s_addr) &&
				(_sockAddr.sin_port == rhs._sockAddr.sin_port)
			);
	}

public:
	inline in_addr Addr() const
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
	sockaddr_in _sockAddr;
};

//
// BkSocket
//

class BkSocket
{
public:
	BkSocket();
	~BkSocket();

public:
	bool Connect(const BkSockAddr& sockAddr);
	bool Listen(const BkSockAddr& sockAddr);
	bool Accept(BkSocket& socket);
	bool Broadcast(int port);
	void Close();

public:
	int TestConnection(BkSockAddr& sockAddr);

public:
	int Receive(void* data, int maxLength);
	int Send(const void* data, int length);

public:
	int ReceiveFrom(void* data, int maxLength, BkSockAddr& address);
	int SendTo(const void* data, int length, const BkSockAddr& address);

public:
	void Attach(BkSocket& socket);

public:
	inline bool IsOpen() const
	{
		return _socket >= 0;
	}

public:
	inline int Handle() const
	{
		return _socket;
	}

private:
	int _socket;
};

bool CrossConnect(BkSocket& a, BkSocket& b);

#endif // SYSTEM_BKSOCKET_H
