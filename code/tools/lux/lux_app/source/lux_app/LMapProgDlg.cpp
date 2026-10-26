// LMapProgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "lux_app/LMapProgDlg.h"
#include "..\..\include\lux_app\lmapprogdlg.h"


// CLMapProgDlg dialog

IMPLEMENT_DYNAMIC(CLMapProgDlg, CDialog)
CLMapProgDlg::CLMapProgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLMapProgDlg::IDD, pParent)
	, m_szProgress(_T(""))
{
}

CLMapProgDlg::~CLMapProgDlg()
{
}

void CLMapProgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_TotalProgress);
	DDX_Text(pDX, IDC_PROGRESS_STRING, m_szProgress);
}


BEGIN_MESSAGE_MAP(CLMapProgDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, OnCancelButtonClicked)
END_MESSAGE_MAP()


// CLMapProgDlg message handlers

BOOL CLMapProgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_TotalProgress.SetRange(0,10000);
	m_TotalProgress.SetStep(100);

	m_nTimer = SetTimer(1, 200, 0);
	m_bCancel			= false;
	m_fProgress			= 0;

	m_TotalProgress.SetPos((int)(m_fProgress*100));

	// start the lightmapping thread
	m_hThread = CreateThread(0, 0, LightMapperThreadStartProc, this, 0, 0);	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CLMapProgDlg::OnThreadFinished()
{
	if (m_hWnd) // check that the window isn't already closed
		SendMessage(WM_CLOSE);
}

void CLMapProgDlg::UpdateProgressBar(char *psz, float f)
{
	m_TotalProgress.SetPos((int)(f*10000));
}

BOOL CLMapProgDlg::DestroyWindow()
{
	KillTimer(m_nTimer);

	return __super::DestroyWindow();
}

void CLMapProgDlg::OnTimer(UINT nIDEvent)
{
	m_TotalProgress.SetPos((int)(m_fProgress*100));

	// check whether the thread has finished
	if (WaitForSingleObject(m_hThread, 0) != WAIT_TIMEOUT)
	{
		// it has finished - exit the dialog box
		OnThreadFinished();
	}

	__super::OnTimer(nIDEvent);
}

void CLMapProgDlg::OnCancelButtonClicked()
{
	// send a message to the thread to make it cancel itself
	m_bCancel = true;

	// close the dialog
	__super::OnCancel();
}

// main lightmapping thread entry point
DWORD CALLBACK CLMapProgDlg::LightMapperThreadStartProc(void* Parameter)
{
	LPVS::LPVSCORE.SetProgressCallBackProc(Parameter, ProgressCallbackProc);

	LPVS::LPVSCORE.LightScene(0);
//	((C_Network*)Parameter)->CompletionThread();
	// TODO: get some kind of progress callback function

	return 0;
}

// progress update callback
bool CLMapProgDlg::ProgressCallbackProc(void *pData, float fProgress, char *pszDescription)
{
	CLMapProgDlg *pThis = (CLMapProgDlg*)pData;

	if (pThis->m_bCancel)
		return true;

	pThis->m_fProgress = fProgress;
	pThis->SendDlgItemMessage(IDC_PROGRESS_STRING, WM_SETTEXT, 0, (LPARAM)pszDescription);

	return false;
}

// wait for thread to end
void CLMapProgDlg::WaitForThreadToFinish()
{
	WaitForSingleObject(m_hThread, INFINITE);
}
