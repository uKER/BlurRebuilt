#ifndef __MERCURY_SYSTEM_SPU_RWLOCK_H__
#define __MERCURY_SYSTEM_SPU_RWLOCK_H__

#if PLATFORM_PS3 | PLATFORM_SPU

#include "core/threads/spuEvent.h"
#include "core/threads/atomicMutex.h"

// Times in milliseconds
#define WAIT_TIME_INFINITE		(0xffffffff)

namespace MT
{

/// a read-write lock
/// allows multiple concurrent reads, or one write
class __attribute__((aligned(16))) SpuRwLock
{
#ifdef PLATFORM_SPU
private:
	SpuRwLock();
#endif
public:
#ifndef PLATFORM_SPU
	/// constructor
	SpuRwLock(const char *pszName = NULL) :
	  m_ReadEvent(pszName),
	  m_iReaders(0)
	{
	}

	/// destructor
	~SpuRwLock()
	{
	}

	/// acquire a write-lock
	bool WriteLock(const U32 timeMs = WAIT_TIME_INFINITE )
	{
		SpuRwLock *pThis = (SpuRwLock*)m_WriteMutex.ThisPtr;
		if (!m_WriteMutex.Wait(timeMs))
			return false;

		while (AtomicGet(&pThis->m_iReaders) > 0)
		{
			// wait for readers == 0 before we use the write lock
			if (!m_ReadEvent.Wait(timeMs))
			{
				// timed out
				m_WriteMutex.Release();
				return false;
			}
		}
		return true;
	}

	/// release a write-lock
	void ReleaseWriteLock()
	{
		bool bExit = m_WriteMutex.Release();
		assert(bExit);
	}
#endif

	/// acquire a read-lock
	bool ReadLock(const U32 timeMs = WAIT_TIME_INFINITE )
	{
		SpuRwLock *pThis = (SpuRwLock*)m_WriteMutex.ThisPtr;
		if (!m_WriteMutex.Wait(timeMs))
			return false;
		
		AtomicIncrement(&pThis->m_iReaders);

		m_WriteMutex.Release();
		return true;
	}

	/// release a read-lock
	void ReleaseReadLock()
	{
		SpuRwLock *pThis = (SpuRwLock*)m_WriteMutex.ThisPtr;
		if (AtomicDecrement(&pThis->m_iReaders)-1 == 0)
			m_ReadEvent.Signal();
	}

private:
	AtomicLock			m_WriteMutex;		// exclusive write lock
	SpuEvent			m_ReadEvent;		// an event that is signaled when readers reaches 0
	S32					m_iReaders;			// number of concurrent reads
};

} // namespace MT

#else

// on other platforms SpuRwLock is the same as RwLock
#include "core/threads/RwLock.h"
namespace MT
{
class SpuRwLock : public RwLock
{
};
}
#endif

#endif
