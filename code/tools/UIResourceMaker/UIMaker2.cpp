// UIMaker2.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "UIMaker2.h"
#include "MainFrm.h"

#include "UIMaker2Doc.h"
#include "UIMaker2View.h"

#include "afxcmn.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUIMaker2App

BEGIN_MESSAGE_MAP(CUIMaker2App, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)

	ON_COMMAND(ID_BUTTON_CONFIG, OnConfig)
	ON_COMMAND(ID_BUTTON_LANG, OnAppLang)
END_MESSAGE_MAP()


// CUIMaker2App construction

CUIMaker2App::CUIMaker2App()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CUIMaker2App object

CUIMaker2App theApp;

// CUIMaker2App initialization

BOOL CUIMaker2App::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CUIMaker2Doc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CUIMaker2View));
	AddDocTemplate(pDocTemplate);
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;	
	ParseCommandLine(cmdInfo);
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:

};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CUIMaker2App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

class CConfigDlg : public CDialog
{
public:
	CConfigDlg();

	enum { IDD = ID_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
public:
	CString m_StringUIMakerDir;
	CString m_StringTextureDir;
	CString m_StringTextDir;
	CString m_StringXMLDir;
	afx_msg void OnEnChangeEdit3();
	BOOL mCopytoServer;
	BOOL mJustShowErrors;
	afx_msg void OnBnClickedOk();
	DWORD XboxIPAVar;
	CString mStringTargetDir;
	CString mStringServerDir;
	CString mStringEditor;
	CString mStringLuaDir;
	afx_msg void OnBnClickedFullExport();
	afx_msg void OnBnClickedEAFontExport();
	int FullExport;
	int EAFontExport;
};

BEGIN_MESSAGE_MAP(CConfigDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO1, OnBnClickedFullExport)
	ON_BN_CLICKED(IDC_RADIO2, OnBnClickedEAFontExport)
END_MESSAGE_MAP()

CConfigDlg::CConfigDlg() : CDialog(CConfigDlg::IDD)
, m_StringUIMakerDir(_T(""))
, m_StringTextureDir(_T(""))
, m_StringTextDir(_T(""))
, m_StringXMLDir(_T(""))
, mCopytoServer(FALSE)
, mJustShowErrors(FALSE)
, XboxIPAVar(0)
, mStringTargetDir(_T(""))
, mStringServerDir(_T(""))
, mStringEditor(_T(""))
, mStringLuaDir(_T(""))
, FullExport(true)
, EAFontExport(false)
{
}

void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{

}

void CUIMaker2App::OnConfig()
{

}

// CUIMaker2App message handlers





void CConfigDlg::OnBnClickedOk()
{

}

// CLangDlg dialog used for App Lang

class CLangDlg : public CDialog
{
public:
	CLangDlg();

// Dialog Data
	enum { IDD = IDD_LANG_BOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnBnClickedEnglish();
	afx_msg void OnBnClickedJapanese();
	afx_msg void OnBnClickedGerman();
	afx_msg void OnBnClickedFrench();
	afx_msg void OnBnClickedSpanish();
	afx_msg void OnBnClickedItalian();
	afx_msg void OnBnClickedKorean();
	afx_msg void OnBnClickedChinese();
	afx_msg void OnBnClickedPortuguese();
};

CLangDlg::CLangDlg() : CDialog(CLangDlg::IDD)
{
}

void CLangDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLangDlg, CDialog)
	ON_BN_CLICKED(IDC_ENGLISH, OnBnClickedEnglish)
	ON_BN_CLICKED(IDC_JAPANESE, OnBnClickedJapanese)
	ON_BN_CLICKED(IDC_GERMAN, OnBnClickedGerman)
	ON_BN_CLICKED(IDC_FRENCH, OnBnClickedFrench)
	ON_BN_CLICKED(IDC_SPANISH, OnBnClickedSpanish)
	ON_BN_CLICKED(IDC_ITALIAN, OnBnClickedItalian)
	ON_BN_CLICKED(IDC_KOREAN, OnBnClickedKorean)
	ON_BN_CLICKED(IDC_CHINESE, OnBnClickedChinese)
	ON_BN_CLICKED(IDC_PORTUGUESE, OnBnClickedPortuguese)
END_MESSAGE_MAP()

// App command to run the dialog
void CUIMaker2App::OnAppLang()
{
	CLangDlg LangDlg;
	LangDlg.DoModal();
}

void CLangDlg::OnBnClickedEnglish()
{
	// TODO: Add your control notification handler code here
	this->OnOK();
}

void CLangDlg::OnBnClickedJapanese()
{
	// TODO: Add your control notification handler code here
	this->OnOK();
}

void CLangDlg::OnBnClickedGerman()
{
	// TODO: Add your control notification handler code here
	this->OnOK();
}

void CLangDlg::OnBnClickedFrench()
{
	// TODO: Add your control notification handler code here
	this->OnOK();
}

void CLangDlg::OnBnClickedSpanish()
{
	// TODO: Add your control notification handler code here
	this->OnOK();
}

void CLangDlg::OnBnClickedItalian()
{
	// TODO: Add your control notification handler code here
	this->OnOK();
}

void CLangDlg::OnBnClickedKorean()
{
	// TODO: Add your control notification handler code here
	this->OnOK();
}

void CLangDlg::OnBnClickedChinese()
{
	// TODO: Add your control notification handler code here
	this->OnOK();
}

void CLangDlg::OnBnClickedPortuguese()
{
	// TODO: Add your control notification handler code here
	this->OnOK();
}

void CConfigDlg::OnBnClickedFullExport()
{
	// TODO: Add your control notification handler code here
}

void CConfigDlg::OnBnClickedEAFontExport()
{
	// TODO: Add your control notification handler code here
}
