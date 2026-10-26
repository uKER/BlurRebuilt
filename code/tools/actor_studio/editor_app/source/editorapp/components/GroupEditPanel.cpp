#include "stdafx.h"

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Core.h>

#include <EditorApp/Components/Scene_Graph/SelectionSceneGraphPanel.h>
#include <EditorApp/Components/GroupEditPanel.h>
#include <EditorApp/Components/SplitterControl.h>

using namespace Editor;

C_GroupEditPanel::C_GroupEditPanel(wxWindow *parent, wxWindowID winid, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
: wxPanel(parent, winid, pos, size, style, name)
{
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainSizer);

	m_SplitterCtrl = new wxSplitterCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_NOBORDER);

	wxPanel* leftPanel = new wxPanel(this);
	wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
	leftPanel->SetSizer(leftSizer);
	m_LeftPanel = new C_SelectionSceneGraphPanel(leftPanel);
	m_LeftButton = new wxButton(leftPanel, wxID_ANY, ">>>");
	leftSizer->Add(m_LeftPanel, 1, wxEXPAND, 2);
	leftSizer->Add(m_LeftButton, 0, wxALIGN_RIGHT);
	leftPanel->Layout();

	wxPanel* rightPanel = new wxPanel(this);
	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	rightPanel->SetSizer(rightSizer);
	m_RightPanel = new C_SelectionSceneGraphPanel(rightPanel);
	m_RightButton = new wxButton(rightPanel, wxID_ANY, "<<<");
	rightSizer->Add(m_RightPanel, 1, wxEXPAND, 2);
	rightSizer->Add(m_RightButton, 0, wxALIGN_LEFT);
	rightPanel->Layout();

	m_SplitterCtrl->Split(E_SplitterLayout_2Vertical, leftPanel, rightPanel);
	m_SplitterCtrl->SetSplitterProportion(5000);

	mainSizer->Add(m_SplitterCtrl, 1, wxEXPAND, 5);

	m_LeftButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_GroupEditPanel::OnButton), NULL, this);
	m_RightButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_GroupEditPanel::OnButton), NULL, this);

	Layout();
}

C_GroupEditPanel::~C_GroupEditPanel()
{
	SAFE_DELETE_PTR(m_RightPanel);
	SAFE_DELETE_PTR(m_LeftPanel);
}

void C_GroupEditPanel::Shutdown()
{
	m_LeftButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_GroupEditPanel::OnButton), NULL, this);
	m_RightButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_GroupEditPanel::OnButton), NULL, this);

	m_RightPanel->Shutdown();
	m_LeftPanel->Shutdown();
}

void C_GroupEditPanel::OnButton(wxCommandEvent& e)
{
	C_SelectionSceneGraphPanel* toPanel = 0;
	C_SelectionSceneGraphPanel* fromPanel = 0;

	if (e.GetId() == m_LeftButton->GetId())
	{
		toPanel = m_RightPanel;
		fromPanel = m_LeftPanel;
	}
	else if (e.GetId() == m_RightButton->GetId())
	{
		toPanel = m_LeftPanel;
		fromPanel = m_RightPanel;
	}
	else
		return;

	const EditorLib::C_Selection toSelection = toPanel->GetSelection();
	const EditorLib::C_Selection fromSelection = fromPanel->GetSelection();
	
	if (toSelection.GetItems().Size() > 1)
		return;

	if (fromSelection.GetItems().Size() < 1)
		return;

	Btl::String fromIdString;

	for (EditorLib::C_Selection::ItemArray::ConstIterator it = fromSelection.GetItems().Begin(); it != fromSelection.GetItems().End(); ++it)
	{
		fromIdString += it->id.AsString() + ",";
	}

	Btl::String toIdString;
	
	if (toSelection.GetItems().Size())
	{
		toIdString = toSelection.GetItems()[0].id.AsString();
	} else {
		EditorLib::I_Level* level = EditorLib::C_Core::Instance().GetDgManager().LevelOf(fromSelection.GetItems()[0].id);
		if (!level)
			return;
		toIdString = level->GetRootSceneNode()->GetId().AsString();
	}

	fromIdString = fromIdString(0, fromIdString.Length()-1);
	EditorLib::C_Core::Instance().GetCommandProcessor().Execute("parent", fromIdString + " -p " + toIdString);
}