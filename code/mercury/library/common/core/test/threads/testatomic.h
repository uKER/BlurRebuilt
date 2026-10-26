#include <core/core.h>
#include <core/threads.h>
#include <cxxtest/TestSuite.h>
#include <core/threads/atomic.h>

class AtomicTest : public CxxTest::TestSuite
{
public:
	U32 iValue1, iValue2;

	void setUp()
	{
		iValue1 = 10;
		iValue2 = 20;
	}

	void testAtomicCompareAndSwap()
	{
		TS_ASSERT(MT::AtomicCompareAndSwap(&iValue1, 20, 10));
		TS_ASSERT_EQUALS(iValue1, 20);
	}

	void testAtomicCompareAndSwap2()
	{
		TS_ASSERT(!MT::AtomicCompareAndSwap(&iValue1, 10, 20));
		TS_ASSERT_EQUALS(iValue1, 10);
	}

	void testAtomicAdd()
	{
		int iResult = MT::AtomicAdd(&iValue1, 5);
		TS_ASSERT_EQUALS(iResult, 10);
		TS_ASSERT_EQUALS(iValue1, 10+5);
	}
	
	void testAtomicIncrement()
	{
		int iResult = MT::AtomicIncrement((int*)&iValue1);
		TS_ASSERT_EQUALS(iResult, 10);
		TS_ASSERT_EQUALS(iValue1, 10+1);
	}

	void testAtomicDecrement()
	{
		int iResult = MT::AtomicDecrement((int*)&iValue1);
		TS_ASSERT_EQUALS(iResult, 10);
		TS_ASSERT_EQUALS(iValue1, 10-1);
	}

	void testAtomicAnd()
	{
		int iResult = MT::AtomicAnd(&iValue1, 8);
		TS_ASSERT_EQUALS(iResult, 10);
		TS_ASSERT_EQUALS(iValue1, 10&8);
	}

	void testAtomicOr()
	{
		int iResult = MT::AtomicOr(&iValue1, 32);
		TS_ASSERT_EQUALS(iResult, 10);
		TS_ASSERT_EQUALS(iValue1, 10|32);
	}

	void testAtomicGet()
	{
		int iResult = MT::AtomicGet(&iValue1);
		TS_ASSERT_EQUALS(iResult, 10);
	}

	void testAtomicSet()
	{
		MT::AtomicSet(&iValue1, 20);
		TS_ASSERT_EQUALS(iValue1, 20);
	}
};

