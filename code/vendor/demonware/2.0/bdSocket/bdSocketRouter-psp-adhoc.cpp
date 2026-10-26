// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdSocketRouter.h>
#include <bdSocket/bdAddressMap.h>

#include <bdCore/bdUtilities/bdBytePacker.h>

#if defined BD_SOCKET_DO_BROADCAST

#include <kernel.h>
#include <pspnet.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>

#define BD_SOCKET_MAX_BROADCAST_PEERS			(8u)
#define BD_SOCKET_MAX_BROADCAST_PACKET_SIZE		(100u)

#define BD_SOCKET_BROADCAST_SIGNAL_DATA_SIZE	(BD_IN_ADDR_SIZE + 1)

#define BD_SOCKET_BROADCAST_BUFFER_SIZE			((1 + BD_SOCKET_MAX_BROADCAST_PEERS) * \
	(BD_SOCKET_MAX_BROADCAST_PACKET_SIZE + BD_SOCKET_BROADCAST_SIGNAL_DATA_SIZE))

#endif // BD_SOCKET_DO_BROADCAST

#define BD_LOG_LEVEL							("bdSocket/socket")



bdSocketRouter::bdSocketRouter()
: m_socket(BD_NULL),
  m_status(BD_SOCKET_ROUTER_UNINITIALIZED)
{
}

bdSocketRouter::~bdSocketRouter()
{
}

bdBool bdSocketRouter::init(bdDefaultSocket *socket,
							const bdCommonAddrRef localCommonAddr,
							const bdSocketRouterConfig &config)
{
	BD_ASSERT(socket != BD_NULL, "Invalid socket");
	bdBool result = true;
	if(BD_SOCKET_ROUTER_UNINITIALIZED != m_status)
	{
		result = false;
		BD_WARN(BD_LOG_LEVEL, "init() called multiple times.");
	}
	else
	{
		m_socket = socket;
		m_localCommonAddr = localCommonAddr;
		SceNetEtherAddr macAddr;
		if(0 != sceNetGetLocalEtherAddr(&macAddr))
		{
			BD_ASSERT(false, "Failed to get local addr.");
			result = false;
		}
		else if(true == result)
		{
			m_config = config;
			m_status = BD_SOCKET_ROUTER_INITIALIZED;

#if defined BD_SOCKET_DO_BROADCAST
			m_buffer = bdAllocate<bdUByte8>(BD_SOCKET_BROADCAST_BUFFER_SIZE);
			m_bufferOffset = 1;
			m_destPort = 0;
			m_localAddr = bdInetAddr(macAddr.data);
#endif // BD_SOCKET_DO_BROADCAST
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Socket Router initialization failed.");
			m_status = BD_SOCKET_ROUTER_ERROR;
		}
	}
	return result;
}

bdBool bdSocketRouter::quit()
{
	bdBool result = false;

	if(BD_SOCKET_ROUTER_UNINITIALIZED == m_status)
	{
		result = false;
		BD_WARN(BD_LOG_LEVEL, "quit() called multiple times.");
	}
	else
	{

		// clear pointers
		m_socket = BD_NULL;

		m_status = BD_SOCKET_ROUTER_UNINITIALIZED;
#if defined BD_SOCKET_DO_BROADCAST
		bdDeallocate(m_buffer);
		m_buffer = BD_NULL;
#endif

	}

	return result;
}


bdBool bdSocketRouter::connect(bdAddrHandleRef &addr)
{
	return true;
}

bdBool bdSocketRouter::disconnect(bdAddrHandleRef &addrHandle)
{
	bdBool success = false;
	if(BD_SOCKET_ROUTER_INITIALIZED != m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot disconnect unless class is initialized");
	}
	else if(addrHandle.isNull())
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot disconnect using null addr handle ref");
	}
	else
	{
		success = m_addrMap.unregisterAddr(addrHandle);
	}

	return success;
}

bdDTLSAssociationStatus bdSocketRouter::getStatus(const bdAddrHandleRef addr) const
{
	bdDTLSAssociationStatus status = BD_SOCKET_CONNECTED;

	return status;
}

bdSocketRouter::bdSocketRouterStatus bdSocketRouter::getStatus() const
{
	return m_status;
}

bdInt bdSocketRouter::sendTo(const bdAddrHandleRef addr,
							 void *data,
							 const bdUInt length)
{
#if !defined BD_SOCKET_DO_BROADCAST
	return m_socket->sendTo(addr, data, length);
#else

	if (m_destPort == 0)
	{
		m_destPort = addr.getPort();
	}

	bdBool sendSingle = false;

	if (m_destPort != addr.getPort())
	{
		BD_WARN(BD_LOG_LEVEL,
			"Asked to send to port %u, but multicasts are sent to %u.", addr.getPort(), m_destPort);
		sendSingle = true;
	}

	if (length >= BD_SOCKET_MAX_BROADCAST_PACKET_SIZE ||
		(BD_SOCKET_BROADCAST_BUFFER_SIZE - m_bufferOffset - BD_SOCKET_BROADCAST_SIGNAL_DATA_SIZE) < length)
	{
		BD_INFO(BD_LOG_LEVEL, "Packet too big to send multicast.");
		sendSingle = true;
	}

	if (!sendSingle)
	{
		bdBool status = true;
		status = status && bdBytePacker::appendBasicType(m_buffer,
			BD_SOCKET_BROADCAST_BUFFER_SIZE,
			m_bufferOffset,
			m_bufferOffset,
			static_cast<bdUByte8>(length));
		const bdUInt oldOffset = m_bufferOffset;
		status = status && addr.getAddress().serialize(m_buffer,
			BD_SOCKET_BROADCAST_BUFFER_SIZE,
			m_bufferOffset,
			m_bufferOffset);

		BD_ASSERT(m_bufferOffset - oldOffset == BD_IN_ADDR_SIZE,
			"InetAddr serialized into unexpected no. of bytes.");

		status = status && bdBytePacker::appendBuffer(m_buffer,
			BD_SOCKET_BROADCAST_BUFFER_SIZE,
			m_bufferOffset,
			m_bufferOffset,
			reinterpret_cast<const bdUByte8*>(data),
			length);

		BD_ASSERT(status, "Failed to add a packet to the buffer.");
		return length;
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "Sending packet of size %u with unicast.", length);

		BD_ASSERT(length+1 <= BD_MAX_DATAGRAM_SIZE, "Datagram too big.");
		bdUByte8 tmpBuf[BD_MAX_DATAGRAM_SIZE];
		tmpBuf[0] = 0;
		bdMemcpy(tmpBuf+1, data, length);
		return m_socket->sendTo(addr, tmpBuf, length + 1);
	}
#endif
}

bdInt bdSocketRouter::receiveFrom(bdAddrHandleRef &addr,
								  void *data,
								  const bdUInt size)
{
#if !defined BD_SOCKET_DO_BROADCAST
	bdAddr realAddr;
	bdInt val = m_socket->receiveFrom(addr, data, size);
	return val;
#else

	bdUByte8 tmpBuffer[BD_MAX_DATAGRAM_SIZE];

	bdInt val;

	do {
		val = m_socket->receiveFrom(addr,  tmpBuffer, BD_MAX_DATAGRAM_SIZE);

		if (val > 0)
		{
			if (tmpBuffer[0] == 0)
			{
				// Single packet
				if (val - 1 <= size)
				{
					bdMemcpy(data, tmpBuffer + 1, val - 1);
					return val - 1;
				}
				else
				{
					return BD_NET_MSG_SIZE;
				}
			}
			else
			{
				// Aggregate packet

				bdBool status = true;
				bdUInt offset = 1;
				while (offset < val && status)
				{
					bdUByte8 size = 0;
					bdInetAddr tmpAddr;

					status = status && bdBytePacker::removeBasicType(tmpBuffer, val, offset, offset, size);
					status = status && tmpAddr.deserialize(tmpBuffer, val, offset, offset);

					if (status && m_localAddr == tmpAddr)
					{
						bdMemcpy(data, tmpBuffer + offset, size);
						return size;
					}
					offset += size;
				}
			}
		}
		else
		{
			return val;
		}
	} while (val > 0);

	return val;

#endif
}

void bdSocketRouter::pump()
{
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

#if defined BD_SOCKET_DO_BROADCAST

bdInt bdSocketRouter::flushBuffer()
{
	if (m_bufferOffset == 1)
	{
		//BD_INFO(BD_LOG_LEVEL, "Nothing to send.");
		return 0;
	}

	m_buffer[0] = 1;
	bdInt result = m_socket->sendTo(bdAddr(bdInetAddr(BD_IN_ADDR_BROADCAST), m_destPort),
		m_buffer,
		m_bufferOffset);
	m_bufferOffset = 1;
	return result;
}

#endif
