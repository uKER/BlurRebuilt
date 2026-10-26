#ifndef PAGED_ARRAY_H
#define PAGED_ARRAY_H

#ifdef _MSC_VER
#pragma once
#endif

//#include "assert.h"

#define PAGED_ARRAY_MAX_SIZE		(1024 * 8)


namespace Audio
{

	template<class T> class Container
	{

	public:
		Container() { m_Contents = 0; m_u16Pid = 0; }

		inline void SetContents(T Contents)
		{
			m_Contents = Contents;
			m_u16Pid = (m_u16Pid % 0xfe) + 1;	// make sure pid is never 0 as invalid handle is 0
		}


	public:

		T									m_Contents;
		U16									m_u16Pid;
	};

	template<class T> class PagedArray
	{

	public:
		PagedArray();
		~PagedArray() { assert (m_u16ArraySize - m_u16NumFreeNodes == 0); if (m_pContainers) delete [] m_pContainers; }

		PagedArrayHandle					AddEntry(T toAdd);
		inline PagedArrayHandle				GetHandleFromIndex(U16 u16Index) { return CreateHandle(m_pContainers[u16Index].m_u16Pid, u16Index); }
		T									GetByHandle(PagedArrayHandle hNode);
		T									GetByIndex(U16 u16Index);
		inline T							operator [](U16 u16Index) { return GetByIndex(u16Index); }
		bool								Free(PagedArrayHandle hNode);
		bool								FreeByIndex(U16 u16Index);
		bool								IsValid(PagedArrayHandle hNode);
		const inline U32					GetSize() { return m_u16ArraySize - m_u16NumFreeNodes; }
		const inline U32					GetHighestNodeIndex() { return m_u16ArraySize; }
		const inline U32					IntDivideBy32(U32 u32Int) { return (u32Int >> 5); }
		const inline U32					Mod32(U32 u32Int) { return (u32Int & 31); }

//		inline PagedArrayHandle				InvalidHandle() { return PagedArrayInvalidHandle(); }

	private:
		inline void							SetContents(U16 u16EntryNum, T Contents);
		const inline U16					ExtractEntryNumber(PagedArrayHandle hNode) { return (hNode.GetIndex()); }
		const inline U16					ExtractUniqueId(PagedArrayHandle hNode) { return hNode.GetUniqueId(); }
		inline PagedArrayHandle				CreateHandle(U16 u16UniqueId, U16 u16EntryNum)
		{
			return PagedArrayHandle(u16UniqueId, u16EntryNum);
		}



		Container<T>*						m_pContainers;
		U16									m_u16NumFreeNodes;
		U16									m_u16ArraySize;
		U32									m_pFreeNodeBitArray[PAGED_ARRAY_MAX_SIZE / 32];
	};

}



template<class T>
Audio::PagedArray<T>::PagedArray()
{
	// make sure max array size is divisble by 32
	assert(! Mod32(PAGED_ARRAY_MAX_SIZE));

	// zero bit array, one bit per array element
#ifndef PLATFORM_PS3
	ZeroMemory(&m_pFreeNodeBitArray, PAGED_ARRAY_MAX_SIZE / 8);
#else
	memset(&m_pFreeNodeBitArray, 0x00, PAGED_ARRAY_MAX_SIZE / 8);
#endif
	m_pContainers = new Container<T>[32];

	m_u16ArraySize = 32;

	// we've allocated an initial array with 32 elements, so set 1st 32 bits of bit array to 1
	m_pFreeNodeBitArray[0] = 0xffffffff;

	m_u16NumFreeNodes = 32;
}


template<class T>
Audio::PagedArrayHandle Audio::PagedArray<T>::AddEntry(T toAdd)
{
	U32 u32ElementToPopulate = 0;

	if (m_u16NumFreeNodes)
	{
		// there are free elements in the list, locate them using the bit array
		U32 u32PageWithFreeElements = IntDivideBy32(m_u16ArraySize);
		for (U32 i = 0; i < IntDivideBy32(m_u16ArraySize); i++)
		{
			if (m_pFreeNodeBitArray[i])	// means there are free entries in this 32 bit portion
			{
				u32PageWithFreeElements = i;
				break;
			}
		}

		for (U32 i = 0; i < 32; i++)
		{
			if (m_pFreeNodeBitArray[u32PageWithFreeElements] & (1 << i))
			{
				u32ElementToPopulate = i + (u32PageWithFreeElements << 5);
				break;
			}
		}
	}
	else
	{
		if (m_u16ArraySize > PAGED_ARRAY_MAX_SIZE)
		{
			assert (0 && "Audio- paged array has exceeded max size");
			DebugBreak();
			return PagedArrayHandle();	//	returns invalid handle
		}

		// add new entry after highest used element of array
		if (! Mod32(m_u16ArraySize))	// new entry has crossed 32 element page boundary
		{
			Container<T>* pNewContainers = new Container<T>[m_u16ArraySize + 32];

			if (m_pContainers)
			{
				memcpy(pNewContainers, m_pContainers, sizeof(Container<T>) * (m_u16ArraySize));
				delete [] m_pContainers;
			}
			m_pContainers = pNewContainers;
			m_pFreeNodeBitArray[IntDivideBy32(m_u16ArraySize)] = 0xffffffff;
			m_u16NumFreeNodes += 32;
		}

		u32ElementToPopulate = m_u16ArraySize;
		m_u16ArraySize += 32;
	}

	if (u32ElementToPopulate > m_u16ArraySize)
		m_u16ArraySize = (U16)u32ElementToPopulate;

	m_u16NumFreeNodes--;

	SetContents((U16)u32ElementToPopulate, toAdd);
	return CreateHandle(m_pContainers[u32ElementToPopulate].m_u16Pid, (U16)u32ElementToPopulate);
}


template<class T>
inline T Audio::PagedArray<T>::GetByHandle(PagedArrayHandle hNode)
{
	if (! IsValid(hNode))
		return 0;

	return m_pContainers[ExtractEntryNumber(hNode)].m_Contents;
}


template<class T>
inline T Audio::PagedArray<T>::GetByIndex(U16 u16Index)
{
	U32 u32Bit = 1 << (u16Index & 0x1f);

	if (u16Index >= m_u16ArraySize || (m_pFreeNodeBitArray[u16Index >> 5] & u32Bit))
		return 0;

	return m_pContainers[u16Index].m_Contents;
}


template<class T>
bool Audio::PagedArray<T>::Free(PagedArrayHandle hNode)
{
	U16 u16EntryNum = ExtractEntryNumber(hNode);

	FreeByIndex(u16EntryNum);

	return true;
}


template<class T>
bool Audio::PagedArray<T>::FreeByIndex(U16 u16Index)
{
	U32 u32Bit = 1 << (Mod32(u16Index));

	if (u16Index >= m_u16ArraySize || (m_pFreeNodeBitArray[IntDivideBy32(u16Index)] & u32Bit))
		return false;

	m_pContainers[u16Index].m_Contents = 0;

	m_pFreeNodeBitArray[IntDivideBy32(u16Index)] |= u32Bit;
	
	m_u16NumFreeNodes++;

	// let's check to see if we can make the array any smaller
	U32 u32MaxNecessaryArraySize = m_u16ArraySize;

	for (U32 i = (IntDivideBy32(m_u16ArraySize)) - 1; i > 0; i--)
	{
		if (m_pFreeNodeBitArray[i] == 0xffffffff)
			u32MaxNecessaryArraySize -= 32;
		else
			break;	// we can only make smaller if cells at the end of the array are free, so don't carry on checking
	}

	if (u32MaxNecessaryArraySize != m_u16ArraySize && u32MaxNecessaryArraySize >= 32)
	{
		// let's not allow the array to be resized to 0, as it gets created with a default size of 32..

/*		if (u32MaxNecessaryArraySize == 0)
		{
			delete [] m_pContainers;
			m_u16ArraySize = 0;
			m_u16NumFreeNodes = 0;
			m_pContainers = 0;
		}
		else*/
		{
			U32 u32ArrayDiff = m_u16ArraySize - u32MaxNecessaryArraySize;
			Container<T>* pNewContainers = new Container<T>[u32MaxNecessaryArraySize];
			memcpy(pNewContainers, m_pContainers, sizeof(Container<T>) * (u32MaxNecessaryArraySize));
			delete [] m_pContainers;
			m_pContainers = pNewContainers;
			m_u16ArraySize = (U16)u32MaxNecessaryArraySize;
			m_u16NumFreeNodes -= u32ArrayDiff;
		}
	}

	return true;
}


template<class T>
bool Audio::PagedArray<T>::IsValid(PagedArrayHandle hNode)
{
	if (! hNode.IsValid())
		return false;

	U16 u16EntryNum = ExtractEntryNumber(hNode);

	if (u16EntryNum >= m_u16ArraySize)
		return false;

	if (m_pFreeNodeBitArray[u16EntryNum >> 5] & (1 << (u16EntryNum & 0x1f)))	// bit set to 1 indicates element is free
		return false;
	else
		return (m_pContainers[u16EntryNum].m_u16Pid == ExtractUniqueId(hNode));
}


template<class T>
void Audio::PagedArray<T>::SetContents(U16 u16EntryNum, T Contents)
{
	U32 u32BitToSet = (Mod32(u16EntryNum));
	m_pFreeNodeBitArray[IntDivideBy32(u16EntryNum)] &= ~(1 << u32BitToSet);

	m_pContainers[u16EntryNum].SetContents(Contents);
}


#endif	//	PAGED_ARRAY_H

