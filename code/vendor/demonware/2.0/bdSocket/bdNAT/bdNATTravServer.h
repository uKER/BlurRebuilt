// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_NAT_TRAV_SERVER_H
#define BD_NAT_TRAV_SERVER_H

#include <bdCore/bdCore.h>

#include <bdSocket/bdPacketInterceptor.h>

// Nat traversal works according to the following table.
// F - full cone
// M - Moderate
// S - Strict
// X - Can Connect
// 0 - Can't connect
//
//     F    M     S
//    ----------------
// F | X |  X  |  X  |
//   |---------------|
// M | X |  X  |  0  |
//   |---------------|
// S | X |  0  |  0  |
//   |---------------|

class bdNATTravServer : public bdPacketInterceptor
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		bdNATTravServer();

		/// Virtual Destructor
		virtual ~bdNATTravServer();

		bdBool init(bdDefaultSocket *socket);

		virtual bdBool acceptPacket(bdDefaultSocket *socket, 
									bdAddr addr, 
									void *data, 
									const bdUInt size,
									const bdUByte8 type);

	protected:

		bdInt processPacket(const bdAddr &addr,
						  const void *data,
						  const bdUInt size);

	protected:

		bdDefaultSocket *m_socket;
};

#endif // BD_NAT_TRAV_SERVER_H
