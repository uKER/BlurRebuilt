#include "test/btl/test.h"
#include "btl/containers/list.h"
#include "btl/containers/algorithm.h"
#include "test/btl/archive.h"
#include <list>
#include <cxxtest/TestSuite.h>

using namespace Btl;

struct ListTest;
List<ListTest, ListParameters_Internal<> > g_list; // list of undefined type

struct ListTest : public ListElementSingle<ListTest>
{
	ListTest(int i) : m_i(i) {} 
	int m_i;
};

class Btl_List_Tests : public CxxTest::TestSuite
{
public:
	// external list
	void testExternalList()
	{
		List<U32> list;
		list.PushBack(5);
		list.PushFront(1);
		list.InsertAfter(2, list.Begin());
		list.InsertBefore(4, list.ReverseBegin().Iterator());
		list.InsertAfter(3, Find(list, U32(2)));
		assert(list.Size() == 5);

		list.Swap(list.Begin(), list.Begin());
		list.Swap(list.Begin(), ++list.Begin());
		assert(list.Front() == 2);
		list.Swap(list.Begin(), ++list.Begin());

		U32 count = 1;

		for (List<U32>::Iterator i = list.Begin(); i != list.End(); ++i, ++count)
			assert(*i == count);

		list.PopFront();
		list.PopBack();
		list.Erase(Find(list, U32(3)));
		assert(list.Size() == 2);
		list.Clear();
		assert(list.Size() == 0);
		assert(list.Begin() == 0);
	}

	// internal list
	void testInternalList()
	{
		List<ListTest> list;
		ListTest t0(0);
		ListTest t1(1);
		ListTest t2(2);

		//list.PushBack(); // doesn't work with internal links
		list.PushBack(t2);
		list.PushFront(t0);
		list.InsertAfter(t1, list.Begin());
		assert(list.Size() == 3);

		int count = 0;

		for (List<ListTest>::Iterator i = list.Begin(); i != list.End(); ++i, ++count)
			assert(i->m_i == count);

		list.Clear();
		assert(list.Size() == 0);
		assert(list.Empty());
		assert(list.Begin() == 0);

		//List<ListTest> list2 = list; won't compile for internal lists
	}

	// simple list
	void testSimpleList()
	{
		typedef List<U32, ListParameters<SingleEndedList, OnDemandCounter, ListElementSingle> > SimpleList;
		SimpleList list;
		//list.PushBack(5); // not supported
		list.PushFront(1);
		list.InsertAfter(2, list.Begin());
		//list.InsertBefore(4, list.ReverseBegin().Iterator()); // not supported
		list.InsertAfter(3, Find(list, U32(2)));
		assert(list.Size() == 3);

		U32 count = 1;

		for (SimpleList::Iterator i = list.Begin(); i != list.End(); ++i, ++count)
			assert(*i == count);

		list.PopFront();
		//list.PopBack(); // not supported
		//list.Erase(Find(list, 3)); // not supported
		list.EraseNext(Find(list, U32(2)));
		assert(list.Size() == 1);
		list.Clear();
		assert(list.Size() == 0);
		assert(list.Begin() == 0);
	}

	void testReverse()
	{
		List<U32> list;
		list.PushBack(5);
		list.PushBack(4);
		list.PushBack(3);
		list.PushBack(2);
		list.PushBack(1);

		list.Reverse();

		U32 count = 1;

		for (List<U32>::Iterator i = list.Begin(); i != list.End(); ++i, ++count)
			assert(*i == count);
	}

	void testSort()
	{
		List<U32> list;
		list.PushBack(1);
		list.PushBack(4);
		list.PushBack(7);
		list.PushBack(9);
		list.PushBack(2);
		list.PushBack(6);
		list.PushBack(8);
		list.PushBack(5);
		list.PushBack(10);
		list.PushBack(3);

		list.Sort();

		U32 count = 1;

		for (List<U32>::Iterator i = list.Begin(); i != list.End(); ++i, ++count)
			assert(*i == count);
	}

	/*{
		std::list<U32> list;

		for (int i = 0; i < 1000; i++)
			list.push_back(rand());

		list.sort();

		int breakhere = 0;
	}*/

	void testSort2()
	{
		List<U32> list;

		for (int i = 0; i < 1000; i++)
			list.PushBack(rand());

		list.Sort();

		//int breakhere = 0;

		/*for (List<U32>::Iterator i = list.Begin(); i != list.End(); ++i)
		{
			char buffer[128];
			_itoa_s(*i, buffer, 128, 10);
			OutputDebugStringA(buffer);
			OutputDebugStringA("\n");
		}*/
	}

	void testSort3()
	{
		typedef List<U32, ListParameters_Pool<> > ListPool;
		ListPool list;

		for (int i = 0; i < 1000; i++)
			list.PushBack(rand());

		list.Sort();

		/*for (StandardList::Iterator i = list.Begin(); i != list.End(); ++i)
		{
			char buffer[128];
			_itoa_s(*i, buffer, 128, 10);
			OutputDebugStringA(buffer);
			OutputDebugStringA("\n");
		}*/
	}

	void testCopy()
	{
		List<U32> l1, l2;
		l1.PushBack(0);
		l1.PushBack(1);
		l2.PushBack(2);
		l2 = l1;
		l1.Clear();
		assert(l1.Empty());
		assert(l2.Size() == 2);
	}

};
