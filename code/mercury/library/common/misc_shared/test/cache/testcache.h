#include <core/core.h>
#include <cxxtest/TestSuite.h>

#include <system/serialise.h>

#include <misc_shared/cache/cache.h>
#include <misc_shared/cache/cachekey.h>

class CacheTest : public CxxTest::TestSuite
{
public:
	void testCache()
	{
		const char *pstTmp = getenv("TMP");

		DiskCache::Cache cache(pstTmp, 0, "jan");

		Util::C_String test("test cache key string");
		Util::C_String teststring("jan test hello");

		// create an entry
		DiskCache::CacheEntry *pEntry = cache.CreateEntry(DiskCache::GenerateCacheKey(test));
		TS_ASSERT(pEntry);
		if (!pEntry)
			return;
		Sys::I_BinaryStream *pStream = pEntry->SetData("jan");
		TS_ASSERT(pStream);
		if (!pStream)
			return;
		Sys::BinarySerialiser<Ut::NoEndianSwap> ar(*pStream);
		ar << teststring << 10 << 12 << 100;
		delete pStream;
		delete pEntry;

		// read an entry back
		pEntry = cache.GetEntry(DiskCache::GenerateCacheKey(test));
		TS_ASSERT(pEntry);
		if (pEntry)
		{
			Util::C_String teststr;
			pStream = pEntry->GetData("jan");
			TS_ASSERT(pStream);
			if (!pStream)
				return;
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pStream);
			ar >> teststr;
			
			TS_ASSERT(!strcmp(teststr.Chars(), teststring.Chars()));

			delete pStream;
			delete pEntry;
		}
	}
};

