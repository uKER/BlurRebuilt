#include "test/btl/test.h"
#include <cxxtest/TestSuite.h>
#include "btl/utility/scopedpointer.h"

using namespace Btl;

class ScopedPointerTest
{
public:
	~ScopedPointerTest()
	{
		//int breakhere = 0;
	}
};

class Btl_ScopedPointer_Tests : public CxxTest::TestSuite
{
public:
	void testScopedPointer()
	{
		ScopedPointer<ScopedPointerTest> p1;
		assert(Pointer(p1) == 0);
		Allocate(p1);

		//ScopedPointer<Test> p2 = p1; // not allowed
	}
};

