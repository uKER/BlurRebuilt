#include "CorePch.h"
#include "core/core.h"

#include "core/Threads/Mutex.h"
#include "core/Threads/Event.h"
#include "core/Threads/RwLock.h"
#include "core/Threads/Atomic.h"

namespace MT
{


/// constructor
RwLock::RwLock() :
	m_iReaders(0)
{
}

/// destructor
RwLock::~RwLock()
{
}

/// acquire a read-lock
bool RwLock::ReadLock(const U32 timeout)
{
	if (!m_WriteMutex.Wait(timeout))
		return false;

	// increase the number of readers
	AtomicIncrement(&m_iReaders);

	// it's ok to release the write mutex. no writes are possible anyway since the number of readers > 0
	m_WriteMutex.Release();

    return true;
}

/// acquire a write-lock
bool RwLock::WriteLock(const U32 timeout)
{
	if (!m_WriteMutex.Wait(timeout))
		return false;

	while (m_iReaders > 0)
	{
		// wait for readers == 0 before we use the write lock
		if (!m_ReadEvent.Wait(timeout))
		{
			// timed out
			m_WriteMutex.Release();
			return false;
		}
	}

	return true;
}

/// release the write lock
void RwLock::ReleaseWriteLock()
{
	bool bExit = m_WriteMutex.Release();
	assert(bExit);
}

/// release the read lock
void RwLock::ReleaseReadLock()
{
	assert(m_iReaders>0);
	if (AtomicDecrement(&m_iReaders)-1 == 0)
		m_ReadEvent.Signal();
}

}; // namespace MT
