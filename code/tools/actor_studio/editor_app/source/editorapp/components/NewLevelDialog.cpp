#include "stdafx.h"
#include <wx/treectrl.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <System/Misc/StringUtil.h>
#include <System/Serialise/I_FileSystem.h>
#include <System/Serialise/C_FileSystem.h>

#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/LevelPack.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

#include <xtwin_wx/Explorer/Explorer.h>

#include <EditorApp/Components/LevelExplorer.h>
#include <EditorApp/Components/NewLevelDialog.h>


namespace Editor
{

class C_NewLevelDialogExplorer : public C_LevelExplorer
{
public:
	C_NewLevelDialogExplorer(C_NewLevelDialog* pDialog);
protected:
	virtual void OnSelectionChanged(const XtUt::Array<TreeItem*>& selection, TcSelectionType selectionType);
private:
	C_NewLevelDialog* m_pDialog;
};

C_NewLevelDialogExplorer::C_NewLevelDialogExplorer(C_NewLevelDialog* pDialog)
{
	m_pDialog = pDialog;
	SetFilterEmpty(false);
}

void C_NewLevelDialogExplorer::OnSelectionChanged(const XtUt::Array<TreeItem*>& selection, TcSelectionType selectionType)
{
	m_pDialog->UpdateUI();
}

C_NewLevelDialog::C_NewLevelDialog(
	wxWindow *parent, 
	wxWindowID id, 
	const wxString& title,
	const wxPoint& pos, 
	const wxSize& size,
	long style)
: 
wxDialog(
	parent, 
	id, 
	title,
	pos, 
	size, 
	style)
{
	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer(wxVERTICAL);
	
	wxBoxSizer* pSizer1;
	pSizer1 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticText *pXtLabel = new wxStaticText(this, wxID_ANY, wxT("Level Name"), wxDefaultPosition, wxDefaultSize, 0);
	pXtLabel->Wrap(-1);
	pSizer1->Add(pXtLabel, 0, wxALL|wxEXPAND, 8);
	
	m_pLevelName = new wxTextCtrl(this, wxID_ANY, "NewLevel", wxDefaultPosition, wxDefaultSize, 0);
	pSizer1->Add( m_pLevelName, 1, wxTOP|wxRIGHT, 5 );
	
	pSizer0->Add(pSizer1, 0, wxEXPAND, 5);

	m_pExplorerPanel = new wxPanel(this);

	m_pXtTreeView = new C_NewLevelDialogExplorer(this);
	m_pXtTreeView->Create((HWND)m_pExplorerPanel->GetHandle());
	m_pXtTreeView->SetDb(EditorLib::C_Core::Instance().GetXtManager().GetDb());

	pSizer0->Add(m_pExplorerPanel, 1, wxALL|wxEXPAND, 5);
	
	wxBoxSizer* pSizer2;
	pSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	m_pButtonOK = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	pSizer2->Add( m_pButtonOK, 0, wxALL, 5 );
	
	m_pButtonCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	pSizer2->Add( m_pButtonCancel, 0, wxALL, 5 );
	
	pSizer0->Add( pSizer2, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	SetSizer(pSizer0);
	Layout();

	WinRect rcClient((HWND)m_pExplorerPanel->GetHandle());
	SetWindowPos(*m_pXtTreeView, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pLevelName->SetFocus();

	Connect(m_pButtonOK->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_NewLevelDialog::OnButtonClick));
	Connect(m_pButtonCancel->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_NewLevelDialog::OnButtonClick));

	m_pExplorerPanel->Connect(wxEVT_SIZE, wxSizeEventHandler(C_NewLevelDialog::OnSize), NULL, this);
}

C_NewLevelDialog::~C_NewLevelDialog()
{
	delete m_pXtTreeView;
}

void C_NewLevelDialog::OnButtonClick(wxCommandEvent& e)
{
	if (e.GetId() == m_pButtonOK->GetId())
	{
		if (m_pXtTreeView->GetSelection().count != 1)
			return;

		Explorer::ExplorerData* selectedItem = (Explorer::ExplorerData*)m_pXtTreeView->GetSelection()[0];

		if (selectedItem->GetType() == kExItemTypePack || selectedItem->GetType() == kExItemTypeFilterRecord)
		{
			const Xt::DomPack *pPack = selectedItem->GetDomPack();

			EditorLib::C_XtLevel* level = 0;

			if (EditorLib::C_Core::Instance().GetLevelManager()->OpenPack(pPack->id))
				level = EditorLib::C_Core::Instance().GetLevelManager()->GetPack(pPack->id)->NewLevel();

			if (level)
			{
				XtString prefix;
	
				if (selectedItem->GetType() == kExItemTypeFilterRecord)
					m_pXtTreeView->GetFilterName(selectedItem, prefix);
				
				Btl::String levelName = prefix.Text();
				levelName += m_pLevelName->GetValue().c_str();
				
				level->SetName(levelName);

				EditorLib::C_Core::Instance().GetLevelManager()->SetActiveLevel(level);

				EndModal(0);
			}
			else
			{
				wxString msg;
				msg << "Error creating new level '" 
					<< selectedItem->GetLabel().Text()
					<< "' (pack " << pPack->id << ")";
				LOGERROR(g_Log, msg.c_str());
				wxMessageDialog dialog(this, msg, "Error", wxOK|wxICON_ERROR);
				dialog.ShowModal();
			
				//Error out.
				EndModal(1);
			}

			return;
		}

		wxMessageDialog dialog(this, "Please select a pack file", "Error", wxOK|wxICON_ERROR);
		dialog.ShowModal();
		return;
	}

	EndModal(0);
	Close();
}

void C_NewLevelDialog::UpdateUI()
{
	if (m_pXtTreeView->GetSelection().count != 1)
			return;

	Explorer::ExplorerData* selectedItem = (Explorer::ExplorerData*)m_pXtTreeView->GetSelection()[0];

	if (selectedItem->GetType() == kExItemTypePack || selectedItem->GetType() == kExItemTypeFilterRecord)
		m_pButtonOK->Enable(true);
	else
		m_pButtonOK->Enable(false);
}

void C_NewLevelDialog::OnSize(wxSizeEvent& e)
{
	WinRect rcClient((HWND)m_pExplorerPanel->GetHandle());
	SetWindowPos(*m_pXtTreeView, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

}
