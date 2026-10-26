namespace Ut
{

template<typename Allocator, typename ThreadingModel>
void BlockAllocator<Allocator, ThreadingModel>::Initialise(U32 blockSize, U32 totalSizeInBytes)
{
	assert(m_totalSize == 0);
	m_totalSize = totalSizeInBytes;
	m_blockSize = blockSize;

	// reserve memory
	m_buffer.Reserve(m_totalSize, 0);

	assert(m_buffer.Capacity() > 0);

	m_freeList.Insert(MemoryMap::DataType(&m_buffer[0], m_totalSize));

	m_totalAllocated = 0;
	m_allocationCount = 0;
	m_failedCount = 0;
	//m_current = m_freeList.begin();
}
	
template<typename Allocator, typename ThreadingModel>
void BlockAllocator<Allocator, ThreadingModel>::Shutdown()
{
	assert(m_totalAllocated == 0);
}

template<typename Allocator, typename ThreadingModel>
__forceinline U32 BlockAllocator<Allocator, ThreadingModel>::AllocationSize(U32 size) const
{
	if (size % m_blockSize == 0)
		return size;
	else
		return (size / m_blockSize + 1) * m_blockSize;
}

/*
template<typename Allocator, typename ThreadingModel>
__forceinline void MemoryAllocator<Allocator, ThreadingModel>::IncrementCurrent()
{
	if (m_current != m_freeList.end())
	{
		m_current++;

		if (m_current == m_freeList.end())
			m_current = m_freeList.begin();
	}
}
*/

template<typename Allocator, typename ThreadingModel>
__forceinline void BlockAllocator<Allocator, ThreadingModel>::RemoveFromFreeList(MemoryMap::Iterator i)
{
	//if (m_current == i)
	//	IncrementCurrent();

	m_freeList.Erase(i);
}

template<typename Allocator, typename ThreadingModel>
__forceinline BlockAllocator<Allocator, ThreadingModel>::MemoryMap::Iterator BlockAllocator<Allocator, ThreadingModel>::AddToFreeList(U8* p, U32 size)
{
	Ut::Pair<MemoryMap::Iterator, bool> result = m_freeList.Insert(MemoryMap::DataType(p, size));
	assert(result.second && "Memory Allocator: Block already in free list.");

	//if (m_current == m_freeList.end())
	//	m_current = result.first;

	return result.first;
}

template<typename Allocator, typename ThreadingModel>
__forceinline U8* BlockAllocator<Allocator, ThreadingModel>::AllocateFrom(MemoryMap::Iterator i, U32 size)
{
	U8* p = 0;
	U32 blockSize = i->GetValue();

	if (blockSize >= size)
	{
		p = i->GetKey();
		
		if (blockSize > size)
			m_freeList.Insert(i, MemoryMap::DataType(p + size, blockSize - size));

		RemoveFromFreeList(i);
	}

	return p;
}

template<typename Allocator, typename ThreadingModel>
U8* BlockAllocator<Allocator, ThreadingModel>::Allocate(U32 sizeInBytes)
{
	ThreadingModel::Lock lock(m_threadData);

	U8* p = 0;
	U32 size = AllocationSize(sizeInBytes);

	// first fit
	for (MemoryMap::Iterator i = m_freeList.Begin(); i != m_freeList.End(); ++i)
	{
		p = AllocateFrom(i, size);
		if (p) break;
	}

	// next fit
	/*if (m_current != m_freeList.end())
	{
		MemoryMap::iterator last = m_current;

		do 
		{
			p = AllocateFrom(m_current, size);
			if (p) break;

			IncrementCurrent();

		} while (m_current != last);
	}*/

	if (p)
	{
		m_allocationCount++;
		m_totalAllocated += size;
		m_totalRequested += sizeInBytes;

		// debug
		//std::pair<MemoryMap::iterator, bool> result = m_usedList.insert(MemoryMap::value_type(p, size));
		//assert(result.second && "Memory Allocator: Block already in used list.");
	}
	else
	{
		m_failedCount++;
	}

	return p;
}

// may erase entry passed in
template<typename Allocator, typename ThreadingModel>
__forceinline void BlockAllocator<Allocator, ThreadingModel>::MergeWithPrevious(MemoryMap::Iterator entry)
{
	MemoryMap::Iterator i = entry;

	if (i != m_freeList.Begin())
	{
		--i;
		if (i->GetKey() + i->GetValue() == entry->GetKey())
		{
			i->GetValue() += entry->GetValue();
			RemoveFromFreeList(entry);
		}
	}
}

template<typename Allocator, typename ThreadingModel>
__forceinline void BlockAllocator<Allocator, ThreadingModel>::MergeWithNext(MemoryMap::Iterator entry)
{
	MemoryMap::Iterator i = entry;
	++i;

	if (i != m_freeList.End())
	{
		if (entry->GetKey() + entry->GetValue() == i->GetKey())
		{
			entry->GetValue() += i->GetValue();
			RemoveFromFreeList(i);
		}
	}
}

template<typename Allocator, typename ThreadingModel>
void BlockAllocator<Allocator, ThreadingModel>::Free(U8* p, U32 sizeInBytes)
{
	if (!p) return;

	U32 size = AllocationSize(sizeInBytes);

	ThreadingModel::Lock lock(m_threadData);

	// debug
	/*{
		MemoryMap::iterator found = m_usedList.find(p);

		if (found == m_usedList.end())
			assert(0 && "Memory Allocator: Block not in used list.");
		else
		{
			assert(found->second == size);
			m_usedList.erase(found);
		}
	}*/

	MemoryMap::Iterator result = AddToFreeList(p, size);
	
	MergeWithNext(result);
	MergeWithPrevious(result);

	m_allocationCount--;
	m_totalAllocated -= size;
	m_totalRequested -= sizeInBytes;
}

template<typename Allocator, typename ThreadingModel>
void BlockAllocator<Allocator, ThreadingModel>::GetStats(U32& count, U32& largest, U32& totalFree) const
{
	ThreadingModel::Lock lock(m_threadData);

	count = 0;
	largest = 0;
	totalFree = 0;

	for (MemoryMap::ConstIterator i = m_freeList.Begin(); i != m_freeList.End(); ++i)
	{
		count++;
		totalFree += i->GetValue();
		if (i->GetValue() > largest) largest = i->GetValue();
	}
}

template<typename Allocator, typename ThreadingModel>
void BlockAllocator<Allocator, ThreadingModel>::PrintDebug(Debug::Logger& log) const
{
	ThreadingModel::Lock lock(m_threadData);

	U32 count, largest, totalFree;
	GetStats(count, largest, totalFree);

	LOGDEBUG(log, "Memory Allocator Debug:");
	LOGDEBUG(log, "Total allocated: %i k", m_totalAllocated / 1024);
	LOGDEBUG(log, "Total requested: %i k", m_totalRequested / 1024);
	LOGDEBUG(log, "Total free: %i k (%0.1f percent)", totalFree / 1024, (totalFree * 100.f) / m_totalSize);
	LOGDEBUG(log, "Number of free blocks: %i", count);
	LOGDEBUG(log, "Largest block free: %i k", largest / 1024);
}

}
