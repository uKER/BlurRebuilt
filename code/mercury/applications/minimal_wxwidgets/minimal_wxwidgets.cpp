/////////////////////////////////////////////////////////////////////////////
// Name:        minimal_wxwidgets.cpp
// Purpose:     Minimal wxWidgets sample integrating Mercury::Display
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "system\memory\MemoryTrackerDefines.h"
#include "system\memory\MemoryTracker.h"
#include "system\memory\AllocatorFunctions.h"

void* operator new[]( size_t size ) { return Memory::TrackedAlignedAlloc(size, 16); }
void operator delete[]( void* ptr ) { Memory::TrackedAlignedFree(ptr); }
void * operator new (size_t size) { return Memory::TrackedAlignedAlloc(size,16); }
void operator delete(void *mem) { Memory::TrackedAlignedFree(mem); }

#include "wx/aui/aui.h"
#include "wx/mercury/displaywnd.h"
//#include "Samples/Section/SectionSkyDome.h"

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "minimal_wxwidgets.xpm"
#endif

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title);
	~MyFrame();

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

private:
	wxAuiManager*		m_pAuiManager;
	wxDisplayWindow*	m_pDisplayWindow;
	wxLog*				m_pLog;
	wxTextCtrl*			m_pText;

	Samples::C_Section*	m_pSection;
	
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    EVT_MENU(Minimal_About, MyFrame::OnAbout)
END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // create the main application window
	MyFrame *frame = new MyFrame(_T("Minimal wxWidgets Mercury::Display App"));

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MyFrame::MyFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title, wxPoint(20, 20), wxSize(640,600))
{
    // set the frame icon
    SetIcon(wxICON(sample));

#if wxUSE_MENUS
    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Minimal_About, _T("&About...\tF1"), _T("Show about dialog"));

    fileMenu->Append(Minimal_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, _T("&File"));
    menuBar->Append(helpMenu, _T("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

	m_pSection = NULL;

	m_pAuiManager = new wxAuiManager;
	m_pAuiManager->SetManagedWindow(this);

	m_pDisplayWindow = new wxDisplayWindow(this, wxID_ANY, wxDefaultPosition, wxSize(640, 400));

	m_pText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(640, 200), wxTE_READONLY | wxTE_MULTILINE | wxTE_DONTWRAP);
	m_pAuiManager->AddPane(m_pText, wxBOTTOM, wxT("Log"));

	m_pLog = wxLog::SetActiveTarget(new wxLogTextCtrl(m_pText));
	
	if(m_pDisplayWindow->Create())
	{
		wxLogMessage("Display Device created successfully!");
		m_pAuiManager->AddPane(m_pDisplayWindow, wxCENTER, wxT("Display Window"));

		//m_pSection = new Samples::C_SectionSkyDome("SkyDome");
		//m_pDisplayWindow->SetSection(m_pSection);
	}
	else
	{
		m_pSection = NULL;
		wxLogMessage("ERROR: Display Device failed to create");
		delete m_pDisplayWindow;
	}
	
	m_pAuiManager->Update();

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
	SetStatusText(_T("Welcome to the wxWidgets Mercury::Display App!"));
#endif // wxUSE_STATUSBAR


	
}

MyFrame::~MyFrame()
{
	if(m_pDisplayWindow)
		m_pDisplayWindow->DestroyRenderers();

//	if(m_pSection)
//		delete m_pSection;

	if(m_pDisplayWindow)
		delete m_pDisplayWindow;

	delete wxLog::SetActiveTarget(m_pLog);
}


// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format(
                    _T("Welcome to %s!\n")
                    _T("\n")
					_T("This is the minimal wxWidgets Mercury::Display sample\n")
                    _T("running under %s."),
                    wxVERSION_STRING,
                    wxGetOsDescription().c_str()
                 ),
                 _T("About wxWidgets minimal display sample"),
                 wxOK | wxICON_INFORMATION,
                 this);
}
