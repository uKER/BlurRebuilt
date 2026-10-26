#ifndef __SYSTEM_SOCKET_SUBSCRIBER_H__
#define __SYSTEM_SOCKET_SUBSCRIBER_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class I_File;
}
namespace MT
{
	class Mutex;
}

namespace Network
{
	class Socket;
}

namespace Debug
{

/// a subscriber that sends the events across the network to a specified host
class SocketSubscriber : public I_LogSubscriber
{
public:
						DECLARE_SUBSCRIBER(SocketSubscriber);

						/// constructor
						SocketSubscriber();
						/// destructor
	virtual				~SocketSubscriber();

						/// send a log event to this subscriber
	virtual void		Log(LogEvent *pEvent);

						/// parse a configuration parameter for this subscriber
	virtual void		SetXMLParameter(const char *pName, const char *pValue);

						/// open the socket to write to
	virtual bool		Start();
protected:
						/// send a block of memory to all connections
	virtual void		SendToAllConnections(U8 *pData, U64 cData);

	Util::C_String					m_sHostname;
	S32								m_iPort;
	Ut::Vector<Network::Socket*>	m_Connections;
	MT::Mutex						*m_pMutex;
};

};


#endif
