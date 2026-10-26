// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_QOS_REPLY_PACKET_H
#define BD_QOS_REPLY_PACKET_H

#include <bdCore/bdCore.h>

class bdQoSReplyPacket
{
	public:

		bdQoSReplyPacket();

		bdQoSReplyPacket(bdUInt32 id, bdUInt64 timestamp);

		~bdQoSReplyPacket();

		bdBool setData(const bdUByte8 *data, bdUInt dataSize);
		void setId(bdUInt32 id);
		void setTimestamp(bdUInt64 timestamp);
		void setEnabled(bdBool mode);

		bdUByte8 getType() const;
		bdFloat32 getElapsedTime() const;
		bdUInt32 getId() const;
		bdUByte8* getData();
		bdUInt32 getDataSize() const;
		bdBool getEnabled() const;

		bdUInt getSerializedSize();

		bdBool serialize(void *data, const bdUInt size, const bdUInt offset, bdUInt &newOffset) const;
		bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt &newOffset);

	protected:

		bdUInt getHeaderSize();

		bdUByte8 m_type;
		bdUInt32 m_id;
		bdUInt64 m_timestamp;
		bdBool m_enabledMode;

		bdUByte8 *m_data;
		bdUInt32 m_dataSize;

		static bdUInt headerSize;
		bdUInt m_serializedSize;
};

#endif // BD_QOS_REPLY_PACKET_H

