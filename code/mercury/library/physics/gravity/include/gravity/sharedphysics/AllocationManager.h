#ifndef ALLOCATIONMANAGER_H
#define ALLOCATIONMANAGER_H
namespace SharedPhysics
{
#include "gravity/sharedphysics/AllocationState.h"
//Note! to use this correctly, T_Data MUST extend C_BlockItem!

template <class T_Data>
class C_BlockItem
{
public:
	C_BlockItem() : m_pNext(NULL), m_Allocated(true)
	{
	}

	virtual ~C_BlockItem()
	{
		m_Allocated = false;
	}
#if 0 //def _DEBUG
	void* operator new(size_t size)
	{
		void* data = ::malloc(size);
		::memset(data, 0, size);
		return data;
	}

	void* operator new(size_t size, void* memAddy)
	{
		::memset(memAddy, 0, size);
		return memAddy;
	}
#endif
	T_Data* m_pNext;
	bool m_Allocated;
};

template <class T_Data, U32 blockSize, bool bConstructDestruct>
class ALIGN16 C_AllocationBlock
{
//Only an allocation manager can create or destroy an allocationblock. This way, we can non-recursively 
//deallocate the linked list of allocationblocks
public:
	C_AllocationBlock() : m_pDataArray((T_Data*)&m_memory[0]), m_pNextBlock(NULL)
	{
		if(bConstructDestruct)
		{
			for(U32 a = 0; a < blockSize; ++a)
			{
				//__dcbt( 0x0, &m_pDataArray[a] );
				//new (&m_pDataArray[a]) T_Data;
				m_pDataArray[a].m_Allocated = false;
			}
			//Note - using array new constructor eats up the 1st 16 bytes with a header!!!!!
			//new (&m_memory[0]) T_Data[blockSize];
		}
	}
	~C_AllocationBlock()
	{
		if(bConstructDestruct)
		{
			for(U32 a = 0; a < blockSize; ++a)
			{
				if(m_pDataArray[a].m_Allocated)
				{
					m_pDataArray[a].~T_Data();
				}
			}
		}
	}
	ALIGN16 U8 m_memory[sizeof(T_Data) * blockSize];
	T_Data *m_pDataArray;
	C_AllocationBlock<T_Data, blockSize, bConstructDestruct>* m_pNextBlock;
};

template <class T_Data, U32 blockSize = 32, bool bConstructDestruct = false, bool oneTimeInit = false>
class C_AllocationManager
{
public:

#ifdef BUFFERED_ALLOCATIONS
	C_AllocationManager() : m_pFreeBlockItems(NULL)
	{
		m_pRootBlock = new C_AllocationBlock<T_Data, blockSize, bConstructDestruct>;
		m_pCurrentBlock = m_pRootBlock;
		PushBlockToFree(m_pCurrentBlock);
	}
	~C_AllocationManager()
	{
		//Job... delete additional blocks...
		
		C_AllocationBlock<T_Data, blockSize, bConstructDestruct> * pBlock = m_pRootBlock;
		while(pBlock)
		{
			C_AllocationBlock<T_Data, blockSize, bConstructDestruct>* pNext = pBlock->m_pNextBlock;
			delete pBlock;
			pBlock = pNext;
		}
	}
	NOALIAS FORCE_INLINE void Reset()
	{
		m_pCurrentBlock = m_pRootBlock;
		m_pFreeBlockItems = NULL;
		PushBlockToFree(m_pCurrentBlock);
	}
	NOALIAS T_Data* GetNextFreeItem();
	NOALIAS void PushToFree(T_Data* pItem);
	NOALIAS void PushListToFree(T_Data* pList);
private:
	NOALIAS void PushBlockToFree(C_AllocationBlock<T_Data, blockSize, bConstructDestruct>* pBlock);
	NOALIAS FORCE_INLINE void Grow()
	{
		if(m_pCurrentBlock->m_pNextBlock == NULL)
			m_pCurrentBlock->m_pNextBlock = new C_AllocationBlock<T_Data, blockSize, bConstructDestruct>;
		m_pCurrentBlock = m_pCurrentBlock->m_pNextBlock;
		//__dcbt(0x0, m_pCurrentBlock);
		PushBlockToFree(m_pCurrentBlock);
	}
	C_AllocationBlock<T_Data, blockSize, bConstructDestruct> *m_pRootBlock;
	C_AllocationBlock<T_Data, blockSize, bConstructDestruct> * m_pCurrentBlock;
	T_Data* m_pFreeBlockItems;
#else

	~C_AllocationManager()
	{
		Reset();
	}

	NOALIAS FORCE_INLINE void Reset()
	{
		for(S32 a = 0; a < m_SortedArray.m_numElements; ++a)
		{
			if(m_SortedArray[a])
				delete m_SortedArray[a];
			m_SortedArray[a] = NULL;
		}
		m_SortedArray.m_numElements = 0;
	}
	NOALIAS T_Data* GetNextFreeItem()
	{
		T_Data* pData = new T_Data;
		m_SortedArray.Insert(&pData);
		return pData;
	}
	NOALIAS void PushToFree(T_Data* pItem)
	{
		m_SortedArray.Remove(pData);
		delete pItem;
	}
	NOALIAS void PushListToFree(T_Data* pList)
	{
		//Do nothing - not used anymore
	}
private:
	SharedPhysics::SortedArray<T_Data*> m_SortedArray;
#endif
};

template <class T_Data, U32 size, bool bConstructDestruct>
class C_StaticAllocator
{
	static C_AllocationManager<T_Data, size, bConstructDestruct> s_AllocationManager;
public:
	static T_Data* GetType()
	{
		s_AllocationManager.GetNextFreeElem();
	}

	static void Release(T_Data* pData)
	{
		s_AllocationManager.PushToFree(pData);
	}

};

};

#include "gravity/sharedphysics/Allocationmanager.inl"

#endif
