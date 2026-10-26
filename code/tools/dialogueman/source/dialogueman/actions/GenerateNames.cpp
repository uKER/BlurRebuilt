#include "stdafx.h"

#include <wx/regex.h>

#include <DialogueMan/Actions/GenerateNames.h>
#include <DialogueMan/Actions/NameHelpers.h>

#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>

#include <DialogueMan/App.h>


namespace DMan
{

namespace GenerateNames
{

BEGIN_EVENT_TABLE(C_OptionsDlg, wxDialog)
	EVT_BUTTON(wxID_OK, C_OptionsDlg::OnOK)
END_EVENT_TABLE()

wxString C_OptionsDlg::defaultRowNameFormat = "";
wxString C_OptionsDlg::defaultWavNameFormat= "";

C_OptionsDlg::C_OptionsDlg(wxWindow *parent, C_Options& optionsToFill) :
	wxDialog(
		parent,
		wxID_ANY,
		wxString(wxT("Generate Names Options"))),
	m_optionsToFill(optionsToFill)
{	
    wxBoxSizer* pMainSizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* pGridSizer = new wxFlexGridSizer(2, 5, 5);

	pGridSizer->Add(new wxStaticText(this, wxID_ANY, _T("Row Name Format: ")), 
		wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));

	if(defaultRowNameFormat == "")
		defaultRowNameFormat = optionsToFill.GetRowNameFormat();
	
	m_pRowNameFormatTxtCtrl = new wxTextCtrl(this, 
		wxID_ANY, defaultRowNameFormat, wxDefaultPosition, wxSize(400, 20));

    pGridSizer->Add(m_pRowNameFormatTxtCtrl,
                   wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));

	pGridSizer->Add(new wxStaticText(this, wxID_ANY, _T("Wav Name Format: ")), 
		wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));

	if(defaultWavNameFormat == "")
		defaultWavNameFormat = optionsToFill.GetWavNameFormat();

	m_pWavNameFormatTxtCtrl = new wxTextCtrl(this, 
		wxID_ANY, defaultWavNameFormat, wxDefaultPosition, wxSize(400, 20));

	pGridSizer->Add(m_pWavNameFormatTxtCtrl, 
                   wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));

	pMainSizer->Add(pGridSizer, 0, wxALL, 8);

	wxBoxSizer* pButtonsSizer = new wxBoxSizer(wxHORIZONTAL);

	pButtonsSizer->Add(new wxButton(this, wxID_OK, wxT("Ok")),
					   0,
					   wxRIGHT | wxALIGN_RIGHT,
					   2);

	pButtonsSizer->Add(new wxButton(this, wxID_CANCEL, wxT("Cancel")),
					   0,
					   wxALIGN_RIGHT,
					   0);

	pMainSizer->Add( pButtonsSizer, 0, wxALL | wxALIGN_RIGHT, 8 );
	
	pMainSizer->SetSizeHints(this);
    pMainSizer->Fit(this);

    SetSizer(pMainSizer);
	Layout();
}

void C_OptionsDlg::OnOK(wxCommandEvent& event)
{
	m_optionsToFill.SetRowNameFormat(m_pRowNameFormatTxtCtrl->GetValue());
	m_optionsToFill.SetWavNameFormat(m_pWavNameFormatTxtCtrl->GetValue());

	defaultRowNameFormat = m_pRowNameFormatTxtCtrl->GetValue();
	defaultWavNameFormat = m_pWavNameFormatTxtCtrl->GetValue();

	event.Skip();

	return;
}

bool Do(const C_Options& options)
{
	LOGINFO(wxGetApp().GetLogger(), ">>> STARTING TO GENERATE NAMES FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());

	C_RowSet* pRowSet = options.GetRecord()->GetRowSet(0);
	if (!pRowSet)
		return false;

	NameHelpers::ConversationGroups groups;
	NameHelpers::ConversationGroupMapping groupMapping;		
	NameHelpers::BuildConversationGroups(groups, groupMapping, pRowSet);
	
	if (!NameHelpers::GenerateNames(
		options.GetRecord(),
		pRowSet, 
		groups,
		groupMapping,
		options.GetRowNameFormat()))
		return false;

	if (!NameHelpers::GenerateWavNames(
		options.GetRecord(),
		pRowSet, 
		groups,
		groupMapping,
		options.GetWavNameFormat()))
		return false;

	LOGINFO(wxGetApp().GetLogger(), "<<< DONE GENERATING NAMES FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());

	return true;
}

} //namespace GenerateNames

} //namespace DMan