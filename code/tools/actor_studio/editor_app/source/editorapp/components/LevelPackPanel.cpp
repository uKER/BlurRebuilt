#include "stdafx.h"

#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/LevelPack.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Components/LevelPackPanel.h>
#include <EditorApp/Tasks/LevelTasks.h>

namespace Editor
{

class PackWrapper : public wxPanel
{
public:
	PackWrapper(wxWindow *parent,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const wxString& name = wxPanelNameStr)
	: wxPanel(parent, winid, pos, size, style, name)
	{
	}

	bool MSWShouldPreProcessMessage(WXMSG* msg)
	{
		return __super::MSWShouldPreProcessMessage(msg);
	}

	bool MSWTranslateMessage(WXMSG* msg)
	{
		if (msg->message == WM_KEYDOWN)
		{
			if (wxWindow::MSWTranslateMessage(msg))
				return true;
			else
			{
				//send a new event to parent panel.
				//This is because ::IsDialog() will dispatch messages to the underlying ScarabForm
				//so we'll never see it again in wxWidgets land
				::SendMessage((HWND)GetHWND(), msg->message, msg->wParam, msg->lParam);
				return false;
			}
		}
		else
			return __super::MSWTranslateMessage(msg);
	}
};

C_LevelPackPanelExplorer::C_LevelPackPanelExplorer(C_LevelPackPanel* panel)
{
	m_pPanel = panel;
	SetFilterEmpty(true);
	SetAllowRename(false);
}

bool C_LevelPackPanelExplorer::OnFilterItem(Explorer::ExplorerData* pData)
{
	if (pData->GetType() == kExItemTypeRecord)
	{
		EditorLib::I_Level* pLevel = EditorLib::C_Core::Instance().GetLevelManager()->GetLevel(pData->GetDomRecord()->id);
		if (pLevel)
			return true;
		else
			return false;
	}
	return true;
}

void C_LevelPackPanelExplorer::OnContextMenu(Explorer::ExplorerData* item, const WinPoint& pt)
{
	if (item->GetType() == kExItemTypeRecord || item->GetType() == kExItemTypePack)
		m_pPanel->ShowMenu(item, pt);
}

void C_LevelPackPanelExplorer::OnActivateItem(Explorer::ExplorerData* pData)
{
	if (pData->GetType() == kExItemTypeRecord)
	{
		EditorLib::C_LevelPack* pack = EditorLib::C_Core::Instance().GetLevelManager()->GetPack(Xt::PackOf(pData->GetDomRecord())->id);
		EditorLib::I_Level* pLevel = pack->GetLevel(pData->GetDomRecord()->id);
		BIZ_ASSERT(pLevel->GetType() == EditorLib::E_LevelType_XtLevel);

		Tasks::SetActiveLevel((EditorLib::C_XtLevel*)pLevel);
	}
}

C_LevelPackPanel::C_LevelPackPanel(
			wxWindow *parent, 
			wxWindowID id,
			const wxPoint& pos,
			const wxSize& size,
			long style)
			: wxPanel(parent, id, pos, size, style),
			m_slots(this)
{
	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer( wxVERTICAL );
	SetSizer(pSizer0);

	m_pExplorer = new C_LevelPackPanelExplorer(this);
	m_pTreeCtrlPanel = new PackWrapper(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);

	m_pExplorer->Create((HWND)m_pTreeCtrlPanel->GetHandle());

	m_pExplorer->SetDb(EditorLib::C_Core::Instance().GetXtManager().GetDb());

	m_pExplorer->LoadIcon("./icons/Project.bmp", kExItemTypeRoot);
	m_pExplorer->LoadIcon("./icons/FolderHiddenClosed.bmp", kExItemTypePack);
	m_pExplorer->LoadIcon("./icons/File.bmp", kExItemTypeRecord);
	m_pExplorer->LoadIcon("./icons/PackFolderClosed.bmp", kExItemTypeFilterPack);
	m_pExplorer->LoadIcon("./icons/PackFolderClosed.bmp", kExItemTypeFilterRecord);

	m_pExplorer->GetFlags().HasIcons = true;

	pSizer0->Add(m_pTreeCtrlPanel, 1, wxALL|wxEXPAND, 0);
	
	WinRect rcClient((HWND)m_pTreeCtrlPanel->GetHandle());
	SetWindowPos(*m_pExplorer, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pTreeCtrlPanel->Connect(wxEVT_SIZE, wxSizeEventHandler(C_LevelPackPanel::OnSize), NULL, this);

	m_slots.Connect();
}

C_LevelPackPanel::~C_LevelPackPanel()
{
}

void C_LevelPackPanel::Shutdown()
{
	SAFE_DELETE_PTR(m_pExplorer);
	m_slots.Disconnect();
}

void C_LevelPackPanel::ShowMenu(Explorer::ExplorerData* item, const WinPoint& pt)
{
	pt.ScreenToClient((HWND)this);
	wxPoint wxPt(pt.x, pt.y);

	enum E_MenuItems
	{
		E_MenuItem_ClosePack,
		E_MenuItem_LoadLevel,
		E_MenuItem_UnloadLevel,
		E_MenuItem_SetActiveLevel,
	};

	m_pRCRecord = item;

	wxString title;
	wxMenu menu(WcToMb(item->GetLabel()).Text());
    
	if (item->GetType() == kExItemTypeRecord)
	{
		menu.Append(E_MenuItem_LoadLevel, "Load");
		menu.Append(E_MenuItem_UnloadLevel, "Unload");
		menu.Append(E_MenuItem_SetActiveLevel, "Set Active");
	} else if (item->GetType() == kExItemTypePack)
	{
		menu.Append(E_MenuItem_ClosePack, "Close Pack");
	}

	Connect(E_MenuItem_ClosePack, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_LevelPackPanel::OnClosePack));
	Connect(E_MenuItem_LoadLevel, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_LevelPackPanel::OnLoadLevel));
	Connect(E_MenuItem_UnloadLevel, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_LevelPackPanel::OnUnloadLevel));
	Connect(E_MenuItem_SetActiveLevel, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_LevelPackPanel::OnSetActiveLevel));
	
	PopupMenu(&menu, wxPt);

	Disconnect(E_MenuItem_ClosePack, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(E_MenuItem_LoadLevel, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(E_MenuItem_UnloadLevel, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(E_MenuItem_SetActiveLevel, wxEVT_COMMAND_MENU_SELECTED);
}

void C_LevelPackPanel::OnDatabaseEvent(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt)
{
	if (evt == EditorLib::E_DatabaseEvent_Opened)
	{
		m_pExplorer->SetDb(&db);
	} else if (evt == EditorLib::E_DatabaseEvent_Closing)
	{
		m_pExplorer->SetDb(0);
	}
}

void C_LevelPackPanel::OnPackEvent(const EditorLib::C_LevelPack& levelPack, EditorLib::E_PackEvent e)
{
	m_pExplorer->UpdateFilter();
}

void C_LevelPackPanel::OnSize(wxSizeEvent& e)
{
	if (!m_pExplorer)
		return;

	WinRect rcClient((HWND)m_pTreeCtrlPanel->GetHandle());
	SetWindowPos(*m_pExplorer, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void C_LevelPackPanel::OnClosePack(wxCommandEvent& e)
{
	EditorLib::C_LevelPack* pack = EditorLib::C_Core::Instance().GetLevelManager()->GetPack(m_pRCRecord->GetDomPack()->id);
	Tasks::ClosePack(pack);
}

void C_LevelPackPanel::OnLoadLevel(wxCommandEvent& e)
{
	EditorLib::C_LevelPack* pack = EditorLib::C_Core::Instance().GetLevelManager()->GetPack(Xt::PackOf(m_pRCRecord->GetDomRecord())->id);
	pack->LoadLevel(m_pRCRecord->GetDomRecord()->id);
}

void C_LevelPackPanel::OnUnloadLevel(wxCommandEvent& e)
{
	EditorLib::C_LevelPack* pack = EditorLib::C_Core::Instance().GetLevelManager()->GetPack(Xt::PackOf(m_pRCRecord->GetDomRecord())->id);
	Tasks::CloseLevel(pack->GetLevel(m_pRCRecord->GetDomRecord()->id));
}

void C_LevelPackPanel::OnSetActiveLevel(wxCommandEvent& e)
{
	EditorLib::C_LevelPack* pack = EditorLib::C_Core::Instance().GetLevelManager()->GetPack(Xt::PackOf(m_pRCRecord->GetDomRecord())->id);
	EditorLib::I_Level* pLevel = pack->GetLevel(m_pRCRecord->GetDomRecord()->id);
	BIZ_ASSERT(pLevel->GetType() == EditorLib::E_LevelType_XtLevel);

	Tasks::SetActiveLevel((EditorLib::C_XtLevel*)pLevel);
}

C_LevelPackPanel::C_Slots::C_Slots(C_LevelPackPanel* pLevelPackPanel)
{
	m_pLevelPackPanel = pLevelPackPanel;
}

void C_LevelPackPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.connect(this, &Editor::C_LevelPackPanel::C_Slots::OnDatabaseEventSlot);
	s.packEvent.connect(this, &Editor::C_LevelPackPanel::C_Slots::OnPackEventSlot);
}

void C_LevelPackPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.disconnect(this);
	s.packEvent.disconnect(this);
}

void C_LevelPackPanel::C_Slots::OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt)
{
	m_pLevelPackPanel->OnDatabaseEvent(db, evt);
}

void C_LevelPackPanel::C_Slots::OnPackEventSlot(const EditorLib::C_LevelPack& levelPack, EditorLib::E_PackEvent evt)
{
	m_pLevelPackPanel->OnPackEvent(levelPack, evt);
}

}