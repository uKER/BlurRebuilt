#include "stdafx.h"

#include <system/misc/StringUtil.h>
#include <system/serialise/I_FileSystem.h>
#include <system/serialise/C_FileSystem.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>

#include <EditorApp/Components/LevelExplorer.h>
#include <EditorApp/Components/LevelSelectDialog.h>

#include <EditorApp/App.h>

using namespace Editor;

class C_LevelSelectDialogExplorer : public C_LevelExplorer
{
public:
	C_LevelSelectDialogExplorer(C_LevelSelectDialog* pDialog);
protected:
	virtual void OnActivateItem(Explorer::ExplorerData* item);
	virtual void OnSelectionChanged(const XtUt::Array<TreeItem*>& selection, TcSelectionType selectionType);
private:
	C_LevelSelectDialog* m_pDialog;
};

C_LevelSelectDialogExplorer::C_LevelSelectDialogExplorer(C_LevelSelectDialog* pDialog)
{
	m_pDialog = pDialog;
	SetFilterEmpty(true);
}

void C_LevelSelectDialogExplorer::OnActivateItem(Explorer::ExplorerData* item)
{
	if (item->GetType() == kExItemTypePack)
		m_pDialog->OpenPack(item->GetDomPack());
	else if (item->GetType() == kExItemTypeRecord)
		m_pDialog->OpenRecord(item->GetDomRecord());
}

void C_LevelSelectDialogExplorer::OnSelectionChanged(const XtUt::Array<TreeItem*>& selection, TcSelectionType selectionType)
{
	m_pDialog->UpdateUI();
}

C_LevelSelectDialog::C_LevelSelectDialog(
	wxWindow* pParent, 
	wxWindowID id, 
	const wxString& title, 
	const wxPoint& pos, 
	const wxSize& size, 
	long style)
: wxDialog(
	pParent,
	id, 
	title, 
	pos,
	size, 
	style)
{
	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer(wxVERTICAL);
	
	m_pExplorerPanel = new wxPanel(this);

	m_pXtTreeView = new C_LevelSelectDialogExplorer(this);
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

	Connect(m_pButtonOK->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_LevelSelectDialog::OnButtonClick));
	Connect(m_pButtonCancel->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_LevelSelectDialog::OnButtonClick));

	m_pExplorerPanel->Connect(wxEVT_SIZE, wxSizeEventHandler(C_LevelSelectDialog::OnSize), NULL, this);
}

C_LevelSelectDialog::~C_LevelSelectDialog()
{
	delete m_pXtTreeView;
}

void C_LevelSelectDialog::OnSize(wxSizeEvent& e)
{
	WinRect rcClient((HWND)m_pExplorerPanel->GetHandle());
	SetWindowPos(*m_pXtTreeView, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void C_LevelSelectDialog::UpdateUI()
{
	if (m_pXtTreeView->GetSelection().count == 1)
	{
		ExItemType type = ((Explorer::ExplorerData*)m_pXtTreeView->GetSelection()[0])->GetType();
	
		if (type == kExItemTypePack || type == kExItemTypeRecord)
		{
			m_pButtonOK->Enable(true);
			return;
		}
	}

	m_pButtonOK->Enable(false);
}

void C_LevelSelectDialog::OnButtonClick(wxCommandEvent& e)
{
	if (e.GetId() == m_pButtonOK->GetId())
	{
		if (m_pXtTreeView->GetSelection().count != 1)
			return;

		Explorer::ExplorerData* selectedItem = (Explorer::ExplorerData*)m_pXtTreeView->GetSelection()[0];

		if (selectedItem->GetType() == kExItemTypePack)
			OpenPack(selectedItem->GetDomPack());
		else if (selectedItem->GetType() == kExItemTypeRecord)
			OpenRecord(selectedItem->GetDomRecord());
	}
	else
		EndModal(wxID_CANCEL);
}

void C_LevelSelectDialog::OpenPack(Xt::DomPack *pPack)
{
	m_pSelectedItem = pPack;
	EndModal(wxID_OK);
}

void C_LevelSelectDialog::OpenRecord(Xt::DomRecord *pRecord)
{
	m_pSelectedItem = pRecord;
	EndModal(wxID_OK);
}

Xt::DomElement* C_LevelSelectDialog::GetSelectedItem() const
{
	return m_pSelectedItem;
}
