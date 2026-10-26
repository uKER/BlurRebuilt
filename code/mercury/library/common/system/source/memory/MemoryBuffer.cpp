#include "SystemPch.h"
#include "system/memory/memorybuffer.h"
#include "core/maths/mathsutil.h"
#include <system/memory/MemoryTracker.h>

//#define DEBUG_BLOCK_ALLOCATOR

//#pragma optimize("", off)

namespace Sys
{

MemoryBuffer::MemoryBuffer() : m_totalSize(0), m_blockSize(0)
{
	m_totalAllocated = 0;
	m_allocationCount = 0;
	m_failedCount = 0;
	m_pMemoryAllocated = 0;
}

MemoryBuffer::~MemoryBuffer() 
{
	Shutdown();
}

U32 BitCount(U32 v)
{    
	v = v - ((v >> 1) & 0x55555555);
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
	return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

S32 nlz(U32 x)
{
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
    return(32 - BitCount(x));
}

S32 GetLevel(S32 nBlocks)
{
	S32 level = 31 - nlz(nBlocks);
	return level;
}

bool MemoryBuffer::Initialise(U8* pBuffer, U32 sizeInBytes, U32 blockSize, U32 numQuickLists)
{
	BIZ_ASSERT(pBuffer);
	BIZ_ASSERT(m_totalSize == 0);
	BIZ_ASSERT(m_pMemoryAllocated == 0);

	m_totalSize = sizeInBytes;
	m_blockSize = blockSize;
	m_pBuffer = pBuffer;
	m_pMemoryAllocated = 0;

	m_freeList.Insert(MemoryMap::DataType(m_pBuffer, m_totalSize), GetLevel(m_totalSize / m_blockSize));

	//m_quickFitLists.Resize(numQuickLists + 1);

	// counters
	m_totalAllocated = 0;
	m_allocationCount = 0;
	m_failedCount = 0;

/*#ifdef DEBUG_BLOCK_ALLOCATOR
	m_allocationCountList.Resize(1024);

	for (int i = 0; i < m_allocationCountList.Size(); i++)
	{
		m_allocationCountList[i].fast = 0;
		m_allocationCountList[i].slow = 0;
		m_allocationCountList[i].current = 0;
	}

	m_slowAllocations = 0;
	m_quickAllocations = 0;
#endif*/

	return true;
}

bool MemoryBuffer::Initialise(U32 sizeInBytes, U32 blockSize, U32 numQuickLists)
{
	BIZ_ASSERT(m_totalSize == 0);
	BIZ_ASSERT(m_pMemoryAllocated == 0);
	
	m_totalSize = sizeInBytes;
	m_blockSize = blockSize;

	m_pMemoryAllocated = new U8[sizeInBytes];
	m_pBuffer = m_pMemoryAllocated;

	m_freeList.Insert(MemoryMap::DataType(m_pBuffer, m_totalSize), GetLevel(m_totalSize / m_blockSize));

	//m_quickFitLists.Resize(numQuickLists + 1);

	// counters
	m_totalAllocated = 0;
	m_allocationCount = 0;
	m_failedCount = 0;

/*#ifdef DEBUG_BLOCK_ALLOCATOR
	m_allocationCountList.Resize(1024);

	for (int i = 0; i < m_allocationCountList.Size(); i++)
	{
		m_allocationCountList[i].fast = 0;
		m_allocationCountList[i].slow = 0;
		m_allocationCountList[i].current = 0;
	}

	m_slowAllocations = 0;
	m_quickAllocations = 0;
#endif*/

	return true;
}

void MemoryBuffer::Shutdown()
{
	BIZ_ASSERT(m_totalAllocated == 0);
	delete m_pMemoryAllocated;
	m_pMemoryAllocated = 0;
}

U8* MemoryBuffer::Allocate(U32 sizeInBytes, U32 alignment)
{
	MEMORY_TRACKER_PUSH("mercury.memorybuffer");

	BIZ_ASSERT(m_blockSize > 0);

	U8* p = 0;
	U32 size = Btl::AlignUp(sizeInBytes, m_blockSize);

	if (sizeInBytes == 0)
		return 0;

	BIZ_ASSERT(alignment > 0);
	
	// quick fit
	/*int quickListIndex = size / m_blockSize;

	if (alignment < m_blockSize && quickListIndex < m_quickFitLists.Size())
	{
		if (!m_quickFitLists[quickListIndex].Empty())
		{
			p = m_quickFitLists[quickListIndex].Back();
			m_quickFitLists[quickListIndex].PopBack();

#ifdef DEBUG_BLOCK_ALLOCATOR
			m_quickAllocations++;

			if (quickListIndex < m_allocationCountList.Size())
				m_allocationCountList[quickListIndex].fast++;
#endif
		}
		else 
		{
			S32 n = m_blockSize * 64 / size;

			// Allocate space for n items, return 1 and put the rest on the quick list.
			for (MemoryMap::Iterator i = m_freeList.Begin(); i != m_freeList.End(); ++i)
			{
				p = AllocateFrom(i, size * n, alignment);

				if (p)
				{
					for (int i = 1; i < n; i++)
						m_quickFitLists[quickListIndex].PushFront(p + size * i);

					break;
				}
			}
		}
	}*/

	if (p == 0)
	{
		// first fit
		for (MemoryMap::Iterator i = m_freeList.Begin(GetLevel(size / m_blockSize)); i != m_freeList.End(); ++i)
		{
			if (i->GetValue() >= size)
			{
				p = AllocateFrom(i, size, alignment);
				if (p) break;
			}
		}

/*#ifdef DEBUG_BLOCK_ALLOCATOR
		if (p)
		{
			if (quickListIndex < m_allocationCountList.Size())
				m_allocationCountList[quickListIndex].slow++;

			m_slowAllocations++;
		}
#endif*/
		
	}

	if (p)
	{
		m_allocationCount++;
		m_totalAllocated += size;
		m_totalRequested += sizeInBytes;

/*#ifdef DEBUG_BLOCK_ALLOCATOR
		if (quickListIndex < m_allocationCountList.Size())
			m_allocationCountList[quickListIndex].current++;
#endif*/
	}
	else
	{
		//bool retry = ClearQuickLists();

		//if (retry)
		//	return Allocate(sizeInBytes, alignment);
		//else
			m_failedCount++;
	}

/*#ifdef DEBUG_BLOCK_ALLOCATOR
	if (0) // clear counts
	{
		for (int i = 0; i < m_allocationCountList.Size(); i++)
		{
			m_allocationCountList[i].fast = 0;
			m_allocationCountList[i].slow = 0;
			m_allocationCountList[i].current = 0;
		}

		//m_slowAllocations = 0;
		//m_quickAllocations = 0;
	}
#endif*/

#ifdef VERIFY_MEMORY_BUFFER_INTEGRITY
	if (p)
	{
		MemoryMap::InsertionReturnType result = m_allocatedList.Insert(MemoryMap::DataType(p, sizeInBytes));
		BIZ_ASSERT_MSG_SPEED(result.inserted, "Memory Allocator: Block already allocated");
	}
#endif

	MEMORY_TRACKER_POP("mercury.memorybuffer");

	return p;
}

void MemoryBuffer::Free(U8* p, U32 sizeInBytes)
{
	if (!p) return;

#ifdef VERIFY_MEMORY_BUFFER_INTEGRITY
	MemoryMap::Iterator mmi = m_allocatedList.Find(p);
	BIZ_ASSERT_SPEED(mmi != m_allocatedList.End());
	BIZ_ASSERT_SPEED(mmi->GetValue() == sizeInBytes);
	m_allocatedList.Erase(p);
#endif

	MEMORY_TRACKER_PUSH("mercury.memorybuffer");

	U32 size = Btl::AlignUp(sizeInBytes, m_blockSize);

	/*int quickListIndex = size / m_blockSize;
	int nBlocks = m_totalSize / m_blockSize;
	int blocksPerList = 1024;
	int quickListSize = Maths::Clamped(blocksPerList / quickListIndex, 1, 1024);

	if (quickListIndex < m_quickFitLists.Size() 
		&& m_quickFitLists[quickListIndex].Size() < quickListSize)
	{
		m_quickFitLists[quickListIndex].PushFront(p);
	}
	else*/
	{
		MemoryMap::InsertionReturnType result = m_freeList.Insert(MemoryMap::DataType(p, size), GetLevel(size / m_blockSize));
		assert(result.inserted && "Memory Allocator: Block already in free list.");
		MergeWithNext(result.iterator);
		MergeWithPrevious(result.iterator);
	}

/*#ifdef DEBUG_BLOCK_ALLOCATOR
	if (quickListIndex < m_allocationCountList.Size())
		m_allocationCountList[quickListIndex].current--;
#endif*/

	m_allocationCount--;
	m_totalAllocated -= size;
	m_totalRequested -= sizeInBytes;

	MEMORY_TRACKER_POP("mercury.memorybuffer");
}

/*bool MemoryBuffer::ClearQuickLists()
{
	bool retry = false;

	for (int i = 0; i < m_quickFitLists.Size(); i++)
	{
		S32 size = m_blockSize * i;

		while (!m_quickFitLists[i].Empty())
		{
			U8* p = m_quickFitLists[i].Back();
			m_quickFitLists[i].PopBack();
			MemoryMap::InsertionReturnType result = m_freeList.Insert(MemoryMap::DataType(p, size), GetLevel(i));
			assert(result.inserted && "Memory Allocator: Block already in free list.");
			MergeWithNext(result.iterator);
			MergeWithPrevious(result.iterator);
			retry = true;
		}
	}

	return retry;
}*/

U32 MemoryBuffer::GetTotalSize() const 
{
	return m_totalSize;
}

U32 MemoryBuffer::GetTotalAllocated() const 
{
	return m_totalAllocated;
}

U32 MemoryBuffer::GetTotalFree() const 
{
	return m_totalSize - m_totalAllocated;
}

U32 MemoryBuffer::GetFailedCount() const 
{
	return m_failedCount;
}

U32 MemoryBuffer::GetTotalRequested() const 
{
	return m_totalRequested;
} 

const U8* MemoryBuffer::BufferBegin() const
{
	return m_pBuffer;
}

const U8* MemoryBuffer::BufferEnd() const
{
	return BufferBegin() + m_totalSize;
}

U32 MemoryBuffer::BlockSize() const
{
	return m_blockSize;
}

void MemoryBuffer::PrintDebug(Debug::Logger& log) const
{
	U32 count, largest, totalFree, totalQuick;
	GetStats(count, largest, totalFree, totalQuick);

	LOGDEBUG(log, "Memory Allocator Debug:");
	LOGDEBUG(log, "Total allocated: %i k", m_totalAllocated / 1024);
	LOGDEBUG(log, "Total requested: %i k", m_totalRequested / 1024);
	LOGDEBUG(log, "Total free: %i k (%0.1f percent)", totalFree / 1024, (totalFree * 100.f) / m_totalSize);
	//LOGDEBUG(log, "Total quick: %i k", totalQuick / 1024);
	LOGDEBUG(log, "Number of free blocks: %i", count);
	LOGDEBUG(log, "Largest block free: %i k", largest / 1024);
}

void MemoryBuffer::GetStats(U32& count, U32& largest, U32& totalFree, U32& totalQuick) const
{
	count = 0;
	largest = 0;
	totalFree = 0;
	//totalQuick = 0;

	for (MemoryMap::ConstIterator i = m_freeList.Begin(); i != m_freeList.End(); ++i)
	{
		count++;
		totalFree += i->GetValue();
		if (i->GetValue() > largest) largest = i->GetValue();
	}

	/*for (int i = 0; i < m_quickFitLists.Size(); i++)
	{
		int n = m_quickFitLists[i].Size();
		U32 size = m_blockSize * i;

		count += n;
		totalFree += n * size;
		totalQuick += n * size;
		if (size > largest) largest = size;
	}*/
}

void MemoryBuffer::GetFreeList(Btl::Vector<MemoryBlock>& blocks) const
{
	for (MemoryMap::ConstIterator i = m_freeList.Begin(); i != m_freeList.End(); ++i)
	{
		blocks.PushBack();
		blocks.Back().size = i->GetValue();
		blocks.Back().address = i->GetKey();
		//blocks.Back().quick = false;
	}

	/*for (int i = 0; i < m_quickFitLists.Size(); i++)
	{
		for (QuickList::ConstIterator it = m_quickFitLists[i].Begin(); it !=  m_quickFitLists[i].End(); ++it)
		{
			blocks.PushBack();
			blocks.Back().size = m_blockSize * i;
			blocks.Back().address = *it;
			blocks.Back().quick = true;
		}
	}*/
}

void MemoryBuffer::MergeWithPrevious(MemoryMap::Iterator entry)
{
	MemoryMap::Iterator i = entry;

	if (i != m_freeList.Begin())
	{
		--i;

		if (i->GetKey() + i->GetValue() == entry->GetKey())
		{
			S32 newSize = i->GetValue() + entry->GetValue();
			U8* p = i->GetKey();

			m_freeList.Erase(entry);
			m_freeList.Erase(i);
			m_freeList.Insert(MemoryMap::DataType(p, newSize), GetLevel(newSize / m_blockSize));
		}
	}
}

void MemoryBuffer::MergeWithNext(MemoryMap::Iterator entry)
{
	MemoryMap::Iterator i = entry;
	++i;

	if (i != m_freeList.End())
	{
		if (entry->GetKey() + entry->GetValue() == i->GetKey())
		{
			S32 newSize = i->GetValue() + entry->GetValue();
			U8* p = entry->GetKey();

			m_freeList.Erase(i);
			m_freeList.Erase(entry);
			m_freeList.Insert(MemoryMap::DataType(p, newSize), GetLevel(newSize / m_blockSize));
		}
	}
}

U8* MemoryBuffer::AllocateFrom(MemoryMap::Iterator i, U32 size, U32 alignment)
{
	U8* p = i->GetKey();
	U32 blockSize = i->GetValue();

	U8* pAligned = Btl::AlignUp(p, alignment);
	U32 alignedOffset = pAligned - p;
	U32 endOffset = alignedOffset + size;

	if (blockSize >= endOffset)
	{
		p = i->GetKey();
		
		if (blockSize > endOffset)
		{
			S32 nBlocks = (blockSize - endOffset) / m_blockSize;
			m_freeList.Insert(MemoryMap::DataType(p + endOffset, blockSize - endOffset), GetLevel(nBlocks));
		}

		m_freeList.Erase(i);

		if (alignedOffset > 0)
			m_freeList.Insert(MemoryMap::DataType(p, alignedOffset), GetLevel(alignedOffset / m_blockSize));

		return pAligned;
	}

	return 0;
}

}
