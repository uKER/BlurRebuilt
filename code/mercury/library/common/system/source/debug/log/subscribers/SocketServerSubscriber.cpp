#include "SystemPch.h"

#include "system/debug/log/DebugLog.h"
#include <core/threads.h>
#include <system/network/address.h>
#include <system/network/socket.h>
#include <system/debug/log/Subscribers/I_LogSubscriber.h>
#include <system/debug/log/Subscribers/SocketSubscriber.h>
#include <system/debug/log/Subscribers/SocketServerSubscriber.h>
#include <system/Serialise.h>

namespace Debug
{

IMPLEMENT_SUBSCRIBER(SocketServerSubscriber, "server");

// constructor
SocketServerSubscriber::SocketServerSubscriber()
{
	m_SignalQuit = false;
}

SocketServerSubscriber::~SocketServerSubscriber()
{
	m_SignalQuit = true;
	Thread::Join();
}


/// start the subscriber (allows the subscriber to open a file or do whatever it needs to do before it starts receiving events)
/// returns false if the subscriber fails to start
bool SocketServerSubscriber::Start()
{
	// start the thread that accepts the incoming connections
	Thread::Start();

	return true;
}

/// entrypoint of server thread that handles the connnections
void SocketServerSubscriber::ThreadMain()
{
	Network::TCPSocket socket(m_iPort, 6);

	while (1)
	{
		// wait a second for an incoming connection
		if (socket.IsInputPending(1000))
		{
			// create a socket containing the new connection
			Network::Socket *pConnection = new Network::Socket(accept(socket.GetSocket(), NULL, NULL));
			if (m_Mutex.Wait())
			{
				m_Connections.PushBack(pConnection);
				m_Mutex.Release();
			}
		}

		// check for exit signal
		if (m_SignalQuit)
			return;
	}
}


};

