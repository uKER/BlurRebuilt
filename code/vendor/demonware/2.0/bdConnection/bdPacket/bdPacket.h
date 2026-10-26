// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to represent a packet.

#ifndef BD_PACKET_H
#define BD_PACKET_H

#include <bdCore/bdCore.h>

BD_REFERENCE(bdChunk);

class bdHMacSHA1;

//0                   1                   2                   3
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                       Verification Tag                        |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                          Chunk #1                             |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                           ...                                 |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                          Chunk #n                             |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define BD_SOCKET_SIZE_FIELD_SIZE (2u)

#define BD_PF_VTAG_SIZE			(4u)
#define BD_PF_HEADER_SIZE		(BD_PF_VTAG_SIZE)
#define BD_BASIC_PACKET_OVERHEAD (BD_PF_HEADER_SIZE + BD_SOCKET_SIZE_FIELD_SIZE)

class bdPacket
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdPacket();

		bdPacket(const bdUInt32 tag,
				 const bdUInt bufferSize);

		/// Destructor.
		virtual ~bdPacket();

		bdUInt serialize(bdUByte8 *buffer, 
						const bdUInt size);

		bdBool deserialize(const bdUByte8 *data, 
						   const bdUInt size);

		bdUInt32 getVerificationTag() const;

		bdBool addChunk(bdChunkRef chunk);

		bdBool getNextChunk(bdChunkRef &chunk);

		bdBool isEmpty() const;

	protected:
/*
		bdBool appendEncodedInt(bdUByte8* buffer,
								const bdUInt bufferSize,
								bdUInt &offset,
								bdUInt16 value) const;

		bdBool removeEncodedInt(const bdUByte8* buffer,
								const bdUInt bufferSize,
								bdUInt &offset,
								bdUInt16 &value) const;
*/
		bdLinkedList<bdChunkRef> m_chunks;

		bdCommonAddrRef m_me;

		bdUInt32 m_tag;

		bdUInt m_bufferSize;

		bdUInt m_currentSize;
};

#endif // BD_PACKET_H
