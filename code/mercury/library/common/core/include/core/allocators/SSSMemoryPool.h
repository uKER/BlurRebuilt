#ifndef UT_MEMORY_POOL_H
#define UT_MEMORY_POOL_H


namespace Ut
{
	#define UT_SSS_MEMORY_POOL_SIZE_INCREMENT_IN_BYTES 65536

	class SSSMemoryPool
	{
	private:
		struct SSSHeap
		{
			void*		pData;
			SSSHeap*	pNext;
		};

		struct SSSChunk
		{
			SSSChunk*	pNext;
		};

	public:
		SSSMemoryPool();

		SSSMemoryPool(const SSSMemoryPool& rhs);

		~SSSMemoryPool();

		const SSSMemoryPool& operator=(const SSSMemoryPool& rhs);

		bool Create(U32 elementSize, U32 capacityInBytes);

		void Destroy();

		void Clear();

		void* Allocate();

		void Free(void* pItem);

		U32 Size() const;

		U32 SizeBytes() const;

		U32 Capacity() const;

		U32 CapacityBytes() const;

		///
		///Returns the current memory usage in bytes.
		U32 MemoryUsage() const;

		U32 NumHeaps() const;

		void Output();

		bool Reserve(U32 capacityBytes);

		U32 CollectGarbage();

	private:
		void InternalInitialise(SSSHeap* pNewHeap, U32 numChunks);

		bool InternalGrow();

		///Reserve an additional contiguous block of memory (heap) and append it to the existing heap
		///for this pool.
		bool InternalReserveBytes(U32 capacityBytes);

		SSSHeap*	m_pHeapListHead;
		SSSChunk*	m_pFreeListHead;
		
		U32 m_elementSize;
		U32 m_nElements;
		U32 m_nHeaps;
		U32 m_capacity;
	};

	inline SSSMemoryPool::SSSMemoryPool()
	{
		m_pHeapListHead = 0;
		m_pFreeListHead = 0;

		m_elementSize = 0;
		m_nHeaps = 0;
		m_nElements = 0;
		m_capacity = 0;
	}

	inline SSSMemoryPool::~SSSMemoryPool()
	{
		//Free our heaps.
		SSSHeap* pHeap = m_pHeapListHead;
		while (pHeap)
		{
			//Delete heap data.
			free(pHeap->pData);

			SSSHeap* pNext = pHeap->pNext;
			//Remove heap struct.
			delete pHeap;

			//Go to the next heap;
			pHeap = pNext;
		}
	}

	inline bool SSSMemoryPool::Create(U32 elementSize, U32 capacityInBytes)
	{
		assert(elementSize <= UT_SSS_MEMORY_POOL_SIZE_INCREMENT_IN_BYTES);
		assert(elementSize >= sizeof(SSSChunk));
		if (elementSize < sizeof(SSSChunk))
		{
			assert(!"Element size too small - minimum of 4 bytes.");
			return false;
		}
		
		m_elementSize = elementSize;
		return Reserve(capacityInBytes);
	}

	inline void SSSMemoryPool::Destroy()
	{
		Clear();
		
		m_elementSize = 0;
	}

	inline void SSSMemoryPool::Clear()
	{
		//Free our heaps.
		SSSHeap* pHeap = m_pHeapListHead;
		while (pHeap)
		{
			//Delete heap data.
			free(pHeap->pData);

			SSSHeap* pNext = pHeap->pNext;
			//Remove heap struct.
			delete pHeap;

			//Go to the next heap;
			pHeap = pNext;
		}

		m_pHeapListHead = 0;
		m_pFreeListHead = 0;

		m_nElements = 0;
		m_nHeaps = 0;
		m_capacity = 0;
	}

	inline void* SSSMemoryPool::Allocate()
	{
		if (!m_pFreeListHead)
		{
			if (!InternalGrow())
			{
				//Out of memory.
				return 0;
			}
		}
		void* pAlloc = m_pFreeListHead;
		m_pFreeListHead = m_pFreeListHead->pNext;
		m_nElements++;
		return pAlloc;
	}

	inline void SSSMemoryPool::Free(void* pItem)
	{
		if (pItem)
		{
			SSSChunk* pChunk = (SSSChunk*)pItem;
			pChunk->pNext = m_pFreeListHead;
			m_pFreeListHead = pChunk;
			
			m_nElements--;
		}
	}

	inline U32 SSSMemoryPool::Size() const
	{
		return m_nElements;
	}

	inline U32 SSSMemoryPool::SizeBytes() const
	{
		return m_nElements*m_elementSize;
	}

	inline U32 SSSMemoryPool::Capacity() const
	{
		return m_capacity;
	}

	inline U32 SSSMemoryPool::CapacityBytes() const
	{
		return m_capacity*m_elementSize;
	}

	inline void SSSMemoryPool::Output()
	{
		SSSChunk* pChunk = (SSSChunk*)m_pFreeListHead;
		printf("Dumping free chunks...\n");
		if (!pChunk)
		{
			printf("There are no free chunks.\n");
		}
		else
		{
			U32 chunkNumber = 0;
			while(pChunk)
			{
				#pragma warning(push)
				{
					#pragma warning(disable: 4311)
#ifndef __GNUC__
					printf("%i : Free chunk [0x%x]. Points to [0x%x]\n", chunkNumber, (int)pChunk, (int)pChunk->pNext);
#endif
				}
				#pragma warning(pop)
				pChunk = pChunk->pNext;
				chunkNumber++;
			}

		}
		printf("Finished dumping free chunks.\n");
	}

	inline U32 SSSMemoryPool::NumHeaps() const
	{
		return m_nHeaps;
	}

	inline bool SSSMemoryPool::Reserve(U32 capacityInBytes)
	{
		while (CapacityBytes() < capacityInBytes)
		{
			if (!InternalGrow())
				return false;
		}
		return true;
	}

	inline void SSSMemoryPool::InternalInitialise(SSSHeap* pHeap, U32 numChunks)
	{
		char* pAddr = (char*)pHeap->pData;

		SSSChunk* pChunk;
		if (numChunks > 1)
		{
			for (U32 iChunk = 0; iChunk < numChunks-1; iChunk++)
			{
				U32 offsetInBytes = iChunk*m_elementSize;
				pChunk = (SSSChunk*)(pAddr + offsetInBytes);
				
				//Set next pointer by incrementing current addr by chunk size.
				U32 nextOffsetInBytes = (iChunk+1)*m_elementSize;
				char* pNextChunkAddr = pAddr + nextOffsetInBytes;
				pChunk->pNext = (SSSChunk*)pNextChunkAddr;
			}
		}
		U32 offsetInBytes = (numChunks-1)*m_elementSize;
		pChunk = (SSSChunk*)(pAddr + offsetInBytes);
		//Null the next pointer of the last list element.
		pChunk->pNext = 0;
	}

	inline bool SSSMemoryPool::InternalGrow()
	{
		if (!InternalReserveBytes(m_capacity*m_elementSize + UT_SSS_MEMORY_POOL_SIZE_INCREMENT_IN_BYTES))
		{
			return false;
		}
		return true;
	}

	inline bool SSSMemoryPool::InternalReserveBytes(U32 newCapacityInBytes)
	{
		U32 currentCapacityInBytes = m_capacity*m_elementSize;
		if (newCapacityInBytes > currentCapacityInBytes)
		{
			SSSHeap* pNewHeap = new SSSHeap();
			if (!pNewHeap)
			{
				assert(!"Failed to allocate memory for heap structure.");
				return false;
			}
			//Set this as our new head heap.
			pNewHeap->pNext = m_pHeapListHead;
			m_pHeapListHead = pNewHeap;

			//Allocate the storage space for this heap.

			//TU_SSS_MEMORY_POOL_SIZE_INCREMENT is the ideal size increment.
			//This must be padded so that it can contain multiples of the cluster size.
			U32 growthSizeInBytes = newCapacityInBytes;
			U32 remainder = growthSizeInBytes % m_elementSize;
			growthSizeInBytes -= remainder;

			growthSizeInBytes -= currentCapacityInBytes;

			U32 newHeapNElements = growthSizeInBytes / m_elementSize;

			void* pNewHeapData = malloc(growthSizeInBytes);
			if (!pNewHeapData)
			{
				assert(!"Failed to allocate memory for heap data.");
				throw std::bad_alloc();
				return false;
			}
			pNewHeap->pData = pNewHeapData;
			m_nHeaps++;

			//Initialise the chunks in our new heap.
			InternalInitialise(pNewHeap, newHeapNElements);

			//Attach our additional new chunks to our existing heap.
			SSSChunk* pNewFreeChunkHead = (SSSChunk*)pNewHeap->pData;

			if (m_pFreeListHead)
			{
				pNewFreeChunkHead->pNext = m_pFreeListHead;
				m_pFreeListHead = pNewFreeChunkHead;
			}
			else
			{
				m_pFreeListHead = pNewFreeChunkHead;
			}

			m_capacity += newHeapNElements;

			return true;
		}

		return true;
	}

	inline U32 SSSMemoryPool::CollectGarbage()
	{
		return 0;
	}
};

#endif
