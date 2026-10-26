#include "test/btl/test.h"
#include <cxxtest/TestSuite.h>
#include "btl/allocators/pool.h"

using namespace Btl;

class Btl_Pool_Tests : public CxxTest::TestSuite
{
public:
	void testStaticPool()
	{
		StaticPool<U32, 10> pool;

		assert(pool.Remaining() == 10);

		U32* p1 = pool.Allocate();
		U32* p2 = pool.Allocate();
		U32* p3 = pool.Allocate();

		*p1 = 1;
		*p2 = 2;
		*p3 = 3;

		assert(pool.Allocated() == 3);

		pool.Free(p1);
		pool.Free(p2);
		pool.Free(p3);

		assert(pool.Allocated() == 0);
	}

	void testDynamicPool()
	{
		DynamicPool<U32> pool;

		assert(pool.Remaining() == 0);

		U32* p1 = pool.Allocate();
		U32* p2 = pool.Allocate();
		U32* p3 = pool.Allocate();

		*p1 = 1;
		*p2 = 2;
		*p3 = 3;

		assert(pool.Allocated() == 3);

		pool.Free(p1);
		pool.Free(p2);
		pool.Free(p3);

		assert(pool.Allocated() == 0);
	}
};

