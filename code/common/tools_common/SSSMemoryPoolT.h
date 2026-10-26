#ifndef TU_MEMORY_POOL_H
#define TU_MEMORY_POOL_H

#include "Status.h"

namespace Tu
{
	#define TU_SSS_MEMORY_POOL_SIZE_INCREMENT 1

	template< U32 elementSize >
	class SSSMemoryPoolT
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
		SSSMemoryPoolT();
		SSSMemoryPoolT(const SSSMemoryPoolT& rhs);
		~SSSMemoryPoolT();

		const SSSMemoryPoolT& operator=(const SSSMemoryPoolT& rhs);

		Tu::Status Create(U32 numElements);

		void Destroy();

		void Clear();

		void* Allocate();

		void Free(void* pItem);

		U32 Size() const;

		U32 Capacity() const;
		
		///
		///Returns the current memory usage in bytes.
		U32 MemoryUsage() const;

		///Reserve an additional contiguous block of memory (heap) and append it to the existing heap
		///for this pool.
		Tu::Status Reserve(U32 capacity);

		void Output();

	private:
		void Initialise(SSSHeap* pNewHeap, U32 numChunks);
		bool Grow();

		SSSHeap*	m_pHeapListHead;
		SSSChunk*	m_pFreeListHead;
		
		U32 m_nElements;
		U32 m_nHeaps;
		U32 m_capacity;
	};

	template< U32 elementSize >
	inline SSSMemoryPoolT<elementSize>::SSSMemoryPoolT()
	{
		m_pHeapListHead = 0;
		m_pFreeListHead = 0;

		m_nHeaps = 0;
		m_nElements = 0;
		m_capacity = 0;
	}

	template< U32 elementSize >
	inline SSSMemoryPoolT<elementSize>::~SSSMemoryPoolT()
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

	template< U32 elementSize >
	inline Tu::Status SSSMemoryPoolT<elementSize>::Create(U32 numElements)
	{
		assert(elementSize >= sizeof(SSSChunk));
		if (elementSize < sizeof(SSSChunk))
		{
			return Tu::Status(Tu::Status::FAILURE, "Element size too small - minimum of 4 bytes.");
		}
		
		return Reserve(numElements);
	}

	template< U32 elementSize >
	inline void SSSMemoryPoolT<elementSize>::Destroy()
	{
		Clear();
	}

	template< U32 elementSize >
	inline void SSSMemoryPoolT<elementSize>::Clear()
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

	template< U32 elementSize >
	inline void* SSSMemoryPoolT<elementSize>::Allocate()
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

	template< U32 elementSize >
	inline void SSSMemoryPoolT<elementSize>::Free(void* pItem)
	{
		SSSChunk* pChunk = (SSSChunk*)pItem;
		pChunk->pNext = m_pFreeListHead;
		m_pFreeListHead = pChunk;
		
		m_nElements--;
	}

	template< U32 elementSize >
	inline U32 SSSMemoryPoolT<elementSize>::Size() const
	{
		return m_nElements;
	}

	template< U32 elementSize >
	inline U32 SSSMemoryPoolT<elementSize>::Capacity() const
	{
		return m_capacity;
	}

	template< U32 elementSize >
	inline void SSSMemoryPoolT<elementSize>::Output()
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
					printf("%i : Free chunk [0x%x]. Points to [0x%x]\n", chunkNumber, (int)pChunk, (int)pChunk->pNext);
				}
				#pragma warning(pop)
				pChunk = pChunk->pNext;
				chunkNumber++;
			}

		}
		printf("Finished dumping free chunks.\n");
	}

	template< U32 elementSize >
	inline Tu::Status SSSMemoryPoolT<elementSize>::Reserve(U32 capacity)
	{
		if (capacity > m_capacity)
		{
			SSSHeap* pNewHeap = new SSSHeap();
			if (!pNewHeap)
			{
				return Tu::Status(Tu::Status::OUT_OF_MEMORY, "Failed to allocate memory for heap structure.");
			}
			//Set this as our new head heap.
			pNewHeap->pNext = m_pHeapListHead;
			m_pHeapListHead = pNewHeap;

			//Allocate the storage space for this heap.
			U32 newHeapNElements = capacity - m_capacity;
			void* pNewHeapData = malloc(elementSize*newHeapNElements);
			if (!pNewHeapData)
			{
				return Tu::Status(Tu::Status::OUT_OF_MEMORY, "Failed to allocate memory for heap data.");
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

			m_capacity = capacity;

			return Tu::Status::SUCCESS;
		}

		return Tu::Status::FAILURE;
	}

	template< U32 elementSize >
	inline U32 SSSMemoryPoolT<elementSize>::MemoryUsage() const
	{
		return m_nElements * m_elementSize + m_nHeaps * sizeof(SSSHeap) + 16;
	}

	template< U32 elementSize >
	inline void SSSMemoryPoolT<elementSize>::Initialise(SSSHeap* pHeap, U32 numChunks)
	{
		char* pAddr = (char*)pHeap->pData;

		SSSChunk* pChunk;
		if (numChunks > 1)
		{
			for (U32 iChunk = 0; iChunk < numChunks-1; iChunk++)
			{
				U32 offsetInBytes = iChunk*elementSize;
				pChunk = (SSSChunk*)(pAddr + offsetInBytes);
				
				//Set next pointer by incrementing current addr by chunk size.
				U32 nextOffsetInBytes = (iChunk+1)*elementSize;
				char* pNextChunkAddr = pAddr + nextOffsetInBytes;
				pChunk->pNext = (SSSChunk*)pNextChunkAddr;
			}
		}
		U32 offsetInBytes = (numChunks-1)*elementSize;
		pChunk = (SSSChunk*)(pAddr + offsetInBytes);
		//Null the next pointer of the last list element.
		pChunk->pNext = 0;
	}

	template< U32 elementSize >
	inline bool SSSMemoryPoolT<elementSize>::Grow()
	{
		if (!Reserve(m_capacity + TU_SSS_MEMORY_POOL_SIZE_INCREMENT))
		{
			return false;
		}
		return true;
	}
};

#endif