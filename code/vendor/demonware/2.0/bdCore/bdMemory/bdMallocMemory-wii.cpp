// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdMemory/bdDefaultMemory.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdCore/bdUtilities/bdBitOperations.h>

#if defined (BD_MALLOC_MEMORY)

#define BD_LOG_LEVEL "mallocmemory"

bdMallocMemory::bdMemoryChainElement *bdMallocMemory::m_memoryChain = BD_NULL;
MEMHeapHandle bdMallocMemory::m_handles[2];

bdUWord bdMallocMemory::m_allocatedBytes = 0;
bdUInt bdMallocMemory::m_numAllocations = 0;

bdMutex bdMallocMemory::m_mutex;

void BD_CALL bdMallocMemory::init()
{
	static bdBool once = false;
	if(!once)
	{
		// MEM1 is used for general allocations, consider this the 'normal' heap
		// Some libraries need to be allocated from MEM1 e.g. graphics
		{
			bdByte8 *lo = (bdByte8*)OSGetMEM1ArenaLo();
			bdByte8 *hi = (bdByte8*)OSGetMEM1ArenaHi();
	
			m_handles[0] = MEMCreateExpHeapEx(lo, hi - lo, MEM_HEAP_OPT_THREAD_SAFE);
			BD_ASSERT(m_handles[0] != MEM_HEAP_INVALID_HANDLE, "Unable to creap heap in MEM1.");
		
			OSSetMEM1ArenaLo(hi);
		}
		// MEM2 is used for contiguous allocations i.e. buffers
		// Some libraries need to be allocated from MEM2 e.g. wii-mote and sockets
		{
			bdByte8 *lo = (bdByte8*)OSGetMEM2ArenaLo();
			bdByte8 *hi = (bdByte8*)OSGetMEM2ArenaHi();
	
			m_handles[1] = MEMCreateExpHeapEx(lo, hi - lo, MEM_HEAP_OPT_THREAD_SAFE);
			BD_ASSERT(m_handles[1] != MEM_HEAP_INVALID_HANDLE, "Unable to creap heap in MEM2.");
	
			OSSetMEM2ArenaLo(hi);
		}
	}
}

void* BD_CALL bdMallocMemory::allocate(const bdUWord size)
{
	const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

	// use 8 byte alignment even for basic malloc
	bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(alignedOffsetMalloc(totalSize, 8, sizeof(bdMemoryChainElement), 1));

	return recordMemory(link, size, false, 1);
}

void* BD_CALL bdMallocMemory::allocate2(const bdUWord size)
{
	const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

	// use 8 byte alignment even for basic malloc
	bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(alignedOffsetMalloc(totalSize, 8, sizeof(bdMemoryChainElement), 2));

	return recordMemory(link, size, false, 2);
}

void BD_CALL bdMallocMemory::deallocate(void *p)
{
	if(p)
	{
		bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(p) - 1;

		BD_ASSERT(link->m_aligned == false, "Memory block allocated as aligned "
											"but is beign deallocated with bdMallocMemory::deallocate. "
											"Use bdMallocMemory::alignedDeallocate instead.");

		eraseMemory(link);
		alignedOffsetFree(link);
	}
}

void* BD_CALL bdMallocMemory::reallocate(void *p,
										 const bdUWord size)
{	
	if(p)
	{
		bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(p) - 1;
		const bdUWord origSize = link->m_size + sizeof(bdMemoryChainElement);
		eraseMemory(link);

		const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

		// use 8 byte alignment even for basic realloc
		link = reinterpret_cast<bdMemoryChainElement*>(alignedOffsetRealloc(link, origSize, totalSize, 8, sizeof(bdMemoryChainElement), 1));

		return recordMemory(link, size, false, 1);
	}
	else
	{
		return allocate(size);
	}
}

void* BD_CALL bdMallocMemory::reallocate2(void *p,
										 const bdUWord size)
{	
	if(p)
	{
		bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(p) - 1;
		const bdUWord origSize = link->m_size + sizeof(bdMemoryChainElement);
		eraseMemory(link);

		const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

		// use 8 byte alignment even for basic realloc
		link = reinterpret_cast<bdMemoryChainElement*>(alignedOffsetRealloc(link, origSize, totalSize, 8, sizeof(bdMemoryChainElement), 2));

		return recordMemory(link, size, false, 2);
	}
	else
	{
		return allocate2(size);
	}
}

void* BD_CALL bdMallocMemory::alignedAllocate(const bdUWord size, 
											  const bdUWord align)
{
	const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

	// use alignedOffsetMalloc here to ensure the returned pointer is
	// correctly aligned rather than the address of the link.
	bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(alignedOffsetMalloc(totalSize, align, sizeof(bdMemoryChainElement), 1));

	return recordMemory(link, size, true, 1);
}

void* BD_CALL bdMallocMemory::alignedAllocate2(const bdUWord size, 
											  const bdUWord align)
{
	const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

	// use alignedOffsetMalloc here to ensure the returned pointer is
	// correctly aligned rather than the address of the link.
	bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(alignedOffsetMalloc(totalSize, align, sizeof(bdMemoryChainElement), 2));

	return recordMemory(link, size, true, 2);
}

void BD_CALL bdMallocMemory::alignedDeallocate(void *p)
{
	if(p)
	{
		bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(p) - 1;
		
		BD_ASSERT(link->m_aligned == true, "Memory block allocated unaligned "
										   "but is beign deallocated with bdMallocMemory::alignedDeallocate. "
										   "Use bdMallocMemory::deallocate instead.");

		eraseMemory(link);

		alignedOffsetFree(link);
	}
}

void* BD_CALL bdMallocMemory::alignedReallocate(void *p,
												const bdUWord size,
												const bdUWord align)
{
	if(p)
	{
		bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(p) - 1;
		const bdUWord origSize = link->m_size + sizeof(bdMemoryChainElement);
		eraseMemory(link);

		const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

		link = reinterpret_cast<bdMemoryChainElement*>(alignedOffsetRealloc(link, origSize, totalSize, align, sizeof(bdMemoryChainElement), 1));

		return recordMemory(link, size, true, 1);
	}
	else
	{
		return alignedAllocate(size, align);
	}
}

void* BD_CALL bdMallocMemory::alignedReallocate2(void *p,
											     const bdUWord size,
												 const bdUWord align)
{
	if(p)
	{
		bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(p) - 1;
		const bdUWord origSize = link->m_size + sizeof(bdMemoryChainElement);
		eraseMemory(link);

		const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

		link = reinterpret_cast<bdMemoryChainElement*>(alignedOffsetRealloc(link, origSize, totalSize, align, sizeof(bdMemoryChainElement), 2));

		return recordMemory(link, size, true, 2);
	}
	else
	{
		return alignedAllocate2(size, align);
	}
}

void BD_CALL bdMallocMemory::leakCheck()
{
	if(m_allocatedBytes)
	{
		m_mutex.lock();
		bdNChar8 buf[100];
		bdSnprintf(buf, 100, "%u Bytes leaked in %u allocation(s)\n", m_allocatedBytes, m_numAllocations);
		m_mutex.unlock();

#if defined (BD_PLATFORM_WIN32)

		OutputDebugString("********************************************\n");
		OutputDebugString("*      BITDEMON MEMORY LEAKS DETECTED      *\n");
		OutputDebugString("********************************************\n");
		OutputDebugString(buf);
		OutputDebugString("********************************************\n");

#else // defined (BD_PLATFORM_WIN32)

		bdFprintf(BD_STDERR, "********************************************\n");
		bdFprintf(BD_STDERR, "*      BITDEMON MEMORY LEAKS DETECTED      *\n");
		bdFprintf(BD_STDERR, "********************************************\n");
		bdFprintf(BD_STDERR, buf);
		bdFprintf(BD_STDERR, "********************************************\n");

#endif // defined (BD_PLATFORM_WIN32)
	}
}

void BD_CALL bdMallocMemory::releaseAllMemory()
{
	m_mutex.lock();
	while(m_memoryChain)
	{
		bdMemoryChainElement *const link = m_memoryChain;

		eraseMemory(link);
		
		alignedOffsetFree(link);

	}
	m_mutex.unlock();
}

void* BD_CALL bdMallocMemory::recordMemory(bdMemoryChainElement *link,
										   const bdUWord size,
										   const bdBool aligned,
										   const bdUByte8 heapNum)				   
{
	if(link)
	{
		m_mutex.lock();
		link->m_magic = BD_MEMORY_MAGIC;
		link->m_size = size;
		link->m_aligned = aligned;
		link->m_heapNum = heapNum;
		link->m_next = m_memoryChain;
		link->m_prev = BD_NULL;
		if(m_memoryChain)
		{
			m_memoryChain->m_prev = link;
		}
		m_memoryChain = link;

		m_allocatedBytes += size;
		m_numAllocations++;

		m_mutex.unlock();
		return reinterpret_cast<void*>(link+1);
	}

	return BD_NULL;
}

void BD_CALL bdMallocMemory::eraseMemory(bdMemoryChainElement *link)
{
	m_mutex.lock();
	if(link->m_magic != BD_MEMORY_MAGIC)
	{
		// log might cause memory allocation...
		m_mutex.unlock();
		BD_ERR(BD_LOG_LEVEL, " BD_MEMORY_MAGIC is incorrect.");
		m_mutex.lock();
	}

	if(link->m_prev)
	{
		link->m_prev->m_next = link->m_next;
	}
	else
	{
		m_memoryChain = link->m_next;
	}

	if(link->m_next)
	{
		link->m_next->m_prev = link->m_prev;
	}

	m_allocatedBytes -= link->m_size;
	m_numAllocations--;
	m_mutex.unlock();
}

void* BD_CALL bdMallocMemory::alignedOffsetMalloc(const bdUWord size,
												  const bdUWord align,
												  const bdUWord offset,
												  const bdUByte8 heapNum)
{
	if(!BD_IS_POWER_OF_2(align))
	{	
		BD_ASSERT(BD_IS_POWER_OF_2(align), "alignedOffsetMalloc, alignment must "
										   "a power of 2.");
		return BD_NULL;
	}

	BD_ASSERT(heapNum == 1 || heapNum == 2, "Invalid memory heap handle: %u.", heapNum);

	const bdUWord padding = align + sizeof(void*) + offset;
	const bdUWord blockPtr = reinterpret_cast<bdUWord>(MEMAllocFromExpHeap(m_handles[heapNum-1], padding + size));
	
	if(!blockPtr)
	{
		return BD_NULL;
	}

	const bdUWord alignedPtr = BD_PREVIOUS_MULTIPLE_OF_M(blockPtr + padding, align);
	const bdUWord dataPtr = alignedPtr - offset;

	// store the original blockPtr before the dataPtr
	bdUWord *const headerPtr =  reinterpret_cast<bdUWord*>(dataPtr - sizeof(void*));
	*headerPtr = blockPtr;

	// check the alignment
	if(!BD_IS_MULTIPLE_OF_M(alignedPtr, align))
	{
		BD_ASSERT(BD_IS_MULTIPLE_OF_M(dataPtr, align), "alignedOffsetMalloc, incorrect alignment.");
	}
	
	return reinterpret_cast<void*>(dataPtr);
}

void BD_CALL bdMallocMemory::alignedOffsetFree(bdMemoryChainElement *link)
{
	// get the pointer originally returned by bdPlatformMalloc
	const bdUWord dataPtr = reinterpret_cast<bdUWord>(link);
	const bdUWord *const headerPtr =  reinterpret_cast<bdUWord*>(dataPtr - sizeof(void*));
	void *const blockPtr = reinterpret_cast<void*>(*headerPtr);

	BD_ASSERT(link->m_heapNum == 1 || link->m_heapNum == 2, "Invalid memory heap handle: %u.", link->m_heapNum);

	MEMFreeToExpHeap(m_handles[link->m_heapNum-1], blockPtr);
}

void* BD_CALL bdMallocMemory::alignedOffsetRealloc(bdMemoryChainElement *link,
												   const bdUWord origSize,
												   const bdUWord size,
												   const bdUWord align,
												   const bdUWord offset,
												   const bdUByte8 heapNum)
{
	void *const dataPtr = alignedOffsetMalloc(size, align, offset, heapNum);
	bdMemcpy(dataPtr, link, BD_MIN(size, origSize));
	alignedOffsetFree(link);
	return dataPtr;
}

bdMallocMemory::bdMallocMemory()
{
}

#endif // defined (BD_MALLOC_MEMORY)
