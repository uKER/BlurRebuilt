#ifndef __SYSTEM_NETWORK_SOCKET_H__
#define __SYSTEM_NETWORK_SOCKET_H__

#pragma once

namespace Network
{

// enum for IsPending function
namespace Pending
{
	enum Pending
	{
		Input,
		Output,
		Error
	};
};

#if !defined( COMPILER_VC )
typedef int SOCKET;
#endif

/// Platform-independant network socket class
class Socket
{
public:
				/// create a socket
				Socket(int domain, int type, int protocol = 0);

				/// close a socket
				~Socket();

				/// Initialize using an exisiting SOCKET structure
				Socket(SOCKET s);

				/// connect to a specified address + port
	bool		Connect(const Address &ia, U32 port);

				/// wait for input on the socket
	bool		IsInputPending(U32 timeout = -1);

				/// send data to connected peer
	U32			Send(const void *buf, U64 len);

				/// receive data from connected peer
	U32			Receive(void *buf, U64 len);

				/// peek; data is copied into the buffer without removing it from the input queue
	U32			Peek(void *buf, U64 len);

				/// get associated SOCKET structure
	SOCKET		GetSocket();

				/// close socket
	void		Close();

protected:
	volatile SOCKET m_socket;
};

/// Platform-independant socket class, specialized to TCP protocol
class TCPSocket : public Socket
{
public:
				/// create an unconnected socket (client), connect to peer by calling 'Connect'
				TCPSocket();

				/// bind to a port and listen for incoming connections (server)
				TCPSocket(U16 port, unsigned backlog = -1);

				/// Initialize using an exisiting SOCKET structure
				TCPSocket(SOCKET s);

				~TCPSocket();
};

};


#endif
