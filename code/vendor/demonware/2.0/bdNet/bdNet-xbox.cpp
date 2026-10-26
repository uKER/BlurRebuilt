// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdNet.h>

#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>
#include <bdCore/bdSocket/bdSecurityKey.h>
#include <bdConnection/bdConnectionStore.h>
#include <bdConnection/bdConnection.h>
#include <bdSocket/bdSocketRouter.h>

BD_REGISTER_SINGLETON(bdNetImpl)

#define BD_LOG_LEVEL	"bdNet/net"

#define BD_NET_MAX_SHUTDOWN_DURATION (1.0f)

bdBool bdNetImpl::start(const bdNetStartParams &params)
{
	m_params = params;

	if(!m_params.m_socket)
	{
		m_params.m_socket = new bdSocket;
		const bdBool broadcast = false;
		const bdBool blocking = false;
		m_params.m_socket->create(blocking, broadcast);
	}
	m_params.m_socket->bind(m_params.m_gamePort);

#if defined BD_COMPILE_RELAY_SOCKET_ROUTER
	if(m_params.m_useRelaySocketRouter)
	{
		m_socketRouter = new bdRelaySocketRouter();
	}
	else
	{
		m_socketRouter = new bdSocketRouter();
	}
#else
	m_socketRouter = new bdSocketRouter();
#endif
	m_status = BD_NET_PENDING;

	return true;
}

void bdNetImpl::pump()
{
	if(m_status == BD_NET_PENDING)
	{
		XNADDR addr;
		bdUWord val = XNetGetTitleXnAddr(&addr);

		if(val != XNET_GET_XNADDR_PENDING)
		{
			if(val == XNET_GET_XNADDR_TROUBLESHOOT || val == XNET_GET_XNADDR_NONE)
			{
				m_status = BD_NET_INIT_FAILED;
				BD_WARN(BD_LOG_LEVEL, "XNetGetTitleXnAddr failed.");
			}
			else
			{
				bdCommonAddrRef localCommonAddr = new bdCommonAddr(addr, m_params.m_gamePort);

				// create the socket router.
				m_socketRouter->init(m_params.m_socket, localCommonAddr);

				//create the connection store.
				m_connectionStore.init(m_socketRouter);
				m_status = BD_NET_DONE;
			}
		}
	}
}

/*
void bdNetImpl::stop()
{
	m_connectionStore.startShutdown();

	// Failsafe timer...!
	bdStopwatch timer;
	timer.start();

	bdBool waiting = true;
	while(waiting)
	{
		// Stop by default
		waiting = false;
		if(bdConnectionStore::BD_CONNECTION_STORE_UNINITIALIZED != m_connectionStore.getStatus())
		{
			waiting = true;
			receiveAll();
			dispatchAll();
			sendAll();
		}

		if(timer.getElapsedTimeInSeconds() > BD_NET_MAX_SHUTDOWN_DURATION)
		{
			// waiting is over!
			waiting = false;
			// force immediate shutdown..
			m_connectionStore.startShutdown(bdConnectionStore::BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE);
		}

		bdPlatformTiming::sleep(20);
	}
	if( m_socketRouter )
	{
		m_socketRouter->quit();
		delete m_socketRouter;
		m_socketRouter = BD_NULL;
	}

	if(m_params.m_socket)
	{
		m_params.m_socket->close();
		delete m_params.m_socket;
		m_params.m_socket = BD_NULL;
	}

	m_status = BD_NET_STOPPED;
}
*/

void bdNetImpl::stop()
{
	if (bdConnectionStore::BD_CONNECTION_STORE_UNINITIALIZED != m_connectionStore.getStatus())
		m_connectionStore.startShutdown(bdConnectionStore::BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE);

	if( m_socketRouter )
	{
		m_socketRouter->quit();
		delete m_socketRouter;
		m_socketRouter = BD_NULL;
	}

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
	if(false == result)
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
	return m_socketRouter;
}

bdCommonAddrRef bdNetImpl::getLocalCommonAddr() const
{
	bdCommonAddrRef local = BD_NULL;
	if( m_socketRouter != BD_NULL )
	{
		local = m_socketRouter->getLocalCommonAddr();
	}
	return local;
}

const bdNetStartParams& bdNetImpl::getParams()
{
	return m_params;
}


bdNetImpl::bdNetImpl()
	:m_socketRouter(BD_NULL),
	m_status(BD_NET_STOPPED)
{
}

bdNetImpl::~bdNetImpl()
{
	stop();
}
