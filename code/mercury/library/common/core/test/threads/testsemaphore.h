#include <core/core.h>
#include <core/threads.h>
#include <cxxtest/TestSuite.h>
#include <core/threads/atomic.h>

struct SemaphoreTestStruct
{
	bool m_bQuit;
	int m_iSwaps;
	MT::Semaphore m_Semaphore;
};

class SemaphoreTestThread : public MT::Thread
{
public:
	SemaphoreTestStruct *m_pTestStruct;
	int m_iThreadID;

	void ThreadMain()
	{
		while (m_pTestStruct->m_Semaphore.Wait(1000))
		{
			MT::AtomicIncrement(&m_pTestStruct->m_iSwaps);
			MT::Thread::Sleep(1);
		}
	}
};

class SemaphoreTest : public CxxTest::TestSuite
{
public:
	static const int m_iNumIterations = 20;
	static const int m_nThreads = 20;

	SemaphoreTestStruct m_test;

	void testSingle()
	{
		SemaphoreTestThread thread[m_nThreads];
		m_test.m_iSwaps = 0;
		m_test.m_bQuit = false;
		m_test.m_Semaphore.Release(100);
		for (int i=0; i<m_nThreads; i++)
		{
			thread[i].m_pTestStruct = &m_test;
			thread[i].m_iThreadID = i;
			thread[i].Start();
		}
		MT::Thread::Sleep(50);
		for (int iOuter=0; iOuter<20; iOuter++)
		{
			m_test.m_Semaphore.Release(100);
			MT::Thread::Sleep(1);
			MT::AtomicAdd((U32*)&m_test.m_iSwaps, -100);
			MT::Thread::Sleep(9);
		}
		for (int i=0; i<m_nThreads; i++)
			thread[i].Join();

		if (m_test.m_Semaphore.Wait(10))
		{
			TS_FAIL("Semaphore count is not 0 after all threads have exited.\n");
		}

		TS_ASSERT_EQUALS(100, m_test.m_iSwaps);
	}
};

