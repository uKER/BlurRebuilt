#ifndef __SYSTEM_NETWORK_PLATFORM_H__
#define __SYSTEM_NETWORK_PLATFORM_H__

#if !defined( COMPILER_VC )
#pragma once
#endif

/*! \mainpage Mercury Network Abstraction Library
*
* \section intro_sec Introduction
*
* The Mercury Network Library facilitates writing code network code that is easily portable
* between Xenon, PS3, PC and Linux.
* 
* For an example of a server see "mercury\library\system\source\debug\log\NetworkRepeater.cpp"
* 
* For an example of a client, see below
*

<code><pre>

##include <system\\network.h>

void Main()
{
	// try to connect to a specified address + port
	Network::TCPSocket *pSocket = new Network::TCPSocket();

	if (pSocket->Connect( Network::Address( "zappa" ), m_iPort ))
	{
		// send some data to the socket
		pSocket->Send(&dummy, 4);

		// receive some data
		pSocket->Receive(&buffer, 256);

		// close the socket
		pSocket->Close();
	}

	delete pSocket;
}

</code></pre>
*/


// network platform includes

#if defined( COMPILER_VC )
#if defined( PLATFORM_PC)
#include <winsock2.h>
#endif

#include <ws2tcpip.h>

#else
#include <fcntl.h>
#include <cerrno>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#if defined( PLATFORM_PS3 )
#include <sys/select.h>
#include <sys/time.h>
#include <netex/net.h>
#else
#include <sys/ioctl.h>
#include <net/if.h>
#endif
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#endif
