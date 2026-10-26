#ifndef __SYSTEM_SOCKET_SERVER_SUBSCRIBER_H__
#define __SYSTEM_SOCKET_SERVER_SUBSCRIBER_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class I_File;
}

namespace Network
{
	class Socket;
}

namespace Debug
{

/// a subscriber that allows clients to connect to a network port to receive logging events
class SocketServerSubscriber : public SocketSubscriber, public MT::Thread
{
public:
						DECLARE_SUBSCRIBER(SocketServerSubscriber);

						/// constructor
						SocketServerSubscriber();
						/// destructor
						~SocketServerSubscriber();

protected:
						/// open the socket to write to
	virtual bool		Start();

private:
	void				ThreadMain();

	MT::Mutex			m_Mutex;
	bool				m_SignalQuit;
};

};


#endif
