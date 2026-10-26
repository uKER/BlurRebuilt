#include "stdafx.h"

#include <wx/progdlg.h>
#include <wx/regex.h>

#include <System/Misc/StringUtil.h>

#include <DialogueMan/Actions/GenerateWavs.h>
#include <DialogueMan/Actions/TTSWav.h>
#include <DialogueMan/Actions/NameHelpers.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <DialogueMan/Misc/Utils.h>
#include <DialogueMan/Misc/MacroExpander.h>

#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/DbManager.h>

#include <DialogueMan/App.h>

namespace DMan
{

namespace GenerateWavs
{

void C_Options::SetRecord(C_Record* pRecord) 
{ 
	m_pRecord = pRecord;

	if(m_pRecord)
	{
		Btl::Map<Character, Voice>& voices = wxGetApp().GetConfig().tts.voices; 
		Btl::Map<Character, Voice>::Iterator itVoices;

		Btl::OpenHashSet<Character> charactersInRecord = NameHelpers::GetCharactersInRecord(m_pRecord);
		Btl::OpenHashSet<Character>::Iterator charactersIt;

		for(charactersIt = charactersInRecord.Begin(); charactersIt != charactersInRecord.End(); ++charactersIt)
		{
			Util::C_String theCharacter = charactersIt->GetKey();
			Voice voice;
			//set default voice for Characters not specified on config file
			if(voices.Find(theCharacter) == voices.End()) 
				voices[theCharacter.Upper()] = voice;
		}
	}
}

Btl::OpenHashSet<Character> C_Options::GetCharactersInRecord() const 
{ 
	return NameHelpers::GetCharactersInRecord(m_pRecord); 
}

/////////////////////////// C_OptionsDlg \\\\\\\\\\\\\\\\\\\\\\\\\\\

BEGIN_EVENT_TABLE(C_OptionsDlg, wxDialog)
	EVT_BUTTON(wxID_OK, C_OptionsDlg::OnOK)
END_EVENT_TABLE()

C_OptionsDlg::C_OptionsDlg(wxWindow *parent, C_Options& optionsToFill) :
	wxDialog(
		parent,
		wxID_ANY,
		wxString(wxT("Text-to-Speech Options"))),
	m_optionsToFill(optionsToFill)
{	
    wxBoxSizer* pMainSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* pOutputSizer = new wxBoxSizer(wxHORIZONTAL);

	pOutputSizer->Add(new wxStaticText(this, wxID_ANY, _T("Output Folder: ")),
                   0, wxALL, 0);

	wxString defaultPath = optionsToFill.GetPath();

	m_pPathTxtCtrl = new wxTextCtrl(this, 
		wxID_ANY, defaultPath, wxDefaultPosition, wxSize(407, 20));

	pOutputSizer->Add(m_pPathTxtCtrl, 0, wxALL, 0);

	pMainSizer->Add(pOutputSizer, 0, wxALL | wxEXPAND, 8);

	wxGrid* pGrid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

	wxGridTableBase* pTable = new C_TTSSettingsTable(optionsToFill);
    pGrid->SetTable(pTable, true);

	wxGridCellAttr* pCharacter = new wxGridCellAttr;
	pCharacter->SetReadOnly();
	pCharacter->SetBackgroundColour(wxColour(230, 230, 230));

	pGrid->SetColAttr(C_TTSSettingsTable::COL_CHARACTER, pCharacter);

	C_TTSWav waver;
	wxArrayString speakers = waver.EnumerateSpeakers();

	wxGridCellAttr* pSpeaker = new wxGridCellAttr;
	pSpeaker->SetEditor(new wxGridCellChoiceEditor(speakers));
	pGrid->SetColAttr(C_TTSSettingsTable::COL_SPEAKER, pSpeaker);

	wxGridCellAttr* pSpeed = new wxGridCellAttr;
	pSpeed->SetEditor(new wxGridCellNumberEditor(-10, 10));
	pGrid->SetColAttr(C_TTSSettingsTable::COL_SPEED, pSpeed);

	wxGridCellAttr* pVolume = new wxGridCellAttr;
	pVolume->SetEditor(new wxGridCellNumberEditor(0, 100));
	pGrid->SetColAttr(C_TTSSettingsTable::COL_VOLUME, pVolume);

	pGrid->SetRowLabelSize(1);
	pGrid->SetColSize(0, 200);
	pGrid->SetColSize(1, 180);
	pGrid->SetColSize(2, 50);
	pGrid->SetColSize(3, 50);
	for(int i = 0; i < pGrid->GetNumberRows(); i++)
	{
		pGrid->SetRowSize(i, 25);
	}
	
	pMainSizer->Add(pGrid, 1, wxALL | wxEXPAND, 8);

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
	CenterOnParent();
	Layout();
}

void C_OptionsDlg::OnOK(wxCommandEvent& event)
{
	m_optionsToFill.SetPath(m_pPathTxtCtrl->GetValue());

	if(IsPathOk(m_optionsToFill.GetPath()))
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
}

/////////////////////////// C_TTSSettingsTable \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_TTSSettingsTable::C_TTSSettingsTable(C_Options& options)
{
	Btl::Map<Character, Voice>& voices = wxGetApp().GetConfig().tts.voices; 
	Btl::Map<Character, Voice>::Iterator itVoices;

	Btl::OpenHashSet<Character> charactersInRecord = options.GetCharactersInRecord();
	
	for(itVoices = voices.Begin(); itVoices != voices.End(); ++itVoices)
	{
		if(charactersInRecord.Find( itVoices->GetKey() ) != charactersInRecord.End())
			m_rowsToCharacter.PushBack(itVoices->GetKey());
	}	
}

wxString C_TTSSettingsTable::GetTypeName(int WXUNUSED(row), int col)
{
    switch ( col )
    {      
        case COL_CHARACTER:
		case COL_SPEAKER:
            return wxString::Format(_T("%s:80"), wxGRID_VALUE_STRING);

		case COL_SPEED:
		case COL_VOLUME:
			 return wxGRID_VALUE_NUMBER;
    }

    wxFAIL_MSG(_T("unknown column"));

    return wxEmptyString;
}

int C_TTSSettingsTable::GetNumberRows()
{
	return m_rowsToCharacter.Size();
}

int C_TTSSettingsTable::GetNumberCols()
{
    return COL_MAX;
}

bool C_TTSSettingsTable::IsEmptyCell( int WXUNUSED(row), int WXUNUSED(col) )
{
    return false;
}

wxString C_TTSSettingsTable::GetValue( int row, int col )
{
    Btl::Map<Character, Voice>& voices = wxGetApp().GetConfig().tts.voices;

    switch ( col )
    {
        case COL_CHARACTER:
			return wxString(m_rowsToCharacter[row].Chars()).Upper();

        case COL_SPEAKER:
			return voices[m_rowsToCharacter[row].Upper()].speaker.Chars();

        case COL_SPEED:
            return wxString::Format(_T("%d"), voices[m_rowsToCharacter[row].Upper()].speed);

        case COL_VOLUME:
            return wxString::Format(_T("%d"), voices[m_rowsToCharacter[row].Upper()].volume);
    }

    return wxEmptyString;
}

void C_TTSSettingsTable::SetValue( int row, int col, const wxString& value )
{
	Btl::Map<Character, Voice>& voices = wxGetApp().GetConfig().tts.voices;

    switch ( col )
    {
        case COL_CHARACTER:
		case COL_SPEED:
		case COL_VOLUME:
            wxFAIL_MSG(_T("unexpected column"));
            break;

        case COL_SPEAKER:
            voices[m_rowsToCharacter[row].Upper()].speaker = value.c_str();
            break;
    } 
}

bool C_TTSSettingsTable::CanGetValueAs(int WXUNUSED(row),
                             int col,
                             const wxString& typeName)
{
    if ( typeName == wxGRID_VALUE_STRING )
    {
        return true;
    }
    else if ( typeName == wxGRID_VALUE_NUMBER )
    {
        return col == COL_SPEED || col == COL_VOLUME;
    }
    else
    {
        return false;
    }
}

bool C_TTSSettingsTable::CanSetValueAs( int row, int col, const wxString& typeName )
{
    return CanGetValueAs(row, col, typeName);
}

long C_TTSSettingsTable::GetValueAsLong( int row, int col )
{
    Btl::Map<Character, Voice>& voices = wxGetApp().GetConfig().tts.voices;

    switch ( col )
    {
        case COL_SPEED:
            return voices[m_rowsToCharacter[row].Upper()].speed;

        case COL_VOLUME:
            return voices[m_rowsToCharacter[row].Upper()].volume;
        
        default:
            wxFAIL_MSG(_T("unexpected column"));
            return -1;
    }
}


void C_TTSSettingsTable::SetValueAsLong( int row, int col, long value )
{
    Btl::Map<Character, Voice>& voices = wxGetApp().GetConfig().tts.voices;

    switch ( col )
    {
		case COL_SPEED:
            voices[m_rowsToCharacter[row].Upper()].speed = value;
			break;

        case COL_VOLUME:
            voices[m_rowsToCharacter[row].Upper()].volume = value;
			break;
       
        default:
            wxFAIL_MSG(_T("unexpected column"));
    }
}


wxString C_TTSSettingsTable::GetColLabelValue( int col )
{
    switch ( col )
    {
		case COL_CHARACTER:
            return "Character";

		case COL_SPEAKER:
            return "Speaker";

		case COL_SPEED:
            return "Speed";

        case COL_VOLUME:
            return "Volume";
       
        default:
            wxFAIL_MSG(_T("unexpected column"));
    }
	return wxEmptyString;
}

wxString C_TTSSettingsTable::GetRowLabelValue( int row )
{
	return "";
}

/////////////////////////// GenerateWavs::Do \\\\\\\\\\\\\\\\\\\\\\\\\\\

bool Do(const C_Options& options)
{
	LOGINFO(wxGetApp().GetLogger(), ">>> STARTING TO GENERATE TTS FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());

	C_TTSWav waver;
	
	C_RowSet* pRowSet = options.GetRecord()->GetRowSet(0);
	if (!pRowSet)
		return false;

	U32 count = pRowSet->GetNumRows();
	if (!count) 
	{
		LOGINFO(wxGetApp().GetLogger(), "Empty rowset, nothing to generate.\n");
		return false;	
	}

	DialogueManConfig& config = wxGetApp().GetConfig(); 

	wxProgressDialog progress(_T("Generating Text-to-Speech..."),
							_T("Generating Text-to-Speech"),
							count,    // range
							NULL,   // parent
							wxPD_CAN_ABORT |
							wxPD_APP_MODAL |
							wxPD_ELAPSED_TIME |
							wxPD_ESTIMATED_TIME |
							wxPD_REMAINING_TIME
							);

	C_MacroExpander expander;
	int rowsGenerated = 0;
	for(U32 i = 0; i < count; i++)
	{
		bool cont = progress.Update(i, wxString::Format("Row %d of %d", i, count));
		if (!cont)
		{
			if (wxMessageBox(_T("Do you really want to cancel?"),
				_T("Generate TTS"),  // caption
				wxYES_NO | wxICON_QUESTION) == wxYES)
			{
				break;
			}

			cont = true;
			progress.Resume();
		}

		wxMilliSleep(10);
		
		expander.Clear();
		C_Row* pRow = pRowSet->GetRow(i);
		NameHelpers::AddFieldValue(expander, pRowSet->GetAllColumnSet(), pRow);		
		Util::C_String fullWavFileName = NameHelpers::GetFullWavFileName(expander);

		//Generate the WAV file
		if(waver.ToWav(
			(const char*)expander.GetValue("English"), 
			(const char*)fullWavFileName, 
			config.tts.voices[expander.GetValue("Character").Upper()]))
		{
			rowsGenerated++;	
		}

	}

	LOGINFO(wxGetApp().GetLogger(), "Generated %d TTS files.\n", rowsGenerated);
	LOGINFO(wxGetApp().GetLogger(), "<<< DONE GENERATING TTS FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());
	return true;
}

} //namespace GenerateWavs

} //namespace DMan
