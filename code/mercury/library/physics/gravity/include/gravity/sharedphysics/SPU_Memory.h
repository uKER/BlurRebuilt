#ifndef SPU_MEMORY_H
#define SPU_MEMORY_H

//#include <core/threads.h>
//#include <system/scheduler/jobqueue_mem.h>
#include <core/threads/atomicMutex.h>
#include <system/debuglog.h>

namespace SharedPhysics
{
	const U32 CachePageSize = 2048;
	const U32 NumDummyBlocks = 32;
	class C_StaticSPUMemoryAllocator;
	class C_SPUMemoryPool;

	class ALIGN16 C_SPUMemoryBlock
	{
	public:
		U8 *m_pData;
		U32 m_CurrentIndex;
		C_SPUMemoryBlock* m_pNext;
		//PPU ptr is the memory address for this thing on the PPU!
		C_SPUMemoryBlock* m_pPPUPtr;
		const U32 BlockSize;


#ifndef PLATFORM_SPU

		C_SPUMemoryBlock() : m_pData(NULL), BlockSize(0)
		{
			m_pPPUPtr = this;
		}

		C_SPUMemoryBlock(const U32 BlockSize) : m_CurrentIndex(0), m_pNext(NULL),  BlockSize(BlockSize)
		{
#ifndef PLATFORM_SPU
			m_pData = (U8*)BizAlignedMalloc(BlockSize, 16);
#else
			m_pData = (U8*)BizAlignedMalloc(BlockSize, 16); //Allocate 1k alignment cache page to write back to - avoid buffer crashes
#endif
			m_pPPUPtr = this;
		}

		~C_SPUMemoryBlock()
		{
			if(m_pData)
				BizAlignedFree(m_pData);
			m_pData = NULL;
		}
#endif

		void Reset()
		{
			m_CurrentIndex = 0;
		}

		void SetCurrentIndex(U32 currentIndex)
		{
			m_CurrentIndex = currentIndex;
		}

		void* Alloc(const U32 _size);

#ifndef PLATFORM_SPU
	private:
		C_SPUMemoryBlock(const C_SPUMemoryBlock& other);
		C_SPUMemoryBlock& operator = (const C_SPUMemoryBlock& other);
#endif
	};

	class C_SPUMemoryMarker
	{
		C_SPUMemoryPool* m_pPool;
		C_SPUMemoryBlock* m_pMarker;
		U32 m_CurrentIndex;

	public:
		C_SPUMemoryMarker(C_SPUMemoryPool* pPool);

		~C_SPUMemoryMarker()
		{
			Restore();
		}

		void Restore();
	};


	template <typename Thing>
	struct C_SPUPtr
	{
		Thing* m_pSPUPtr;
		Thing* m_pPPUPtr;

		C_SPUPtr(Thing* spuPtr, Thing* ppuPtr) : m_pSPUPtr(spuPtr), m_pPPUPtr(ppuPtr)
		{
		}

		void Write()
		{
#ifdef PLATFORM_SPU
			Spu::Store(m_pPPUPtr, m_pSPUPtr, sizeof(Thing));
#endif
		}

		Thing* PPUPtr()
		{
			return m_pPPUPtr;
		}

		Thing* SPUPtr()
		{
			return m_pSPUPtr;
		}
	};

	//Stores a per-spu-block memory pool!
	class C_SPUMemoryPool
	{
	public:
		C_SPUMemoryBlock* m_pFirstBlock;
		C_SPUMemoryBlock* m_pCurrentBlock;
		C_StaticSPUMemoryAllocator* m_pPool;
#ifdef PLATFORM_SPU
		C_SPUMemoryBlock m_pDummyBlocks[NumDummyBlocks];
		U32 m_NumBlocks;
#endif

		C_SPUMemoryPool(C_StaticSPUMemoryAllocator* pPool);

		~C_SPUMemoryPool();

		

		template <typename Thing, bool bConstruct>
		Thing* Allocate()
		{
			Thing* thing = (Thing*)Alloc(sizeof(Thing));
			if(bConstruct)
			{
				//Spu::MemBaseMarker marker;
				//Thing* localThing = (Thing*)Spu::FetchAlloc(thing, sizeof(Thing));
				//Work-around for SPU - we construct an SPU v-table for this thing in PPU memory!
				//new (localThing) Thing;
#ifdef PLATFORM_SPU
				Thing localThing;
				Spu::Store(thing, &localThing, sizeof(Thing));
#else
				new (thing) Thing;
#endif
			}
			return thing;
		}

		template <typename Thing, bool bConstruct>
		Thing* Allocate(const U32 numThings, const U32 align)
		{
#ifdef PLATFORM_SPU
			Thing* things = (Thing*)Alloc((sizeof(Thing) * numThings) + align, align);
#else
			Thing* things = (Thing*)Alloc(sizeof(Thing) * numThings);
#endif
			if(bConstruct)
			{
				for(U32 a = 0; a < numThings; ++a)
				{
					new (things + a) Thing;
				}
			}
			return things;
		}

		template <typename Thing, bool bConstruct>
		Thing* Allocate(const U32 numThings)
		{
			Thing* things = (Thing*)Alloc(sizeof(Thing) * numThings);
			if(bConstruct)
			{
				for(U32 a = 0; a < numThings; ++a)
				{
					new (things + a) Thing;
				}
			}
			return things;
		}

		void Reset()
		{
			m_pCurrentBlock = m_pFirstBlock;
			m_pCurrentBlock->m_CurrentIndex = 0;
		}
	private:
		void* Alloc(const U32 size);
		void* Alloc(const U32 size, const U32 align);
	};

	class C_SPUFreeList
	{
	public:
		C_SPUMemoryBlock* m_pFreeElems;
	};

	class ALIGN16 C_StaticSPUMemoryAllocator
	{
		friend class C_SPUMemoryPool;

		MT::AtomicLock m_Mutex;
		C_SPUMemoryBlock* m_pBlocks;
		C_SPUFreeList* m_pFreeList;
		//C_SPUMemoryBlocks* m_pFreeList;
		U32 m_NumElements;
	public:
#ifndef PLATFORM_SPU
		C_StaticSPUMemoryAllocator(const U32 numElements, const U32 blockSize) : m_NumElements(numElements)
		{
			m_pBlocks = new C_SPUMemoryBlock[numElements];
			for(U32 a = 0; a < numElements; ++a)
				new (m_pBlocks + a) C_SPUMemoryBlock(blockSize);
			m_pFreeList = new C_SPUFreeList;
			Reset();

		}

		~C_StaticSPUMemoryAllocator()
		{
			delete m_pFreeList;
			delete [] m_pBlocks;
			m_pBlocks = NULL;
		}

		void Reset()
		{
			if(m_NumElements > 0)
			{
				for(U32 a = 1; a < m_NumElements; ++a)
				{
					m_pBlocks[a-1].m_pNext = &m_pBlocks[a];
				}
				m_pBlocks[m_NumElements - 1].m_pNext = NULL;
			}
			m_pFreeList->m_pFreeElems = m_pBlocks;
		}
#endif
		//
#ifndef PLATFORM_SPU
		C_SPUMemoryBlock* Alloc();
#else
		C_SPUMemoryBlock* Alloc(C_SPUMemoryBlock* pBlock);
#endif
		void Release(C_SPUMemoryBlock* pBlock);
		void ReleaseList(C_SPUMemoryBlock* pBlock);
	};

#ifdef PLATFORM_SPU	
	class ALIGN16 C_CachePage
	{
	public:
		char m_Data[CachePageSize];
	};


	//A generic cache will need to be able to deal with data crossing
	//between pages. However, this won't be necessary for now because I know that my
	//start addess will be 16-byte aligned and 32-bytes long, meaning they will fit exactly into
	//my 256-byte pages without any overhang :-)

	class ALIGN16 C_CacheHeader
	{
	public:
		C_CachePage* m_pPage;
		S32 m_PPUStartAddress;
	
		bool m_bTouched;

		C_CacheHeader() : m_PPUStartAddress(-1)
		{
			m_pPage = (C_CachePage*)Spu::Alloc(sizeof(C_CachePage));
		}

		bool IsPpuAddyFullyInRange(const void* ppuAddress, const U32 size)
		{
			U32 addy = (U32)ppuAddress;
			return (addy >= m_PPUStartAddress) && ((addy + size) <= (m_PPUStartAddress + CachePageSize));
		}

	};

#endif


	/*
	 * An SPU Cache. It locally stores data blocks (pages) for quick access and evicts them using 
	 * a LRU scheme. 
	 */
	const U32 NumCachePages = 4;
	class C_SPUCache
	{
#ifdef PLATFORM_SPU
		//C_CachePage m_Pages[NumCachePages];
		C_CacheHeader m_Headers[NumCachePages];
		U32 m_Oldest;
#endif
	public:
		C_SPUCache();
		~C_SPUCache();
		//Allocates a given pointer from its memory pool and fetches the data into it
		void* FetchAllocData(void* ppuAddress, const U32 size);
		void* FetchAlloc(void* ppuAddress, const U32 size);
		void* FetchAllocAsync(void* ppuAddress, const U32 size, const U32 readMask);
		void* CreateAlloc(void* ppuAddress, const U32 size);
		void Store(void* ppuAddress, void* spuAddress, const U32 size);
		void Wait(const U32 tag)
		{
#ifdef PLATFORM_SPU
			Spu::Wait(tag);
#endif
		}
	};

	const U32 SmallCachePageSize = 1024;
#ifdef PLATFORM_SPU	
	class ALIGN16 C_SmallCachePage
	{
	public:
		char m_Data[SmallCachePageSize];
	};


	//A generic cache will need to be able to deal with data crossing
	//between pages. However, this won't be necessary for now because I know that my
	//start addess will be 16-byte aligned and 32-bytes long, meaning they will fit exactly into
	//my 256-byte pages without any overhang :-)

	class ALIGN16 C_SmallCacheHeader
	{
	public:
		C_SmallCachePage m_Page;
		S32 m_PPUStartAddress;
	
		bool m_bTouched;

		C_SmallCacheHeader() : m_PPUStartAddress(-1)
		{
		}


		bool IsPpuAddyFullyInRange(const void* ppuAddress, const U32 size)
		{
			U32 addy = (U32)ppuAddress;
			return (addy >= m_PPUStartAddress) && ((addy + size) <= (m_PPUStartAddress + SmallCachePageSize));
		}

	};
#endif
//#define C_SmallSPUCache C_SPUCache
	
	/*
	 * An SPU Cache. It locally stores data blocks (pages) for quick access and evicts them using 
	 * a LRU scheme. 
	 */
	//const U32 SmallNumCachePages = 4;
	
	class C_SmallSPUCache
	{
#ifdef PLATFORM_SPU
		C_SmallCacheHeader* m_Headers;
		U32 m_Oldest;
#endif
		U32 m_NumCachePages;
	public:
		C_SmallSPUCache(U32 numCachePages = 2) : m_NumCachePages(numCachePages)
		{
#ifdef PLATFORM_SPU
			m_Headers = (C_SmallCacheHeader*) Spu::Alloc(sizeof(C_SmallCacheHeader)*m_NumCachePages);
			//Reset the headers!!!
			for(U32 a = 0; a < numCachePages; ++a)
			{
				m_Headers[a].m_PPUStartAddress = -1;
			}

			m_Oldest = 0;
#endif
		}

		~C_SmallSPUCache()
		{

		}
		//Allocates a given pointer from its memory pool and fetches the data into it
		void* FetchAllocData(void* ppuAddress, const U32 size);
		void* FetchAlloc(void* ppuAddress, const U32 size);
		void* FetchAllocAsync(void* ppuAddress, const U32 size, const U32 readMask);
		void* CreateAlloc(void* ppuAddress, const U32 size);
		void Store(void* ppuAddress, void* spuAddress, const U32 size);
		void Wait(const U32 tag)
		{
#ifdef PLATFORM_SPU
			Spu::Wait(tag);
#endif
		}
	};

#ifdef PLATFORM_SPU
	void PreFetchMessage(U32 fetchElements);
	void NextElementMessage(U32 fetchElements);
	void WaitMessage(U32 m_NextStreamElements, U32 index);
	void StreamMessage(U32 m_NextStreamElements, U32 index);
	void WriteBackMessage(U32 currentIndex, U32 m_WriteIndex);
#endif


	template <typename Element, S32 BufferSize, S32 BufferReadBackStride>
	class C_SpuArrayStreamer
	{
#ifdef PLATFORM_SPU
		Element* m_Elements;
		S32 m_NextStreamElement;
		S32 m_WriteIndex;
		S32 m_CurrentElement;
#endif

		Element** m_pPpuElements;
		
		S32 m_CurrentIndex;
		S32 m_MaxElement;
		

	public:
		C_SpuArrayStreamer(Element** pElements, S32 numElements) : m_pPpuElements(pElements), m_MaxElement(numElements - 1),
			m_CurrentIndex(0)
		{
#ifdef PLATFORM_SPU
			m_Elements = (Element*)Spu::Alloc(sizeof(Element) * BufferSize);
			const S32 fetchElements = Maths::Min(numElements, BufferReadBackStride);
			for(U32 a = 0; a < fetchElements; ++a)
			{
				PreFetchMessage(a);
				Spu::FetchAsync(&m_Elements[a], m_pPpuElements[a], sizeof(Element), a);
			}
			NextElementMessage(fetchElements);
			m_NextStreamElement = fetchElements;
			m_CurrentElement = 0;
#endif
		}

		~C_SpuArrayStreamer()
		{
#ifdef PLATFORM_SPU
			U32 mask = 0;
			for(U32 a = 0; a < BufferSize+1; ++a)
			{
				mask |= (1 << a);
			}
			Spu::WaitMultiple(mask);
#endif
		}

		bool HasNextElement()
		{
			return m_CurrentIndex <= m_MaxElement;
		}

		Element* NextElement()
		{
#ifdef PLATFORM_SPU
			m_WriteIndex = m_CurrentElement;
			WaitMessage(m_CurrentIndex, m_CurrentElement);
			Spu::Wait(m_CurrentElement);
			Element* pElement = &m_Elements[m_CurrentElement];
			m_CurrentElement = (m_CurrentElement + 1)%BufferSize;
			if(m_NextStreamElement <= m_MaxElement)
			{
				U32 index = m_NextStreamElement % BufferSize;
				StreamMessage(m_NextStreamElement, index);
				Spu::Wait(index);
				//Spu::WaitMultiple(0xffffffff);
				Spu::FetchAsync(&m_Elements[index], m_pPpuElements[m_NextStreamElement++], sizeof(Element), index);
			}
#else
			Element* pElement = m_pPpuElements[Maths::Min(m_CurrentIndex, m_MaxElement)];
#endif
			++m_CurrentIndex;
			return pElement;
		}
		void WriteBackElement()
		{
#ifdef PLATFORM_SPU
			S32 index = m_WriteIndex;
			WriteBackMessage(m_CurrentIndex - 1, m_WriteIndex);
			Element* pElement = &m_Elements[index];
			Spu::StoreAsync(m_pPpuElements[m_CurrentIndex - 1], pElement, sizeof(Element), index);
#endif
		}
	};

}

#include "gravity/sharedphysics/SPU_Memory.inl"

#endif
