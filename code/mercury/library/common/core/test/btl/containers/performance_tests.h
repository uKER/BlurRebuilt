#include "test/btl/test.h"
#include "btl/containers/map.h"
#include "btl/containers/set.h"
#include "btl/containers/openhashmap.h"
#include "btl/containers/openhashset.h"
#include "btl/containers/chainedhashmap.h"
#include "btl/containers/chainedhashset.h"
#include "btl/containers/skiplist.h"
#include <hash_set>
#include <hash_map>
#include <set>
#include <map>
#include <cxxtest/TestSuite.h>

using namespace Btl;

static U32 g_test2;

#ifdef PLATFORM_PS3
#define stdext std
#endif

struct TestItem
{
	TestItem() {}
	TestItem(int i) {data[0] = i;}
	U32 data[100];
};

bool operator < (const TestItem& lhs, const TestItem& rhs)
{
	return lhs.data[0] < rhs.data[0];
}

bool operator == (const TestItem& lhs, const TestItem& rhs)
{
	return lhs.data[0] == rhs.data[0];
}

U32 Hash(const TestItem& t)
{
	return Hash(t.data[0]);
}

U32 hash_value(const TestItem& t)
{
	return t.data[0];
}

void InsertElements()
{
	Timer t("Insert elements");
	const static int N = 1000000;
	typedef U32 T; 

	{
		Timer t("Null");
		for (int i = 0; i < N; i++)
			g_test2 = rand() | (rand() << 16);
	}

	{
		Timer t("Btl::Set");
		Set<T> set;
		for (int i = 0; i < N; i++)
			set.Insert(rand() | (rand() << 16));
	}

	{
		Timer t("std::set");
		std::set<T> set;
		for (int i = 0; i < N; i++)
			set.insert(rand() | (rand() << 16));
	}

	{
		Timer t("Btl::SkipList");
		SkipListSet<T> skipList;
		for (int i = 0; i < N; i++)
			skipList.Insert(rand() | (rand() << 16));
	}

	{
		Timer t("Btl::OpenHashSet");
		OpenHashSet<T, HashKeyOpDefault<T> > set;
		for (int i = 0; i < N; i++)
			set.Insert(rand() | (rand() << 16));
	}

	{
		Timer t("Btl::ChainedHashSet");
		ChainedHashSet<T, HashKeyOpDefault<T> > set;
		for (int i = 0; i < N; i++)
			set.Insert(rand() | (rand() << 16));
	}

	{
		Timer t("Btl::ChainedHashSet (pool)");
		ChainedHashSet<T, HashKeyOpDefault<T>, DynamicPool<NullType> > set;
		for (int i = 0; i < N; i++)
			set.Insert(rand() | (rand() << 16));
	}

	{
		Timer t("std::hash_set");
		stdext::hash_set<T> set;
		for (int i = 0; i < N; i++)
			set.insert(rand() | (rand() << 16));
	}
}

void InsertAndEraseElements()
{
	Timer t("Insert and erase elements");
	const static int N = 5000000;
	typedef U32 T; 

	{
		Timer t("Null");
		for (int i = 0; i < N; i++)
		{
			g_test2 = rand();
			g_test2 = rand();
		}
	}

	{
		Timer t("Btl::Set");
		Set<T> set;
		for (int i = 0; i < N; i++)
		{
			set.Insert(rand());
			set.Erase(rand());
		}
	}

	{
		Timer t("std::set");
		std::set<T> set;
		for (int i = 0; i < N; i++)
		{
			set.insert(rand());
			set.erase(rand());
		}
	}

	{
		Timer t("Btl::SkipList");
		SkipListSet<T> set;
		for (int i = 0; i < N; i++)
		{
			set.Insert(rand());
			set.Erase(rand());
		}
	}

	{
		Timer t("Btl::OpenHashSet");
		typedef OpenHashSet<T, HashKeyOpDefault<T> > SetType;
		SetType set;
		for (int i = 0; i < N; i++)
		{
			set.Insert(rand());
			SetType::Iterator it = set.Find(rand());
			if (it != set.End()) set.Erase(it);
		}
	}

	{
		Timer t("Btl::ChainedHashSet");
		ChainedHashSet<T, HashKeyOpDefault<T> > set;
		for (int i = 0; i < N; i++)
		{
			set.Insert(rand());
			set.Erase(rand());
		}
	}

	{
		Timer t("Btl::ChainedHashSet (pool)");
		ChainedHashSet<T, HashKeyOpDefault<T>, DynamicPool<NullType> > set;
		for (int i = 0; i < N; i++)
		{
			set.Insert(rand());
			set.Erase(rand());
		}
	}

	{
		Timer t("std::hash_set");
		stdext::hash_set<T> set;
		for (int i = 0; i < N; i++)
		{
			set.insert(rand());
			set.erase(rand());
		}
	}
}

void SearchForElements()
{
	Timer t("Search for elements");
	const static int N = 10000000;
	typedef U32 T; 

	{
		Timer t("Null");
		for (int i = 0; i < N; i++)
			g_test2 = rand();
	}

	{
		Set<T> set;
		for (int i = 0; i < N; i++)
		{
			set.Insert(rand());
			//set.Erase(rand());
		}

		Timer t("Btl::Set");
		for (int i = 0; i < N; i++)
			g_test2 = (set.Find(rand()) != set.End());
	}

	{
		std::set<T> set;
		for (int i = 0; i < N; i++)
		{
			set.insert(rand());
			//set.erase(rand());
		}

		Timer t("std::set");
		for (int i = 0; i < N; i++)
			g_test2 = (set.find(rand()) != set.end());
	}

	{
		SkipListSet<T> set;
		for (int i = 0; i < N; i++)
		{
			set.Insert(rand());
			//set.Erase(rand());
		}

		Timer t("Btl::SkipListSet");
		for (int i = 0; i < N; i++)
			g_test2 = (set.Find(rand()) != set.End());
	}

	{
		typedef OpenHashSet<U32, HashKeyOpDefault<U32> > OpenHashSetType;
		OpenHashSetType openhashset;

		for (int i = 0; i < N; i++)
		{
			openhashset.Insert(rand());
			OpenHashSetType::Iterator it = openhashset.Find(rand());
			//if (it != openhashset.End()) openhashset.Erase(it);
		}

		Timer t("Btl::OpenHashSet");
		for (int i = 0; i < N; i++)
			g_test2 = (openhashset.Find(rand()) != openhashset.End());
	}

	{
		ChainedHashSet<T, HashKeyOpDefault<T> > set;
		for (int i = 0; i < N; i++)
		{
			set.Insert(rand());
			//set.Erase(rand());
		}

		Timer t("Btl::ChainedHashSet");
		for (int i = 0; i < N; i++)
			g_test2 = (set.Find(rand()) != set.End());
	}

	{
		typedef ChainedHashSet<T, HashKeyOpDefault<T>, DynamicPool<NullType> > SetType;
		SetType set;
		for (int i = 0; i < N; i++)
		{
			set.Insert(rand());
			//set.Erase(rand());
		}

		SetType::Iterator end = set.End();

		Timer t("Btl::ChainedHashSet (pool)");
		for (int i = 0; i < N; i++)
			g_test2 = (set.Find(rand()) != end);
	}

	{
		stdext::hash_set<T> set;
		for (int i = 0; i < N; i++)
		{
			set.insert(rand());
			//set.erase(rand());
		}

		Timer t("std::hash_set");
		for (int i = 0; i < N; i++)
			g_test2 = (set.find(rand()) != set.end());
	}
}

void IterateThroughContainer()
{
	Timer t("IterateThroughContainer");
	const static int N = 50000;
	typedef U32 T; 

	{
		Timer t("Null");
		for (int i = 0; i < N; i++)
			g_test2 = rand();
	}

	{
		Set<T> set;
		for (int i = 0; i < N; i++)
			set.Insert(rand());

		Timer t("Btl::Set");
		for (int i = 0; i < N; i++)
			for (Set<T>::Iterator it = set.Begin(); it != set.End(); ++it)
				g_test2 = it->GetValue();
	}

	{
		std::set<T> set;
		for (int i = 0; i < N; i++)
			set.insert(rand());

		Timer t("std::set");
		for (int i = 0; i < N; i++)
			for (std::set<T>::iterator it = set.begin(); it != set.end(); ++it)
				g_test2 = *it;
	}

	{
		SkipListSet<T> set;
		for (int i = 0; i < N; i++)
			set.Insert(rand());

		Timer t("Btl::SkipListSet");
		for (int i = 0; i < N; i++)
			for (SkipListSet<T>::Iterator it = set.Begin(); it != set.End(); ++it)
				g_test2 = it->GetValue();
	}

	{
		typedef OpenHashSet<U32, HashKeyOpDefault<U32> > OpenHashSetType;
		OpenHashSetType openhashset;

		for (int i = 0; i < N; i++)
			openhashset.Insert(rand());

		Timer t("Btl::OpenHashSet");
		for (int i = 0; i < N; i++)
			for (OpenHashSetType::Iterator it = openhashset.Begin(); it != openhashset.End(); ++it)
				g_test2 = it->GetValue();
	}

	{
		ChainedHashSet<T, HashKeyOpDefault<T> > set;
		for (int i = 0; i < N; i++)
			set.Insert(rand());

		Timer t("Btl::ChainedHashSet");
		for (int i = 0; i < N; i++)
			for (ChainedHashSet<T, HashKeyOpDefault<T> >::Iterator it = set.Begin(); it != set.End(); ++it)
				g_test2 = it->GetValue();
	}

	{
		typedef ChainedHashSet<T, HashKeyOpDefault<T>, DynamicPool<NullType> > SetType;
		SetType set;
		for (int i = 0; i < N; i++)
			set.Insert(rand());

		SetType::Iterator end = set.End();

		Timer t("Btl::ChainedHashSet (pool)");
		for (int i = 0; i < N; i++)
			for (SetType::Iterator it = set.Begin(); it != set.End(); ++it)
				g_test2 = it->GetValue();
	}

	{
		stdext::hash_set<T> set;
		for (int i = 0; i < N; i++)
			set.insert(rand());

		Timer t("std::hash_set");
		for (int i = 0; i < N; i++)
			for (stdext::hash_set<T>::iterator it = set.begin(); it != set.end(); ++it)
				g_test2 = *it;
	}
}

class Btl_Performance_Tests : public CxxTest::TestSuite
{
public:
	void testInsertElements()
	{
		InsertElements();
	}

	void testInsertAndEraseElements()
	{
		InsertAndEraseElements();
	}

	void testSearchForElements()
	{
		SearchForElements();
	}

	void testIterateThroughContainer()
	{
		IterateThroughContainer();
	}
};

