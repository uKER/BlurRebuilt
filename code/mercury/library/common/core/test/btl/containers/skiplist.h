#include "test/btl/test.h"
#include "btl/containers/map.h"
#include "btl/containers/multimap.h"
#include "btl/containers/set.h"
#include "btl/containers/multiset.h"
#include "btl/containers/skiplist.h"
#include "test/btl/archive.h"
#include <cxxtest/TestSuite.h>

using namespace Btl;

class Value2
{
public:
	Value2() {}
	Value2(int i) : m_i(i) {}
	int m_i;
};

class Btl_SkipList_Tests : public CxxTest::TestSuite
{
public:
	void testMap()
	{
		typedef SkipListMap<U32, Value2> Map1;
		Map1 map;
		Map1::DataType v(0, Value2(1));
		map.Insert(v);

		assert(map.Size() == 1);
		assert(map.Begin()->GetKey() == 0);
		assert(map.Begin()->GetValue().m_i == 1);

		map[1] = Value2(2);
		
		map.Clear();

		assert(map.Size() == 0);
		assert(map.Empty());
	}

	void testSet()
	{
		SkipListSet<U32> set;
		set.Insert(1);
		set.Insert(4);
		set.Insert(7);
		set.Insert(9);
		set.Insert(2);
		set.Insert(6);
		set.Insert(8);
		set.Insert(5);
		set.Insert(10);
		set.Insert(3);

		U32 count = 1;

		for (SkipListSet<U32>::Iterator i = set.Begin(); i != set.End(); ++i, ++count)
			assert(*i == count);
	}

	void testMultiSet()
	{
		typedef SkipListMultiSet<U32> Set1;
		Set1 set;
		set.Insert(0);
		set.Insert(1);
		set.Insert(1);
		set.Insert(2);
		set.Insert(3);
		assert(set.Size() == 5);

		Set1::Iterator i;

		i = set.Find(4);
		assert(i == set.End());

		i = set.Find(1);
		assert(i != set.End());

		set.Erase(i);
		i = set.Find(1);

		assert(i != set.End());
		set.Erase(1);

		i = set.Find(1);
		assert(i == set.End());
	}

	void testMultiMap()
	{
		typedef SkipListMultiMap<U32, Value> Map1;
		Map1 map;

		for (int i = 0; i < 1000; i++)
		{
			int n = rand();
			map.Erase(n);
			n = rand();
			map.Insert(Map1::DataType(n, Value(n)));
		}
	}

	void testSetCopy()
	{
		SkipListSet<U32> set1, set2;
		set1.Insert(0);
		set1.Insert(1);
		set2.Insert(2);
		set2 = set1;
		set1.Clear();
		assert(set1.Empty());
		assert(set2.Size() == 2);
	}
};


