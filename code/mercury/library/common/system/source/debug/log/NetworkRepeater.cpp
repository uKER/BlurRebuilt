#include "SystemPch.h"

#include "system/debug/log/DebugLog.h"
#include <core/threads.h>
#include "system/network/address.h"
#include "system/network/socket.h"
#include "system/network/service.h"
#include "system/network/connection.h"
#include "system/debug/log/NetworkRepeater.h"
#include <core/utility/endian.h>
#include "system/Serialise.h"
#include "system/serialise/C_MemoryStream.h"
#ifdef PLATFORM_PS3
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#endif

namespace Debug
{

/// --- RepeaterConnection

/// Constructor
RepeaterConnection::RepeaterConnection(Socket &s) :
	Network::Connection(s),
	m_pBuffer(NULL),
	m_bReadHeader(true)
{
}

/// Destructor
RepeaterConnection::~RepeaterConnection()
{
	if (m_pBuffer)
		delete m_pBuffer;
	m_pBuffer = NULL;
}

/// Called when data is to be read for this connection
void RepeaterConnection::Pending()
{
	if (m_bReadHeader)
	{
		// first thing we read is the header, to check whether we need to endian-swap
		U32 dummy;
		Socket::Receive(&dummy, 4);

		m_bNeedEndianSwap = (dummy != 1);
		m_bReadHeader = false;
	}

	// keep trying to receive data, in case we are receiving multiple messages at the same time
	while (1)
	{
		if (!m_pBuffer)
			m_pBuffer = new Sys::C_MemoryStream(1024);

		// keep reading into buffer until either the message is complete or we can't read anymore
		while (1)
		{
			U64 iBytesToRead = m_pBuffer->m_pBufferEnd - m_pBuffer->m_pCurBuffer;
			U64 iBytesReadSoFar = m_pBuffer->m_pCurBuffer - m_pBuffer->GetBufferPtr();
			if (iBytesReadSoFar<4)
				iBytesToRead = 4 - iBytesReadSoFar;
			else
				iBytesToRead = 4 + ntohl( *((int*)m_pBuffer->GetBufferPtr()) ) - iBytesReadSoFar;
			if (iBytesToRead == 0)
				break;	// read a complete message

			while (iBytesToRead)
			{
				U64 x = iBytesToRead;
				// read as much as we can (to the end of the current buffer)
				U64 iLeft = m_pBuffer->m_pBufferEnd-m_pBuffer->m_pCurBuffer;
				if (x > iLeft)
					x = iLeft;
				int c = Socket::Receive(m_pBuffer->m_pCurBuffer, x);
				if (c<=0)
					return; // data ended with an incomplete message

				m_pBuffer->m_pCurBuffer += c;
				iBytesToRead -= c;

				if (m_pBuffer->m_pCurBuffer >= m_pBuffer->m_pBufferEnd)
					m_pBuffer->Overflow();	// need more space
			}
		}

		// got a complete message
		LogEvent event;
		Sys::C_MemoryStream buf(m_pBuffer->GetBufferPtr()+4, (U32)m_pBuffer->GetBufferSize()-4);
		if (m_bNeedEndianSwap)
		{
			Sys::BinaryDeserialiser<Ut::DoEndianSwap> ar(buf);

			ar >> event;
		}
		else
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(buf);

			ar >> event;
		}

		delete m_pBuffer;
		m_pBuffer = NULL;

		// detect feedback by checking whether this message originated from this machine and this pid
		static const Util::C_String sHostName = Network::GetHostName();
		if (event.pid == MT::GetProcessID() && event.host == sHostName)
			return;

		GetLogger(event.logger).Log(event);
	}
}

/// Called when this connection is ready to receive data
void RepeaterConnection::Output()
{
}

/// Called on disconnect
void RepeaterConnection::Disconnect()
{
	Socket::Close();
	if ( m_pServiceThread )
	{
		printf("detaching connection\n");
		m_pServiceThread->Detach(this);	// this function calls the connection's destructor!
	}
}

/// Called when the connection has been idle for a set amount of time
void RepeaterConnection::Idle()
{
}

/// --- RepeaterServer

/// Constructor
RepeaterServer::RepeaterServer(U16 iPort = 825) :
	m_iPort(iPort)
{
	m_SignalQuit = false;
	m_pService = new Network::Service();
}

/// Destructor
RepeaterServer::~RepeaterServer()
{
	delete m_pService;
}

/// Start the server
void RepeaterServer::Start()
{
	MT::Thread::Start();	
}

/// Stop the server
void RepeaterServer::Stop()
{
	m_SignalQuit = true;
	MT::Thread::Join();

	// kill all connections
	m_pService->Stop();
}

/// main function for the thread that accepts new connections
void RepeaterServer::ThreadMain()
{
	// open listening socket 
	Network::TCPSocket socket(m_iPort, 6);

	while (1)
	{
		if (m_SignalQuit)
			return;

		// wait a second for an incoming connection
		if (socket.IsInputPending(1000))
		{
			printf("accepting connection\n");

			// create a connection with the new socket and add to the service
			Network::Socket s(accept(socket.GetSocket(), NULL, NULL));
			m_pService->Attach(new RepeaterConnection(s));
		}
	}
}


}
