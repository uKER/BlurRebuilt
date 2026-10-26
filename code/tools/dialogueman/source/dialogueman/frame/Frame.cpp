#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <wx/artprov.h>

#include <system/debug/buildstamp.h>

#include <xtwin_wx/Explorer/Explorer.h>

#include <wx_shared/EventProxy.h>
#include <wx_shared/MenuBuilder.h>
#include <wx_shared/Log.h>
#include <wx_shared/Registry.h>

#include <DialogueMan/Components/DataTable/DataGrid.h>
#include <DialogueMan/Components/DataTable/DataGridPanel.h>
#include <DialogueMan/Components/Scarab_Form/WxScarabForm.h>
#include <DialogueMan/Components/Explorer/Explorer.h>
#include <DialogueMan/Components/LocForm/LocForm.h>

#include <DialogueMan/Xt/XtTypeHelpers.h>
#include <DialogueMan/Xt/XtTypeDatabase.h>

#include <DialogueMan/Misc/StringUtil.h>

#include <DialogueMan/Frame/Frame.h>
#include <DialogueMan/Frame/Menus/FrameMenuBar.h>
#include <DialogueMan/Frame/Menus/FrameToolsMenu.h>
#include <DialogueMan/Frame/Menus/FrameFileMenu.h>
#include <DialogueMan/Frame/Menus/FrameEditMenu.h>

#include <DialogueMan/App.h>

namespace DMan
{

BEGIN_EVENT_TABLE(C_Frame, wxFrame)
	EVT_IDLE(C_Frame::OnIdle)
END_EVENT_TABLE()

C_Frame::C_Frame(wxWindow* parent,
	 wxWindowID id,
	 const wxString& title, 
	 const wxPoint& pos, 
	 const wxSize& size,
	 long style)
	: wxFrame(parent, id, title, pos, size, style), 
	  m_pAuiManager(0),
	  m_pScarabForm(0),
	  m_pLocForm(0),
	  m_pMenuBar(0)
{
}

C_Frame::~C_Frame()
{
	if (m_pAuiManager)
	{
		m_pAuiManager->UnInit();
		SAFE_DELETE_PTR(m_pAuiManager)
	}

	SAFE_DELETE_PTR(m_pMenuBar);
}

bool C_Frame::Initialise()
{
	//AUI Manager 
	m_pAuiManager = new wxAuiManager();
	m_pAuiManager->SetManagedWindow(this);
	m_pAuiManager->SetFlags(wxAUI_MGR_ALLOW_FLOATING |  
							wxAUI_MGR_TRANSPARENT_DRAG | 
							wxAUI_MGR_RECTANGLE_HINT | 
							wxAUI_MGR_NO_VENETIAN_BLINDS_FADE);

	m_pAuiManager->GetArtProvider()->SetMetric(wxAUI_DOCKART_SASH_SIZE, 2);
	m_pAuiManager->GetArtProvider()->SetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE, 1);
	m_pAuiManager->GetArtProvider()->SetMetric(wxAUI_DOCKART_CAPTION_SIZE, 17);
	m_pAuiManager->GetArtProvider()->SetMetric(wxAUI_DOCKART_GRADIENT_TYPE, wxAUI_GRADIENT_HORIZONTAL);

	m_pMenuBar = new C_FrameMenuBar(this);

	if (!m_pMenuBar->InitialiseUI("dialogueman"))
		return false;

	//Toolbar
	wxImage toolIcon;
	wxToolBarToolBase* pTool = NULL;

	wxToolBar* tb1 = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                   wxTB_FLAT | wxTB_NODIVIDER | wxTB_TEXT);
    tb1->SetToolBitmapSize(wxSize(16,16));
    
	toolIcon.LoadFile(wxT("./icons/table_save.png"));
    pTool = tb1->AddTool(wxID_ANY, wxT("Save"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Save"), wxT("Save changes to the database"));
	ConnectEvent1<wxCommandEvent, Ut::Delegate<void(wxCommandEvent&)> >(tb1, wxEVT_COMMAND_TOOL_CLICKED, Ut::Delegate<void(wxCommandEvent&)>(&C_FrameFileMenu::OnSave), pTool->GetId());
	toolIcon.LoadFile(wxT("./icons/cog.png"));
    pTool = tb1->AddTool(wxID_ANY, wxT("Import"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Import"), wxT("Import Dialogue Lines from an Excel file"));
	ConnectEvent1<wxCommandEvent, Ut::Delegate<void(wxCommandEvent&)> >(tb1, wxEVT_COMMAND_TOOL_CLICKED, Ut::Delegate<void(wxCommandEvent&)>(&C_FrameFileMenu::OnImport), pTool->GetId());
	toolIcon.LoadFile(wxT("./icons/cog.png"));
    pTool = tb1->AddTool(wxID_ANY, wxT("Export"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Export"), wxT("Export Dialogue Lines to an Excel file"));
	ConnectEvent1<wxCommandEvent, Ut::Delegate<void(wxCommandEvent&)> >(tb1, wxEVT_COMMAND_TOOL_CLICKED, Ut::Delegate<void(wxCommandEvent&)>(&C_FrameFileMenu::OnExport), pTool->GetId());
    tb1->Realize();
	
	wxToolBar* tb2 = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                   wxTB_FLAT | wxTB_NODIVIDER | wxTB_TEXT);
    tb2->SetToolBitmapSize(wxSize(16,16));

	toolIcon.LoadFile(wxT("./icons/table_row_insert.png"));
    pTool = tb2->AddTool(wxID_ANY, wxT("Add"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Add Row"), wxT("Add new row into the table"));
	ConnectEvent1<wxCommandEvent, Ut::Delegate<void(wxCommandEvent&)> >(tb2, wxEVT_COMMAND_TOOL_CLICKED, Ut::Delegate<void(wxCommandEvent&)>(&C_FrameEditMenu::OnAdd), pTool->GetId());
	toolIcon.LoadFile(wxT("./icons/table_row_delete.png"));
    pTool = tb2->AddTool(wxID_ANY, wxT("Delete"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Delete Row"), wxT("Delete row from the table"));
	ConnectEvent1<wxCommandEvent, Ut::Delegate<void(wxCommandEvent&)> >(tb2, wxEVT_COMMAND_TOOL_CLICKED, Ut::Delegate<void(wxCommandEvent&)>(&C_FrameEditMenu::OnDelete), pTool->GetId());
	tb2->Realize();	
	/*
  
	toolIcon.LoadFile(wxT("./icons/arrow_undo.png"));
    tb2->AddTool(wxID_ANY, wxT("Undo"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Undo"), wxT("Undo last change"));
	toolIcon.LoadFile(wxT("./icons/arrow_redo.png"));
    tb2->AddTool(wxID_ANY, wxT("Redo"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Redo"), wxT("Redo last change"));
	tb2->AddSeparator();
    toolIcon.LoadFile(wxT("./icons/cut.png"));
    tb2->AddTool(wxID_ANY, wxT("Cut"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Cut"), wxT("Cut selection to clipboad"));
	toolIcon.LoadFile(wxT("./icons/page_copy.png"));
    tb2->AddTool(wxID_ANY, wxT("Copy"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Copy"), wxT("Copy selection to clipboad"));
	toolIcon.LoadFile(wxT("./icons/page_paste.png"));
    tb2->AddTool(wxID_ANY, wxT("Paste"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Paste"), wxT("Paste from clipboad"));
	tb2->AddSeparator();
	
    toolIcon.LoadFile(wxT("./icons/find.png"));
    pTool = tb2->AddTool(wxID_ANY, wxT("Find"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Find"), wxT("Find in table"));
    tb2->Realize();*/

	
	wxToolBar* tb3 = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                   wxTB_FLAT | wxTB_NODIVIDER | wxTB_TEXT);
    tb3->SetToolBitmapSize(wxSize(16,16));

	toolIcon.LoadFile(wxT("./icons/arrow_refresh.png"));
	pTool = tb3->AddTool(wxID_ANY, wxT("Generate Names"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Generate Names"), wxT("Generate Names"));
	ConnectEvent1<wxCommandEvent, Ut::Delegate<void(wxCommandEvent&)> >(tb3, wxEVT_COMMAND_TOOL_CLICKED, Ut::Delegate<void(wxCommandEvent&)>(&C_FrameToolsMenu::OnGenerateNames), pTool->GetId());
	
	toolIcon.LoadFile(wxT("./icons/music.png"));
	pTool = tb3->AddTool(wxID_ANY, wxT("Generate TTS"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Generate TTS"), wxT("Generate Placeholder TTS"));
	ConnectEvent1<wxCommandEvent, Ut::Delegate<void(wxCommandEvent&)> >(tb3, wxEVT_COMMAND_TOOL_CLICKED, Ut::Delegate<void(wxCommandEvent&)>(&C_FrameToolsMenu::OnGenerateTTS), pTool->GetId());
	
	toolIcon.LoadFile(wxT("./icons/page_white_code.png"));
	pTool = tb3->AddTool(wxID_ANY, wxT("Audio XML"), toolIcon, wxNullBitmap, wxITEM_NORMAL, wxT("Audio XML"), wxT("Audio XML"));
	ConnectEvent1<wxCommandEvent, Ut::Delegate<void(wxCommandEvent&)> >(tb3, wxEVT_COMMAND_TOOL_CLICKED, Ut::Delegate<void(wxCommandEvent&)>(&C_FrameToolsMenu::OnAudioXML), pTool->GetId());
	tb3->Realize();
 
	m_pAuiManager->AddPane(tb1, wxAuiPaneInfo().
		Name(wxT("tb1")).Caption(wxT("File Toolbar")).
		ToolbarPane().Top().
		LeftDockable(false).RightDockable(false));

    m_pAuiManager->AddPane(tb2, wxAuiPaneInfo().
		Name(wxT("tb2")).Caption(wxT("Edit Toolbar")).
		ToolbarPane().Top().Row(0).Position(1).
		LeftDockable(false).RightDockable(false));
	
	m_pAuiManager->AddPane(tb3, wxAuiPaneInfo().
		Name(wxT("tb3")).Caption(wxT("Tools Toolbar")).
		ToolbarPane().Top().Row(0).Position(2).
		LeftDockable(false).RightDockable(false));

	//Panes
	m_pExplorer = CreateExplorer();
	AddUIPane(m_pExplorer, wxAuiPaneInfo().Name(wxT("Pane_Database")).
		Caption(wxT("Database")).
		Left().Layer(1).Position(1).
		CloseButton(false).MaximizeButton(true));

	m_pGridPanel = CreateGridPanel();
	AddUIPane(m_pGridPanel, wxAuiPaneInfo().Name(wxT("Pane_Grid")).
		Caption(wxT("Data Grid")).
		Top().
		CloseButton(false).
		PinButton(false).
		CaptionVisible(true).
		MaximizeButton(true).
		Show(true).
		MinSize(300, 300));

	m_pScarabForm = CreateForm();
	AddUIPane(m_pScarabForm, wxAuiPaneInfo().Name(wxT("Pane_Details")).
		Caption(wxT("Details View")).
		Centre().
		Layer(0).
		Position(0).
		CloseButton(false).
		PinButton(false).
		CaptionVisible(true).
		MaximizeButton(true).
		Resizable(true).
		Show(true));

	m_pLocForm = CreateLocForm();
	AddUIPane(m_pLocForm, wxAuiPaneInfo().Name(wxT("Pane_Loc")).
		Caption(wxT("Loc View")).
		Centre().
		Layer(1).
		Position(1).
		CloseButton(false).
		PinButton(false).
		CaptionVisible(true).
		MaximizeButton(true).
		Resizable(true).
		Show(true));

	m_pAuiManager->Update();

	toolIcon.LoadFile(wxT("./icons/script.png"));
	wxIcon frameIcon;
	wxBitmap frameIconBmp(toolIcon);
    frameIcon.CopyFromBitmap(frameIconBmp);
	SetIcon(frameIcon);

	return true;
}

bool C_Frame::Uninitialise()
{
	m_pExplorer->Shutdown();
	m_pGridPanel->Shutdown();
	m_pScarabForm->Shutdown();
	m_pLocForm->Shutdown();

	RemoveUIPane(m_pExplorer);
	RemoveUIPane(m_pGridPanel);
	RemoveUIPane(m_pScarabForm);
	RemoveUIPane(m_pLocForm);

	return true;
}

C_Explorer* C_Frame::CreateExplorer()
{
	m_pExplorer = new C_Explorer(
		this, 
		ID_EXPLORER, 
		wxDefaultPosition, 
		wxSize(300,250),
		wxTR_EDIT_LABELS|
		wxTR_HAS_BUTTONS|
		wxTR_SINGLE|
		wxTR_EXTENDED|
		wxTR_FULL_ROW_HIGHLIGHT|
		wxTAB_TRAVERSAL|
		wxHSCROLL|
		wxVSCROLL|
		wxSTATIC_BORDER);

    return m_pExplorer;
}

C_DataGridPanel* C_Frame::CreateGridPanel()
{
	C_DataGridPanel* pGridPanel = new C_DataGridPanel(
										this,
										wxID_ANY,
										wxDefaultPosition,
										wxDefaultSize);

	return pGridPanel;
}

C_WxScarabForm* C_Frame::CreateForm()
{
	C_WxScarabForm* pForm = new C_WxScarabForm(
		this, 
		new wxPanel(this),
		wxID_ANY, 
		wxDefaultPosition,
		wxDefaultSize,
		wxNO_FULL_REPAINT_ON_RESIZE |
		wxCLIP_CHILDREN |
		wxTAB_TRAVERSAL);

	return pForm;
}

C_LocForm* C_Frame::CreateLocForm()
{
	C_LocForm* pLocForm = new C_LocForm(
		this, 
		wxID_ANY, 
		wxDefaultPosition,
		wxDefaultSize,
		wxNO_FULL_REPAINT_ON_RESIZE |
		wxCLIP_CHILDREN |
		wxTAB_TRAVERSAL);

	return pLocForm;
}

wxAuiNotebook* C_Frame::CreateNotebook()
{
	wxSize client_size = GetClientSize();

	m_pNotebook = new wxAuiNotebook(this, wxID_ANY,
									wxPoint(client_size.x, client_size.y),
									wxSize(430,200),
									wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

//	m_pNotebook->AddPage(new wxTextCtrl(this,wxID_ANY, wxT("I just did something...")), wxT("Log Window") , false);

	return m_pNotebook;
}

void C_Frame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	const char* buildTime = Debug::GetBuildTime();
	const char* buildDate = Debug::GetBuildDate();
	const char* buildUser = Debug::GetBuildUser();
	const char* buildPc = Debug::GetBuildPc();
	const char* buildRevision = Debug::GetBuildRevision();

	wxString errorMessage;
	errorMessage.Printf(wxT("ActorStudio Revision %s\n\nBuilt by %s on %s\nBuild time:  %s %s"), 
		ToUnicode(buildRevision), 
		ToUnicode(buildUser), 
		ToUnicode(buildPc), 
		ToUnicode(buildDate), 
		ToUnicode(buildTime) );

	wxMessageBox(errorMessage, wxT("About"), wxICON_INFORMATION);
}

void C_Frame::OnIdle(wxIdleEvent& idle)
{
	wxGetApp().IdleTick();
}

bool C_Frame::AddUIPane(wxWindow *pWindow, wxAuiPaneInfo info, E_UIPanelType type)
{
	/*
	info.FloatingPosition(Tools::Registry::Read(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("Pos"), wxDefaultPosition));
	info.FloatingSize(Tools::Registry::Read(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("Size"), wxSize(640,480)));
	if (Tools::Registry::HasValue(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("Docked"))) {
		if (Tools::Registry::Read(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("Docked"), true)) {
			info.Dock();
			info.Direction(Tools::Registry::Read(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockDir"), info.dock_direction));
			info.Layer(Tools::Registry::Read(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockLayer"), info.dock_layer));
			int position = Tools::Registry::Read(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockPos"), info.dock_pos);
			info.Position(position);
			info.Row(Tools::Registry::Read(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockRow"), info.dock_row));
			info.BestSize(Tools::Registry::Read(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockSizeX"), pWindow->GetSize().x),
				Tools::Registry::Read(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockSizeY"), pWindow->GetSize().y));
			info.Show(!Tools::Registry::Read(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockVisible"), info.HasFlag(wxAuiPaneInfo::optionHidden)));
		} else {
			info.Float();
		}
	}   */
	
	
	pWindow->Reparent(this);
	if (!m_pAuiManager->AddPane(pWindow, info))
		return false;

/*	wxMenu *pViewMenu = GetMenuBar()->GetMenu(GetMenuBar()->FindMenu(_T("View")));
	if (pViewMenu && type)
	{
		wxMenu *pParentMenu = 0;

		if (type == E_UIPanelType_ToolBar)
			pParentMenu = pViewMenu->FindItem(pViewMenu->FindItem(_T("Tool Bars")))->GetSubMenu();
		else //if (type == E_UIPanelType_Window)
			pParentMenu = pViewMenu->FindItem(pViewMenu->FindItem(_T("Windows")))->GetSubMenu();

		wxMenuItem *pMenuItem = pParentMenu->AppendCheckItem(wxID_ANY, info.caption);
		m_MenuItemIdWindowMap[pMenuItem->GetId()] = pWindow;
		Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetVisibleUIPane));
	}*/

	m_pAuiManager->Update();

	return true;
}

void C_Frame::RemoveUIPane(wxWindow *pWindow)
{
	wxAuiPaneInfo& info = m_pAuiManager->GetPane(pWindow);
	Tools::Registry::Write(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("Pos"), info.floating_pos);
	Tools::Registry::Write(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("Size"), info.floating_size);
	Tools::Registry::Write(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("Docked"), info.IsDocked());
	Tools::Registry::Write(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockDir"), info.dock_direction);
	Tools::Registry::Write(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockPos"), info.dock_pos);
	Tools::Registry::Write(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockRow"), info.dock_row);
	Tools::Registry::Write(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockLayer"), info.dock_layer);
	Tools::Registry::Write(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockSizeX"), pWindow->GetSize().x);
	Tools::Registry::Write(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockSizeY"), pWindow->GetSize().y);
	Tools::Registry::Write(wxString::FromUTF8(kRegKeyPath), info.name + wxString::FromUTF8("DockVisible"), info.HasFlag(wxAuiPaneInfo::optionHidden));
	
	/*wxMenu *pViewMenu = GetMenuBar()->GetMenu(GetMenuBar()->FindMenu(_T("View")));
	if (pViewMenu)
	{
		wxMenu *pWindowMenu = pViewMenu->FindItem(pViewMenu->FindItem(_T("Windows")))->GetSubMenu();
		assert(pWindowMenu && "Failed to find 'Window' menu");

		S32 index = pWindowMenu->FindItem(info.caption);
		if (index != -1)
		{
			wxMenuItem *pMenuItem = pWindowMenu->FindItem(index);
			Disconnect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED);
			m_MenuItemIdWindowMap.Erase(index);

			pWindowMenu->Delete(pMenuItem);
		}
	}*/

	m_pAuiManager->DetachPane(pWindow);
	m_pAuiManager->Update();
}

}//namespace DMan
