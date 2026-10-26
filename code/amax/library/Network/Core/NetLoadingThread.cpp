#include "stdafx.h"
#include "NetServices.h"	// [Blur2PC]
#include "NetLoadingThread.h"

namespace RacerNet
{

C_NetLoadingThread::C_NetLoadingThread() 
: m_Active( false )
{
}

C_NetLoadingThread::~C_NetLoadingThread()
{
	Leave();
}


void C_NetLoadingThread::Enter(void)
{
	if (!IsActive())
	{
		m_Active = true;
		Start(0, 951, "NetLoadingThread");
	}
}

void C_NetLoadingThread::Leave(void)
{
	if (IsActive())
	{
		m_Event.Signal();
		Join();
#if defined( COMPILER_VC )
		Close();
#endif
		m_Active = false;
	}
}

void C_NetLoadingThread::ThreadMain(void)
{
	while (1)
	{
		if (m_Event.Wait(16) == false)
		{
			// pump network smeg to keep connections alive.
			// critical section stops this running at the same time as the call in C_RaceBookLayer::Update() although
			// don't reference any network structures externally while this thread is active. 
			RacerNet::C_NetServices::Instance().Poll();
		}
		else
			break; //allow thread to terminate
	}
}

}; // namespace RacerNet