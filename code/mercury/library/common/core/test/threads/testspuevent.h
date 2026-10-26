#include <core/core.h>
#include <core/threads.h>
#include <system/scheduler.h>
#include <core/threads/spuevent.h>
#include <cxxtest/TestSuite.h>
#include <system/debuglog.h>

struct SpuEventTestStruct
{
	bool m_bQuit;
	int m_iValue1, m_iValue2;
	int m_iSwaps;
	MT::SpuEvent m_EventSwap;
};

class SpuEventTestThread : public MT::Thread
{
public:
	SpuEventTestStruct *m_pTestStruct;
	int m_iThreadID;

	void ThreadMain()
	{
		while (!m_pTestStruct->m_bQuit)
		{
			if (!m_pTestStruct->m_EventSwap.Wait(1000))
			{
				TS_FAIL("Failed to receive swap event waiting for 1 second.");
				break;
			}
			m_pTestStruct->m_iSwaps++;
			int iTmp = m_pTestStruct->m_iValue1;
			m_pTestStruct->m_iValue1 = m_pTestStruct->m_iValue2;
			m_pTestStruct->m_iValue2 = iTmp;
			m_pTestStruct->m_EventSwap.Signal();
			MT::Thread::Sleep(10);
		}
	}
};

class SpuEventTest : public CxxTest::TestSuite
{
public:
	static const int m_iNumIterations = 20;
	static const int m_nThreads = 20;

	SpuEventTestStruct *m_ptest;

	void setUp()
	{
		Scheduler::Policy piMed[] = { 0, 0, 7, 7, 7, 7, 7, 7 };
		static Scheduler::PolicyHandle _hPolicy = Scheduler::CreatePolicy(piMed);

		Scheduler::Init();
		Scheduler::Start();

		m_ptest = new SpuEventTestStruct;

		Scheduler::SetGroupPoolSize(2);
	}

	void tearDown()
	{
		delete m_ptest;
		Scheduler::Shutdown();
	}

	void testSimple()
	{
		MT::Event ev;

		// signal once
		TS_ASSERT(!ev.Wait(10));
		ev.Signal();
		TS_ASSERT(ev.Wait(10));

		// signal twice
		TS_ASSERT(!ev.Wait(10));
		ev.Signal();
		ev.Signal();
		TS_ASSERT(ev.Wait(10));
		TS_ASSERT(!ev.Wait(10));
	}

	void testThread()
	{
		SpuEventTestThread thread[m_nThreads];
		m_ptest->m_iValue1 = 10;
		m_ptest->m_iValue2 = 20;
		m_ptest->m_iSwaps = 0;
		m_ptest->m_bQuit = false;
		for (int i=0; i<m_nThreads; i++)
		{
			thread[i].m_pTestStruct = m_ptest;
			thread[i].m_iThreadID = i;
			thread[i].Start();
		}
		MT::Thread::Sleep(500);
		TS_ASSERT_EQUALS(m_ptest->m_iSwaps, 0);
		m_ptest->m_EventSwap.Signal();
		MT::Thread::Sleep(1500);
		m_ptest->m_bQuit = true;
		for (int i=0; i<m_nThreads; i++)
			thread[i].Join();

		if (!m_ptest->m_EventSwap.Wait(1000))
			TS_FAIL("Failed to receive final swap event waiting for 1 second.");

		TS_ASSERT_LESS_THAN(0,m_ptest->m_iSwaps);

		if (m_ptest->m_iSwaps&1)
		{
			TS_ASSERT_EQUALS(m_ptest->m_iValue1, 20);
			TS_ASSERT_EQUALS(m_ptest->m_iValue2, 10);
		}
		else
		{
			TS_ASSERT_EQUALS(m_ptest->m_iValue1, 10);
			TS_ASSERT_EQUALS(m_ptest->m_iValue2, 20);
		}
	}
};

