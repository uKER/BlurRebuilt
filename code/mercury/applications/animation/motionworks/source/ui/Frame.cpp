#include "stdafx.h"
#include "Frame.h"
#include <wx/aui/aui.h>
#include <wx/filename.h>
#include <wx/file.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <System/Debug/Log/DebugLog.h>
#include <System/DebugLog.h>
#include <system/debug/log/Filter.h>
#include <system/debug/log/Format.h>
#include <System/Debug/Log/Subscribers/ConsoleSubscriber.h>
#include <system/Debug/BuildStamp.h>
#include <wx/config.h>
#include <wx/wxFlatNotebook/wxFlatNotebook.h>

#include "PreviewState.h"
#include "AppSettings.h"
#include "Db.h"
#include "GlobalSignals.h"

#include "ui/DisplayWindow.h"
#include "ui/AnimationPreview.h"
#include "ui/SkeletonPanel.h"
#include "ui/CurveViewerPanel.h"
#include "ui/XtWindow.h"
#include "ui/XtTreeView.h"
#include "ui/OptionsDialogs.h"
#include "ui/ConstraintsPanel.h"
#include "ui/JointPropertiesPanel.h"
#include "ui/DynamicsPanel.h"
#include "ui/TimelinePanel.h"

#include <animation/tools/Scene.h>
#include <animation/tools/CycleBuilder.h>

C_Frame* gFrame = NULL;

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT,
    
    E_File_OpenSkeleton = wxID_HIGHEST + 1,
    E_File_OpenCycle,
    E_File_ReloadChangedFiles,
	E_File_ExportDynamicJointSettings,
    E_File_Exit,

	E_View_ShowKeyReducedCycle,
	E_View_ShowRawCycle,
	
	// Character menu
	E_Character_ResetRoot,
	E_Character_FreezeRoot,
	E_Character_AbsoluteRootMotion,
	E_Character_EliminateRedundantPushPops,
	
	// Camera menu.
	E_Camera_OrbitCharacter,
	E_Camera_OrbitSelected,
	E_Camera_AttachToJoint,
	E_Camera_CycleMode,
	
	// Skeleton menu
	E_View_ShowSkeleton,
	E_View_ShowJointNames,
	E_View_ShowOnlySelectedJointName,
	E_View_HideUnselectedJoints,
	E_View_ShowConstraints,
		
	// Skinned Mesh menu
	E_View_ShowSkinnedMesh,
	E_View_ShowWireframe,
	
	// HUD menu
	E_View_HudShowCustomCurves,
	E_View_HudShowCommandList,
	E_View_HudShowSelectedJointTransform,
	E_View_HudShowWorldAxes,
	
	// Window menu.
	E_View_ShowLogWindow,
	E_View_ShowToolsWindow,
	E_View_ShowPropertiesWindow,
	E_View_ShowCurveWindow,
	
	E_Tool_OpenSkeleton,
	E_Tool_OpenCycle,
	E_Tool_ReloadSchema,
	E_Tool_SceneScale,				// applied to imported skeletons and cycles.
	E_Tool_PlaybackSpeedText,
	E_Tool_TogglePaused,
	
	
	E_Debug_OpenDbFolder,
	E_Debug_ReloadSchema,
	
	E_Help_ShowHelp
};

enum E_Timer
{
	E_Timer_XtUpdate	= 0
};

BEGIN_EVENT_TABLE(C_Frame, wxFrame)
	EVT_TIMER(E_Timer_XtUpdate, C_Frame::OnTimer)
    EVT_MENU(Minimal_Quit,  C_Frame::OnQuit)
    EVT_MENU(Minimal_About, C_Frame::OnAbout)
    EVT_MENU(E_Help_ShowHelp, C_Frame::OnHelp)
    
    EVT_MENU_RANGE(E_View_ShowSkeleton, E_View_ShowCurveWindow, C_Frame::OnViewMenuCommand)
    EVT_MENU_RANGE(E_Character_ResetRoot, E_Character_EliminateRedundantPushPops, C_Frame::OnCharacterMenuCommand)
    EVT_MENU_RANGE(E_Camera_OrbitCharacter, E_Camera_CycleMode, C_Frame::OnCameraMenuCommand)
    
    EVT_MENU(E_Debug_OpenDbFolder, C_Frame::OnDebugMenu)
    EVT_MENU(E_Debug_ReloadSchema, C_Frame::OnReloadSchema)
    
    EVT_UPDATE_UI_RANGE(E_File_ReloadChangedFiles, E_Tool_TogglePaused, C_Frame::OnUpdateUI)
END_EVENT_TABLE()


// frame constructor
C_Frame::C_Frame(const wxString& title, C_XtDatabase& db, C_AnimationGraphEditor& animGraphEditor)
       : wxFrame(NULL, wxID_ANY, title, wxPoint(20, 20), wxSize(1024,768)),
       C_XtDatabaseListener( &db ),
       m_db(db),
       m_animGraphEditor( animGraphEditor )
{
#if 0
	// Accelerator table.
	wxAcceleratorEntry accEntries[] =
	{
		//wxAcceleratorEntry( wxACCEL_NORMAL, 'O', E_Camera_OrbitCharacter ),
		//wxAcceleratorEntry( wxACCEL_NORMAL, 'F', E_Camera_OrbitSelected ),
		//wxAcceleratorEntry( wxACCEL_NORMAL, 'Z', E_Camera_AttachToJoint ),
		//wxAcceleratorEntry( wxACCEL_NORMAL, 'S', E_View_ShowSkeleton ),
		//wxAcceleratorEntry( wxACCEL_NORMAL, 'P', E_Tool_TogglePaused ),
		//wxAcceleratorEntry( wxACCEL_NORMAL, 'N', E_View_ShowJointNames ),
		//wxAcceleratorEntry( wxACCEL_NORMAL, 'H', E_View_HideUnselectedJoints ),
		
		//wxAcceleratorEntry( wxACCEL_CTRL, 'S', E_File_OpenSkeleton ),
		//wxAcceleratorEntry( wxACCEL_CTRL, 'E', E_File_OpenCycle ),
		//wxAcceleratorEntry( wxACCEL_CTRL, 'R', E_Tool_ReloadSchema )
	};
	
	wxAcceleratorTable accTable( sizeof(accEntries)/sizeof(accEntries[0]), accEntries );
	
	SetAcceleratorTable( accTable );
#endif


    // set the frame icon
    wxIcon icon;
    icon.LoadFile("./icons/MotionWorksIcon.bmp");
    SetIcon(icon);
        
    // Create tool bar.
    m_toolBar = CreateToolBar( wxTB_FLAT ); //  | wxTB_NODIVIDER );
    PopulateToolBar( m_toolBar );
    
    // Connect up keyboard events.
    Connect( wxEVT_KEY_DOWN, wxKeyEventHandler(C_Frame::OnKeyEvent) );
    Connect( wxEVT_KEY_UP, wxKeyEventHandler(C_Frame::OnKeyEvent) );
       

    // create a menu bar
    wxMenu *fileMenu = new wxMenu;
    
    wxMenuItem* item;
    item = fileMenu->Append( E_File_OpenSkeleton, _T("Open &Skeleton...\tCTRL+S"), _T("Open skeleton file") );
    Connect(item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnOpenSkeleton));
    item = fileMenu->Append( E_File_OpenCycle, _T("Open &Cycle...\tCTRL+E"), _T("Open cycle file") );
    Connect(item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnOpenCycle));
    item = fileMenu->Append( E_File_ReloadChangedFiles, _T("&Reload Changed Files\tCTRL+R"), _T("Reload changed skeleton and cycle files") );
    Connect( item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnReloadChangedFiles) );
	fileMenu->AppendSeparator();
	item = fileMenu->Append( E_File_ExportDynamicJointSettings, _T("Export &Dynamic Joint Settings..."), _T("Export dynamic joint settings to a text file for import into Maya.") );
	Connect( item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnExportDynamicJointSettings) );
    fileMenu->AppendSeparator();
    fileMenu->Append(Minimal_Quit, _T("E&xit"), _T("Quit this program"));
    
    // Debug menu.
    wxMenu* debugMenu =  new wxMenu;
    debugMenu->Append( E_Debug_OpenDbFolder, _T("&Open Db Folder"), _T("Open the local database folder") );
    debugMenu->Append( E_Debug_ReloadSchema, _T("&Reload Schema"), _T("Reload database schema") );
        
    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append( E_Help_ShowHelp, _T("&Help Contents\tF1"), _T("Show help" ) );
    helpMenu->AppendSeparator();
    helpMenu->AppendSubMenu( debugMenu, _T("Debug") );
    helpMenu->AppendSeparator();
    helpMenu->Append(Minimal_About, _T("&About"), _T("Show about dialog"));
    
    wxMenu* characterMenu = new wxMenu;
    characterMenu->Append( E_Character_ResetRoot, _T("&Reset Root"), _T("Reset character root to lie at the world origin") );
    characterMenu->Append( E_Character_FreezeRoot, _T("&Freeze Root"), _T("Freezes position of character root"), wxITEM_CHECK );
    characterMenu->Append( E_Character_AbsoluteRootMotion, _T("&Absolute Root Motion"), _T("Makes root use exact absolute position and orientation from currently active animations"), wxITEM_CHECK );
    characterMenu->AppendSeparator();
    characterMenu->Append( E_Character_EliminateRedundantPushPops, _T("&Eliminate Redundant Push/Pops"), _T("Eliminates redundant push/pops in the command list"), wxITEM_CHECK );

    wxMenu* cameraMenu = new wxMenu;
    cameraMenu->AppendRadioItem( E_Camera_OrbitCharacter, _T("Orbit &Character"), _T("Orbit the character") );
    cameraMenu->AppendRadioItem( E_Camera_OrbitSelected, _T("Orbit &Selected"), _T("Orbit the selected joint") );
    cameraMenu->AppendRadioItem( E_Camera_AttachToJoint, _T("&Attach To Joint"), _T("Place camera on selected joint and aim down its Z axis") );
    cameraMenu->AppendSeparator();
    cameraMenu->Append( E_Camera_CycleMode, _T("C&ycle Camera Mode") );
    cameraMenu->Check( E_Camera_OrbitCharacter, true );

    
	// View menu.
	wxMenu* viewMenu = new wxMenu;
	
	wxMenu* skelMenu = new wxMenu;
	skelMenu->Append( E_View_ShowSkeleton, wxT( "Show &Skeleton" ), wxT( "Show skeleton in 3D viewport" ), wxITEM_CHECK );
	skelMenu->Append( E_View_HideUnselectedJoints, wxT( "Hide Unselected Joints" ), wxT( "Hide all unselected joints in 3D viewport" ), wxITEM_CHECK );
	skelMenu->Append( E_View_ShowJointNames, wxT( "Show Joint Names" ), wxT( "Show joint names in 3D viewport" ), wxITEM_CHECK );
	skelMenu->Append( E_View_ShowOnlySelectedJointName, wxT( "Show Selected Joint Name &Only" ), wxT( "Show only the name of the selected joint in the 3D viewport" ), wxITEM_CHECK );
	skelMenu->Append( E_View_ShowConstraints, _T("Show Selected Constraints"), _T("Show selected constraints in 3D viewport."), wxITEM_CHECK );
	viewMenu->AppendSubMenu( skelMenu, "Skeleton" );
	viewMenu->AppendSeparator();
	
	wxMenu* meshMenu = viewMenu; // new wxMenu;
	meshMenu->Append( E_View_ShowSkinnedMesh, wxT( "Show Skinned &Mesh" ), wxT( "Show skinned mesh in 3D viewport" ), wxITEM_CHECK );
	meshMenu->Append( E_View_ShowWireframe, wxT( "Show &Wireframe" ), wxT( "Show wireframe version of skinned mesh" ), wxITEM_CHECK );
	viewMenu->AppendSeparator();
	//viewMenu->AppendSubMenu( meshMenu, "Skinned Mesh" );
	
	//viewMenu->AppendSeparator();
	//viewMenu->Append( E_View_HudShowCustomCurves, _T("Show Custom Curves"), _T("Show list of custom curves and their current values in viewport"), wxITEM_CHECK );
	//viewMenu->Append( E_View_HudShowCommandList, _T("Show Command List"), wxT("Show command list in viewport"), wxITEM_CHECK );
	
	viewMenu->Append( E_View_HudShowSelectedJointTransform, _T("Show Selected Joint Transform"), _T("Show selected joint transform values in viewport"), wxITEM_CHECK );
	viewMenu->Append( E_View_HudShowWorldAxes, _T("Show World Axes"), _T("Show character's coordinate system"), wxITEM_CHECK );
	
		
	// Windows.
	viewMenu->AppendSeparator();
	wxMenu* windowMenu = new wxMenu;
	windowMenu->Append( E_View_ShowLogWindow, wxT( "Show &Log Window" ), wxT( "Show log window." ), wxITEM_CHECK );
	windowMenu->Append( E_View_ShowToolsWindow, wxT( "Show Tools Window" ), wxT( "Show tools window." ), wxITEM_CHECK );
	windowMenu->Append( E_View_ShowPropertiesWindow, wxT( "Show Properties Window" ), wxT( "Show propreties window." ), wxITEM_CHECK );
	//viewMenu->Append( E_View_ShowCurveWindow, wxT( "Show &Curves Window" ), wxT( "Show curves window." ), wxITEM_CHECK );
	viewMenu->AppendSubMenu(windowMenu, _T("Windows"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append( fileMenu, _T("&File") );
    menuBar->Append( characterMenu, _T("&Character") );
    menuBar->Append( cameraMenu, _T("Ca&mera") );
	menuBar->Append( viewMenu, _T("&View") );
    menuBar->Append( helpMenu, _T("&Help") );

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);

	//m_pSection = NULL;

	SetTitle("MotionWorks");
	
	//
	// Create UI panels.
	//
	
	

//	m_pDisplayWindow = new wxDisplayWindow(this, wxID_ANY, wxDefaultPosition, wxSize(640, 400));

	
	static const S32 kLogWindowHeight = 40;
	static const S32 kLayerStackPanelHeight = 500;

	m_pText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxTE_DONTWRAP);
	
	m_pLog = wxLog::SetActiveTarget(new wxLogTextCtrl(m_pText));

	wxRect clientRect = GetClientRect();

#define ENABLE_3D_DISPLAY

	m_animationPreviewWindow = NULL;
	
#ifdef ENABLE_3D_DISPLAY
	const S32 k3dViewWidth = clientRect.GetWidth() / 2;
	const S32 k3dViewHeight= clientRect.GetHeight();

	m_animationPreviewWindow = new AnimationPreviewWindow( this, wxID_ANY, wxDefaultPosition, wxSize( k3dViewWidth, k3dViewHeight ) );
#else
	m_animationPreviewWindow = NULL;
#endif
	

	
#define ENABLE_CURVE_VIEWER

#ifdef ENABLE_CURVE_VIEWER

	static const U32 kMinCurveViewerHeight = 300;

	m_curveViewerPanel = new C_CurveViewerPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize );
	
#endif

	const S32 kXtWindowWidth = (S32) (clientRect.GetWidth() * 0.4f);
	const S32 kXtWindowHeight = clientRect.GetHeight();

	// Create Sorcery scarab form panel.
	m_xtWindow = new C_XtWindow(
		new wxPanel(this),
		wxID_ANY,
		wxDefaultPosition,
		wxSize(kXtWindowWidth, kXtWindowHeight),
		wxFULL_REPAINT_ON_RESIZE |  wxCLIP_CHILDREN | wxTAB_TRAVERSAL);
		
	m_xtWindow->SetDefaultCaption(_T("Blend Tree"));

	// Create scene hierarchy panel.
	m_skeletonPanel = new C_SkeletonPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize );

	// Create constraint list panel.
	m_constraintsPanel = new C_ConstraintsPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	
	// Create global dynamics settings panel.
	m_dynamicsPanel = new C_DynamicsPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	m_dynamicsPanel->SetDynamicsContext(&gClipPreviewState->m_dynamicsContext);
	
	m_jointPropertiesPanel = new C_JointPropertiesPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
		
	//
	// Create Sorcery pack file tree view panel.
	//
	m_xtTreeView = new C_XtTreeView( m_db, m_animGraphEditor, *m_xtWindow, this, wxID_ANY, wxDefaultPosition, wxSize( clientRect.GetWidth(), (S32)(clientRect.GetHeight() * 0.2f) ) );
	m_xtTreeView->Populate();
	
	//
	// Create timeline panel.
	//
	m_timelinePanel = new C_TimelinePanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize );

#define USE_NOTEBOOK

#ifdef USE_NOTEBOOK
	m_notebook = new wxFlatNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(clientRect.GetWidth() / 2, (clientRect.GetHeight() * 2) / 3), wxFNB_ALLOW_FOREIGN_DND|wxFNB_NO_X_BUTTON|wxFNB_VC8);
	m_notebook->AddPage(m_xtWindow, _T("Blend Tree"));
	m_notebook->AddPage(m_xtTreeView, _T("Blend Tree Explorer"));	
	m_notebook->AddPage(m_skeletonPanel, _T("Scene Hierarchy"));
	m_notebook->AddPage(m_constraintsPanel, _T("Constraints"));
		
	m_propertiesNotebook = new wxFlatNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(clientRect.GetWidth() / 2, clientRect.GetHeight() / 3),
		wxFNB_ALLOW_FOREIGN_DND|wxFNB_NO_X_BUTTON|wxFNB_VC8);
	
	m_propertiesNotebook->AddPage(m_jointPropertiesPanel, _T("Joint"));
	m_jointPropertiesPanel->UpdateParentNotebookTabCaption();
	m_propertiesNotebook->AddPage(m_dynamicsPanel, _T("Dynamics"));
	
#endif

	//
	// Create AUI manager and its panes.
	//

	m_pAuiManager = new wxAuiManager;
	m_pAuiManager->SetManagedWindow(this);
	m_pAuiManager->SetDockSizeConstraint( 0.6f, 0.6f );

#ifdef ENABLE_CURVE_VIEWER
	m_pAuiManager->AddPane(
		m_curveViewerPanel,
		wxAuiPaneInfo().Bottom().Caption( wxT( "Curve Viewer" ) ).Hide() );
#endif
	
	m_pAuiManager->AddPane(m_pText, wxAuiPaneInfo().Right().Caption(wxT("Log")).Position( 0 ).Hide());
	
	if (m_animationPreviewWindow)
	{
		m_pAuiManager->AddPane(
			m_animationPreviewWindow,
			wxAuiPaneInfo().
			Caption( _T("3D View") ).
			Centre().
			MaximizeButton(true).
			CloseButton( false ).
			//Floatable( true ).
			//Dockable(true).
			//Movable(true).
			Layer(0) );
	}
	
	m_pAuiManager->AddPane(
		m_timelinePanel,
		wxAuiPaneInfo().
			Bottom().
			CloseButton(false).
			MaximizeButton(false).
			CaptionVisible(false).
			Movable(false).
			Gripper(false).
			Floatable(false).			
			Layer(4)
	);
	
#ifdef USE_NOTEBOOK
	m_pAuiManager->AddPane(
		m_notebook,
		wxAuiPaneInfo().Right().Layer(2).MaximizeButton(true).FloatingSize(600, 400).Position(3).Caption(_T("Tools"))
	);

	m_pAuiManager->AddPane(
		m_propertiesNotebook,
		wxAuiPaneInfo().Right().Layer(2).MaximizeButton(true).FloatingSize(600, 400).Position(4).Caption(_T("Properties"))
	);
#else
	m_pAuiManager->AddPane(
		m_xtTreeView,
		wxAuiPaneInfo().
			Caption(_T("Blend Tree Browser")).
			Bottom().
			Layer(1).
			Position(0).
			CloseButton(true).
			MaximizeButton(false).
			Show(true)
	);
	
	m_pAuiManager->AddPane(
		m_dynamicsPanel,
		wxAuiPaneInfo().Float().FloatingSize(wxSize(300, 500)).
		Caption(_T("Dynamics Settings"))
	);
	
	m_pAuiManager->AddPane( m_constraintsPanel,
		wxAuiPaneInfo().
		Right().Layer(2).MaximizeButton(true).FloatingSize(wxSize(600, 600)).
		Position(3).Caption(_T("Constraints")).Hide()
	);

	m_pAuiManager->AddPane(
		m_xtWindow,
		wxAuiPaneInfo().
		Caption(wxT("Blend Tree")).
		Right().
		Layer(2).
		Position(0).
		CloseButton(true).
		MaximizeButton(true).
		//BestSize(kXtWindowWidth,kXtWindowHeight).
		Float().
		Show(true)
	);
	
	m_pAuiManager->AddPane( m_skeletonPanel,
		wxAuiPaneInfo().
		Right().
		Layer(2).
		Position(1).
		//Row( 0 ).
		Caption( _T( "Scene Hierarchy") )
		//Hide().
		//MinSize( kRightDockWidth, kSkeletonPanelHeight ).
		//BestSize( kSkeletonPanelWidth, kSkeletonPanelHeight )
	);	

#endif
	
	m_pAuiManager->Update();
	
	m_pAuiManager->GetPane(m_xtWindow).Dock();
	m_pAuiManager->Update();

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
	SetStatusText(_T("Welcome!"));
#endif // wxUSE_STATUSBAR

	C_ResourceCache::Instance().SetWindowHandle( this );

	InitDialog();

	ReadConfig();
	
	m_xtTreeView->SelectRecord( m_animGraphEditor.GetCurrentGraphRecord() );
	m_xtWindow->SetRecord( m_animGraphEditor.GetCurrentGraphRecord() );
	
	Show(true);
	
}

C_Frame::~C_Frame()
{
	WriteConfig();
	
	//if(m_pDisplayWindow)
	//	m_pDisplayWindow->DestroyRenderers();

	//if(m_pSection)
	//	delete m_pSection;

	//if(m_pDisplayWindow)
	//	delete m_pDisplayWindow;
	

	delete m_timelinePanel;
	
	delete m_skeletonPanel;

	delete wxLog::SetActiveTarget(m_pLog);
}


// event handlers

void C_Frame::OnTimer(wxTimerEvent& e)
{
	//Db::Update();
}

void C_Frame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void C_Frame::OnHelp( wxCommandEvent& e )
{
	// [Blur2PC] The internal wiki page this opened is gone.
}

void C_Frame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxString msg = wxString::Format( 
		_T("MotionWorks\n")
		_T("Animation Tech Preview Tool\n")
		_T("Copyright (C) Bizarre Creations 2009\n\n")
		_T("Built at %s on %s by %s on %s\nRevision:%s\n"),
		Debug::GetBuildTime(),
		Debug::GetBuildDate(),
		Debug::GetBuildUser(),
		Debug::GetBuildPc(),
		Debug::GetBuildRevision()
	);

    wxMessageBox(msg,
                 _T("About MotionWorks"),
                 wxOK | wxICON_INFORMATION,
                 this);
}

inline void Toggle( bool& value )
{
	value = !value;
}

void C_Frame::OnCharacterMenuCommand( wxCommandEvent& e )
{
	switch ( e.GetId() )
	{
		case E_Character_ResetRoot:
			gClipPreviewState->rootTransform.SetIdentity();
			break;
			
		case E_Character_FreezeRoot:
			Toggle( gClipPreviewState->freezeRoot );
			break;
			
		case E_Character_AbsoluteRootMotion:
			Toggle( gClipPreviewState->absoluteRootMotion );
			break;
			
		case E_Character_EliminateRedundantPushPops:
			Toggle( gClipPreviewState->enableCommandListPushPopElimination );
			break;
			
		default:
			e.Skip();
			break;
	}
}

void C_Frame::OnCameraMenuCommand( wxCommandEvent& e )
{
	switch ( e.GetId() )
	{
		case E_Camera_OrbitCharacter:
			C_GlobalSignals::Instance().cameraChanged(E_CameraMode_OrbitCharacter);
			break;
			
		case E_Camera_OrbitSelected:
			C_GlobalSignals::Instance().cameraChanged(E_CameraMode_OrbitCharacter);
			break;
			
		case E_Camera_AttachToJoint:
			C_GlobalSignals::Instance().cameraChanged(E_CameraMode_AttachToJoint);
			break;
			
		case E_Camera_CycleMode:
		{
			C_GlobalSignals::Instance().cameraChanged((E_CameraMode)( (gClipPreviewState->GetCameraMode() + 1) % E_CameraMode_Count ));
			break;
		}		
			
		default:
			e.Skip();
			break;
	}
}

void C_Frame::OnViewMenuCommand( wxCommandEvent& e )
{
	switch ( e.GetId() )
	{
		case E_View_ShowSkeleton:
			gClipPreviewState->showSkeleton = e.IsChecked();
			break;
			
		case E_View_ShowJointNames:
			gClipPreviewState->showJointNames = e.IsChecked();
			break;
			
		case E_View_ShowOnlySelectedJointName:
			gClipPreviewState->showOnlySelectedJointName = e.IsChecked();
			break;
			
		case E_View_ShowConstraints:
			gClipPreviewState->showConstraints = e.IsChecked();
			break;
			
		case E_View_HideUnselectedJoints:
			gClipPreviewState->hideUnselectedJoints = e.IsChecked();
			
			break;
		
		case E_View_ShowSkinnedMesh:
			gClipPreviewState->showSkinnedMesh = e.IsChecked();
			break;
			
		case E_View_ShowWireframe:
			gClipPreviewState->showWireframe = e.IsChecked();
			break;
			
		case E_View_HudShowCustomCurves:
			gClipPreviewState->hudShowCustomCurves = e.IsChecked();
			break;
	
		case E_View_HudShowCommandList:
			gClipPreviewState->hudShowCommandList = e.IsChecked();
			break;
			
		case E_View_HudShowSelectedJointTransform:
			gClipPreviewState->hudShowSelectedJointTransform = e.IsChecked();
			break;
			
		case E_View_HudShowWorldAxes:
			gClipPreviewState->showWorldAxes = e.IsChecked();
			break;
			
		case E_View_ShowLogWindow:
			TogglePane( m_pText );
			break;
			
		case E_View_ShowToolsWindow:
			TogglePane( m_notebook );
			break;
			
		case E_View_ShowPropertiesWindow:
			TogglePane( m_propertiesNotebook );
			break;
			
		case E_View_ShowCurveWindow:
			TogglePane( m_curveViewerPanel );
			break;
		
		default:
			break;
	}
	
	WriteConfig();
}

void C_Frame::OnOpenSkeleton( wxCommandEvent& e )
{
	wxString selectedFileName = ::wxFileSelector(
		wxT( "Select Skeleton File" ),
		wxT( "" ),
		wxT( "" ),
		wxT( "" ),
		wxT( "All Supported Files (*.a3d; *.xsi;*.fbx;*.skeleton)|*.a3d;*.xsi;*.fbx;*.skeleton" ),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST,
		this
	);
	
	if ( !selectedFileName.IsEmpty() )
	{
		AnimationTools::C_SceneConfig sceneConfig = C_ResourceCache::Instance().GetSceneConfig();
		
		bool doLoad = true;
		
		if (IsGameSkeletonFilename(selectedFileName.c_str()))
		{
			doLoad = true;
		}
		else
		{			
			C_SkeletonOptionsDialog optionsDialog( sceneConfig, this );
		
			doLoad = optionsDialog.ShowModal() == wxID_OK;
		}
		
		if (doLoad)
		{
			wxBusyCursor busy;
			C_ResourceCache::Instance().SetSceneConfig( sceneConfig );
			
			AnimationTools::C_Status status = gClipPreviewState->LoadGlobalSkeleton( selectedFileName.c_str(), sceneConfig );
			if (status)
			{
				wxFileName fileName( selectedFileName );
				
				C_GlobalSignals::Instance().skeletonLoaded(&gClipPreviewState->GetGlobalSkeleton(), &gClipPreviewState->m_skeletonInstance, &gClipPreviewState->m_pose, fileName.GetName().c_str());
								
				if ( m_constraintsPanel )
				{
					m_constraintsPanel->SetConstraints( &gClipPreviewState->constraints, &gClipPreviewState->GetGlobalSkeleton() );
				}
			}
			else
			{
				::wxMessageBox(status.GetErrorMessage().Chars(), "Error Loading Skeleton");
			}
		}
	}
}

void C_Frame::OnOpenCycle( wxCommandEvent& e )
{
	wxString selectedFileName = ::wxFileSelector(
		wxT( "Select Cycle File" ),
		wxT( "" ),
		wxT( "" ),
		wxT( "" ),
		wxT( "All Supported Files (*.a3d; *.xsi;*.fbx;*.cycle)|*.a3d;*.xsi;*.fbx;*.cycle" ),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST,
		this
	);
	
	if ( !selectedFileName.IsEmpty() )
	{
		AnimationTools::C_SceneConfig sceneConfig = C_ResourceCache::Instance().GetSceneConfig();
		AnimationTools::C_CycleParameters cycleParams = C_ResourceCache::Instance().GetCycleParameters();
		
		bool doLoad = true;
		
		if (IsGameCycleFilename(selectedFileName.c_str()))
		{
			doLoad = true;
		}
		else
		{
			C_CycleOptionsDialog optionsDialog( sceneConfig, cycleParams, this );
			doLoad = (optionsDialog.ShowModal() == wxID_OK);
		}
		
		if ( doLoad )
		{
			wxBusyCursor busy;
			C_ResourceCache::Instance().SetCycleParameters( cycleParams );
			C_ResourceCache::Instance().SetSceneConfig( sceneConfig );
			Xt::DomBlock* cycleBlock = m_animGraphEditor.CreateCycleBlock( m_animGraphEditor.GetCurrentGraphRecord(), selectedFileName.c_str() );
			
			// Set scene config parameters.
			if ( Xt::DomBlock* sceneOptions = Xt::AsBlock( Xt::At( cycleBlock, "SceneOptions") ) )
			{
				if ( Xt::DomField* f = Xt::AsField( Xt::At( sceneOptions, "ScaleFactor" ) ) )
				{
					f->value = Util::C_String::Format( "%g", sceneConfig.scaleFactor ).Chars();
				}
			}
			
			// Set cycle parameters.
			if ( Xt::DomBlock* cycleOptions = Xt::AsBlock( Xt::At( cycleBlock, "CycleOptions") ) )
			{
				if ( Xt::DomField* f = Xt::AsField( Xt::At( cycleOptions, "MaxGlobalRotationError" ) ) )
				{
					f->value = Util::C_String::Format( "%g", cycleParams.maxGlobalRotationError ).Chars();
				}

				if ( Xt::DomField* f = Xt::AsField( Xt::At( cycleOptions, "MaxGlobalTranslationError" ) ) )
				{
					f->value = Util::C_String::Format( "%g", cycleParams.maxGlobalTranslationError ).Chars();
				}
				
				if ( Xt::DomField* f = Xt::AsField( Xt::At( cycleOptions, "RemoveChannelsWithNoKeys" ) ) )
				{
					f->value = cycleParams.removeChannelsWithNoKeys ? "1" : "0";
				}

				if ( Xt::DomField* f = Xt::AsField( Xt::At( cycleOptions, "RemoveTranslationChannelsMatchingBasePose" ) ) )
				{
					f->value = cycleParams.removeTranslationChannelsMatchingBasePose ? "1" : "0";
				}
			}
	
		}
	}
}

void C_Frame::OnReloadChangedFiles( wxCommandEvent& e )
{
	wxBusyCursor busy;
	C_ResourceCache::Instance().ClearReloadFlag();
	C_ResourceCache::Instance().ReloadChangedFiles();
	if ( C_ResourceCache::Instance().GetReloadFlag() )
	{
		if ( AnimationGraph::C_Graph::s_activeGraph )
		{
			AnimationGraph::C_Graph::s_activeGraph->UpdateCustomCurveNames();
		}
	}
}

void C_Frame::OnExportDynamicJointSettings( wxCommandEvent& e )
{
	wxFileName skeletonFileName(C_ResourceCache::Instance().GetSkeletonFilename());

	wxString selectedFileName = ::wxFileSelector(
		wxT( "Export Dynamic Joint Settings" ),
		wxT( "" ),
		skeletonFileName.GetName() + ".txt",
		wxT( "txt" ),
		wxT( "Text Files (*.txt)|*.txt" ),
		wxFD_SAVE, // | wxFD_OVERWRITE_PROMPT,
		this
	);

	if ( !selectedFileName.IsEmpty() )
	{
		wxFile file(selectedFileName, wxFile::write);
		if (!file.IsOpened())
		{
			wxMessageBox(wxString::Format(_T("Error writing to file '%s'"), selectedFileName),
                 _T("Error"),
                 wxOK | wxICON_INFORMATION,
                 this);
			return;
		}

		wxFileOutputStream fileOutputStream(file);
		wxTextOutputStream out(fileOutputStream);

		const Animation::C_SkeletonInstance& skelInst = gClipPreviewState->m_skeletonInstance;
		const AnimationTools::C_SceneConfig& sceneConfig = C_ResourceCache::Instance().GetSceneConfig();
		bool flipHandedness = sceneConfig.flipX ^ sceneConfig.flipY ^ sceneConfig.flipZ;

#define WRITE_PARAM(paramName,value) file.Write(wxString::Format("%s." paramName "=%.4f\n", name, value))
	
		for (U32 i = 0; i < skelInst.GetNumDynamicJoints(); ++i)
		{
			S32 jointIndex = skelInst.GetDynamicJointSkeletonJointIndex(i);
			if (jointIndex < 0) continue;

			Animation::E_DynamicJointType type = skelInst.GetDynamicJointType(i);

			const char* name = skelInst.GetSkeleton()->GetJointName(jointIndex).GetString();

			// Common settings.
			WRITE_PARAM("Mass", skelInst.GetDynamicJointMass(i));
			WRITE_PARAM("AirDampening", skelInst.GetDynamicJointAirResistance(i));
			WRITE_PARAM("WindInfluence", skelInst.GetDynamicJointWindInfluence(i));
			WRITE_PARAM("BuoyancyInfluence", skelInst.GetDynamicJointBuoyancyInfluence(i));

			if (type == Animation::E_DynamicJointType_Aim)
			{
				F32 pitchSpring, pitchDamping, headingSpring, headingDamping;
				skelInst.GetDynamicAimJointSpringConstants(i, &pitchSpring, &pitchDamping, &headingSpring, &headingDamping);

				WRITE_PARAM("HeadingSpringStrength", headingSpring);
				WRITE_PARAM("HeadingSpringDampening", headingDamping);
				WRITE_PARAM("PitchSpringStrength", pitchSpring);
				WRITE_PARAM("PitchSpringDampening", pitchDamping);

				F32 pitchMin, pitchMax, headingMin, headingMax;
				skelInst.GetDynamicAimJointLimits(i, &pitchMin, &pitchMax, &headingMin, &headingMax);

				pitchMin = Maths::RadToDeg(pitchMin);
				pitchMax = Maths::RadToDeg(pitchMax);
				headingMin = Maths::RadToDeg(headingMin);
				headingMax = Maths::RadToDeg(headingMax);

				if (flipHandedness ^ sceneConfig.flipY)
				{
					headingMin = -headingMin;
					headingMax = -headingMax;
					Btl::Swap(headingMin, headingMax);
				}
				
				if (flipHandedness ^ sceneConfig.flipX)
				{
					pitchMin = -pitchMin;
					pitchMax = -pitchMax;
					Btl::Swap(pitchMin, pitchMax);
				}

				WRITE_PARAM("MinHeading", headingMin);
				WRITE_PARAM("MaxHeading", headingMax);
				WRITE_PARAM("MinPitch", pitchMin);
				WRITE_PARAM("MaxPitch", pitchMax);
			}
			else if (type == Animation::E_DynamicJointType_Slider)
			{
				F32 slideMin, slideMax;
				skelInst.GetDynamicSliderJointLimits(i, &slideMin, &slideMax);

				slideMin /= sceneConfig.scaleFactor;
				slideMax /= sceneConfig.scaleFactor;

				WRITE_PARAM("MinSlide", slideMin);
				WRITE_PARAM("MaxSlide", slideMax);

				F32 spring, damping;
				skelInst.GetDynamicSliderJointSpringConstants(i, &spring, &damping);

				WRITE_PARAM("SpringConstant", spring);
				WRITE_PARAM("SpringDampening", damping);
			}
		}

#undef WRITE_PARAM

		SetStatusText(wxString::Format(_T("Saved dynamic joint settings to '%s'."), selectedFileName.c_str()));
	}

}

void C_Frame::OnReloadSchema( wxCommandEvent& e )
{
	wxBusyCursor busy;
	
	const Xt::DomRecord* currentRecord = m_animGraphEditor.GetCurrentGraphRecord();
	XtString selectedRecordName = currentRecord ? currentRecord->name : "";
	
	m_db.Commit();
	m_db.Reopen();
	m_xtTreeView->Populate();
	m_animGraphEditor.SetCurrentGraphRecord( m_animGraphEditor.FindGraphRecord( selectedRecordName.Text() ) );
	m_xtWindow->SetRecord( m_animGraphEditor.GetCurrentGraphRecord() );
	m_xtTreeView->SelectRecord( m_animGraphEditor.GetCurrentGraphRecord() );
}

void C_Frame::OnDebugMenu( wxCommandEvent& e )
{
	switch ( e.GetId() )
	{
		case E_Debug_OpenDbFolder:
			::ShellExecute( GetHwnd(), "open", m_db.GetDbPath(), NULL, NULL, SW_SHOWDEFAULT );
			break;
		
		default:
			break;
	}
}

void C_Frame::OnTogglePaused( wxCommandEvent& e )
{
	gClipPreviewState->paused = !gClipPreviewState->paused;
	m_toolBar->SetToolNormalBitmap( E_Tool_TogglePaused, gClipPreviewState->paused ? m_playImage : m_pauseImage );
}

void C_Frame::OnKeyEvent( wxKeyEvent& e )
{
	AnimationGraph::C_ControllerState& cs = gClipPreviewState->controllerState;
	
	F32 s = e.GetEventType() == wxEVT_KEY_DOWN ? 1.f : 0.f;
	
	switch ( e.GetKeyCode() )
	{
		case 'W': cs.forwardBack = s; break;
		case 'S': cs.forwardBack = -s; break;
		case 'A': cs.leftRight = -s; break;
		case 'D': cs.leftRight = s; break;
		default:
			break;
	}	
}

void C_Frame::OnSceneScaleChanged( wxCommandEvent& e )
{
	F32 scale = static_cast<F32>(atof( m_sceneScaleText->GetValue().c_str() ) );
	if ( scale < 0.f )
	{
		scale = 1.f;
		m_sceneScaleText->ChangeValue( wxString::Format("%g", scale) );
	}
	gClipPreviewState->SetSceneScale( scale <= 0.f ? 1.f : scale );
}

void C_Frame::OnUpdateUI( wxUpdateUIEvent& e )
{
	switch ( e.GetId() )
	{
		case E_File_ExportDynamicJointSettings:
			e.Enable( gClipPreviewState->m_skeletonInstance.HasDynamicJoints() );
			break;

		case E_Character_FreezeRoot:
			e.Check( gClipPreviewState->freezeRoot );
			break;
			
		case E_Character_AbsoluteRootMotion:
			e.Check( gClipPreviewState->absoluteRootMotion );
			break;
			
		case E_Character_EliminateRedundantPushPops:
			e.Check( gClipPreviewState->enableCommandListPushPopElimination );
			break;
			
		case E_View_ShowSkeleton:
			e.Check( gClipPreviewState->showSkeleton );
			break;
		case E_View_ShowJointNames:
			e.Check( gClipPreviewState->showJointNames );
			break;
		case E_View_ShowOnlySelectedJointName:
			e.Check( gClipPreviewState->showOnlySelectedJointName );
			e.Enable( gClipPreviewState->showJointNames );
			break;
			
		case E_View_ShowConstraints:
			e.Check( gClipPreviewState->showConstraints );
			break;
			
		case E_View_HideUnselectedJoints:
			e.Check( gClipPreviewState->hideUnselectedJoints );
			e.Enable( gClipPreviewState->showSkeleton );
			break;
		case E_View_ShowSkinnedMesh:
			e.Check( gClipPreviewState->showSkinnedMesh );
			break;
		case E_View_ShowWireframe:
			e.Check( gClipPreviewState->showWireframe );
			break;
		case E_View_ShowLogWindow:
			e.Check( IsPaneShown( m_pText ) );
			break;
		case E_View_ShowToolsWindow:
			e.Check( IsPaneShown( m_notebook ) );
			break;
		case E_View_ShowPropertiesWindow:
			e.Check( IsPaneShown( m_propertiesNotebook ) );
			break;	
		case E_View_ShowCurveWindow:
			e.Check( IsPaneShown( m_curveViewerPanel ) );
			break;
		case E_View_HudShowCustomCurves:
			e.Check( gClipPreviewState->hudShowCustomCurves );
			break;
		case E_View_HudShowCommandList:
			e.Check( gClipPreviewState->hudShowCommandList );
			break;
			
		case E_View_HudShowSelectedJointTransform:
			e.Check( gClipPreviewState->hudShowSelectedJointTransform );
			break;
			
		case E_Tool_PlaybackSpeedText:
			// dangerous if user is editing this!
			//e.SetText( wxString::Format( "%g", gClipPreviewState->playbackSpeed ) );
			break;
			
		default:
			e.Skip();
			break;
	}
}

void C_Frame::OnPlaybackSpeedSliderScrolled( wxCommandEvent& e )
{
	gClipPreviewState->playbackSpeed = m_playbackSpeedSlider->GetValue() / 50.f;
	m_playbackSpeedText->ChangeValue( wxString::Format( "%g", gClipPreviewState->playbackSpeed ) );
}

void C_Frame::OnPlaybackSpeedTextChanged( wxCommandEvent& e )
{
	// slider goes from 0-2.
	double value = 0.0;
	if ( e.GetString().ToDouble( &value ) )
	{
		F32 fValue = static_cast<F32>(value);
		gClipPreviewState->playbackSpeed = fValue;
		m_playbackSpeedSlider->SetValue( Maths::Clamped( static_cast<S32>(fValue * 50.f), 0, 100 ) );
	}
}

void C_Frame::ShowCurvesForCycle( const AnimationTools::C_CycleBuilder* cycle )
{
#ifdef ENABLE_CURVE_VIEWER
	m_curveViewerPanel->SetCycle( cycle );
#endif
}

void C_Frame::SetPlaybackTime( F32 time )
{
#ifdef ENABLE_CURVE_VIEWER
	m_curveViewerPanel->SetPlaybackTime( time );
#endif
}

void C_Frame::OnXtDatabaseChanged( Xt::DomEvent* domEvent )
{
	RequestDisplayUpdate();
}

void C_Frame::RequestDisplayUpdate()
{
	if ( m_animationPreviewWindow )
	{
		m_animationPreviewWindow->AddPendingEvent( wxCommandEvent( wxEVT_REFRESH_DISPLAY_WINDOW ) );
	}
}

void C_Frame::PopulateToolBar( wxToolBar* toolBar )
{
    wxImage img;
    img.LoadFile( "./icons/Skeleton16x16.png" );
    wxToolBarToolBase* tool = NULL;
    
	toolBar->SetToolBitmapSize(wxSize(16,16));
	
	//toolBar->SetMargins( 4, 4 );

	tool = toolBar->AddTool( E_Tool_OpenSkeleton, _T("Open Skeleton..."), img, img, wxITEM_NORMAL, _T("Open Skeleton") );
	Connect( tool->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(C_Frame::OnOpenSkeleton) );
	img.LoadFile( "./icons/Cycle16x16.png" );
	tool = toolBar->AddTool( E_Tool_OpenCycle, _T("Open Cycle..."), img, img, wxITEM_NORMAL, _T("Open Cycle") );
	Connect( tool->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(C_Frame::OnOpenCycle) );

	toolBar->AddSeparator();
	
	// Playback speed slider.
	// Playback Speed: |-------|------| 2.1x
	toolBar->AddControl( new wxStaticText( toolBar, wxID_ANY, _T("Playback Speed:") ) );
	m_playbackSpeedSlider = new wxSlider( toolBar, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxHORIZONTAL );
	tool = toolBar->AddControl( m_playbackSpeedSlider );
	
	Connect( tool->GetId(), wxEVT_SCROLL_THUMBTRACK, wxCommandEventHandler(C_Frame::OnPlaybackSpeedSliderScrolled) );
	Connect( tool->GetId(), wxEVT_SCROLL_TOP, wxCommandEventHandler(C_Frame::OnPlaybackSpeedSliderScrolled) );
	Connect( tool->GetId(), wxEVT_SCROLL_BOTTOM, wxCommandEventHandler(C_Frame::OnPlaybackSpeedSliderScrolled) );
	Connect( tool->GetId(), wxEVT_SCROLL_LINEUP, wxCommandEventHandler(C_Frame::OnPlaybackSpeedSliderScrolled) );
	Connect( tool->GetId(), wxEVT_SCROLL_LINEDOWN, wxCommandEventHandler(C_Frame::OnPlaybackSpeedSliderScrolled) );
	Connect( tool->GetId(), wxEVT_SCROLL_PAGEUP, wxCommandEventHandler(C_Frame::OnPlaybackSpeedSliderScrolled) );
	Connect( tool->GetId(), wxEVT_SCROLL_PAGEDOWN, wxCommandEventHandler(C_Frame::OnPlaybackSpeedSliderScrolled) );
	Connect( tool->GetId(), wxEVT_SCROLL_CHANGED, wxCommandEventHandler(C_Frame::OnPlaybackSpeedSliderScrolled) );
			
	//Connect( tool->GetId(), wxEVT_SCROLL_THUMBTRACK, wxCommandEventHandler(C_Frame::OnPlaybackSpeedSliderScrolled) );
	m_playbackSpeedText = new wxTextCtrl( toolBar, E_Tool_PlaybackSpeedText, _T("1.00"), wxDefaultPosition, wxSize(50, 32), wxBORDER_SUNKEN | wxTE_CENTRE );
	m_playbackSpeedText->SetToolTip( _T("Set playback speed scale factor" ) );
	
	Connect( m_playbackSpeedText->GetId(), wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(C_Frame::OnPlaybackSpeedTextChanged) );
		
	tool = toolBar->AddControl( m_playbackSpeedText );
	Connect( E_Tool_PlaybackSpeedText, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_Frame::OnUpdateUI) );

	m_pauseImage.LoadFile( "icons/pause.png" );
	m_playImage.LoadFile( "icons/play.png" );
	
	tool = toolBar->AddTool( E_Tool_TogglePaused, _T(""), m_pauseImage, m_pauseImage, wxITEM_NORMAL, _T("Pause/Play") );
	Connect( E_Tool_TogglePaused, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(C_Frame::OnTogglePaused) );
	
	
	toolBar->Realize();
	
	toolBar->Refresh();
}

void C_Frame::TogglePane( wxWindow* win )
{
	wxAuiPaneInfo& paneInfo = m_pAuiManager->GetPane( win );
	
	assert( paneInfo.IsOk() );
	
	if ( paneInfo.IsOk() )
	{
		paneInfo.Show( !paneInfo.IsShown() );
		
		m_pAuiManager->Update();
	}
}

bool C_Frame::IsPaneShown( wxWindow* win ) const
{
	wxAuiPaneInfo& paneInfo = m_pAuiManager->GetPane( win );
	
	return paneInfo.IsOk() && paneInfo.IsShown();
}

void C_Frame::ReadConfig()
{
	// Read config data.
	wxConfig::Get()->Read( "Character/FreezeRoot", &gClipPreviewState->freezeRoot );
	wxConfig::Get()->Read( "View/Skeleton", &gClipPreviewState->showSkeleton );
	wxConfig::Get()->Read( "View/JointNames", &gClipPreviewState->showJointNames );
	wxConfig::Get()->Read( "View/SelectedJointOnly", &gClipPreviewState->showOnlySelectedJointName );
	wxConfig::Get()->Read( "View/HideUnselectedJoints", &gClipPreviewState->hideUnselectedJoints );
	wxConfig::Get()->Read( "View/SkinnedMesh", &gClipPreviewState->showSkinnedMesh );
	wxConfig::Get()->Read( "View/Wireframe", &gClipPreviewState->showWireframe );
	wxConfig::Get()->Read( "View/CommandList", &gClipPreviewState->hudShowCommandList );
	wxConfig::Get()->Read( "View/CustomCurves", &gClipPreviewState->hudShowCustomCurves );
	bool showConstraintsWindow = m_constraintsPanel->IsShown();
	wxConfig::Get()->Read( "View/ShowConstraintsWindow", &showConstraintsWindow );
	F64 scale = gClipPreviewState->GetSceneScale();
	wxConfig::Get()->Read( "Import/SceneScale", &scale );
	gClipPreviewState->SetSceneScale( (F32) scale );
	
	bool foundRecord = false;
	Xt::DomRecord* graphRecord = NULL;
	wxString graphRecordName = "";
	if ( wxConfig::Get()->Read( "Graph/CurrentRecord", &graphRecordName ) )
	{
		graphRecord = m_animGraphEditor.FindGraphRecord( graphRecordName );
	}
	
	if ( graphRecord != NULL )
	{
		m_animGraphEditor.SetCurrentGraphRecord( graphRecord );
	}
	
	//m_pAuiManager->GetPane(m_constraintsPanel).Show(showConstraintsWindow);
	m_pAuiManager->Update();
	
		
	//wxConfig::Get()->Write( "View/LogWindow", m_pText->IsShown() );
	//wxConfig::Get()->Write( "View/SkeletonWindow", m_skeletonPanel->IsShown() );
	//wxConfig::Get()->Write( "View/NodeGraphWindow", m_xtWindow->IsShown() );
	
	// Keeps crashing
	//wxString persp;
	//if (wxConfig::Get()->Read( "Windows/Layout", &persp ) )
	//{
	//	m_pAuiManager->LoadPerspective( persp, true );
	//}
}

void C_Frame::WriteConfig()
{
	wxConfig::Get()->Write( "Character/FreezeRoot", gClipPreviewState->freezeRoot );
	wxConfig::Get()->Write( "View/Skeleton", gClipPreviewState->showSkeleton );
	wxConfig::Get()->Write( "View/JointNames", gClipPreviewState->showJointNames );
	wxConfig::Get()->Write( "View/SelectedJointOnly", gClipPreviewState->showOnlySelectedJointName );
	wxConfig::Get()->Write( "View/HideUnselectedJoints", gClipPreviewState->hideUnselectedJoints );
	wxConfig::Get()->Write( "View/SkinnedMesh", gClipPreviewState->showSkinnedMesh );
	wxConfig::Get()->Write( "View/Wireframe", gClipPreviewState->showWireframe );
	wxConfig::Get()->Write( "View/CommandList", gClipPreviewState->hudShowCommandList );
	wxConfig::Get()->Write( "View/CustomCurves", gClipPreviewState->hudShowCustomCurves );
	wxConfig::Get()->Write( "View/SelectedJointTransform", gClipPreviewState->hudShowSelectedJointTransform );
	wxConfig::Get()->Write( "View/ShowConstraintsWindow",  m_constraintsPanel->IsShown());
	wxConfig::Get()->Write( "Import/SceneScale", gClipPreviewState->GetSceneScale() );
	//wxConfig::Get()->Write( "View/LogWindow", m_pText->IsShown() );
	//wxConfig::Get()->Write( "View/SkeletonWindow", m_skeletonPanel->IsShown() );
	//wxConfig::Get()->Write( "View/NodeGraphWindow", m_xtWindow->IsShown() );

	//wxConfig::Get()->Write( "Windows/Layout", m_pAuiManager->SavePerspective() );
	
	Xt::DomRecord* graphRecord = m_animGraphEditor.GetCurrentGraphRecord();
	wxConfig::Get()->Write( "Graph/CurrentRecord", graphRecord ? graphRecord->name.Text() : "" );
}
