#include "test/btl/test.h"
#include "btl/containers/openhashmap.h"
#include "btl/containers/openhashset.h"
#include <hash_set>
#include <cxxtest/TestSuite.h>

using namespace Btl;

class Btl_HashTable_Tests : public CxxTest::TestSuite
{
public:
	void testOpenHashSet()
	{
		typedef OpenHashSet<U32> SetType;
		SetType set;
		set.Insert(0);
		set.Insert(0);
		set.Insert(1);
		set.Insert(2);
		assert(set.Size() == 3);

		SetType::Iterator i = set.Find(1);
		assert(*i == 1);

		//set.Erase(1); missing function

		SetType::InsertionReturnType r = set.Insert(4);
		assert(r.inserted);
		assert(*r.iterator == 4);

		set.Clear();
		assert(set.Empty());
	}

	void testOpenHashSet2()
	{
		typedef OpenHashSet<U32> SetType;
		SetType set1;
		set1.Insert(0);
		SetType set2(set1);
		set2.Insert(1);
		set1 = set2;
		set1.Insert(2);
		Swap(set1, set2); // note: could use more efficient swap
	}

	void testOpenHashSet3()
	{
		typedef OpenHashSet<U32> SetType;
		SetType set1, set2;
		set1.Insert(0);
		set2.Insert(0);
		set2.Insert(1);
		set1 = set2;
		assert(set1.Size() == 2);
	}

	void testOpenHashMap()
	{
		typedef OpenHashMap<U32, U32> MapType;
		MapType map1, map2;
		map1.Insert(MapType::DataType(0, 0));
		map2.Insert(MapType::DataType(0, 1));
		map2.Insert(MapType::DataType(1, 1));
		map1 = map2;
		assert(map1.Size() == 2);

		for (int i = 0; i < 1000; i++)
		{
			int n = rand();
			map1.Insert(MapType::DataType(n, 0));
		}
	}

	void testOpenhashSet4()
	{
		typedef OpenHashSet<U32> SetType;
		SetType set;

		for (int i = 0; i < 1000; i++)
		{
			U32 n = rand();
			SetType::Iterator it = set.Find(n);
			if (it != set.End()) set.Erase(it);
			assert(set.Find(n) == set.End());
			n = rand();
			set.Insert(n);
			assert(*set.Find(n) == n);
		}

		for (SetType::Iterator it = set.Begin(); it != set.End(); ++it)
		{
			//char buffer[128];
			//_itoa_s(*i, buffer, 128, 10);
			//OutputDebugStringA(buffer);
			//OutputDebugStringA("\n");
		}
	}
};
