#ifndef _NEW_MEMORY_ALLOCATOR_H
#define _NEW_MEMORY_ALLOCATOR_H

#include "gravity/sharedphysics/AllocationState.h"
#include <core/threads.h>
#include <system/DebugLog.h>

namespace SharedPhysics
{

template<class Elem, int blockSize, bool bConstructDestruct>
class ALIGN16 C_Block
{
public:
	ALIGN16 char m_Elems[blockSize*sizeof(Elem)];
	C_Block<Elem, blockSize, bConstructDestruct>* m_pNextBlock;
	C_Block() : m_pNextBlock(NULL)
	{
		if(bConstructDestruct)
		{
			Elem* pElem((Elem*)&m_Elems[0]);
			for(U32 a = 0; a < blockSize; ++a)
			{
				new ((pElem+a)) Elem;
			}
		}
	}

	~C_Block()
	{
		if(bConstructDestruct)
		{
			Elem* pElem((Elem*)&m_Elems[0]);
			for(U32 a = 0; a < blockSize; ++a)
			{
				pElem[a].~Elem();
			}
		}
		if(m_pNextBlock)
			delete m_pNextBlock;
		m_pNextBlock = NULL;
	}

	inline void Grow()
	{
		m_pNextBlock = new C_Block<Elem, blockSize, bConstructDestruct>;
	}

};

template <class Elem, int blockSize, bool bConstructDestruct>
class C_MemoryIterator;

template <class Elem, int blockSize=32, bool bConstructDestruct = false>
class MemoryAllocator
{
public:
	C_Block<Elem, blockSize, bConstructDestruct> m_FirstBlock;

	NOALIAS inline Elem* Create(U32 index);
	NOALIAS inline Elem* Get(U32 index);
	void Reset();
};

/*
 * Useful optimisation on MemoryAllocator if you are going to be allocating memory in a contiguous
 * range - which is pretty much all the time!
 */

template <class Elem, int blockSize = 32, bool bConstructDestruct = false>
class C_MemoryIterator
{
	MemoryAllocator<Elem, blockSize, bConstructDestruct>* m_pAllocator;
	C_Block<Elem, blockSize, bConstructDestruct>* m_pCurrentBlock;
	U32 m_Index;
	U32 m_Size;
public:
	C_MemoryIterator(MemoryAllocator<Elem, blockSize, bConstructDestruct> * pAllocator) : m_pAllocator(pAllocator), 
		m_pCurrentBlock(&pAllocator->m_FirstBlock), m_Index(0), m_Size(0)
	{
	}

	C_MemoryIterator() : m_pAllocator(NULL), 
		m_pCurrentBlock(NULL), m_Index(0), m_Size(0)
	{
	}
	
	void SetAllocator(MemoryAllocator<Elem, blockSize, bConstructDestruct> * pAllocator)
	{
		m_pAllocator = pAllocator;
		Reset();
	}

	NOALIAS inline void Reset()
	{
		m_Index = 0;
		m_Size = 0;
		m_pCurrentBlock = &m_pAllocator->m_FirstBlock;
	}

	NOALIAS inline Elem* Get()
	{
		BIZ_ASSERT(m_pCurrentBlock);
		if(m_Index < blockSize)
		{
			return (Elem*)&m_pCurrentBlock->m_Elems[sizeof(Elem) * m_Index++];
		}
		m_pCurrentBlock = m_pCurrentBlock->m_pNextBlock;
		m_Index = 1;
		return (Elem*)&m_pCurrentBlock->m_Elems[0];
	}
	NOALIAS inline Elem* Create()
	{
		BIZ_ASSERT(m_pCurrentBlock);
		++m_Size;
		if(m_Index < blockSize)
		{
			Elem* pElem = (Elem*)&m_pCurrentBlock->m_Elems[sizeof(Elem) * m_Index++];
			if(bConstructDestruct)
			{
				pElem->~Elem();
				new (pElem) Elem;
			}
			return pElem;
		}
		if(m_pCurrentBlock->m_pNextBlock == NULL)
			m_pCurrentBlock->m_pNextBlock = new C_Block<Elem, blockSize, bConstructDestruct>;
		m_pCurrentBlock = m_pCurrentBlock->m_pNextBlock;
		m_Index = 1;
		Elem* pElem = (Elem*)&m_pCurrentBlock->m_Elems[0];
		if(bConstructDestruct)
		{
			pElem->~Elem();
			new (pElem) Elem;
		}
		return pElem;
	}

	NOALIAS inline U32 NumElements()
	{
		return m_Size;
	}
};
template <class Elem, int blockSize = 32, bool bConstructDestruct = false>

class C_SynchronisedMemoryIterator
{
public:
	MT::Lock m_Mutex;
	C_MemoryIterator<Elem, blockSize, bConstructDestruct> m_Iterator;	
	
	C_SynchronisedMemoryIterator(MemoryAllocator<Elem, blockSize, bConstructDestruct> * pAllocator) : m_Iterator(pAllocator), m_Mutex()
	{
	}
	
	void SetAllocator(MemoryAllocator<Elem, blockSize, bConstructDestruct> * pAllocator)
	{
		m_Iterator.SetAllocator(pAllocator);
	}
	
	NOALIAS void Reset()
	{
		m_Iterator.Reset();
	}
	
	NOALIAS inline Elem* Get()
	{
		m_Mutex.Enter();
		Elem* result = m_Iterator.Get();
		m_Mutex.Exit();
		return result;
	}
	
	NOALIAS inline Elem* Create()
	{
		m_Mutex.Enter();
		Elem* result = m_Iterator.Create();
		return result;
	}
};

}

#include "gravity/sharedphysics/MemoryAllocator.inl"

#endif
