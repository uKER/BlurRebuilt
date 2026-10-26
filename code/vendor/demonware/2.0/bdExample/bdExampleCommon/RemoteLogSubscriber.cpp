// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/RemoteLogSubscriber.h>

RemoteLogSubscriber::RemoteLogSubscriber()
: m_socketType(BD_UDP_BROADCAST_SOCKET)
{	
}
 
RemoteLogSubscriber::~RemoteLogSubscriber()
{
	quit();
}

bdBool RemoteLogSubscriber::init(	const bdSocketType type,
									const bdAddr &logMachine)
{
	bdBool result = false;
	m_socketType = type;
	switch(type)
	{
		case BD_UDP_BROADCAST_SOCKET:
		{
			const bdBool broadcastFlag = true;
			result = createSocket(broadcastFlag);
			setDestination(logMachine);
			break;
		}
		case BD_UDP_SOCKET:
		{
			const bdBool broadcastFlag = false;
			result = createSocket(broadcastFlag);
			setDestination(logMachine);
			break;
		}
		case BD_TCP_SOCKET:
		{
			result = createStreamSocket(logMachine);
			break;
		}
		default:
		{
			break;
		}
	}
	return result;
}

void RemoteLogSubscriber::quit()
{
	closeSocket();
}

bdBool RemoteLogSubscriber::createSocket(const bdBool isBroadcast)
{
	const bdBool blocking = false;
	return m_socket.create(blocking,isBroadcast);
}

bdBool RemoteLogSubscriber::createStreamSocket(const bdAddr &destination)
{
	const bdBool blocking = false;
	bdBool result = m_streamSocket.create(blocking);
	if(result)
	{
		bdSocketStatusCode status = m_streamSocket.connect(destination);
		if(status == BD_NET_SUCCESS || status == BD_NET_WOULD_BLOCK)
		{
			m_logMachine = destination;
		}
		else
		{
			result = false;
		}
	}
	return result;
}

void RemoteLogSubscriber::closeSocket()
{
	m_socket.close();
}

void RemoteLogSubscriber::setDestination(const bdAddr &logMachine)
{
	m_logMachine = logMachine;
}

void RemoteLogSubscriber::publish(const bdLogMessageType type,
								  const bdNChar8 *const, 
								  const bdNChar8 *const file, 
								  const bdNChar8 *const, 
								  const bdUInt line, 
								  const bdNChar8 *const msg)const
{
	const bdNChar8 *prefix = " ";
	const bdNChar8 *const warningPrefix = "\nWARNING: ";
	const bdNChar8 *const errorPrefix = "\nERROR: ";

	switch(type)
	{
		case BD_LOG_WARNING:
		{
			prefix = warningPrefix;
		}
		break;
		case BD_LOG_ERROR:
		{
			prefix = errorPrefix;
		}
		break;
		case BD_LOG_INFO:
		break;
	}

	const bdNChar8 *filename = bdStrrchr(file, '\\');
	if(filename == BD_NULL)
	{
		filename = file;
	}
	else
	{
		// skip the "\"
		filename++;
	}

	if(	(BD_UDP_BROADCAST_SOCKET == m_socketType ) ||
		(BD_UDP_SOCKET == m_socketType ))
	{
		if(m_logMachine.getAddress().isValid())
		{

			bdNChar8 string[2048];
			bdSnprintf(string, sizeof(string), "%s(%u):%s%s\n", filename, line, prefix, msg);
			const_cast<bdDefaultSocket&>(m_socket).sendTo(m_logMachine, string, static_cast<bdUInt>(bdStrlen(string)));
		}
	}
	else if (BD_TCP_SOCKET == m_socketType)
	{
		if(m_logMachine.getAddress().isValid())
		{

			bdNChar8 string[2048];
			bdSnprintf(string, sizeof(string), "%s(%u):%s%s\n", filename, line, prefix, msg);
			m_streamSocket.send(string, static_cast<bdUInt>(bdStrlen(string)));

		}
	}
	else
	{
		bdUInt socTypeAsInt = m_socketType;
		BD_ERR("remoteLog", "Coding Error in RemoteLogSubscriber m_socketType has unexpected type : %u" , socTypeAsInt);
	}
}

