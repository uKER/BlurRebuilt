// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_IP_DISCOVERY_SERVER_H
#define BD_IP_DISCOVERY_SERVER_H

#include <bdCore/bdCore.h>

#include <bdSocket/bdPacketInterceptor.h>

class bdIPDiscoveryServer : public bdPacketInterceptor
{

	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdIPDiscoveryServer();

 		/// Virtual destructor
		virtual ~bdIPDiscoveryServer();

		virtual bdBool acceptPacket(bdDefaultSocket *socket, 
									bdAddr addr, 
									void *data, 
									const bdUInt size,
									const bdUByte8 type);

	protected:

};


#endif // BD_IP_DISCOVERY_SERVER_H
