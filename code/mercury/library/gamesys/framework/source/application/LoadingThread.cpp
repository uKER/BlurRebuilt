#include "FrameworkPch.h"
#include "Framework/Application/LoadingThread.h"
#include "framework/application/ApplicationBase.h"

namespace Framework
{

//! Constructor.
C_LoadingThread::C_LoadingThread( C_ApplicationBase* pApplication )
	: m_pApplication(pApplication)
	, m_bLoadingScreenActive(false)
{
}

//! Destructor.
C_LoadingThread::~C_LoadingThread()
{

}

void C_LoadingThread::SetLoadingScreenActive(bool state)
{
	m_bLoadingScreenActive = state;
}

void C_LoadingThread::ThreadMain()
{

	bool shouldQuit = false;
	while(!shouldQuit)
	{
		if(!m_bLoadingScreenActive)
		{
			m_EventProcessMessage.Wait();
			switch(m_Message)
			{
				case E_LoadingThreadMessage_Quit:
					shouldQuit = true;
					SetLoadingScreenActive(false);
				break;

				case E_LoadingThreadMessage_ActivateThread:
					SetLoadingScreenActive(true);
				break;
			}
			m_EventProcessMessageComplete.Signal();
		}
		else
		{
			if(m_EventProcessMessage.Wait(0))
			{
				switch(m_Message)
				{
					case E_LoadingThreadMessage_Quit:
						shouldQuit = true;
						SetLoadingScreenActive(false);
					break;

					case E_LoadingThreadMessage_DeactivateThread:
						SetLoadingScreenActive(false);
					break;
				}

				m_EventProcessMessageComplete.Signal();
			}
		}

		if(m_bLoadingScreenActive)
		{
			m_pApplication->ProcessLoadingScreen();
		}
	}
}

// Switch loading screen thread on and off. This should block the main thread only.
void C_LoadingThread::SyncLoadingScreen( bool bLoadingScreenActive )
{

	if(bLoadingScreenActive)
	{
		if(!m_bLoadingScreenActive)
		{
			// signal the loading screen thread to start rendering and block until it has begun
			m_Message = E_LoadingThreadMessage_ActivateThread;
			m_EventProcessMessage.Signal();
			m_EventProcessMessageComplete.Wait();
		}
	}
	else
	{
		if(m_bLoadingScreenActive)
		{
			// signal the loading screen thread to stop and block until it has
			m_Message = E_LoadingThreadMessage_DeactivateThread;
			m_EventProcessMessage.Signal();
			m_EventProcessMessageComplete.Wait();
		}
	}
}

// signal the loading thread to quit and wait for the thread to exit
void C_LoadingThread::SendQuitMessage()
{
	m_Message = E_LoadingThreadMessage_Quit;
	m_EventProcessMessage.Signal();
	m_EventProcessMessageComplete.Wait();

	Join();
}

}; // namespace Framework
