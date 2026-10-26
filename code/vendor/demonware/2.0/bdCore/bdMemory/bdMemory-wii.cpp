// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdMemory/bdMemory.h>

#include <bdCore/bdUtilities/bdBitOperations.h>
#include <bdCore/bdThread/bdMutex.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>

#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
extern bdMutex g_MemoryThreadLock;
#endif // BD_MEMORY_FUNCTION_NOT_THREAD_SAFE

bdMemory::bdAllocateFunc bdMemory::m_allocateFunc2(BD_NULL);
bdMemory::bdDeallocateFunc bdMemory::m_deallocateFunc2(BD_NULL);
bdMemory::bdReallocateFunc bdMemory::m_reallocateFunc2(BD_NULL);

bdMemory::bdAlignedAllocateFunc bdMemory::m_alignedAllocateFunc2(BD_NULL);
bdMemory::bdAlignedDeallocateFunc bdMemory::m_alignedDeallocateFunc2(BD_NULL);
bdMemory::bdAlignedReallocateFunc bdMemory::m_alignedReallocateFunc2(BD_NULL);

void BD_CALL bdMemory::setAllocateFunc2(const bdAllocateFunc allocator)
{
	m_allocateFunc2 = allocator;
}

void BD_CALL bdMemory::setDeallocateFunc2(const bdDeallocateFunc deallocator)
{
	m_deallocateFunc2 = deallocator;
}

void BD_CALL bdMemory::setReallocateFunc2(const bdReallocateFunc reallocator)
{
	m_reallocateFunc2 = reallocator;
}

void BD_CALL bdMemory::setAlignedAllocateFunc2(const bdAlignedAllocateFunc allocator)
{
	m_alignedAllocateFunc2 = allocator;
}

void BD_CALL bdMemory::setAlignedDeallocateFunc2(const bdAlignedDeallocateFunc deallocator)
{
	m_alignedDeallocateFunc2 = deallocator;
}

void BD_CALL bdMemory::setAlignedReallocateFunc2(const bdAlignedReallocateFunc reallocator)
{
	m_alignedReallocateFunc2 = reallocator;
}

bdMemory::bdAllocateFunc BD_CALL bdMemory::getAllocateFunc2()
{
	return m_allocateFunc2;
}
		
bdMemory::bdDeallocateFunc BD_CALL bdMemory::getDeallocateFunc2()
{
	return m_deallocateFunc2;
}

bdMemory::bdReallocateFunc BD_CALL bdMemory::getReallocateFunc2()
{
	return m_reallocateFunc2;
}
		
bdMemory::bdAlignedAllocateFunc BD_CALL bdMemory::getAlignedAllocateFunc2()
{
	return m_alignedAllocateFunc2;
}
		
bdMemory::bdAlignedDeallocateFunc BD_CALL bdMemory::getAlignedDeallocateFunc2()
{
	return m_alignedDeallocateFunc2;
}

bdMemory::bdAlignedReallocateFunc BD_CALL bdMemory::getAlignedReallocateFunc2()
{
	return m_alignedReallocateFunc2;
}

void* BD_CALL bdMemory::allocate2(const bdUWord size)
{
	void *block = BD_NULL;
	if(m_allocateFunc2)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		block = m_allocateFunc2(size);

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

void BD_CALL bdMemory::deallocate2(void *p)
{
	if(m_deallocateFunc2)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		m_deallocateFunc2(p);

#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.unlock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
	}
}

void* BD_CALL bdMemory::reallocate2(void *p, 
								    const bdUWord size)
{
	void *block = BD_NULL;
	if(m_reallocateFunc2)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		block = m_reallocateFunc2(p, size);

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

void* BD_CALL bdMemory::alignedAllocate2(const bdUWord size,
										 const bdUWord align)
{
	void *block = BD_NULL;
	if(m_alignedAllocateFunc2)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		block = m_alignedAllocateFunc2(size, align);

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

void BD_CALL bdMemory::alignedDeallocate2(void *p)
{
	if(m_alignedDeallocateFunc2)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		m_alignedDeallocateFunc2(p);

#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.unlock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
	}
}

void* BD_CALL bdMemory::alignedReallocate2(void *p, 
										   const bdUWord size,
										   const bdUWord align)
{
	void *block = BD_NULL;
	if(m_alignedReallocateFunc2)
	{
#ifdef BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
		g_MemoryThreadLock.lock();
#endif // BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE

		block = m_alignedReallocateFunc2(p, size, align);

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
