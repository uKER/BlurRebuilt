#include "stdafx.h"

#include <BasicExcel/BasicExcel.h>

#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/Rowset.h>
#include <DialogueMan/Document/Record.h>
#include <Dialogueman/Diff/DiffViewer.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <Dialogueman/Xt/XtTypeDatabase.h>
#include <Dialogueman/ImportExport/ImportWizard.h>
#include <Dialogueman/ImportExport/Reader.h>
#include <Dialogueman/ImportExport/ExcelReader.h>
#include <Dialogueman/ImportExport/FinalDraftReader.h>
#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <Dialogueman/App.h>

namespace DMan
{

/////////////////////////// C_ImportWizard \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_ImportWizard::C_ImportWizard(wxFrame* frame, bool useSizer)
        : wxWizard(frame, wxID_ANY,	wxT("Import Wizard")), 
	m_pPage1(NULL), m_pPage2(NULL), m_pPage3(NULL), m_pReader(NULL)
{	
	SetReader(C_Reader::E_ReaderType_Excel);

	m_pPage1 = new C_SelectFilePage(this);

	m_pPage2 = new C_SelectItemTypePage(this, m_pPage1);
	m_pPage1->SetNext(m_pPage2);

	m_pPage3 = new C_SelectLocTablePage(this, m_pPage2);
	m_pPage2->SetNext(m_pPage3);
    
	if (useSizer)
    {
        // allow the wizard to size itself around the pages
        GetPageAreaSizer()->Add(m_pPage1);
    }
}

C_ImportWizard::~C_ImportWizard()
{
	if(m_pReader)
	{
		delete m_pReader;
		m_pReader = NULL;
	}
}

void C_ImportWizard::SetReader(C_Reader::E_ReaderType type)
{
	if(m_pReader)
	{
		delete m_pReader;
		m_pReader = NULL;
	}

	switch(type)
	{
	case  C_Reader::E_ReaderType_Excel:
		m_pReader = new C_ExcelReader();
		break;
	case  C_Reader::E_ReaderType_FinalDraft:
		m_pReader = new C_FinalDraftReader();
		break;
	}
}

/////////////////////////// C_SelectFilePage \\\\\\\\\\\\\\\\\\\\\\\\\\\

BEGIN_EVENT_TABLE(C_SelectFilePage, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY, C_SelectFilePage::OnWizardPageChanging)
END_EVENT_TABLE()

C_SelectFilePage::C_SelectFilePage(wxWizard* parent,
								   wxWizardPage* prev,
								   wxWizardPage* next)
	: wxWizardPageSimple(parent, prev, next), m_pWizard((C_ImportWizard*)parent)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(new wxStaticText(this, wxID_ANY, wxT("This wizard makes it easy to import Dialogue Lines")),
				   0,
				   wxALL,
				   2);

	mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 2);
	mainSizer->AddSpacer(100);


	wxBoxSizer* horizSizer = new wxBoxSizer(wxHORIZONTAL);

	horizSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Please specify the file to import:")),
				   0,
				   wxALL,
				   2);

	m_pFileNamePicker = new wxFilePickerCtrl(this, 
		wxID_ANY, 
		wxT(""), 
		wxT("Select a File to Import"),
		wxT("Excel and Final Draft files (*.xls;*.fcf)|*.xls;*.fcf"),
		wxDefaultPosition,
		wxDefaultSize,
		wxFLP_DEFAULT_STYLE,
		wxDefaultValidator); 

	horizSizer->Add(m_pFileNamePicker,
				   1,
				   wxALL,
				   2);
	
	mainSizer->Add(horizSizer, 0);

	SetSizer(mainSizer);
	mainSizer->Fit(this);
}

void C_SelectFilePage::OnWizardPageChanging(wxWizardEvent& event)
{
	if ( event.GetDirection() ) //going forward
	{
		wxFileName fileName(m_pFileNamePicker->GetPath());
		if(fileName.GetExt() == "xls")
		{
			m_pWizard->SetReader(C_Reader::E_ReaderType_Excel);	
		} 
		else if(fileName.GetExt() == "fcf")
		{
			m_pWizard->SetReader(C_Reader::E_ReaderType_FinalDraft);	
		}
		
		C_Reader& reader = m_pWizard->GetReader();
		reader.SetFilename(m_pFileNamePicker->GetPath());

		if(!reader.OpenFile())
		{
			wxMessageDialog msg(this, 
					wxT("The specified file cannot be opened\n") 
					wxT("Is the filename correct?\n") 
					wxT("Is there any other program using it?"), 
					wxT("Error Loading File"),
					wxOK | wxICON_WARNING);
			msg.ShowModal();
			event.Veto(); //don't allow to change page
		} 
	}
}

/////////////////////////// C_SelectItemTypePage \\\\\\\\\\\\\\\\\\\\\\\\\\\

BEGIN_EVENT_TABLE(C_SelectItemTypePage, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY, C_SelectItemTypePage::OnWizardPageChanging)
END_EVENT_TABLE()

C_SelectItemTypePage::C_SelectItemTypePage(wxWizard* parent,
								   wxWizardPage* prev,
								   wxWizardPage* next)
	: wxWizardPageSimple(parent, prev, next), m_pWizard((C_ImportWizard*)parent)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(new wxStaticText(this, wxID_ANY, wxT("This wizard makes it easy to import Dialogue Lines")),
				   0,
				   wxALL,
				   2);

	mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 2);
	mainSizer->AddSpacer(100);

	wxBoxSizer* horizSizer = new wxBoxSizer(wxHORIZONTAL);

	horizSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Please select the type of the imported lines: ")),
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

void C_SelectItemTypePage::FillTypeChoice()
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

void C_SelectItemTypePage::OnWizardPageChanging(wxWizardEvent& event)
{
	C_Reader& reader = m_pWizard->GetReader();

	if ( event.GetDirection() ) //going forward
	{
		int selection = m_pTypeChoice->GetSelection();
		LarvaType* pItemType = (LarvaType*)m_pTypeChoice->GetClientData(selection);
		if(pItemType)
		{
			reader.SetItemType(pItemType);
		}
	}
}

/////////////////////////// C_SelectLocTablePage \\\\\\\\\\\\\\\\\\\\\\\\\\\

BEGIN_EVENT_TABLE(C_SelectLocTablePage, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY, C_SelectLocTablePage::OnWizardPageChanging)
END_EVENT_TABLE()

C_SelectLocTablePage::C_SelectLocTablePage(wxWizard* parent,
								   wxWizardPage* prev,
								   wxWizardPage* next)
	: wxWizardPageSimple(parent, prev, next), m_pWizard((C_ImportWizard*)parent)
{
	m_pXlsOptions = new C_ExcelImportOptionsPage(parent, this);
	m_pFcfOptions = new C_FinalDraftImportOptionsPage(parent, this);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(new wxStaticText(this, wxID_ANY, wxT("This wizard makes it easy to import Dialogue Lines")),
				   0,
				   wxALL,
				   2);

	mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 2);
	mainSizer->AddSpacer(100);

	wxBoxSizer* horizSizer = new wxBoxSizer(wxHORIZONTAL);

	horizSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Please select the LocTable to store the imported lines: ")),
				   0,
				   wxALL,
				   2);

	m_pLocTableChoice = new wxChoice(
		this, 
		wxID_ANY, 
		wxDefaultPosition, 
		wxDefaultSize);

	PopulateLocTableChoice();

	horizSizer->Add(m_pLocTableChoice,
				   1,
				   wxEXPAND | wxALL,
				   2);

	mainSizer->Add(horizSizer, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 2);

	SetSizer(mainSizer);
	mainSizer->Fit(this);
}

void C_SelectLocTablePage::PopulateLocTableChoice()
{
	LarvaType* pLarvaType = wxGetApp().GetDb()->schema->types["XtLocTable"];
	Btl::Map<XtString, Xt::DomRecord*> sortedlocTables;
	Btl::Map<XtString, Xt::DomRecord*>::Iterator sortedlocTablesIt;
	
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(wxGetApp().GetDb()->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		for(XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator recordIt(&(pPack->records)); recordIt; ++recordIt)
		{
			Xt::DomRecord* pRec = recordIt;
			LarvaType* pCurrentType = Xt::TypeOf(wxGetApp().GetDb()->schema, pRec);
			if (pCurrentType != pLarvaType)
				continue;
						
			sortedlocTables[pRec->name] = pRec;
		}
	}

	wxString recordName = CurrentDialogueRecordName().Chars();
	int selection = 0;
	int idx = 0;

	for(sortedlocTablesIt = sortedlocTables.Begin(); sortedlocTablesIt != sortedlocTables.End(); ++sortedlocTablesIt, ++idx)
	{
		wxString locTableName = sortedlocTablesIt->GetKey().Text();
		m_pLocTableChoice->Append(locTableName, sortedlocTablesIt->GetValue());

		if(locTableName.Contains(recordName))
			selection = idx;
	}

	m_pLocTableChoice->SetSelection(selection);
}

void C_SelectLocTablePage::OnWizardPageChanging(wxWizardEvent& event)
{
	C_Reader& reader = m_pWizard->GetReader();
	if ( event.GetDirection() ) //going forward
	{
		int selection = m_pLocTableChoice->GetSelection();
		Xt::DomRecord* pLocTable =	(Xt::DomRecord*)m_pLocTableChoice->GetClientData(selection);
		if(pLocTable)
		{
			reader.SetLocTable(pLocTable);
		}

		if(reader.GetReaderType() == C_Reader::E_ReaderType_Excel)
		{
			C_ExcelReader& excelReader = (C_ExcelReader&)reader;
			if(excelReader.IsFileOpen())
			{	
				//Set the Sheets list
				YExcel::BasicExcel& excel = excelReader.GetExcelDoc();
				size_t sheetsCount = excel.GetTotalWorkSheets();
				C_ExcelImportOptionsPage& nextPage = (C_ExcelImportOptionsPage&)*GetNext();
				wxChoice& sheetsChoice = nextPage.GetSheetsChoice();
				sheetsChoice.Clear();
				for(size_t sheetIndex = 0; sheetIndex  < sheetsCount; ++sheetIndex)
				{
					wxString sheetName =  excel.GetUnicodeSheetName(sheetIndex) == 0 ? 
						ToUnicode(excel.GetAnsiSheetName(sheetIndex)) : 
						excel.GetUnicodeSheetName(sheetIndex);
					
					sheetsChoice.Append(sheetName);
				}
				sheetsChoice.Select(0);
				excelReader.SetSheet(0);

				//...and default Range...
				YExcel::BasicExcelWorksheet* sheet = excel.GetWorksheet((size_t)0);

				excelReader.SetRange( 1, 1, sheet->GetTotalRows(), sheet->GetTotalCols() );
				wxTextCtrl& rangeCtrl = nextPage.GetRangeCtrl();
				rangeCtrl.ChangeValue(excelReader.GetRangeStr());

				//...and the Expected Cols 
				//nextPage.ResetExpectedColumns();
				
				nextPage.RefreshPreview();
				
			}
		}
	}
}

wxWizardPage* C_SelectLocTablePage::GetNext() const
{
	C_Reader& reader = m_pWizard->GetReader();
	if(reader.GetReaderType() == C_Reader::E_ReaderType_FinalDraft)
	{
		return m_pFcfOptions;		
	}
	return m_pXlsOptions;
}

/////////////////////////// C_ExcelImportOptionsPage \\\\\\\\\\\\\\\\\\\\\\\\\\\

BEGIN_EVENT_TABLE(C_ExcelImportOptionsPage, wxWizardPageSimple)
	EVT_BUTTON(ID_REFRESH_BTN, C_ExcelImportOptionsPage::OnRefresh) 
	EVT_BUTTON(ID_MOVE_UP_BTN, C_ExcelImportOptionsPage::OnMoveUp) 
	EVT_BUTTON(ID_MOVE_DOWN_BTN, C_ExcelImportOptionsPage::OnMoveDown) 
	EVT_BUTTON(ID_REMOVE_BTN, C_ExcelImportOptionsPage::OnRemove) 
	EVT_BUTTON(ID_ADD_IGNORE_COL_BTN, C_ExcelImportOptionsPage::OnAddIgnoreColumn) 
	EVT_BUTTON(ID_RESET_BTN, C_ExcelImportOptionsPage::OnReset) 
	EVT_CHOICE(C_ExcelImportOptionsPage::ID_SHEETS_CHOICE, C_ExcelImportOptionsPage::OnSheetsChoiceSelected)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, C_ExcelImportOptionsPage::OnWizardPageChanging)
END_EVENT_TABLE()

const wxString C_ExcelImportOptionsPage::kIgnoreStr = wxT("::: IGNORE :::");
const int C_ExcelImportOptionsPage::kPreviewRows = 3;
const int C_ExcelImportOptionsPage::kPreviewCols = 5;

C_ExcelImportOptionsPage::C_ExcelImportOptionsPage(wxWizard* parent,
									   wxWizardPage* prev,
								       wxWizardPage* next)
	: wxWizardPageSimple(parent, prev, next), m_pWizard((C_ImportWizard*)parent)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxGridSizer* sheetNrange = new wxGridSizer(2, 3, 0, 0);

	sheetNrange->Add(new wxStaticText(this, wxID_ANY, wxT("Sheet:")),
				   1,
				   wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,
				   2);

	sheetNrange->Add(new wxStaticText(this, wxID_ANY, wxT("Range: (first row must have Column Labels)")),
				   1,
				   wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,
				   2);

	sheetNrange->Add(1, 1);

	
	m_pSheetsChoice = new wxChoice(this, ID_SHEETS_CHOICE, wxDefaultPosition, wxDefaultSize, 0, NULL);
	
	sheetNrange->Add(m_pSheetsChoice,
				   1,
				   wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL,
				   2);

	m_pRangeCtrl = new wxTextCtrl(this, ID_RANGE_CTRL, wxT(""), wxDefaultPosition, wxSize(150, 20));
	sheetNrange->Add(m_pRangeCtrl,
				   1,
				   wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,
				   2);

	sheetNrange->Add(new wxButton(this, 
								  ID_REFRESH_BTN, 
								  wxT("Refresh Preview")),
				   1,
				   wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL,
				   2);

	sheetNrange->Fit(this);
	sheetNrange->SetSizeHints(this);

	mainSizer->Add(sheetNrange, 0, wxALL, 2);

	mainSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Preview:")),
				   0,
				   wxALL,
				   2);

	m_pExcelPreview = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxSize(400, 110));
	m_pExcelPreview->CreateGrid(kPreviewRows, kPreviewCols);
	m_pExcelPreview->SetRowLabelSize(20);
	m_pExcelPreview->SetColLabelSize(20);
	m_pExcelPreview->EnableEditing(false);
	
	mainSizer->Add(m_pExcelPreview,
				   1,
				   wxEXPAND | wxALL,
				   2);

	/*
	wxStaticBox* colBox = new wxStaticBox(this, wxID_ANY, wxT("Expected Columns"), wxDefaultPosition, wxSize(400, 75));
	wxStaticBoxSizer* colBoxSizer = new wxStaticBoxSizer(colBox, wxVERTICAL);
	
	wxBoxSizer* colButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
	colButtonsSizer->Add(new wxButton(this, 
								  ID_MOVE_UP_BTN, 
								  wxT("Move Up")),
				   0,
				   wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL,
				   2);

	colButtonsSizer->Add(new wxButton(this, 
								  ID_MOVE_DOWN_BTN, 
								  wxT("Move Down")),
				   0,
				   wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL,
				   2);

	colButtonsSizer->Add(new wxButton(this, 
								  ID_REMOVE_BTN, 
								  wxT("Remove")),
				   0,
				   wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL,
				   2);

	colButtonsSizer->Add(new wxButton(this, 
								  ID_ADD_IGNORE_COL_BTN, 
								  wxT("Add Ignore Column")),
				   0,
				   wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL,
				   2);

	colButtonsSizer->Add(new wxButton(this, 
								  ID_RESET_BTN, 
								  wxT("Reset")),
				   0,
				   wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL,
				   2);

	colBoxSizer->Add(colButtonsSizer, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 2);

	m_pExpectedColsList = new wxListBox(this, ID_EXPECTED_COLS_LIST, wxDefaultPosition, wxSize(100, 75), 0, NULL, wxLB_SINGLE);
	colBoxSizer->Add(m_pExpectedColsList, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 2);
	

	mainSizer->Add(colBoxSizer, 0, wxEXPAND | wxALL, 2);*/

	SetSizer(mainSizer);
	mainSizer->Fit(this);
}

void C_ExcelImportOptionsPage::OnRefresh(wxCommandEvent& event)
{
	C_ExcelReader& reader = (C_ExcelReader&)m_pWizard->GetReader();
	reader.SetRange(m_pRangeCtrl->GetValue());
	RefreshPreview();
}

void C_ExcelImportOptionsPage::RefreshPreview()
{
	C_ExcelReader& reader = (C_ExcelReader&)m_pWizard->GetReader();
		
	if(reader.IsFileOpen())
	{
		YExcel::BasicExcel& excel = reader.GetExcelDoc();
		YExcel::BasicExcelWorksheet* pSheet = excel.GetWorksheet(reader.GetSheet());
		if(pSheet)
		{	
			//Reset the grid
			m_pExcelPreview->ClearGrid();
			m_pExcelPreview->DeleteRows(0, m_pExcelPreview->GetNumberRows()); 
			m_pExcelPreview->DeleteCols(0, m_pExcelPreview->GetNumberCols());
			int rowsToAdd = reader.GetMaxRow() - reader.GetMinRow() + 1;
			int colsToAdd = reader.GetMaxCol() - reader.GetMinCol() + 1;
			assert(rowsToAdd >= 0 && colsToAdd >= 0);
			m_pExcelPreview->AppendRows(rowsToAdd);
			m_pExcelPreview->AppendCols(colsToAdd);

			//For each row
			for(size_t r = reader.GetMinRow() - 1, i = 0; r < reader.GetMaxRow(); ++r, ++i)
			{
				//For each column
				for(size_t c = reader.GetMinCol() - 1, j = 0; c < reader.GetMaxCol(); ++c, ++j)
				{
					YExcel::BasicExcelCell* pCell = pSheet->Cell(r, c, false);
						
					if(pCell)
					{
						switch (pCell->Type())
						{
							case YExcel::BasicExcelCell::INT:
							{
								m_pExcelPreview->SetCellValue(i, j, ToString((S32)pCell->GetInteger()) );
								break;
							}

							case YExcel::BasicExcelCell::DOUBLE:
							{
								m_pExcelPreview->SetCellValue(i, j, ToString(pCell->GetDouble()) );
								break;
							}

							case YExcel::BasicExcelCell::STRING:
							{
								m_pExcelPreview->SetCellValue(i, j, pCell->GetString() );
								break;
							}

							case YExcel::BasicExcelCell::WSTRING:
							{
								const wxCharBuffer value = ToAscii(wxString(pCell->GetWString()));
								m_pExcelPreview->SetCellValue(i, j, value);
								break;
							}							
						}
					}
				} ////For each column
			}//For each row
		}//if(pSheet)
	}	
}

void C_ExcelImportOptionsPage::OnMoveUp(wxCommandEvent& event)
{
	int current = m_pExpectedColsList->GetSelection();
	if(current == wxNOT_FOUND)
		return;

	int previous = current - 1;
	if(previous >= 0)
	{
		C_Column* pCurrentCol = (C_Column*)m_pExpectedColsList->GetClientData(current);
		C_Column* pPreviousCol = (C_Column*)m_pExpectedColsList->GetClientData(previous);

		//update previous
		m_pExpectedColsList->SetString(previous, pCurrentCol ? pCurrentCol->GetLabel() : kIgnoreStr);
		m_pExpectedColsList->SetClientData(previous, pCurrentCol);

		//update current
		m_pExpectedColsList->SetString(current, pPreviousCol ? pPreviousCol->GetLabel() : kIgnoreStr);
		m_pExpectedColsList->SetClientData(current, pPreviousCol);

		//select the moved item
		m_pExpectedColsList->SetSelection(previous);
	}
}

void C_ExcelImportOptionsPage::OnMoveDown(wxCommandEvent& event)
{
	int current = m_pExpectedColsList->GetSelection();
	if(current == wxNOT_FOUND)
		return;

	int next = current + 1;
	if(next < (int)m_pExpectedColsList->GetCount())
	{
		C_Column* pCurrentCol = (C_Column*)m_pExpectedColsList->GetClientData(current);
		C_Column* pNextCol = (C_Column*)m_pExpectedColsList->GetClientData(next);

		//update next
		m_pExpectedColsList->SetString(next, pCurrentCol ? pCurrentCol->GetLabel() : kIgnoreStr);
		m_pExpectedColsList->SetClientData(next, pCurrentCol);

		//update current
		m_pExpectedColsList->SetString(current, pNextCol ? pNextCol->GetLabel() : kIgnoreStr);
		m_pExpectedColsList->SetClientData(current, pNextCol);

		//select the moved item
		m_pExpectedColsList->SetSelection(next);
	}
}

void C_ExcelImportOptionsPage::OnRemove(wxCommandEvent& event)
{
	int current = m_pExpectedColsList->GetSelection();
	if(current == wxNOT_FOUND)
		return;

	//Do we still have columns other than "Ignore"?
	int nonIgnoredColsCount = 0;
	for(U32 i = 0; i < m_pExpectedColsList->GetCount(); ++i)
	{
		C_Column* pCol = (C_Column*)m_pExpectedColsList->GetClientData(i);
		if(pCol)
			nonIgnoredColsCount++;	
	}

	if(m_pExpectedColsList->GetCount() > 1)
	{
		C_Column* pCol = (C_Column*)m_pExpectedColsList->GetClientData(current);
		//Delete allowed on Ignore columns or on Non-Ignored column as long as is not the only one remaining
		if(!pCol || (pCol && nonIgnoredColsCount > 1))
		{
			m_pExpectedColsList->Delete(current);
			//Select the element after the deleted one, which now has cuurent position
			if(current < (int)m_pExpectedColsList->GetCount())
				m_pExpectedColsList->SetSelection(current);
			else 
				m_pExpectedColsList->SetSelection(m_pExpectedColsList->GetCount() - 1);
		}
	}
}

void C_ExcelImportOptionsPage::OnAddIgnoreColumn(wxCommandEvent& event)
{
	int current = m_pExpectedColsList->GetSelection();
	if(current == wxNOT_FOUND)
		return;

	m_pExpectedColsList->Insert(kIgnoreStr, current);
	//select the added item
	m_pExpectedColsList->SetSelection(current);
}

void C_ExcelImportOptionsPage::OnReset(wxCommandEvent& event)
{
	ResetExpectedColumns();
}

void C_ExcelImportOptionsPage::OnSheetsChoiceSelected(wxCommandEvent& event)
{
	C_ExcelReader& reader = (C_ExcelReader&)m_pWizard->GetReader();
	reader.SetSheet(m_pSheetsChoice->GetSelection());
		
	if(reader.IsFileOpen())
	{
		//Reset the default Range
		YExcel::BasicExcelWorksheet* sheet = reader.GetExcelDoc().GetWorksheet(reader.GetSheet());
				
		reader.SetRange( 1, 1, sheet->GetTotalRows(), sheet->GetTotalCols() );
		m_pRangeCtrl->ChangeValue(reader.GetRangeStr());
	}
}

void C_ExcelImportOptionsPage::OnWizardPageChanging(wxWizardEvent& event)
{
	C_ExcelReader& reader = (C_ExcelReader&)m_pWizard->GetReader();
	if ( event.GetDirection() ) //going forward (i.e because it's the last page, equals finish)
	{
		if( !reader.HasValidColumnMapping() )
		{
			wxMessageDialog msg(this, 
				wxT("Cannot find cells with valid Column Labels\n") 
				wxT("Please make sure the first row contains them\n"), 
				wxT("Import Error"),
				wxOK | wxICON_WARNING);
			msg.ShowModal();
			event.Veto(); //don't allow to finish
		}
	}
}

void C_ExcelImportOptionsPage::ResetExpectedColumns()
{
	/*
	m_pExpectedColsList->Clear();
	C_ExcelReadOptions& ro = m_pWizard->GetReadOptions();
	C_ColumnSet* pColSet = wxGetApp().GetDbManager().GetTypeColumnSet(ro.GetItemType());

	//Add expected Columns
	U32 numCols = pColSet->GetNumColumns();
	for(U32 i = 0; i < numCols; ++i)
	{
		C_Column* pCol = pColSet->GetColumn(i);
		if(pCol && pCol->IsImportable())
		{
			m_pExpectedColsList->Append(pCol->GetLabel(), pCol);
		}
	}  */
}

/////////////////////////// C_ExcelImportOptionsPage \\\\\\\\\\\\\\\\\\\\\\\\\\\

BEGIN_EVENT_TABLE(C_FinalDraftImportOptionsPage, wxWizardPageSimple)
END_EVENT_TABLE()

C_FinalDraftImportOptionsPage::C_FinalDraftImportOptionsPage(wxWizard* parent,
									   wxWizardPage* prev,
								       wxWizardPage* next)
	: wxWizardPageSimple(parent, prev, next), m_pWizard((C_ImportWizard*)parent)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Please choose the options for the import process")),
				   0,
				   wxALL,
				   2);

	mainSizer->Add(new wxStaticText(this, wxID_ANY, wxT("No options yet!")),
				   0,
				   wxALL,
				   2);
	
	SetSizer(mainSizer);
	mainSizer->Fit(this);
}

}//namespace DMan