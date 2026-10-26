// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-ADHOC PSP-INFRA PS3 XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdMemory/bdDefaultMemory.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdCore/bdMemory/bdAlignedOffsetMemory.h>

#if defined (BD_MALLOC_MEMORY)

#if defined (BD_PLATFORM_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif // defined (BD_PLATFORM_WIN32)

#define BD_LOG_LEVEL "mallocmemory"

bdMallocMemory::bdMemoryChainElement *bdMallocMemory::m_memoryChain = BD_NULL;
bdUWord bdMallocMemory::m_allocatedBytes = 0;
bdUInt bdMallocMemory::m_numAllocations = 0;

bdMutex bdMallocMemory::m_mutex;

void* BD_CALL bdMallocMemory::allocate(const bdUWord size)
{
	const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

	// use 8 byte alignment even for basic malloc
	bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(bdAlignedOffsetMalloc(totalSize, 8, sizeof(bdMemoryChainElement)));

	return recordMemory(link, size, false);
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
		bdAlignedOffsetFree(link);
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
		link = reinterpret_cast<bdMemoryChainElement*>(bdAlignedOffsetRealloc(link, origSize, totalSize, 8, sizeof(bdMemoryChainElement)));

		return recordMemory(link, size, false);
	}
	else
	{
		return allocate(size);
	}
}

void* BD_CALL bdMallocMemory::alignedAllocate(const bdUWord size, 
											  const bdUWord align)
{
	const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

	// use bdAlignedOffsetMalloc here to ensure the returned pointer is
	// correctly aligned rather than the address of the link.
	bdMemoryChainElement *link =  reinterpret_cast<bdMemoryChainElement*>(bdAlignedOffsetMalloc(totalSize, align, sizeof(bdMemoryChainElement)));

	return recordMemory(link, size, true);
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

		bdAlignedOffsetFree(link);
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

		link = reinterpret_cast<bdMemoryChainElement*>(bdAlignedOffsetRealloc(link, origSize, totalSize, align, sizeof(bdMemoryChainElement)));

		return recordMemory(link, size, true);
	}
	else
	{
		return alignedAllocate(size, align);
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
		
		// all memory is allocated through bdAlignedOffsetMalloc
		bdAlignedOffsetFree(link);
	}
	m_mutex.unlock();
}

void* BD_CALL bdMallocMemory::recordMemory(bdMemoryChainElement *link,
										   const bdUWord size,
										   const bdBool aligned)				   
{
	if(link)
	{
		m_mutex.lock();
		link->m_magic = BD_MEMORY_MAGIC;
		link->m_size = size;
		link->m_aligned = aligned;
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

bdMallocMemory::bdMallocMemory()
{
}

#endif // defined (BD_MALLOC_MEMORY)
