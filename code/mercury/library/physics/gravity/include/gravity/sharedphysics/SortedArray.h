#ifndef SORTED_ARRAY_H
#define SORTED_ARRAY_H

//#include "gravity/sharedphysics/safestack.h"


namespace SharedPhysics
{


#define DEFAULT_MAX_GROWTH 8192

	class C_SPUMemoryPool;
	class C_S32SafeStack;

	//This is a helper wrapper class to wrap pointers being stored the sorted array
	template <class Elem>
	class C_PointerWrapper
	{
	public:
		C_PointerWrapper() : m_pElement(NULL)
		{
		}
		C_PointerWrapper(const C_PointerWrapper& other) : m_pElement(other.m_pElement)
		{
		}
		C_PointerWrapper(Elem* pElem) : m_pElement(pElem)
		{
		}
		C_PointerWrapper& operator = (const C_PointerWrapper& other)
		{
			m_pElement = other.m_pElement;
			return *this;
		}

		NOALIAS inline bool operator < (const C_PointerWrapper& other) const
		{
			return *m_pElement < *other.m_pElement;
		}

		NOALIAS inline bool operator <= (const C_PointerWrapper& other) const
		{
			return *m_pElement <= *other.m_pElement;
		}

		NOALIAS inline bool operator > (const C_PointerWrapper& other) const
		{
			return *m_pElement > *other.m_pElement;
		}

		NOALIAS inline bool operator >= (const C_PointerWrapper& other) const
		{
			return *m_pElement >= *other.m_pElement;
		}

		NOALIAS inline bool operator == (const C_PointerWrapper& other) const
		{
			return *m_pElement == *other.m_pElement;
		}

		NOALIAS inline bool operator != (const C_PointerWrapper& other) const
		{
			return *m_pElement == *other.m_pElement;
		}

		Elem* m_pElement;
	};

	template <class Elem>
class C_RadixPointerWrapper
{
public:
	public:
		C_RadixPointerWrapper() : m_pElement(NULL)
		{
		}
		C_RadixPointerWrapper(const C_RadixPointerWrapper& other) : m_pElement(other.m_pElement)
		{
		}
		C_RadixPointerWrapper(Elem* pElem) : m_pElement(pElem)
		{
		}
		C_RadixPointerWrapper& operator = (const C_RadixPointerWrapper& other)
		{
			m_pElement = other.m_pElement;
			return *this;
		}

		NOALIAS inline bool operator < (const C_RadixPointerWrapper& other) const
		{
			return *m_pElement < *other.m_pElement;
		}

		NOALIAS inline bool operator <= (const C_RadixPointerWrapper& other) const
		{
			return *m_pElement <= *other.m_pElement;
		}

		NOALIAS inline bool operator > (const C_RadixPointerWrapper& other) const
		{
			return *m_pElement > *other.m_pElement;
		}

		NOALIAS inline bool operator >= (const C_RadixPointerWrapper& other) const
		{
			return *m_pElement >= *other.m_pElement;
		}

		NOALIAS inline bool operator == (const C_RadixPointerWrapper& other) const
		{
			return *m_pElement == *other.m_pElement;
		}

		NOALIAS inline bool operator != (const C_RadixPointerWrapper& other) const
		{
			return *m_pElement == *other.m_pElement;
		}

		/*NOALIAS __forceinline U32& GetRadix()
		{
			return m_pElement->GetRadix();
		}*/

		NOALIAS inline U32& GetRadix()
		{
			return m_pElement->GetRadix();
		}

		Elem* m_pElement;
};

	template <class Elem>
	class SortedArray
	{
	public:
		C_SPUMemoryPool* m_pPool;
		Elem* m_pElems;
		S32 m_size;
		S32 m_numElements;
		S32 m_MaxGrowth;

		SortedArray(S32 _size = 256);

		SortedArray(C_SPUMemoryPool* pPool, S32 _size = 256);

		SortedArray<Elem>& operator = (SortedArray<Elem>& other)
		{
			Resize(other.m_size);
			m_numElements = other.m_numElements;
			for(S32 a = 0; a < m_numElements; ++a)
			{
				m_pElems[a] = other.m_pElems[a];
			}
			return *this;
		}

		~SortedArray();

		NOALIAS inline U32 Size() { return m_numElements; }

		NOALIAS inline void RestoreDataBlocks();
		NOALIAS inline void InsertOverwrite(Elem* elem);
		NOALIAS inline void Insert(Elem* elem);
		NOALIAS inline void Resize(int size);
		NOALIAS inline bool Remove(Elem* elem);
		NOALIAS inline void QuickRemove(S32 index);
		NOALIAS inline void Remove(S32 index, S32 size);

		NOALIAS FORCE_INLINE void Set(Elem* elem, S32 index);

		Elem& operator [] (const S32 index) const
		{
			BIZ_ASSERT(index >= 0 && index < m_numElements);
			return m_pElems[index];
		}


		NOALIAS inline Elem* AddToEndElement(Elem* elem);
		NOALIAS inline void AddToEnd(Elem* elem);
		NOALIAS inline Elem* GetNextFreeElem();

//#if 1 //def USE_QUICKSORT
		//NOALIAS __forceinline void QuickSort(int* stack, const int qSortThreshold);
		NOALIAS inline void QuickSort(C_S32SafeStack& stack, const int qSortThreshold);
//#endif

		NOALIAS inline Elem* Find(Elem* val) const;
		//Elem* Find(Elem* val)
		//{
		//	int l = 0, r = m_numElements, mid;
		//	while(l < r)
		//	{
		//		mid = (l + r)/2;
		//		//cmp = CompFunc(val, m_pElems + mid);
		//		if(*val < m_pElems[mid])
		//			r = mid;
		//		else if(*val > m_pElems[mid])
		//			l = mid + 1;
		//		else
		//			return m_pElems + mid;
		//	}
		//	return NULL;
		//}

		NOALIAS inline int FindIndex(Elem* elem, int& insertPos) const;

		//int FindIndex(Elem* elem, int& insertPos)
		//{
		//	int l = 0, r = m_numElements, mid;
		//	insertPos = 0;
		//	while(l < r)
		//	{
		//		mid = (l + r)/2;
		//		//cmp = CompFunc(elem, m_pElems + mid);
		//		if(*elem < m_pElems[mid])
		//		{
		//			r = mid;
		//			insertPos = r;
		//		}
		//		else if(*elem > m_pElems[mid])
		//		{
		//			l = mid + 1;
		//			insertPos = l;
		//		}
		//		else
		//			return mid;
		//	}
		//	return -1;
		//}

		NOALIAS inline void Insert(Elem* elem, int index);
		/*void Insert(Elem* elem, int index)
		{
			if(m_numElements == m_size)
			{
				Resize((m_size * 2)+1);
			}
			for(int a = m_numElements; a > index; --a)
			{
				m_pElems[a] = m_pElems[a-1];
			}
			m_pElems[index] = *elem;
			++m_numElements;
		}*/

		NOALIAS inline bool IsEmpty() const;
		/*bool IsEmpty()
		{
			return m_numElements == 0;
		}*/

		NOALIAS inline void Clear();
	/*	{
			m_numElements = 0;
		}*/
	};
};

#include "sortedarray.inl"

#endif

