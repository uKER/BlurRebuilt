#include <core/core.h>
#include <core/threads.h>
#include <cxxtest/TestSuite.h>

struct MutexTestStruct
{
	bool m_bQuit;
	int m_iValue1, m_iValue2;
	int m_iSwaps;
	MT::Mutex m_Mutex;
};

class MutexTestThread : public MT::Thread
{
public:
	MutexTestStruct *m_pTestStruct;

	void ThreadMain()
	{
		while (!m_pTestStruct->m_bQuit)
		{
			if (!m_pTestStruct->m_Mutex.Wait(1000))
			{
				TS_FAIL("Failed to acquire mutex after waiting for 1 second.");
				break;
			}
			m_pTestStruct->m_iSwaps++;
			int iTmp = m_pTestStruct->m_iValue1;
			m_pTestStruct->m_iValue1 = m_pTestStruct->m_iValue2;
			m_pTestStruct->m_iValue2 = iTmp;
			m_pTestStruct->m_Mutex.Release();
		}
	}
};

class MutexTest : public CxxTest::TestSuite
{
public:
	static const int m_iNumIterations = 20;
	static const int m_nThreads = 20;

	MutexTestStruct m_test;

	void testMutex()
	{
		MutexTestThread thread[m_nThreads];
		m_test.m_iValue1 = 10;
		m_test.m_iValue2 = 20;
		m_test.m_iSwaps = 0;
		m_test.m_bQuit = false;
		for (int i=0; i<m_nThreads; i++)
		{
			thread[i].m_pTestStruct = &m_test;
			thread[i].Start();
		}
		MT::Thread::Sleep(2000);
		m_test.m_bQuit = true;
		for (int i=0; i<m_nThreads; i++)
			thread[i].Join();
		if (m_test.m_iSwaps&1)
		{
			TS_ASSERT_EQUALS(m_test.m_iValue1, 20);
			TS_ASSERT_EQUALS(m_test.m_iValue2, 10);
		}
		else
		{
			TS_ASSERT_EQUALS(m_test.m_iValue1, 10);
			TS_ASSERT_EQUALS(m_test.m_iValue2, 20);
		}
	}

	void testMutexRelease()
	{
		MT::Mutex mutex;
		TS_ASSERT(mutex.Wait());
		TS_ASSERT(mutex.Wait(10));
		TS_ASSERT(mutex.Release());
		TS_ASSERT(mutex.Release());
	}
};

