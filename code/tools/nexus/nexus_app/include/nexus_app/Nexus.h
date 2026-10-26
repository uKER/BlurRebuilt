#ifndef NEXUS_NEXUS_H
#define NEXUS_NEXUS_H

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


namespace Nexus
{
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
		virtual int ExitInstance();
	};

	extern NexusApp g_theApp;
}

#endif
