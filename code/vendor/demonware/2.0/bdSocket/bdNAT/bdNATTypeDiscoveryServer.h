// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_NAT_TYPE_DISCOVERY_SERVER_H
#define BD_NAT_TYPE_DISCOVERY_SERVER_H

#include <bdCore/bdCore.h>

#include <bdSocket/bdPacketInterceptor.h>

class bdNATTypeDiscoveryPacket;

class bdNATTypeDiscoveryServer  : public bdPacketInterceptor
{

	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdNATTypeDiscoveryServer();

		/// Virtual destructor
		virtual ~bdNATTypeDiscoveryServer();

		bdBool init(bdDefaultSocket *primarySocket,
					const bdAddr &primaryAddr,
					const bdAddr &secondaryAddr);

		void pump();

		void quit();

		virtual bdBool acceptPacket(bdDefaultSocket *socket, 
									bdAddr addr, 
									void *data, 
									const bdUInt size,
									const bdUByte8 type);


	protected:

		bdBool acceptSerializedPacket(bdAddr addr, bdNATTypeDiscoveryPacket &packet);

		// Handles for the different sockets
		bdDefaultSocket *m_primarySocket;
		bdDefaultSocket m_secondarySocketPrimPort;
		bdDefaultSocket m_primSocketSecPort;
		bdDefaultSocket m_secSocketSecPort;

		// Address and port pairs
		bdAddr m_primaryAddr;
		bdAddr m_secondaryAddrPrimPort;
		bdAddr m_primaryAddrSecPort;
		bdAddr m_secondaryAddrSecPort;
};

#endif // BD_NAT_TYPE_DISCOVERY_SERVER_H
