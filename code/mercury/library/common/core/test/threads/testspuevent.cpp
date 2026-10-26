#include <core/core.h>
#include <core/threads.h>
#include <system/scheduler.h>
#include <core/threads/spuevent.h>
#include <cxxtest/TestSuite.h>
#include <system/debuglog.h>

const int NUM_GROUPS = 7;
const int NUM_TASKS = 256;

::Scheduler::GroupHandle Groups[NUM_GROUPS];

struct TaskData
{
	MT::SpuEvent m_Event;
	int* ResultsPtr;
};

#ifndef PLATFORM_SPU
static Debug::Logger &g_Log = Debug::GetLogger("samples.schedulertest");
#endif

void SpuEventTestTask(Scheduler::Task* pTask)
{
	TaskData *pData = (TaskData*)pTask->ParamData;
	// set result to 1, then signal
	Spu::Set((U32*)pData->ResultsPtr, (U32)1);
#ifdef PLATFORM_SPU
	pData->m_Event.Signal();
#endif
}

JQ_DECLARE_SPU_MODULE(SpuEventTestModule, SpuEventTestTask, testspuevent, "core.test")

#ifndef PLATFORM_SPU

class SchedulerTest : public CxxTest::TestSuite
{
public:
	TaskData *pData;
	int NResults;
	int Results[NUM_TASKS];
	int hPolicy;

	void setUp()
	{
		Scheduler::Policy piMed[] = { 0, 0, 7, 7, 7, 7, 7, 7 };
		static Scheduler::PolicyHandle _hPolicy = Scheduler::CreatePolicy(piMed);
		hPolicy = _hPolicy;

		Scheduler::Init();
		Scheduler::Start();

		pData = new TaskData;
		pData->ResultsPtr = Results;
		memset(Results, 0, sizeof(Results));

		Scheduler::SetGroupPoolSize(5);
		Groups[0] = Scheduler::CreateGroup();
	}

	void tearDown()
	{
		delete pData;
		Scheduler::Shutdown();
	}

	void testEventConstructorAndDestructor()
	{
		MT::SpuEvent test;
	}

	void testSpuEvent()
	{
		TS_ASSERT_EQUALS(Results[0], 0);
		Scheduler::AddTask(SpuEventTestModule, (void*)0, NULL, Groups[0], hPolicy, pData, sizeof(*pData));
		bool bResult = pData->m_Event.Wait();
		TS_ASSERT_EQUALS(bResult, true);
		TS_ASSERT_EQUALS(Results[0], 1);	// by the time the event is signalled, the result should be set to 1
		Scheduler::Flush();

		// test running a task using event-driven group flush
		Scheduler::GroupHandle h = Scheduler::CreateGroup(true);
		Scheduler::AddTask(SpuEventTestModule, (void*)0, NULL, h, hPolicy, pData, sizeof(*pData));
		Scheduler::Flush(h);
		Scheduler::Flush(h);
		Scheduler::DestroyGroup(h);
	}
};

#endif

