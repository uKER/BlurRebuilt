#ifndef __SYSTEM_SMALLBLOCKALLOCATOR_H__
#define __SYSTEM_SMALLBLOCKALLOCATOR_H__

namespace MT
{
class Mutex;
}

namespace Memory
{

typedef void* AllocFunctionType();
typedef void  FreeFunctionType(void *pData);

class Page;
class Area;
class BlockAllocator;

enum
{
	enum_PageSize			= 64 * 1024,	// Each page is 64K
	enum_LargestBlockSize	= 256,          // Defines the size of m_iSizeToArea table
};

struct Block
{
	Block* nextBlock;
};

/// A 64K page of memory, which is part of an area
class Page
{
	friend class Area;
	friend class BlockAllocator;

protected:
				// add a page to an area's free list
	void		Add(Area *pArea );
				// remove a page from an area's free list
	void		Remove(Area  *pArea );
				// remove the top page from a area's free list
	void		RemoveHead(Area *pArea );

	int			Dummy[4]; // some space at the start of the 64K page for Jason's memory header to go

	// linked list of pages for each area
	Page		*prevPage;
	Page		*nextPage;
				
	Block		*firstBlock;	// pointer to first block of this page
	U32         nBlocks;		// number of blocks
	Area		*pArea;			// pointer to the area that this page is in
};

/// hash table to determine whether a page of memory is used by the block allocator
class PageUsage
{
public:
					// initialize the table
	void			Initialize();

					// check whether a particular memory address is allocated by the block allocator
	bool			GetBit(void *pAddr) const;
					// set a particular page's flag
	void			SetBit(void *pAddr);
					// clear a particular page's flag
	void			ResetBit(void *pAddr);

private:
#if defined(WIN64) // [Blur2PC][x64] One 8 KB bitmap per 4 GB of the 47-bit user address space, made on first use.
	enum { enum_PageUsageLeaves = 1 << 15 };
	U8*				m_PageMemoryUsage[enum_PageUsageLeaves];
#else
	U8				m_PageMemoryUsage[0x10000/8]; // 4 Gb
#endif
};

/// An Area is a collection of pages that have the same block allocation size
class Area
{
public:
					// allocate a block from this area
	void			*AllocateBlock(PageUsage& pageUsage);
					// free the specified block
	void			FreeBlock(void *pMemory, Page *pPage, PageUsage& pageUsage);
					// get the first page that is not completely full
	Page			*FindPageWithFreeBlocks(PageUsage& pageUsage);
					// allocate a new page
	void			*AllocPage(PageUsage& pageUsage);
					// free a page
	bool			FreePage( void* PageMemory, PageUsage& pageUsage);
					// initialize a newly allocated page
	Page			*InitializePage( void* PageMemory, U32 blockSize );

	int				m_iBlockSize;
	Page			*m_PageList;
	int				m_nFullPages;
	int				m_nPages;
};

/// The Block Allocator
class BlockAllocator
{
	friend class Area;

public:
	// memory stats structure
	struct StatsItem
	{
		U32	iAllocSize;
		U32 nPages;
		U32 nAllocs;
		U32 iBytesUsed;
		U32 iBytesTotal;
	};

	class Stats
	{
	public:
		Stats(U32 nAreas);
		~Stats();

		StatsItem	*m_pStats;
		U32			m_nItems;

		U32			m_iTotalMem;
		U32			m_iUsedMem;	
		U32			m_nAllocs;
	};

public:
						/// constructor
						BlockAllocator();
						/// pass in the list of block sizes
						/// block sizes must be 4-byte aligned and in ascending order
						BlockAllocator(int *pAreaSizes, bool useMutex = true);
						/// as above, but specifying custom allocator functions for allocating the 64K aligned memory blocks
						BlockAllocator(int *pAreaSizes, AllocFunctionType *pAllocFunc, FreeFunctionType *pFreeFunc);
						/// destructor
						~BlockAllocator();
						/// returns true if the block allocator has been fully initialized.
	bool				Initialized() { return m_bInitialized; }

						/// allocate memory
						/// returns NULL if the block size is too big to be allocated by the block allocator
	void				*Allocate( U32 blockSize );
						/// free memory
						/// returns false if the memory was not allocated by the block allocator
	bool				Free(void *p);
						/// check whether a particular memory addres was allocated by the block allocator
	bool				MemoryOwnedByAllocator( void *p ) const;
						/// returns the size of the allocation at the specified address
						/// assumes the memory was allocated using the block allocator!
	int					AlocationSize( void *p ) const;
						/// retrieve a structure containing allocation statistics
	Stats				*GetStats() const;

	void				PreStatic_Initialise(int *pAreaSizes);

	void				DumpStats();

protected:
	void				Initialise(int *pAreaSizes, bool useMutex = true);
	void				InitSizeTable();
	U32					SizeToAreaIndex( U32 size ) const;

	PageUsage			m_PageUsage;
	MT::Lock			*m_pMutex;
	Area				*m_pArea;
	int					m_nAreas;
	U8					m_iSizeToArea[enum_LargestBlockSize / 4];
	bool				m_bInitialized;
	static bool			s_bInitialized;
};

}

#endif
