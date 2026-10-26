// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to represent a chunk.

#ifndef BD_CHUNK_H
#define BD_CHUNK_H

#include <bdCore/bdCore.h>

//0                   1                   2                   3
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|   Chunk Type  | Chunk  Flags  |        Chunk Length           |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//\                                                               \.
///                          Chunk Value                          /
//\                                                               \.
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define BD_CHUNK_MIN_SIZE	(1 + 1 + 2)

enum bdChunkTypes
{
	BD_CT_INVALID = 0,
	BD_CT_PADDING = 1,
	BD_CT_DATA, // Payload Data
	BD_CT_INIT, // Initiation
	BD_CT_INIT_ACK, // Initiation Acknowledgment
	BD_CT_SACK, // Selective Acknowledgment
	BD_CT_HEARTBEAT, // Heartbeat Request
	BD_CT_HEARTBEAT_ACK, // Heartbeat Acknowledgment
	BD_CT_ABORT, // Abort
	BD_CT_SHUTDOWN, // Shutdown
	BD_CT_SHUTDOWN_ACK, // Shutdown Acknowledgment
	BD_CT_SHUTDOWN_COMPLETE, // Shutdown Complete
	BD_CT_ERROR, // Operation Error
	BD_CT_COOKIE_ECHO, // State Cookie
	BD_CT_COOKIE_ACK // Cookie Acknowledgment
};

BD_REFERENCE(bdChunk);

/// A class to represent a chunk.
class bdChunk : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdChunk(const bdChunkTypes type);

		/// Destructor.
		virtual ~bdChunk();

		virtual bdChunkTypes getType() const;

		static bdChunkTypes BD_CALL getType(const bdUByte8 *const data,
											const bdUInt size);

		virtual bdUInt serialize(bdUByte8 *data,
								 const bdUInt32 size) const;

		virtual bdBool deserialize(const bdUByte8 *const data,
								   const bdUInt size,
								   bdUInt &offset);

		virtual bdUInt getSerializedSize() const;

		bdBool isControl() const;

	protected:

		struct bdChunkData
		{
			bdUByte8 m_type;
			bdUByte8 m_flags;
			bdUInt16 m_length;
		};

	protected:

		bdChunkTypes m_type;

		bdChunkData m_headerData;
};

#endif // BD_CHUNK_H
