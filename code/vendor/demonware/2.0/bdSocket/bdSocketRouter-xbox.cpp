// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdSocketRouter.h>
#include <bdSocket/bdAddressMap.h>

#define BD_LOG_LEVEL "bdSocket/socket"

bdSocketRouter::bdSocketRouter()
: m_socket(BD_NULL),
  m_status(BD_SOCKET_ROUTER_UNINITIALIZED)
{
}

bdSocketRouter::~bdSocketRouter()
{
}

bdBool bdSocketRouter::init(bdSocket *socket,
							bdCommonAddrRef localCommonAddr,
							const bdSocketRouterConfig &config)
{
	bdBool result = true;
	
	if(BD_SOCKET_ROUTER_UNINITIALIZED != m_status)
	{
		result = false;
		BD_WARN(BD_LOG_LEVEL, "init() called multiple times.");
	}
	else if(BD_NULL == socket)
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "A valid socket pointer is required by this class");
	}
	else if (localCommonAddr.isNull())
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "A valid local common addr ref is required by this class");
	}
	else
	{
		m_socket = socket;
		m_localCommonAddr = localCommonAddr;
		m_addrMap.setupLocalCommonAddr(m_localCommonAddr);
		m_status = BD_SOCKET_ROUTER_INITIALIZED;
		m_config = config;
	}

	return result;
}

bdBool bdSocketRouter::quit()
{
	bdBool result = true;

	if(BD_SOCKET_ROUTER_UNINITIALIZED == m_status)
	{
		result = false;
		BD_WARN(BD_LOG_LEVEL, "quit() called multiple times.");
	}
	else
	{
		m_socket = BD_NULL;
		m_status = BD_SOCKET_ROUTER_UNINITIALIZED;
	}

	return result;
}

bdBool bdSocketRouter::connect(bdAddrHandleRef &addrHandle)
{
	return XNetConnect(addrHandle->m_addr) == 0;
}

bdBool bdSocketRouter::disconnect(bdAddrHandleRef &addrHandle, bdBool allowDisconnectBroadcast)
{
	return m_addrMap.unregisterAddr(addrHandle);
}

bdDTLSAssociationStatus bdSocketRouter::getStatus(const bdAddrHandleRef addrHandle)
{
	bdDTLSAssociationStatus status = BD_SOCKET_IDLE;

	switch(XNetGetConnectStatus(addrHandle->m_addr))
	{
		case XNET_CONNECT_STATUS_IDLE:
			status = BD_SOCKET_IDLE;
			break;
		case XNET_CONNECT_STATUS_PENDING:
			status = BD_SOCKET_PENDING;
			break;
		case XNET_CONNECT_STATUS_CONNECTED:
			status = BD_SOCKET_CONNECTED;
			break;
		case XNET_CONNECT_STATUS_LOST:
			status = BD_SOCKET_LOST;
			break;
	}

	return status;
}

bdSocketRouter::bdSocketRouterStatus bdSocketRouter::getStatus()
{
	return m_status;
}

bdInt bdSocketRouter::sendTo(const bdAddrHandleRef addrHandle, 
							 void *data, 
							 const bdUInt length)
{
	// TODO consider adding info to the socket router config to allow 
	// dynamic setting for this [little endian, big endian|network order, platform order]

	bdAddr address(bdInetAddr(addrHandle->m_addr.s_addr), addrHandle->m_port);

	// The first 2 bytes of the data buffer indicate the amount of the buffer
	// that contains encrypted data, the remaining data is considered to be 
	// unencrypted.
	// This data is written into the buffer using bdBytePacker::appendBasicType()
	// however on XBox 360 this is not the expected order, so we must repack the 
	// data before passing to the socket.

	// Read out the little endian size from data
	const bdUInt16 sizeDesc = *(reinterpret_cast<bdUInt16*>(data));
	bdUInt16 swappedSizeDesc = 0;
	// We can use endian swap on big endian platforms, on little 
	// endian platforms it does nothing.
	bdBitBuffer::endianSwap(sizeDesc, swappedSizeDesc);

	bdMemcpy(data, &swappedSizeDesc , sizeof(bdUInt16));
	const bdInt sendResult = m_socket->sendTo(address, data, length);

	// Reset the size data to it's original state.
	bdMemcpy(data, &sizeDesc , sizeof(bdUInt16));

	return sendResult;

}

bdInt bdSocketRouter::receiveFrom(bdAddrHandleRef &addrHandle, 
								  void *data,
								  const bdUInt size,
								  bool& didErrorOccur)
{
	bdAddr realAddr;
	bdInt val = m_socket->receiveFrom(realAddr, data, size);

	// The first 2 bytes of the data buffer indicate the amount of the buffer
	// that contains encrypted data, the remaining data is considered to be 
	// unencrypted.
	// This data will be read from the buffer using bdBytePacker::removeBasicType()
	// however on XBox 360 this is not the order, so we must repack the 
	// data before passing up to the calling code.

	// Read out the big endian size from data
	bdUInt16 *sizeDesc = reinterpret_cast<bdUInt16*>(data);
	bdUInt16 swappedSizeDesc = 0;
	// We can use endian swap on big endian platforms, on little 
	// endian platforms it does nothing.
	bdBitBuffer::endianSwap(*sizeDesc, swappedSizeDesc);

	bdMemcpy(data, &swappedSizeDesc , sizeof(swappedSizeDesc));

	if(realAddr.getAddress().isValid())
	{
		XNADDR xnAddr;
		bdSecurityID id;
		IN_ADDR xrealAddr;
		xrealAddr.s_addr = realAddr.getAddress().toUInt32();
		if( XNetInAddrToXnAddr(xrealAddr, &xnAddr, &id) == 0 )
		{
			bdCommonAddrRef ca = new bdCommonAddr(m_localCommonAddr, xnAddr, m_localCommonAddr->getPort() );
			m_addrMap.getAddrHandle( ca, id, addrHandle );
			addrHandle->setRealAddr( xrealAddr, realAddr.getPort() );

			didErrorOccur = false;
		}
		else
		{
			didErrorOccur = true;
			
			bdBool found = m_addrMap.findAddrHandle( realAddr, addrHandle );
			if (!found)
			{
				val = BD_NET_ERROR;
			}
			
		}
	}
	else
	{
		didErrorOccur = true;
	}

	return val;
}

void bdSocketRouter::pump()
{
	m_qosProber.pump();
}

bdQoSProbe* bdSocketRouter::getQoSProber()
{
	return &m_qosProber;
}

bdAddressMap* bdSocketRouter::getAddressMap()
{
	return &m_addrMap;
}

bdSecurityKeyMap* bdSocketRouter::getKeyMap()
{
	return &m_keyStore;
}

void bdSocketRouter::getConfig(bdSocketRouterConfig &config) const
{
	config = m_config;
}

bdCommonAddrRef bdSocketRouter::getLocalCommonAddr() const
{
	return m_localCommonAddr;
}

bdBool bdSocketRouter::isRelayEnabled() const
{
	return false;
}

