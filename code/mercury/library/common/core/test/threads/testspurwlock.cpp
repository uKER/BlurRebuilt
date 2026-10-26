#include <core/core.h>
#include <core/threads.h>
#include <cxxtest/TestSuite.h>
#include <core/threads/SpuRwLock.h>
#include <system/scheduler.h>

struct SpuRwLockResultsStruct
{
	int iValue1, iValue2, iSwaps, iSuccess;
};

struct SpuRwLockTestStruct
{
	MT::SpuRwLock m_SpuRwLock;
	SpuRwLockResultsStruct *pResults;
	bool m_bDoSimpleTest;
};

void SpuRwLockTestFunc(Scheduler::Task* pTask)
{
	SpuRwLockTestStruct *pTestStruct = (SpuRwLockTestStruct*)pTask->ParamData;

	SpuRwLockResultsStruct tmp;
	tmp.iSuccess = 1;
	if (!pTestStruct->m_bDoSimpleTest)
	{
		// try to acquire the read-lock, return the result in iSuccess
		tmp.iSuccess = pTestStruct->m_SpuRwLock.ReadLock(10)?1:0;
		if (tmp.iSuccess)
			pTestStruct->m_SpuRwLock.ReleaseReadLock();
	}

	Spu::Store(pTestStruct->pResults, &tmp, sizeof(tmp));
}

JQ_DECLARE_DUAL_MODULE(SpuRwLockTestModule, SpuRwLockTestFunc, testspurwlock, "core.test")

#ifndef PLATFORM_SPU

class SpuRwLockTest : public CxxTest::TestSuite
{
public:
	static const int NUM_GROUPS = 7;
	static const int NUM_TASKS = 256;

	static const int m_iNumIterations = 20;
	static const int m_nTasks = 20;
	int hPolicyPPU, hPolicySPU;

	Scheduler::GroupHandle m_Groups[NUM_GROUPS];
	SpuRwLockTestStruct *m_pData;

	void setUp()
	{
		Scheduler::Policy piSPU[] = { 0, 0, 7, 7, 7, 7, 7, 7 };
		Scheduler::Policy piPPU[] = { 0, 7, 0, 0, 0, 0, 0, 0 };
		static Scheduler::PolicyHandle _hPolicySPU = Scheduler::CreatePolicy(piSPU);
		hPolicySPU = _hPolicySPU;
		static Scheduler::PolicyHandle _hPolicyPPU = Scheduler::CreatePolicy(piPPU);
		hPolicyPPU = _hPolicyPPU;

		Scheduler::Init();
		Scheduler::Start();

		Scheduler::SetGroupPoolSize(2);
		m_Groups[0] = Scheduler::CreateGroup();
		m_Groups[1] = Scheduler::CreateGroup();

		m_pData = new SpuRwLockTestStruct;
		m_pData->pResults = new SpuRwLockResultsStruct;
		m_pData->pResults->iSuccess = 2;

		Scheduler::AddGroupDependency(m_Groups[1], &m_pData->m_SpuRwLock);
		m_pData->m_bDoSimpleTest = false;
	}

	void tearDown()
	{
		delete m_pData->pResults;
		delete m_pData;
		Scheduler::Shutdown();
	}

	void testSimpleSPU() { _Simple(hPolicySPU); }
	void testSimplePPU() { _Simple(hPolicyPPU); }

	void _Simple(int iPolicy)
	{
		// attempt to get the read-lock without a write-lock, should succeed
		m_pData->pResults->iSuccess = 2;
		TS_ASSERT_EQUALS(m_pData->pResults->iSuccess, 2);
		Scheduler::AddTask(SpuRwLockTestModule, (void*)0, NULL, m_Groups[0], iPolicy, m_pData, sizeof(*m_pData));
		Scheduler::Flush();
		TS_ASSERT_EQUALS(m_pData->pResults->iSuccess, 1);
	}

	void testReadLockPPU() { _ReadLock(hPolicyPPU); }
	void testReadLockSPU() { _ReadLock(hPolicySPU); }

	void _ReadLock(int iPolicy)
	{
		// attempt to get the read-lock on the SPU during a write-lock, should fail
		TS_ASSERT(m_pData->m_SpuRwLock.WriteLock(10));

		m_pData->pResults->iSuccess = 2;
		TS_ASSERT_EQUALS(m_pData->pResults->iSuccess, 2);
		Scheduler::AddTask(SpuRwLockTestModule, (void*)0, NULL, m_Groups[0], iPolicy, m_pData, sizeof(*m_pData));
		Scheduler::Flush();
		TS_ASSERT_EQUALS(m_pData->pResults->iSuccess, 0);

		m_pData->m_SpuRwLock.ReleaseWriteLock();
	}

	void testReadLockGroupDependencyPPU() { _ReadLockGroupDependency(hPolicyPPU); }
	void testReadLockGroupDependencySPU() { _ReadLockGroupDependency(hPolicySPU); }

	void _ReadLockGroupDependency(int iPolicy)
	{
		// test the scheduler built-in group dependency feature. the task shouldn't get kicked off as long
		// as the write-lock has been acquired
		m_pData->m_bDoSimpleTest = true;

		// run this three times to make sure the lock release works properly
		for (int i=0; i<3; i++)
		{
			TS_ASSERT(m_pData->m_SpuRwLock.WriteLock(10));

			m_pData->pResults->iSuccess = 2;
			TS_ASSERT_EQUALS(m_pData->pResults->iSuccess, 2);
			Scheduler::AddTask(SpuRwLockTestModule, (void*)0, NULL, m_Groups[1], iPolicy, m_pData, sizeof(*m_pData));
			MT::Thread::Sleep(1000);							// wait a second for the task to run
			TS_ASSERT_EQUALS(m_pData->pResults->iSuccess, 2);	// task shouldn't have run because of the write-lock

			m_pData->m_SpuRwLock.ReleaseWriteLock();			// release lock

			Scheduler::Flush();
			TS_ASSERT_EQUALS(m_pData->pResults->iSuccess, 1);	// task should now have completed
		}
	}
};

#endif
