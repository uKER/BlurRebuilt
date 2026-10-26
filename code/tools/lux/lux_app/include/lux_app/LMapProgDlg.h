#pragma once
#include "afxcmn.h"
#include "../../lux_app\resource.h"

// CLMapProgDlg dialog
#include "CustomControls\ProgressPercentComplete.h"

class CLMapProgDlg : public CDialog
{
	DECLARE_DYNAMIC(CLMapProgDlg)

public:
	CLMapProgDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLMapProgDlg();
	void WaitForThreadToFinish();

// Dialog Data
	enum { IDD = IDD_LMAPPROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// total rendering progress
	TProgressPercentComplete m_TotalProgress;
	virtual BOOL OnInitDialog();

	UINT_PTR m_nTimer;

	virtual void OnThreadFinished();
	virtual void UpdateProgressBar(char *psz, float f);
	virtual BOOL DestroyWindow();
	afx_msg void OnTimer(UINT nIDEvent);

	bool	m_bCancel;
protected:

	float	m_fProgress;
	HANDLE	m_hThread;
	char	m_pszProgress[512];

	static DWORD CALLBACK LightMapperThreadStartProc(void* Parameter);
	static bool ProgressCallbackProc(void *pThis, float fProgress, char *pszDescription);
public:
	CString m_szProgress;
	afx_msg void OnCancelButtonClicked();
};
