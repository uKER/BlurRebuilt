#ifndef UT_MULTI_MEMORY_POOL_H
#define UT_MULTI_MEMORY_POOL_H


namespace Ut
{
	#if WORD_SIZE == 64
	#define UT_SSS_MULTI_MEMORY_POOL_MINIMUM_ELEMENT_SIZE 8
	#else
	#define UT_SSS_MULTI_MEMORY_POOL_MINIMUM_ELEMENT_SIZE 4
	#endif

	#define UT_SSS_MULTI_MEMORY_POOL_MAXIMUM_ELEMENT_SIZE 32768

	class SSSMultiMemoryPool
	{
	public:
		SSSMultiMemoryPool();
		SSSMultiMemoryPool(const SSSMultiMemoryPool& rhs);
		~SSSMultiMemoryPool();

		const SSSMultiMemoryPool& operator=(const SSSMultiMemoryPool& rhs);

		void Create();

		void Destroy();

		void Clear();

		bool ReserveBytes(U32 elementSize, U32 capacityInBytes);

		void* Allocate(U32 elementSize);

		void Free(U32 elementSize, void* pItem);

		///
		///Returns the number of pools in use.
		U32 NumPoolsInUse() const;

		///
		///Returns the number of pools.
		U32 NumPools() const;

		SSSMemoryPool* PoolPtr(U32 index);

		const SSSMemoryPool* PoolPtr(U32 index) const;

		U32 MaximumElementSize() const;

		///
		///Returns the current memory usage in bytes.
		U32 MemoryUsage() const;

		bool SetMaximumElementSize(U32 maximumElementSize);

		void SetOutputNotificationCallback(void (*pCallbackFunc)(const std::string& text));

		void Output();

		U32 CollectGarbage();

	private:
		bool Grow(U32 elementSize, U32 reserveCapacityInBytes);

		SSSMemoryPool** m_ppPoolList;
		U32 m_nPools;
		U32 m_referenceCount;

		void (*m_pOutputNotificationCallback)(const std::string& text);
	};

	inline SSSMultiMemoryPool::SSSMultiMemoryPool()
	{
		m_referenceCount = 0;
	}

	inline SSSMultiMemoryPool::~SSSMultiMemoryPool()
	{
	}


	inline void SSSMultiMemoryPool::Create()
	{
		if (m_referenceCount == 0)
		{
			//Initialise.
			m_ppPoolList = 0;
			m_nPools = 0;
			m_referenceCount = 1;
		}
		else
		{
			m_referenceCount++;
		}
	}

	inline void SSSMultiMemoryPool::Destroy()
	{
		if (m_referenceCount < 2)
		{
			Clear();
			m_referenceCount = 0;
		}
		else
		{
			m_referenceCount--;
		}
	}

	inline void SSSMultiMemoryPool::Clear()
	{
		for (U32 iPool = 0; iPool < m_nPools; iPool++)
		{
			if (m_ppPoolList[iPool])
			{
				m_ppPoolList[iPool]->Destroy();
				delete m_ppPoolList[iPool];
			}
		}
		delete[] m_ppPoolList;
		m_ppPoolList = 0;
		m_nPools = 0;
	}

	inline U32 SSSMultiMemoryPool::CollectGarbage()
	{
		U32 bytesFreed = 0;
		for (U32 iPool = 0; iPool < m_nPools; iPool++)
		{
			if (m_ppPoolList[iPool])
			{
				bytesFreed += m_ppPoolList[iPool]->CollectGarbage();
				//Delete this pool if it's no longer needed.
				if (m_ppPoolList[iPool]->Size() == 0)
				{
					bytesFreed += m_ppPoolList[iPool]->CapacityBytes();
					delete m_ppPoolList[iPool];
					m_ppPoolList[iPool] = 0;
				}
			}
		}

		return bytesFreed;
	}

	inline bool SSSMultiMemoryPool::ReserveBytes(U32 elementSize, U32 capacityInBytes)
	{
		if (capacityInBytes == 0)
			return true;

		if (elementSize < UT_SSS_MULTI_MEMORY_POOL_MINIMUM_ELEMENT_SIZE)
		{
			return true;
		}
		if (elementSize > UT_SSS_MULTI_MEMORY_POOL_MAXIMUM_ELEMENT_SIZE)
		{
			return true;
		}

		if (elementSize >= m_nPools)
		{
			if (!Grow(elementSize, capacityInBytes))
			{
				//Out of memory.
				return false;
			}
		}
		else
		{
			//Create a new pool if needed.
			if (!m_ppPoolList[elementSize])
			{
				m_ppPoolList[elementSize] = new SSSMemoryPool();
				m_ppPoolList[elementSize]->Create(elementSize, capacityInBytes);
			}
			else
			{
				m_ppPoolList[elementSize]->Reserve(capacityInBytes);
			}
		}

		return true;
	}

	inline void* SSSMultiMemoryPool::Allocate(U32 elementSize)
	{
		if (elementSize < UT_SSS_MULTI_MEMORY_POOL_MINIMUM_ELEMENT_SIZE
			||
			elementSize > UT_SSS_MULTI_MEMORY_POOL_MAXIMUM_ELEMENT_SIZE
			)
		{
			void* p = malloc(elementSize);
			if (!p)
				throw std::bad_alloc();
			
			return p;
		}

		if (elementSize >= m_nPools)
		{
			if (!Grow(elementSize, UT_SSS_MEMORY_POOL_SIZE_INCREMENT_IN_BYTES))
			{
				//Out of memory.
				return 0;
			}
		}

		SSSMemoryPool* pPool = m_ppPoolList[elementSize];
		//Create a new memory pool if needed.
		if (!pPool)
		{
			pPool = new SSSMemoryPool();
			if (!pPool)
			{
				assert(pPool);
				return 0;
			}
			pPool->Create(elementSize, UT_SSS_MEMORY_POOL_SIZE_INCREMENT_IN_BYTES);
			m_ppPoolList[elementSize] = pPool;
		}

		return pPool->Allocate();
	}

	inline void SSSMultiMemoryPool::Free(U32 elementSize, void* pItem)
	{
		if (elementSize != 0)
		{
			if (elementSize > UT_SSS_MULTI_MEMORY_POOL_MAXIMUM_ELEMENT_SIZE)
			{
				free(pItem);
				return;
			}
			if (elementSize < UT_SSS_MULTI_MEMORY_POOL_MINIMUM_ELEMENT_SIZE)
			{
				free(pItem);
				return;
			}


			assert(m_ppPoolList[elementSize]);
			m_ppPoolList[elementSize]->Free(pItem);
/*
			//Delete this pool if it's no longer needed.
			if (m_ppPoolList[elementSize]->Size() == 0)
			{
				delete m_ppPoolList[elementSize];
				m_ppPoolList[elementSize] = 0;
			}
*/
		}
	}

	inline U32 SSSMultiMemoryPool::NumPoolsInUse() const
	{
		U32 useCount = 0;
		for (U32 iPool = 0; iPool < m_nPools; iPool++)
		{
			if (m_ppPoolList[iPool])
			{
				useCount++;
			}
		}

		return useCount;
	}

	inline U32 SSSMultiMemoryPool::NumPools() const
	{
		return m_nPools;
	}

	inline SSSMemoryPool* SSSMultiMemoryPool::PoolPtr(U32 index)
	{
		return m_ppPoolList[index];
	}

	inline const SSSMemoryPool* SSSMultiMemoryPool::PoolPtr(U32 index) const
	{
		return m_ppPoolList[index];
	}

	inline U32 SSSMultiMemoryPool::MaximumElementSize() const
	{
		return m_nPools;
	}

	inline void SSSMultiMemoryPool::SetOutputNotificationCallback(void (*pCallbackFunc)(const std::string& text))
	{
		m_pOutputNotificationCallback = pCallbackFunc;
	}

	inline void SSSMultiMemoryPool::Output()
	{
		printf("Dumping pools...\n");
		if (m_nPools == 0)
		{
			printf("There are no pools.\n");
		}
		else
		{
			for (U32 iPool = 0; iPool < m_nPools; iPool++)
			{
				if (m_ppPoolList[iPool])
				{
					printf("Pool proporties: element size [%i] capacity [%i] size [%i] num heaps [%i]\n", 
						iPool, m_ppPoolList[iPool]->Capacity(), m_ppPoolList[iPool]->Size(), m_ppPoolList[iPool]->NumHeaps());
				}
			}
		}
		printf("Finished dumping pools.\n");
	}

	inline bool SSSMultiMemoryPool::Grow(U32 elementSize, U32 reserveCapacityInBytes)
	{
		if (elementSize < UT_SSS_MULTI_MEMORY_POOL_MINIMUM_ELEMENT_SIZE)
		{
			assert(elementSize >= UT_SSS_MULTI_MEMORY_POOL_MINIMUM_ELEMENT_SIZE);
			return false;
		}

		if (m_nPools > elementSize)
		{
			//No need to grow.
			return true;
		}

		SSSMemoryPool** ppTempPoolCopy = m_ppPoolList;

		//Create new pool array.
		m_ppPoolList = new SSSMemoryPool*[elementSize+1];

		//Copy existing pool pointers.
		for (U32 iPool = 0; iPool < m_nPools; iPool++)
		{
			m_ppPoolList[iPool] = ppTempPoolCopy[iPool];
		}
		delete[] ppTempPoolCopy;

		//Null space between existing pool pointers and our new pool.
		for (U32 iPool = m_nPools; iPool < elementSize; iPool++)
		{
			m_ppPoolList[iPool] = 0;
		}

		//Create our new pool.
		SSSMemoryPool* pNewPool = new SSSMemoryPool();
		pNewPool->Create(elementSize, reserveCapacityInBytes);
		m_ppPoolList[elementSize] = pNewPool;

		m_nPools = elementSize+1;

		return true;
	}
};

#endif
