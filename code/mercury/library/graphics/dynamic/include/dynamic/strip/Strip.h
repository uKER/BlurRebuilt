#ifndef RENDER_DYNAMICEFFECT_STRIP_H
#define RENDER_DYNAMICEFFECT_STRIP_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Manager.h"
#include "../Block.h"

namespace DynamicEffect
{
namespace Strip
{

// strip contains a number of blocks which get drawn together as a strip. blocks are contiguous memory for a fixed amount of elements
// idea is this improves cache performance rather than using a linked list which could get fragmented
template<class T_Element, class T_Strip>
class C_Strip : public Ut::ListElementDouble<C_Strip<T_Element, T_Strip> >, public T_Strip, public C_RenderNode
{
public:
	C_Strip() : m_pManager(NULL), m_iOffset(0) {};
	void							Destroy() { return; }
	
	//const static U32 ElementSize = sizeof(T_Element);
	
	typedef T_Element																	ElementType;
	typedef T_Strip																		StripType;
	typedef ElementBlock																ElementBlockType;
	typedef	Ut::UtList<ElementBlockType, Ut::DoubleEndedList, Ut::MaintainedCounter>	BlockList;

	BlockList																			m_Blocks;
	
	// these allow the front and back of the blocks to not be rendered/updated, effectively allowing removal from front or back
	C_Manager*																			m_pManager;
	U32																					m_iOffset;

public:
	void							SetElementSize(U32 iElementSize) { m_iElementSize = iElementSize; }
	U32								GetElementCount() const;
	ElementType*					GetElement(U32 index);
	const ElementType*				GetElement(U32 index) const;
	void							CalculateExtents();

	void							PopFront();
	void							PopBack();
	ElementType*					PushBack();
	void							Free();

private:
	U32								m_iElementSize;
};

template<class T_Element, class T_Strip>
const T_Element* C_Strip<T_Element, T_Strip>::GetElement(U32 index) const
{
	index += m_iOffset;

	// not particularly a fast operation cause we have to iterate list
	for(typename BlockList::ConstIterator pIterator = m_Blocks.Begin(); pIterator != m_Blocks.End(); ++pIterator)
	{
		if(index < U32(pIterator->m_Element.Size()))
			return (const ElementType*)&pIterator->m_Element[index];
		else
			index -= pIterator->m_Element.Size();
	}

	return NULL;
}

template<class T_Element, class T_Strip>
U32 C_Strip<T_Element, T_Strip>::GetElementCount() const
{
	U32 count = 0;
	for(typename BlockList::ConstIterator pIterator = m_Blocks.Begin(); pIterator != m_Blocks.End(); ++pIterator)
	{
		if(pIterator == m_Blocks.Begin())
		{
			assert(U32(pIterator->m_Element.Size()) > m_iOffset);
			count += pIterator->m_Element.Size() - m_iOffset;
		}
		else
			count += pIterator->m_Element.Size();
	}

	return count;
}

template<class T_Element, class T_Strip>
void C_Strip<T_Element, T_Strip>::Free()
{
	// remove all storage that this strip is using
	for(typename BlockList::Iterator pBlock = m_Blocks.Begin(); pBlock != m_Blocks.End(); )
	{
		ElementBlockType* pDelete = &(*pBlock);
		pBlock = m_Blocks.Erase(pBlock);
		FreeBlock(m_pManager, pDelete, m_iElementSize);
	}
}

template<class T_Element, class T_Strip>
void C_Strip<T_Element, T_Strip>::PopFront()
{
	if(m_Blocks.Size() > 0)
	{
		ElementBlockType* pBlock = &m_Blocks.Front();

		assert(U32(pBlock->m_Element.Size()) > m_iOffset);
		++m_iOffset;
		if(m_iOffset == pBlock->m_Element.Size())
		{
			// if offset is the same as block count remove block
			m_Blocks.PopFront();
			FreeBlock(m_pManager, pBlock, m_iElementSize);
			m_iOffset = 0;
		}
		else if(&m_Blocks.Front() == &m_Blocks.Back() && m_iOffset == pBlock->m_Element.Size() - 1)
		{
			// if this is only block and offset is 1 less than count then pop
			m_Blocks.PopFront();
			FreeBlock(m_pManager, pBlock, m_iElementSize);
			m_iOffset = 0;
		}
	}
}

template<class T_Element, class T_Strip>
void C_Strip<T_Element, T_Strip>::PopBack()
{
	ElementBlockType* pBlock = NULL;
	
	if(m_Blocks.Size() > 0)
	{
		pBlock = &m_Blocks.Back();
		pBlock->m_Element.PopBack();

		if(pBlock->m_Element.Size() == 0)
		{
			// if block is empty then pop from block list and return
			m_Blocks.PopBack();
			FreeBlock(m_pManager, pBlock, m_iElementSize);
		}
		else if(m_iOffset == pBlock->m_Element.Size() && pBlock == &m_Blocks.Front())
		{
			// if this is the first block and offset equals block count then pop
			m_Blocks.PopFront();
			FreeBlock(m_pManager, pBlock, m_iElementSize);

			m_iOffset = 0;
		}

		// see if we need to remove the first block
		if(!m_Blocks.Empty())
		{
			pBlock = &m_Blocks.Front(); 
			if(pBlock == &m_Blocks.Back() && pBlock->m_Element.Size() - m_iOffset < 2)
			{
				// if this is only block and count is 1 then pop
				m_Blocks.PopBack();
				FreeBlock(m_pManager, pBlock, m_iElementSize);
			}
		}
	}

	
}

template<class T_Element, class T_Strip>
T_Element* C_Strip<T_Element, T_Strip>::PushBack()
{
	ElementBlockType* pBlock = NULL;
	if(m_Blocks.Size() == 0)
	{
		pBlock = NewBlock(m_pManager, m_iElementSize);
		m_Blocks.PushBack(*pBlock);
	}

	pBlock = &m_Blocks.Back();
	
	if(pBlock->m_Element.Size() == GetBlockSize(m_pManager, m_iElementSize))
	{
		if((pBlock = NewBlock(m_pManager, m_iElementSize)))
			m_Blocks.PushBack(*pBlock);
	}

	if(pBlock)
		return (ElementType*)pBlock->m_Element.PushBack();
	else
		return NULL;
}

template<class T_Element, class T_Strip>
void C_Strip<T_Element, T_Strip>::CalculateExtents()
{
	Maths::HVec3 vMax(Maths::HF32(-10000.0f));
	Maths::HVec3 vMin(Maths::HF32(10000.0f));

	for(typename BlockList::ConstIterator pIterator = m_Blocks.Begin(); pIterator != m_Blocks.End(); ++pIterator)
	{
		U32 begin = 0;
		const U32 end = pIterator->m_Element.Size();

		if(pIterator == m_Blocks.Begin())
			begin = m_iOffset;

		for(U32 i = begin; i < end; ++i)
		{
			const ElementType* pElement = (const ElementType*)&pIterator->m_Element[i];
			Maths::HVec3 vPosition = pElement->GetPosition();

			vMax = Maths::Max(vMax, vPosition);
			vMin = Maths::Min(vMin, vPosition);
		}
	}

	SetBounds(Maths::C_AxisBox(vMin, vMax), 1000.0f);
}



} // Strip
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_STRIP_EFFECT_H
