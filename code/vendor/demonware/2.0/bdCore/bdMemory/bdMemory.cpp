// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdMemory/bdMemory.h>

#include <bdCore/bdUtilities/bdBitOperations.h>
#include <bdCore/bdThread/bdMutex.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>

#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
bdMutex g_MemoryThreadLock;
#endif // BD_MEMORY_FUNCTION_NOT_THREAD_SAFE

bdMemory::bdAllocateFunc bdMemory::m_allocateFunc(BD_NULL);
bdMemory::bdDeallocateFunc bdMemory::m_deallocateFunc(BD_NULL);
bdMemory::bdReallocateFunc bdMemory::m_reallocateFunc(BD_NULL);

bdMemory::bdAlignedAllocateFunc bdMemory::m_alignedAllocateFunc(BD_NULL);
bdMemory::bdAlignedDeallocateFunc bdMemory::m_alignedDeallocateFunc(BD_NULL);
bdMemory::bdAlignedReallocateFunc bdMemory::m_alignedReallocateFunc(BD_NULL);

void BD_CALL bdMemory::setAllocateFunc(const bdAllocateFunc allocator)
{
	m_allocateFunc = allocator;
}

void BD_CALL bdMemory::setDeallocateFunc(const bdDeallocateFunc deallocator)
{
	m_deallocateFunc = deallocator;
}

void BD_CALL bdMemory::setReallocateFunc(const bdReallocateFunc reallocator)
{
	m_reallocateFunc = reallocator;
}

void BD_CALL bdMemory::setAlignedAllocateFunc(const bdAlignedAllocateFunc allocator)
{
	m_alignedAllocateFunc = allocator;
}

void BD_CALL bdMemory::setAlignedDeallocateFunc(const bdAlignedDeallocateFunc deallocator)
{
	m_alignedDeallocateFunc = deallocator;
}

void BD_CALL bdMemory::setAlignedReallocateFunc(const bdAlignedReallocateFunc reallocator)
{
	m_alignedReallocateFunc = reallocator;
}

bdMemory::bdAllocateFunc BD_CALL bdMemory::getAllocateFunc()
{
	return m_allocateFunc;
}
		
bdMemory::bdDeallocateFunc BD_CALL bdMemory::getDeallocateFunc()
{
	return m_deallocateFunc;
}

bdMemory::bdReallocateFunc BD_CALL bdMemory::getReallocateFunc()
{
	return m_reallocateFunc;
}
		
bdMemory::bdAlignedAllocateFunc BD_CALL bdMemory::getAlignedAllocateFunc()
{
	return m_alignedAllocateFunc;
}
		
bdMemory::bdAlignedDeallocateFunc BD_CALL bdMemory::getAlignedDeallocateFunc()
{
	return m_alignedDeallocateFunc;
}

bdMemory::bdAlignedReallocateFunc BD_CALL bdMemory::getAlignedReallocateFunc()
{
	return m_alignedReallocateFunc;
}

void* BD_CALL bdMemory::allocate(const bdUWord size)
{
	void *block = BD_NULL;
	if(m_allocateFunc)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		block = m_allocateFunc(size);

#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.unlock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
				
		if(block == BD_NULL	)
		{
			BD_BREAKPOINT();
		}

		// fixme: If we assert here it may recursively try to allocate memory
		//BD_ASSERT( block != BD_NULL, "bdMemory::allocate, failed to allocate memory.");
	}

	return block;
}

void BD_CALL bdMemory::deallocate(void *p)
{
	if(m_deallocateFunc)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		m_deallocateFunc(p);

#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.unlock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
	}
}

void* BD_CALL bdMemory::reallocate(void *p, 
								   const bdUWord size)
{
	void *block = BD_NULL;
	if(m_reallocateFunc)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		block = m_reallocateFunc(p, size);
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.unlock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		if(block == BD_NULL	)
		{
			BD_BREAKPOINT();
		}

		// fixme: If we assert here it may recursively try to allocate memory
		//BD_ASSERT( block != BD_NULL, "bdMemory::allocate, failed to allocate memory.");

	}

	return block;
}

void* BD_CALL bdMemory::alignedAllocate(const bdUWord size,
										const bdUWord align)
{
	void *block = BD_NULL;
	if(m_alignedAllocateFunc)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		block = m_alignedAllocateFunc(size, align);

#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.unlock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		if(block == BD_NULL	)
		{
			BD_BREAKPOINT();
		}

		// fixme: If we assert here it may recursively try to allocate memory
		//BD_ASSERT( block != BD_NULL, "bdMemory::allocate, failed to allocate memory.");
		BD_ASSERT(BD_IS_MULTIPLE_OF_M(reinterpret_cast<bdUWord>(block), align), "Memory block has incorrect alignment.");		
	}
	return block;
}

void BD_CALL bdMemory::alignedDeallocate(void *p)
{
	if(m_alignedDeallocateFunc)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		m_alignedDeallocateFunc(p);

#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.unlock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
	}
}

void* BD_CALL bdMemory::alignedReallocate(void *p, 
										  const bdUWord size,
										  const bdUWord align)
{
	void *block = BD_NULL;
	if(m_alignedReallocateFunc)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		block = m_alignedReallocateFunc(p, size, align);

#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.unlock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		if(block == BD_NULL	)
		{
			BD_BREAKPOINT();
		}

		// fixme: If we assert here it may recursively try to allocate memory
		//BD_ASSERT( block != BD_NULL, "bdMemory::allocate, failed to allocate memory.");
		BD_ASSERT(BD_IS_MULTIPLE_OF_M(reinterpret_cast<bdUWord>(block), align), "Memory block has incorrect alignment.");		
	}
	return block;
}


bdMemory::bdMemory()
{
}

// define memory handling functions for LibTomCrypt
extern "C" void* BD_CALL libTomCryptMalloc(size_t n)
{
	return bdMemory::allocate(n);
}

extern "C" void* BD_CALL libTomCryptRealloc(void *p, 
									size_t n)
{
	return bdMemory::reallocate(p, n);
}

extern "C" void* BD_CALL libTomCryptCalloc(size_t n, 
								   size_t s)
{
	void *const p = bdMemory::allocate(n*s);
	if(p)
	{
		bdMemset(p, BD_NULL, n*s);
	}
	return p;
}

extern "C" void BD_CALL libTomCryptFree(void *p)
{
	bdMemory::deallocate(p);
}
