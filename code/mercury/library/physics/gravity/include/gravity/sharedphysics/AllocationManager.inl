#ifndef ALLOCATIONMANAGER_INL
#define ALLOCATIONMANAGER_INL


namespace SharedPhysics
{

#ifdef BUFFERED_ALLOCATIONS
template <class T_Data, U32 blockSize, bool bConstructDestruct, bool bOneTimeInit>
NOALIAS FORCE_INLINE T_Data* C_AllocationManager<T_Data, blockSize, bConstructDestruct, bOneTimeInit>::GetNextFreeItem()
{
	if(m_pFreeBlockItems == NULL)
	{
		Grow();
	}
	T_Data * pItem = m_pFreeBlockItems;
	m_pFreeBlockItems = m_pFreeBlockItems->m_pNext;
	pItem->m_pNext = NULL;
#ifdef CHECK_ALLCATIONS
	BIZ_ASSERT(pItem->bAllocated == false);
#endif
	if(bConstructDestruct)
	{
		//__dcbt( 0x0, pItem );
		//pItem->~T_Data(); //Destruct any remaining data, then construct it again!
		new (pItem) T_Data;
	}
#ifdef CHECK_ALLCATIONS
	pItem->bAllocated = true;
#endif
	return pItem;
}

template <class T_Data, U32 blockSize, bool bConstructDestruct, bool bOneTimeInit>
NOALIAS FORCE_INLINE void C_AllocationManager<T_Data, blockSize, bConstructDestruct, bOneTimeInit>::PushToFree(T_Data* pItem)
{
	BIZ_ASSERT(pItem != NULL);
#ifdef CHECK_ALLCATIONS
	BIZ_ASSERT(pItem->bAllocated == true);
	pItem->bAllocated = false;
#endif
	if(bConstructDestruct)
	{
		pItem->~T_Data();
	}
	pItem->m_pNext = m_pFreeBlockItems;
	m_pFreeBlockItems = pItem;
}

template <class T_Data, U32 blockSize, bool bConstructDestruct, bool bOneTimeInit>
NOALIAS FORCE_INLINE void C_AllocationManager<T_Data, blockSize, bConstructDestruct, bOneTimeInit>::PushListToFree(T_Data* pList)
{
	BIZ_ASSERT(pList != NULL);

	T_Data* pLast = pList;
	while(pLast->m_pNext)
	{
		T_Data* pNextItem = pLast->m_pNext;
		if(bConstructDestruct && pLast->m_Allocated)
		{
			pLast->~T_Data();
		}
		pLast = pLast->m_pNext;
	}
	pLast->m_pNext = m_pFreeBlockItems;
	m_pFreeBlockItems = pList;
}

template <class T_Data, U32 blockSize, bool bConstructDestruct, bool bOneTimeInit>
NOALIAS FORCE_INLINE void C_AllocationManager<T_Data, blockSize, bConstructDestruct, bOneTimeInit>::PushBlockToFree(C_AllocationBlock<T_Data, blockSize, bConstructDestruct>* pBlock)
{
	for(U32 a = 1; a < blockSize; ++a)
	{
#ifdef CHECK_ALLCATIONS
		pBlock->m_pDataArray[a-1].bAllocated = false;
#endif
		if(bConstructDestruct && pBlock->m_pDataArray[a-1].m_Allocated)
		{
			pBlock->m_pDataArray[a-1].~T_Data();
		}
		pBlock->m_pDataArray[a-1].m_pNext = &pBlock->m_pDataArray[a];
	}
	if(bConstructDestruct && pBlock->m_pDataArray[blockSize-1].m_Allocated)
	{
		pBlock->m_pDataArray[blockSize-1].~T_Data();
	}

	pBlock->m_pDataArray[blockSize-1].m_pNext = m_pFreeBlockItems;
#ifdef CHECK_ALLCATIONS
	pBlock->m_pDataArray[blockSize - 1].bAllocated = false;
#endif
	m_pFreeBlockItems = &pBlock->m_pDataArray[0];
}

#endif

};

#endif

