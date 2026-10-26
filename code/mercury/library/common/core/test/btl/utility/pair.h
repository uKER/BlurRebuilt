#include "test/btl/test.h"
#include "btl/utility/pair.h"
#include "test/btl/archive.h"
#include <cxxtest/TestSuite.h>

using namespace Btl;

class Btl_Pair_Tests : public CxxTest::TestSuite
{
public:
	void testPair()
	{
		typedef Pair<U32, U16> Pair;
		Pair pair;

		pair.first = 0;
		pair.second = 1;

		Pair pair2(pair);

		assert(pair2 == pair);

		Pair pair3 = MakePair(U32(1), U16(2));

		assert(pair3.first == 1 && pair3.second == 2);
	}

};
