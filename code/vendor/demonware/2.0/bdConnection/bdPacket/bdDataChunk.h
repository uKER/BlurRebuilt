// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to represent a data chunk.

#ifndef BD_DATA_CHUNK_H
#define BD_DATA_CHUNK_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdPacket/bdChunk.h>

//0                   1                   2                   3
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|   BD_CT_DATA  |  Data Flags   |        Data Length            |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|   Data Type   |   Stream ID   |      Stream Seq Number        |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//\                                                               \.
///                             Data                              /
//\                                                               \. 
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

BD_REFERENCE(bdDataChunk);
BD_REFERENCE(bdMessage);

/// A class to represent a data chunk.
class bdDataChunk : public bdChunk
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdDataFlags
		{
			BD_DC_NO_FLAGS		= 0,
			BD_DC_UNRELIABLE	= (1 << 0),
			BD_DC_ENC_DATA		= (1 << 1),
			BD_DC_UNENC_DATA	= (1 << 2)
		};

		bdDataChunk();

		bdDataChunk(bdMessageRef data,
					const bdDataFlags flags);

		/// Destructor.
		virtual ~bdDataChunk();

		void setSequenceNumber(bdSeqNumber seqNum);

		bdSeqNumber getSequenceNumber() const;

		bdMessageRef getMessage() const;

		bdUByte8 getFlags() const;

		virtual bdUInt getSerializedSize() const;

	protected:

		friend class bdPacket;

		virtual bdUInt serialize(bdUByte8 *data, 
								 const bdUInt32 size) const;
		
		bdUInt serializeUnencrypted(bdUByte8 *data, 
									const bdUInt32 size) const;
				

		virtual bdBool deserialize(const bdUByte8 *const data,
								   const bdUInt size,
								   bdUInt &offset);

		bdBool deserialize(const bdUByte8 *const data, 
						   const bdUInt size,
						   bdUInt &offset,
						   const bdUByte8 *const unencData, 
						   const bdUInt unencSize,
						   bdUInt &unencOffset);

	protected:

		bdMessageRef m_message;
		bdUByte8 m_flags;
		bdSeqNumber m_seqNum;
};

#endif // BD_DATA_CHUNK_H
