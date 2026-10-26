// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdNet.h>

#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

#include <bdConnection/bdConnectionStore.h>
#include <bdConnection/bdConnection.h>
#include <bdSocket/bdSocketRouter.h>


BD_REGISTER_SINGLETON(bdNetImpl)

#define BD_LOG_LEVEL	"bdNet/net"

bdBool bdNetImpl::start(const bdNetStartParams& params)
{
	if (m_status != BD_NET_STOPPED)
	{
		BD_WARN(BD_LOG_LEVEL,"bdNet subsystem already started");
		return false;
	}

	m_params = params;
	m_status = BD_NET_PENDING;

	if(!m_params.m_socket)
	{
		m_params.m_socket = new bdDefaultSocket;
		m_params.m_socket->create();
		bdAddr localAddr(m_params.m_localAddresses[0], m_params.m_gamePort);
		m_params.m_socket->bind(localAddr);
	}


	return true;
}

void bdNetImpl::pump()
{

	if(m_status == BD_NET_PENDING)
	{

		const bdAddr localAddr(m_params.m_localAddresses[0], m_params.m_gamePort);
		bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
		localAddr.getAddress().toString(addrStr, sizeof(addrStr));
		BD_INFO(BD_LOG_LEVEL, "Using local address: %s \n", addrStr);

		// Store the common addr
		//m_localCommonAddr  = new bdCommonAddr(localAddr);


		// Create the socket router
		m_socketRouter.init(m_params.m_socket, new bdCommonAddr(localAddr));

		// Create the connection store.
		m_connectionStore.init(&m_socketRouter);

		m_status = BD_NET_DONE;
	}
}


void bdNetImpl::stop()
{

	m_connectionStore.startShutdown(bdConnectionStore::BD_CONNECTION_STORE_SHUTDOWN_ELEGANT);

	// final flush for messages in the connections
	bdStopwatch timer;
	timer.start();

	while(timer.getElapsedTimeInSeconds() < 1.f)
	{
		receiveAll();
		dispatchAll();
		sendAll();
		bdPlatformTiming::sleep(20);
	}
	m_connectionStore.startShutdown(bdConnectionStore::BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE);


	m_socketRouter.quit();

	if(m_params.m_socket)
	{
		m_params.m_socket->close();
		delete m_params.m_socket;
		m_params.m_socket = BD_NULL;
	}
	m_status = BD_NET_STOPPED;
}

bdNetImpl::bdNetStatus bdNetImpl::getStatus() const
{
	return m_status;
}

bdBool bdNetImpl::sendAll()
{
	const bdBool result = m_connectionStore.flushAll();
	if(!result)
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to flush all connections in the connection store");
	}
	return result;
}

bdBool bdNetImpl::receiveAll()
{
	const bdBool result = m_connectionStore.receiveAll();
	if(false == result)
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to receive data for all connections in the connection store");
	}
	return result;
}

void bdNetImpl::dispatchAll()
{
	m_connectionStore.dispatchAll();
}

bdConnectionStore *bdNetImpl::getConnectionStore()
{
	return &m_connectionStore;
}

bdSocketRouter* bdNetImpl::getSocketRouter()
{
	return &m_socketRouter;
}

bdCommonAddrRef bdNetImpl::getLocalCommonAddr() const
{
	return m_socketRouter.getLocalCommonAddr();
}


bdNetImpl::bdNetImpl()
: m_status(BD_NET_STOPPED)
{
}

bdNetImpl::~bdNetImpl()
{
}

