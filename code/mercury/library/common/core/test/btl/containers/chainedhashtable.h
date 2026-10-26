#include "test/btl/test.h"
#include <cxxtest/TestSuite.h>
#include "btl/containers/chainedhashmap.h"
#include "btl/containers/chainedhashset.h"
#include "test/btl/archive.h"
#include "btl/utility/string.h"

using namespace Btl;

class Btl_ChainedHashTable_Tests : public CxxTest::TestSuite
{
public:
	void testChainedHashSet()
	{
		typedef ChainedHashSet<U32> SetType;
		SetType set;

		SetType::InsertionReturnType r = set.Insert(0);

		assert(r.iterator != set.End());
		assert(*r.iterator == 0);
		assert(r.inserted = true);
		assert(set.Size() == 1);

		set.Insert(1);
		set.Insert(2);
		set.Insert(3);
		set.Insert(4);
		r = set.Insert(5);
		assert(*r.iterator == 5);

		r = set.Insert(5);
		assert(r.inserted == false);
		assert(*r.iterator == 5);

		assert(set.Size() == 6);
		int count = 0;

		for (SetType::Iterator i = set.Begin(); i != set.End(); i++)
		{
			assert(*i >= 0 && *i <= 5);
			count++;
		}

		assert(count == 6);
	}
	
	void testChainedHashSet2()
	{
		typedef ChainedHashSet<U32> SetType;
		SetType set1;
		set1.Insert(0);
		set1.Insert(1);

		SetType set2(set1);

		assert(set1.Size() == 2);
		assert(set2.Size() == 2);

		set2.Insert(2);

		SetType set3;
		set3 = set2;

		assert(set3.Size() == 3);

		set2 = set2;
		assert(set2.Size() == 3);
	}

	void testChainedHashSet3()
	{
		typedef ChainedHashSet<U32> SetType;
		SetType set;
		set.Insert(0);
		set.Insert(1);
		set.Insert(2);

		SetType::Iterator i = set.Find(1);
		assert(*i == 1);

		set.Erase(1);
		i = set.Find(1);
		assert(i == set.End());

		set.Clear();
		assert(set.Size() == 0);
	}

	void testChainedHashSet4()
	{
		typedef ChainedHashSet<U32> SetType;
		SetType set;

		for (int i = 0; i < 1000; i++)
		{
			int n = rand();
			set.Erase(n);
			assert(set.Find(n) == set.End());
			n = rand();
			set.Insert(n);
			assert(set.Find(n) != set.End());
		}

		for (SetType::Iterator it = set.Begin(); it != set.End(); ++it)
		{
			//char buffer[128];
			//_itoa_s(*i, buffer, 128, 10);
			//OutputDebugStringA(buffer);
			//OutputDebugStringA("\n");
		}
	}

	void testChainedHashSet5()
	{
		typedef ChainedHashSet<U32, HashKeyOpSmall<U32>, DynamicPool<NullType> > SetType;
		SetType set;

		for (int i = 0; i < 1000; i++)
		{
			int n = rand();
			set.Erase(n);
			assert(set.Find(n) == set.End());
			n = rand();
			set.Insert(n);
			assert(set.Find(n) != set.End());
		}

		for (SetType::Iterator it = set.Begin(); it != set.End(); ++it)
		{
			//char buffer[128];
			//_itoa_s(*i, buffer, 128, 10);
			//OutputDebugStringA(buffer);
			//OutputDebugStringA("\n");
		}
	}

	void testChainedHashMultiSet()
	{
		ChainedHashMultiSet<U32> set;
		set.Insert(0);
		set.Insert(1);
		set.Insert(1);

		assert(set.Size() == 3);

		for (ChainedHashMultiSet<U32>::Iterator i = set.Begin(); i != set.End(); ++i)
		{
			//int breakhere = 0;
		}

		int count = set.Erase(1);
		assert(count == 2);
	}

	void testChainedHashMap()
	{
		typedef ChainedHashMap<U32, String> MapType;
		MapType map;
		map.Insert(MapType::DataType(0, "Test0"));
		map.Insert(MapType::DataType(1, "Test1"));
		map[2] = "Test2";
		MapType::InsertionReturnType i = map.Insert(3);
		i.iterator->GetValue() = "Test3";

		assert(map.Size() == 4);

		MapType::Iterator r = map.Find(0);
		assert(r != map.End());
		assert(r->GetValue() == "Test0");

		r = map.Find(5);
		assert(r == map.End());

		map.Erase(2);
		r = map.Find(2);
		assert(r == map.End());
		assert(map.Size() == 3);

		map.Erase(map.Begin());

		MapType::Iterator it = map.Begin();
		it++;
		map.Erase(it);
		assert(map.Size() == 1);

		map.Clear();
		assert(map.Size() == 0);
	}

	void testChainedHashMultiMap()
	{
		typedef ChainedHashMultiMap<U32, U32> MapType;
		MapType map;
		map.Insert(MapType::DataType(0, 10));
		map.Insert(MapType::DataType(1, 11));
		map.Insert(MapType::DataType(2, 12));

		assert(map.Size() == 3);

		MapType::Iterator i = map.Find(1);
		assert(i->GetKey() == 1);
		assert(i->GetValue() == 11);

		for (MapType::Iterator i = map.Begin(); i != map.End(); ++i)
		{
			//int breakhere = 0;
		}

		int count = map.Erase(1);
		assert(count == 1);
	}
};
