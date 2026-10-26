// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdContainers/bdSequenceNumber.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#define BD_LOG_LEVEL	"bdCore/bdContainers/sequenceNumber"

bdSequenceNumber::bdSequenceNumber(const bdInt seqNum)
: m_seqNum(seqNum)
{
}

bdSequenceNumber::bdSequenceNumber(const bdSequenceNumber &last, 
								   const bdUInt seqNumber, 
								   const bdUInt bits)
: m_seqNum(-1)
{
	set(last, seqNumber, bits);
}

void bdSequenceNumber::set(const bdSequenceNumber &last, 
						   const bdUInt seqNumber, 
						   const bdUInt bits)
{
	// Sequence comparisons are based on RFC 1982 - Serial Number Arithmetic

	const bdInt range = 2 << (bits - 1);
	const bdInt lastSeq = last.m_seqNum % range;
	const bdInt lastRangeBase = last.m_seqNum - (last.m_seqNum % range);
	const bdInt curSeq = static_cast<bdInt>(seqNumber) % range;

	BD_ASSERT(static_cast<bdInt>(seqNumber) < range, "Sequence number given outside the range.");

	if (lastSeq < 0)
	{
		// Last was initial number.
		m_seqNum = seqNumber;
	}
	else if (lastSeq == curSeq) 
	{
		// We assume numbers are equal.
		m_seqNum = last.m_seqNum;
	}
	else
	{
		// Comparisons from RFC1982
		const bdBool lastSmaller = 
			((lastSeq < curSeq) && (curSeq - lastSeq < range / 2)) ||
			((lastSeq > curSeq) && (lastSeq - curSeq > range / 2));
		const bdBool lastLarger = 
			((lastSeq < curSeq) && (curSeq - lastSeq) > range / 2) ||
			((lastSeq > curSeq) && (lastSeq - curSeq < range / 2));

		bdInt rangeChange = 0;

		if (!(lastLarger || lastSmaller))
		{
			BD_WARN(BD_LOG_LEVEL, "Sequence numbers are too far away and cannot "
				"be compared.");
		}

		if (lastLarger && lastSeq > curSeq)
		{
			// |-----xllll|----------|
			// Do nothing
		}
		else if (lastLarger && lastSeq < curSeq)
		{
			// |------xlll|ll--------|
			rangeChange = -1;
		}
		else if (lastSmaller && lastSeq > curSeq)
		{
			// |-------lll|llx-------|
			rangeChange = 1;
		}
		else if (lastSmaller && lastSeq < curSeq)
		{
			// |lllllx----|----------|
			// Do nothing
		}

		m_seqNum = lastRangeBase + range * rangeChange + curSeq;
	}
}

bdInt32 bdSequenceNumber::getValue() const
{
	return m_seqNum;
}

bdSequenceNumber bdSequenceNumber::operator + (const bdSequenceNumber& other) const
{
	bdSequenceNumber seqNum(m_seqNum);
	seqNum.m_seqNum += other.m_seqNum;
	return seqNum;
}

bdSequenceNumber& bdSequenceNumber::operator += (const bdSequenceNumber& other)
{
	m_seqNum += other.m_seqNum;
	return *this;
}

bdSequenceNumber& bdSequenceNumber::operator ++ ()
{
	m_seqNum++;
	return *this;
}

bdSequenceNumber bdSequenceNumber::operator ++ (bdInt)
{
	bdSequenceNumber other = *this;
	m_seqNum += 1;
	return other;
}

bdSequenceNumber bdSequenceNumber::operator - (const bdSequenceNumber& other) const
{
	bdSequenceNumber seqNum(m_seqNum);	
	seqNum.m_seqNum -= other.m_seqNum;
	return seqNum;
}

bdBool bdSequenceNumber::operator > (const bdSequenceNumber &other) const
{
	return m_seqNum > other.m_seqNum;
}

bdBool bdSequenceNumber::operator < (const bdSequenceNumber &other) const
{
	return m_seqNum < other.m_seqNum;
}

bdBool bdSequenceNumber::operator <= (const bdSequenceNumber &other) const
{
	return m_seqNum <= other.m_seqNum;
}

bdBool bdSequenceNumber::operator >= (const bdSequenceNumber &other) const
{
	return m_seqNum >= other.m_seqNum;
}

bdBool bdSequenceNumber::operator == (const bdSequenceNumber &other) const
{
	return m_seqNum == other.m_seqNum;
}

bdBool bdSequenceNumber::operator != (const bdSequenceNumber &other) const
{
	return m_seqNum != other.m_seqNum;
}

