#include "SystemPch.h"

#include "system\debug\log\DebugLog.h"
#include <system/debug/log/Subscribers/I_LogSubscriber.h>
#include <system/debug/log/Subscribers/SocketSubscriber.h>
#include <system/network/address.h>
#include <system/network/socket.h>
#include <core/utility/endian.h>
#include <system/Serialise.h>
#include <system/serialise/C_MemoryStream.h>
#ifdef PLATFORM_PS3
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#endif

namespace Debug
{

IMPLEMENT_SUBSCRIBER(SocketSubscriber, "socket");

// constructor
SocketSubscriber::SocketSubscriber()
{
	m_sHostname = "";
	m_iPort = 825;

	m_pMutex = new MT::Mutex;
}

// destructor
SocketSubscriber::~SocketSubscriber()
{
	for (int i=0; i<m_Connections.Size(); i++)
	{
		m_Connections[i]->Close();
		delete m_Connections[i];
	}

	delete m_pMutex;
}

/// start the subscriber (allows the subscriber to open a file or do whatever it needs to do before it starts receiving events)
/// returns false if the subscriber fails to start
bool SocketSubscriber::Start()
{
	// try to connect to the specified address + port
	Network::TCPSocket *pSocket = new Network::TCPSocket();

	if (pSocket->Connect( Network::Address( m_sHostname ), m_iPort ))
	{
		m_Connections.PushBack( pSocket );

		// send header
		U32 dummy = 1;
		pSocket->Send(&dummy, 4);

		return true;
	}

	// fail
	delete pSocket;
	return false;
}

/// receive a log event and send it across the network
void SocketSubscriber::Log(LogEvent *pEvent)
{
	// create message packet
	Sys::C_MemoryStream buffer(4096);
	Sys::BinarySerialiser<Ut::NoEndianSwap> msg(buffer);

	msg << *pEvent;

	m_pMutex->Wait();

	SendToAllConnections( buffer.GetBufferPtr(), buffer.GetBufferSize() );

	m_pMutex->Release();
}

/// send a block of memory to all connections
void SocketSubscriber::SendToAllConnections(U8 *pData, U64 cData)
{
	U32 c = htonl((U32)cData);

	// send this message to all connections
	for (int i=0; i<m_Connections.Size(); i++)
	{
		m_Connections[i]->Send(&c, sizeof(c));
		m_Connections[i]->Send(pData, (size_t)cData);
	}
}

/// parse a configuration parameter for this subscriber
void SocketSubscriber::SetXMLParameter(const char *pName, const char *pValue)
{
	if (!stricmp(pName, "host"))
		m_sHostname = Util::C_String( pValue );
	else if (!stricmp(pName, "port"))
		m_iPort = atoi(pValue);
}


};
