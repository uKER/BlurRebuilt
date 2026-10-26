// UIMaker2.h : main header file for the UIMaker2 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CUIMaker2App:
// See UIMaker2.cpp for the implementation of this class
//

class CUIMaker2App : public CWinApp
{
public:
	CUIMaker2App();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppLang();
	afx_msg void OnAppAbout();
	afx_msg void OnConfig();
	DECLARE_MESSAGE_MAP()
};

extern CUIMaker2App theApp;