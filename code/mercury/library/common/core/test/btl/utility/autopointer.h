#include "test/btl/test.h"
#include <cxxtest/TestSuite.h>
#include "btl/utility/autopointer.h"

using namespace Btl;

class Test
{
public:
	virtual ~Test()
	{
		int breakhere = 0;
	}
};

class Test2 : public Test
{
public:
	~Test2()
	{
		int breakhere = 0;
	}
};

class MyTestSuite : public CxxTest::TestSuite
{
public:
	void testAssign1()
	{
		AutoPointer<Test> p1;
		Allocate(p1);

		AutoPointer<Test> p2 = p1;

		TS_ASSERT(Pointer(p1) == 0 && Pointer(p2) != 0);
	}

	void testAssign2()
	{
		AutoPointer<Test> p1;
		AutoPointer<Test2> p2;

		Allocate(p2);
		p1 = p2;

		TS_ASSERT(p1 != 0);
		TS_ASSERT(p2 == 0);

		Free(p1); // calls Test2 destructor
		TS_ASSERT(p1 == 0);
	}

	void testCopy()
	{
		AutoPointer<Test> p1;
		Allocate(p1);
		AutoPointer<Test> p2(p1);
		TS_ASSERT(p1 == 0);
		TS_ASSERT(p2 != 0);
	}

	AutoPointer<Test> Source()
	{
		AutoPointer<Test> p;
		Allocate(p);
		return p;
	}

	void Sink(AutoPointer<Test> p)
	{
	}

	void testSourceSink()
	{
		AutoPointer<Test> p = Source();
		TS_ASSERT(p != 0);

		AutoPointer<Test> p2(Source());
		TS_ASSERT(p2 != 0);

		Sink(p);
		TS_ASSERT(p == 0);

		Allocate(p);

		const AutoPointer<Test> c = p;
		//Sink(c); // can't copy const AutoPointer
	}
};

