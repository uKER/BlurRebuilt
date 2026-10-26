// Nexus.h : main header file for the Nexus application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// NexusApp:
// See Nexus.cpp for the implementation of this class
//

class NexusApp : public CWinApp
{
public:
	NexusApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileImport();
	#ifdef NEXUS_IDLE_PROCESS
	virtual BOOL OnIdle(LONG lCount);
	#endif
};

extern NexusApp theApp;