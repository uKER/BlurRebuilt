/*
 * Block Allocator
 *
 * Allocates same-sized memory blocks in 64K pages. We have a predefined amount of areas,
 * each with their own block size. Each area can allocate/free 64K pages as needed.
 *
 */

#include <SystemPch.h>
#include <system/memory/BlockAllocator.h>
#include <system/memory/VirtualMemory.h>

#if !defined(WIN64) // [Blur2PC][x64] Core.h defines WIN64 empty.
#define PAGE_MASK 0xffff0000
#define PTR_TO_INT(X) ((U32)((U64)(X)))
#define INT U32
#else
#define PAGE_MASK 0xffffffffffff0000
#define PTR_TO_INT(X) ((U64)(X))
#define INT U64
#endif

namespace Memory
{

static Debug::Logger &g_LogBa = Debug::GetLogger("debug.blockallocator");

bool BlockAllocator::s_bInitialized = false;
//PageUsage BlockAllocator::m_PageUsage;

AllocFunctionType *gpAllocFunction = NULL;
FreeFunctionType *gpFreeFunction = NULL;

/*
 * Page Usage
 */

/// initialize the page usage hash table
void PageUsage::Initialize()
{
	memset(&m_PageMemoryUsage, 0, sizeof(m_PageMemoryUsage));
}

#if defined(WIN64) // [Blur2PC][x64] Two-level table; see PageUsage::m_PageMemoryUsage.
void PageUsage::SetBit(void *pAddr)
{
	const U64 a = PTR_TO_INT(pAddr);
	BIZ_ASSERT((a >> 32) < enum_PageUsageLeaves);
	U8 *&pLeaf = m_PageMemoryUsage[a >> 32];
	if (pLeaf == NULL)
		pLeaf = (U8*)::VirtualAlloc(NULL, 0x10000/8, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);	// zero-filled
	pLeaf[(a & 0xffffffff) >> 19] |= 1 << ((a >> 16) & 7);
}

void PageUsage::ResetBit(void *pAddr)
{
	const U64 a = PTR_TO_INT(pAddr);
	U8 *pLeaf = (a >> 32) < enum_PageUsageLeaves ? m_PageMemoryUsage[a >> 32] : NULL;
	if (pLeaf != NULL)
		pLeaf[(a & 0xffffffff) >> 19] &= ~(1 << ((a >> 16) & 7));
}

bool PageUsage::GetBit(void *pAddr) const
{
	const U64 a = PTR_TO_INT(pAddr);
	const U8 *pLeaf = (a >> 32) < enum_PageUsageLeaves ? m_PageMemoryUsage[a >> 32] : NULL;
	return pLeaf != NULL && (pLeaf[(a & 0xffffffff) >> 19] & (1 << ((a >> 16) & 7))) != 0;
}
#else
void PageUsage::SetBit(void *pAddr)
{
	INT idx = PTR_TO_INT(pAddr)>>19;
	INT bit = 1 << ((PTR_TO_INT(pAddr)>>16) & 7);
	m_PageMemoryUsage[idx] |= bit;
}

void PageUsage::ResetBit(void *pAddr)
{
	INT idx = PTR_TO_INT(pAddr)>>19;
	INT bit = 1 << ((PTR_TO_INT(pAddr)>>16) & 7);
	m_PageMemoryUsage[idx] &= ~bit;
}

bool PageUsage::GetBit(void *pAddr) const
{
	INT idx = PTR_TO_INT(pAddr)>>19;
	INT bit = 1 << ((PTR_TO_INT(pAddr)>>16) & 7);
	return (m_PageMemoryUsage[idx] & bit)!=0;
}
#endif

/*
 * 64K memory page
 */

/// retrieve a page pointer given a block memory address
Page *GetPage(void *p)
{
	//return (Page*)(U64)((PTR_TO_INT(p) & 0xffff0000) + 0x10000 - sizeof(Page));
	return (Page*)(INT)(U64)((PTR_TO_INT(p) & PAGE_MASK) + 0x10000 - sizeof(Page)); // JASOND: Intel compiler: "warning #1195: conversion from integer to smaller pointer"
}

/// 64K memory page code
void Page::Add(Area *pArea)
{
	nextPage = pArea->m_PageList;
	prevPage = 0;

	if (nextPage)
		nextPage->prevPage = this;

	pArea->m_PageList = this;
}

void Page::Remove(Area *pArea)
{
	if (prevPage)
		prevPage->nextPage = nextPage;
	if (nextPage)
		nextPage->prevPage = prevPage;
	if (pArea->m_PageList == this)
		pArea->m_PageList = nextPage;
}

void Page::RemoveHead(Area *pArea)
{
	pArea->m_PageList = nextPage;
	if (nextPage)
		nextPage->prevPage = 0;
}

/*
 * Area
 */
Page *Area::FindPageWithFreeBlocks(PageUsage& pageUsage)
{
	// get a page from the empty list
	Page *pPage = m_PageList;
	if (pPage)
		return pPage;

	// alloc a new page
	void *pNew = AllocPage(pageUsage);
	if (!pNew)
		return NULL;

	// get the page footer class from the page base address
	pPage = GetPage(pNew);

	// add the new page to this area
	pPage->Add( this );

	return pPage;
}

void *Area::AllocateBlock(PageUsage& pageUsage)
{
	Page *pPage = FindPageWithFreeBlocks(pageUsage);
	if (!pPage)
		return NULL;

	// Remove the block memory from the Page
	Block *pBlock = pPage->firstBlock;
	

	// assert that the free block is inside the page it came from
	// if this asserts it indicates heap corruption
	assert((((INT)(U64)pBlock)& PAGE_MASK) == (((INT)(U64)pPage)&PAGE_MASK));
	assert(!pBlock->nextBlock || (((INT)(U64)pBlock->nextBlock)&PAGE_MASK) == (((INT)(U64)pPage)&PAGE_MASK));

	Block *pNextBlock = pBlock->nextBlock;
	pPage->firstBlock = pNextBlock;
	pPage->nBlocks--;

#ifndef CFG_SPEED
	{
		Block *pBlock = pPage->firstBlock;
		U32 nFreeBlocks = 0;
		while (pBlock)
		{
			pBlock = pBlock->nextBlock;
			nFreeBlocks++;
		}
		assert(nFreeBlocks == pPage->nBlocks);	// check that the number of free blocks is correct
	}	
#endif

	// if the page is comletely full, remove it from this area's empty list
	if (pPage->nBlocks == 0)
	{
		pPage->RemoveHead( this );
		m_nFullPages++;
	}
	else
	{
		DCBT(0, pNextBlock);
	}

	return pBlock;
}

void Area::FreeBlock(void *pMemory, Page *pPage, PageUsage& pageUsage)
{
#ifndef CFG_SPEED
	{
		Block *pBlock = pPage->firstBlock;
		U32 nFreeBlocks = 0;
		while (pBlock)
		{
			assert(pMemory != pBlock);			// check whether this block is already in the free list
			pBlock = pBlock->nextBlock;
			nFreeBlocks++;
		}
		assert(nFreeBlocks == pPage->nBlocks);	// check that the number of free blocks is correct
	}	
#endif

	// check whether this page is now empty
	U32 numTotalBlocks = (enum_PageSize - sizeof(Page)) / m_iBlockSize;
	assert(pPage->nBlocks < numTotalBlocks);
 	if (pPage->nBlocks+1 == numTotalBlocks)
	{
		// free the page
		pPage->Remove(this);

		//void *pPageMemory = (void*)(U64)(PTR_TO_INT(pMemory) & 0xffff0000);
		void *pPageMemory = (void*)(INT)(U64)(PTR_TO_INT(pMemory) & PAGE_MASK); // JASOND: Intel compiler: "warning #1195: conversion from integer to smaller pointer"

		FreePage(pPageMemory, pageUsage);
		return;
	}

	// add free block back into linked list
	Block *pBlock = (Block*)(pMemory);
	pBlock->nextBlock = pPage->firstBlock;

	pPage->firstBlock = pBlock;
	pPage->nBlocks += 1;

	if (pPage->nBlocks == 1)
	{
		pPage->Add(this);
		m_nFullPages--;
	}
}

Page *Area::InitializePage(void *pMemory, U32 blockSize )
{
	Page * p = (Page*)((U8*)pMemory + enum_PageSize - sizeof(Page));

	U32 nBlocks = (enum_PageSize - sizeof(Page)) / blockSize;

	p->nBlocks  = nBlocks;
	p->nextPage  = NULL;
	p->prevPage  = NULL;
	p->firstBlock = (Block*)(pMemory);
	p->pArea  = this;

	// Initialize the available block list
	Block * prevBlock = (Block*)(pMemory);
	for (U32 blockIndex = 1; blockIndex < nBlocks; blockIndex++)
	{
		Block * currentBlock = (Block*)((U8*)pMemory + blockSize * blockIndex);

		prevBlock->nextBlock = currentBlock;
		prevBlock = currentBlock;
	}

	prevBlock->nextBlock = 0;

	return p;
}

void *Area::AllocPage(PageUsage& pageUsage)
{
	if (gpAllocFunction)
		return (*gpAllocFunction)();

	void *pVirtualPageMem = AllocVirtual(0x10000);
	if (!pVirtualPageMem)
		return NULL;
	void *pPhysicalPageMem = AllocPhysical(pVirtualPageMem, 0x10000);
	if (!pPhysicalPageMem)
	{
		FreeVirtual(pVirtualPageMem);
		return NULL;
	}
	BIZ_ASSERT(pPhysicalPageMem == pVirtualPageMem);

	InitializePage(pPhysicalPageMem, m_iBlockSize);

	pageUsage.SetBit(pPhysicalPageMem);

	m_nPages++;

	return pPhysicalPageMem;
}

bool Area::FreePage(void *pPageMem, PageUsage& pageUsage)
{
	if (gpFreeFunction)
	{
		(*gpFreeFunction)(pPageMem);
		return true;
	}

	pageUsage.ResetBit(pPageMem);

	FreePhysical(pPageMem);
	FreeVirtual(pPageMem);

	m_nPages--;

	return true;
}

/*
 * Block Allocator
 */
BlockAllocator::BlockAllocator(int *pAreaSizes, AllocFunctionType *pAllocFunc, FreeFunctionType *pFreeFunc)
{
	m_bInitialized = false;
	gpAllocFunction = pAllocFunc;
	gpFreeFunction = pFreeFunc;
	Initialise(pAreaSizes);
}

BlockAllocator::BlockAllocator(int *pAreaSizes, bool useMutex)
{
	m_bInitialized = false;
	Initialise(pAreaSizes, useMutex);
}

BlockAllocator::BlockAllocator()
{
	assert(s_bInitialized == true);
	m_bInitialized = s_bInitialized;
}

void BlockAllocator::PreStatic_Initialise(int *pAreaSizes)
{
	Initialise(pAreaSizes);
	s_bInitialized = true;
}

void BlockAllocator::Initialise(int *pAreaSizes, bool useMutex)
{
	assert(m_bInitialized == false);

	if (useMutex)
		m_pMutex = new MT::Lock;
	else
		m_pMutex = 0;

	if (m_pMutex)
		m_pMutex->Enter();

	// count areas
	int nAreas = 0;
	while (pAreaSizes[nAreas] && (pAreaSizes[nAreas] <= enum_LargestBlockSize))
		nAreas++;

	m_pArea = new Area[nAreas];

	// create the areas
	for (int i=0; i<nAreas; i++)
	{
		int c = pAreaSizes[i];

		// area must be aligned to 4 bytes
		assert( (c % 4) == 0 );
		assert( c <= enum_LargestBlockSize );
		// areas must be added in order of smallest to biggest
		assert( i == 0 || m_pArea[i-1].m_iBlockSize < c );

		m_pArea[i].m_iBlockSize = c;
		m_pArea[i].m_PageList = NULL;
		m_pArea[i].m_nFullPages = 0;
		m_pArea[i].m_nPages = 0;
	}
	m_nAreas = nAreas;

	InitSizeTable();
	m_PageUsage.Initialize();

	m_bInitialized = true;

	if (m_pMutex)
		m_pMutex->Exit();
}

BlockAllocator::~BlockAllocator()
{
	m_bInitialized = false;
	delete m_pMutex;
	delete [] m_pArea;
}

void BlockAllocator::InitSizeTable()
{
	int iArea = 0;

	for (int i=0; i<enum_LargestBlockSize/4; i++)
	{
		int iSize = (i+1)*4;

		for (; iArea<m_nAreas; iArea++)
		{
			if (iSize <= m_pArea[iArea].m_iBlockSize)
				break;
		}

		m_iSizeToArea[i] = iArea;
	}

}

bool BlockAllocator::MemoryOwnedByAllocator(void *p) const
{
	return m_PageUsage.GetBit(p);
}

/// lookup which area to use for a given size
U32 BlockAllocator::SizeToAreaIndex(U32 size) const
{
//	return m_iSizeToArea[((size+3)/4)-1];
	return m_iSizeToArea[(size-1)>>2];
}

//extern void AssertOutOfMemory(void);

void *BlockAllocator::Allocate(U32 blockSize)
{
	// Check to see if the allocation can be handled by the block allocator
	if (blockSize == 0 || blockSize > enum_LargestBlockSize)
		return NULL;

	if (m_pMutex)
		m_pMutex->Enter();

	// Calculate which small-block area this allocation should go to
	U32 areaIndex = SizeToAreaIndex(blockSize);
	// [Blur2PC][x64] A size above the largest area maps one past the area array.
	BIZ_ASSERT_MSG(areaIndex < (U32)m_nAreas, "block size %u exceeds the largest area", blockSize);
	if (areaIndex >= (U32)m_nAreas)
	{
		if (m_pMutex)
			m_pMutex->Exit();
		return NULL;
	}
	void* memory = m_pArea[areaIndex].AllocateBlock(m_PageUsage);
	if (!memory)
	{
		if (m_pMutex)
			m_pMutex->Exit();

		//AssertOutOfMemory();

		return NULL;
	}

	// if this asserts it indicates heap corruption
	assert(MemoryOwnedByAllocator(memory));

	if (m_pMutex)
		m_pMutex->Exit();

#ifdef BUILD_CONFIG_DEBUG
	memset(memory, 0xcd, m_pArea[areaIndex].m_iBlockSize);
#endif

	return memory;
}

/// free a block of allocated memory
bool BlockAllocator::Free(void *pMemory)
{
	if (!m_bInitialized)
		return true;	// NOTE: we couldn't free this memory, because the block allocator has been freed itself. pretend everything was ok

	if (m_pMutex)
		m_pMutex->Enter();

	// check that the memory came from us
	if (!m_PageUsage.GetBit(pMemory))
	{
		if (m_pMutex)
			m_pMutex->Exit();

		return false;
	}

	Page *pPage = GetPage(pMemory);
	Area *pArea = pPage->pArea;

#ifdef BUILD_CONFIG_DEBUG
	memset(pMemory, 0xfe, pArea->m_iBlockSize);
#endif

	pArea->FreeBlock(pMemory, pPage, m_PageUsage);

	if (m_pMutex)
		m_pMutex->Exit();

	return true;
}

/// returns the size of the allocation at the specified address
int BlockAllocator::AlocationSize(void *pMemory) const
{
	if (!m_bInitialized)
		return 0;

	Page *pPage = GetPage(pMemory);
	return pPage->pArea->m_iBlockSize;
}

/// constructor
BlockAllocator::Stats::Stats(U32 nAreas)
{
	m_pStats = new StatsItem[nAreas];
}

/// destructor
BlockAllocator::Stats::~Stats()
{
	delete [] m_pStats;
}

/// return a structure containing memory allocation stats
BlockAllocator::Stats *BlockAllocator::GetStats() const
{
	if (m_pMutex)
		m_pMutex->Enter();

	BlockAllocator::Stats *pStats = new BlockAllocator::Stats(m_nAreas);

	pStats->m_iTotalMem = 0;
	pStats->m_iUsedMem = 0;
	pStats->m_nItems = 0;

	for (int i=0; i<m_nAreas; i++)
	{
		StatsItem *pItem = &pStats->m_pStats[i];

		pItem->iAllocSize = m_pArea[i].m_iBlockSize;

		// count pages and allocations
		int nPages = 0;
		int nAllocs = 0;
		int nAllocsPerPage = ((enum_PageSize - sizeof(Page)) / m_pArea[i].m_iBlockSize);
		for (Page *pPage = m_pArea[i].m_PageList; pPage!=NULL; pPage=pPage->nextPage)
		{
			nPages++;
			nAllocs += nAllocsPerPage - pPage->nBlocks;
		}
		nAllocs += m_pArea[i].m_nFullPages * nAllocsPerPage;
		nPages += m_pArea[i].m_nFullPages;
		assert(nPages == m_pArea[i].m_nPages);

		pItem->nAllocs = nAllocs;
		pItem->nPages = nPages;
		pItem->iBytesTotal = nPages * 256 * 256;
		pItem->iBytesUsed = nAllocs * m_pArea[i].m_iBlockSize;

		pStats->m_iTotalMem += pItem->iBytesTotal;
		pStats->m_iUsedMem += pItem->iBytesUsed;
		pStats->m_nAllocs += pItem->nAllocs;
		pStats->m_nItems++;
	}

	if (m_pMutex)
		m_pMutex->Exit();

	return pStats;
}

/*
Here is what the stats mean (taking the 32-byte page as an example)

Allocs is the number of allocations. This doesn’t take into account the fact that you could have a
24 byte allocation which ends up in the 32-byte page. This would then waste 8 bytes, but it doesn’t
take that into account in these stats.

BytesUsed is simply the number of allocations times the size of the page e.g.  50389 * 32 = 1612448

BytesTotal is the amount of physical memory allocated to 32-byte pages. Each page is 64K, so in this
case 1703936/65536 = 26 pages have been allocated.

Wastage is the amount of physical memory that has been allocated, but that doesn’t have any allocations
in yet. In other words, as soon as you do a single allocation the block allocator will map a 64K page
and put a single 32-byte allocation in there. At this point your wasted memory would be approximately
65536-32 bytes. There is a small extra overhead for storing some information per page, so if you had
exactly 2048 32-byte allocations that wouldn’t quite fit.
*/

void BlockAllocator::DumpStats()
{
	if (m_pMutex)
		m_pMutex->Enter();

	Memory::BlockAllocator::Stats* stats = GetStats();

	LOGINFO(g_LogBa, "\nSmall Block Allocator:");
	LOGINFO(g_LogBa, "----------------------");
	LOGINFO(g_LogBa, "NodeSize   Allocs  BytesUsed  BytesTotal   Waste");

	for (unsigned int i=0; i < stats->m_nItems; i++)
	{
		LOGINFO(g_LogBa, "%8d  %7d  %9d   %9d  %6d",
				stats->m_pStats[i].iAllocSize,
				stats->m_pStats[i].nAllocs,
				stats->m_pStats[i].iBytesUsed,
				stats->m_pStats[i].iBytesTotal,
				stats->m_pStats[i].iBytesTotal - stats->m_pStats[i].iBytesUsed);
	}

	LOGINFO(g_LogBa, "                   ---------   ---------  ------");

	LOGINFO(g_LogBa, "           Totals  %9d   %9d  %6d",
				stats->m_iUsedMem,
				stats->m_iTotalMem,
				stats->m_iTotalMem - stats->m_iUsedMem);

	if (m_pMutex)
		m_pMutex->Exit();

	delete stats;
}

}
