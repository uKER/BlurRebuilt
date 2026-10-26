#include "stdafx.h"

#include <DialogueMan/Actions/BatchGenerate.h>
#include <DialogueMan/Actions/GenerateNames.h>
#include <DialogueMan/Actions/GenerateHashes.h>
#include <DialogueMan/Actions/GenerateWavs.h>
#include <DialogueMan/Actions/AudioXML.h>

#include <DialogueMan/Document/DbManager.h>

namespace DMan
{
namespace BatchGenerate
{

/////////////////////////// C_Options \\\\\\\\\\\\\\\\\\\\\\\\\\\

void C_Options::SetRecords(const Btl::Vector<Xt::DomRecord*>& records)
{
	m_records.Clear();
	for(int i = 0; i < records.Size(); i++)
	{
		m_records.PushBack(records[i]);
	}
}

/////////////////////////// C_OptionsDlg \\\\\\\\\\\\\\\\\\\\\\\\\\\

BEGIN_EVENT_TABLE(C_OptionsDlg, wxDialog)
	EVT_BUTTON(wxID_OK, C_OptionsDlg::OnOK)
END_EVENT_TABLE()

C_OptionsDlg::C_OptionsDlg(wxWindow *parent, C_Options& optionsToFill) :
	wxDialog(
		parent,
		wxID_ANY,
		wxString(wxT("Batch Generate"))),
	m_optionsToFill(optionsToFill)
{	
    wxBoxSizer* pMainSizer = new wxBoxSizer(wxVERTICAL);

	m_generateNamesChkBox	 = new wxCheckBox(this, wxID_ANY, "Generate Names");
	m_generateHashesChkBox	 = new wxCheckBox(this, wxID_ANY, "Generate Hashes");
	m_generateTTSChkBox		 = new wxCheckBox(this, wxID_ANY, "Generate Text-To-Speech");
	m_generateAudioXMLChkBox = new wxCheckBox(this, wxID_ANY, "Generate Audio XML");

	m_generateNamesChkBox->SetValue(optionsToFill.GenerateNames());
	m_generateHashesChkBox->SetValue(optionsToFill.GenerateHashes());
	m_generateTTSChkBox->SetValue(optionsToFill.GenerateTTS());
	m_generateAudioXMLChkBox->SetValue(optionsToFill.GenerateAudioXML());

	pMainSizer->Add(m_generateNamesChkBox,	  0, wxUP | wxRIGHT | wxLEFT | wxEXPAND, 8);
	pMainSizer->Add(m_generateHashesChkBox,	  0, wxUP | wxRIGHT | wxLEFT | wxEXPAND, 8);
	pMainSizer->Add(m_generateTTSChkBox,	  0, wxUP | wxRIGHT | wxLEFT | wxEXPAND, 8);
	pMainSizer->Add(m_generateAudioXMLChkBox, 0, wxUP | wxRIGHT | wxLEFT | wxEXPAND, 8);

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
	m_optionsToFill.SetGenerateNames(m_generateNamesChkBox->GetValue());
	m_optionsToFill.SetGenerateHashes(m_generateHashesChkBox->GetValue());
	m_optionsToFill.SetGenerateTTS(m_generateTTSChkBox->GetValue());
	m_optionsToFill.SetGenerateAudioXML(m_generateAudioXMLChkBox->GetValue());

	event.Skip();
}

/////////////////////////// BatchGenerate::Do \\\\\\\\\\\\\\\\\\\\\\\\\\\

bool Do(const C_Options& batchOptions)
{
	C_Options& _batchOptions = const_cast<C_Options&>(batchOptions);

	Btl::Vector<Xt::DomRecord*>& records = _batchOptions.GetRecords();

	for(int i = 0; i < records.Size(); i++)
	{
		C_Record* pRecord = wxGetApp().GetDbManager().GetRecord(records[i]->id);
		
		if(_batchOptions.GenerateNames())
		{
			GenerateNames::C_Options options;
			options.SetRecord(pRecord);
			GenerateNames::Do(options);
		}
		if(_batchOptions.GenerateHashes())
		{
			GenerateHashes::C_Options options;
			options.SetRecord(pRecord);
			GenerateHashes::Do(options);
		}
		if(_batchOptions.GenerateTTS())
		{
			GenerateWavs::C_Options options;
			options.SetRecord(pRecord);
			GenerateWavs::Do(options);
		}
		if(_batchOptions.GenerateAudioXML())
		{
			AudioXML::C_Options options;
			options.SetRecord(pRecord);
			AudioXML::Do(options);
		}
	}
	return true;
}

} //namespace BatchGenerate
} //namespace DMan
