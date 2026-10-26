#ifndef __CORE_THREAD_ATOMIC_MUTEX_H__
#define __CORE_THREAD_ATOMIC_MUTEX_H__

#include "core/threads/misc.h"
#include "core/threads/atomic.h"

namespace MT
{

/** Simple mutex class based on atomic primitives.
	Supports both PPU and SPU (unlike MT::Mutex). Works using wait-spin; if a thread is blocked it will not yield(!)

	The mutex is recursive, meaning that it can be locked more than once by the owning thread, and maintains a count of the number of times
	it was locked.

	The behavior of the mutex, when Lock is called and another thread has the mutex locked, is to yield one timeslice at a time until the mutex
	can be locked.

	\remark  To enable this class to work on SPU, it saves a pointer to itself internally.  This allows the SPU to atomically update 
	         the mutex's fields in main memory, no matter what SPU it is running on.  Therefore it is extremely important that AtomicLock
	         objects are not moved around in memory after they are created.
*/
class AtomicLock
{
public:
	mutable U64 m_ThreadId;   //!< ID of the thread which currently owns this mutex, or 0 if the mutex is unlocked.
	mutable int m_LockCount;  //!< Number of times the mutex has been locked recursively by the owner thread, or 0 if the mutex is unlocked.
	AtomicLock* ThisPtr; //!< Pointer to this, used by SPU implementation to reference original memory location.

	AtomicLock()
	{
		Create();
	}

	~AtomicLock()
	{
		Destroy();
	}

	//! Initializes the mutex data.
	inline void Create()
	{
		ThisPtr = this;
		m_ThreadId = 0;
		m_LockCount = 0;
	}

	//! Clears the mutex data.
	inline void Destroy()
	{
#if TL_DEBUG_ATOMIC_MUTEX
		assert(m_LockCount == 0); // AtomicLock was locked when being destroyed!
#endif
		m_ThreadId = 0;
		ThisPtr = NULL;
	}

	//! Waits for the mutex to become available, then locks the mutex.  If the mutex was already locked by the calling thread, increments the lock count and returns.
	inline bool Wait(U32 _iTime) const
	{
		if (_iTime == INFINITE)
			return Wait();
		U64 iTime = (U64)_iTime * 1000;

		ThreadId CurThread = GetThreadId();
#if PLATFORM_SPU
		// On SPU, this function must be implemented using atomic primitives, which can
		//  access main memory directly.
		if (MT::AtomicCompareAndSwap(&ThisPtr->m_ThreadId, CurThread, CurThread))
			MT::AtomicIncrement(&ThisPtr->m_LockCount);
		else
		{
			while(!MT::AtomicCompareAndSwap(&ThisPtr->m_ThreadId, CurThread, 0))
			{
				if (iTime--==0)
					return false;
				MT::YieldThread();
			}
			MT::AtomicSet((U32*)&ThisPtr->m_LockCount, 1);
		}
#else
		// On normal processors it can be implemented using regular primitives, plus a fence
		//  to make sure the thread id reaches memory before the lock count.
		if (ThisPtr->m_ThreadId == CurThread)
		{
#if TL_DEBUG_ATOMIC_MUTEX
			assert(ThisPtr->m_LockCount > 0); // AtomicLock lock count error when trying to lock with same thread!
#endif
			ThisPtr->m_LockCount++;
		}
		else
		{
			while(!MT::AtomicCompareAndSwap(&ThisPtr->m_ThreadId, CurThread, 0)) 
			{
				if (iTime--==0)
					return false;
				YieldThread();
			}
			MT::MemoryFence();
#if TL_DEBUG_ATOMIC_MUTEX
			assert(ThisPtr->m_LockCount == 0); // AtomicLock lock count error when trying to lock with another thread!
#endif
			ThisPtr->m_LockCount = 1;
		}
#endif
		return true;
	}

	//! Waits for the mutex to become available, then locks the mutex.  If the mutex was already locked by the calling thread, increments the lock count and returns.
	inline bool Wait() const
	{
		ThreadId CurThread = GetThreadId();
#if PLATFORM_SPU
		// On SPU, this function must be implemented using atomic primitives, which can
		//  access main memory directly.
		if (MT::AtomicCompareAndSwap(&ThisPtr->m_ThreadId, CurThread, CurThread))
			MT::AtomicIncrement(&ThisPtr->m_LockCount);
		else
		{
			while(!MT::AtomicCompareAndSwap(&ThisPtr->m_ThreadId, CurThread, 0))
				MT::YieldThread();
			MT::AtomicSet((U32*)&ThisPtr->m_LockCount, 1);
		}
#else
		// On normal processors it can be implemented using regular primitives, plus a fence
		//  to make sure the thread id reaches memory before the lock count.
		if (ThisPtr->m_ThreadId == CurThread)
		{
#if TL_DEBUG_ATOMIC_MUTEX
			assert(ThisPtr->m_LockCount > 0); // AtomicLock lock count error when trying to lock with same thread!
#endif
			ThisPtr->m_LockCount++;
		}
		else
		{
			while(!MT::AtomicCompareAndSwap(&ThisPtr->m_ThreadId, CurThread, 0)) 
				YieldThread();
			MT::MemoryFence();
#if TL_DEBUG_ATOMIC_MUTEX
			assert(ThisPtr->m_LockCount == 0); // AtomicLock lock count error when trying to lock with another thread!
#endif
			ThisPtr->m_LockCount = 1;
		}
#endif
		return true;
	}

	//! Unlocks the mutex.  The mutex must be owned by the calling thread.  If the mutex was locked more than once, decrements the lock count and returns immediately.
	inline bool Release() const
	{
#if PLATFORM_SPU
		// On SPU, this function must be implemented using atomic primitives, which can
		//  access main memory directly.
		if (MT::AtomicDecrement(&ThisPtr->m_LockCount)-1 == 0)
		{
			uint64_t buf[16] __attribute__((aligned(128)));
			MT::AtomicSet(&ThisPtr->m_ThreadId, 0);
		}
#else
#if TL_DEBUG_ATOMIC_MUTEX
		assert(ThisPtr->m_ThreadId == GetThreadId()); // AtomicLock error, current thread does not match locking thread when unlocking!
		assert(ThisPtr->m_LockCount > 0); // AtomicLock was unlocked without being locked!
#endif
		// On normal processors it can be implemented using normal primitives, plus a fence
		//  to make sure the lock count reaches memory before the thread id.
		if (--ThisPtr->m_LockCount == 0)
		{
			MT::MemoryFence();
			ThisPtr->m_ThreadId = 0;
		}
#endif
		return true;
	}
};

//@}

} // namespace MT

#endif
