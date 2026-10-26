#ifndef NEWLIST_H
#define NEWLIST_H
#include "gravity/sharedphysics/Select.h"
#include "gravity/sharedphysics/AllocationManager.h"

namespace SharedPhysics
{

/*
	C_SingleList is designed to use with memory allocator. Memory allocator do the job of allocating and releasing memory, and list is
	used to iterator through the whole chunk of memory.
*/
template <class Elem>
class C_SingleList
{
public:
	C_SingleList()
	{
		m_pHead=&m_Dummy;
		m_pTail=&m_Dummy;
	}


	Elem* GetFirstElem()
	{
		return m_pHead->m_pNext;
	}

	void InsertToEnd(Elem* elem)
	{
		m_pTail->m_pNext=elem;
		elem->m_pPre=m_pTail;
		elem->m_pNext=NULL;
		m_pTail=elem;
	}

	void Remove(Elem* elem)
	{
		elem->m_pPrev->m_pNext = elem->m_pNext;
		Elem* next = Fast_Select<Elem>(elem->m_pNext == NULL, elem, elem->m_pNext);
		next->m_pPrev = elem->m_pPrev;
		elem->m_pPrev = elem->m_pNext = NULL;
	}

	Elem* m_pHead;
	Elem* m_pTail;
	Elem m_Dummy;

};

template <class Elem>
class C_DoubleListElem : public C_BlockItem<Elem>
{
public:
	C_DoubleListElem() : m_pPrev(NULL)
	{
	}

	Elem* m_pPrev;
};

template <class Elem>
class C_DoubleList
{
public:
	C_DoubleList(): m_NumElems(0)
	{
		m_pHead = &m_Dummy;
		m_pTail = &m_Dummy;
	}

	//Head always point to the m_Dummy
	NOALIAS inline Elem* GetHead()
	{
		return m_pHead;
	}

	NOALIAS inline Elem* GetTail()
	{
		return m_pTail;
	}

	NOALIAS inline Elem* GetFirstElem()
	{
		return m_pHead->m_pNext;
	}

	NOALIAS inline U32 Size()
	{
		return m_NumElems;
	}

	//Swap 
	NOALIAS inline void Swap(Elem* curElem, Elem* elem)
	{
		/*Elem* curPrev = curElem->m_pPrev;
		curPrev->m_pNext = elem;
		Elem* next = SharedPhysics::Fast_Select<Elem>(elem->m_pNext == NULL, &m_Garbage, elem->m_pNext);
		next->m_pPrev=curElem;

		Elem* curPrev(curElem->m_pPrev);
		curElem->m_pPrev=SharedPhysics::Fast_Select<Elem>(elem->m_pPrev == curElem, elem, elem->m_pPrev);
		elem->m_pPrev=SharedPhysics::Fast_Select<Elem>(pPrev == elem, curElem, pPrev);


		Elem* pNext(curElem->m_pNext);
		curElem->m_pNext=SharedPhysics::Fast_Select<Elem>(elem->m_pNext == curElem, elem, elem->m_pNext);
		elem->m_pNext = SharedPhysics::Fast_Select<Elem>(pNext == elem, curElem, pNext);

		
		Elem* newTail = Fast_Select<Elem>(m_pTail == curElem, elem, m_pTail);
		m_pTail = Fast_Select<Elem>(m_pTail == elem, curElem, newTail);*/

		//Swap 2 elements in a linked list!

		if(curElem == m_pTail)
		{
			m_pTail = elem;
		}
		else if (elem == m_pTail)
		{
			m_pTail = curElem;
		}

		if(curElem->m_pNext == elem)
		{
			elem->m_pPrev = curElem->m_pPrev;
			elem->m_pPrev->m_pNext=  elem;
			curElem->m_pPrev = elem;
			curElem->m_pNext = elem->m_pNext;
			if(curElem->m_pNext)
				curElem->m_pNext->m_pPrev = curElem;
			elem->m_pNext = curElem;
		}
		else if (curElem->m_pPrev == elem)
		{
			curElem->m_pPrev = elem->m_pPrev;
			curElem->m_pPrev->m_pNext=  curElem;
			elem->m_pPrev = curElem;
			elem->m_pNext = curElem->m_pNext;
			if(elem->m_pNext)
				elem->m_pNext->m_pPrev = elem;
			curElem->m_pNext = elem;
		}
		else
		{
			Elem* curPrev = curElem->m_pPrev;
			Elem* curNext = curElem->m_pNext;
			curElem->m_pNext = elem->m_pNext;
			curElem->m_pPrev = elem->m_pPrev;
			elem->m_pNext = curNext;
			elem->m_pPrev = curPrev;

			curElem->m_pPrev->m_pNext = curElem;
			elem->m_pPrev->m_pNext = elem;

			if(curElem->m_pNext)
				curElem->m_pNext->m_pPrev = curElem;
			if(elem->m_pNext)
				elem->m_pNext->m_pPrev = elem;
		}


	}


	NOALIAS inline void InsertAfter(Elem* curElem, Elem* elem)
	{
		Elem* next(Fast_Select<Elem>(curElem->m_pNext == NULL, &m_Garbage, curElem->m_pNext));
		next->m_pPrev = elem;
		elem->m_pNext= curElem->m_pNext;
		curElem->m_pNext=elem;
		elem->m_pPrev = curElem;
		if(curElem == m_pTail)
		{
			m_pTail = elem;
		}
		m_NumElems++;
	}

	NOALIAS inline void InsertToEnd(Elem* elem)
	{
		m_pTail->m_pNext = elem;
		elem->m_pPrev = m_pTail;
		elem->m_pNext = NULL;
		m_pTail = elem;
		m_NumElems++;
	}


	NOALIAS inline void Remove(Elem* elem)
	{
		elem->m_pPrev->m_pNext = elem->m_pNext;
		m_pTail = Fast_Select<Elem>(elem == m_pTail, elem->m_pPrev, m_pTail);
		Elem* next = Fast_Select<Elem>(elem->m_pNext == NULL, &m_Garbage, elem->m_pNext);
		next->m_pPrev = elem->m_pPrev;
		elem->m_pPrev = elem->m_pNext = NULL;
		m_NumElems--;
	}

	NOALIAS inline Elem* Remove(const U32 index, const U32 size)
	{
		Elem* pElem = Get(index);
		Elem* pLast = pElem;
		S32 _size = size-1;
		while(_size--)
		{
			pLast = pLast->m_pNext;
		}
		pElem->m_pPrev->m_pNext = pLast->m_pNext;
		m_pTail = Fast_Select<Elem>(pLast == m_pTail, pElem->m_pPrev, m_pTail);
		Elem* next = Fast_Select<Elem>(pLast->m_pNext == NULL, &m_Garbage, pLast->m_pNext);
		next->m_pPrev = pElem->m_pPrev;
		pElem->m_pPrev = NULL;
		pLast->m_pNext = NULL;
		/*pElem->m_pPrev = pElem->m_pNext = NULL;
		pLast->m_pPrev = pLast->m_pNext= NULL;*/
		m_NumElems-= size;
		return pElem;
	}

	NOALIAS inline void Insert(Elem* elem, const U32 index)
	{
		Elem* pNext = Get(index);
		InsertAfter(pNext->m_pPrev, elem);
	}

	NOALIAS inline Elem& operator [] (const U32 index)
	{
		return *Get(index);
	}

	NOALIAS inline Elem* Get(const U32 index)
	{
		BIZ_ASSERT(index < m_NumElems);
		Elem* pRet = m_pHead->m_pNext;
		U32 _index = index;
		while(_index--)
		{
			//assert(pRet);
			pRet = pRet->m_pNext;
		}
		return pRet;
	}


	NOALIAS inline void Reset()
	{
		m_Dummy.m_pNext=NULL;
		m_pHead = &m_Dummy;
		m_pTail = &m_Dummy;
		m_NumElems=0;
	}

	Elem* m_pHead;
	Elem* m_pTail;
	Elem m_Dummy;
	Elem m_Garbage;
	U32 m_NumElems;
};

template <class Elem>
class C_DoubleListAlloc
{
public:
	SharedPhysics::C_AllocationManager<Elem, 32, false>* m_pAllocator;
	C_DoubleList<Elem> m_List;


	C_DoubleListAlloc(SharedPhysics::C_AllocationManager<Elem, 32, false>* pAllocator) : m_pAllocator(pAllocator)
	{
	}

	void SplitLists(C_DoubleListAlloc& listA, C_DoubleListAlloc& listB, const U32 splitIndex)
	{
		Elem* endNodeA = Get(splitIndex);
		Elem* headA = listA.GetHead();
		headA->m_pNext = GetFirstElem();
		Elem* headB = listB.GetHead();
		headB->m_pNext = endNodeA->m_pNext;
		headA->m_pNext->m_pPrev = headA;
		headB->m_pNext->m_pPrev = headB;
		endNodeA->m_pNext = NULL;
		listA.m_List.m_pTail = endNodeA;
		listB.m_List.m_pTail = GetTail();
		listA.m_List.m_NumElems = splitIndex + 1;
		listB.m_List.m_NumElems = (m_List.Size() - splitIndex - 1);
		Elem* head = m_List.GetHead();
		head->m_pNext = NULL;
		m_List.m_NumElems = 0;
		BIZ_ASSERT(listA.m_List.m_pTail->m_pNext == NULL);
		BIZ_ASSERT(listB.m_List.m_pTail->m_pNext == NULL);

	}

	void MergeLists(C_DoubleListAlloc& listA, C_DoubleListAlloc& listB)
	{
		Elem* firstA = listA.GetFirstElem();
		Elem* firstB = listB.GetFirstElem();
		Elem* endA = listA.GetTail();
		Elem* endB = listB.GetTail();
		BIZ_ASSERT(endA->m_pNext == NULL);
		BIZ_ASSERT(endB->m_pNext == NULL);
		endA->m_pNext = firstB;
		firstB->m_pPrev = endA;
		m_List.m_pTail = endB;
		m_List.m_pHead->m_pNext = firstA;
		m_List.m_NumElems = listA.Size() + listB.Size();
	}

	const U32 CountElements()
	{
		U32 count = 0;
		Elem* pElem = GetFirstElem();
		while(pElem)
		{
			count++;
			pElem = pElem->m_pNext;
		}
		return count;
	}


	//Head always point to the m_Dummy
	NOALIAS inline Elem* GetHead()
	{
		return m_List.GetHead();
	}

	NOALIAS inline Elem* GetTail()
	{
		return m_List.GetTail();
	}

	NOALIAS inline Elem* GetFirstElem()
	{
		return m_List.GetFirstElem();
	}

	NOALIAS inline U32 Size()
	{
		return m_List.Size();
	}

	//Swap 
	NOALIAS inline void Swap(Elem* curElem, Elem* elem)
	{
		m_List.Swap(curElem, elem);
	}


	NOALIAS inline void InsertAfter(Elem* curElem, Elem* elem)
	{
		Elem* newElem = m_pAllocator->GetNextFreeItem();
		const Elem& _elem = *elem;
		newElem = new(newElem)Elem(_elem);
		//assert(newElem->m_MaterialIndex < 64);
		m_List.InsertAfter(curElem, newElem);
	}

	NOALIAS inline void AddToEnd(Elem* elem)
	{
		Elem* newElem = m_pAllocator->GetNextFreeItem();
		const Elem& _elem = *elem;
		newElem = new(newElem)Elem(_elem);
		m_List.InsertToEnd(newElem);
	}


	NOALIAS inline void Remove(Elem* elem)
	{
		m_List.Remove(elem);
		m_pAllocator->PushToFree(elem);
	}

	NOALIAS inline void Remove(const U32 index, const U32 size)
	{
		Elem* pElem = m_List.Remove(index, size);
		while(pElem)
		{
			Elem* pNext = pElem;
			m_pAllocator->PushToFree(pElem);
			pElem = pNext;
		}
	}

	NOALIAS inline void Insert(Elem* elem, const U32 index)
	{
		Elem* newElem = m_pAllocator->GetNextFreeItem();
		const Elem& _elem = *elem;
		newElem = new(newElem)Elem(_elem);
		m_List.Insert(newElem, index);
	}

	NOALIAS inline Elem& operator [] (const U32 index)
	{
		return m_List[index];
	}

	NOALIAS inline Elem* Get(const U32 index)
	{
		return m_List.Get(index);
	}


	NOALIAS inline void Reset()
	{
		m_List.Reset();
		m_pAllocator->Reset();
	}
};

template <typename Elem>
class C_DoubleAllocIterator
{
	C_DoubleListAlloc<Elem>& m_List;
	Elem* m_pCurrent;
public:
	C_DoubleAllocIterator(C_DoubleListAlloc<Elem>& list) : m_List(list), 
		m_pCurrent(m_List.GetHead())
	{
	}

	C_DoubleAllocIterator(C_DoubleListAlloc<Elem>& list, const U32 startIndex) : m_List(list), 
		m_pCurrent((startIndex == 0) ? (m_List.GetHead()) : m_List.Get(startIndex-1))
	{
	}

	bool HasNext()
	{
		return m_pCurrent->m_pNext != NULL;
	}

	Elem* Next()
	{
		m_pCurrent = m_pCurrent->m_pNext;
		return m_pCurrent;
	}

	void InsertAfter(Elem* pElem)
	{
		m_List.InsertAfter(m_pCurrent, pElem);
	}

	void InsertBefore(Elem* pElem)
	{
		m_List.InsertAfter(m_pCurrent->m_pPrev, pElem);
		m_pCurrent = pElem->m_pPrev;
	}

	void Remove()
	{
		//Elem* pNext = (m_pCurrent->m_pNext != NULL) ? : m_pCurrent->m_pNext : m_pCurrent->m_pPrev;
		m_pCurrent = m_pCurrent->m_pPrev;
		m_List.Remove(m_pCurrent->m_pNext);
	}



};

};

#endif
