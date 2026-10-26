// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdNATTravServer.h>

#include <bdSocket/bdNAT/bdNAT.h>
#include <bdSocket/bdNAT/bdPacket/bdNATTraversalPacket.h>

#define BD_LOG_LEVEL "bdSocket/nat"

bdNATTravServer::bdNATTravServer()
: m_socket(BD_NULL)
{
}

bdNATTravServer::~bdNATTravServer()
{
	m_socket = BD_NULL;
}

bdBool bdNATTravServer::init(bdDefaultSocket *socket)
{
	bdBool result = true;
	m_socket = socket;
	return result;
}

bdBool bdNATTravServer::acceptPacket(bdDefaultSocket *, bdAddr addr, void *data, const bdUInt size, const bdUByte8 type)
{
	bdBool accepted = false;

	if (size < 1)
	{
		return false;
	}


	if(type >= BD_NAT_TRAV_TYPE_MIN && type <= BD_NAT_TRAV_TYPE_MAX)
	{
		processPacket(addr, data, size);

		accepted = true;
	}

	return accepted;
}


// protected:

bdInt bdNATTravServer::processPacket(const bdAddr &addr, 
							 const void *data,
							 const bdUInt size)
{
	bdUInt tmpInt;
	bdNATTraversalPacket packet;
	if (packet.deserialize(data, size, 0, tmpInt))
	{
		switch(packet.getType())
		{
			// requesting us to send the introduction packet onto the destination.
			case BD_NAT_TRAV_INTRO_REQ:
			{
				BD_INFO(BD_LOG_LEVEL, "Received intro request. Forwarding.");

				bdUByte8 buffer[BD_MAX_DATAGRAM_SIZE];

				const bdAddr dest = packet.getAddrDest();
				packet.setType(BD_NAT_TRAV_INTRO_FORWARD);
				packet.serialize(buffer, BD_MAX_DATAGRAM_SIZE, 0, tmpInt);
				m_socket->sendTo(dest, buffer, tmpInt);

				const bdUWord strSize = 100;
				bdNChar8 src[strSize];
				addr.toString(src, strSize);

				bdNChar8 dst[strSize];
				dest.toString(dst, strSize);
				BD_INFO(BD_LOG_LEVEL, "INTRO REQ from %s to %s", src, dst);
			}
			break;
			// requesting us to send the packet back to the src address.
			case BD_NAT_TRAV_INTRO_FORWARD:
			{
				BD_WARN(BD_LOG_LEVEL, "Server received client packet");
			}
			break;
			// reply to our original introduction request
			case BD_NAT_TRAV_INTRO_REPLY:
			{
				BD_WARN(BD_LOG_LEVEL, "Server received client packet");
			}
			break;
			case BD_NAT_TRAV_KEEP_ALIVE:
			{
				// not required to do anything. ignore.
			}
			break;
		}
	}

	return 0;
}


