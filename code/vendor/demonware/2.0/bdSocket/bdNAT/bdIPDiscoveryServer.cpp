// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdIPDiscoveryServer.h>

#include <bdSocket/bdNAT/bdPacket/bdIPDiscoveryPacket.h>
#include <bdSocket/bdNAT/bdPacket/bdIPDiscoveryPacketReply.h>
#include <bdSocket/bdNAT/bdNAT.h>

bdIPDiscoveryServer::bdIPDiscoveryServer()
{
}

bdIPDiscoveryServer::~bdIPDiscoveryServer()
{
}

bdBool bdIPDiscoveryServer::acceptPacket(bdDefaultSocket *socket, bdAddr addr, void *data, const bdUInt size, const bdUByte8 type)
{
	bdBool accepted = false;

	if(BD_IP_DISC_REQ == type)
	{
		bdIPDiscoveryPacket packet;
		bdUInt tmpUInt;
		if (packet.deserialize(data, size, 0, tmpUInt))
		{
			if(packet.getType() == BD_IP_DISC_REQ)
			{
				bdIPDiscoveryPacketReply reply(addr, packet.getProtocolVersion());
				const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
				bdUByte8 buffer[bufferSize];

				BD_ASSERT(reply.serialize(buffer, bufferSize, 0, tmpUInt), "Failed to serialize a packet.");

				socket->sendTo(addr, buffer, tmpUInt);

				accepted = true;
			}
		}
	}
	return accepted;
}

