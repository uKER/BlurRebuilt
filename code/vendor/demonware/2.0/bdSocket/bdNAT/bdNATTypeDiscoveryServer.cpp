// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdNATTypeDiscoveryServer.h>

#include <bdSocket/bdNAT/bdNAT.h>
#include <bdSocket/bdNAT/bdPacket/bdNATTypeDiscoveryPacket.h>
#include <bdSocket/bdNAT/bdPacket/bdNATTypeDiscoveryPacketReply.h>

#define BD_LOG_LEVEL "bdSocket/nat"

bdNATTypeDiscoveryServer::bdNATTypeDiscoveryServer()
: m_primarySocket(BD_NULL)
{
}

bdNATTypeDiscoveryServer::~bdNATTypeDiscoveryServer()
{
	m_primarySocket = BD_NULL;
}

bdBool bdNATTypeDiscoveryServer::init(	bdDefaultSocket *primarySocket,
										const bdAddr &primaryAddr,
										const bdAddr &secondaryAddr)
{
	BD_INFO(BD_LOG_LEVEL, "Initialising NAT type discovery server.");

	bdBool result = true;

	m_primarySocket = primarySocket;

	m_primaryAddr = primaryAddr;
	m_secondaryAddrPrimPort.set(secondaryAddr.getAddress(), primaryAddr.getPort());
	m_primaryAddrSecPort.set(m_primaryAddr.getAddress(), secondaryAddr.getPort());
	m_secondaryAddrSecPort.set(secondaryAddr.getAddress(), secondaryAddr.getPort());

	const bdBool blocking = true;
	if (!m_secondarySocketPrimPort.create(blocking)) 
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to create socket 2");
		result = false;
	}

	if (m_secondarySocketPrimPort.bind(m_secondaryAddrPrimPort) != BD_NET_SUCCESS)
	{
		bdNChar8 addrString[BD_ADDR_STRING_SIZE];
		m_secondaryAddrPrimPort.toString(addrString, BD_ADDR_STRING_SIZE);
		BD_WARN(BD_LOG_LEVEL, "Failed to bind to socket 2 with addr : %s" , addrString);
		result = false;
	}

	if (!m_primSocketSecPort.create(blocking))
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to create socket 3");
		result = false;
	}

	if (m_primSocketSecPort.bind(m_primaryAddrSecPort) != BD_NET_SUCCESS)
	{
		bdNChar8 addrString[BD_ADDR_STRING_SIZE];
		m_primaryAddrSecPort.toString(addrString, BD_ADDR_STRING_SIZE);
		BD_WARN(BD_LOG_LEVEL, "Failed to bind to socket 3 with addr : %s" , addrString);
		result = false;
	}

	if (!m_secSocketSecPort.create(blocking))
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to create socket 4");
		result = false;
	}

	if (m_secSocketSecPort.bind(m_secondaryAddrSecPort) != BD_NET_SUCCESS)
	{
		bdNChar8 addrString[BD_ADDR_STRING_SIZE];
		m_secondaryAddrSecPort.toString(addrString, BD_ADDR_STRING_SIZE);
		BD_WARN(BD_LOG_LEVEL, "Failed to bind to socket 4 with addr : %s" , addrString);
		result = false;
	}
	return result;
}

void bdNATTypeDiscoveryServer::pump()
{
	bdAddr addr;
	const bdUInt dataSize = BD_MAX_DATAGRAM_SIZE;
	bdUByte8 data[dataSize];

	bdBool receiving = true;

	while(receiving)
	{
		const bdInt received = m_secondarySocketPrimPort.receiveFrom(addr, data, dataSize);

		bdNATTypeDiscoveryPacket packet(bdNATTypeDiscoveryPacket::BD_NTDP_INVALID_TYPE);
		bdUInt tmpUInt;

		if (received > 0 && packet.deserialize(data, static_cast<bdUInt>(received), 0, tmpUInt))
		{
			acceptSerializedPacket(addr, packet);
		}
		else
		{
			switch(received)
			{
			case BD_NET_WOULD_BLOCK:
			case BD_NET_BLOCKING_CALL_CANCELED:
				receiving = false;
				break;
			case BD_NET_CONNECTION_RESET:
				break;
			case BD_NET_MSG_SIZE:
				break;
			case BD_NET_NOT_BOUND:
			default:
				receiving = false;
				BD_WARN(BD_LOG_LEVEL, "bdNATTypeDiscoveryServer::pump, net error.");
				break;
			}
		}
	}
}

void bdNATTypeDiscoveryServer::quit()
{
}

bdBool bdNATTypeDiscoveryServer::acceptPacket(bdDefaultSocket *,
											  bdAddr addr,
											  void *data,
											  const bdUInt size,
											  const bdUByte8)
{
	bdNATTypeDiscoveryPacket packet(bdNATTypeDiscoveryPacket::BD_NTDP_INVALID_TYPE);
	bdUInt tmpUInt;

	if (packet.deserialize(data, size, 0, tmpUInt) && tmpUInt == size)
	{
		return acceptSerializedPacket(addr, packet);
	}

	return false;
}

bdBool bdNATTypeDiscoveryServer::acceptSerializedPacket(bdAddr addr,
												 bdNATTypeDiscoveryPacket &packet)
{
	bdBool accept = false;

	if(packet.getType() == BD_NTDP_BINDING_REQUEST)
	{
		const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
		bdUByte8 buffer[bufferSize];
		bdUInt tmpUInt;

		accept = true;

		const bdUWord fromAddressSize = 100;
		bdNChar8 fromAddress[fromAddressSize];
		addr.toString(fromAddress, fromAddressSize);

		BD_INFO(BD_LOG_LEVEL, "Recvd Packet(%d) from %s", packet.getRequest(), fromAddress);

		switch(packet.getRequest())
		{
			case bdNATTypeDiscoveryPacket::BD_NTDP_SAME_ADDR:
			{
				bdNATTypeDiscoveryPacketReply reply(addr, m_secondaryAddrPrimPort, packet.getProtocolVersion());
				BD_ASSERT(reply.serialize(buffer, bufferSize, 0, tmpUInt), "Failed to serialize reply.");
				m_primarySocket->sendTo(addr, buffer, tmpUInt);
			}
			break;
			case bdNATTypeDiscoveryPacket::BD_NTDP_DIFF_PORT:
			{
				bdNATTypeDiscoveryPacketReply reply(addr, m_secondaryAddrPrimPort, packet.getProtocolVersion());
				BD_ASSERT(reply.serialize(buffer, bufferSize, 0, tmpUInt), "Failed to serialize reply.");
				m_primSocketSecPort.sendTo(addr, buffer, tmpUInt);
			}
			break;
			case bdNATTypeDiscoveryPacket::BD_NTDP_DIFF_IP:
			{
				bdNATTypeDiscoveryPacketReply reply(addr, m_secondaryAddrPrimPort, packet.getProtocolVersion());
				BD_ASSERT(reply.serialize(buffer, bufferSize, 0, tmpUInt), "Failed to serialize reply.");
				m_secondarySocketPrimPort.sendTo(addr, buffer, tmpUInt);
			}
			break;
			case bdNATTypeDiscoveryPacket::BD_NTDP_DIFF_PORT_IP:
			{
				bdNATTypeDiscoveryPacketReply reply(addr, m_secondaryAddrPrimPort, packet.getProtocolVersion());
				BD_ASSERT(reply.serialize(buffer, bufferSize, 0, tmpUInt), "Failed to serialize reply.");
				m_secSocketSecPort.sendTo(addr, buffer, tmpUInt);
			}
			break;
			case bdNATTypeDiscoveryPacket::BD_NTDP_INVALID_TYPE:
			{
				BD_ERR(BD_LOG_LEVEL, "Invalid packet passed to this function");
			}
			break;
		}
	}

	return accept;
}


