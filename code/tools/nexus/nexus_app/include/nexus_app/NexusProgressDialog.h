#pragma once
#include "afxcmn.h"

namespace Nexus
{
	class NexusProgressDialog : 
		public CDialog, 
		public Nexus::Progress
	{
		DECLARE_DYNAMIC(NexusProgressDialog)

	public:
		NexusProgressDialog(CWnd* pParent = NULL);   // standard constructor

		virtual ~NexusProgressDialog();

		Tu::Status ExecuteTask(
			Tu::Status (*pTaskCallbackFunc)(void *pThis, const Nexus::StringArray& arguments),
			const Nexus::StringArray& arguments = Nexus::StringArray());

		BOOL OnInitDialog();

		UINT_PTR m_nTimer;

		void OnThreadFinished();

		BOOL DestroyWindow();

		afx_msg void OnTimer(UINT nIDEvent);

		void WaitForThreadToFinish();

		bool NexusProgressDialog::Cancelled() const;

		void SetTitle(const std::string& text);

	// Dialog Data
		enum { IDD = IDD_PROGRESS };

	protected:
		void OnPreExecuteTask();

		void OnPostExecuteTask();

		void SetTaskCallbackProc(Tu::Status (*pCallbackFunc)(void *pThis, void* pArgument));

		void SetTaskArgument(void* pArgument);

		void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		//Returning false will eventually abort the current operation. For the moment this is however
		//not implemented.
		bool OnProgress(F32 percent);

		//Callback function for printing output to the user while they wait.
		void OnPrintProgress(const std::string& text);

		DECLARE_MESSAGE_MAP()

		void OnCancel();

		void OnBnClickedCancel();

		CString	m_szProgress;

		char	m_pszProgress[512];

		static DWORD CALLBACK WorkThreadStartProc(void* pThis);
		
		static void ProgressCallbackProc(void *pThis, float fProgress, char *pszDescription);

		void SetTaskCallback(Tu::Status (*pTaskCallbackFunc)(void *pThis, void* pArgument));
		
		Tu::Status (*m_pTaskCallbackProc)(void *pThis, void* pArgument);

	private:
		static void*	m_pTaskArgument;

		CProgressCtrl	m_progressCtrl;
		CStatic			m_static;
	public:
		afx_msg void OnClose();
	};
}
