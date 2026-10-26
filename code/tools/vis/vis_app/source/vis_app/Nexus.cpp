// Nexus.cpp : Defines the class behaviors for the application.
//
//#define NEXUS_IDLE_PROCESS

#include <vis_app/stdafx.h>
#include <vis_app/Nexus.h>
#include <vis_app/NexusSystem.h>
#include <vis_app/NexusRender.h>
#include <vis_app/MainFrm.h>

#include <vis_app/NexusDoc.h>

#include "../../resource.h"
#include <vis_app/NexusProgressDialog.h>
#include <mfc_extras/BaseThread.h>
//#include "..\..\include\vis_app\nexusprogress.h"
#include <vis_app/NexusView.h>
#include "..\..\include\vis_app\nexus.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Nexus::NexusSystem* Nexus::g_pNexusSystem = 0;
Nexus::NexusRender* Nexus::g_pNexusRender = 0;

// NexusApp

BEGIN_MESSAGE_MAP(NexusApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
END_MESSAGE_MAP()


// NexusApp construction

NexusApp::NexusApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only NexusApp object

NexusApp theApp;

// NexusApp initialization

BOOL NexusApp::InitInstance()
{
	Aftk::C_Global::Create();

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
	SetRegistryKey(_T("Bizarre Creations"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(NexusDoc),
		RUNTIME_CLASS(MainFrame),       // main SDI frame window
		RUNTIME_CLASS(NexusView));

	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
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
void NexusApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// NexusApp message handlers


void NexusApp::OnFileImport()
{
}

#ifdef NEXUS_IDLE_PROCESS
BOOL NexusApp::OnIdle(LONG lCount)
{
	CWinApp::OnIdle(lCount);

	if (Nexus::g_pNexusRender)
	{
		if (Nexus::g_pNexusRender->AnimationPaused())
		{
			return false;
		}
		else
		{
			AfxGetMainWnd()->Invalidate(false); 
		}
	}

	return TRUE;
}
#endif

