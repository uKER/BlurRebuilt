// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: 


#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdContainers/bdSequenceNumberStore.h>

bdSequenceNumberStore::bdSequenceNumberStore(const bdSequenceNumber &inital)
: m_bitmap(0),
  m_lastSeq(inital)	  
{
}

bdSequenceNumberStore::bdSequenuceStatus bdSequenceNumberStore::check(const bdSequenceNumber &thisSeq)
{
	const bdUInt windowSize = sizeof(m_bitmap) << 3;

	bdUInt diff;

	// Is thisSeq in the future, or else in the past?
	if (thisSeq > m_lastSeq)
	{    
		// It is in the future. Find how far away.
		bdSequenceNumber sdiff = thisSeq - m_lastSeq;
		diff = static_cast<bdUInt>(sdiff.getValue());

		// check to see if it is not too far away into the future.
		if (diff < windowSize) 
		{
			// It is not too far away. Slide the window and mark it.
			m_bitmap = (m_bitmap << diff) | 1; 
			m_lastSeq = thisSeq;
			return BD_SN_VALID_LARGER;
		} 
		else 
		{
			// Far away in the future
			m_lastSeq = thisSeq;
			m_bitmap = 1;
			return BD_SN_VALID_MUCH_LARGER;
		}
	}
	else
	{
		// It is in the past.
		bdSequenceNumber sdiff = m_lastSeq - thisSeq;
		diff = static_cast<bdUInt>(sdiff.getValue());

		if (diff >= windowSize)
		{
			// too old, outside the window.
			return BD_SN_INVALID_SMALLER;
		}

		if (m_bitmap & (1 << diff))
		{
			// Is in the window, but was already seen.
			return BD_SN_INVALID_DUPLICATE; 
		}

		// Mark, and report correct.
		m_bitmap |= (1 << diff); 
		return BD_SN_VALID_SMALLER;
	}
}

const bdSequenceNumber &bdSequenceNumberStore::getLastSequenceNumber() const
{
	return m_lastSeq;
}

void bdSequenceNumberStore::reset(const bdSequenceNumber &inital)
{
	m_bitmap = 0;
	m_lastSeq = inital;
} 
