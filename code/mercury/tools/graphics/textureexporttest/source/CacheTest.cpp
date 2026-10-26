#include "stdafx.h"
#include <system/xml/XmlDom.h>

#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <misc_shared/cache/cache.h>
#include <misc_shared/cache/cachekey.h>
#include <system/perf/profiler.h>
#include <system/perf/profilerbackend.h>
#include <core/threads.h>

#define MEMORY_TRACKER_CALLSTACK 1
#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS 1
#define MEMORY_TRACKER_BLOCK_LOCATIONS 1

#include "system\memory\MemoryTrackerDefines.h"
#include "system\memory\MemoryTracker.h"
#include "system\memory\AllocatorFunctions.h"

void* operator new[]( size_t size ) { return Memory::TrackedAlignedAlloc(size, 16); }
void operator delete[]( void* ptr ) { Memory::TrackedAlignedFree(ptr); }
void * operator new (size_t size) { return Memory::TrackedAlignedAlloc(size,16); }
void operator delete(void *mem) { Memory::TrackedAlignedFree(mem); }
void* BizAlignedMalloc(size_t size, size_t alignment) { return Memory::TrackedAlignedAlloc(size,alignment); }
void BizAlignedFree(void *p) { Memory::TrackedAlignedFree(p); }

void CacheTest()
{
	Memory::Tracker::Instance().DumpBlocksOnExit(true);
	MEMORY_TRACKER_ENABLE_CALLSTACK_CAPTURE;

	DiskCache::Cache cache("c:\\temp\\testcache", 0, "jan");

	Util::C_String test("test cache key string");

	// create an entry
	DiskCache::CacheEntry *pEntry = cache.CreateEntry(DiskCache::GenerateCacheKey(test));
	Sys::I_BinaryStream *pStream = pEntry->SetData("jan");
	Sys::BinarySerialiser<Ut::NoEndianSwap> ar(*pStream);
	ar << Util::C_String("jan test hello") << 10 << 12 << 100;
	delete pStream;
	delete pEntry;

	// read an entry back
	pEntry = cache.GetEntry(DiskCache::GenerateCacheKey(test));
	if (pEntry)
	{
		Util::C_String teststr;
		pStream = pEntry->GetData("jan");
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pStream);
		ar >> teststr;
		printf("result = %s\n", test.Chars());

		delete pStream;
		delete pEntry;
	}
}

