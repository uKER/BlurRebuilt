/////////////////////////////////////////////////////////////////////////////
// Name:        particle_editor.cpp
// Purpose:     Minimal wxWidgets sample integrating Mercury::Display
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Frame.h"

#include "wx/aui/aui.h"
#include "src/Preview/PreviewWindow.h"
#include "src/Explorer/Explorer.h"
#include "src/RecordGraph/Graph.h"
#include "src/DynamicForm/Form.h"

#include "src/Record/GenericRecord.h"
#include "src/Record/ParticleEventRecord.h"

#include "Samples/Section/SectionSkyDome.h"

#include "Network/Network.h"

#include "xt\xttool.h"

#include "Render/Dynamic/Particle/Generic/Sorcery.h"

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "particle_editor.xpm"
#endif

#include "MenuEnums.h"

DEFINE_EVENT_TYPE(wxEVT_MY_EVENT)

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(C_Frame, wxFrame)
    EVT_MENU(Frame_Quit,  C_Frame::OnQuit)
    EVT_MENU(Frame_About, C_Frame::OnAbout)
	
	EVT_MENU(Frame_Open, C_Frame::OnOpen)
	EVT_MENU(Frame_Open_TheClub, C_Frame::OnOpen)
	EVT_MENU(Frame_Open_Gotham, C_Frame::OnOpen)
	EVT_MENU(Frame_Close, C_Frame::OnClose)

	EVT_MENU(Frame_Save, C_Frame::OnSave)
	



	EVT_AUINOTEBOOK_PAGE_CLOSE(RecordGraphNotebook_Ctrl, C_Frame::OnRecordGraphNotebookPageClose)
	EVT_AUINOTEBOOK_PAGE_CHANGED(RecordGraphNotebook_Ctrl, C_Frame::OnRecordGraphNotebookPageChanged)

	EVT_TIMER(wxID_ANY, C_Frame::OnTimer)
	EVT_COMMAND(wxID_ANY, wxEVT_MY_EVENT, C_Frame::OnMyEvent)
END_EVENT_TABLE()

// frame constructor
C_Frame::C_Frame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title, wxPoint(20, 20), wxSize(1000,600))
{
    // set the frame icon
    SetIcon(wxICON(sample));

#if wxUSE_MENUS
    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Frame_About, _T("&About...\tF1"), _T("Show about dialog"));

	fileMenu->Append(Frame_Open, _T("O&pen Particles\tCtrl-O"), _T("Open Particles Database"));
	fileMenu->Append(Frame_Open_TheClub, _T("O&pen TheClub"), _T("Open TheClub Database"));
	fileMenu->Append(Frame_Open_Gotham, _T("O&pen Gotham"), _T("Open Gotham Database"));
	fileMenu->Append(Frame_Close, _T("C&lose"), _T("Close Database"));
	fileMenu->AppendSeparator();
	fileMenu->Append(Frame_Save, _T("S&ave\tCtrl-S"), _T("Save"));
	fileMenu->AppendSeparator();
    fileMenu->Append(Frame_Quit, _T("E&xit"), _T("Quit this program"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, _T("&File"));
    menuBar->Append(helpMenu, _T("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS


	m_pAuiManager = new wxAuiManager;
	m_pAuiManager->SetManagedWindow(this);

	m_pPreviewWindow = new C_PreviewWindow(this, wxID_ANY, wxDefaultPosition, wxSize(320, 300));

	m_pText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(640, 300), wxTE_READONLY | wxTE_MULTILINE | wxTE_DONTWRAP);
	
	
	m_pDatabaseNotebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(300, 200));
	{
		U32 style = wxAUI_NB_DEFAULT_STYLE;
		style &= ~wxAUI_NB_CLOSE_ON_ACTIVE_TAB;

		m_pDatabaseNotebook->SetWindowStyleFlag(style);
        m_pDatabaseNotebook->Refresh();

		m_pExplorer = new C_Explorer(this, Explorer_Ctrl, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxTR_EDIT_LABELS);
		m_pDatabaseNotebook->AddPage(m_pExplorer, _T("Hierarchy"));
	}

	m_pRecordGraphNotebook = new wxAuiNotebook(this, RecordGraphNotebook_Ctrl, wxDefaultPosition, wxDefaultSize);
	{
		//C_NodeGraph* pSchematic = new C_NodeGraph(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
		//pSchematic->Initialise();
		//m_pRecordGraphNotebook->AddPage( pSchematic, _T("Schematic") );
	}

	m_pAuiManager->AddPane(m_pText, wxBOTTOM, wxT("Log"));
	m_pAuiManager->AddPane(m_pDatabaseNotebook, wxLEFT, wxT("Explorer"));
	m_pAuiManager->AddPane(m_pRecordGraphNotebook, wxCENTER, wxT("Node Schematic"));

	m_pLog = wxLog::SetActiveTarget(new wxLogTextCtrl(m_pText));

	m_pSection = NULL;
	
	if(m_pPreviewWindow->Create())
	{
		wxLogMessage("Display Device created successfully!");
		m_pAuiManager->AddPane(m_pPreviewWindow, wxRIGHT, wxT("Display Window"));

		/*m_pSection = new Samples::C_SectionParticles("Particles");
		
		if(!m_pPreviewWindow->SetSection(m_pSection))
		{
			m_pPreviewWindow->GetDevice()->DestroyRenderersVolatile();
			m_pPreviewWindow->GetDevice()->DestroyRenderers();
		
			delete m_pSection;
			m_pSection = NULL;

			m_pPreviewWindow->GetDevice()->CreateRenderers();
			m_pPreviewWindow->GetDevice()->CreateRenderersVolatile();
			
			wxLogMessage("ERROR: Section failed to initialise");
		}*/
	}
	else
	{
		m_pSection = NULL;
		wxLogMessage("ERROR: Display Device failed to create");
		delete m_pPreviewWindow;
	}

	m_pDynamicForm = new DynamicForm::C_Form(this, this, wxID_ANY, wxDefaultPosition, wxSize(320, 300),
											wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxTAB_TRAVERSAL);

	m_pAuiManager->AddPane(m_pDynamicForm, wxRIGHT, wxT("Form"));
	m_pAuiManager->Update();

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
	SetStatusText(_T("Welcome to the Visual Xt Editor!"));
#endif // wxUSE_STATUSBAR

	m_pXtNetCon = new C_XtNetCon;
	m_NetworkPumpTimer.SetOwner(this, 0);
	m_NetworkIdleTimer.SetOwner(this, 1);
	
}

C_Frame::~C_Frame()
{
	CloseDb();

	if(m_pPreviewWindow)
		m_pPreviewWindow->DestroyRenderers();

	if(m_pSection)
		delete m_pSection;

	if(m_pPreviewWindow)
		delete m_pPreviewWindow;

	m_pAuiManager->UnInit();
	delete m_pAuiManager;

	delete wxLog::SetActiveTarget(m_pLog);

	delete m_pXtNetCon;
}

// event handlers

void C_Frame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
     Close(true);
}

void C_Frame::OnOpen(wxCommandEvent& event)
{
	wxString databasePath;
	wxString binPath;
	Util::C_String assetPath;

	/*wxFileDialog dialog(this, _T("Open file dialog"), wxEmptyString, wxEmptyString, _T("Xt Project (*.xtp)|*.xtp"));
    dialog.CentreOnParent();
    dialog.SetDirectory(wxGetHomeDir());

    if (dialog.ShowModal() == wxID_OK)
    {
        wxString info;
        info.Printf(_T("Full file name: %s\n")
                    _T("Path: %s\n")
                    _T("Name: %s"),
                    dialog.GetPath().c_str(),
                    dialog.GetDirectory().c_str(),
                    dialog.GetFilename().c_str());
        wxMessageDialog dialog2(this, info, _T("Selected file"));
        dialog2.ShowModal();
    }*/

	switch(event.GetId())
	{
	case Frame_Open:
		databasePath = "Z:\\devroot\\code\\mercury\\applications\\samples\\particles\\Sorcery2\\db";
		assetPath = "Z:\\devroot\\code\\mercury\\applications\\samples\\particles\\Image";
		break;
	case Frame_Open_TheClub:
		databasePath = "Z:\\shitstorm\\sorcery2";
		assetPath = "Z:\\shitstorm\\game\\PC\\Data\\Image";
		break;
	case Frame_Open_Gotham:
		databasePath = "Z:\\devroot\\code\\gotham4\\Data\\GameData\\db";
		break;
	}

	
	m_pPreviewWindow->OpenAssets(assetPath);

	// open xt database
	m_XtLoader.Open(databasePath);

	// open binary packs in preview window
	if(m_XtLoader.GetPacks().Size() > 0)
	{
		m_pPreviewWindow->OpenFlasks(m_XtLoader.GetPacks());
		wxLogMessage(_T("Loading complete!"));
	}
	else
		wxLogMessage(_T("Could not find Xt flask"));
	
	// open explorer
	m_pExplorer->ExpandRoot();

	wxLogMessage(_T("%d records loaded."), m_Records.Size());

	// initialise network
	m_pXtNetCon->Create(this);
	m_NetworkPumpTimer.Start(1000);
	m_NetworkIdleTimer.Start(250);
}

void C_Frame::OnClose(wxCommandEvent& event)
{
	CloseDb();
}

void C_Frame::CloseDb()
{
	// remove all directories from explorer
	m_pExplorer->Unselect();
	RemoveRecords();
	m_pExplorer->Initialise();

	// clear dynamic form and update layout
	m_pDynamicForm->ClearForm();

	// destroy network connection
	m_pXtNetCon->Destroy();
	m_NetworkPumpTimer.Stop();
	m_NetworkIdleTimer.Stop();

	m_XtLoader.Close();
	m_pPreviewWindow->CloseFlask();
	
	// hack - there seems to be some xt records not getting destroyed at flask close?
	DynamicEffect::SorceryInterface::Reset();
}

void C_Frame::OnSave(wxCommandEvent& event)
{
	m_XtLoader.Save();
}

void C_Frame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	

    wxMessageBox(wxString::Format(
                    _T("This is the Visual Xt Editor\n")
                    _T("running under %s."),
                    wxVERSION_STRING,
                    wxGetOsDescription().c_str()
                 ),
                 _T("About Visual Xt Editor"),
                 wxOK | wxICON_INFORMATION,
                 this);
}

void C_Frame::OnRecordGraphNotebookPageClose(wxAuiNotebookEvent& event)
{
    wxAuiNotebook* ctrl = (wxAuiNotebook*)event.GetEventObject();
    
	S32 index = ctrl->GetSelection();
	if(RecordGraph::C_Graph* pGraph = (RecordGraph::C_Graph*)ctrl->GetPage(index))
		pGraph->GetRecord()->Close();

	event.Veto();
}

void C_Frame::OnRecordGraphNotebookPageChanged(wxAuiNotebookEvent& event)
{
    wxAuiNotebook* ctrl = (wxAuiNotebook*)event.GetEventObject();
   
	S32 index = event.GetSelection();
	
	if(RecordGraph::C_Graph* pGraph = (RecordGraph::C_Graph*)ctrl->GetPage(index))
		pGraph->GetRecord()->Select();
	
}

void C_Frame::OnTimer(wxTimerEvent& event)
{
	if(event.GetId() == 0)
	{
		if(m_pXtNetCon)
			m_pXtNetCon->Pump();
	}
	else if(event.GetId() == 1)
	{
		if(m_pXtNetCon)
			m_pXtNetCon->NetSendIdle();
	}

	NetworkSendRecords();
}

void C_Frame::OnMyEvent(wxCommandEvent& event)
{
	if(m_pXtNetCon)
		m_pXtNetCon->Pump();
}

void C_Frame::NetworkSendRecords()
{
	Ut::Vector<Xt::DomRecord*> networkRecords;

	for(S32 i = 0; i < m_NetworkRecords.Size(); ++i)
	{
		Xt::DomRecord* rec = Xt::AsRecord(m_NetworkRecords[i]);

		if (!rec)
			continue;

		bool found = false;

		for(S32 j = 0; j < networkRecords.Size(); ++j)
		{
			if (rec == networkRecords[j])
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			networkRecords.PushBack(rec);
		}
	}


	if (networkRecords.Size() > 0)
		m_pXtNetCon->NetSendRecords(m_pDynamicForm->GetDomDb(), &networkRecords[0], networkRecords.Size());

	m_NetworkRecords.Clear();
}

void C_Frame::NetworkAddRecord(Xt::DomRecord* pRecord)
{
	m_NetworkRecords.PushBack(pRecord);
}

C_GenericRecord* C_Frame::AddRecord(Xt::DomRecord* pRecord, const wxString& name)
{
	wxString typeName((const char*)pRecord->type);

	C_GenericRecord* pGenericRecord = NULL;

	if(typeName == "Particle_Event")
		pGenericRecord = new C_ParticleEventRecord(pRecord, m_pDynamicForm);
	else
		pGenericRecord = new C_GenericRecord(pRecord, m_pDynamicForm);

	if(pGenericRecord)
	{
		pGenericRecord->SetName(name);
		pGenericRecord->Create();

		m_Records.PushBack(pGenericRecord);

		return pGenericRecord;
	}
	else
		return NULL;
}

void C_Frame::RemoveRecords()
{
	for(S32 i = 0; i < m_Records.Size(); ++i)
	{
		C_Record* pRecord = m_Records[i];

		pRecord->Close();
		pRecord->Destroy();

		delete m_Records[i];
	}

	m_Records.Clear();
}

