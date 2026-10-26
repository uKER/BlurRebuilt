#include "stdafx.h"

#include <wx/treectrl.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/manager.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/App.h>
#include <EditorApp/Components/PropertyDialog.h>

#include <EditorApp/Properties/EditorProperties.h>

namespace Editor
{

void C_PropertyDialog::OnShowDialog()
{
	C_PropertyDialog dialog(
		wxGetApp().GetFrame(), 
		wxID_ANY,
		wxString("Preferences"));

	dialog.AppendPropertyGroup(new C_EditorPropertyGroup());
	dialog.ShowModal();
}

C_PropertyDialog::C_PropertyDialog(
	wxWindow* pParent, 
	wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style)
	: 
	wxDialog(pParent, id, title, pos, size, style)
{
	SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* m_pMainBoxSizer;
	m_pMainBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* m_pTopBoxSizer;
	m_pTopBoxSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* m_pTreeBoxSizer;
	m_pTreeBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	m_pTreeCtrl = new wxTreeCtrl( 
		this, 
		wxID_ANY, 
		wxDefaultPosition, 
		wxDefaultSize, 
		wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT );
	m_pTreeBoxSizer->Add( m_pTreeCtrl, 1, wxALL|wxEXPAND, 5 );
	
	m_pTopBoxSizer->Add( m_pTreeBoxSizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* m_pPropBoxSizer;
	m_pPropBoxSizer = new wxBoxSizer( wxVERTICAL );

	m_pPropGridManager = new wxPropertyGridManager(
		this, 
		wxID_ANY,
		wxDefaultPosition, 
		wxDefaultSize, 
		wxNO_BORDER|wxPG_SPLITTER_AUTO_CENTER);
	const wxColour lightgray(212,208,200);
	const wxColour darkgray(113,111,100);

	m_pPropGridManager->GetGrid()->SetWindowStyle(wxNO_BORDER);
	m_pPropGridManager->GetGrid()->SetMarginColour(*wxWHITE);
	m_pPropGridManager->GetGrid()->SetCaptionBackgroundColour(*wxWHITE);
	m_pPropGridManager->GetGrid()->SetCellBackgroundColour(*wxWHITE);
	m_pPropGridManager->GetGrid()->SetCellTextColour(*wxBLACK);
	m_pPropGridManager->GetGrid()->SetCellDisabledTextColour(lightgray);
	m_pPropGridManager->GetGrid()->SetLineColour(lightgray);

	m_pPropBoxSizer->Add( m_pPropGridManager, 3, wxALL|wxEXPAND, 5 );
	
	wxArrayString m_pHelpTextChoices;
	m_pHelpText = new wxStaticText(
		this, 
		wxID_ANY,
		"");
	m_pPropBoxSizer->Add( m_pHelpText, 0, wxALL|wxEXPAND, 5 );
	
	m_pTopBoxSizer->Add( m_pPropBoxSizer, 2, wxEXPAND, 5 );
	
	m_pMainBoxSizer->Add( m_pTopBoxSizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* m_pBottomBoxSizer;
	m_pBottomBoxSizer = new wxBoxSizer( wxHORIZONTAL );
	
	m_pButtonOK = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pBottomBoxSizer->Add( m_pButtonOK, 0, wxALIGN_BOTTOM|wxALL, 5 );
	
	m_pButtonCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pBottomBoxSizer->Add( m_pButtonCancel, 0, wxALIGN_BOTTOM|wxALL, 5 );

	m_pMainBoxSizer->Add( m_pBottomBoxSizer, 0, wxALIGN_RIGHT, 5 );
	
	m_pPropGridManager->GetGrid()->ClearInternalFlag(wxPG_FL_SPLITTER_PRE_SET);

	//PopulateTree();

	SetSizer( m_pMainBoxSizer );
	Layout();

	Connect(
		this->GetId(),
		wxEVT_SHOW,
		wxShowEventHandler(C_PropertyDialog::OnShow));
	//Connect tree events.
	Connect(
		wxID_ANY, 
		wxEVT_COMMAND_TREE_SEL_CHANGED, 
		wxTreeEventHandler(C_PropertyDialog::OnNodeSelect));

	//Connect property events.
	Connect(
		wxID_ANY, 
		wxID_ANY, 
		wxEVT_PG_CHANGED, 
		wxObjectEventFunction(&C_PropertyDialog::OnPropertyChanged), 
		0, 
		this);
	Connect(
		wxID_ANY, 
		wxID_ANY, 
		wxEVT_PG_SELECTED, 
		wxObjectEventFunction(&C_PropertyDialog::OnPropertySelected),
		0, 
		this);

	//Connect button events.
	Connect(
		m_pButtonOK->GetId(), 
		wxEVT_COMMAND_BUTTON_CLICKED, 
		wxCommandEventHandler(C_PropertyDialog::OnButtonClick));
	Connect(
		m_pButtonCancel->GetId(), 
		wxEVT_COMMAND_BUTTON_CLICKED, 
		wxCommandEventHandler(C_PropertyDialog::OnButtonClick));
}

C_PropertyDialog::~C_PropertyDialog()
{
	for (S32 i=0; i<m_pPropertyGroups.Size(); ++i)
		delete m_pPropertyGroups[i];
}

void C_PropertyDialog::AppendPropertyGroup(I_PropertyGroup *pPropGroup)
{
	pPropGroup->Initialise(m_pPropGridManager);
	m_pPropertyGroups.PushBack(pPropGroup);
}

void C_PropertyDialog::PopulateTree()
{
	m_pTreeCtrl->Freeze();
	m_pTreeCtrl->DeleteAllItems();
	wxTreeItemId rootId = m_pTreeCtrl->AddRoot(wxT("Preferences"));
	for (U32 i = 0; i < m_pPropGridManager->GetPageCount(); i++)
	{
		wxPropertyGridPage* pPage = m_pPropGridManager->GetPage(i);
		assert(pPage);
		m_pTreeCtrl->AppendItem(rootId, pPage->GetLabel().c_str());
	}
	m_pTreeCtrl->ExpandAll();
	m_pTreeCtrl->Thaw();
}

void C_PropertyDialog::OnShow(wxShowEvent &e)
{
	if (e.GetShow())
	{
		PopulateTree();

		//Select the first page.
		{
			wxTreeItemId root = m_pTreeCtrl->GetRootItem();
			if (!root.IsOk())
				return;

			wxTreeItemIdValue cookie;
			wxTreeItemId child = m_pTreeCtrl->GetFirstChild(root, cookie);
			if (!child.IsOk())
				return;
		
			m_pTreeCtrl->SelectItem(child);
			m_pPropGridManager->Refresh();
		}
	}
}

void C_PropertyDialog::OnNodeSelect(wxTreeEvent& e)
{
	wxTreeItemId node = e.GetItem();
	if (!node.IsOk())//|| m_pTreeCtrl->GetRootItem() == node)
		return;

	wxString txt = m_pTreeCtrl->GetItemText(node);
	wxPropertyGridPage* pPage = m_pPropGridManager->GetPage(txt.c_str());
	if (pPage)
		m_pPropGridManager->SelectPage(pPage);
	else
		m_pPropGridManager->SelectPage(-1);

}

void C_PropertyDialog::OnPropertyChanged(wxPropertyGridEvent& e)
{
	//send event to all property groups...
	//it should be up to them to check if the changed property belongs affects them
	for (S32 i=0; i<m_pPropertyGroups.Size(); ++i)
		m_pPropertyGroups[i]->OnPropertyChanged(e);
}

void C_PropertyDialog::OnPropertySelected(wxPropertyGridEvent& e)
{
	//Grab the name of the selected tree node and change the current
	//property page accordingly.
	wxPGId propId = m_pPropGridManager->GetSelectedProperty();
	if (!propId)
	{
		m_pHelpText->SetLabel("");
		return;
	}

	wxPGProperty* pProp = m_pPropGridManager->GetPropertyPtr(propId);
	m_pHelpText->SetLabel(pProp->GetHelpString().c_str());
}

void C_PropertyDialog::OnUpdateUI(wxUpdateUIEvent& e)
{
}

void C_PropertyDialog::OnButtonClick(wxCommandEvent& e)
{
	if (e.GetId() == m_pButtonOK->GetId())
	{
		for (S32 i=0; i<m_pPropertyGroups.Size(); ++i)
			m_pPropertyGroups[i]->Apply();
	}
	else
	{
		for (S32 i=0; i<m_pPropertyGroups.Size(); ++i)
			m_pPropertyGroups[i]->Revert();
	}

	Close();
}

}
