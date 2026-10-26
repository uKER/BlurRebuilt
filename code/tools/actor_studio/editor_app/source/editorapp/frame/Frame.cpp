#include "stdafx.h"

#include <wx/aui/aui.h>
#include <wx/progdlg.h>
#include <wx/display.h>

#ifdef USE_WXFLATNOTEBOOK
#include <wx/wxflatnotebook/wxFlatNotebook.h>
#endif

#include <system/misc/Progress.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Core.h>

#include <EditorApp/Components/Progress.h>
#include <EditorApp/Components/SplitterControl.h>
#include <EditorApp/Frame/Menus/FrameMenuBar.h>
#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/Registry.h>
#include <EditorApp/Misc/ViewportHelpers.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/Manipulators/Contexts/IManipulatorContext.h>
#include <EditorApp/Editor/Manipulators/Manipulator.h>
#include <EditorApp/Editor/ViewportPanel/ViewportPanel.h>
#include <EditorApp/App.h>

using namespace Editor;

BEGIN_EVENT_TABLE(C_Frame, wxFrame)
EVT_IDLE(C_Frame::OnIdle)
END_EVENT_TABLE()

C_Frame::C_Frame(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
: wxFrame(NULL, wxID_ANY, title, pos, size, style, wxFrameNameStr)
, m_pAuiManager(0)
, m_pMenuBar(0)
, m_pNotebook(0)
, m_pStatusBar(0)
{
}

bool C_Frame::Initialise(const Util::C_String& projectName)
{
	SetPosition(Registry::Read(kRegKeyPath, "framePos", wxDefaultPosition));
	SetSize(Registry::Read(kRegKeyPath, "frameSize", wxSize(800,600)));

	if (wxDisplay::GetFromWindow(this) == -1)
		SetPosition(wxDefaultPosition);	

	if(Registry::Read(kRegKeyPath, "isMaximized", false))
	{
		Maximize();
	}

	{
		wxIcon icon;
		icon.LoadFile("\"IDC_ATVI_ICON\"", wxBITMAP_TYPE_ICO_RESOURCE);
		SetIcon(icon);
	}

	m_pMenuBar = new C_FrameMenuBar(this);

	if (!m_pMenuBar->InitialiseUI(projectName))
		return false;

	m_pStatusBar = new C_ProgressStatusBar(this, wxID_ANY, wxST_SIZEGRIP);

	SetStatusBar(m_pStatusBar);
		
	m_pAuiManager = new wxAuiManager();
	m_pAuiManager->SetManagedWindow(this);
	m_pAuiManager->SetFlags(wxAUI_MGR_ALLOW_FLOATING |  
							wxAUI_MGR_TRANSPARENT_DRAG | 
							wxAUI_MGR_TRANSPARENT_HINT | 
							wxAUI_MGR_NO_VENETIAN_BLINDS_FADE);

	m_pAuiManager->GetArtProvider()->SetMetric(wxAUI_DOCKART_SASH_SIZE, 2);
	m_pAuiManager->GetArtProvider()->SetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE, 1);
	m_pAuiManager->GetArtProvider()->SetMetric(wxAUI_DOCKART_CAPTION_SIZE, 17);
	m_pAuiManager->GetArtProvider()->SetMetric(wxAUI_DOCKART_GRADIENT_TYPE, wxAUI_GRADIENT_HORIZONTAL);

	#ifdef _DEBUG
	Debug::GetRootLogger().SetLevel(DebugLevel::Debug);
	#else
	Debug::AutoConfigureDebugLog();
	#endif
	

	// Main viewport pane
	m_pSplitterCtrl = new wxSplitterCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(200, 200), wxNO_BORDER);
	
	// Now that each viewport can have a different camera attached, we can differentiate between them with names such as 
	// "one", "two"..etc, which is useful to load/save setttings between sessions. See C_RenderManager::OnViewportCreate and 
	// C_ViewportShadingMenu::~C_ViewportShadingMenu()

	C_RenderViewport *pPersp = new C_RenderViewport(this, wxID_ANY, wxDefaultPosition, wxSize(200, 200), wxNO_BORDER, wxDefaultValidator, "one");
	//initialise d3d device with this window
	if (!C_RenderCore::Instance().Initialise((HWND)pPersp->GetHandle()))
		return false;

	if (!pPersp->Create())	return false;
	pPersp->SetRefreshRate(0);
	C_ViewportHelpers::SetViewportCamera(pPersp, C_ViewportHelpers::E_ViewportView_Perspective);

	C_ViewportPanel *pPerspPanel = new C_ViewportPanel(this, wxID_ANY);
	pPerspPanel->SetViewport(pPersp);
	pPerspPanel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVEBORDER));

	C_RenderViewport *pSide = new C_RenderViewport(this, wxID_ANY, wxDefaultPosition, wxSize(200, 200), wxNO_BORDER, wxDefaultValidator, "two");
	if (!pSide->Create()) return false;
	pSide->SetRefreshRate(0);
	C_ViewportHelpers::SetViewportCamera(pSide, C_ViewportHelpers::E_ViewportView_Side);

	C_ViewportPanel *pSidePanel = new C_ViewportPanel(this, wxID_ANY);
	pSidePanel->SetViewport(pSide);

	C_RenderViewport *pFront = new C_RenderViewport(this, wxID_ANY, wxDefaultPosition, wxSize(200, 200), wxNO_BORDER, wxDefaultValidator, "three");
	if (!pFront->Create()) return false;
	pFront->SetRefreshRate(0);
	C_ViewportHelpers::SetViewportCamera(pFront, C_ViewportHelpers::E_ViewportView_Front);
	
	C_ViewportPanel *pFrontPanel = new C_ViewportPanel(this, wxID_ANY);
	pFrontPanel->SetViewport(pFront);

	C_RenderViewport *pTop = new C_RenderViewport(this, wxID_ANY, wxDefaultPosition, wxSize(200, 200), wxNO_BORDER, wxDefaultValidator, "four");
	if (!pTop->Create()) return false;
	pTop->SetRefreshRate(0);
	C_ViewportHelpers::SetViewportCamera(pTop, C_ViewportHelpers::E_ViewportView_Top);

	C_ViewportPanel *pTopPanel = new C_ViewportPanel(this, wxID_ANY);
	pTopPanel->SetViewport(pTop);

	m_pSplitterCtrl->Split(E_SplitterLayout_4, pTopPanel, pPerspPanel, pFrontPanel, pSidePanel);
	m_pSplitterCtrl->ExpandWindow(pPerspPanel);
	m_pAuiManager->AddPane(m_pSplitterCtrl, wxAuiPaneInfo().
		Name(wxT("Pane_Viewports")).Caption(wxT("Viewports")).
		Center().Layer(0).Position(0).CaptionVisible(false).
		CloseButton(false).MaximizeButton(true).PaneBorder(false));

	// Attribute notebook...
#ifdef USE_WXFLATNOTEBOOK
    m_pNotebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(200,400));
#else
    m_pNotebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(200,400), wxAUI_NB_TOP|wxAUI_NB_TAB_SPLIT|wxAUI_NB_TAB_MOVE|wxAUI_NB_TAB_EXTERNAL_MOVE|wxAUI_NB_SCROLL_BUTTONS|wxNO_BORDER);
	m_pNotebook->SetArtProvider(new wxAuiSimpleTabArt);
#endif
	AddUIPane(
		m_pNotebook, 
		wxAuiPaneInfo().
		Name(_T("Pane_Attribute")).Caption(_T("Attribute")).
		Right().Layer(0).Position(0).CloseButton(true).Show(false).Float());

	pPersp->Freeze();
	pFront->Freeze();
	pSide->Freeze();
	pTop->Freeze();

	m_pAuiManager->Update();

	pPersp->Thaw();
	pFront->Thaw();
	pSide->Thaw();
	pTop->Thaw();

	//Delegate<void(wxKeyEvent&)> keyDelegate(m_pRenderManager, &C_RenderManager::OnKeyDown);
	//keyDelegate(wxKeyEvent());
	//ConnectEvent1<wxKeyEvent, Delegate<void(wxKeyEvent&)>>(m_pLogText, wxEVT_KEY_DOWN, keyDelegate, wxID_ANY);

	Connect(wxID_ANY, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_Frame::OnUpdateUI));

	LOGINFO(g_Log, "Initialisation successful\n");
	return true;
}

C_Frame::~C_Frame()
{
	C_RenderCore::Instance().Shutdown();
	//Need to get the normal window placement irrespective of max/min-imised state.
	//wxWidgets does't seem to encapsulate this info..
	{
		wxPoint pos = wxDefaultPosition;
		wxSize size = wxDefaultSize;

		WINDOWPLACEMENT placement;
		if (::GetWindowPlacement((HWND)GetHandle(), &placement))
		{
			const RECT& r = placement.rcNormalPosition;
			pos = wxPoint(r.left, r.top);
			size = wxSize(r.right - r.left, r.bottom - r.top);
		}

		Registry::Write(kRegKeyPath, "frameSize", size);
		Registry::Write(kRegKeyPath, "framePos", pos);
		Registry::Write(kRegKeyPath, "isMaximized", IsMaximized());
	}

	//delete any orphaned windows (as a result of tab hiding in the attribute panel)
	Ut::UtMap<S32, wxWindow*>::Iterator it = m_MenuItemIdWindowMap.Begin();
	while (it != m_MenuItemIdWindowMap.End())
	{
		if (!it->GetValue()->GetParent())
			delete it->GetValue();
		it++;
	}

#ifdef USE_WXFLATNOTEBOOK
	if (m_pNotebook)
	{
		m_pNotebook->DeleteAllPages();
		wxFlatNotebook::CleanUp();
	}
#endif

	if (m_pAuiManager)
	{
		RemoveUIPane(m_pNotebook);
		m_pAuiManager->UnInit();
		SAFE_DELETE_PTR(m_pAuiManager)
	}

	SAFE_DELETE_PTR(m_pMenuBar);
}

void C_Frame::OnUpdateUI(wxUpdateUIEvent &e)
{
	Ut::UtMap<S32, wxWindow*>::Iterator it = m_MenuItemIdWindowMap.Find(e.GetId());
	if (it != m_MenuItemIdWindowMap.End())
	{
		if (it->GetValue()->GetParent() && 
			(it->GetValue()->GetParent()->IsKindOf(CLASSINFO(wxFlatNotebook)) || it->GetValue()->GetParent()->IsKindOf(CLASSINFO(wxFlatNotebook))))
			e.Check(true);
		else
		{
			wxAuiPaneInfo& info = m_pAuiManager->GetPane(it->GetValue());
			e.Check(info.IsShown() && it->GetValue()->GetParent());
		}
	}
	else
		e.Skip();
}

void C_Frame::OnSetVisibleUIPane(wxCommandEvent& e)
{

	Ut::UtMap<S32, wxWindow*>::Iterator it = m_MenuItemIdWindowMap.Find(e.GetId());
	if (it != m_MenuItemIdWindowMap.End())
	{
		wxWindow *pWindow = it->GetValue();
		wxMenuItem *pMenuItem = GetMenuBar()->FindItem(e.GetId());

		if (!pWindow->GetParent())
		{
			if (m_AttributeTabLabels.Find(pWindow) != m_AttributeTabLabels.End())
			{
				m_pNotebook->AddPage(pWindow, m_AttributeTabLabels[pWindow].Chars());
				pWindow->Show();
			}
		}
		else if (pWindow->GetParent()->IsKindOf(CLASSINFO(wxFlatNotebook)))
		{
			wxFlatNotebook *pParent = (wxFlatNotebook*)pWindow->GetParent();

			//we cannot hide tabs, so instead remove the child window
			assert(!pMenuItem->IsChecked());
			S32 index = pParent->GetPageIndex(pWindow);
			pParent->RemoveChild(pWindow);
			pParent->RemovePage(index);
			pParent->Update();
			pWindow->Hide();
		}
		else if (GetParent() && GetParent()->IsKindOf(CLASSINFO(wxAuiNotebook)))
		{
			wxAuiNotebook *pParent = (wxAuiNotebook*)pWindow->GetParent();

			assert(!pMenuItem->IsChecked());
			S32 index = pParent->GetPageIndex(pWindow);
			pParent->RemoveChild(pWindow);
			pParent->RemovePage(index);
			pParent->Update();
			pWindow->Hide();
		}
		else
		{
			wxAuiPaneInfo& info = m_pAuiManager->GetPane(pWindow);
			if (info.window == pWindow)
			{
				if (!pMenuItem->IsChecked() && info.IsMaximized())
					m_pAuiManager->RestorePane(info);

				info.Show(pMenuItem->IsChecked());
				if (info.frame && info.window)
					//HACK: propagate to child
					info.window->Show(pMenuItem->IsChecked());
				m_pAuiManager->Update();
			}
		}
	}
}

void C_Frame::OnSetViewLayout(wxCommandEvent& e)
{
	m_pSplitterCtrl->ChangeLayout((E_SplitterLayout)((S32)e.GetId()));
	m_pSplitterCtrl->Refresh();
	e.Skip();
}

bool C_Frame::AddUIPane(wxWindow *pWindow, wxAuiPaneInfo& info, E_UIPanelType type)
{
	if (!info.IsToolbar())
	{
		info.FloatingPosition(Registry::Read(kRegKeyPath, info.name + "Pos", wxDefaultPosition));
		info.FloatingSize(Registry::Read(kRegKeyPath, info.name + "Size", wxSize(640,480)));
		if (Registry::HasValue(kRegKeyPath, info.name + "Docked")) {
			if (Registry::Read(kRegKeyPath, info.name + "Docked", true)) {
				info.Dock();
				info.Direction(Registry::Read(kRegKeyPath, info.name + "DockDir", info.dock_direction));
				info.Layer(Registry::Read(kRegKeyPath, info.name + "DockLayer", info.dock_layer));
				int position = Registry::Read(kRegKeyPath, info.name + "DockPos", info.dock_pos);
				info.Position(position);
				info.Row(Registry::Read(kRegKeyPath, info.name + "DockRow", info.dock_row));
				info.BestSize(Registry::Read(kRegKeyPath, info.name + "DockSizeX", pWindow->GetSize().x),
					Registry::Read(kRegKeyPath, info.name + "DockSizeY", pWindow->GetSize().y));
				
			} else {
				info.Float();
			}
		}

		//"DockVisible" is an unfortunate name to control the visibility of either floatable or docked panels
		info.Show(!Registry::Read(kRegKeyPath, info.name + "DockVisible", info.HasFlag(wxAuiPaneInfo::optionHidden)));
	}
	
	pWindow->Reparent(this);
	if (!m_pAuiManager->AddPane(pWindow, info))
		return false;

	wxMenu *pViewMenu = GetMenuBar()->GetMenu(GetMenuBar()->FindMenu(_T("View")));
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
	}

	m_pAuiManager->Update();

	info = m_pAuiManager->GetPane(pWindow);
	if (!info.IsToolbar() && info.frame && info.window)
		//HACK: propagate to child
		info.window->Show(!Registry::Read(kRegKeyPath, info.name + "DockVisible", info.HasFlag(wxAuiPaneInfo::optionHidden)));

	if (wxDisplay::GetFromWindow(pWindow) == -1)
	{
		wxAuiPaneInfo& pane = m_pAuiManager->GetPane(pWindow);
		pane.FloatingPosition(wxPoint(0,0));
		m_pAuiManager->Update();
	}

	return true;
}

void C_Frame::RemoveUIPane(wxWindow *pWindow)
{
	if (!pWindow)
		return;

	wxAuiPaneInfo& info = m_pAuiManager->GetPane(pWindow);
	if (!info.IsToolbar())
	{
		Registry::Write(kRegKeyPath, info.name + "Pos", info.floating_pos);
		Registry::Write(kRegKeyPath, info.name + "Size", info.floating_size);
		Registry::Write(kRegKeyPath, info.name + "Docked", info.IsDocked());
		Registry::Write(kRegKeyPath, info.name + "DockDir", info.dock_direction);
		Registry::Write(kRegKeyPath, info.name + "DockPos", info.dock_pos);
		Registry::Write(kRegKeyPath, info.name + "DockRow", info.dock_row);
		Registry::Write(kRegKeyPath, info.name + "DockLayer", info.dock_layer);
		Registry::Write(kRegKeyPath, info.name + "DockSizeX", pWindow->GetSize().x);
		Registry::Write(kRegKeyPath, info.name + "DockSizeY", pWindow->GetSize().y);
		Registry::Write(kRegKeyPath, info.name + "DockVisible", info.HasFlag(wxAuiPaneInfo::optionHidden));
	}
	
	wxMenu *pViewMenu = GetMenuBar()->GetMenu(GetMenuBar()->FindMenu(_T("View")));
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
	}

	m_pAuiManager->DetachPane(pWindow);
	m_pAuiManager->Update();
}

bool C_Frame::AddAttributePanel(wxWindow *pWindow, const char* pName)
{
	pWindow->Reparent(m_pNotebook);
	if (!m_pNotebook->AddPage(pWindow, pName))
		return false;

	wxMenu *pViewMenu = GetMenuBar()->GetMenu(GetMenuBar()->FindMenu(_T("View")));
	if (pViewMenu)
	{
		wxMenu *pParentMenu = pViewMenu->FindItem(pViewMenu->FindItem(_T("Attributes")))->GetSubMenu();

		wxMenuItem *pMenuItem = pParentMenu->AppendCheckItem(wxID_ANY, pName);
		m_MenuItemIdWindowMap[pMenuItem->GetId()] = pWindow;
		m_AttributeTabLabels[pWindow] = pName;

		Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetVisibleUIPane));
	}

	return true;
}

void C_Frame::RemoveAttributePanel(wxWindow *pWindow)
{
	wxMenu *pViewMenu = GetMenuBar()->GetMenu(GetMenuBar()->FindMenu(_T("View")));
	wxMenu *pAttributeMenu = pViewMenu->FindItem(pViewMenu->FindItem(_T("Attributes")))->GetSubMenu();

	Ut::UtMap<S32, wxWindow*>::Iterator it = m_MenuItemIdWindowMap.Begin();
	while (it != m_MenuItemIdWindowMap.End())
	{
		if (it->GetValue() == pWindow)
		{
			S32 index = it->GetKey();
			wxMenuItem *pMenuItem = pAttributeMenu->FindItem(index);
			Disconnect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED);
			pAttributeMenu->Delete(pMenuItem);

			Ut::UtMap<wxWindow*, Util::C_String>::Iterator it2 = m_AttributeTabLabels.Find(pWindow);
			if (it2!=m_AttributeTabLabels.End())
				m_AttributeTabLabels.Erase(it2);

			m_MenuItemIdWindowMap.Erase(it);

			for (size_t i=0; i<m_pNotebook->GetPageCount(); i++)
			{
				if (m_pNotebook->GetPage(i) == pWindow)
				{
					m_pNotebook->DeletePage(i);
					break;
				}
			}

			break;
		}
		else
			it++;
	}
	m_pNotebook->Update();
}

Progress::I_ProgressBar* C_Frame::GetProgressBar()
{
	return m_pStatusBar;
}

const Progress::I_ProgressBar* C_Frame::GetProgressBar() const
{
	return m_pStatusBar;
}

wxAuiManager* C_Frame::GetAuiManager()
{
	return m_pAuiManager;
}

const wxAuiManager* C_Frame::GetAuiManager() const
{
	return m_pAuiManager;
}

C_FrameMenuBar* C_Frame::GetFrameMenuBar()
{
	return m_pMenuBar;
}

void C_Frame::OnIdle(wxIdleEvent& idle)
{
	EditorLib::C_Core::Instance().IdleTick();
}
