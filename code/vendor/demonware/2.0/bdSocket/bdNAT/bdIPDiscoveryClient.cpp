// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdIPDiscoveryClient.h>
#include <bdSocket/bdNAT/bdPacket/bdIPDiscoveryPacket.h>
#include <bdSocket/bdNAT/bdPacket/bdIPDiscoveryPacketReply.h>

#define BD_LOG_LEVEL "bdSocket/nat"

bdIPDiscoveryClient::bdIPDiscoveryClient()
: m_socket(BD_NULL),
  m_status(BD_IP_DISC_UNINITIALIZED),
  m_retries(0)
{
}

bdIPDiscoveryClient::~bdIPDiscoveryClient()
{
	m_status = BD_IP_DISC_UNINITIALIZED;
	m_socket = BD_NULL;
}

bdBool bdIPDiscoveryClient::init(	bdDefaultSocket *socket,
									const bdAddr &server,
									bdIPDiscoveryConfig config)
{
	bdBool result = false;
	if(BD_IP_DISC_UNINITIALIZED != m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "init() called multiple times.");
	}
	else if(BD_NULL == socket)
	{
		BD_ERR(BD_LOG_LEVEL, "A valid socket pointer is required by this class");
	}
	else
	{
		m_socket  = socket;
		m_serverAddr = server;
		m_retries = 0;

		// Sanity check config, and warn if insane but use anyway...
		config.sanityCheckConfig();
		m_config = config;

		m_status = BD_IP_DISC_RUNNING;

		const bdBool sendIPDiscPacketResult = sendIPDiscoveryPacket();
		if(true == sendIPDiscPacketResult)
		{
			result = true;
		}
	}
	return result;
}

const bdAddr& bdIPDiscoveryClient::getPublicAddress() const
{
	if( BD_IP_DISC_SUCCESS != m_status )
	{
		BD_ASSERT(false, "bdIPDiscoveryClient::getPublicAddress() called, but "
			"ip discovery was not successful (or not yet finished).");
	}

	return m_publicAddr;
}

void bdIPDiscoveryClient::pump(bdAddr fromAddr, bdUByte8 data[],	
								const bdInt dataSize )
{
	// Do all the packet receiving from the socket.
	if(BD_IP_DISC_RUNNING == m_status && dataSize > 0)
	{
		bdIPDiscoveryPacketReply packet;
		bdUInt tmpUInt;

		if( (fromAddr == m_serverAddr) &&
			packet.deserialize(data, static_cast<bdUInt>(dataSize), 0, tmpUInt))
		{
			m_publicAddr = packet.getAddr();
			m_status = BD_IP_DISC_SUCCESS;

			const bdUWord addrBufferSize = 100;
			bdNChar8 addrBuffer[addrBufferSize];
			m_publicAddr.toString(addrBuffer, addrBufferSize);
			BD_INFO(BD_LOG_LEVEL, "Public IP discovered: %s", addrBuffer);
		}
	}

	// Retry discovery as timer dictates.
	if(m_status == BD_IP_DISC_RUNNING)
	{
		if( (m_timer.getElapsedTimeInSeconds() > m_config.m_requestTimeout) &&
			(m_retries++ < m_config.m_retries))
		{
			BD_WARN(BD_LOG_LEVEL, "IP Discovery failed. Retrying. (%u/%u)", m_retries, m_config.m_retries);
			sendIPDiscoveryPacket();
		}
		else if(m_retries >= m_config.m_retries)
		{
			BD_ERR(BD_LOG_LEVEL, "IP Discovery failed with %u retries, giving up. ", m_config.m_retries);
			m_status = BD_IP_DISC_FAIL;
		}
	}
}

void bdIPDiscoveryClient::quit()
{
	m_socket = BD_NULL;
	m_serverAddr = bdAddr();
	m_publicAddr = bdAddr();
	m_timer.reset();
	m_status = BD_IP_DISC_UNINITIALIZED;
	m_retries = 0;
}

bdIPDiscoveryClient::bdIPDiscoveryClientStatus bdIPDiscoveryClient::getStatus() const
{
	return m_status;
}

bdBool bdIPDiscoveryClient::sendIPDiscoveryPacket()
{
	bdBool result = false;
	if(m_status == BD_IP_DISC_RUNNING)
	{
		m_timer.start();

		bdIPDiscoveryPacket packet;
		const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
		bdUByte8 buffer[bufferSize];
		bdUInt len = 0;

		const bdBool serializeResult = packet.serialize(buffer, bufferSize, 0, len);
		if(false == serializeResult)
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to serialize IP Discovery packet.");
			m_status = BD_IP_DISC_ERROR;
		}
		else
		{
			const bdInt sendResult = m_socket->sendTo(m_serverAddr, buffer, len);
			if((sendResult > 0) &&  (sendResult == static_cast<bdInt>(len)))
			{
				result = true;
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Failed to send IP discovery packet.");
				m_status = BD_IP_DISC_ERROR;
			}
		}
	}
	return result;
}
