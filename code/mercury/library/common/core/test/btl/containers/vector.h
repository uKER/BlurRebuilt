#include "test/btl/test.h"
#include "btl/containers/vector.h"
#include "btl/containers/algorithm.h"
#include <algorithm>
#include <iterator>
#include "test/btl/archive.h"
#include "btl/containers/radix.h"
#include <cxxtest/TestSuite.h>

using namespace Btl;

class Btl_Vector_Tests : public CxxTest::TestSuite
{
public:
	void testVector()
	{
		Vector<U32, VectorParameters_Array<10> > v;
		assert(v.Size() == 10 && v.Capacity() == 10);

		for (int i = 0; i < v.Size(); i++)
			v[i] = i;

		Vector<U32, VectorParameters_Array<10> > v2;

		v2 = v;
		Swap(v, v2);
	}

	void testVectorStatic()
	{
		Vector<U32, VectorParameters_Static<10> > v;
		assert(v.Size() == 0 && v.Capacity() == 10);

		for (int i = 0; i < v.Capacity(); i++)
		{
			v.PushBack();
			v[i] = i;
		}

		assert(v.Size() == 10);

		while (v.Size() > 0)
		{
			v.PopBack();
		}

		assert(v.Size() == 0);
	}

	void testVectorMisc()
	{
		Vector<U32> v;
		assert(v.Size() == 0 && v.Capacity() == 0);

		for (int i = 0; i < 100; i++)
		{
			v.PushBack();
			v[i] = i;
		}

		assert(v.Size() == 100);

		Vector<U32>::Iterator it;
		it = Find(v, 7);
		assert(*it == 7);

		Remove(v, 7);
		RemoveNonOrdered(v, 10);
		
		it = Find(v, 7);
		assert(it == v.End());
		it = Find(v, 10);
		assert(it == v.End());

		Vector<U32> v2;

		v2 = v;

		for (Vector<U32>::Iterator i = v.Begin(); i < v.End(); i++)
			*i = 0;

		Swap(v, v2);

		while (v.Size() > 0)
		{
			v.PopBack();
		}

		assert(v.Size() == 0);
	}

	void testVectorSort()
	{
		Vector<U32> v;

		for (int i = 0; i < 1000; i++)
			v.PushBack(rand());

		std::sort(v.Begin(), v.End());

		//int breakhere = 0;
	}

	void testVectorPushBack()
	{
		Vector<U32> v1, v2;
		v1.PushBack(0);
		v1.PushBack(1);
		v2.PushBack(2);
		v2 = v1;
		v1.Clear();
		assert(v1.Empty());
		assert(v2.Size() == 2);
	}

	void testVectorRadixSort()
	{
		Vector<U32> v;
		v.PushBack(1);
		v.PushBack(4);
		v.PushBack(7);
		v.PushBack(9);
		v.PushBack(2);
		v.PushBack(6);
		v.PushBack(8);
		v.PushBack(5);
		v.PushBack(10);
		v.PushBack(3);

		RadixSort(v);

		int count = 1;

		for (int i = 0; i < v.Size(); ++i, ++count)
			assert(v[i] == count);
	}
};

