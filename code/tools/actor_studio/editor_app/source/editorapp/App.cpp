#include "stdafx.h"
#include "minmax.h"
#include <gdiplus.h>

#include <algorithm>

#include <Python.h>

#include <wx/aui/aui.h>
#include <wx/treectrl.h>
#include <wx/dialog.h>
#include <wx/cmdline.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/propdev.h>
#include <wx/propgrid/advprops.h>
#include <wx/propgrid/manager.h>
#include <wx/evtloop.h>
#include <wx/ptr_scpd.h>

#include <System/misc/StringUtil.h>
#include <System/memory/MemoryTrackerDefines.h>
#include <System/memory/MemoryTracker.h>
#include <System/memory/AllocatorFunctions.h>
#include <system/debug/buildstamp.h>
#include <system/xml/Xml.h>

#include <xtwin/ASyncOpWin.h>
#include <xtwin/SendEMail.h>
#include <xtwin/TimelineChart.h>
#include <xtwin/SvnXt.h>
#include <xtwin/TreeCtrl.h>

#include <Tools_Shared/MiniDumper.h>
#include <Tools_Shared/ToolsAssert.h>
#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>
//Slightly guerilla memory tracking for finding leaks.
#include <Tools_Shared/memorydbg.h>
#include <Tools_Shared/Utility/MacroExpander.h>

#include <EditorLib/Aftk/AftkRefHelpers.h>
#include <EditorLib/Context/EditorContext.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Misc/AsyncHelpers.h>
#include <EditorLib/Misc/Status.h>
#include <EditorLib/Network/NetworkManager.h>
#include <EditorLib/Python/PythonManager.h>
#include <EditorLib/Python/PythonHelpers.h>
#include <EditorLib/Xt/XtTypeDatabaseHelpers.h>
#include <EditorLib/Xt/XtVisualisation.h>
#include <EditorLib/Xt/XtUserData.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>
#include <EditorLib/CoreConfig.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Level/XtLevel.h>

#include <UserNode/Register.h>
#include <UserCmd/Register.h>

#include <EditorApp/Command/Register.h>

#include <EditorApp/Misc/Console.h>
#include <EditorApp/App.h>
#include <EditorApp/EventLoop.h>
#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Config.h>

#include <EditorApp/Editor/RenderCore.h>

#include <EditorApp/Editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/Editor/manipulators/contexts/IVectorContext.h>
#include <EditorApp/Editor/manipulators/contexts/IOrientationContext.h>
#include <EditorApp/Editor/manipulators/contexts/MoveSceneNodeContext.h>
#include <EditorApp/Editor/manipulators/contexts/RotateSceneNodeContext.h>
#include <EditorApp/Editor/manipulators/contexts/ScaleSceneNodeContext.h>
#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/manipulators/ActorCreateManipulator.h>
#include <EditorApp/Editor/manipulators/DropperSelectManipulator.h>
#include <EditorApp/Editor/manipulators/SplineCreateManipulator.h>
#include <EditorApp/Editor/manipulators/PolyCreateManipulator.h>
#include <EditorApp/Editor/manipulators/SelectManipulator.h>
#include <EditorApp/Editor/manipulators/MoveManipulator.h>
#include <EditorApp/Editor/manipulators/RotateManipulator.h>
#include <EditorApp/Editor/manipulators/ScaleManipulator.h>
#include <EditorApp/Editor/manipulators/ManipulatorContextFactory.h>
#include <EditorApp/Input_Handlers/ShortcutHandler.h>

#include <EditorApp/Components/IView.h>
#include <EditorApp/Components/SceneNodePropertyPanel.h>
#include <EditorApp/Components/CameraPropertyPanel.h>
#include <EditorApp/Components/LevelInfoPanel.h>
#include <EditorApp/Components/Log.h>
#include <EditorApp/Components/GroupEditPanel.h>
#include <EditorApp/Components/ReferenceDialog.h>
#include <EditorApp/Components/HypergraphDialog.h>
#include <EditorApp/Components/AnimationDialog.h>
#include <EditorApp/Components/ManipulatorPropertyPanel.h>
#include <EditorApp/Components/SorceryDialog.h>
#include <EditorApp/Components/SorceryHypergraphPanel.h>
#include <EditorApp/Components/DynamicObjectDialog.h>
#include <EditorApp/Components/VerificationPanel.h>
#include <EditorApp/Components/Layer/LayerPropertyPanel.h>
#include <EditorApp/Components/Layer/LayerEditPanel.h>
#include <EditorApp/Components/scarab_form/WxScarabForm.h>
#include <EditorApp/Components/Script_Log/ScriptLog.h>

#include <EditorApp/Toolbars/ActorToolPanel.h>
#include <EditorApp/Toolbars/ManipulatorToolBar.h>
#include <EditorApp/Toolbars/SelectionToolPanel.h>
#include <EditorApp/Toolbars/SnapToolBar.h>
#include <EditorApp/Toolbars/DisplayToolBar.h>
#include <EditorApp/Toolbars/StandardToolBar.h>

#include <EditorApp/Properties/CameraProperties.h>
#include <EditorApp/Properties/ManipulatorProperties.h>

#include <EditorApp/Python/EditorFuncs.h>

#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>

#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Tasks/UITasks.h>

#include <EditorApp/UserData/UserData.h>
		

namespace Editor
{
	IMPLEMENT_APP(C_App)

	wxDEFINE_TIED_SCOPED_PTR_TYPE(C_EventLoop);

	void AppYield()
	{
		wxSafeYield();
	}
}

using namespace Editor;


void XtLoadDbProgress(float progress, void* context)
{
}



bool C_App::OnInit()
{
	XtLogInit("Z:\\Tools\\Temp\\Logs\\ActorStudio", "Xt");

	//override internal wx message displayer to use a stderr version until
	//our application type is known
	wxMessageOutput *pOutput = new wxMessageOutputDebug();
	wxMessageOutput::Set(pOutput);

    if (!wxApp::OnInit())
        return false;

	{
		LOGINFO(g_Log, "Initialising ActorStudio\n");

		const char* buildTime = Debug::GetBuildTime();
		const char* buildDate = Debug::GetBuildDate();
		const char* buildUser = Debug::GetBuildUser();
		const char* buildPc = Debug::GetBuildPc();
		const char* buildRevision = Debug::GetBuildRevision();

		LOGINFO(g_Log, "ActorStudio Revision %s\n\nBuilt by %s on %s\nBuild time:  %s %s\n", buildRevision, buildUser, buildPc, buildDate, buildTime );
	}

	//_crtBreakAlloc = 12448;
	//_CrtSetBreakAlloc(12448);

	m_pFrame = 0;
	m_pScriptLogPanel = 0;
	m_pLevelInfoPanel = 0;
	m_pReferencePanel = 0;
	m_pSorceryHypergraphPanel = 0;
	m_pHypergraphPanel = 0;
	m_pAnimationPanel = 0;
	m_pLayerEditPanel = 0;
	m_pSceneNodePropertyPanel = 0;
	m_pCameraPropertyPanel = 0;
	m_pManipulatorPropertyPanel = 0;
	m_pLayerPropertyPanel = 0;
	m_pSorceryPanel = 0;
	m_pDynamicObjectPanel = 0;
	m_pStandardToolBar = 0;
	m_pActorTypeToolPanel = 0;
	m_pXtDynamicForm = 0;
	m_pShortcutHandler = 0;
	m_flags.isDirty = true;
	m_mainLevelId = EditorLib::kXtInvalidId;

	Gdiplus::GdiplusStartupInput	gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	SetVendorName(kVendorName);

	Tools::InitialiseMemoryDbg();

	Btl::String configFilePath = m_EditorConfig.configPath + "/editor_config.xml";
	if (!LoadConfig(configFilePath.Chars(), m_EditorConfig.macroFilePath.Chars(), m_EditorConfig))
	{
		LOGERROR(g_Log, "Unable to open editor_config.xml");
		return false;
	}

	if (m_EditorConfig.appType != EditorLib::E_AppType_Interactive)
		CreateConsole();
	else
	{
		wxMessageOutput *pOldOutput = wxMessageOutput::Set(new wxMessageOutputLog());
		delete pOldOutput;
	}


	m_pDummyToolsApp = new C_App::C_DummyToolsApp(m_EditorConfig.dumpPath.Chars());
	m_pDummyToolsApp->Run(0, NULL, false);

	Tasks::InstallWndProcHook();

	Aftk::C_Global::Create();

	EditorLib::C_Status status = EditorLib::C_Core::Instance().Initialise(
		m_EditorConfig.appType,
		configFilePath);
	if (!status)
	{
		LOGERROR(g_Log, "ActorStudio core failed to initialise");
		OnExit();
		return false;
	}

	if (m_EditorConfig.appType == EditorLib::E_AppType_Interactive)
	{
		m_pFrame = new C_Frame(
			_T(kEditorName),
			wxDefaultPosition,
			wxSize(800,600));
		if (!m_pFrame)
		{
			OnExit();
			return false;
		}

		if (!InitialiseManipulatorContexts())
		{
			OnExit();
			return false;
		}
	}

	if (wxGetApp().GetFrame())
		EditorLib::C_Core::Instance().SetWindowHandle((HWND)wxGetApp().GetFrame()->GetHandle());

	m_slots.Connect();

	const EditorLib::C_CoreConfig& coreConfig = EditorLib::C_Core::Instance().GetConfig();

	if (m_EditorConfig.appType == EditorLib::E_AppType_Interactive)
	{
		EditorLib::C_Core::Instance().GetXtVisualisationData().RegisterDefinitions(
			(coreConfig.configPath + "editor_visualisaton.xml").Chars());

		if (!InitialiseUI())
		{
			OnExit();
			return false;
		}
	}

	status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute(
		"xtdb", Btl::String("-o ") + 
		(m_EditorConfig.xtdbOverridePath.Length() ? m_EditorConfig.xtdbOverridePath : coreConfig.xtDbPath));
	if (!status)
	{
		HWND window = m_EditorConfig.appType == EditorLib::E_AppType_Interactive ?
			(HWND)wxGetApp().GetFrame()->GetHandle() : 0;
		XtString xtStringTest = (m_EditorConfig.xtdbOverridePath.Length() ? m_EditorConfig.xtdbOverridePath.Chars() : coreConfig.xtDbPath.Chars());
		bool attemptRecovery = false;

		if (m_EditorConfig.appType != EditorLib::E_AppType_Interactive)
		{
			//Automatically attempt recovery in batch mode.
			attemptRecovery = true;
		}
		else if (MessageBox(window,
			"ActorStudio failed to load the database:\r\n\r\n" +
				xtStringTest +
				"\r\n\r\nWould you like to attempt a recovery to fix the problem?",
				_T("ActorStudio"),
				MB_YESNO | MB_ICONQUESTION
			) == IDYES)
		{
			attemptRecovery = true;
		}

		if (!attemptRecovery)
		{
			LOGERROR(g_Log, "Failed to open Xt database from %s", xtStringTest.Text());
			OnExit();
			return false;
		}

		SvnXtParams svnXtParams(
			EditorLib::C_Core::Instance().GetXtManager().GetDb(), xtStringTest);
		svnXtParams.noWindow = !window ? true : false;
		if (!SvnXtRecover(window, &svnXtParams)) 
		{
			LOGERROR(g_Log, "Failed to recover Xt database from %s", xtStringTest.Text());
			OnExit();
			return false;
		}
	}

	EditorLib::AftkRefHelpers::LoadAllReferences(false, true, &Editor::AppYield);

	UserNode::C_Register::Instance().Initialise();

	if (!Editor::C_Register::Instance().Initialise(coreConfig.projectName))
	{
		OnExit();
		return false;
	}

	if (!EditorUserCmd::C_Register::Instance().Initialise(coreConfig.projectName))
	{
		OnExit();
		return false;
	}

	// Register editor python methods
	{
		EditorLib::C_Core::Instance().GetPythonManager().RegisterMethods("editor", EditorFuncs::GetEdHelperModuleDefs());
	}

	//Run app startup script
	{
		int result = EditorLib::PythonHelpers::ExecFile("./scripts/editor/startup/boot.py");
		if (result != 0)
			LOGERROR(g_Log, "Failed to execute app boot script");
	}

	// Run project startup script
	{
		Btl::String projectScript = coreConfig.configPath + "/scripts/boot.py";
		int result = EditorLib::PythonHelpers::ExecFile(projectScript.Chars());
		if (result != 0)
			LOGERROR(g_Log, "Failed to execute app boot script");
	}

	if (m_EditorConfig.startupCommand.Length() != 0)
	{
		//Run user startup command
		int result = PyRun_SimpleString(m_EditorConfig.startupCommand.Chars());
		if (result != 0)
			LOGERROR(g_Log, "Failed to execute %s", m_EditorConfig.startupCommand.Chars());
	}
	if (m_EditorConfig.startupScript.Length() != 0)
	{
		//Run user startup script
		int result = EditorLib::PythonHelpers::ExecFile(m_EditorConfig.startupScript.Chars());
		if (result != 0 && m_EditorConfig.appType == EditorLib::E_AppType_Batch)
			return false;
	}

	if (m_EditorConfig.appType == EditorLib::E_AppType_Interactive)
	{
		//Connect timers
		Connect(m_DirtyTimer.GetId(), wxEVT_TIMER, wxTimerEventHandler(C_App::OnTimer));

		Connect(m_NetworkTimer.GetId(), wxEVT_TIMER, wxTimerEventHandler(C_App::OnTimer));

		Connect(m_AutosaveTimer.GetId(), wxEVT_TIMER, wxTimerEventHandler(C_App::OnTimer));
		SetAutosaveInterval(Tasks::GetAutosaveInterval());

		Tasks::ImportUnsavedChanges();
	}

	return true;
}

int C_App::MainLoop()
{
	if (m_EditorConfig.appType == EditorLib::E_AppType_Interactive)
	{
		C_EventLoopTiedPtr mainLoop((C_EventLoop **)&m_mainLoop, new C_EventLoop); 
		return m_mainLoop->Run(); 
	}

	return 0;
}

int C_App::OnExit()
{
	if (!IsDebuggerPresent())
	{
		//Quick exit.
		EditorLib::C_Core::Instance().Quit();
	}

	if (m_pScriptLogPanel)
		m_pScriptLogPanel->Detach();
	m_pFrame = 0;
	m_pScriptLogPanel = 0;
	m_pLevelInfoPanel = 0;
	m_pCameraPropertyPanel = 0;
	m_pManipulatorPropertyPanel = 0;
	
	m_slots.Disconnect();

	UserNode::C_Register::Instance().Shutdown();
	if (m_EditorConfig.appType == EditorLib::E_AppType_Interactive)
	{
		//Save UI state.
		UninitialiseUI();
		C_RenderCore::Instance().Shutdown();
	}
	EditorLib::C_Core::Instance().Shutdown();
	Aftk::C_Global::Destroy();

	SAFE_DELETE_PTR(m_pDummyToolsApp);

	if (m_EditorConfig.appType == EditorLib::E_AppType_Interactive)
	{
		Disconnect(m_DirtyTimer.GetId(), wxEVT_TIMER, wxTimerEventHandler(C_App::OnTimer));
		Disconnect(m_NetworkTimer.GetId(), wxEVT_TIMER, wxTimerEventHandler(C_App::OnTimer));
		Disconnect(m_AutosaveTimer.GetId(), wxEVT_TIMER, wxTimerEventHandler(C_App::OnTimer));
	
		DestroyConsole();
	}

	FreeBitmapCache();
	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	return 0;
}

static const wxCmdLineEntryDesc g_cmdLineDesc[] =
{
    { wxCMD_LINE_PARAM,	NULL, NULL,		"config file path", wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY },
	{ wxCMD_LINE_PARAM,	NULL, NULL,		"macro file path", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_SWITCH,"b", "batch",	"run in batch mode", wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_OPTION,"c", "command",	"execute given python string", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_OPTION,"d", "xtdb","override the xtdb location given in the config", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_OPTION,"s", "script",	"execute given python file", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_SWITCH,"h", "help",	"displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },

    { wxCMD_LINE_NONE }
};

void C_App::OnInitCmdLine(wxCmdLineParser& parser)
{
	m_flags.postLevelLoadPending = false;
    parser.SetDesc (g_cmdLineDesc);
    // must refuse '/' as parameter starter or cannot use "/path" style paths
    parser.SetSwitchChars (wxT("-"));
}
 
bool C_App::OnCmdLineParsed(wxCmdLineParser& parser)
{
	wxString param0 = parser.GetParam(0);
	m_EditorConfig.configPath = Misc::RemoveTrailingSlash(param0.c_str());
	Tools::C_MacroExpander::Instance().Expand(m_EditorConfig.configPath);

	if (parser.GetParamCount() > 1)
	{
		wxString param1 = parser.GetParam(1);
		m_EditorConfig.macroFilePath = Misc::RemoveTrailingSlash(param1.c_str());
		Tools::C_MacroExpander::Instance().Expand(m_EditorConfig.macroFilePath);
	}

	if (parser.Found("batch"))
		m_EditorConfig.appType = EditorLib::E_AppType_Batch;
	else
		m_EditorConfig.appType = EditorLib::E_AppType_Interactive;

	{
		wxString value;
		if (parser.Found("command", &value))
		{
			m_EditorConfig.startupCommand = value.c_str();
		}
		else if (parser.Found("script", &value))
		{
			m_EditorConfig.startupScript = value.c_str();
		}

		if (parser.Found("xtdb", &value))
			m_EditorConfig.xtdbOverridePath = value.c_str();
	}

    return true;
}

void C_App::UpdateTitleBar(const EditorLib::I_Level* pLevel)
{
	wxString namePrefix;
	if (pLevel)
	{
		namePrefix = pLevel->GetName().Chars();
		namePrefix += " - ";
	}

	namePrefix += kEditorName;

	if (EditorLib::C_Core::Instance().GetNetworkManager().IsWorkingOffline())
	{
		namePrefix += " - (working offline)";
	} else {
		if (EditorLib::C_Core::Instance().GetNetworkManager().IsConnected())
		{
			namePrefix += " - ";
			namePrefix += EditorLib::C_Core::Instance().GetNetworkManager().TargetName();
		}
		else
			namePrefix += " - (disconnected)";
	}
	
   	if (wxGetApp().m_pFrame)
   		wxGetApp().m_pFrame->SetLabel(namePrefix);
}

bool C_App::InitialiseManipulatorContexts()
{
	//scenenode manipulator contexts
	if (!C_ManipulatorContextFactory::Instance().Register<C_MoveSceneNodeContext>()) return false;
	if (!C_ManipulatorContextFactory::Instance().Register<C_RotateSceneNodeContext>()) return false;
	//if (!C_ManipulatorContextFactory::Instance().Register<C_ScaleSceneNodeContext>()) return false;

	return true;
}

bool C_App::InitialiseUI()
{
	wxInitAllImageHandlers();

	//wire up the window close event
	wxConnect<wxEVENT_CLOSE_WINDOW>(m_pFrame, &C_App::C_Slots::OnQuitSlot, &m_slots, false, m_pFrame->GetId());

	//wire up window creation handler (to copy accelerator table to child frames)
	Connect(wxEVT_CREATE, wxWindowCreateEventHandler(C_App::OnWindowCreate), NULL, this);

	//create our custom log subscriber . wxWidgets will handle the clean up
	m_pScriptLogPanel = new C_ScriptLogPanel(m_pFrame);

	const EditorLib::C_CoreConfig& coreConfig = EditorLib::C_Core::Instance().GetConfig();

	if (!m_pFrame->Initialise(coreConfig.projectName.Chars()))
		return false;

	//add a log window
	m_pFrame->AddUIPane(
		m_pScriptLogPanel,
		wxAuiPaneInfo().Name(_T("Pane_Log")).Caption(_T("Log Window")).
		Bottom().Layer(0).Position(0).CloseButton(true).MaximizeButton(false).PinButton(false).Show(true));

	//add the level info panel
	m_pLevelInfoPanel = new C_LevelInfoPanel(m_pFrame, wxID_ANY, wxDefaultPosition, wxSize(200,400), wxNO_BORDER);
	m_pFrame->AddUIPane(m_pLevelInfoPanel, wxAuiPaneInfo().
		Name(wxT("Pane_LevelInfo")).Caption(wxT("Level Information")).
		Left().Layer(0).Position(0).CloseButton(true).MaximizeButton(false));

	m_pXtDynamicForm = new C_WxScarabForm(
		m_pFrame,
		wxID_ANY, 
		wxDefaultPosition,
		wxDefaultSize);

	m_views.PushBack(m_pXtDynamicForm);
	
	m_pFrame->AddUIPane(
		m_pXtDynamicForm, 
		wxAuiPaneInfo().
		Name(wxT("Pane_Properties")).
		Caption(wxT("Node Properties")).
		Right().
		Layer(0).
		Position(0).
		CloseButton(true).
		MaximizeButton(false).
		Float().
		Show(true));

	m_pReferencePanel = new C_ReferencePanel(m_pFrame, wxID_ANY, wxDefaultPosition);
	m_pFrame->AddUIPane(m_pReferencePanel, wxAuiPaneInfo().
		Name(wxT("Pane_Reference")).Caption(wxT("References")).
		Left().Layer(0).Position(0).CloseButton(true).MaximizeButton(false).Float().Show(false));

	m_pSorceryHypergraphPanel = new C_SorceryHypergraphPanel(m_pFrame, wxID_ANY, wxDefaultPosition);
	m_pFrame->AddUIPane(m_pSorceryHypergraphPanel, wxAuiPaneInfo().
		Name(wxT("Pane_SorceryHypergraph")).Caption(wxT("Sorcery Hypergraph")).
		Left().Layer(0).Position(0).CloseButton(true).MaximizeButton(false).Float().Show(false));

	m_pHypergraphPanel = new C_HypergraphPanel(m_pFrame, wxID_ANY, wxDefaultPosition);
	m_pFrame->AddUIPane(m_pHypergraphPanel, wxAuiPaneInfo().
		Name(wxT("Pane_Hypergraph")).Caption(wxT("Hypergraph")).
		Left().Layer(0).Position(0).CloseButton(true).MaximizeButton(true).Float().Show(false));

	m_pAnimationPanel = new C_AnimationPanel(m_pFrame, wxID_ANY, wxDefaultPosition);
	m_pFrame->AddUIPane(m_pAnimationPanel, wxAuiPaneInfo().
		Name(wxT("Pane_Animation")).Caption(wxT("Animation")).
		Left().Layer(0).Position(0).CloseButton(true).MaximizeButton(true).Float().Show(false));

	m_pGroupEditPanel = new C_GroupEditPanel(m_pFrame, wxID_ANY, wxDefaultPosition);
	m_pFrame->AddUIPane(m_pGroupEditPanel,
		wxAuiPaneInfo().Name(_T("Pane_GroupEdit")).Caption(_T("Group Editor")).
		Right().Layer(0).Position(0).CloseButton(true).MaximizeButton(true).Float().Show(false));

	m_pVerificationPanel = new C_VerificationPanel(m_pFrame, wxID_ANY, wxDefaultPosition);
	m_pFrame->AddUIPane(m_pVerificationPanel,
		wxAuiPaneInfo().Name(_T("Pane_Verification")).Caption(_T("Verification Summary")).
		Right().Layer(0).Position(0).CloseButton(true).MaximizeButton(true).Float().Show(false));

	m_pDopeSheetPanel = new C_DopeSheetPanel(m_pFrame, wxID_ANY, wxDefaultPosition);
	m_pFrame->AddUIPane(m_pDopeSheetPanel, wxAuiPaneInfo().
		Name(wxT("Pane_AnimationDopeSheet")).Caption(wxT("Animation Dope Sheet")).
		Left().Layer(0).Position(0).CloseButton(true).MaximizeButton(true).Float().Show(false),
		Editor::E_UIPanelType_Unknown);

	m_pLayerEditPanel = new C_LayerEditPanel(m_pFrame, wxID_ANY, wxDefaultPosition, wxSize(300, 500), wxNO_BORDER);
	m_pFrame->AddUIPane(m_pLayerEditPanel,
		wxAuiPaneInfo().Name(_T("Pane_LayerEdit")).Caption(_T("Layer Editor")).
		Right().Layer(0).Position(0).CloseButton(true).MaximizeButton(false).PinButton(false).Float().Show(false),
		Editor::E_UIPanelType_Unknown);

	m_pLayerPropertyPanel = new C_LayerPropertyPanel(m_pFrame, wxID_ANY, wxDefaultPosition);
	m_pFrame->AddUIPane(m_pLayerPropertyPanel, wxAuiPaneInfo().
		Name(wxT("Pane_Layer")).Caption(wxT("Layer")).
		Left().Layer(0).Position(0).CloseButton(true).MaximizeButton(true).Float().Show(false));

	m_pSorceryPanel = new C_SorceryPanel(m_pFrame, wxID_ANY, wxDefaultPosition);
	m_pFrame->AddUIPane(m_pSorceryPanel, wxAuiPaneInfo().
		Name(wxT("Pane_Sorcery")).Caption(wxT("Sorcery")).
		Left().Layer(0).Position(0).CloseButton(true).MaximizeButton(true).Float().Show(false));

	m_pDynamicObjectPanel = new C_DynamicObjectPanel(m_pFrame, wxID_ANY, wxDefaultPosition);
	m_pFrame->AddUIPane(m_pDynamicObjectPanel, wxAuiPaneInfo().
		Name(wxT("Pane_PhysicalObject")).Caption(wxT("Physical Object Browser")).
		Left().Layer(0).Position(0).CloseButton(true).MaximizeButton(true).Float().Show(false));

	// Standard toolbar
	m_pStandardToolBar = ToolBars::CreateStandardToolBar(m_pFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER);

	S32 offset = 0;
	m_pFrame->AddUIPane(m_pStandardToolBar, wxAuiPaneInfo().
		Name(wxT("Pane_StandardToolBar")).Caption(wxT("Standard Tools")).Position(offset).
		ToolbarPane().Top().DockFixed(true).LeftDockable(true).RightDockable(true).PaneBorder(false).Gripper(false),
		E_UIPanelType_ToolBar);
	offset += m_pStandardToolBar->GetSize().x;

	m_pManipulatorToolBar = ToolBars::CreateManipulatorToolBar(m_pFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER);
	m_pFrame->AddUIPane(m_pManipulatorToolBar, wxAuiPaneInfo().
		Name(wxT("Pane_ManipulatorToolBar")).Caption(wxT("Manipulator Tools")).Position(offset).
		ToolbarPane().Top().DockFixed(true).LeftDockable(true).RightDockable(true).PaneBorder(false).Gripper(false),
		E_UIPanelType_ToolBar);
	offset += m_pManipulatorToolBar->GetSize().x;

	m_pSelectionToolPanel = new C_SelectionToolPanel(m_pFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	m_pFrame->AddUIPane(m_pSelectionToolPanel, wxAuiPaneInfo().
		Name(wxT("Pane_SelectionToolBar")).Caption(wxT("Selection Tools")).Position(offset).
		ToolbarPane().Top().DockFixed(true).LeftDockable(true).RightDockable(true).PaneBorder(false).Gripper(false),
		E_UIPanelType_ToolBar);
	offset += m_pSelectionToolPanel->GetSize().x;

	m_pSnapToolBar = ToolBars::CreateSnapToolBar(m_pFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER);
	m_pFrame->AddUIPane(m_pSnapToolBar, wxAuiPaneInfo().
		Name(wxT("Pane_SnapToolBar")).Caption(wxT("Snapping Tools")).Position(offset).
		ToolbarPane().Top().DockFixed(true).LeftDockable(true).RightDockable(true).PaneBorder(false).Gripper(false),
		E_UIPanelType_ToolBar);
	offset += m_pSnapToolBar->GetSize().x;

	m_pDisplayToolBar = ToolBars::CreateDisplayToolBar(m_pFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER);
	m_pFrame->AddUIPane(m_pDisplayToolBar, wxAuiPaneInfo().
		Name(wxT("Pane_DisplayToolBar")).Caption(wxT("Display Tools")).Position(offset).
		ToolbarPane().Top().DockFixed(true).LeftDockable(true).RightDockable(true).PaneBorder(false).Gripper(false),
		E_UIPanelType_ToolBar);
	offset += m_pDisplayToolBar->GetSize().x;

	m_pActorTypeToolPanel = new C_ActorToolPanel(m_pFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	m_pFrame->AddUIPane(m_pActorTypeToolPanel, wxAuiPaneInfo().
		Name(wxT("Pane_ActorToolBar")).Caption(wxT("Actor Type Tool")).Position(offset).
		ToolbarPane().Top().DockFixed(true).LeftDockable(true).RightDockable(true).PaneBorder(false).Gripper(false),
		E_UIPanelType_ToolBar);

	//Scene node property panel
	m_pSceneNodePropertyPanel = new C_SceneNodePropertyPanel(m_pFrame);
	m_pFrame->AddAttributePanel(m_pSceneNodePropertyPanel, "Current");

	//Camera property panel
	m_pCameraPropertyPanel = new C_CameraPropertyPanel(m_pFrame);
	m_pFrame->AddAttributePanel(m_pCameraPropertyPanel, "View");

	//Manipulator property panel
	m_pManipulatorPropertyPanel = new C_ManipulatorPropertyPanel(m_pFrame);
	C_RenderCore::Instance().GetManipulatorManager().AttachPropertyPanel(m_pManipulatorPropertyPanel);

	I_Manipulator *pHandler = 0;

	pHandler = new C_ActorCreateManipulator;
	C_RenderCore::Instance().GetManipulatorManager().AddManipulator("create", pHandler);
	m_pManipulatorPropertyPanel->Register<C_CreateManipulatorProperties>("create", (C_ActorCreateManipulator*)pHandler);

	pHandler = new C_PolyCreateManipulator;
	C_RenderCore::Instance().GetManipulatorManager().AddManipulator("createpoly", pHandler);

	pHandler = new C_SplineCreateManipulator;
	C_RenderCore::Instance().GetManipulatorManager().AddManipulator("createspline", pHandler);

	pHandler = new C_DropperSelectManipulator;
	C_RenderCore::Instance().GetManipulatorManager().AddManipulator("dropperselect", pHandler);

	pHandler = new C_SelectManipulator;
	C_RenderCore::Instance().GetManipulatorManager().AddManipulator("select", pHandler);
	m_pManipulatorPropertyPanel->Register<C_SelectManipulatorProperties>("select", (C_SelectManipulator*)pHandler);
	//m_pManipulatorToolBar->FindById(selectId)->SetClientData(pHandler);

	pHandler = new C_MoveManipulator;
	C_RenderCore::Instance().GetManipulatorManager().AddManipulator("move", pHandler);
	m_pManipulatorPropertyPanel->Register<C_MoveManipulatorProperties>("move", (C_MoveManipulator*)pHandler);

	pHandler = new C_RotateManipulator;
	C_RenderCore::Instance().GetManipulatorManager().AddManipulator("rotate", pHandler);
	m_pManipulatorPropertyPanel->Register<C_RotateManipulatorProperties>("rotate", (C_RotateManipulator*)pHandler);

	pHandler = new C_ScaleManipulator;
	C_RenderCore::Instance().GetManipulatorManager().AddManipulator("scale", pHandler);
	m_pManipulatorPropertyPanel->Register<C_ScaleManipulatorProperties>("scale", (C_ScaleManipulator*)pHandler);

	m_pFrame->AddAttributePanel(m_pManipulatorPropertyPanel, "Manipulator");
	EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);

	m_pShortcutHandler = new C_ShortcutHandler();

	m_pFrame->Show(true);

	wxGetApp().m_NetworkTimer.Start(30);
	return true;
}

bool C_App::UninitialiseUI()
{
	if (EditorLib::C_Core::Instance().GetConfig().appType != EditorLib::E_AppType_Interactive)
		return true;

	wxGetApp().m_NetworkTimer.Stop();

	LOGINFO(g_Log, "Destroying UI...");

	LOGINFO(g_Log, "Shutting down forms...");
	LOGINFO(g_Log, "	m_pXtDynamicForm...");
	if (m_pXtDynamicForm)		m_pXtDynamicForm->Shutdown();
	LOGINFO(g_Log, "	m_pLevelInfoPanel...");
	if (m_pLevelInfoPanel)		m_pLevelInfoPanel->Shutdown();
	LOGINFO(g_Log, "	m_pReferencePanel...");
	if (m_pReferencePanel)		m_pReferencePanel->Shutdown();
	LOGINFO(g_Log, "    m_pGroupEditPanel...");
	if (m_pGroupEditPanel)		m_pGroupEditPanel->Shutdown();
	LOGINFO(g_Log, "    m_pVerificationPanel...");
	if (m_pVerificationPanel)	m_pVerificationPanel->Shutdown();
	LOGINFO(g_Log, "	m_pSorceryHypergraphPanel...");
	if (m_pSorceryHypergraphPanel)	m_pSorceryHypergraphPanel->Shutdown();
	LOGINFO(g_Log, "	m_pHypergraphPanel...");
	if (m_pHypergraphPanel)		m_pHypergraphPanel->Shutdown();
	LOGINFO(g_Log, "	m_pAnimationPanel...");
	if (m_pAnimationPanel)		m_pAnimationPanel->Shutdown();
	LOGINFO(g_Log, "	m_pDopeSheetPanel...");
	if (m_pDopeSheetPanel)		m_pDopeSheetPanel->Shutdown();
	LOGINFO(g_Log, "	m_pLayerEditPanel...");
	if (m_pLayerEditPanel)	m_pLayerEditPanel->Shutdown();
	LOGINFO(g_Log, "	m_pLayerPropertyPanel...");
	if (m_pLayerPropertyPanel)	m_pLayerPropertyPanel->Shutdown();
	LOGINFO(g_Log, "	m_pSorceryPanel...");
	if (m_pSorceryPanel)		m_pSorceryPanel->Shutdown();
	LOGINFO(g_Log, "	m_pDynamicObjectPanel...");
	if (m_pDynamicObjectPanel)	m_pDynamicObjectPanel->Shutdown();

	LOGINFO(g_Log, "Shutting down camera controllers...");
	C_RenderCore::Instance().DestroyAllCameraControllers();

	LOGINFO(g_Log, "Removing panes...");
	if (m_pFrame)
	{
		m_pFrame->RemoveUIPane(m_pStandardToolBar);
		m_pFrame->RemoveUIPane(m_pActorTypeToolPanel);
		m_pFrame->RemoveUIPane(m_pSelectionToolPanel);
		m_pFrame->RemoveUIPane(m_pManipulatorToolBar);
		m_pFrame->RemoveUIPane(m_pXtDynamicForm);
		m_pFrame->RemoveUIPane(m_pReferencePanel);
		m_pFrame->RemoveUIPane(m_pGroupEditPanel);
		m_pFrame->RemoveUIPane(m_pVerificationPanel);
		m_pFrame->RemoveUIPane(m_pSorceryHypergraphPanel);
		m_pFrame->RemoveUIPane(m_pHypergraphPanel);
		m_pFrame->RemoveUIPane(m_pAnimationPanel);
		m_pFrame->RemoveUIPane(m_pDopeSheetPanel);
		m_pFrame->RemoveUIPane(m_pLayerEditPanel);
		m_pFrame->RemoveUIPane(m_pLayerPropertyPanel);
		m_pFrame->RemoveUIPane(m_pSorceryPanel);
		m_pFrame->RemoveUIPane(m_pDynamicObjectPanel);
		m_pFrame->RemoveUIPane(m_pLevelInfoPanel);
		m_pFrame->RemoveUIPane(m_pScriptLogPanel);
	}
	SAFE_DELETE_PTR(m_pShortcutHandler);

	Tasks::UninstallWndProcHook();

	LOGINFO(g_Log, "UI has shut down.");
	return true;
}

const EditorConfig& C_App::GetConfig() const
{
	return m_EditorConfig;
}

void C_App::OnTimer(wxTimerEvent& e)
{
	if (e.GetId() == m_NetworkTimer.GetId())
	{
		EditorLib::C_Core::Instance().GetNetworkManager().Update();
	}
	else if (e.GetId() == m_DirtyTimer.GetId())
	{
		OnDirty();
	}
	else if (e.GetId() == m_AutosaveTimer.GetId())
		Tasks::ExportUnsavedChanges();
}

void C_App::OnWindowCreate(wxWindowCreateEvent& e)
{
	wxWindow *pWindow = e.GetWindow();
	wxFrame *pFrame = GetFrame();
}


C_Frame* C_App::GetFrame()
{
	return m_pFrame;
}

C_SceneNodePropertyPanel* C_App::GetSceneNodePropertyPanel()
{
	return m_pSceneNodePropertyPanel;
}

C_CameraPropertyPanel* C_App::GetCameraPropertyPanel()
{
	return m_pCameraPropertyPanel;
}

C_ManipulatorPropertyPanel* C_App::GetManipulatorPropertyPanel()
{
	return m_pManipulatorPropertyPanel;
}

C_ShortcutHandler* C_App::GetShortcutHandler()
{
	return m_pShortcutHandler;
}

void C_App::SetAutosaveInterval(U32 interval)
{
	m_AutosaveTimer.Start(interval*1000*60);	//min->ms
}

void C_App::OnDirty()
{
	if (m_flags.postLevelLoadPending  &&
		Tasks::IsLevelLoaded())
	{
		m_flags.postLevelLoadPending = false;

		C_RenderCore::Instance().RefreshAllViewports();
		m_flags.isDirty = false;
	}
	else if (m_flags.isDirty)
	{
		C_RenderCore::Instance().RefreshAllViewports();
		for (S32 i = 0; i < m_views.Size(); i++)
			m_views[i]->Refresh();
		m_flags.isDirty = false;
	}

	if (m_pFrame)
		m_pFrame->GetStatusBar()->Refresh();
}

////// SLOTS //////

void C_App::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelEvent.connect(this, &Editor::C_App::C_Slots::OnLevelEventSlot);
	s.levelNameChanged.connect(this, &Editor::C_App::C_Slots::OnLevelNameChangedSlot);
	s.levelOpening.connect(this, &Editor::C_App::C_Slots::OnLevelOpeningSlot);
	s.levelClosing.connect(this, &Editor::C_App::C_Slots::OnLevelClosingSlot);
	s.levelOpened.connect(this, &Editor::C_App::C_Slots::OnLevelOpenedSlot);
	s.levelClosed.connect(this, &Editor::C_App::C_Slots::OnLevelClosedSlot);

	s.coreDirtyViews.connect(this, &Editor::C_App::C_Slots::OnCoreDirtyViewsSlot);

	s.quit.connect(this, &Editor::C_App::C_Slots::OnQuitSlot);
}

void C_App::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.dgNodeCreated.disconnect(this);

	s.levelEvent.disconnect(this);
	s.levelNameChanged.disconnect(this);
	s.levelOpening.disconnect(this);
	s.levelClosing.disconnect(this);
	s.levelOpened.disconnect(this);
	s.levelClosed.disconnect(this);

	s.coreDirtyViews.disconnect(this);

	s.quit.disconnect(this);
}

void C_App::C_Slots::OnLevelNameChangedSlot(const EditorLib::I_Level& level)
{
	if (&level == EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel())
		wxGetApp().UpdateTitleBar(&level);
}

void C_App::C_Slots::OnLevelEventSlot(const EditorLib::I_Level* level, EditorLib::E_LevelEvent e)
{
	if (!level || level->GetType() != EditorLib::E_LevelType_XtLevel)
		return;

	if (e == EditorLib::E_LevelEvent_ActiveLevelChanging && level)
		UserData::SaveUserData(((EditorLib::C_XtLevel*)level)->GetXtPackId());
	else if (e == EditorLib::E_LevelEvent_ActiveLevelChanged && level)
		UserData::LoadUserData(((EditorLib::C_XtLevel*)level)->GetXtPackId());
}

void C_App::C_Slots::OnLevelOpeningSlot(const EditorLib::I_Level& level)
{
	wxGetApp().m_DirtyTimer.Stop();
}

void C_App::C_Slots::OnLevelClosingSlot(const EditorLib::I_Level& level)
{
	wxGetApp().m_DirtyTimer.Stop();
}

void C_App::C_Slots::OnLevelOpenedSlot(const EditorLib::I_Level& level)
{
	//Handle opening of main level.
	if (EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel() == &level)
	{
		BIZ_ASSERT(level.GetType() == EditorLib::E_LevelType_XtLevel);

		wxGetApp().m_mainLevelId = ((EditorLib::C_XtLevel&)level).GetXtRecordId();
		wxGetApp().UpdateTitleBar(&level);
		wxGetApp().m_flags.postLevelLoadPending = true;
	}

	LOGINFO(EditorLib::C_Core::Instance().GetLogger(), "Level (%s) opened", level.GetName().Chars());
	wxSafeYield(); // HACK!!

	wxGetApp().m_flags.isDirty = true;
	wxGetApp().m_DirtyTimer.Start(250);
}

void C_App::C_Slots::OnLevelClosedSlot(const EditorLib::I_Level& level)
{
	wxGetApp().m_DirtyTimer.Start(250);
	wxGetApp().UpdateTitleBar(0);
	wxGetApp().m_mainLevelId = EditorLib::kXtInvalidId;
	wxGetApp().m_flags.isDirty = true;
	wxGetApp().OnDirty();
	wxGetApp().m_flags.postLevelLoadPending = true;
}

void C_App::C_Slots::OnCoreDirtyViewsSlot(EditorLib::E_DirtyEventType e)
{
	wxGetApp().m_flags.isDirty = true;
}

void C_App::C_Slots::OnQuitSlot()
{
	//Quick exit path.
	if (!IsDebuggerPresent())
	{
		//FastQuit - Save changes before uninitialising UI just in case of crashes during shutdown.
		if (Tasks::SaveChanges() == wxID_CANCEL)
			return;

		//Save UI state.
		wxGetApp().UninitialiseUI();
		//Terminate.
		Tasks::FastQuit();
	}
	//Slow exit path.
	else
	{
		//Close level.
		if (!Tasks::CloseAllPacks())
			return;

		//Save UI State and uninitialise to speed shutdown.
		wxGetApp().UninitialiseUI();
	}

	//Exit application at first opportunity.
	wxGetApp().ExitMainLoop();
}

////// END SLOTS //////

void XtLibrary_Application() {}

void XtGenerateApplicationSchema() {}
