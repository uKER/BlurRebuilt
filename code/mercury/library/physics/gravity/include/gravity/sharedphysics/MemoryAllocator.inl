#ifndef MEMORYALLOCATOR_INL
#define MEMORYALLOCATOR_INL

#include "gravity/sharedphysics/MemoryAllocator.h"

namespace SharedPhysics
{

template<class Elem, int blockSize, bool bConstructDestruct>
NOALIAS inline Elem* MemoryAllocator<Elem, blockSize, bConstructDestruct>::Create(U32 index)
{
	C_Block<Elem, blockSize, bConstructDestruct>* block = &m_FirstBlock;
	while(index >= blockSize)
	{
		if(!block->m_pNextBlock)
		{
			block->Grow();
		}
		block = block->m_pNextBlock;	
		index -= blockSize;
	}
	Elem* pElem = (Elem*)&block->m_Elems[index*sizeof(Elem)];
	if(bConstructDestruct)
	{
		pElem->~Elem();
		new (pElem) Elem;
	}
	return pElem;
	//return &m_FirstBlock.m_Elems[index*sizeof(Elem)];
}

template <class Elem, int blockSize, bool bConstructDestruct>
NOALIAS inline Elem* MemoryAllocator<Elem, blockSize, bConstructDestruct>::Get(U32 index)
{
	C_Block<Elem, blockSize, bConstructDestruct>* block = &m_FirstBlock;
	while(index >= blockSize)
	{
		BIZ_ASSERT(block->m_pNextBlock);
		block = block->m_pNextBlock;	
		index -= blockSize;
	}
	return (Elem*)&block->m_Elems[index*sizeof(Elem)];
	//return &m_FirstBlock.m_Elems[index*sizeof(Elem)];
}

template <class Elem, int blockSize, bool bConstructDestruct>
NOALIAS inline void MemoryAllocator<Elem, blockSize, bConstructDestruct>::Reset()
{
	if(m_FirstBlock.m_pNextBlock)
		delete m_FirstBlock.m_pNextBlock;
	m_FirstBlock.m_pNextBlock = NULL;
}

};

#endif

