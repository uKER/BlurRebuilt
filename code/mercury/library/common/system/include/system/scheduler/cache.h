#ifndef JOBQUEUE_CACHE_H
#define JOBQUEUE_CACHE_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/vector.h"

namespace Spu
{
	class CircularCache
	{
	public:
		CircularCache();
		
		~CircularCache();

		// Initialise the cache with a size in bytes and pointer to the buffer.
		bool Initialise(S32 cacheSize, U8* pBuffer);

		// Cache the area of memory specified.
		// The next call to CacheMemory invalidates the returned pointer.
		const U8* CacheMemory(const void* pMain, S32 size);

		// After calling CacheMemory, call this before using the cache again.
		void Release(const void* pMain);

		// Call this if the data has changed.
		void Reset();

	private:
		// Copy constructor initialises to the size of the other cache.
		// Operator = does nothing.
		CircularCache(const CircularCache& other);
		CircularCache& operator = (const CircularCache& other) {return *this;}

		void Clear();

		U8* m_pBuffer;
		S32 m_bufferSize;

		struct Block
		{
			const U8* pCachedAddress;
			U8* pData;
			S32 nBlocks;
			S32 wrapCount;
		};

		S32 m_wrapCount;
		U8* m_pWritePosition;
		Btl::Vector<Block, Btl::VectorParameters_Array<16> > m_blocks;
		bool m_locked;

		S32 m_nMisses;
		S32 m_nHits;
	};

	void TestCircularCache();
}

#endif