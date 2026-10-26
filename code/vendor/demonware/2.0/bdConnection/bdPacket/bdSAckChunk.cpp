// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdPacket/bdSAckChunk.h>

#define BD_LOG_LEVEL "bdConnection/chunks"

bdSAckChunk::bdSAckChunk()
: bdChunk(BD_CT_SACK), 
  m_flags(BD_SC_NO_FLAGS),
  m_cumulativeAck(0),
  m_windowCredit(0)
{
}

bdSAckChunk::bdSAckChunk(const bdWord windowCredit,
						 bdSAckFlags flags)
: bdChunk(BD_CT_SACK), 
  m_flags(flags),
  m_cumulativeAck(0),
  m_windowCredit(windowCredit)
{
}

bdSAckChunk::~bdSAckChunk()
{
}

bdSeqNumber bdSAckChunk::getCumulativeAck() const
{
	return m_cumulativeAck;
}

void bdSAckChunk::setCumulativeAck(const bdSeqNumber seqNumAck)
{
	m_cumulativeAck = seqNumAck;
}

void bdSAckChunk::addGap(const bdGapAckBlock &gap)
{
	m_gaps.addTail(gap);
}

bdLinkedList<bdSAckChunk::bdGapAckBlock> &bdSAckChunk::getGapList()
{
	return m_gaps;
}

void bdSAckChunk::setWindowCredit(const bdWord credit)
{
	m_windowCredit = credit;
}

bdWord bdSAckChunk::getWindowCredit() const
{
	return m_windowCredit;
}

bdSAckChunk::bdSAckFlags bdSAckChunk::getFlags() const
{
	return m_flags;
}


bdUInt bdSAckChunk::serialize(bdUByte8 *data, 
							  const bdUInt size) const
{
	// WARNING: This code must always return the serialized size of the chunk.
	// Even when data == BD_NULL and size == 0, be careful when editing.

	bdUInt offset = bdChunk::serialize(data, size);
	bdBool ok = true;

	const bdUByte8 flags = static_cast<bdUByte8>(m_flags);
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, flags);

	bdUInt16 length = 0;
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, length);

	// reserved
	offset += 1;

	// Cumulative TSN Ack
	const bdUInt16 seqNum = static_cast<bdUInt16>(m_cumulativeAck);
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, seqNum);

	// Advertised Receiver Window Credit
	const bdUInt32 recvWndCredit = static_cast<bdUInt32>(m_windowCredit);
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, recvWndCredit);

	// now write gap ack blocks
	// number of gap ack blocks:
	const bdUInt16 numGaps = static_cast<bdUInt16>(m_gaps.getSize());
	ok= ok && bdBytePacker::appendBasicType(data, size, offset, offset, numGaps); 

	// number of dup tsns - fixme - not implemented
	const bdUInt16 tmpZero = 0;
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, tmpZero);

	// gaps
	bdLinkedList<bdGapAckBlock>::Position pos = m_gaps.getHeadPosition();
	while(pos)
	{
		const bdGapAckBlock &gap = m_gaps.getAt(pos);

		const bdUInt16 start = static_cast<bdUInt16>(gap.m_start);
		ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, start);

		const bdUInt16 end = static_cast<bdUInt16>(gap.m_end);
		ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, end);

		m_gaps.forward(pos);
	}

	return offset;
}

bdBool bdSAckChunk::deserialize(const bdUByte8 *const data, 
								const bdUInt size,
								bdUInt &offset)
{
	bdUInt bytesRead = offset;
	bdBool ok = bdChunk::deserialize(data, size, bytesRead);
	
	bdUByte8 flags = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, flags);
	m_flags = static_cast<bdSAckFlags>(flags);

	bdUInt16 length = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, length);

	// reserved
	bytesRead += 1;

	bdUInt16 seqNum = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, seqNum);
	m_cumulativeAck = seqNum;

	bdUInt32 recvWndCredit = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, recvWndCredit);
	m_windowCredit = static_cast<bdUWord>(recvWndCredit);

	// now read out gap ack blocks
	bdUInt16 numGaps = 0;
	// number of gap ack blocks:
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, numGaps);
	// number of dup tsns - not implemented yet - fixme
	//bdMemcpy(numDupTSNS, data+offset, 2);
	bytesRead += 2;

	bdUInt16 numGapsRead = 0;
	while(ok && (numGapsRead < numGaps))
	{
		bdUInt16 start = 0;
		ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, start);

		bdUInt16 end = 0;
		ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, end);

		m_gaps.addTail(bdGapAckBlock(start, end));

		BD_INFO(BD_LOG_LEVEL, "gap ack: %hu-%hu", start, end);

		numGapsRead++;
	}

	if(ok)
	{
		offset = bytesRead;
	}

	return ok;
}

