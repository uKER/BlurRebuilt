#include "test/btl/test.h"
#include <cxxtest/TestSuite.h>

#include "btl/allocators/stackallocator.h"

using namespace Btl;

class Btl_StackAllocator_Tests : public CxxTest::TestSuite
{
public:
	void testStackAllocator()
	{
		StackAllocatorSimple<> stackAllocator;

		// fixed size buffer which needs to be initialised
		stackAllocator.Initialise(12);

		U32* p1 = stackAllocator.Allocate<U32>();
		U32* p2 = stackAllocator.Allocate<U32>();

		// Freeing only calls the destructors.
		// Memory is not released until the allocator is destroyed.
		stackAllocator.Free(p1);
		stackAllocator.Free(p2);
	}
};
