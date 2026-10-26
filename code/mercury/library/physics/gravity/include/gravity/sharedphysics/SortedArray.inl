#ifndef __SORTEDARRAY_INL_H__
#define __SORTEDARRAY_INL_H__

//#include "SortedArray.h"
#include "gravity/sharedPhysics/SafeStack.h"
#include "gravity/sharedPhysics/spu_memory.h"

namespace SharedPhysics
{

	template <class Elem>
	SortedArray<Elem>::SortedArray(S32 _size) : m_pPool(NULL), m_size(_size), m_numElements(0),
		m_MaxGrowth(DEFAULT_MAX_GROWTH)
	{
#ifndef PLATFORM_SPU
		m_pElems = new Elem[m_size];
#else
		m_pElems = (Elem*)Spu::Alloc(m_size * sizeof(Elem));
#endif
	}

	template <class Elem>
	SortedArray<Elem>::SortedArray(C_SPUMemoryPool* pPool, S32 _size) : m_pPool(pPool), m_pElems(NULL), m_size(_size), m_numElements(0),
		m_MaxGrowth(DEFAULT_MAX_GROWTH)
	{
		//m_pElems = (Elem*)m_pPool->Allocate<Elem, false>(m_size);
	}

	template <class Elem>
	SortedArray<Elem>::~SortedArray()
	{
#ifndef PLATFORM_SPU
		if(m_pPool == NULL) //If no memory pool for allocations!
		{
			if(m_pElems)
			{
				delete [] m_pElems;
			}
		}
#endif
		m_pElems = NULL;
		m_numElements = 0;
		m_size = 0;
	}

template <class Elem>
NOALIAS inline void SortedArray<Elem>::InsertOverwrite(Elem* elem)
{
	int ival = 0, mid;
	int l = 0;
	int r = m_numElements;
	while(l < r)
	{
		mid = (l + r)>>1;
		if(*elem < m_pElems[mid])
		{
			r = mid;
			ival = mid;
		}
		else if(*elem > m_pElems[mid])
		{
			l = mid + 1;
			ival = mid + 1;
		}
		else
		{
			m_pElems[mid]=*elem;
			//vals[mid] = *elem;
			return;
		}
	}

	if(m_numElements >= m_size)
	{
		const int growth = Maths::Min(m_size + 16, m_MaxGrowth);
		Resize(m_size + growth);
	}

	for(int i = m_numElements; i > ival; --i)
	{
		m_pElems[i] = m_pElems[i-1];
	}
	m_pElems[ival] = *elem;
	++m_numElements;
}
		
template <class Elem>
NOALIAS inline void SortedArray<Elem>::Insert(Elem* elem)
{
	int ival = 0, mid;
	int l = 0;
	int r = m_numElements;
	while(l < r)
	{
		mid = (l + r)>>1;
		if(*elem < m_pElems[mid])
		{
			r = mid;
			ival = mid;
		}
		else if(*elem > m_pElems[mid])
		{
			l = mid + 1;
			ival = mid + 1;
		}
		else
		{
			ival = mid;
			break;
		}
	}

	if(m_numElements >= m_size)
	{
		const int growth = Maths::Min(m_size + 16, m_MaxGrowth);
		Resize(m_size + growth);
	}

	for(int i = m_numElements; i > ival; --i)
	{
		m_pElems[i] = m_pElems[i-1];
	}
	m_pElems[ival] = *elem;
	++m_numElements;
}

template <class Elem>
NOALIAS inline void SortedArray<Elem>::RestoreDataBlocks()
{
	if(m_pPool != NULL)
	{
		m_pElems = m_pPool->Allocate<Elem, false>(m_size);
	}
	m_numElements = 0;
}

template <class Elem>
NOALIAS inline void SortedArray<Elem>::Resize(int size)
{
	if(size <= m_size)
		return;

	//const int growth = Maths::Max(_size - m_size, m_MaxGrowth);
	//const int size = m_size + growth;

	if(m_pPool != NULL)
	{
		Elem* tmp = m_pElems;
		m_pElems = m_pPool->Allocate<Elem, false>(size);
#ifndef PLATFORM_SPU
		for(S32 a = 0; a < m_numElements; ++a)
		{
			m_pElems[a] = tmp[a];
		}
#else
		//DebugBreak();
		//On SPU, we need to memcpy from ppu memory to ppu memory
		Spu::MemBaseMarker marker;
		const S32 bytes = ((sizeof(Elem) * m_numElements) + 15) & 0xfffffff0; 
		S32 end = bytes - 1024;
		char* oldPtr = (char*)tmp;
		char* newPtr = (char*)m_pElems;
		S32 a;
		char* data = (char*)Spu::Alloc(1024);

		for(a = 0; a <= end; a += 1024)
		{
			//Spu::MemBaseMarker marker;
			//const char* data = (char*)Spu::FetchAlloc(oldPtr + a, 1024);
			Spu::Fetch(data, oldPtr + a, 1024);
			Spu::Store(newPtr + a, data, 1024);
		}

		end = bytes - 128;
		for(; a <= end; a += 128)
		{
			//Spu::MemBaseMarker marker;
			//const char* data = (char*)Spu::FetchAlloc(oldPtr + a, 128);
			Spu::Fetch(data, oldPtr + a, 128);
			Spu::Store(newPtr + a, data, 128);
		}

		end = bytes - 16;
		for(; a <= end; a += 16)
		{
			//Spu::MemBaseMarker marker;
			//const char* data = (char*)Spu::FetchAlloc(oldPtr + a, 16);
			Spu::Fetch(data, oldPtr + a, 16);
			Spu::Store(newPtr + a, data, 16);
		}

#endif
		this->m_size = size;
	}
#ifndef PLATFORM_SPU
	else
	{
		Elem* tmp = m_pElems;
		m_pElems = new Elem[size];
		for(S32 a = 0; a < m_numElements; ++a)
		{
			m_pElems[a] = tmp[a];
		}
		this->m_size = size;
		delete [] tmp;
	}
#endif
}

template <class Elem>
NOALIAS inline bool SortedArray<Elem>::Remove(Elem* elem)
{
	int l = 0, r = m_numElements, mid;
	while(l < r)
	{
		mid = (l + r)/2;
		if(*elem < m_pElems[mid])
		{
			r = mid;
		}
		else if(*elem > m_pElems[mid])
		{
			l = mid + 1;
		}
		else
		{
			//Same value, so remove...
			--m_numElements;
			for(S32 a = mid; a < m_numElements; ++a)
			{
				m_pElems[a] = m_pElems[a+1];
			}
			return true;
		}
	}
	return false;
}

template <class Elem>
NOALIAS inline void SortedArray<Elem>::Remove(S32 index, S32 size)
{
	//Same value, so remove...
	m_numElements -= size;
	for(S32 a = index; a < m_numElements; ++a)
	{
		m_pElems[a] = m_pElems[a+size];
	}
}

//NB. This will break sorting
template <class Elem>
NOALIAS inline void SortedArray<Elem>::QuickRemove(S32 index)
{
	//Same value, so remove...
	m_numElements -= 1;
	m_pElems[index] = m_pElems[m_numElements];
}


template <class Elem>
NOALIAS inline void SortedArray<Elem>::AddToEnd(Elem* elem)
{
	if(m_numElements >= m_size)
	{
		const int growth = Maths::Min(m_size + 16, m_MaxGrowth);
		Resize(m_size + growth);
	}
#ifdef PLATFORM_SPU
	if(m_pPool)
	{
		Spu::Store(m_pElems + m_numElements++, elem, sizeof(Elem));
	}
	else
#endif
		m_pElems[m_numElements++] = *elem;
}

template <class Elem>
NOALIAS inline Elem* SortedArray<Elem>::AddToEndElement(Elem* elem)
{
	if(m_numElements >= m_size)
	{
		const int growth = Maths::Min(m_size + 16, m_MaxGrowth);
		Resize(m_size + growth);
	}
#ifdef PLATFORM_SPU
	if(m_pPool)
	{
		Spu::Store(m_pElems + m_numElements, elem, sizeof(Elem));
	}
	else
#endif
		m_pElems[m_numElements] = *elem;

	return &m_pElems[m_numElements++];
}


template <class Elem>
NOALIAS inline Elem* SortedArray<Elem>::GetNextFreeElem()
{
	if(m_numElements >= m_size)
	{
		const int growth = Maths::Min(m_size + 16, m_MaxGrowth);
		Resize(m_size + growth);
	}
	return &m_pElems[m_numElements++];
}

template <class Elem>
NOALIAS inline void SortedArray<Elem>::QuickSort(C_S32SafeStack& stack, const int qSortThreshold)
{
	//An implementation of quicksort to help sort the data...
	if(m_numElements < 2)
		return;
	S32 size = 2;
	stack.Push(m_numElements - 1, 0);
	stack.Push(0, 1);
	S32 l, r, l2, r2, /*mid,*/ diff, diff2, a;
	Elem pivot;
	Elem temp;
	bool bChanged;
	while(size)
	{
		l = stack.Pop(size-1);
		r = stack.Pop(size-2);

		BIZ_ASSERT(l < m_numElements && l >= 0);
		BIZ_ASSERT(r < m_numElements && r >= 0);

		size-=2;
		diff = r - l;
		if(diff < qSortThreshold)
		{
			//Do bubble sort
			do
			{
				bChanged = false;
				for(a = l; a < r; ++a)
				{
					if(m_pElems[a] > m_pElems[a+1])
					{
						temp = m_pElems[a];
						m_pElems[a] = m_pElems[a+1];
						m_pElems[a+1] = temp;
						bChanged = true;
					}
				}
			}
			while(bChanged);
			continue;
		}

		l2 = l;
		r2 = r;
		/*U32 tempIndex = (l + r)/2;
		pivot = m_pElems[tempIndex];
		m_pElems[tempIndex] = m_pElems[l];
		m_pElems[l] = pivot;*/
		S32 m = (l + r)/2;
		pivot = m_pElems[m];
		{
			//Elem e = m_pElems[m];
			m_pElems[m] = m_pElems[l];
			m_pElems[l] = pivot;
		}
		
		++l;
		do
		{
			while((m_pElems[l] <= pivot) && (l < r))
				++l;
			while(m_pElems[r] > pivot)
				--r;
			if(l < r)
			{
				BIZ_ASSERT(l < m_numElements && l >= 0);
				BIZ_ASSERT(r < m_numElements && r >= 0);
				//Do a swap...
				temp = m_pElems[l];
				m_pElems[l] = m_pElems[r];
				m_pElems[r] = temp;
			}
		}
		while(l < r);
		//both pointing at the same place so it must be the spot to put the pivot
		BIZ_ASSERT(r < m_numElements && r >= 0);
		m_pElems[l2] = m_pElems[r];
		m_pElems[r] = pivot; //pivot put in final position...

		//Push into the stack...
		diff = r2 - r - 1; //number on top half
		diff2 = r - l2 - 1;

		/*if(diff < 0 || diff2 < 0)
		{
			do
			{
				bChanged = false;
				for(a = l2; a < r2; ++a)
				{
					if(m_pElems[a] > m_pElems[a+1])
					{
						temp = m_pElems[a];
						m_pElems[a] = m_pElems[a+1];
						m_pElems[a+1] = temp;
						bChanged = true;
					}
				}
			}
			while(bChanged);
		}
		else*/
		{
			stack.Push(r2, size);
			stack.Push(r + 1, size + 1);
			size += ((diff > 0)<<1);

			//diff = r - l2 - 1; //number on bottom half
			stack.Push(r - 1, size);
			stack.Push(l2, size + 1);
			size += ((diff2 > 0)<<1);
		}
	}

	//Check to see if its ordered!

#if 0 //def USER_KSTOREY
	for(S32 a = 0; a < m_numElements - 1; ++a)
	{
		BIZ_ASSERT(m_pElems[a] <= m_pElems[a+1]);
	}
#endif

}

template <class Elem>
NOALIAS inline Elem* SortedArray<Elem>::Find(Elem* val) const
{
	int l = 0, r = m_numElements, mid;
	while(l < r)
	{
		mid = (l + r)/2;
		//cmp = CompFunc(val, m_pElems + mid);
		if(*val < m_pElems[mid])
			r = mid;
		else if(*val > m_pElems[mid])
			l = mid + 1;
		else
			return m_pElems + mid;
	}
	return NULL;
}

template <class Elem>
NOALIAS inline int SortedArray<Elem>::FindIndex(Elem* elem, int& insertPos) const
{
	int l = 0, r = m_numElements, mid;
	insertPos = 0;
	while(l < r)
	{
		mid = (l + r)/2;
		//cmp = CompFunc(elem, m_pElems + mid);
		if(*elem < m_pElems[mid])
		{
			r = mid;
			insertPos = r;
		}
		else if(*elem > m_pElems[mid])
		{
			l = mid + 1;
			insertPos = l;
		}
		else
			return mid;
	}
	return -1;
}

template <class Elem>
NOALIAS inline void SortedArray<Elem>::Insert(Elem* elem, int index)
{
	if(m_numElements == m_size)
	{
		const int growth = Maths::Min(m_size + 16, m_MaxGrowth);
		Resize(m_size + growth);
	}
	for(int a = m_numElements; a > index; --a)
	{
		m_pElems[a] = m_pElems[a-1];
	}
	m_pElems[index] = *elem;
	++m_numElements;
}

template <class Elem>
NOALIAS inline void SortedArray<Elem>::Set(Elem* elem, S32 index)
{
	BIZ_ASSERT(index < m_numElements);
	m_pElems[index] = *elem;
}

template <class Elem>
NOALIAS inline bool SortedArray<Elem>::IsEmpty() const
{
	return m_numElements == 0;
}

template <class Elem>
NOALIAS inline void SortedArray<Elem>::Clear()
{
	m_numElements = 0;
}

};

#endif
