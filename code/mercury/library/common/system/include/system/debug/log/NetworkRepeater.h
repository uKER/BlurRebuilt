#ifndef __SYSTEM_LOG_NETWORK_REPEATER_H__
#define __SYSTEM_LOG_NETWORK_REPEATER_H__

#pragma once

namespace Sys
{
	class C_MemoryStream;
}

namespace Debug
{

/// receive network messages and turn them into local debug events
class RepeaterConnection : public Network::Connection
{
public:
						/// Constructor
						RepeaterConnection(Network::Socket &s);
						/// Destructor
						~RepeaterConnection();
protected:
						/// Called when data is to be read for this connection
	void				Pending();
						/// Called when this connection is ready to receive data
	void				Output();
						/// Called on disconnect
	void				Disconnect();
						/// Called when the connection has been idle for a set amount of time
	void				Idle();

private:
	Sys::C_MemoryStream	*m_pBuffer;

	bool				m_bNeedEndianSwap;
	bool				m_bReadHeader;
};

/// repeater server, which can receive log events from multiple clients simultaneously
class RepeaterServer : public MT::Thread
{
public:
						/// Constructor
						RepeaterServer(U16 iPort);
						/// Destructor
						~RepeaterServer();

						/// Start the server
	void				Start();
						/// Stop the server
	void				Stop();

protected:
						/// main function for the thread that accepts new connections
	void				ThreadMain();

	Network::Service	*m_pService;
	U16					m_iPort;
	bool				m_SignalQuit;
};


};

#endif
