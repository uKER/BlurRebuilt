// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to represent a selective ack chunk.

#ifndef BD_SACK_CHUNK_H
#define BD_SACK_CHUNK_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdPacket/bdChunk.h>

//0                   1                   2                   3
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|   BD_CT_SACK  |  SACK Flags   |           Length              |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|   Reserved    |      Cumulative TSN Ack       |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|          Advertised Receiver Window Credit (a_rwnd)           |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//| Number of Gap Ack Blocks = N  |  Number of Duplicate TSNs = X |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|  Gap Ack Block #1 Start       |   Gap Ack Block #1 End        |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///                                                               /
//\                              ...                              \. 
///                                                               /
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|   Gap Ack Block #N Start      |  Gap Ack Block #N End         |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                       Duplicate TSN 1                         |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///                                                               /
//\                              ...                              \. 
///                                                               /
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                       Duplicate TSN X                         |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   SACK Flags: 8 bits
//
//      Set to all zeros on transmit and ignored on receipt.
//
//   Cumulative TSN Ack: 16 bits (unsigned integer)
//
//      This parameter contains the TSN of the last DATA chunk received in
//      sequence before a gap.
//
//   Advertised Receiver Window Credit (a_rwnd): 32 bits (unsigned
//      integer)
//
//      This field indicates the updated receive buffer space in bytes of
//      the sender of this SACK, see Section 6.2.1 for details.
//
//   Number of Gap Ack Blocks: 16 bits (unsigned integer)
//
//      Indicates the number of Gap Ack Blocks included in this SACK.
//
//   Number of Duplicate TSNs: 16 bit
//
//      This field contains the number of duplicate TSNs the endpoint has
//      received.  Each duplicate TSN is listed following the Gap Ack
//      Block list.
//
//   Gap Ack Blocks:
//
//      These fields contain the Gap Ack Blocks.  They are repeated for
//      each Gap Ack Block up to the number of Gap Ack Blocks defined in
//      the Number of Gap Ack Blocks field.  All DATA chunks with TSNs
//      greater than or equal to (Cumulative TSN Ack + Gap Ack Block
//      Start) and less than or equal to (Cumulative TSN Ack + Gap Ack
//      Block End) of each Gap Ack Block are assumed to have been received
//      correctly.
//
//   Gap Ack Block Start: 16 bits (unsigned integer)
//
//      Indicates the Start offset TSN for this Gap Ack Block.  To
//      calculate the actual TSN number the Cumulative TSN Ack is added to
//      this offset number.  This calculated TSN identifies the first TSN
//      in this Gap Ack Block that has been received.
//
//   Gap Ack Block End:  16 bits (unsigned integer)
//
//      Indicates the End offset TSN for this Gap Ack Block.  To calculate
//      the actual TSN number the Cumulative TSN Ack is added to this
//      offset number.  This calculated TSN identifies the TSN of the last
//      DATA chunk received in this Gap Ack Block.
//
//   For example, assume the receiver has the following DATA chunks newly
//   arrived at the time when it decides to send a Selective ACK,
//
//                        ----------
//                        | TSN=17 |
//                        ----------
//                        |        | <- still missing
//                        ----------
//                        | TSN=15 |
//                        ----------
//                        | TSN=14 |
//                        ----------
//                        |        | <- still missing
//                        ----------
//                        | TSN=12 |
//                        ----------
//                        | TSN=11 |
//                        ----------
//                        | TSN=10 |
//                        ----------
//
//   then, the parameter part of the SACK MUST be constructed as follows
//   (assuming the new a_rwnd is set to 4660 by the sender):
//
//                  +--------------------------------+
//                  |   Cumulative TSN Ack = 12      |
//                  +--------------------------------+
//                  |        a_rwnd = 4660           |
//                  +----------------+---------------+
//                  | num of block=2 | num of dup=0  |
//                  +----------------+---------------+
//                  |block #1 strt=2 |block #1 end=3 |
//                  +----------------+---------------+
//                  |block #2 strt=5 |block #2 end=5 |
//                  +----------------+---------------+
//
//
//   Duplicate TSN: 32 bits (unsigned integer)
//
//      Indicates the number of times a TSN was received in duplicate
//      since the last SACK was sent.  Every time a receiver gets a
//      duplicate TSN (before sending the SACK) it adds it to the list of
//      duplicates.  The duplicate count is re-initialized to zero after
//      sending each SACK.
//
//      For example, if a receiver were to get the TSN 19 three times it
//      would list 19 twice in the outbound SACK.  After sending the SACK
//      if it received yet one more TSN 19 it would list 19 as a duplicate
//      once in the next outgoing SACK.

BD_REFERENCE(bdSAckChunk);

class bdSAckChunk : public bdChunk
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdSAckFlags
		{
			BD_SC_NO_FLAGS		= 0x00000000
		};

		class bdGapAckBlock
		{
			public:
				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				bdGapAckBlock()
				: m_start(0),
				  m_end(0)
				{
				}

				bdGapAckBlock(bdUInt start,	bdUInt end)
				: m_start(start),
				  m_end(end)
				{
				}

				bdUInt m_start;
				bdUInt m_end;
		};

		bdSAckChunk();

		bdSAckChunk(const bdWord windowCredit,
					bdSAckFlags flags = BD_SC_NO_FLAGS);

		/// Destructor.
		virtual ~bdSAckChunk();

		bdSeqNumber getCumulativeAck() const;

		void setCumulativeAck(const bdSeqNumber seqNumAck);

		void addGap(const bdGapAckBlock &gap);

		bdLinkedList<bdGapAckBlock> &getGapList();

		void setWindowCredit(const bdWord credit);

		bdWord getWindowCredit() const;

		bdSAckFlags getFlags() const;

	protected:

		friend class bdPacket;

		virtual bdUInt serialize(bdUByte8 *data, 
								 const bdUInt size) const;

		virtual bdBool deserialize(const bdUByte8 *const data, 
								   const bdUInt32 size, 
								   bdUInt &offset);

	protected:

		bdSAckFlags m_flags;

		bdSeqNumber m_cumulativeAck;

		bdLinkedList<bdGapAckBlock> m_gaps;

		bdWord m_windowCredit;
};

#endif // BD_SACK_CHUNK_H
