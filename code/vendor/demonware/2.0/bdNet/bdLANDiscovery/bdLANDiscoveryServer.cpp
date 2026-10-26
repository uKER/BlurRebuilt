// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PSP-INFRA XENON XBOX PS3 UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdLANDiscovery/bdLANDiscoveryServer.h>
#include <bdConnection/bdMessage/bdMessageTypes.h>
#include <bdNet/bdLANDiscovery/bdGameInfo.h>
#include <bdNet/bdNet.h>
#include <bdNet/bdLANDiscovery/bdLANDiscoveryListener.h>

#define BD_LOG_LEVEL	"bdNet/discovery server"

bdLANDiscoveryServer::bdLANDiscoveryServer()
: m_status(BD_IDLE)
{
}

bdLANDiscoveryServer::~bdLANDiscoveryServer()
{
	stop();
}

bdBool bdLANDiscoveryServer::start(const bdGameInfoRef gameInfo,
								const bdInetAddr& localAddr,
								const bdPort discoveryPort)
{
	bdBool ok = true;
	const bdBool blocking = false;
	if(m_socket.create(blocking))
	{
		const bdAddr bindAddr(localAddr, discoveryPort);
		if(BD_NET_SUCCESS == m_socket.bind(bindAddr))
		{
			m_status = BD_PENDING;
			m_gameInfo = gameInfo;
		}
		else
		{
			m_status = BD_ERROR;
			ok = false;
		}
	}
	else
	{
		m_status = BD_ERROR;
		ok = false;
	}
	return ok;
}

void bdLANDiscoveryServer::update()
{
	if(m_status == BD_PENDING)
	{
		bdAddr from;
		bdUByte8 data[BD_MAX_DATAGRAM_SIZE];

		const bdInt32 received = m_socket.receiveFrom(from, data, BD_MAX_DATAGRAM_SIZE);

		if(received > BD_MIN_DISCOVERY_SIZE)
		{
			bdBitBuffer payload(data, received << 3, true);
			bdMessageType type;

			// read out the message's type
			bdBool ok;
			ok = payload.readUByte8(type);
			if (ok && (type == BD_DISCOVERY_BROADCAST))
			{
				// read out the nonce
				bdUByte8 tempNonce[BD_NONCE_SIZE];
				ok = ok && payload.readFull(tempNonce);

				// read out the game's title id
				bdUInt32 titleID = 0;
				ok = ok && payload.readUInt32(titleID);

				// Does the title ID match our own?
				if (ok && (titleID == m_gameInfo->getTitleID()))
				{
					bdBitBuffer discoveryReply;
					discoveryReply.writeUByte8(BD_DISCOVERY_REPLY);
					discoveryReply.writeFull(tempNonce);
                    m_gameInfo->serialize(discoveryReply);

#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON

					bdNChar8 buf2[BD_ADDR_STRING_SIZE];
					from.toString(buf2, sizeof(buf2));
					BD_INFO(BD_LOG_LEVEL, "Received discovery request from : %s", buf2);

					if(true == from.getAddress().isLoopback())
					{
						// do nothing.
					}
					else
					{
						// xbox must reply to the broadcast address...
						bdAddr bcAddr(bdInetAddr::Broadcast(), from.getPort());
						from = bcAddr;
					}
#endif
					const bdInt sendResult = m_socket.sendTo(from , discoveryReply.getData(), discoveryReply.getDataSize());
					if(sendResult > 0)
					{
						bdNChar8 buf[BD_ADDR_STRING_SIZE];
						from.toString(buf, sizeof(buf));
						BD_INFO(BD_LOG_LEVEL, "Sent discovery reply to: %s", buf);

						for (bdUInt i=0; i < m_listeners.getSize(); i++)
						{
							m_listeners[i]->onRequest();
						}
					}
				}
			}
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Not initialized.");
	}
}

void  bdLANDiscoveryServer::stop()
{
	m_socket.close();
	m_status = BD_IDLE;
}

void bdLANDiscoveryServer::registerListener(bdLANDiscoveryListener *listener)
{
	m_listeners.pushBack(listener);
}

void bdLANDiscoveryServer::unregisterListener(bdLANDiscoveryListener *listener)
{
	m_listeners.removeAll(listener);
}

bdLANDiscoveryServer::bdStatus bdLANDiscoveryServer::getStatus() const
{
	return m_status;
}

bdGameInfoRef bdLANDiscoveryServer::getGameInfo()
{
	return m_gameInfo;
}


