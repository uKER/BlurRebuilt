#include <core/core.h>
#include <core/threads.h>
#include <cxxtest/TestSuite.h>
#include <core/threads/atomicmutex.h>
#include <system/scheduler.h>

struct AtomicLockResultsStruct
{
	int iValue1, iValue2, iSwaps, iFail;
};

struct AtomicLockTestStruct
{
	AtomicLockResultsStruct *pResults;
	MT::AtomicLock m_AtomicLock;
};

void AtomicLockTestFunc(Scheduler::Task* pTask)
{
	AtomicLockTestStruct *pTestStruct = (AtomicLockTestStruct*)pTask->ParamData;

	bool bFirst = pTestStruct->m_AtomicLock.Wait(1000);
	pTestStruct->m_AtomicLock.Release();

	if (!bFirst || !pTestStruct->m_AtomicLock.Wait(1000))
	{
		// Failed to acquire AtomicLock after waiting for 1 second
		AtomicLockResultsStruct failResults;
		failResults.iFail = 1;
		Spu::Store(pTestStruct->pResults, &failResults, sizeof(failResults));
		return;
	}

	AtomicLockResultsStruct *pResults = pTestStruct->pResults;
	Spu::FetchAlloc(pResults);
	pResults->iSwaps++;
	int iTmp = pResults->iValue1;
	pResults->iValue1 = pResults->iValue2;
	pResults->iValue2 = iTmp;
	Spu::Store(pTestStruct->pResults, pResults, sizeof(*pResults));
	pTestStruct->m_AtomicLock.Release();
}

JQ_DECLARE_SPU_MODULE(AtomicLockTestModule, AtomicLockTestFunc, testatomicmutex, "core.test")

#ifndef PLATFORM_SPU

class AtomicLockTest : public CxxTest::TestSuite
{
public:
	static const int NUM_GROUPS = 7;
	static const int NUM_TASKS = 256;

	static const int m_iNumIterations = 20;
	static const int m_nTasks = 20;
	int hPolicy;

	AtomicLockTestStruct m_test;
	Scheduler::GroupHandle m_Groups[NUM_GROUPS];

	void setUp()
	{
		Scheduler::Policy piMed[] = { 0, 0, 7, 7, 7, 7, 7, 7 };
		static Scheduler::PolicyHandle _hPolicy = Scheduler::CreatePolicy(piMed);
		hPolicy = _hPolicy;

		Scheduler::Init();
		Scheduler::Start();

		Scheduler::SetGroupPoolSize(1);
		m_Groups[0] = Scheduler::CreateGroup();
	}

	void tearDown()
	{
		Scheduler::Shutdown();
	}

	void testAtomicLock()
	{
		AtomicLockTestStruct *pTestStruct = new AtomicLockTestStruct;

		pTestStruct->pResults = new AtomicLockResultsStruct;
		pTestStruct->pResults->iValue1 = 10;
		pTestStruct->pResults->iValue2 = 20;
		pTestStruct->pResults->iSwaps = 0;
		pTestStruct->pResults->iFail = 0;
		for (int i=0; i<m_nTasks; i++)
		{
			Scheduler::AddTask(AtomicLockTestModule, (void*)0, NULL, m_Groups[0], hPolicy, pTestStruct, sizeof(*pTestStruct));
		}
		Scheduler::Flush();
		TS_ASSERT_EQUALS(pTestStruct->pResults->iFail, 0);
		TS_ASSERT_EQUALS(pTestStruct->pResults->iSwaps, m_nTasks);
		if (pTestStruct->pResults->iSwaps&1)
		{
			TS_ASSERT_EQUALS(pTestStruct->pResults->iValue1, 20);
			TS_ASSERT_EQUALS(pTestStruct->pResults->iValue2, 10);
		}
		else
		{
			TS_ASSERT_EQUALS(pTestStruct->pResults->iValue1, 10);
			TS_ASSERT_EQUALS(pTestStruct->pResults->iValue2, 20);
		}

		delete pTestStruct->pResults;
		delete pTestStruct;
	}

	void testAtomicLockRelease()
	{
		MT::AtomicLock AtomicLock;
		TS_ASSERT(AtomicLock.Wait());
		TS_ASSERT(AtomicLock.Wait(1000));
		TS_ASSERT(AtomicLock.Release());
		TS_ASSERT(AtomicLock.Release());
	}
};

#endif
