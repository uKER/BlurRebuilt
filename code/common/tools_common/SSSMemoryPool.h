#ifndef TU_MEMORY_POOL_H
#define TU_MEMORY_POOL_H

#include "Status.h"

namespace Tu
{
	#define TU_SSS_MEMORY_POOL_SIZE_INCREMENT_IN_BYTES 65536

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

		Tu::Status Create(U32 elementSize, U32 capacityInBytes);

		void Destroy();

		void Clear();

		void* Allocate();

		void Free(void* pItem);

		U32 Size() const;

		U32 SizeBytes() const;

		U32 Capacity() const;
		
		U32 CollectGarbage();
		
		///
		///Returns the current memory usage in bytes.
		U32 MemoryUsage() const;

		U32 NumHeaps() const;

		///Reserve an additional contiguous block of memory (heap) and append it to the existing heap
		///for this pool.
		Tu::Status ReserveBytes(U32 capacityBytes);

		void Output();

	private:
		void Initialise(SSSHeap* pNewHeap, U32 numChunks);
		bool Grow();

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

	inline Tu::Status SSSMemoryPool::Create(U32 elementSize, U32 capacityInBytes)
	{
		assert(elementSize <= TU_SSS_MEMORY_POOL_SIZE_INCREMENT_IN_BYTES);
		assert(elementSize >= sizeof(SSSChunk));
		if (elementSize < sizeof(SSSChunk))
		{
			return Tu::Status(Tu::Status::FAILURE, 1, "Element size too small - minimum of 4 bytes.");
		}
		
		m_elementSize = elementSize;
		return ReserveBytes(capacityInBytes);
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
			if (!Grow())
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

	inline Tu::Status SSSMemoryPool::ReserveBytes(U32 newCapacityInBytes)
	{
		U32 currentCapacityInBytes = m_capacity*m_elementSize;
		if (newCapacityInBytes > currentCapacityInBytes)
		{
			SSSHeap* pNewHeap = new SSSHeap();
			if (!pNewHeap)
			{
				return Tu::Status(Tu::Status::OUT_OF_MEMORY, 1, "Failed to allocate memory for heap structure.");
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
				return Tu::Status(Tu::Status::OUT_OF_MEMORY, 1, "Failed to allocate memory for heap data.");
			}
			pNewHeap->pData = pNewHeapData;
			m_nHeaps++;

			//Initialise the chunks in our new heap.
			Initialise(pNewHeap, newHeapNElements);

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

			return Tu::Status::SUCCESS;
		}

		return Tu::Status::FAILURE;
	}

	inline U32 SSSMemoryPool::NumHeaps() const
	{
		return m_nHeaps;
	}

	inline U32 SSSMemoryPool::MemoryUsage() const
	{
		return sizeof(SSSMemoryPool) + m_nHeaps * sizeof(SSSHeap);
	}

	inline void SSSMemoryPool::Initialise(SSSHeap* pHeap, U32 numChunks)
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

	inline bool SSSMemoryPool::Grow()
	{
		if (!ReserveBytes(m_capacity*m_elementSize + TU_SSS_MEMORY_POOL_SIZE_INCREMENT_IN_BYTES))
		{
			return false;
		}
		return true;
	}

	inline U32 SSSMemoryPool::CollectGarbage()
	{
/*		if (m_nHeaps == 0)
			return 0;

		U32 chunksPerHeap = TU_SSS_MEMORY_POOL_SIZE_INCREMENT_IN_BYTES / m_elementSize;
		U32 freed = 0;
		{
			SSSHeap* pLastHeap = 0;
			SSSHeap* pHeap = m_pHeapListHead;
			while(pHeap)
			{
				U32 freeChunks = 0;
				SSSChunk* pChunk = m_pFreeListHead;
				while(pChunk)
				{
					S64 offset = (S64)pChunk - (S64)pHeap->pData;
					if (offset > -1)
					{
						if (offset < TU_SSS_MEMORY_POOL_SIZE_INCREMENT_IN_BYTES)
						{
							//Part of this heap is free...
							freeChunks++;
						}
					}

					pChunk = pChunk->pNext;
				}

				//Delete this heap if required.
				if (freeChunks == chunksPerHeap)
				{
					if (pLastHeap)
					{
						pLastHeap->pNext = pHeap->pNext;
					}
					else
					{
						m_pHeapListHead = pHeap->pNext;
					}

					{
						SSSChunk* pLastGoodChunk = 0;
						pChunk = m_pFreeListHead;
						//Patch up free list
						while(pChunk)
						{
							S64 offset = (S64)pChunk - (S64)pHeap->pData;
							if (offset > -1)
							{
								if (offset < TU_SSS_MEMORY_POOL_SIZE_INCREMENT_IN_BYTES)
								{
									//Part of the heap to be deleted.
									if (pLastGoodChunk)
										pLastGoodChunk->pNext = pChunk->pNext;
									else
										m_pFreeListHead = pChunk->pNext;

									pChunk = pChunk->pNext;
									continue;
								}
							}

							pLastGoodChunk = pChunk;
							pChunk = pChunk->pNext;
						}
					}

					freed++;
					SSSHeap* pToDelete = pHeap;
					pHeap = pHeap->pNext;

					//Delete heap data.
					free(pToDelete->pData);

					delete pToDelete;
				}
				else
				{
					pLastHeap = pHeap;
					pHeap = pHeap->pNext;
				}
			}
			
		}

		return freed;
		*/
		return 0;
	}
};

#endif
