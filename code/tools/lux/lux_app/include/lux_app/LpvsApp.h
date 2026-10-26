// lux_app.h : main header file for the lux_app application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "../../resource.h"       // main symbols


// LpvsApp:
// See lux_app.cpp for the implementation of this class
//

class LpvsApp : public CWinApp
{
public:
	LpvsApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern LpvsApp theApp;