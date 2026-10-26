// ***************************************************************
//  UnreliableSocket   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Basic unreliable UDP socket.
// ***************************************************************

#ifndef NETWORK_UNRELIABLE_SOCKET_H
#define NETWORK_UNRELIABLE_SOCKET_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NetPlay
{
	// A wrapper for sending and receiving data from unreliable sockets
	class C_UnreliableSocket
	{
	public:
		explicit C_UnreliableSocket(); 
		virtual ~C_UnreliableSocket(); 

		/// try to create the socket, pass in false for UPD socket or true for VDP sockets on Xenon
		/// returns true if socket was created or false if bad things happened..
		bool Open(const bool voiceAndData = false);

		/// try and bind the socket to a specified port number
		/// returns true if bind succeeded or false if it did not.
		bool Bind(const U16 portNumber);

		/// Call to turn on/off non blocking mode once socket has been opened
		/// return true if successful or false if error occurred
		bool EnableNonBlockingMode(const bool enableNonBlocking);

		/// Call to change the socket broadcast functionality.
		/// return true if successful or false if error occurred
		bool EnableBroadcastMode(const bool enableBroadcast);

		/// Attempt to receive data from the socket..
		/// maxBytesToReceive	=	How many bytes can we read from the socket on this call.
		/// pOutBuffer			=	Pointer to where we want the data from the socket to go.
		/// outBytesReceived	=	Number of bytes actually written into pOutBuffer (will never go above maxBytesToReceive)
		/// outAddress			=	Address of remote console who sent the data.
		/// returns true if call succeeded or false if failed.
		bool RecvFrom(const U32 maxBytesToReceive, void* pOutBuffer, U32& outBytesReceived, sockaddr_in& outAddress); 

		/// Send a set number of bytes to the specified remote address
		/// pSendBuffer		=	Pointer to the data that is to be sent
		/// numBytesToSend	=	Number of bytes to send.. limited by max size of a UDP packet.
		/// portNumber		=	Port to send data to.
		/// remoteAddress	=	Address to send data to
		/// returns true if send was successful or false if it was not.
		bool SendTo(void* pSendBuffer, const U32 numBytesToSend, const sockaddr_in& remoteAddress);

		/// broadcast a set number of bytes out on a specific port, EnableBroadcastMode(true) must have been previously called 
		/// pSendBuffer		=	Pointer to the data that is to be sent
		/// numBytesToSend	=	Number of bytes to send.. limited by max size of a UDP packet.
		/// portNumber		=	Port to send data to.
		/// returns true if send was successful or false if it was not.
		bool SendBroadcast(void* pSendBuffer, const U32 numBytesToSend, const U32 portNumber);

	private:
		void  CloseSocket(void);

		C_UnreliableSocket(const C_UnreliableSocket& original);				// private copy constructor
		C_UnreliableSocket& operator = (const C_UnreliableSocket rhs);		// private un coded assignment operator

	private:
		U32	m_Socket;
		bool m_BroadcastMode; 
	};
}

#endif


