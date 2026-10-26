// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_QOS_REQUEST_PACKET_H
#define BD_QOS_REQUEST_PACKET_H

#include <bdCore/bdCore.h>

class bdQoSRequestPacket
{
	public:

		bdQoSRequestPacket(bdUInt32 secid, bdUInt32 id);

		bdQoSRequestPacket();

		bdUByte8 getType() const;
		bdUInt64 getTimestamp() const;
		bdUInt32 getId() const;
		bdUInt32 getSecId() const;

		bdBool serialize(void *data, const bdUInt size, const bdUInt offset, bdUInt &newOffset) const;
		bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt &newOffset);

	protected:
		bdUByte8 m_type;
		bdUInt64 m_timestamp;
		bdUInt32 m_id;
		bdUInt32 m_secid;
};

#endif //BD_QOS_REQUEST_PACKET_H


