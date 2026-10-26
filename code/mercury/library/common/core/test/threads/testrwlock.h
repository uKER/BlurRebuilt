#include <core/core.h>
#include <core/threads.h>
#include <cxxtest/TestSuite.h>
#include <stdlib.h>
#include <core/threads/rwlock.h>

struct RwLockTestStruct
{
	bool m_bQuit;
	int m_iValue1, m_iValue2;
	int m_iSwaps;
	bool m_bDoSimpleTest;
	MT::RwLock m_RwLock;
};

class RwLockTestThread : public MT::Thread
{
public:
	RwLockTestStruct *m_pTestStruct;
	int m_iThread;

	void ThreadMain()
	{
		if (m_pTestStruct->m_bDoSimpleTest)
		{
			TS_ASSERT(!m_pTestStruct->m_RwLock.ReadLock(10));
			return;
		}

		char buffer[512];
		MT::Thread::Sleep(10);
		while (!m_pTestStruct->m_bQuit)
		{
			if ((rand()&7)<5)
			{
				// try a read
				if (!m_pTestStruct->m_RwLock.ReadLock(1000))
				{
					sprintf(buffer, "Thread %d failed to acquire readlock after waiting for 1 second.", m_iThread);
					TS_FAIL(buffer);
					break;
				}
				int iValue1 = m_pTestStruct->m_iValue1;
				int iValue2 = m_pTestStruct->m_iValue2;
				MT::Thread::Sleep(1);
				if (iValue1 != m_pTestStruct->m_iValue1 ||
					iValue2 != m_pTestStruct->m_iValue2)
				{
					sprintf(buffer, "Thread %d values changed while readlock was acquired.", m_iThread);
					TS_FAIL(buffer);
					break;
				}
				m_pTestStruct->m_RwLock.ReleaseReadLock();
			}
			else
			{
				// do a swap (write)
				if (!m_pTestStruct->m_RwLock.WriteLock(1000))
				{
					sprintf(buffer, "Thread %d failed to acquire writelock after waiting for 1 second.", m_iThread);
					TS_FAIL(buffer);
					break;
				}
				m_pTestStruct->m_iSwaps++;
				int iTmp = m_pTestStruct->m_iValue1;
				m_pTestStruct->m_iValue1 = m_pTestStruct->m_iValue2;
				m_pTestStruct->m_iValue2 = iTmp;
				m_pTestStruct->m_RwLock.ReleaseWriteLock();
			}
		}
	}
};

class RwLockTest : public CxxTest::TestSuite
{
public:
	static const int m_iNumIterations = 20;
	static const int m_nThreads = 20;

	RwLockTestStruct m_test;

	void testSimple()
	{
		m_test.m_bDoSimpleTest = true;
		
		TS_ASSERT(m_test.m_RwLock.ReadLock(10));
		TS_ASSERT(m_test.m_RwLock.ReadLock(10));
		TS_ASSERT(!m_test.m_RwLock.WriteLock(10));
		m_test.m_RwLock.ReleaseReadLock();
		TS_ASSERT(!m_test.m_RwLock.WriteLock(10));
		m_test.m_RwLock.ReleaseReadLock();
		TS_ASSERT(m_test.m_RwLock.WriteLock(10));
		RwLockTestThread thread;
		thread.m_pTestStruct = &m_test;
		thread.Start();
		thread.Join();
		m_test.m_RwLock.ReleaseWriteLock();
		TS_ASSERT(m_test.m_RwLock.ReadLock(10));
		m_test.m_RwLock.ReleaseReadLock();
	}

	void testThreads()
	{
		RwLockTestThread thread[m_nThreads];
		m_test.m_iValue1 = 10;
		m_test.m_iValue2 = 20;
		m_test.m_iSwaps = 0;
		m_test.m_bQuit = false;
		m_test.m_bDoSimpleTest = false;
		for (int i=0; i<m_nThreads; i++)
		{
			thread[i].m_pTestStruct = &m_test;
			thread[i].m_iThread = i;
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
};

