// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdThread/bdRunnable.h>

bdRunnable::bdRunnable()
:m_stop(false)
{
}

bdRunnable::~bdRunnable()
{
}

void bdRunnable::stop()
{
	m_stop = true;
}

void bdRunnable::start()
{
	m_stop = false;
}


