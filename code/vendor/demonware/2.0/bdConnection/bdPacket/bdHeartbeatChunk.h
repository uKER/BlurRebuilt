// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to represent a heartbeat chunk.

#ifndef BD_HEARTBEAT_CHUNK_H
#define BD_HEARTBEAT_CHUNK_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdPacket/bdChunk.h>

//0                   1                   2                   3
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|BD_CT_HEARTBEAT|  Heartbeat Flags  |        Length             |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

BD_REFERENCE(bdHeartbeatChunk);

class bdHeartbeatChunk : public bdChunk
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdHeartbeatFlags
		{
			BD_CA_NO_FLAGS		= 0x00000000
		};

		bdHeartbeatChunk();

		/// Destructor.
		virtual ~bdHeartbeatChunk();

		bdHeartbeatFlags getFlags() const;

	protected:

		friend class bdPacket;

		virtual bdUInt serialize(bdUByte8 *data, 
								 const bdUInt size) const;

		virtual bdBool deserialize(const bdUByte8 *const data, 
								   const bdUInt size, 
								   bdUInt &offset);

	protected:

		bdHeartbeatFlags m_flags;
};

#endif // BD_HEARTBEAT_CHUNK_H
