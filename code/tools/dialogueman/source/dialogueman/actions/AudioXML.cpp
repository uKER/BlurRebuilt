#include "stdafx.h"

#include <DialogueMan/Actions/AudioXML.h>
#include <DialogueMan/Actions/GenerateSplits.h>
#include <DialogueMan/Actions/Nightwing/NightwingAudioTemplates.h>
#include <DialogueMan/Actions/Amax/AmaxAudioTemplates.h>
#include <Dialogueman/Actions/NameHelpers.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <Dialogueman/Misc/StringUtil.h>
#include <Dialogueman/Misc/Utils.h>

#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/DbManager.h>

#include <DialogueMan/App.h>

#include <System/Misc/StringUtil.h>
#include <tools_shared/utility/MacroExpander.h>

namespace DMan
{
namespace AudioXML
{

/////////////////////////// C_OptionsDlg \\\\\\\\\\\\\\\\\\\\\\\\\\\

BEGIN_EVENT_TABLE(C_OptionsDlg, wxDialog)
	EVT_BUTTON(wxID_OK, C_OptionsDlg::OnOK)
END_EVENT_TABLE()

C_OptionsDlg::C_OptionsDlg(wxWindow *parent, C_Options& optionsToFill) :
	wxDialog(
		parent,
		wxID_ANY,
		wxString(wxT("Audio XML Options"))),
	m_optionsToFill(optionsToFill)
{	
    wxBoxSizer* pMainSizer = new wxBoxSizer(wxVERTICAL);

	pMainSizer->Add(new wxStaticText(this, wxID_ANY, 
		_T("Please specify the names for the following files:")), 
		0, wxUP | wxRIGHT | wxLEFT | wxEXPAND, 8);

	pMainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 8);

	wxFlexGridSizer* pGridSizer = new wxFlexGridSizer(2, 5, 5);

	//Audio XML Path	
	pGridSizer->Add(new wxStaticText(this, wxID_ANY, _T("Audio XML: ")), 
		wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));

	wxString defaultAudioXmlPath = optionsToFill.GetAudioXMLPath();

	m_pAudioXmlPathTxtCtrl = new wxTextCtrl(this, 
		wxID_ANY, defaultAudioXmlPath, wxDefaultPosition, wxSize(407, 20));

	pGridSizer->Add(m_pAudioXmlPathTxtCtrl,
                   wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));

	//Bank PC Path	
	pGridSizer->Add(new wxStaticText(this, wxID_ANY, _T("Bank PC: ")), 
		wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));

	wxString defaultBankPCPath = optionsToFill.GetBankPCPath();

	m_pBankPCPathTxtCtrl = new wxTextCtrl(this, 
		wxID_ANY, defaultBankPCPath, wxDefaultPosition, wxSize(407, 20));

	pGridSizer->Add(m_pBankPCPathTxtCtrl,
                   wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));

	//Bank 360 Path	
	pGridSizer->Add(new wxStaticText(this, wxID_ANY, _T("Bank 360: ")), 
		wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));

	wxString defaultBank360Path = optionsToFill.GetBank360Path();

	m_pBank360PathTxtCtrl = new wxTextCtrl(this, 
		wxID_ANY, defaultBank360Path, wxDefaultPosition, wxSize(407, 20));

	pGridSizer->Add(m_pBank360PathTxtCtrl,
                   wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));

	//Bank PS3 Path	
	pGridSizer->Add(new wxStaticText(this, wxID_ANY, _T("Bank PS3: ")), 
		wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));

	wxString defaultBankPS3Path = optionsToFill.GetBankPS3Path();

	m_pBankPS3PathTxtCtrl = new wxTextCtrl(this, 
		wxID_ANY, defaultBankPS3Path, wxDefaultPosition, wxSize(407, 20));

	pGridSizer->Add(m_pBankPS3PathTxtCtrl,
                   wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));

	//Build 360 Path	
	pGridSizer->Add(new wxStaticText(this, wxID_ANY, _T("Build 360: ")), 
		wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));

	wxString defaultBuild360Path = optionsToFill.GetBuild360Path();

	m_pBuild360PathTxtCtrl = new wxTextCtrl(this, 
		wxID_ANY, defaultBuild360Path, wxDefaultPosition, wxSize(407, 20));

	pGridSizer->Add(m_pBuild360PathTxtCtrl,
                   wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));

	pMainSizer->Add(pGridSizer, 0, wxALL | wxEXPAND, 8);

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
	m_optionsToFill.SetAudioXMLPath(m_pAudioXmlPathTxtCtrl->GetValue());
	m_optionsToFill.SetBankPCPath(m_pBankPCPathTxtCtrl->GetValue());
	m_optionsToFill.SetBank360Path(m_pBank360PathTxtCtrl->GetValue());
	m_optionsToFill.SetBankPS3Path(m_pBankPS3PathTxtCtrl->GetValue());
	m_optionsToFill.SetBuild360Path(m_pBuild360PathTxtCtrl->GetValue());
		
	if(IsPathOk(m_optionsToFill.GetAudioXMLPath()))
	{
		event.Skip();
	}
	else
	{
		wxMessageDialog msg(NULL, 
				wxT("Audio XML Path is incorrect.\n"), 
				wxT("Audio XML Options"),
				wxOK | wxICON_ERROR);
		msg.ShowModal();
		event.StopPropagation();
	}

	if(IsPathOk(m_optionsToFill.GetBankPCPath()))
	{
		event.Skip();
	}
	else
	{
		wxMessageDialog msg(NULL, 
				wxT("Bank PC Path is incorrect.\n"), 
				wxT("Audio XML Options"),
				wxOK | wxICON_ERROR);
		msg.ShowModal();
		event.StopPropagation();
	}

	if(IsPathOk(m_optionsToFill.GetBank360Path()))
	{
		event.Skip();
	}
	else
	{
		wxMessageDialog msg(NULL, 
				wxT("Bank 360 Path is incorrect.\n"), 
				wxT("Audio XML Options"),
				wxOK | wxICON_ERROR);
		msg.ShowModal();
		event.StopPropagation();
	}

	if(IsPathOk(m_optionsToFill.GetBankPS3Path()))
	{
		event.Skip();
	}
	else
	{
		wxMessageDialog msg(NULL, 
				wxT("Bank PS3 Path is incorrect.\n"), 
				wxT("Audio XML Options"),
				wxOK | wxICON_ERROR);
		msg.ShowModal();
		event.StopPropagation();
	}

	if(IsPathOk(m_optionsToFill.GetBuild360Path()))
	{
		event.Skip();
	}
	else
	{
		wxMessageDialog msg(NULL, 
				wxT("Build 360 Path is incorrect.\n"), 
				wxT("Audio XML Options"),
				wxOK | wxICON_ERROR);
		msg.ShowModal();
		event.StopPropagation();
	}
}

/////////////////////////// AudioXML::Do \\\\\\\\\\\\\\\\\\\\\\\\\\\

bool Do(const C_Options& options)
{
	LOGINFO(wxGetApp().GetLogger(), ">>> STARTING TO GENERATE AUDIO XML FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());

	C_RowSet* pRowSet = options.GetRecord()->GetRowSet(0);
	if (!pRowSet)
		return false;

	Util::C_String loadedProject = "%BIZARRE_PROJECT%";
	Tools::C_MacroExpander::Instance().Expand(loadedProject);

	bool ok = true;
	if(loadedProject == "Amax")
	{
		Xt::DomElement* pElement = pRowSet->GetXtData();
		Xt::DomArray* pArray = Xt::AsArray(pElement);

		//Obtain Split Groups
		GenerateSplits::C_Options splitsOptions;
		splitsOptions.SetRecord(options.GetRecord());
		GenerateSplits::Do(splitsOptions);

		//Generate Audio XML
		int splitGroup = 0;
		Xt::DomArray::IndexIterator begin(pArray);
		Xt::DomArray::IndexIterator end(pArray);

		bool ok = true;

		while(end)
		{
			GenerateSplits::FindEnd(begin, end);
			 			
			Amax::AudioXML audioXML(options.GetAudioXMLPath());
			Amax::BankPC bankPC(options.GetBankPCPath());
			Amax::Bank360 bank360(options.GetBank360Path());
			Amax::BankPS3 bankPS3(options.GetBankPS3Path());

			ok = audioXML.Write(options.GetRecord(), splitGroup, begin, end);
			ok = bankPC.Write(options.GetRecord(), splitGroup, begin, end);
			ok = bank360.Write(options.GetRecord(), splitGroup, begin, end);
			ok = bankPS3.Write(options.GetRecord(), splitGroup, begin, end);

			Amax::SetupAudioRecords setupAudioRecords;
			setupAudioRecords.Do(options.GetRecord(), splitGroup);
			
			splitGroup++;
			begin = end;
		}

		Amax::Generate360Bat generate360Bat(options.GetBuild360Path(), options.GetBank360Path(), options.GetAudioXMLPath());
		generate360Bat.Do(options.GetRecord(), splitGroup);

	}
	else if(loadedProject == "Nightwing")
	{
		Nightwing::AudioXML audioXML(options.GetAudioXMLPath());
		ok = audioXML.Write(options.GetRecord());
	}
	
	LOGINFO(wxGetApp().GetLogger(), "<<< DONE GENERATING AUDIO XML FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());
	
	return ok;
}

} //namespace AudioXML
} //namespace DMan
