#include "stdafx.h"
#include <minmax.h>

#include <gdiplus.h>

#include <wx/dialog.h>
#include <wx/cmdline.h>
#include <wx/stdpaths.h>
#include <wx/msw/registry.h>

#include <wx_shared/Log.h>
#include <wx_shared/EventProxy.h>

#include <sigslot/sigslot.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>
#include <Tools_Shared/MiniDumper.h>
#include <Tools_Shared/ToolsAssert.h>
#include <Tools_Shared/Utility/MacroExpander.h>
#include <Tools_Shared/Memorydbg.h>

#include <xtwin/ASyncOpWin.h>
#include <xtwin/conflictwin.h>
#include <xtwin/SendEMail.h>
#include <xtwin/svnxt.h>
#include <xtwin/commitwin.h>

#include <system/misc/StringUtil.h>
#include <system/misc/BtlStringUtil.h>

#include <DialogueMan/Input_Handlers/ShortcutHandler.h>
#include <DialogueMan/Diff/Diff.h>
#include <DialogueMan/Diff/DiffViewer.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/MutantValue.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <DialogueMan/Xt/XtTypeDatabase.h>
#include <DialogueMan/Xt/XtTypeHelpers.h>
#include <DialogueMan/GlobalSignals.h>
#include <DialogueMan/Frame/Frame.h>
#include <DialogueMan/App.h>

namespace DMan
{

IMPLEMENT_APP(C_App)
 
bool C_App::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	SetVendorName(wxString::FromUTF8(kVendorName));
	m_pLogTextSubscriber = 0;
	m_pShortcutHandler = 0;

	Tools::InitialiseMemoryDbg();

	m_pDummyToolsApp = new C_App::C_DummyToolsApp("");
	m_pDummyToolsApp->Run(0, NULL, false);

	memset(&m_flags, 0, sizeof(m_flags));

	m_pGlobalSignals = new C_GlobalSignals();

	m_slots.Connect();

	m_pFrame = new C_Frame(
		NULL, 
		wxID_ANY, 
		wxT("Dialogue Manager"), 
		wxDefaultPosition, 
		wxSize(800, 600));
 
	if (!m_pFrame) 
		return false;

	if (!LoadConfig(
		m_DialogueManConfig.configPath + "/dialogueman_config.xml", 
		m_DialogueManConfig.macroFilePath,
		m_DialogueManConfig))
	{
		LOGERROR(g_Log, "Unable to open dialogueman_config.xml");
		return false;
	}

	if(!OpenDb())
		return false;

	if (!InitialiseUI())
		return false;
	
	//Connect timers
	Connect(m_dirtyTimer.GetId(), wxEVT_TIMER, wxTimerEventHandler(C_App::OnTimer));
	m_dirtyTimer.Start(500);

	m_viewsEnabled = 0;

	return true;
}

int C_App::OnExit()
{	
	wxLog::SetActiveTarget(0);
	Debug::GetRootLogger().RemoveSubscriber(m_pLogTextSubscriber);
	SAFE_DELETE_PTR(m_pLogTextSubscriber);

	//Prompt to store unsaved changes if db dirty.
	if (GetDb()->dirty)
	{
		wxMessageDialog dlg(
			0,
			"Save changes?",
			"Warning: Data Not Saved",
			wxYES_NO | wxICON_QUESTION);
		int choice = dlg.ShowModal();
		if (choice == wxID_YES)
		{
			SaveDb();
		}
	}
	
	CloseDb();

	if(m_inMemorySavingMode)
		TempToSorcery();

	SAFE_DELETE_PTR(m_pDummyToolsApp);

	m_pFrame = 0;

	assert(!m_pXtTypeDatabase);

	m_slots.Disconnect();

	SAFE_DELETE_PTR(m_pGlobalSignals);

	FreeBitmapCache();
	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	return 0;
}

static const wxCmdLineEntryDesc g_cmdLineDesc[] =
{    
    { wxCMD_LINE_PARAM, NULL, NULL, wxT("config file path"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY },
	{ wxCMD_LINE_PARAM,	NULL, NULL,	wxT("macro file path"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), wxT("displays help on the command line parameters"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_NONE }
};

void C_App::OnInitCmdLine(wxCmdLineParser& parser)
{
	parser.SetDesc(g_cmdLineDesc);
	parser.SetSwitchChars (wxT("-"));
}

bool C_App::OnCmdLineParsed(wxCmdLineParser& parser)
{
	wxString param0 = parser.GetParam(0);
	const wxCharBuffer asciiStr0 = ToAscii(param0);
	
	m_DialogueManConfig.configPath = Util::RemoveTrailingSlash((const char*)asciiStr0);
	Tools::C_MacroExpander::Instance().Expand(m_DialogueManConfig.configPath);

	if (parser.GetParamCount() > 1)
	{
		wxString param1 = parser.GetParam(1);
		m_DialogueManConfig.macroFilePath = Util::RemoveTrailingSlash(param1.c_str());
		Tools::C_MacroExpander::Instance().Expand(m_DialogueManConfig.macroFilePath);
	}

	return true;
}

void C_App::OnTimer(wxTimerEvent& e)
{
	if (e.GetId() == m_dirtyTimer.GetId())
	{
		GetGlobalSignals().appRefreshDirty();
	}
}

C_Frame* C_App::GetFrame()
{
	return m_pFrame;
}

Xt::DomDb* C_App::GetDb()
{
	return m_pDb;
}

Xt::DomPack* C_App::GetCurrentPack()
{
	return m_pCurrentPack;
}

C_XtTypeDatabase& C_App::GetXtTypeDatabase()
{
	return *m_pXtTypeDatabase;
}

C_DbManager&	C_App::GetDbManager()
{
	return *m_pDbManager;
}

DialogueManConfig& C_App::GetConfig()
{
	return m_DialogueManConfig;
}

void C_App::SetSelection(const C_Selection& selection)
{
	*m_nonXtState.pSelection = selection;

	if (selection.GetMode() != C_Selection::E_Mode_Row)
	{
		const C_Selection::ItemArray& arr = selection.GetItems();
		if (arr.Size() > 0)
		{
			const C_Selection::C_Item& item = arr.Back();
			XtId packId = item.id.GetPackId();
			Xt::DomPack* pPack = Xt::FindPackById(GetDb(), packId);
			if(pPack)
			{
				m_pCurrentPack = pPack;	
			}
		}
	}
	GetGlobalSignals().appSelectionChanged();
}


void C_App::GetSelection(C_Selection& selection)
{
	selection = *m_nonXtState.pSelection;
}

void C_App::SetRecordsInCurrentFolder(const Btl::Vector<Xt::DomRecord*>& records)
{
	m_recordsInCurrentFolder.Clear();

	for(int i = 0; i < records.Size(); i++)
	{
		m_recordsInCurrentFolder.PushBack(records[i]);
	}
}

Btl::Vector<Xt::DomRecord*>& C_App::GetRecordsInCurrentFolder()
{
	return m_recordsInCurrentFolder;
}

bool C_App::OpenDb()
{
	Btl::String formattedPath = Misc::AddTrailingSlash(m_DialogueManConfig.XtDbPath.Chars(), '/');

	Util::C_String loadedProject = "%BIZARRE_PROJECT%";
	Tools::C_MacroExpander::Instance().Expand(loadedProject);
	if(loadedProject == "Amax")
	{
		wxRegKey key("HKEY_LOCAL_MACHINE\\SOFTWARE\\Bizarre Creations\\DialogueManager\\MemorySavingMode");
		if(!key.Exists())
		{
			wxMessageDialog openMode(m_pFrame, "Open in Memory Saving mode?", "Dialogue Manager", wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
			if(openMode.ShowModal() == wxID_YES)
			{
				m_inMemorySavingMode = true;
				key.Create();
				SorceryToTemp();
				
				//launch another process and quit this one
				wxExecute(wxString::Format("%s %s %s", argv[0], argv[1], argv[2]) , wxEXEC_ASYNC);
				return false;
			}
			else
			{
				m_inMemorySavingMode = false;
			}
		}
		else
		{
			m_inMemorySavingMode = true;
			formattedPath = (wxStandardPaths().GetUserConfigDir() + "/DialogueMan/sorcery3/main").c_str();
		}
	}
	else
	{
		m_inMemorySavingMode = false;
	}

	m_pCurrentPack = 0;
	m_pDb = new Xt::DomDb;
	
	Xt::Monitor(&m_dbMonitor, m_pDb);
	m_dbMonitor.SetCallback(&StaticOnXtDomMonitorEvent, this);

	SvnXtParams svnParams(m_pDb, formattedPath.Chars());
	if (!SvnXtOpen((HWND)wxGetApp().GetFrame()->GetHandle(), &svnParams))
		return false;
	
	return true;
}

void C_App::CloseDb()
{
	if (m_pDb->open) 
	{
		delete m_pDb;
		m_pDb = NULL;
		m_pCurrentPack = 0;
	}
}

void C_App::SorceryToTemp()
{
	//Open Temp DB
	Xt::DomDb* pTempDb = new Xt::DomDb;
	
	Btl::String sorceryPath = Misc::AddTrailingSlash(m_DialogueManConfig.XtDbPath.Chars(), '/');
 	SvnXtParams svnParams(pTempDb, sorceryPath.Chars());
	if (!SvnXtOpen((HWND)wxGetApp().GetFrame()->GetHandle(), &svnParams))
		return;

	//Check if there is any Dialogue or XtLocTable record in this pack
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(pTempDb->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		for(Xt::DomPack::Iterator it(pPack); it; ++it)
		{
			Xt::DomRecord* pRec = it;
			
			if(pRec->root.type == "Dialogue" || pRec->root.type == "XtLocTable")
			{
				m_packsToKeep.Add( Misc::ExtractFilename(pPack->filename.Text()).Chars() );
				break;
			}
		}
	}

	//Close temp db
	delete pTempDb;
	pTempDb = NULL;

	//Compose command line call
	wxString cmd = Misc::ExtractDirectory(wxString(argv[0]).c_str()).Chars();
	cmd += "sorcery_to_temp.bat ";
	cmd += sorceryPath.Chars();

	for(size_t i = 0; i < m_packsToKeep.size(); i++)
	{
		cmd += " ";
		cmd += m_packsToKeep[i];
	}
		
	wxExecute(cmd, wxEXEC_SYNC);
}

void C_App::TempToSorcery()
{	
	//Compose command line call
	Btl::String sorceryPath = Misc::AddTrailingSlash(m_DialogueManConfig.XtDbPath.Chars(), '/');

	wxString cmd = Misc::ExtractDirectory(wxString(argv[0]).c_str()).Chars();
	cmd += "temp_to_sorcery.bat ";
	cmd += sorceryPath.Chars();
		
	wxExecute(cmd, wxEXEC_SYNC);

	wxRegKey key("HKEY_LOCAL_MACHINE\\SOFTWARE\\Bizarre Creations\\DialogueManager\\MemorySavingMode");
	if(key.Exists())
	{
		key.DeleteSelf();
	}
}

bool C_App::InMemorySavingMode()
{
	return m_inMemorySavingMode;
}

void C_App::StaticOnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent, 
	C_App* pThis)
{
	
	if (pEvent->code == Xt::kDomPackChanged)
	{
		Xt::DomPack* pPack = (Xt::DomPack*)pEvent->from;
	}

	if (pEvent->code == Xt::kDomDbClosing)
	{
		pThis->OnCloseDb();
	}
	else if (pEvent->code == Xt::kDomDbOpened)
	{
		pThis->OnOpenDb();
	}
}

void C_App::OnOpenDb()
{
	m_pXtTypeDatabase = new C_XtTypeDatabase;
	m_pXtTypeDatabase->RegisterTypes(m_pDb);

	m_pDbManager = new C_DbManager;
	m_pDbManager->OnDbOpened();

	GetGlobalSignals().appDbOpened();
}

void C_App::OnCloseDb()
{
	m_deferredUpdates.Clear();
	GetGlobalSignals().appDbClose();

	m_pDbManager->OnDbClose();
	SAFE_DELETE_PTR(m_pDbManager);

	SAFE_DELETE_PTR(m_pXtTypeDatabase);
}

bool C_App::SaveDb()
{
	Btl::String formattedPath = Misc::AddTrailingSlash(m_DialogueManConfig.XtDbPath.Chars(), '/');

	SvnXtParams svnParams(m_pDb, formattedPath.Chars());
	bool result = SvnXtSave((HWND)wxGetApp().GetFrame()->GetHandle(), &svnParams);
	return result;
}

void C_App::AddForDeferredUpdate(XtShared::C_XtObjectImpl* pImpl)
{
	m_deferredUpdates.Insert(pImpl);
}

void C_App::RemoveForDeferredUpdate(XtShared::C_XtObjectImpl* pImpl)
{
	XtObjectImplSet::Iterator it = m_deferredUpdates.Find(pImpl);
	if (it == m_deferredUpdates.End())
	{
		assert(!"RemoveForDeferredUpdate failed!");
		return;
	}

	m_deferredUpdates.Erase(it);
}

bool C_App::IsDeferredUpdatePending()
{
	return m_deferredUpdates.Size() != 0;
}

Debug::Logger& C_App::GetLogger() const
{
	 return m_pDummyToolsApp->GetLogger();
}

C_GlobalSignals& C_App::GetGlobalSignals()
{
	return *m_pGlobalSignals;
}

bool C_App::InitialiseUI()
{
	wxInitAllImageHandlers();

	//wire up the window close event
	Ut::Delegate<void()> exitCallback(&m_slots, &C_App::C_Slots::OnQuitSlot);
	ConnectEvent0<wxCloseEvent, Ut::Delegate<void()>, SkipEvent>(m_pFrame, wxEVT_CLOSE_WINDOW, exitCallback, m_pFrame->GetId());

	m_pLogText = new wxTextCtrl(m_pFrame, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(500, 100), wxTE_RICH2 | wxNO_BORDER | wxTE_MULTILINE);
	m_pLogTextSubscriber = new Debug::TextSubscriber(m_pLogText);
	Debug::GetRootLogger().AddSubscriber(m_pLogTextSubscriber);
	wxLog::SetActiveTarget(m_pLogTextSubscriber);

	if (!m_pFrame->Initialise())
		return false;

	//add a log window
	m_pFrame->AddUIPane(m_pLogText,
		wxAuiPaneInfo().Name(_T("Pane_Log")).Caption(_T("Log Window")).
		Bottom().Layer(0).Position(0).CloseButton(false).MaximizeButton(true).PinButton(false).Show(true));

	m_pShortcutHandler = new C_ShortcutHandler();

	SetTopWindow(m_pFrame);
	m_pFrame->Show(true);
	return true;
}

void C_App::UninitialiseUI()
{
	m_pFrame->Uninitialise();

	delete m_pShortcutHandler;
	m_pShortcutHandler = 0;
}

void C_App::IdleTick()
{
	for (XtObjectImplSet::Iterator it = m_deferredUpdates.Begin(); it != m_deferredUpdates.End(); ++it)
		it->GetKey()->XtDeserialiseDeferred();
	m_deferredUpdates.Clear();
}

void C_App::EnableViews(bool enable)
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	if (enable)
	{
		if (m_viewsEnabled > 0)
			m_viewsEnabled--;

		if (m_viewsEnabled == 0)
			s.viewsEnabled(true);
	}
	else
	{
		if (m_viewsEnabled == 0)
			s.viewsEnabled(false);

		m_viewsEnabled++;		
	}
}

int C_App::OnRun()
{
	try
	{
		return wxApp::OnRun();
	}
	catch(std::bad_alloc& e)
	{
		wxMessageBox(
			wxString::Format("DialogueManager needs to close because there is not enough memory to complete this task. Debug: '%s'", e.what()), 
			"Fatal Error", wxICON_ERROR | wxOK);
		return 1;
	}
	catch(std::exception& e)
	{
		wxMessageBox(
			wxString::Format("DialogueManager needs to close due to the following error: %s", e.what()), 
			"Fatal Error", wxICON_ERROR | wxOK);
		return 1;
	}
	catch(...)
	{
		wxMessageBox(
			wxString::Format("DialogueManager needs to close due to an unspecified error"), 
			"Fatal Error", wxICON_ERROR | wxOK);
		return 1;
	}
}

void C_App::C_Slots::Connect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.quit.connect(this, &C_App::C_Slots::OnQuitSlot);
}

void C_App::C_Slots::Disconnect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.quit.disconnect(this);
}

void C_App::C_Slots::OnQuitSlot()
{
	wxGetApp().UninitialiseUI();

	//Exit application at first opportunity.
	wxGetApp().ExitMainLoop();
}

} //namespace DMan

namespace XtShared
{
	void OnChangeXtObjectType(C_XtObjectImpl* pImpl)
	{
	}
	
	void AddXtObjectForDeferredUpdate(C_XtObjectImpl* pImpl)
	{
		DMan::wxGetApp().AddForDeferredUpdate(pImpl);
	}

	void RemoveXtObjectFromDeferredUpdate(C_XtObjectImpl* pImpl)
	{
		DMan::wxGetApp().RemoveForDeferredUpdate(pImpl);
	}
}

void XtLibrary_Application() {}

void XtGenerateApplicationSchema() {}