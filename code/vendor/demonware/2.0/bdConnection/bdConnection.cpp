// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdConnection.h>
#include <bdConnection/bdConnectionListener.h>

#define BD_LOG_LEVEL	"bdConnection/connections"

bdConnection::~bdConnection()
{
}

bdCommonAddrRef bdConnection::getAddress() const
{
	return m_addr;
}

const bdAddrHandleRef &bdConnection::getAddressHandle() const
{
	return m_addrHandle;
}

void bdConnection::setAddressHandle(const bdAddrHandleRef &addr)
{
	m_addrHandle = addr;
}

bdConnection::Status bdConnection::getStatus() const
{
	return m_status;	
}

void bdConnection::setTransmissionRate(const bdUInt bytesPerSecond)
{
	m_maxTransmissionRate = bytesPerSecond;
}

bdUInt bdConnection::getTransmissionRate() const
{
	return m_maxTransmissionRate;
}

void bdConnection::registerListener(bdConnectionListener *const listener)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_listeners.findFirst(listener, index))
	{
		BD_WARN(BD_LOG_LEVEL, "same listener registered multiple times.");
	}
#endif // BD_DEBUG

	m_listeners.pushBack(listener);
}

void bdConnection::unregisterListener(bdConnectionListener *const listener)
{
	m_listeners.removeAllKeepOrder(listener);
}

bdBool bdConnection::connect()
{
	m_status = BD_CONNECTING;

	return true;
}

void bdConnection::disconnect()
{
	m_status = BD_DISCONNECTING;
}

void bdConnection::close()
{
	m_status = BD_DISCONNECTED;
}

bdConnection::bdConnection()
: m_maxTransmissionRate(0),
  m_status(BD_NOT_CONNECTED)
{
}

bdConnection::bdConnection(bdCommonAddrRef addr)
: m_addr(addr),
  m_maxTransmissionRate(0),
  m_status(BD_NOT_CONNECTED)
{
}
