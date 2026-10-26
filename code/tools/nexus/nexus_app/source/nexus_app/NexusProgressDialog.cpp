#include <nexus_app/stdafx.h>

#include <nexus_app/DisplayManager.h>

#include "../../resource.h"
#include "..\..\include\nexus_app\NexusProgressDialog.h"

using namespace Nexus;

IMPLEMENT_DYNAMIC(NexusProgressDialog, CDialog)

void* Nexus::NexusProgressDialog::m_pTaskArgument = 0;

NexusProgressDialog::NexusProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(NexusProgressDialog::IDD, pParent)
{
	m_cancelled = false;
	m_taskResult = Tu::Status::FAILURE;
}

NexusProgressDialog::~NexusProgressDialog()
{
}

void NexusProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_progressCtrl);
	DDX_Control(pDX, IDC_PROGESS_TEXT, m_static);
}

BEGIN_MESSAGE_MAP(NexusProgressDialog, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

Tu::Status NexusProgressDialog::ExecuteTask(
	Tu::Status (*pTaskCallbackFunc)(void *pThis, const Nexus::StringArray& arguments),
	const Nexus::StringArray& arguments)
{
	Tu::Status status;
	OnPreExecuteTask();
	SetTaskCallbackFunc(pTaskCallbackFunc);
	SetTaskArguments(arguments);
	DoModal();
	WaitForThreadToFinish();
	OnPostExecuteTask();

	return TaskResult();
}

BOOL NexusProgressDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_progressCtrl.SetRange(0,100);
	m_progressCtrl.SetStep(1);
	m_nTimer = SetTimer(1, 200, 0);

	StartThread();
//	m_hThread = CreateThread(0, 0, WorkThreadStartProc, this, 0, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void NexusProgressDialog::OnThreadFinished()
{
	// check that the window isn't already closed
	if (m_hWnd) 
	{
		SendMessage(WM_CLOSE);
		EndDialog(0);
	}
}

BOOL NexusProgressDialog::DestroyWindow()
{
	KillTimer(m_nTimer);
	return __super::DestroyWindow();
}

void NexusProgressDialog::OnTimer(UINT nIDEvent)
{
	m_progressCtrl.SetPos((int)(m_progress));
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
	m_cancelled = true;

	// close the dialog
	__super::OnCancel();
}

void NexusProgressDialog::OnBnClickedCancel()
{
	m_taskResult = Tu::Status::SUCCESS;
	m_cancelled = true;
	OnCancel();
}

// main lightmapping thread entry point
DWORD CALLBACK NexusProgressDialog::WorkThreadStartProc(void* pThis)
{
	//Make sure we've got a task to execute.
	AFTK_ASSERT(m_pTaskArgument);

	NexusProgressDialog *pThisDialog = (NexusProgressDialog*)pThis;
	m_taskResult = pThisDialog->m_pTaskCallbackProc(pThis, m_pTaskArgument);

	return 0;
}

// progress update callback
void NexusProgressDialog::ProgressCallbackProc(void *pThis, float progress, char *pszDescription)
{
	NexusProgressDialog *pThisDialog = (NexusProgressDialog*)pThis;
	pThisDialog->m_progress = progress;
}

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

void NexusProgressDialog::SetTaskCallbackProc(Tu::Status (*pCallbackProc)(void *pThis, void* pArgument))
{
	m_pTaskCallbackProc = pCallbackProc;
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

	m_progress = percent;
	return m_cancelled;
}

//Callback function for printing output to the user while they wait.
void NexusProgressDialog::OnPrintProgress(const std::string& text)
{
	m_szProgress = text.c_str();
}

void NexusProgressDialog::OnPreExecuteTask()
{
}

void NexusProgressDialog::OnPostExecuteTask()
{
}



void NexusProgressDialog::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	__super::OnClose();
}
