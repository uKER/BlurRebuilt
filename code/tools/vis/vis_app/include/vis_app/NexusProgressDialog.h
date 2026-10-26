#pragma once
#include "afxcmn.h"

class NexusProgressDialog : public CDialog, SRD::IProgress, AftkProgressNotification
{
	DECLARE_DYNAMIC(NexusProgressDialog)

public:
	NexusProgressDialog(CWnd* pParent = NULL);   // standard constructor

	virtual ~NexusProgressDialog();

	virtual BOOL OnInitDialog();

	UINT_PTR m_nTimer;

	virtual void OnThreadFinished();
	//virtual void UpdateProgressBar(char *psz, float f);
	virtual BOOL DestroyWindow();
	afx_msg void OnTimer(UINT nIDEvent);

	void WaitForThreadToFinish();

	void SetTaskCallbackProc(Tu::Status (*pCallbackFunc)(void *pThis, void* pArgument));

	void SetTaskArgument(void* pArgument);

	bool NexusProgressDialog::Cancelled() const;

//	bool(*)(void*, void*) TaskCallbackProc();

	Tu::Status TaskResult() const;

	void SetTaskResult(Tu::Status result);

	void SetTitle(const std::string& text);

// Dialog Data
	enum { IDD = IDD_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//Returning false will eventually abort the current operation. For the moment this is however
	//not implemented.
	bool OnProgress(F32 percent);

	//Callback function for printing output to the user while they wait.
	void OnPrintProgress(const std::string& text);

	DECLARE_MESSAGE_MAP()

	virtual void OnCancel();

	virtual void OnBnClickedCancel();

	bool	m_bCancel;
	float	m_fProgress;
	CString	m_szProgress;

	HANDLE	m_hThread;
	char	m_pszProgress[512];

	static DWORD CALLBACK WorkThreadStartProc(void* pThis);
	
	static void ProgressCallbackProc(void *pThis, float fProgress, char *pszDescription);

	void SetTaskCallback(Tu::Status (*pTaskCallbackFunc)(void *pThis, void* pArgument));
	
	Tu::Status (*m_pTaskCallbackProc)(void *pThis, void* pArgument);

private:
	static Tu::Status m_taskResult;
	static void*	m_pTaskArgument;

	CProgressCtrl	m_progress;
	CStatic			m_static;
};
