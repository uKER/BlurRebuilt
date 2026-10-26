// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON UNIX OSX IPHONE PS2 PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdLANDiscovery/bdLANDiscoveryClient.h>
#include <bdNet/bdNet.h>
#include <bdNet/bdLANDiscovery/bdGameInfoFactory.h>
#include <bdConnection/bdMessage/bdMessageTypes.h>
#include <bdNet/bdLANDiscovery/bdLANDiscoveryListener.h>


#define BD_LOG_LEVEL	"bdNet/discovery"

bdLANDiscoveryClient::bdLANDiscoveryClient()
: m_timeout(0),
m_status(BD_IDLE)
{
}

bdLANDiscoveryClient::~bdLANDiscoveryClient()
{
	m_socket.close();
}

bdBool bdLANDiscoveryClient::discover(const bdUInt titleID,
								   const bdFloat32 timeout,
								   const bdInetAddr& addr,
								   const bdPort discoveryPort)
{
	bdBool result = false;
	if( m_status != BD_PENDING)
	{
		const bdBool blocking = false;
		const bdBool ok = m_socket.create(blocking);
		if (ok)
		{
			bdTrulyRandom::getInstance()->getRandomUByte8(m_nonce, BD_NONCE_SIZE);
			m_timeout = timeout;
			const bdBitBufferRef payload = new bdBitBuffer;
			// Write the message's type
			payload->writeUByte8(BD_DISCOVERY_BROADCAST);
			// Write the nonce
			payload->writeFull(m_nonce);
			// Write the game's title ID
			payload->writeUInt32(titleID);

			const bdAddr bcAddr(addr, discoveryPort);
			// Send it to a broadcast address
			bdInt success = m_socket.sendTo(bcAddr, payload->getData(), payload->getDataSize());
			if(success < 0)
			{
				m_status = BD_ERROR;
				BD_ERR(BD_LOG_LEVEL, "bdLANDiscoveryClient::discover, Socket sendTo failed");
			}
			else
			{
				m_timer.start();
				m_status = BD_PENDING;
				result = true;
				BD_INFO(BD_LOG_LEVEL, "Starting: will run for %f seconds.", timeout);
			}
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "bdLANDiscoveryClient::discover, Could not create socket");
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "bdLANDiscoveryClient::discover, LAN discovery already running");
	}
	return result;
}

void bdLANDiscoveryClient::update()
{
	if (m_status == BD_PENDING)
	{
		bdAddr from;
		bdUByte8 data[BD_MAX_DISCOVERY_RESPONSE_SIZE];

		const bdInt32 received = m_socket.receiveFrom(from, data, BD_MAX_DISCOVERY_RESPONSE_SIZE);

		if(	received > BD_MIN_DISCOVERY_SIZE)
		{
			bdBitBuffer payload(data, received << 3, true);

			bdMessageType type = 0;

			// read out the message's type
			bdBool ok = payload.readUByte8(type);
			if (ok && (type == BD_DISCOVERY_REPLY))
			{

				bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
				from.toString(addrStr, sizeof(addrStr));
				BD_INFO(BD_LOG_LEVEL, "Reply received from %s " , addrStr);

				// read out the nonce
				bdUByte8 tempNonce[BD_NONCE_SIZE];
				ok = payload.readFull(tempNonce);
				if (ok && bdMemcmp(m_nonce, tempNonce, BD_NONCE_SIZE) == 0)
				{
					bdGameInfoRef gameFound = bdGameInfoFactory::getInstance()->create();
					ok = gameFound->deserialize(bdNet::getInstance()->getLocalCommonAddr(), payload);

					BD_INFO(BD_LOG_LEVEL, "Reply is relevant to client , firing listeners" );
					fireOnDiscoveryListeners(gameFound);
				}
				else
				{
					BD_INFO(BD_LOG_LEVEL, "Reply is not relevant to client , ignoring" );
				}
			}
			else if(ok)
			{
				BD_WARN(BD_LOG_LEVEL, "Received message is not a discovery reply" );
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Malformed message received" );
			}
		}

		if (m_timer.getElapsedTimeInSeconds() > m_timeout)
		{
			for (bdUInt i = 0; i < m_listeners.getSize(); i++)
			{
				m_listeners[i]->onDiscoveryFinished();
			}
			m_status = BD_IDLE;
			m_socket.close();
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "bdLANDiscoveryClient::update, Called while client was not running");
	}
}

bdLANDiscoveryClient::bdStatus bdLANDiscoveryClient::getStatus() const
{
	return m_status;
}

void bdLANDiscoveryClient::registerListener(bdLANDiscoveryListener *listener)
{
	m_listeners.pushBack(listener);
}

void bdLANDiscoveryClient::unregisterListener(bdLANDiscoveryListener *listener)
{
	m_listeners.removeAll(listener);
}

void bdLANDiscoveryClient::fireOnDiscoveryListeners(bdGameInfoRef gameInfo) const
{
	for (bdUInt i = 0; i < m_listeners.getSize(); i++)
	{
		m_listeners[i]->onDiscovery(gameInfo);
	}
}

