#ifndef BIT_LIMITED_PAGED_ARRAY_H
#define BIT_LIMITED_PAGED_ARRAY_H

#ifdef _MSC_VER
#pragma once
#endif

//#include "Assert.h"

#define BL_PAGED_ARRAY_MAX_SIZE		(1024 * 8)	// this in theory could stay but might be nice to make as 3rd template param?

// TODO: get rid of any U16s and make them U32s. better for performance.

// note: T should be a pointer to your class/struct. 
// if you use this paged array class with non-pointers it will be slow!
namespace Audio
{

	typedef	U32			NODE_HANDLE;
	const	U32			INVALID_HANDLE = 0;

	// N is max number of bits to use for the array index
	// P is the maxnumber of bits for the Pid bits
	template<class T, int N, int P> class BlPagedArray
	{
	public:

		static const U32	ENTRY_MASK = ((1<<N)-1);
		static const U32	PID_MASK = ((1<<P)-1);

		class Container
		{

		public:
			Container() { m_Contents = 0; m_u16Pid = 0; }						
			void								SetContents(T Contents);

		public:

			T									m_Contents;
			U16									m_u16Pid;			// WHY PID HERE	AS WELL AS IN THE NODE HANDLE?
		};

		BlPagedArray();
		~BlPagedArray() { Assert (m_u16ArraySize - m_u16NumFreeNodes == 0); if (m_pContainers) delete [] m_pContainers; }

		NODE_HANDLE							AddEntry(T toAdd);
		T									Get(NODE_HANDLE hNode) const;
		T									GetByIndex(U16 u16Index) const;
		bool								Free(NODE_HANDLE hNode);
		bool								FreeByIndex(U16 u16Index);
		bool								IsValid(NODE_HANDLE hNode) const;
		inline U32							GetSize() const { return m_u16ArraySize - m_u16NumFreeNodes; }
		inline U32							GetHighestNodeIndex() const { return m_u16ArraySize; }
		//void								Clear();

		T operator [] (int n)				const { return GetByIndex(n); }

	private:
		
		typedef struct NodeHandleInternal
		{
			NODE_HANDLE u32Handle;
			
			NodeHandleInternal(U32 pid, U32 index) { u32Handle = pid << N | index;   }
			
			bool operator ==(NODE_HANDLE hComp) { return (u32Handle == hComp); }
			void operator =(NODE_HANDLE hAssign) { u32Handle = hAssign; }
		}NodeHandleInternal;
	private:
		void								SetContents(U32 u16EntryNum, T Contents);
		bool								EntryNumExceedsMaxBits(U32 u16EntryNum) const { return (u16EntryNum >> N) ? true : false; } 
		bool								PidExceedsMaxBits(U32 u16Pid) const { return (u16Pid >> P) ? true : false; } 
		inline U32							ExtractEntryNumber(NODE_HANDLE hNode) const { return (hNode & ENTRY_MASK); }
		inline U32							ExtractPid(NODE_HANDLE hNode) const {  return ((hNode >> N) & PID_MASK); }
		NODE_HANDLE							CreateHandle(U32 u16Pid, U32 u16EntryNum);

		Container*							m_pContainers;
		U16									m_u16NumFreeNodes;			// ?
		U16									m_u16ArraySize;				// ?
		U32									m_pFreeNodeBitArray[BL_PAGED_ARRAY_MAX_SIZE / 32];
	};

}


template<class T, int N, int P>
void Audio::BlPagedArray<T, N, P>::Container::SetContents(T Contents)
{
	m_Contents = Contents;		
	// limit this to P num bits
	m_u16Pid = (m_u16Pid % Audio::BlPagedArray<T, N, P>::PID_MASK) + 1;	// make sure pid is never 0 as invalid handle is 0
}


template<class T, int N, int P>
Audio::BlPagedArray<T, N, P>::BlPagedArray()
{
	// make sure max array size is divisble by 32
	Assert(! (BL_PAGED_ARRAY_MAX_SIZE % 32));

	// zero bit array, one bit per array element
#if !defined (PLATFORM_PS3) && !defined (PLATFORM_SPU)
	ZeroMemory(&m_pFreeNodeBitArray, BL_PAGED_ARRAY_MAX_SIZE / 8);
#else
	memset(&m_pFreeNodeBitArray, 0x00, BL_PAGED_ARRAY_MAX_SIZE / 8);
#endif
	m_pContainers = new Container[32];

	m_u16ArraySize = 32;

	// we've allocated an initial array with 32 elements, so set 1st 32 bits of bit array to 1
	m_pFreeNodeBitArray[0] = 0xffffffff;

	m_u16NumFreeNodes = 32;
}


template<class T, int N, int P>
Audio::NODE_HANDLE Audio::BlPagedArray<T, N, P>::AddEntry(T toAdd)
{
	U32 u32ElementToPopulate = 0;
	
	Assert(toAdd != NULL);

	if (! m_u16NumFreeNodes)
	{
		// no free nodes so resize the array

		// TODO:check array size doesnt exceed max bits for array
		if (m_u16ArraySize > BL_PAGED_ARRAY_MAX_SIZE)
		{
			#ifndef PLATFORM_SPU
			AUDIO_SYS_FATAL("Audio- bit limited paged array has exceeded max size");
			#endif
			DebugBreak();
			return INVALID_HANDLE;
		}

		if (EntryNumExceedsMaxBits(m_u16ArraySize))
		{
			#ifndef PLATFORM_SPU
			AUDIO_SYS_FATAL("Bit limited paged array has exceeded %d bits for entry num\n", N);
			#endif
			return INVALID_HANDLE;
		}

		// add new entry after highest used element of array
		if (! (m_u16ArraySize % 32))	// new entry has crossed 32 element page boundary
		{
			Container* pNewContainers = new Container[m_u16ArraySize + 32];

			if (m_pContainers)
			{
				memcpy(pNewContainers, m_pContainers, sizeof(Container) * (m_u16ArraySize));
				delete [] m_pContainers;
			}
			m_pContainers = pNewContainers;
			m_pFreeNodeBitArray[m_u16ArraySize / 32] = 0xffffffff;
			m_u16NumFreeNodes += 32;
		}

		u32ElementToPopulate = m_u16ArraySize;
		m_u16ArraySize += 32;
	}
	else
	{
		// there are free elements in the list, locate them using the bit array
		U32 u32PageWithFreeElements = m_u16ArraySize / 32;
		for (U32 i = 0; i < m_u16ArraySize / (U32)32; i++)
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
				u32ElementToPopulate = i + (u32PageWithFreeElements * 32);
				break;
			}
		}

		if (EntryNumExceedsMaxBits(u32ElementToPopulate))
		{
			#ifndef PLATFORM_SPU
			AUDIO_SYS_FATAL("Bit limited paged array has exceeded %d bits for entry num\n", N);
			#endif
			return INVALID_HANDLE;
		}
	}

	if (u32ElementToPopulate > m_u16ArraySize)
		m_u16ArraySize = (U16)u32ElementToPopulate;

	m_u16NumFreeNodes--;

	SetContents((U16)u32ElementToPopulate, toAdd);

	if (PidExceedsMaxBits(m_pContainers[u32ElementToPopulate].m_u16Pid))
	{
		#ifndef PLATFORM_SPU
		AUDIO_SYS_FATAL("Bit limited paged array has exceeded %d bits for pid\n", P);
		#endif
		return INVALID_HANDLE;
	}

	//LogInfo("creating handle from index %d and pid %d\n", u32ElementToPopulate, m_pContainers[u32ElementToPopulate].m_u16Pid);

	return CreateHandle(m_pContainers[u32ElementToPopulate].m_u16Pid, (U16)u32ElementToPopulate);
}


template<class T, int N, int P>
T Audio::BlPagedArray<T, N, P>::Get(NODE_HANDLE hNode) const
{
	if (! IsValid(hNode))
	{
		#ifndef PLATFORM_SPU
		AUDIO_SYS_FATAL("Audio::BlPagedArray::Get() - handle 0x%x is not valid.", hNode);
		#endif
		return 0;
	}

	return m_pContainers[ExtractEntryNumber(hNode)].m_Contents;
}


template<class T, int N, int P>
T Audio::BlPagedArray<T, N, P>::GetByIndex(U16 u16Index) const
{
	U32 u32Bit = 1 << (u16Index % 32);

	if (u16Index >= m_u16ArraySize || (m_pFreeNodeBitArray[u16Index / 32] & u32Bit))
		return 0;

	return m_pContainers[u16Index].m_Contents;
}


template<class T, int N, int P>
bool Audio::BlPagedArray<T, N, P>::Free(NODE_HANDLE hNode)
{
	U16 u16EntryNum = ExtractEntryNumber(hNode);

	FreeByIndex(u16EntryNum);

	return true;
}


template<class T, int N, int P>
bool Audio::BlPagedArray<T, N, P>::FreeByIndex(U16 u16Index)
{
	U32 u32Bit = 1 << (u16Index % 32);

	if (u16Index >= m_u16ArraySize || (m_pFreeNodeBitArray[u16Index / 32] & u32Bit))
		return false;

	m_pContainers[u16Index].m_Contents = 0;

	m_pFreeNodeBitArray[u16Index / 32] |= u32Bit;
	
	m_u16NumFreeNodes++;

	// let's check to see if we can make the array any smaller
	U32 u32MaxNecessaryArraySize = m_u16ArraySize;

	for (U32 i = (m_u16ArraySize / 32) - 1; i > 0; i--)
	{
		if (m_pFreeNodeBitArray[i] == 0xffffffff)
			u32MaxNecessaryArraySize -= 32;
		else
			break;	// we can only make smaller if cells at the end of the array are free, so don't carry on checking
	}

	if (u32MaxNecessaryArraySize != m_u16ArraySize && u32MaxNecessaryArraySize >= 32)
	{
		U32 u32ArrayDiff = m_u16ArraySize - u32MaxNecessaryArraySize;
		Container* pNewContainers = new Container[u32MaxNecessaryArraySize];
		memcpy(pNewContainers, m_pContainers, sizeof(Container) * (u32MaxNecessaryArraySize));
		delete [] m_pContainers;
		m_pContainers = pNewContainers;
		m_u16ArraySize = (U16)u32MaxNecessaryArraySize;
		m_u16NumFreeNodes -= (U16)u32ArrayDiff;
	}

	return true;
}


template<class T, int N, int P>
bool Audio::BlPagedArray<T, N, P>::IsValid(NODE_HANDLE hNode) const
{
	U32 u16EntryNum = ExtractEntryNumber(hNode);

	if (u16EntryNum >= m_u16ArraySize)
	{
		#if !defined CFG_GOLD && !defined PLATFORM_SPU
		AUDIO_SYS_ERROR("Audio::BlPagedArray::IsValid(0x%x) - u16EntryNum (%d) >= m_u16ArraySize (%d)\n", hNode, u16EntryNum, m_u16ArraySize);
		#endif
		return false;
	}

	if (m_pFreeNodeBitArray[u16EntryNum / 32] & (1 << (u16EntryNum % 32)))	// bit set to 1 indicates element is free
	{
		#if !defined CFG_GOLD && !defined PLATFORM_SPU
		AUDIO_SYS_ERROR("Audio::BlPagedArray::IsValid(0x%x) - free node array bit is not set. u16EntryNum = %d\n", hNode, u16EntryNum);
		#endif
		return false;
	}
	else
	{
		bool bPidMatches = (m_pContainers[u16EntryNum].m_u16Pid == ExtractPid(hNode));
		#if !defined CFG_GOLD && !defined PLATFORM_SPU
		if (!bPidMatches)
			AUDIO_SYS_ERROR("Audio::BlPagedArray::IsValid(0x%x) - Pid extracted from NODE_HANDLE (0x%x) does not match Pid from container (0x%x). Entry num = %d. Array size =%d\n", hNode, ExtractPid(hNode), m_pContainers[u16EntryNum].m_u16Pid, u16EntryNum, m_u16ArraySize);
		#endif
		return bPidMatches;
	}
}

//template<class T, int N, int P>
//void Audio::BlPagedArray<T, N, P>::Clear()
//{
//	Assert (m_u16ArraySize - m_u16NumFreeNodes == 0); 
//	if (m_pContainers) 
//		delete [] m_pContainers;
//}


template<class T, int N, int P>
void Audio::BlPagedArray<T, N, P>::SetContents(U32 u16EntryNum, T Contents)
{
	U32 u32BitToSet = (u16EntryNum % 32);
	m_pFreeNodeBitArray[u16EntryNum / 32] &= ~(1 << u32BitToSet);

	m_pContainers[u16EntryNum].SetContents(Contents);
}


template<class T, int N, int P>
Audio::NODE_HANDLE Audio::BlPagedArray<T, N, P>::CreateHandle(U32 u16Pid, U32 u16EntryNum)
{
	#ifndef CFG_GOLD
	Assert(!PidExceedsMaxBits(u16Pid) && !EntryNumExceedsMaxBits(u16EntryNum));
	#endif

	NodeHandleInternal hInt(u16Pid, u16EntryNum);
	return hInt.u32Handle;
}


#endif	//	BIT_LIMITED_PAGED_ARRAY_H

