#include "stdafx.h"

#include <BasicExcel/BasicExcel.h>

#include <Tools_Shared/Utility/MacroExpander.h>

#include <DialogueMan/Document/Rowset.h>

#include <DialogueMan/Xt/XtTypeDatabase.h>

#include <DialogueMan/ImportExport/ExportWizard.h>
#include <Dialogueman/ImportExport/ExcelWriter.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/Misc/StringUtil.h>

#include <system/misc/StringUtil.h>
#include <system/misc/BtlStringUtil.h>

#include <DialogueMan/App.h>

namespace DMan
{


C_ExportWizard::C_ExportWizard(wxFrame* frame, bool useSizer)
        : wxWizard(frame,wxID_ANY,_T("Export Wizard"))
{
    m_pPage1 = new C_ExpSelectItemTypePage(this);

	m_pPage2 = new C_ExpSelectFilePage(this, m_pPage1);
	m_pPage1->SetNext(m_pPage2);
	    
	if (useSizer)
    {
        // allow the wizard to size itself around the pages
        GetPageAreaSizer()->Add(m_pPage1);
    }

	m_pExcelWriter = new C_ExcelWriter();
}

C_ExportWizard::~C_ExportWizard()
{
	if(m_pExcelWriter)
	{
		delete m_pExcelWriter;
		m_pExcelWriter = NULL;
	}
}

BEGIN_EVENT_TABLE(C_ExpSelectItemTypePage, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY, C_ExpSelectItemTypePage::OnWizardPageChanging)
END_EVENT_TABLE()

C_ExpSelectItemTypePage::C_ExpSelectItemTypePage(wxWizard* parent,
								   wxWizardPage* prev,
								   wxWizardPage* next)
	: wxWizardPageSimple(parent, prev, next), m_pWizard((C_ExportWizard*)parent)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(new wxStaticText(this, wxID_ANY, wxT("This wizard makes it easy to export Dialogue Lines to an Excel file")),
				   0,
				   wxALL,
				   2);

	mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 2);
	mainSizer->AddSpacer(100);

	wxBoxSizer* horizSizer = new wxBoxSizer(wxHORIZONTAL);

	horizSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Please select the type you wish to export: ")),
				   0,
				   wxALL,
				   2);
	
	m_pTypeChoice = new wxChoice(
		this, 
		wxID_ANY,
		wxDefaultPosition,
		wxDefaultSize);
	
	FillTypeChoice();

	horizSizer->Add(m_pTypeChoice,
				   1,
				   wxEXPAND | wxALL,
				   2);

	mainSizer->Add(horizSizer, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 2);

	SetSizer(mainSizer);
	mainSizer->Fit(this);
}

void C_ExpSelectItemTypePage::FillTypeChoice()
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);
	
	C_RowSet* pRowSet = selection.GetCurrentRowSet();
	if (!pRowSet)
		return;
	
	LfArray* pLarvaArray = (LfArray*)pRowSet->GetXtType().AsField();
	LtStruct* pBaseItemType = (LtStruct*)pLarvaArray->field->Type();
		   
	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&wxGetApp().GetDb()->schema->types); it; ++it)
	{
		if (it->kind != kLtStruct)
			continue;
		
		LtStruct* pDerivedItemType = (LtStruct*)(LarvaType*)it;
		
		if (IsBaseOf(pBaseItemType, pDerivedItemType) && pBaseItemType != pDerivedItemType)
		{
			m_pTypeChoice->Append((const char*)pDerivedItemType->label, pDerivedItemType);
		}
	}

	U32 choiceCount = m_pTypeChoice->GetCount();
	if(choiceCount == 0) //If we couldn't find derived types, just add the base one
	{
		m_pTypeChoice->Append((const char*)pBaseItemType->label, pBaseItemType);
	}
	m_pTypeChoice->SetSelection(0);
}

void C_ExpSelectItemTypePage::OnWizardPageChanging(wxWizardEvent& event)
{
	C_ExcelWriter& writer = m_pWizard->GetExcelWriter();
	if ( event.GetDirection() ) //going forward
	{
		int selection = m_pTypeChoice->GetSelection();
		LarvaType* pItemType = (LarvaType*)m_pTypeChoice->GetClientData(selection);
		if(pItemType)
		{
			writer.SetItemType(pItemType);
		}
	}	
}

BEGIN_EVENT_TABLE(C_ExpSelectFilePage, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY, C_ExpSelectFilePage::OnWizardPageChanging)
END_EVENT_TABLE()

C_ExpSelectFilePage::C_ExpSelectFilePage(wxWizard* parent,
								   wxWizardPage* prev,
								   wxWizardPage* next)
	: wxWizardPageSimple(parent, prev, next), m_pWizard((C_ExportWizard*)parent)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(new wxStaticText(this, wxID_ANY, 
		wxT("Please specify a filename where data will be exported to")),
				   0,
				   wxALL,
				   2);

	mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 2);
	mainSizer->AddSpacer(100);


	wxBoxSizer* horizSizer = new wxBoxSizer(wxHORIZONTAL);

	horizSizer->Add(new wxStaticText(this, wxID_ANY, wxT("File name:")),
				   0,
				   wxALL,
				   2);

	Util::C_String defaultFilename = "";
	
	if( !PathHasForbiddenChars(CurrentDialogueRecordName().Chars()) )
	{
		DialogueManConfig& config = wxGetApp().GetConfig();
		config.exportPath = Util::RemoveTrailingSlash(config.exportPath);
		Tools::C_MacroExpander::Instance().Expand(config.exportPath);
		defaultFilename = config.exportPath + "/" + CurrentDialogueRecordName() + ".xls";
	}

	m_pFileNamePicker = new wxFilePickerCtrl(this, 
		wxID_ANY, 
		defaultFilename.Chars(), 
		wxT("Select an Excel File"),
		wxT("*.xls"),
		wxDefaultPosition,
		wxDefaultSize,
		wxFLP_USE_TEXTCTRL | wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT,
		wxDefaultValidator); 

	horizSizer->Add(m_pFileNamePicker,
				   1,
				   wxALL,
				   2);
	
	mainSizer->Add(horizSizer, 0);

	SetSizer(mainSizer);
	mainSizer->Fit(this);
}

void C_ExpSelectFilePage::OnWizardPageChanging(wxWizardEvent& event)
{
	C_ExcelWriter& writer = m_pWizard->GetExcelWriter();
	if ( event.GetDirection() ) //going forward
	{
		const char* fileName = m_pFileNamePicker->GetPath();
		Btl::String dir = Misc::ExtractDirectory(fileName);
		if (!Sys::C_FileSystem::Instance().FileExists(dir.Chars()) &&
			!Sys::C_FileSystem::Instance().CreateDirectory(dir.Chars()))
		{
			wxMessageDialog msg(this, 
				wxT("The specified file cannot be opened\n") 
				wxT("Is the filename correct?\n"), 
				wxT("Error Setting Filename"),
				wxOK | wxICON_WARNING);
			msg.ShowModal();
			event.Veto(); //don't allow to change page
			return;
		}
		writer.SetFilename(m_pFileNamePicker->GetPath());
	}
}	 

}//namespace DMan