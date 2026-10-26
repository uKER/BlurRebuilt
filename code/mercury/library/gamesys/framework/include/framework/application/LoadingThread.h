#ifndef FRAMEWORK_LOADING_THREAD_H
#define FRAMEWORK_LOADING_THREAD_H

#ifdef _MSC_VER
#pragma once
#endif

#include "core/threads/Mutex.h"

namespace Framework
{

class C_ApplicationBase;

enum E_LoadingThreadMessage
{
	E_LoadingThreadMessage_ActivateThread = 0,
	E_LoadingThreadMessage_DeactivateThread,
	E_LoadingThreadMessage_Quit
};

//! C_LoadingThread

class C_LoadingThread : public MT::Thread
{
public:

	//! Constructor.
	C_LoadingThread( C_ApplicationBase* pApplication );

	//! Destructor.
	virtual ~C_LoadingThread();
		
	void ThreadMain();

	void SyncLoadingScreen( bool bLoadingScreenActive );

	void SendQuitMessage();

	bool IsLoadingScreenActive()  const { return m_bLoadingScreenActive; }

private:
	void SetLoadingScreenActive(bool state);

private:
	MT::Event				m_EventProcessMessage;
	MT::Event				m_EventProcessMessageComplete;
	E_LoadingThreadMessage	m_Message;

	C_ApplicationBase*		m_pApplication;
	bool					m_bLoadingScreenActive;
};

}; // namespace Framework

#endif // FRAMEWORK_LOADING_THREAD_H
