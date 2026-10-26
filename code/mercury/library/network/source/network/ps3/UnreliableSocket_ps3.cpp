#include <StdAfx.h>
#include <Network\UnreliableSocket.h>


namespace NetPlay
{
	// constructor
	C_UnreliableSocket::C_UnreliableSocket() : m_Socket(-1), m_BroadcastMode(false) 
	{
	}


	// destructor
	C_UnreliableSocket::~C_UnreliableSocket() 
	{
		CloseSocket();
	}


	bool C_UnreliableSocket::Open(const bool voiceAndData)
	{
		CloseSocket();
#ifndef PLATFORM_PS3 // [DannyP][OliR]
		m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
		return m_Socket >= 0;
	}


	// try and bind the opened socket to a specific port
	bool C_UnreliableSocket::Bind(const U16 portNumber)
	{
		if (m_Socket >= 0)
		{
			sockaddr_in sI;
			memset(&sI, 0, sizeof(sI));
			sI.sin_family		= AF_INET;
			sI.sin_addr.s_addr	= INADDR_ANY;
			sI.sin_port			= htons(portNumber);
#ifndef PLATFORM_PS3 // [DannyP][OliR]
			return bind(m_Socket, (const sockaddr*)(&sI), sizeof(sockaddr_in)) == 0;
#endif
		}
		return false;
	}


	// change the socket into sync or asynchronous send / receive
	bool C_UnreliableSocket::EnableNonBlockingMode(const bool enableNonBlocking)
	{
		if (m_Socket >= 0)
		{
			S32 nonBlock = enableNonBlocking ? 1 : 0;
#ifndef PLATFORM_PS3 // [DannyP][OliR]
			return setsockopt(m_Socket, SOL_SOCKET, SO_NBIO, &nonBlock, sizeof(nonBlock)) == 0;
#endif
		}
		return false;
	}


	// change the socket into or out of broadcast mode..
	bool C_UnreliableSocket::EnableBroadcastMode(const bool enableBroadcast)
	{
		if (m_Socket >= 0)
		{
			const S32 opt = enableBroadcast;
#ifndef PLATFORM_PS3 // [DannyP][OliR]
			if (setsockopt(m_Socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) == 0)
			{
				m_BroadcastMode = enableBroadcast;
				return true;
			}
#endif
		}
		return false;
	}


	// attempt to read data from the socket..
	bool C_UnreliableSocket::RecvFrom(const U32 maxBytesToReceive, void* pOutBuffer, U32& outBytesReceived, sockaddr_in& outAddress)
	{
		outBytesReceived = 0;
		if (m_Socket >= 0 && maxBytesToReceive && pOutBuffer)
		{
			socklen_t size = sizeof(sockaddr_in);
			sockaddr_in sender;	
			memset(&sender, 0, sizeof(sender));
#ifndef PLATFORM_PS3 // [DannyP][OliR]
			const S32 bytesReceived = recvfrom(m_Socket, (char*)(pOutBuffer), maxBytesToReceive, 0, (sockaddr*)(&sender), &size);
			if (bytesReceived >= 0)
			{
				outBytesReceived		=	bytesReceived;
				outAddress				=	sender;
				return true;
			}
#endif			
		}
		return false;
	}

	
	// attempt to send data to a remote client.
	bool C_UnreliableSocket::SendTo(void* pSendBuffer, const U32 numBytesToSend, const sockaddr_in& remoteAddress)
	{
		if (m_Socket >= 0 && numBytesToSend && pSendBuffer)
		{
			const S32 bytesSent = sendto(m_Socket, (const char*)(pSendBuffer), numBytesToSend, 0, (const sockaddr*)(&remoteAddress), sizeof(sockaddr_in));
			if (bytesSent >= 0)
				return (bytesSent == (S32)numBytesToSend) ? true : false; 
		}
		return false;
	}


	// broadcast data to all addresses on specified port..
	bool C_UnreliableSocket::SendBroadcast(void* pSendBuffer, const U32 numBytesToSend, const U32 portNumber)
	{
		if (m_Socket >= 0 && numBytesToSend && pSendBuffer)
		{
			if (m_BroadcastMode)
			{
				sockaddr_in sI;
				memset(&sI, 0, sizeof(sI));
				sI.sin_family		= AF_INET;
				sI.sin_addr.s_addr	= INADDR_BROADCAST;
				sI.sin_port			= htons(portNumber);

#ifndef PLATFORM_PS3 // [DannyP][OliR]
				const S32 bytesSent = sendto(m_Socket, (const char*)(pSendBuffer), numBytesToSend, 0, (const sockaddr*)(&sI), sizeof(sockaddr_in));
				if (bytesSent >= 0)
					return (bytesSent == (S32)numBytesToSend) ? true : false; 
#endif
			}
			else
				LOGWARN( g_Log,  "Attempting to broadcast data on a non broadcast enabled socket..\n");
		}
		return false;
	}


	// private function to close the socket if open
	void C_UnreliableSocket::CloseSocket(void)
	{
		m_BroadcastMode = false;
		if (m_Socket >= 0)
		{
#ifndef PLATFORM_PS3 // [DannyP][OliR]
			const S32 r = socketclose(m_Socket);
			assert(r == 0);
#endif
		}
		m_Socket = -1;
	}
}
