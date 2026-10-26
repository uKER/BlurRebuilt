#include "stdafx.h"

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Misc/Verification.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>

#include <EditorApp/Components/Scene_Graph/SelectionSceneGraphPanel.h>
#include <EditorApp/Components/SplitterControl.h>
#include <EditorApp/Components/VerificationPanel.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Input_Handlers/ShortcutHandler.h>
#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Tasks/UITasks.h>
#include <EditorApp/App.h>

using namespace Editor;

bool C_VerificationTreeItem::OnItemDeleting()
{
	return false;
}

bool C_VerificationTreeItem::OnStartRename(Sz16& newName)
{
	return false;
}

C_VerificationTreeItemLevel::C_VerificationTreeItemLevel(C_VerificationPanel* pPanel, XtId xtId)
{
	m_pVerificationPanel = pPanel;
	m_XtId = xtId;
}

void C_VerificationTreeItemLevel::OnContextMenu(const WinPoint& pt)
{
	enum ExMenuItem
	{
		kExMenuItemOpenLevel = 1,
	};

	HMENU hContextMenu = CreatePopupMenu();

	AppendMenu(hContextMenu, MF_STRING, kExMenuItemOpenLevel, "OpenLevel");

	POINT winPt;
	winPt.x = pt.x;
	winPt.y = pt.y;
	ClientToScreen((HWND)m_pVerificationPanel->GetHandle(), &winPt);

	switch (TrackPopupMenuEx(hContextMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD, winPt.x, winPt.y, (HWND)m_pVerificationPanel->GetHandle(), 0))
	{
	case kExMenuItemOpenLevel:
		{
			Tasks::Open(m_XtId, true);
			break;
		}
	}	

	DestroyMenu(hContextMenu);
}

C_VerificationTreeItemActor::C_VerificationTreeItemActor(XtId xtId)
{
	m_XtId = xtId;
}

bool C_VerificationTreeItemActor::OnActivate()
{
	Tasks::ShowNodePropertiesWindow();
	return true;
}

bool C_VerificationTreeItemActor::OnItemSelectionChanged(bool selected)
{
	if (selected)
	{
		EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", Btl::String::Format("%x|ffffffff", m_XtId));
		const RenderViewportArray& viewports = C_RenderCore::Instance().GetRenderViewports();
		for (RenderViewportArray::ConstIterator viewport = viewports.Begin(); viewport != viewports.End(); ++viewport)
		{
			C_RenderCore::Instance().FrameSelection(*viewport);
		}
	}

	return true;
}

C_VerificationTreeItemXt::C_VerificationTreeItemXt(XtId xtId)
{
	m_XtId = xtId;
}

bool C_VerificationTreeItemXt::OnActivate()
{
	// TODO : Open sorcery window
	return true;
}

C_VerificationTree::C_VerificationTree()
{
}

C_VerificationPanel::C_VerificationPanel(wxWindow *parent, wxWindowID winid, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
: wxPanel(parent, winid, pos, size, style, name)
{
	wxBoxSizer* pSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(pSizer);

	m_ResultsPanel = new wxPanel(this);
	m_LevelVerificationButton = new wxButton(this, wxID_ANY, "Verify Current Level");
	m_AllLevelVerificationButton = new wxButton(this, wxID_ANY, "Verify All Levels");
	m_DbVerificationButton = new wxButton(this, wxID_ANY, "Verify Database");
	
	pSizer->Add(m_ResultsPanel, 1, wxEXPAND, 2);

	wxBoxSizer* pButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	pButtonSizer->Add(m_LevelVerificationButton, 0);
	pButtonSizer->Add(m_AllLevelVerificationButton, 0);

	pSizer->Add(pButtonSizer, 0, wxALIGN_RIGHT);

	m_Results = new C_VerificationTree();
	m_Results->Create((HWND)m_ResultsPanel->GetHandle());
	m_Results->GetFlags().HasIcons = false;

	m_ResultsPanel->Connect(wxEVT_SIZE, wxSizeEventHandler(C_VerificationPanel::OnSize), NULL, this);
	m_LevelVerificationButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_VerificationPanel::OnButton), NULL, this);
	m_AllLevelVerificationButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_VerificationPanel::OnButton), NULL, this);
	m_DbVerificationButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_VerificationPanel::OnButton), NULL, this);

	Layout();
}

C_VerificationPanel::~C_VerificationPanel()
{
	SAFE_DELETE_PTR(m_Results);
}

void C_VerificationPanel::Shutdown()
{
	SAFE_DELETE_PTR(m_Results);
	m_ResultsPanel->Disconnect(wxEVT_SIZE, wxSizeEventHandler(C_VerificationPanel::OnSize), NULL, this);
	m_LevelVerificationButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_VerificationPanel::OnButton), NULL, this);
	m_AllLevelVerificationButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_VerificationPanel::OnButton), NULL, this);
	m_DbVerificationButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_VerificationPanel::OnButton), NULL, this);
}

void C_VerificationPanel::OnSize(wxSizeEvent& e)
{
	if (!m_Results)
		return;

	{
		WinRect rcClient((HWND)m_ResultsPanel->GetHandle());
		SetWindowPos(*m_Results, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
}

void C_VerificationPanel::OnButton(wxCommandEvent& e)
{
	EditorLib::C_Core::Instance().GetVerificationSystem().Clear();

	if (e.GetId() == m_LevelVerificationButton->GetId())
	{
		if (EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel())
			EditorLib::C_Core::Instance().GetVerificationSystem().VerifyLevel(EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId());
	} else if (e.GetId() == m_AllLevelVerificationButton->GetId()) {
		EditorLib::C_Core::Instance().GetVerificationSystem().VerifyAllLevels();
	} else {
		EditorLib::C_Core::Instance().GetVerificationSystem().VerifyDatabase();
	}

	Populate();
}

void C_VerificationPanel::Populate()
{
	C_VerificationTreeItem* root = new C_VerificationTreeItem();
	root->SetLabel("Root");

	m_Results->SetRootNode(root);

	C_VerificationTreeItem* results = new C_VerificationTreeItem();
	results->SetLabel("Results");

	root->AddChild(results);

	EditorLib::C_VerificationSystem& verificationSystem = EditorLib::C_Core::Instance().GetVerificationSystem();

	const EditorLib::C_VerificationSystem::StringGroupHash& groups = verificationSystem.GetGroups();

	for (EditorLib::C_VerificationSystem::StringGroupHash::ConstIterator groupIt = groups.Begin(); groupIt != groups.End(); ++groupIt)
	{
		if (!groupIt->GetValue()->GetRecords().Size())
			continue;

		C_VerificationTreeItem* groupItem = new C_VerificationTreeItem();
		groupItem->SetLabel(groupIt->GetValue()->GetName().Chars());
		results->AddChild(groupItem);

		for (EditorLib::C_VerificationGroup::XtIdRecordHash::ConstIterator levelIt = groupIt->GetValue()->GetRecords().Begin(); levelIt != groupIt->GetValue()->GetRecords().End(); ++levelIt)
		{
			if (!levelIt->GetValue()->GetResults().Size())
				continue;

			C_VerificationTreeItem* levelItem = new C_VerificationTreeItemLevel(this, levelIt->GetValue()->GetXtId());
			levelItem->SetLabel(levelIt->GetValue()->GetName().Chars());
			groupItem->AddChild(levelItem);

			for (Btl::Vector<EditorLib::C_VerificationResult*>::ConstIterator resultIt = levelIt->GetValue()->GetResults().Begin(); resultIt != levelIt->GetValue()->GetResults().End(); ++resultIt)
			{
				C_VerificationTreeItem* resultItem = new C_VerificationTreeItem();
				resultItem->SetLabel((*resultIt)->AsString().Chars());
				levelItem->AddChild(resultItem);

				switch ((*resultIt)->GetErrorLevel())
				{
				case EditorLib::E_VerificationResultLevel_Debug:
					resultItem->SetColour(0,255,0);
					break;
				case EditorLib::E_VerificationResultLevel_Info:
					resultItem->SetColour(0,0,0);
					break;
				case EditorLib::E_VerificationResultLevel_Warning:
					resultItem->SetColour(255,128,0);
					break;
				case EditorLib::E_VerificationResultLevel_Error:
					resultItem->SetColour(200,0,0);
					break;
				case EditorLib::E_VerificationResultLevel_Fatal:
					resultItem->SetColour(255,0,0);
					break;
				}

				for (Btl::Vector<EditorLib::I_VerificationResultData*>::ConstIterator dataIt = (*resultIt)->GetData().Begin(); dataIt != (*resultIt)->GetData().End(); ++dataIt)
				{
					C_VerificationTreeItem* dataItem = 0;

					if ((*dataIt)->GetType() == EditorLib::E_VerificationResultDataType_Actor)
					{
						dataItem = new C_VerificationTreeItemActor(((const EditorLib::C_VerificationResultDataActor*)*dataIt)->GetXtData()->id);
						dataItem->SetLabel((((const EditorLib::C_VerificationResultDataActor*)*dataIt)->GetName() + " (" + ((const EditorLib::C_VerificationResultDataActor*)*dataIt)->GetXtData()->type.Text() + ")").Chars());
					} else if ((*dataIt)->GetType() == EditorLib::E_VerificationResultDataType_Xt)
					{
						//dataItem = new C_VerificationTreeItemXt(((const EditorLib::C_VerificationResultDataXt*)*dataIt)->GetXtData()->id);
						//dataItem->SetLabel("Xt Data");
						continue;
					} else if ((*dataIt)->GetType() == EditorLib::E_VerificationResultDataType_Position)
					{
						dataItem = new C_VerificationTreeItem();
						dataItem->SetLabel("Position");
					} else {
						continue;
					}

					resultItem->AddChild(dataItem);
				}
			}
		}

		groupItem->SetExpanded(true);
	}

	results->SetExpanded(true);
}