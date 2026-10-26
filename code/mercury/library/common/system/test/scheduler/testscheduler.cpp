#include <core/core.h>
#include <system/scheduler.h>
#include <system/debuglog.h>
#include <cxxtest/TestSuite.h>
#include <core/threads.h>

const int NUM_GROUPS = 7;
const int NUM_TASKS = 256;

Scheduler::GroupHandle Groups[NUM_GROUPS];

struct TaskData
{
	int* NResultsPtr;
	int* ResultsPtr;
};

#ifndef PLATFORM_SPU
static Debug::Logger &g_Log = Debug::GetLogger("samples.schedulertest");
#endif

void OrderTask(Scheduler::Task* pTask)
{
	TaskData* bd = (TaskData*)pTask->ParamData;

	int i = MT::AtomicIncrement(bd->NResultsPtr);
	LOGDEBUG( g_Log, "processing job %d\n", (U32)pTask->Input);

	Spu::Set((U32*)bd->ResultsPtr + i, (U32)pTask->Input);
}

JQ_DECLARE_SPU_MODULE(OrderModule, OrderTask, testscheduler, "samples.schedulertest");

#ifndef PLATFORM_SPU

void Init()
{
	Scheduler::Init();
	Scheduler::Start();

	Scheduler::SetGroupPoolSize(NUM_GROUPS);
	for (int i = 0; i < NUM_GROUPS; i++)
		Groups[i] = Scheduler::CreateGroup();

	// 0-1 Single dependency.
	Scheduler::AddGroupDependency(Groups[1], Groups[0]);

	// 2-5 Multiple dependencies.
	Scheduler::AddGroupDependency(Groups[6], Groups[2]);
	Scheduler::AddGroupDependency(Groups[6], Groups[3]);
	Scheduler::AddGroupDependency(Groups[6], Groups[4]);
	Scheduler::AddGroupDependency(Groups[6], Groups[5]);
}

class SchedulerTest : public CxxTest::TestSuite
{
public:
	TaskData bd;
	int NResults;
	int Results[NUM_TASKS];
	int hPolicy;

	void setUp()
	{
		bd.NResultsPtr = &NResults;
		bd.ResultsPtr = Results;
		NResults = 0; memset(Results, 0xcd, sizeof(Results));

		Scheduler::Policy piMed[] = { 7, 7, 7, 7, 7, 7, 7, 7 };
		static Scheduler::PolicyHandle _hPolicy = Scheduler::CreatePolicy(piMed);
		hPolicy = _hPolicy;

		Scheduler::Init();
		Scheduler::Start();

		Scheduler::SetGroupPoolSize(NUM_GROUPS);
		for (int i = 0; i < NUM_GROUPS; i++)
			Groups[i] = Scheduler::CreateGroup();

		// 0-1 Single dependency.
		Scheduler::AddGroupDependency(Groups[1], Groups[0]);

		// 2-5 Multiple dependencies.
		Scheduler::AddGroupDependency(Groups[6], Groups[2]);
		Scheduler::AddGroupDependency(Groups[6], Groups[3]);
		Scheduler::AddGroupDependency(Groups[6], Groups[4]);
		Scheduler::AddGroupDependency(Groups[6], Groups[5]);
	}

	void tearDown()
	{
		Scheduler::Shutdown();
	}

	void testScheduler1()
	{
		// Test 1 - Single dependency.
		Scheduler::AddTask(OrderModule, (void*)0, NULL, Groups[0], hPolicy, &bd, sizeof(bd));
		Scheduler::AddTask(OrderModule, (void*)1, NULL, Groups[1], hPolicy, &bd, sizeof(bd));
		Scheduler::Flush(Groups[1]);

		TS_ASSERT_EQUALS(NResults, 2); // Not enough tasks executed.  Check that dependencies are working properly.
		TS_ASSERT_EQUALS(Results[0], 0);
		TS_ASSERT_EQUALS(Results[1], 1); // Taskes executed out of order.  Check that dependencies are working properly.
	}

	void testScheduler2()
	{
		// Test 2 - Multiple dependency.
		Scheduler::AddTask(OrderModule, (void*)2, NULL, Groups[2], hPolicy, &bd, sizeof(bd));
		Scheduler::AddTask(OrderModule, (void*)3, NULL, Groups[3], hPolicy, &bd, sizeof(bd));
		Scheduler::AddTask(OrderModule, (void*)4, NULL, Groups[4], hPolicy, &bd, sizeof(bd));
		Scheduler::AddTask(OrderModule, (void*)5, NULL, Groups[5], hPolicy, &bd, sizeof(bd));
		Scheduler::AddTask(OrderModule, (void*)6, NULL, Groups[6], hPolicy, &bd, sizeof(bd));
		Scheduler::Flush(Groups[6]);

		TS_ASSERT_EQUALS(NResults, 5); // Not enough tasks executed.  Check that dependencies are working properly.
		TS_ASSERT_EQUALS(Results[4], 6); // Dependency executed after dependent.  Check that dependencies are working properly.
	}

	void testScheduler3()
	{
		// Test 3 - Priority order.
		Scheduler::AddTask(OrderModule, (void*)0, NULL, Groups[0], hPolicy, &bd, sizeof(bd));
		Scheduler::AddTask(OrderModule, (void*)1, NULL, Groups[0], hPolicy, &bd, sizeof(bd));
		Scheduler::AddTask(OrderModule, (void*)2, NULL, Groups[0], hPolicy, &bd, sizeof(bd));
		Scheduler::Flush(Groups[0]);

		TS_ASSERT_EQUALS(NResults, 3); // Not enough tasks executed.  Check that priorities are working properly.
		//// Disabled - JQ is picking up the tasks as they are added, need a way to pause all workers.
		//TS_ASSERT(Results[0] == 2 && Results[1] == 1 && Results[2] == 0); // Priority order failed.  Check that priorities are working properly.
	}

	void testScheduler4()
	{
		// Test 4 - Group specific flush.
		for (int i = 0; i < 100; i++)
			Scheduler::AddTask(OrderModule, (void*)(i&1), NULL, Groups[i&1], hPolicy, &bd, sizeof(bd));
		Scheduler::Flush(Groups[0]);
		int NGroup0 = 0;
		for (int i = 0; i < NResults; i++) 
			NGroup0 += (Results[i] == 0)?1:0;
		TS_ASSERT_EQUALS(NGroup0, 50); // Not enough tasks executed.  Check that jqFlush on specific groups is working.
	}

	void testScheduler5()
	{
		// Test 5 - Global flush.
		for (int i = 0; i < 100; i++)
			Scheduler::AddTask(OrderModule, (void*)(i&1), NULL, Groups[i&1], hPolicy, &bd, sizeof(bd));

		TS_ASSERT_EQUALS(Scheduler::Poll(Groups[0]), false);
		TS_ASSERT_EQUALS(Scheduler::Poll(Groups[1]), false);

		Scheduler::Flush();
		int NGroup0 = 0;
		for (int i = 0; i < NResults; i++) 
			NGroup0 += (Results[i] == 0)?1:0;
		TS_ASSERT_EQUALS(NGroup0, 50); // Not enough tasks executed.  Check that global jqFlush is working.
		TS_ASSERT_EQUALS(Scheduler::Poll(Groups[0]), true);
		TS_ASSERT_EQUALS(Scheduler::Poll(Groups[1]), true);

	}
};

#endif

