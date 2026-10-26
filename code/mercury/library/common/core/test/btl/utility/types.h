#include "test/btl/test.h"
#include "btl/utility/types.h"
#include <cxxtest/TestSuite.h>

using namespace Btl;

class Base
{
};

class Derived : public Base
{
};

class Btl_Types_Tests : public CxxTest::TestSuite
{
public:
	// types
	void testTypes()
	{
		if (!SameType<U32, U32>::result)
			assert(0);

		if (SameType<U32, U16>::result)
			assert(0);
	}

	// alignment
	void testAlignment()
	{
		int a1 = BTL_ALIGNMENT_OF(U32);
		int a2 = BTL_ALIGNMENT_OF(U16);
		int a3 = BTL_ALIGNMENT_OF(U8);

		assert(a1 == 4);
		assert(a2 == 2);
		assert(a3 == 1);

		SameSizeAndAlignment<U32> testAlignment;

		int s = sizeof(testAlignment);
		int a = BTL_ALIGNMENT_OF(SameSizeAndAlignment<U32>);
		
		assert(s == 4 && a == 4);
	}

	// swap
	void testSwap()
	{
		int a = 0;
		int b = 1;

		Swap(a, b);

		assert(a == 1 && b == 0);
	}

	// cast
	void testCast()
	{
		Derived d;
		Base* b1 = StaticCast<Derived, Base>(&d);
		assert(b1 != 0);

		U32 i;
		Base* b2 = StaticCast<U32, Base>(&i);
		assert(b2 == 0);
	}
};
