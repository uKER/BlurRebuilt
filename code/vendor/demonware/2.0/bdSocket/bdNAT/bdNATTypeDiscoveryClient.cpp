// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdNATTypeDiscoveryClient.h>


/// We use a simplified version of the RFC 3489 NAT type discovery.
///
///                     +--------+
///                     |  Test  |
///                     |   I    |
///                     +--------+
///                          |
///                          |
///                          V
///                         /\              /\      .
///                      N /  \ Y          /  \     .
///       UDP     <-------/Resp\--------->/SKIP\    .
///       Blocked         \ ?  /          \TEST/
///                        \  /            \  /
///                         \/              \/
///                                          |
///                                          |
///                                          V
///                                      +--------+
///                                      |  Test  |
///                                      |   II   |
///                                      +--------+
///                                          |
///                                          V
///               /\                         /\      .
/// MODERATE  N  /  \       +--------+   N  /  \     .
///   NAT  <--- / IP \<-----|  Test  |<--- /Resp\    .
///             \Same/      |   III  |     \ ?  /
///              \? /       +--------+      \  /
///               \/                         \/
///               |                           |Y
///               |                           |
///               |                           V
///               |                           OPEN
///               |                           NAT
///               V
///           STRICT NAT
///
///
///

/// \todo Remove this once BD_NAT_UNKNOWN is added to bdNATType
#define BD_NAT_UNKNOWN BD_NAT_STRICT

#include <bdSocket/bdNAT/bdNAT.h>
#include <bdSocket/bdNAT/bdPacket/bdNATTypeDiscoveryPacketReply.h>


#define BD_NAT_DISC_INIT_MAX_ITERATIONS (100u)

#define BD_LOG_LEVEL	"bdSocket/nat"

bdNATTypeDiscoveryClient::bdNATTypeDiscoveryClient()
: m_NATType(BD_NAT_UNKNOWN),
  m_state(BD_NTDCS_UNINITIALIZED),
  m_resends(0)
{
}

bdNATTypeDiscoveryClient::~bdNATTypeDiscoveryClient()
{
}

bdBool bdNATTypeDiscoveryClient::init(	bdDefaultSocket *socket,
										const bdAddr &serverAddr,
										bdNATTypeDiscoveryConfig config)
{
	bdBool result = false;
	if(BD_NTDCS_UNINITIALIZED == m_state)
	{
		if(socket != BD_NULL )
		{
			m_socket = socket;	

			BD_INFO(BD_LOG_LEVEL, "NAT discovery client initialized");

			m_serverAddr1 = serverAddr;
			// Setup the config info.
			m_config = config;
			// Run a sanity check on the config info.
			m_config.sanityCheckConfig();

			result = true;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Must initialize NAT Discovery Client with a valid socket");
			m_state = BD_NTDCS_ERROR;
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot initialize already initialized class.");
	}

	if(true == result)
	{
		const bdBool sendResult = sendForTest1();
		if(true == sendResult)
		{
			m_state = BD_NTDCS_RUN_TEST_1;
			m_timer.start();
			result = true;
		}
		else
		{
			m_state = BD_NTDCS_ERROR;
		}
	}
	return result;
}

void bdNATTypeDiscoveryClient::pump(bdAddr fromAddr, bdUByte8 data[],	
								const bdInt dataSize)
{
	// It only makes sense to pump this class when NAT type detection is
	// running
	if(	true == isRunning())
	{
		receiveReplies(fromAddr, data, dataSize);
		pumpActiveTest();
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "This class does not need to be pumped unless NAT type detection is running" );
	}
}

bdBool bdNATTypeDiscoveryClient::quit()
{
	bdBool result = false;
	if(BD_NTDCS_UNINITIALIZED != m_state)
	{
		BD_INFO(BD_LOG_LEVEL, "NAT discovery client quit");
		m_socket = BD_NULL;
		result = true;
		m_state = BD_NTDCS_UNINITIALIZED;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot quit uninitialized class.");
	}
	return result;
}

bdBool bdNATTypeDiscoveryClient::isRunning() const
{
	bdBool result = false;
	if(	BD_NTDCS_RUN_TEST_1 == m_state ||
		BD_NTDCS_RUN_TEST_2 == m_state ||
		BD_NTDCS_RUN_TEST_3 == m_state )
	{
		result = true;
	}
	return result;
}

bdNATType bdNATTypeDiscoveryClient::getNATType() const
{
	bdNATType result = BD_NAT_UNKNOWN;
	if(BD_NTDCS_FINI == m_state)
	{
		result = m_NATType;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Can only return a meaningful result once NAT type detection is complete" );
	}
	return result;
}

bdNATTypeDiscoveryClient::bdNATTypeDiscoveryClientState bdNATTypeDiscoveryClient::getStatus() const
{
	return m_state;
}


// protected:
bdBool bdNATTypeDiscoveryClient::sendForTest1()
{
	bdBool result = sendNATTypeDiscoveryPacket(	bdNATTypeDiscoveryPacket::BD_NTDP_SAME_ADDR ,
												m_serverAddr1);
	if(true == result)
	{
		BD_INFO(BD_LOG_LEVEL, "Sent packet for NTDC test 1");
	}

	return result;
}

bdBool bdNATTypeDiscoveryClient::sendForTest2()
{
	bdBool result = sendNATTypeDiscoveryPacket(	bdNATTypeDiscoveryPacket::BD_NTDP_DIFF_PORT_IP ,
														m_serverAddr1);
	if(true == result)
	{
		BD_INFO(BD_LOG_LEVEL, "Sent packet for NTDC test 2");
	}
	return result;
}

bdBool bdNATTypeDiscoveryClient::sendForTest3()
{
	bdBool result = sendNATTypeDiscoveryPacket(	bdNATTypeDiscoveryPacket::BD_NTDP_DIFF_IP ,
												m_serverAddr2);
	if(true == result)
	{
		BD_INFO(BD_LOG_LEVEL, "Sent packet for NTDC test 3");
	}
	return result;
}

void bdNATTypeDiscoveryClient::handleResponse(const bdAddr &addr,
											  const bdNATTypeDiscoveryPacketReply &reply)
{
	switch(m_state)
	{
		case BD_NTDCS_RUN_TEST_1:
		{
			m_serverAddr2 = reply.getSecAddr();
			m_mappedAddr = reply.getMappedAddr();

			BD_INFO(BD_LOG_LEVEL, "Reply for test 1. Start test 2.");

			const bdBool sendResult = sendForTest2();
			if(false == sendResult)
			{
				m_state = BD_NTDCS_ERROR;
			}
			else
			{
				m_state = BD_NTDCS_RUN_TEST_2;
				m_resends = 0;
			}
		}
		break;
		case BD_NTDCS_RUN_TEST_2:
		{
			// Ensure reply is from secondary addr
			const bdBool addressMatch = (addr.getAddress() == m_serverAddr2.getAddress());

			//Ensure reply is NOT from primary port.
			const bdBool portMatch = (addr.getPort() == m_serverAddr2.getPort());
			if((true == addressMatch)
				&& (false == portMatch))
			{
				// response to test 2 means we are behind an OPEN NAT (or no NAT)

				BD_INFO(BD_LOG_LEVEL, "Reply for test 2. Open NAT (or no NAT).");

				m_state = BD_NTDCS_FINI;
				m_NATType = BD_NAT_OPEN;
			}
			else
			{
				if(false == addressMatch)
				{
					bdNChar8 receivedAddrString[BD_ADDR_STRING_SIZE];
					addr.toString(receivedAddrString, BD_ADDR_STRING_SIZE);
					bdNChar8 expectedAddrString[BD_ADDR_STRING_SIZE];
					m_serverAddr2.toString(expectedAddrString, BD_ADDR_STRING_SIZE);
					BD_WARN(BD_LOG_LEVEL, "Received test 2 response from unexpected address \n received from : %s \n expected from :%s ", receivedAddrString , expectedAddrString);
				}
				if(true == portMatch)
				{
					BD_WARN(BD_LOG_LEVEL, "Received test 2 response from primary port");
				}
			}
		}
		break;
		case BD_NTDCS_RUN_TEST_3:
		{
			// if the address the STUN server sees me as from test 1 is the same
			// as now:
			if(m_mappedAddr == reply.getMappedAddr())
			{
				BD_INFO(BD_LOG_LEVEL, "Reply for test 3. Moderate NAT.");
				m_state = BD_NTDCS_FINI;
				m_NATType = BD_NAT_MODERATE;
			}
			else
			{
				BD_INFO(BD_LOG_LEVEL, "Reply for test 3. Strict NAT.");
				m_state = BD_NTDCS_FINI;
				m_NATType = BD_NAT_STRICT;
			}
		}
		break;
		default:
		{
			// nothing to do
		}
		break;
	}
}

bdBool bdNATTypeDiscoveryClient::sendNATTypeDiscoveryPacket(bdNATTypeDiscoveryPacket::bdNATTypeDiscoveryPacketRequest packetType,
															const bdAddr &serverAddr)
{
	bdBool result = false;

	bdNATTypeDiscoveryPacket packet(packetType);
	const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
	bdUByte8 buffer[bufferSize];
	bdUInt len = 0;
	const bdBool serializeResult = packet.serialize(buffer, bufferSize, 0, len);
	if(true == serializeResult)
	{
		const bdInt lenDataSent = m_socket->sendTo(serverAddr, buffer, len);
		if(lenDataSent > 0 )
		{
			if(static_cast<bdUInt>(lenDataSent) == len)
			{
				result = true;
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Sent packet length does not match packet length !" );
			}
		}
		else
		{
			// TODO consider a more fine grained error analysis here...
			BD_ERR(BD_LOG_LEVEL, "Failed to send NTDC packet" );
		}
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "Failed to serialize NTDC packet.");
	}
	return result;
}

void bdNATTypeDiscoveryClient::receiveReplies(bdAddr fromAddr, bdUByte8 data[],	
												const bdInt dataSize)
{
	if (dataSize > 0)
	{
		// TODO we should only deserialize here if the type is correct...!
		// the normal DW pattern is to first check the type then decide whether
		// to deserialize or not..
		bdNATTypeDiscoveryPacketReply reply;
		bdUInt tmpUInt;
		const bdBool deserializeResult = reply.deserialize(data, static_cast<bdUInt>(dataSize), 0, tmpUInt);
		if(true == deserializeResult)
		{
			if(reply.getType() == BD_NTDP_BINDING_RESPONSE)
			{
				handleResponse(fromAddr, reply);
			}
		}
	}
}

void bdNATTypeDiscoveryClient::pumpActiveTest()
{
	// Send next test packet if elapsed time so dictates
	switch(m_state)
	{
		case BD_NTDCS_RUN_TEST_1:
		{
			if(m_timer.getElapsedTimeInSeconds() > m_config.m_NtdcsSendTimeout)
			{
				if(m_resends++ < m_config.m_NtdcsMaxResends)
				{
					m_timer.start();
					const bdBool sendResult = sendForTest1();
					if(false == sendResult)
					{
						m_state = BD_NTDCS_ERROR;
					}
				}
				else
				{
					BD_INFO(BD_LOG_LEVEL, "Test 1 failed. Not online.");
					m_state = BD_NTDCS_ERROR;
				}
			}
		}
		break;
		case BD_NTDCS_RUN_TEST_2:
		{
			if(m_timer.getElapsedTimeInSeconds() > m_config.m_NtdcsSendTimeout)
			{
				if(m_resends++ < m_config.m_NtdcsMaxResends)
				{
					m_timer.start();
					const bdBool sendResult = sendForTest2();
					if(false == sendResult)
					{
						m_state = BD_NTDCS_ERROR;
					}
				}
				else
				{
					BD_INFO(BD_LOG_LEVEL, "Test 2 failed.");
					m_resends = 0;
					m_state = BD_NTDCS_RUN_TEST_3;
				}
			}
		}
		break;
		case BD_NTDCS_RUN_TEST_3:
		{
			if(m_timer.getElapsedTimeInSeconds() > m_config.m_NtdcsSendTimeout)
			{
				if(m_resends++ < m_config.m_NtdcsMaxResends)
				{
					m_timer.start();
					const bdBool sendResult = sendForTest3();
					if(false == sendResult)
					{
						m_state = BD_NTDCS_ERROR;
					}
				}
				else
				{
					BD_INFO(BD_LOG_LEVEL, "Test 3 failed. Strict NAT.");
					m_NATType = BD_NAT_STRICT;
					m_state = BD_NTDCS_FINI;
				}
			}
		}
		break;
		case BD_NTDCS_FINI:
		case BD_NTDCS_ERROR:
		{
			// nothing to do...
		}
		break;
		case BD_NTDCS_UNINITIALIZED:
		{
			// It should not be possible for this state to be encountered .
			BD_WARN(BD_LOG_LEVEL, "Code logic error in NTDC pump");
		}
		break;
	}

}

