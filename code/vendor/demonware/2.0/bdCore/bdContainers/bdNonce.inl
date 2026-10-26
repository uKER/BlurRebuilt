// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdUtilities/bdTrulyRandom.h>

template <bdUInt size> inline
bdNonce<size>::bdNonce()
: m_initialised(false)
{
}

template <bdUInt size> inline
bdNonce<size>::~bdNonce()
{
}

template <bdUInt size> inline
void bdNonce<size>::ensureCreated()
{
	if (!m_initialised)
	{
		bdTrulyRandom::getInstance()->getRandomUByte8(m_nonce, size);
		m_initialised = true;
	}
}

template <bdUInt size> inline
const bdUByte8* bdNonce<size>::getData()
{
	BD_ASSERT(m_initialised, "Attempted to retrieve a nonce without initialising it first");
	return m_nonce;
}
