#include "SystemPch.h"
#include "system/scheduler/jobqueue.h"
#include "system/scheduler/cache.h"

#include "btl/utility/types.h"

namespace Spu
{

const int CACHE_LINE_SIZE = 128;
const int N_BITS = 7;

CircularCache::CircularCache()
{
	Clear();
}

void CircularCache::Clear()
{
	m_pBuffer = 0;
	m_bufferSize = 0;
	m_wrapCount = 0;
	m_pWritePosition = 0;
	m_locked = false;
	m_nMisses = 0;
	m_nHits = 0;
}

CircularCache::~CircularCache()
{
	//delete [] m_pBuffer;
}

void CircularCache::Release(const void* p)
{
	m_locked = false;
}

bool CircularCache::Initialise(S32 cacheSize, U8* pBuffer)
{
	if (m_pBuffer)
	{
		BIZ_ASSERT_FAIL("Tried to initialise cache twice.");
		return false;
	}

	// Create the buffer.
	m_pBuffer = pBuffer;
	assert(m_pBuffer);

	m_bufferSize = cacheSize;

	// Initialise the blocks.
	memset(m_blocks.First(), 0, m_blocks.Size() * sizeof(Block));

	m_pWritePosition = m_pBuffer;

	return true;
}

void CircularCache::Reset()
{
	BIZ_ASSERT(!m_locked);

	memset(m_blocks.First(), 0, m_blocks.Size() * sizeof(Block));
	m_pWritePosition = m_pBuffer;
	m_wrapCount = 0;
}

const U8* CircularCache::CacheMemory(const void* p, S32 size)
{
	BIZ_ASSERT(!m_locked);
	BIZ_ASSERT(m_pBuffer);
	BIZ_ASSERT(size <= m_bufferSize);

	if (!p)
		return 0;

	m_locked = true;

	const U8* pMain = (U8*) p;

	// Find the block index.
	U32 startBlock = ((U32(INTPTR(pMain)) >> N_BITS) & (m_blocks.Size() - 1));	// [Blur2PC][x64] low address bits
	Block& block = m_blocks[startBlock];

	const U8* pAligned = Btl::AlignDown(pMain, CACHE_LINE_SIZE);
	U32 offsetInBlock = pMain - pAligned;
	S32 alignedSize = Btl::AlignUp(offsetInBlock + size, CACHE_LINE_SIZE);
	S32 nBlocks = alignedSize >> N_BITS;
	S32 blocksCached = 0;

	// Is it already cached?
	if (block.pCachedAddress == pAligned
		&& (block.wrapCount == m_wrapCount
			|| (block.wrapCount == m_wrapCount - 1 && block.pData > m_pWritePosition)))
	{
		S32 sizeCached = block.nBlocks * CACHE_LINE_SIZE;

		// If there's enough data cached, return the pointer from the block.
		if (sizeCached >= alignedSize)
		{
			m_nHits++;
			return block.pData + offsetInBlock;
		}
		
		// If there's not enough data, see if we can cache some more.
		if (m_pWritePosition == block.pData + sizeCached
			&& block.pData + alignedSize <= m_pBuffer + m_bufferSize)
			blocksCached = block.nBlocks;
	}

	m_nMisses++;

	const U8* pCachedAddress = pAligned + blocksCached * CACHE_LINE_SIZE;
	S32 sizeToCache = alignedSize - blocksCached * CACHE_LINE_SIZE;

	// Is there room in the buffer?
	if (m_pWritePosition + sizeToCache > m_pBuffer + m_bufferSize)
	{
		m_pWritePosition = m_pBuffer;
		m_wrapCount++;
		assert(sizeToCache <= m_bufferSize);
	}

	// Get the data.
	//memcpy(m_pWritePosition, pCachedAddress, sizeToCache);
	Spu::Fetch(m_pWritePosition, pCachedAddress, sizeToCache);

	// Update the blocks
	for (int i = 0; i < nBlocks; i++)
	{
		S32 iBlock = (i + startBlock) % m_blocks.Size();
		S32 offset = (i - blocksCached) * CACHE_LINE_SIZE;
		Block& block = m_blocks[iBlock];
		block.pCachedAddress = pCachedAddress + offset;
		block.pData = m_pWritePosition + offset;
		block.nBlocks = nBlocks - i;
		block.wrapCount = m_wrapCount;
	}

	m_pWritePosition += sizeToCache;

	return block.pData + offsetInBlock;
}

void TestCircularCache()
{
	Btl::Vector<U32> testData;
	testData.Resize(32 * 1024);

	for (int i = 0; i < testData.Size(); i++)
		testData[i] = i;
	
	Spu::CircularCache testCache;

	Btl::Vector<U8> testBuffer;
	testBuffer.Resize(1024);

	testCache.Initialise(testBuffer.Size(), testBuffer.First());

	for (int i = 0; i < 100000; i++)
	{
		S32 index1 = rand() % (testData.Size() - 4);

		U32* pTest1 = (U32*) testCache.CacheMemory(&testData[index1], 16);

		assert(pTest1[0] == index1);
		assert(pTest1[1] == index1 + 1);
		assert(pTest1[2] == index1 + 2);
		assert(pTest1[3] == index1 + 3);

		testCache.Release(pTest1);

		S32 index2 = i % (testData.Size() - 4);

		U32* pTest2 = (U32*) testCache.CacheMemory(&testData[index2], 16);

		assert(pTest2[0] == index2);
		assert(pTest2[1] == index2 + 1);
		assert(pTest2[2] == index2 + 2);
		assert(pTest2[3] == index2 + 3);

		testCache.Release(pTest2);
	}
}

}