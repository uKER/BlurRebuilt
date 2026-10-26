#include "stdafx.h"

#include <sigslot/sigslot.h>

#include <DialogueMan/Components/LocForm/LocForm.h>

#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/ColumnSet.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/Misc/Utils.h>

#include <DialogueMan/GlobalSignals.h>

namespace DMan
{

IMPLEMENT_CLASS(C_LocForm, wxScrolledWindow);

BEGIN_EVENT_TABLE(C_LocForm, wxScrolledWindow)
	EVT_TEXT(ID_NAME, C_LocForm::OnNameChanged)
	EVT_CHOICE(ID_LANGUAGE, C_LocForm::OnLanguageSelected)
	EVT_TEXT(ID_LINE, C_LocForm::OnLineChanged)
	EVT_TEXT(ID_COMMENT, C_LocForm::OnCommentChanged)
END_EVENT_TABLE()

C_LocForm::C_LocForm(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
:
wxScrolledWindow(parent, id, pos, size, style),
m_pCurrRow(NULL),
m_pCurrStringBlock(NULL),
m_slots(this)
{
	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* pGridSizer = new wxFlexGridSizer(2, 5, 5);

	m_pNameLabel = new wxStaticText(this, wxID_ANY, _T("Name:"));
	pGridSizer->Add(m_pNameLabel,
                   wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));

	m_pNameTxtCtrl = new wxTextCtrl(this, 
		ID_NAME, wxEmptyString, wxDefaultPosition, wxSize(400, 20));
	m_pNameTxtCtrl->Disable();

    pGridSizer->Add(m_pNameTxtCtrl,
                   wxSizerFlags(1).Align(wxGROW | wxALIGN_CENTER_VERTICAL));

	m_pLanLabel = new wxStaticText(this, wxID_ANY, _T("Language:"));
	pGridSizer->Add(m_pLanLabel,
                   wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));

	m_pLanChoice = new wxChoice(this, ID_LANGUAGE, wxDefaultPosition, wxSize(200, 20));

    pGridSizer->Add(m_pLanChoice,
                   wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));
    
	m_pLineLabel = new wxStaticText(this, wxID_ANY, _T("Text:"));
	pGridSizer->Add(m_pLineLabel,
                   wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_TOP));

	m_pLineTxtCtrl = new wxTextCtrl(this, 
		ID_LINE, wxEmptyString, wxDefaultPosition, wxSize(400, 120), wxTE_MULTILINE);
    
	pGridSizer->Add(m_pLineTxtCtrl,
                   wxSizerFlags().Align(wxGROW | wxALIGN_CENTER_VERTICAL));

	m_pCommentLabel = new wxStaticText(this, wxID_ANY, _T("Comment:"));
	pGridSizer->Add(m_pCommentLabel,
                   wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_TOP));

	m_pCommentTxtCtrl = new wxTextCtrl(this, 
		ID_COMMENT, wxEmptyString, wxDefaultPosition, wxSize(400, 60), wxTE_MULTILINE);
    
	pGridSizer->Add(m_pCommentTxtCtrl,
                   wxSizerFlags().Align(wxGROW | wxALIGN_CENTER_VERTICAL));

	pSizer0->Add( pGridSizer, 0, wxALL|wxEXPAND, 8 );

	SetSizer(pSizer0);
	Layout();
	SetScrollbars(20, 20, 50, 50);
	EnableScrolling(true, true);
	SetBackgroundColour(*wxWHITE);
	HideAll();

	Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_LocForm::OnIdle));

	m_flags.dirty = false;
	m_slots.Connect();
}

void C_LocForm::Shutdown()
{
	m_slots.Disconnect();
}

void C_LocForm::OnSelectionChanged()
{
	m_flags.dirty = true;
}

void C_LocForm::OnIdle(wxIdleEvent& e)
{
	if (!m_flags.dirty)
		return;

	m_flags.dirty = false;
	
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	const C_Selection::ItemArray& arr = selection.GetItems();
	if (arr.Size() == 0)
	{
		HideAll();
		return;
	}

	const C_Selection::C_Item& item = arr.Back();
	
	ClearAll();
	if (selection.GetMode() == C_Selection::E_Mode_Row)
	{		
		EnableAll();

		C_Record* pRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

		if (item.components.Size() > 0)
		{
			U32 rowIndex = item.components.Back();

			C_RowSet* pRowSet = pRecord->GetRowSet(item.id.GetChildId());
			if(SetCurrentRow(pRowSet, rowIndex))
			{
				ReadValues();
				ShowAll();
			}
			else
			{
				HideAll();
			}
			return;
		}
	}
	else
	{
		HideAll();
	}	
}

void C_LocForm::OnNameChanged(wxCommandEvent& event)
{
	if(!m_pCurrStringBlock || !m_pCurrRow)
		return;

	wxString newName = event.GetString();
	Xt::Set(m_pCurrStringBlock, "name", newName.c_str());
	m_pCurrRow->SetDirty();
}

void C_LocForm::OnLanguageSelected(wxCommandEvent& event)
{
	if(!m_pCurrStringBlock)
		return;

	int newLan = m_pLanChoice->GetSelection();
	for(XtUt::List<Xt::DomLanguage>::Iterator it(&wxGetApp().GetDb()->languages); it; ++it)
	{
		if((int)it.index == newLan)
		{
			XtString line = Xt::AsText(Xt::At(m_pCurrStringBlock, it->name));
			m_pLineTxtCtrl->ChangeValue(line.Text());
			break;
		}
	}
}

void C_LocForm::OnLineChanged(wxCommandEvent& event)
{
	if(!m_pCurrStringBlock || !m_pCurrRow)
		return;

	wxString newLine = event.GetString();
	int currLan = m_pLanChoice->GetSelection();
	Xt::Set(m_pCurrStringBlock, m_pLanChoice->GetString(currLan).c_str(), newLine.c_str());
	m_pCurrRow->SetDirty();
}

void C_LocForm::OnCommentChanged(wxCommandEvent& event)
{
	if(!m_pCurrStringBlock || !m_pCurrRow)
		return;

	wxString newComment = event.GetString();
	Xt::Set(m_pCurrStringBlock, "comment", newComment.c_str());
	m_pCurrRow->SetDirty();
}

void C_LocForm::PopulateLanChoice()
{
	m_englishIndex = 0;
	for(XtUt::List<Xt::DomLanguage>::Iterator it(&wxGetApp().GetDb()->languages); it; ++it)
	{
		if (wxString(it->name.Text()) == "english")
			m_englishIndex = (int)it.index;

		m_pLanChoice->Append(it->name.Text());
	}
}

void C_LocForm::ReadValues()
{
	if(!m_pCurrStringBlock)
		return;

	XtString name = Xt::AsText(Xt::At(m_pCurrStringBlock, "name"));
	m_pNameTxtCtrl->SetValue(name.Text());

	if(m_pLanChoice->GetCount() == 0)
		PopulateLanChoice();
	
	m_pLanChoice->Select(m_englishIndex); //reset to english
	
	XtString line = Xt::AsText(Xt::At(m_pCurrStringBlock, "english"));
	m_pLineTxtCtrl->ChangeValue(line.Text());

	XtString comment = Xt::AsText(Xt::At(m_pCurrStringBlock, "comment"));
	m_pCommentTxtCtrl->ChangeValue(comment.Text());
}

void C_LocForm::ClearAll()
{
	m_pCurrStringBlock = NULL;
	m_pCurrRow = NULL;

	m_pNameTxtCtrl->ChangeValue("");
	m_pLineTxtCtrl->ChangeValue("");
	m_pCommentTxtCtrl->ChangeValue("");
}

void C_LocForm::DisableAll()
{
	//m_pNameTxtCtrl->Disable();
	m_pLanChoice->Disable();
	m_pLineTxtCtrl->Disable();
	m_pCommentTxtCtrl->Disable();

	m_pNameLabel->Disable();
	m_pLanLabel->Disable();
	m_pLineLabel->Disable();
	m_pCommentLabel->Disable();
}

void C_LocForm::EnableAll()
{
	//m_pNameTxtCtrl->Enable();
	m_pLanChoice->Enable();
	m_pLineTxtCtrl->Enable();
	m_pCommentTxtCtrl->Enable();

	m_pNameLabel->Enable();
	m_pLanLabel->Enable();
	m_pLineLabel->Enable();
	m_pCommentLabel->Enable();
}

void C_LocForm::ShowAll()
{
	m_pNameTxtCtrl->Show();
	m_pLanChoice->Show();
	m_pLineTxtCtrl->Show();
	m_pCommentTxtCtrl->Show();

	m_pNameLabel->Show();
	m_pLanLabel->Show();
	m_pLineLabel->Show();
	m_pCommentLabel->Show();
}

void C_LocForm::HideAll()
{
	m_pNameTxtCtrl->Hide();
	m_pLanChoice->Hide();
	m_pLineTxtCtrl->Hide();
	m_pCommentTxtCtrl->Hide();

	m_pNameLabel->Hide();
	m_pLanLabel->Hide();
	m_pLineLabel->Hide();
	m_pCommentLabel->Hide();
}

bool C_LocForm::SetCurrentRow(C_RowSet* pRowSet, int rowIndex)
{
	if (pRowSet)
	{
		m_pCurrRow = pRowSet->GetRow(rowIndex);

		if(!m_pCurrRow)
			return false;
		
		LarvaType* pType = m_pCurrRow->GetXtType().AsType();
		if(!pType)
			return false;

		Xt::DomComponent* pRowComponent = Xt::AsComponent(m_pCurrRow->GetXtData());
		Xt::DomBlock* pRowBlock = &pRowComponent->data;

		m_pCurrStringBlock = FindLocEntry(pRowBlock);
		
		if(m_pCurrRow && m_pCurrStringBlock)
			return true;
		else
			return false;

	}
	return false;
}


C_LocForm::C_Slots::C_Slots(C_LocForm* pLocForm)
:
m_pLocForm(pLocForm)
{
}
	
void C_LocForm::C_Slots::Connect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.appSelectionChanged.connect(this, &C_LocForm::C_Slots::OnSelectionChangedSlot);
}
		
void C_LocForm::C_Slots::Disconnect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.appSelectionChanged.disconnect(this);
}

void C_LocForm::C_Slots::OnSelectionChangedSlot()
{
	m_pLocForm->OnSelectionChanged();
}

}