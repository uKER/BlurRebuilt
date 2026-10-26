#include "stdafx.h"
#include "Display/Memory/MemoryManager.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Display
{

C_MemoryBuffer::C_MemoryBuffer() :
	m_Allocs(0),
	m_Used(0),
	m_Free(0)
{
}

C_MemoryBuffer::~C_MemoryBuffer()
{
}

bool C_MemoryBuffer::Create(const U32 size, const U32 align, const U32 flags)
{
	m_Mutex.Wait();
	
	m_Size		= size;
	m_Align		= align;
	m_Flags		= flags;

	if(CreateAddress())
	{
		m_Used = 0;
		m_Free = m_Size;

		m_FreeList.Insert(C_MemoryMap::DataType(m_pAddress, m_Size));
		
		m_Mutex.Release();
		
		return true;
	}
	else
	{
		LOGFATAL(g_Log, "Failed to create the memory buffer of size %i bytes\n", m_Size);
		
		m_Mutex.Release();
		
		return false;
	}
}

void C_MemoryBuffer::Destroy()
{
	m_Mutex.Wait();

	DestroyAddress();

	m_Mutex.Release();
}

bool C_MemoryBuffer::AllocateFromStart(const U32 align, const U32 size, U8** ppAddress)
{
	m_Mutex.Wait();

	for(C_MemoryMap::Iterator it = m_FreeList.Begin(); it != m_FreeList.End(); ++it)
	{
		if(AllocateFromStartOfBlock(it, align, size, ppAddress))
		{
			m_Allocs++;

			m_Mutex.Release();

			return true;
		}
	}

	m_Mutex.Release();

	return false;
}

bool C_MemoryBuffer::AllocateFromEnd(const U32 align, const U32 size, U8** ppAddress)
{
	m_Mutex.Wait();

	for(C_MemoryMap::ReverseIterator it = m_FreeList.ReverseBegin(); it != m_FreeList.ReverseEnd(); ++it)
	{
		if(AllocateFromEndOfBlock(it, align, size, ppAddress))
		{
			m_Allocs++;

			m_Mutex.Release();

			return true;
		}
	}

	m_Mutex.Release();
	
	return false;
}

void C_MemoryBuffer::Free(const U32 size, U8* pAddress)
{
	m_Mutex.Wait();
	
	assert((U32)(pAddress)			>= (U32)(m_pAddress) && (U32)(pAddress)			<= ((U32)(m_pAddress+m_Size)));
	assert((U32)(pAddress+size) 	>= (U32)(m_pAddress) && (U32)(pAddress+size)	<= ((U32)(m_pAddress+m_Size)));

	//assert(!AlreadyInFreeList(size, pAddress));
	
	C_MemoryMap::Iterator result = InsertIntoFreeList(size, pAddress);
	
	MergeWithNextBlock(result);
	MergeWithPrevBlock(result);
	
	m_Allocs--;
	//assert(m_Allocs >= 0);

	m_Used -= size;
	m_Free += size;
	assert(m_Used + m_Free == m_Size);

	m_Mutex.Release();
}

C_MemoryMap::Iterator C_MemoryBuffer::InsertIntoFreeList(const U32 size, U8* pAddress)
{
	assert((U32)(pAddress)			>= (U32)(m_pAddress) && (U32)(pAddress)			<= ((U32)(m_pAddress+m_Size)));
	assert((U32)(pAddress+size) 	>= (U32)(m_pAddress) && (U32)(pAddress+size)	<= ((U32)(m_pAddress+m_Size)));

	//assert(!AlreadyInFreeList(size, pAddress));

	Ut::Pair<C_MemoryMap::Iterator, bool> result = m_FreeList.Insert(C_MemoryMap::DataType(pAddress, size));
	assert(result.second && "C_MemoryBuffer::InsertIntoFreeList - Block already in free list.");
	return result.first;
}

void C_MemoryBuffer::InsertIntoFreeList(C_MemoryMap::Iterator it, const U32 size, U8* pAddress)
{
	assert((U32)(pAddress)			>= (U32)(m_pAddress) && (U32)(pAddress)			<= ((U32)(m_pAddress+m_Size)));
	assert((U32)(pAddress+size) 	>= (U32)(m_pAddress) && (U32)(pAddress+size)	<= ((U32)(m_pAddress+m_Size)));

	//assert(!AlreadyInFreeList(size, pAddress));

	m_FreeList.Insert(it, C_MemoryMap::DataType(pAddress, size));
}

void C_MemoryBuffer::MergeWithPrevBlock(C_MemoryMap::Iterator it)
{
	C_MemoryMap::Iterator it1 = it;
	if(it1 != m_FreeList.Begin())
	{
		--it1;
		if(it1->GetKey() + it1->GetValue() == it->GetKey())
		{
			it1->GetValue() += it->GetValue();
			m_FreeList.Erase(it);
		}
	}
}

void C_MemoryBuffer::MergeWithNextBlock(C_MemoryMap::Iterator it)
{
	C_MemoryMap::Iterator it1 = it;
	++it1;
	if(it1 != m_FreeList.End())
	{
		if(it->GetKey() + it->GetValue() == it1->GetKey())
		{
			it->GetValue() += it1->GetValue();
			m_FreeList.Erase(it1);
		}
	}
}

bool C_MemoryBuffer::AllocateFromStartOfBlock(C_MemoryMap::Iterator &it, const U32 align, const U32 size, U8** ppAddress)
{
	// This block of memory is free.
	U32 blockHead = (U32)it->GetKey();
	U32 blockSize = it->GetValue();
	U32 blockTail = blockHead + blockSize;
	
	// Align our allocation assuming we'll be using this free block of memory.
	U32 alignHead = (blockHead + align - 1) & ~(align - 1);
	U32 wasteSize = alignHead - blockHead;
	U32 alignTail = alignHead + wasteSize;
	
	// Can we use it?
	if(blockSize >= size + wasteSize)
	{
		*ppAddress =  (U8*)alignHead;
		
		m_FreeList.Erase(it);

		// This is wasted memory cos we had to align the address.
		if(wasteSize > 0)
		{
			U32 newSize = wasteSize;
			U32 newOffset = blockHead;
			InsertIntoFreeList(newSize, (U8*)newOffset);
		}

		// We haven't used the entire block so add what's left over to the free list.
		if(blockSize > size + wasteSize)
		{
			U32 newSize = blockSize - size - wasteSize;
			U32 newOffset = alignHead + size;
			InsertIntoFreeList(newSize, (U8*)newOffset);
		}

		m_Used += size;
		m_Free -= size;
		assert(m_Used + m_Free == m_Size);

		return true;
	}
	else
	{
		return false;
	}
}

bool C_MemoryBuffer::AllocateFromEndOfBlock(C_MemoryMap::ReverseIterator &it, const U32 align, const U32 size, U8** ppAddress)
{
	// This block of memory is free.
	U32 blockHead = (U32)it->GetKey();
	U32 blockSize = it->GetValue();
	U32 blockTail = blockHead + blockSize;
	
	// Align our allocation assuming we'll be using this free block of memory.
	#if 1
	U32 unalignHead = blockTail-size;
	U32 alignHead = (unalignHead + align - 1) & ~(align - 1);
	U32 wasteSize = alignHead - unalignHead;
	if(wasteSize > 0)
	{
		wasteSize = align-wasteSize;
		alignHead = unalignHead-wasteSize;
	}
	U32 alignTail = alignHead + size;
	#else
	U8* alignHead = (U8*)((((U32)blockTail - size) - align - 1) & ~(align - 1));
	const U32 wasteSize = (blockTail - size) - alignHead;
	U8* alignTail = alignHead + wasteSize;
	#endif
	
	// Can we use it?
	if(blockSize >= size + wasteSize)
	{
		*ppAddress = (U8*)alignHead;
		
		m_FreeList.ReverseErase(it);

		// This is wasted memory cos we had to align the address.
		if(wasteSize > 0)
		{
			U32 newSize = wasteSize;
			U32 newOffset = blockTail - wasteSize;
			InsertIntoFreeList(newSize, (U8*)newOffset);
		}

		// We haven't used the entire block so add what's left over to the free list.
		if(blockSize > size + wasteSize)
		{
			U32 newSize = blockSize - size - wasteSize;
			U32 newOffset = blockHead;
			InsertIntoFreeList(newSize, (U8*)newOffset);
		}

		m_Used += size;
		m_Free -= size;
		assert(m_Used + m_Free == m_Size);

		return true;
	}
	else
	{
		return false;
	}
}

void C_MemoryBuffer::PrintDebug(Debug::Logger& log) const
{
	LOGINFO(log, "GraphicsMemoryBuffer : Allocs '%i' Used '%i' Free '%i' FreeBlocks '%i'\n", m_Allocs, m_Used, m_Free, m_FreeList.Size());
}

bool C_MemoryBuffer::AlreadyInFreeList(const U32 size, U8* pAddress)
{
	for(C_MemoryMap::Iterator it = m_FreeList.Begin(); it != m_FreeList.End(); ++it)
	{
		const U32 blockSize = it->GetValue();
		U8* pBlockAddress = it->GetKey();

		if(pAddress < pBlockAddress)
		{
			if((pAddress+size) > pBlockAddress)
			{
				return true;
			}
		}
		else
		if(pAddress > pBlockAddress)
		{
			if(pAddress < (pBlockAddress+blockSize))
			{
				return true;
			}
		}
	}

	return false;
}

void C_MemoryBuffer::GetFreeList(Btl::Vector<MemoryBlock>& blocks) const
{
	blocks.Clear();

	for(C_MemoryMap::ConstIterator it = m_FreeList.Begin(); it != m_FreeList.End(); ++it)
	{
		blocks.PushBack();
		blocks.Back().address = it->GetKey();
		blocks.Back().size = it->GetValue();
		blocks.Back().quick = false;
	}
}

}; // namespace Display
