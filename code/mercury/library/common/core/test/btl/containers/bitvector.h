#include "test/btl/test.h"
#include "btl/containers/bitvector.h"
#include <cxxtest/TestSuite.h>

using namespace Btl;

static bool g_test;

class Btl_BitVector_Tests : public CxxTest::TestSuite
{
public:
	void testBitVector()
	{
		BitVector32 b;
		b.Resize(32);
		b.ClearBits();
		b[16] = 1;
		assert(b[16] == 1);
		assert(b[15] == 0);

		bool test = b.AnyBitsSet();
		assert(test);
		b.SetBits();
		assert(b[10] == 1);
	}

	void testBitVector2()
	{
		BitVector32 b;
		b.Resize(37);
		b.ClearBits();
		b[1] = 1;
		b[4] = 1;
		b[30] = 1;
		b[32] = 1;
		b[34] = 1;

		int index = 0;

		for (BitVector32::Iterator i = b.Begin(); i != b.End(); ++i, index++)
		{
			bool set = (index == 1 || index == 4 || index == 30 || index == 32 || index == 34);
			g_test = *i;

			if (index == 20)
			{
				*i = 1;
				assert(*i == 1);
			}

			if (index == 4)
			{
				*i = 0;
				assert(*i == 0);
			}

			assert(g_test == set);
		}

		index = 0;

		for (BitVector32::ConstIterator i = b.Begin(); i != b.End(); ++i, index++)
		{
			bool set = (index == 1 || index == 30 || index == 32 || index == 34 || index == 20);
			g_test = *i;
			assert(g_test == set);
		}
	}
};

