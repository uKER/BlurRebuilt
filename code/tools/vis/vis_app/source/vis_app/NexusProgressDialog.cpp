#include <vis_app/stdafx.h>

#include <vis_app/NexusRender.h>

#include "../../resource.h"
#include "..\..\include\vis_app\NexusProgressDialog.h"

IMPLEMENT_DYNAMIC(NexusProgressDialog, CDialog)

Tu::Status NexusProgressDialog::m_taskResult = Tu::Status::FAILURE;
void* NexusProgressDialog::m_pTaskArgument = 0;

NexusProgressDialog::NexusProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(NexusProgressDialog::IDD, pParent)
{
	m_taskResult = Tu::Status::FAILURE;
}

NexusProgressDialog::~NexusProgressDialog()
{
}

void NexusProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_progress);
	DDX_Control(pDX, IDC_PROGESS_TEXT, m_static);
}

BEGIN_MESSAGE_MAP(NexusProgressDialog, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL NexusProgressDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_progress.SetRange(0,100);
	m_progress.SetStep(1);

	m_nTimer = SetTimer(1, 200, 0);
	m_bCancel			= false;
	m_fProgress			= 0;

//	m_TotalProgress.SetPos((int)(m_fProgress*100));

	// start the lightmapping thread
	m_hThread = CreateThread(0, 0, WorkThreadStartProc, this, 0, 0);	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void NexusProgressDialog::OnThreadFinished()
{
	if (m_hWnd) // check that the window isn't already closed
		SendMessage(WM_CLOSE);
}

BOOL NexusProgressDialog::DestroyWindow()
{
	KillTimer(m_nTimer);

	return __super::DestroyWindow();
}

void NexusProgressDialog::OnTimer(UINT nIDEvent)
{
	m_progress.SetPos((int)(m_fProgress));
	m_static.SetWindowText(m_szProgress);

	// check whether the thread has finished
	if (WaitForSingleObject(m_hThread, 0) != WAIT_TIMEOUT)
	{
		// it has finished - exit the dialog box
		OnThreadFinished();
	}

	__super::OnTimer(nIDEvent);
}

void NexusProgressDialog::OnCancel()
{
	// send a message to the thread to make it cancel itself
	m_bCancel = true;

	// close the dialog
	__super::OnCancel();
}

void NexusProgressDialog::OnBnClickedCancel()
{
//	assert(false);
//	m_taskResult = false;
	OnCancel();
}

bool NexusProgressDialog::Cancelled() const
{
	return m_bCancel;
}

// main lightmapping thread entry point
DWORD CALLBACK NexusProgressDialog::WorkThreadStartProc(void* pThis)
{
	NexusProgressDialog *pThisDialog = (NexusProgressDialog*)pThis;
	m_taskResult = pThisDialog->m_pTaskCallbackProc(pThis, m_pTaskArgument);

	return 0;
}

// progress update callback
void NexusProgressDialog::ProgressCallbackProc(void *pThis, float fProgress, char *pszDescription)
{
	NexusProgressDialog *pThisDialog = (NexusProgressDialog*)pThis;
	pThisDialog->m_fProgress = fProgress;
//	pThis->SendDlgItemMessage(IDC_PROGRESS_STRING, WM_SETTEXT, 0, (LPARAM)pszDescription);
}
/*
bool (*m_pTaskCallbackProc)(void *pThis, void* pArgument) NexusProgressDialog::TaskCallbackProc()
{
	return m_pTaskCallbackProc;
}
*/
// wait for thread to end
void NexusProgressDialog::WaitForThreadToFinish()
{
	WaitForSingleObject(m_hThread, INFINITE);
}

void NexusProgressDialog::SetTaskCallback(Tu::Status (*pTaskCallbackFunc)(void *pThis, void* pArgument))
{
	m_pTaskCallbackProc = pTaskCallbackFunc;
}

void NexusProgressDialog::SetTaskArgument(void* pArgument)
{
	m_pTaskArgument = pArgument;
}

Tu::Status NexusProgressDialog::TaskResult() const
{
	return m_taskResult;
}

void NexusProgressDialog::SetTaskCallbackProc(Tu::Status (*pCallbackProc)(void *pThis, void* pArgument))
{
	m_pTaskCallbackProc = pCallbackProc;
}
/*
bool(*)(void*, void*) NexusProgressDialog::TaskCallbackProc()
{
	return m_pTaskCallbackProc;
}
*/

void NexusProgressDialog::SetTaskResult(Tu::Status result)
{
	m_taskResult = result;
}

void NexusProgressDialog::SetTitle(const std::string& text)
{
	SetWindowText(text.c_str());
}

//Returning false will eventually abort the current operation. For the moment this is however
//not implemented.
bool NexusProgressDialog::OnProgress(F32 percent)
{
	//If we're starting a new progress bar...
	if (percent < 0.0001f) 
	{
		m_taskResult = Tu::Status::FAILURE;
	}

	m_fProgress = percent;

	return m_bCancel;
}

//Callback function for printing output to the user while they wait.
void NexusProgressDialog::OnPrintProgress(const std::string& text)
{
	m_szProgress = text.c_str();
}