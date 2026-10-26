#include "stdafx.h"

#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/Row.h>

#include <DialogueMan/Document/RowSet.h>

#include <dialogueman/Diff/DiffViewer.h>
#include <dialogueman/Diff/Diff.h>

#include <DialogueMan/ImportExport/Reader.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>

#include <DialogueMan/App.h>
#include <wx/tokenzr.h>

namespace DMan
{

BEGIN_EVENT_TABLE(C_DiffViewer, wxFrame)
    EVT_BUTTON(ID_RESET, C_DiffViewer::OnReset)
	EVT_BUTTON(ID_ACCEPT_ALL, C_DiffViewer::OnAcceptAll)
	EVT_BUTTON(wxID_OK, C_DiffViewer::OnOk)
	EVT_BUTTON(wxID_CANCEL, C_DiffViewer::OnCancel)
	EVT_CLOSE(C_DiffViewer::OnClose)
END_EVENT_TABLE()

C_DiffViewer::C_DiffViewer(wxWindow* parent,
	C_Reader& reader,
	wxWindowID id, 
	const wxString& title, 
	const wxPoint& pos, 
	const wxSize& size, 
	long style) : 
		wxFrame(parent, id, title, pos, size, style),
		m_reader(reader),
		m_pModalLoop(NULL),
		m_returnCode(-1)
{
	CreateControls();
}

void C_DiffViewer::CreateControls()
{    
	std::vector< std::vector< wxString > > leftData;
	std::vector< std::vector< wxString > > rightData;
	std::vector< std::vector< wxString > > leftDataToCompare;
	std::vector< std::vector< wxString > > rightDataToCompare;
		

	//Get Importable Columns and Field Labels
	std::vector< wxString > importableCols;
	std::vector< wxString > comparableCols;
	std::vector< wxString > fieldLabels;

	C_ColumnSet* pColSet = wxGetApp().GetDbManager().GetTypeColumnSet(m_reader.GetItemType());
	if(pColSet)
	{
		U32 numCols = pColSet->GetNumColumns();
		for(U32 i = 0; i < numCols; ++i)
		{
			C_Column* pCol = pColSet->GetColumn(i);
			if(pCol && pCol->IsImportable())
			{
				importableCols.push_back(pCol->GetKey());
				fieldLabels.push_back(pCol->GetLabel());
			}
			if(pCol && pCol->IsComparable())
			{
				comparableCols.push_back(pCol->GetKey());
			}
		}
	}
		
	//Load the Left Data (from DB)
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	C_RowSet* pRowSet = selection.GetCurrentRowSet();
	assert(pRowSet);

	C_RowSet::RowArray& currentDataLeft = pRowSet->GetRows();

	for(S32 i = 0; i < currentDataLeft.Size(); i++)
	{
		C_Row* pRow = currentDataLeft[i];

		//Push Left Data
		std::vector< wxString > rowData;
		for(size_t j = 0; j < importableCols.size(); j++)
		{
			wxString value(wxT(""));
			C_Row::ColumnMap& columnMap = pRow->GetColumns();
			C_Row::ColumnMap::Iterator it = columnMap.Find(importableCols[j]);
			if(it != columnMap.End())
			{
				value = wxString(it->GetValue());
			}
			rowData.push_back(value);
		}
		leftData.push_back(rowData);

		//Push Left Data to Compare
		std::vector< wxString > rowDataToCompare;
		for(size_t j = 0; j < comparableCols.size(); j++)
		{
			wxString value(wxT(""));
			C_Row::ColumnMap& columnMap = pRow->GetColumns();
			C_Row::ColumnMap::Iterator it = columnMap.Find(comparableCols[j]);
			if(it != columnMap.End())
			{
				value = wxString(it->GetValue());
			}
			rowDataToCompare.push_back(value);
		}
		leftDataToCompare.push_back(rowDataToCompare);
	}

	//Load the Right Data (from Excel)
	Btl::Vector< Btl::Map<wxString, C_MutantValue> > readRows = m_reader.GetReadRows();
	
	for(int i = 0; i < readRows.Size(); ++i)
	{
		Btl::Map<wxString, C_MutantValue> row = readRows[i];

		//Push Right Data
		std::vector< wxString > rowData;
		for(size_t j = 0; j < importableCols.size(); ++j)
		{
			wxString value(wxT(""));
			Btl::Map<wxString, C_MutantValue>::Iterator it = row.Find(importableCols[j]);
			if(it != row.End())
			{
				value = wxString(it->GetValue());	
			}
			rowData.push_back(value);
		}
		rightData.push_back(rowData);

		//Push Right Data to Compare
		std::vector< wxString > rowDataToCompare;
		for(size_t j = 0; j < comparableCols.size(); ++j)
		{
			wxString value(wxT(""));
			Btl::Map<wxString, C_MutantValue>::Iterator it = row.Find(comparableCols[j]);
			if(it != row.End())
			{
				value = wxString(it->GetValue());	
			}
			rowDataToCompare.push_back(value);
		}
		rightDataToCompare.push_back(rowDataToCompare);
	}

	wxPanel* pPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
		   
	//Labels

	wxGridSizer* pLabelsSizer = new wxGridSizer(1, 2, 0, 0);

	wxStaticText* currentLabel = new wxStaticText ( pPanel, wxID_ANY,
		wxT("Current Lines"), wxDefaultPosition, wxDefaultSize, 0 );

	wxFont currFont = currentLabel->GetFont();
	currFont.SetWeight(wxFONTWEIGHT_BOLD);
	currentLabel->SetFont(currFont);

	pLabelsSizer->Add(currentLabel, 1, wxALIGN_LEFT | wxALL, 5);

	wxStaticText* importedLabel = new wxStaticText ( pPanel, wxID_ANY,
		wxT("To be Imported"), wxDefaultPosition, wxDefaultSize, 0 );
	importedLabel->SetFont(currFont);

	pLabelsSizer->Add(importedLabel, 1, wxALIGN_RIGHT | wxALL, 5);

	topSizer->Add(pLabelsSizer, 0, wxEXPAND | wxALL, 5); 

	//Table
	m_pCompTable = new C_ComparisonTable(
		fieldLabels, 
		leftData, 
		rightData,
		leftDataToCompare, 
		rightDataToCompare,
		pPanel, 
		m_reader,
		wxID_ANY,
		wxDefaultPosition,
		wxSize(800, 300),
		0);

	m_pCompTable->Scroll(17, 0);
	topSizer->Add(m_pCompTable, 1, wxEXPAND | wxALL, 5);
	

	//Legend
	wxStaticText* legendLabel = new wxStaticText ( pPanel, wxID_STATIC,
		wxT("Legend:"), wxDefaultPosition, wxDefaultSize, 0 );

	topSizer->Add(legendLabel, 0, wxALIGN_LEFT | wxALL, 5);

	wxGrid* legendGrid = new wxGrid(pPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	legendGrid->CreateGrid(3, 1);
	legendGrid->SetCellHighlightPenWidth(0);
	legendGrid->DisableDragGridSize();
	legendGrid->EnableEditing(false);
	legendGrid->SetCellValue(0, 0, wxT("     Added     "));
	legendGrid->SetCellValue(1, 0, wxT("    Deleted    "));
	legendGrid->SetCellValue(2, 0, wxT("    Changed    "));
	legendGrid->SetCellBackgroundColour(0, 0, wxColour(128, 255, 128));
	legendGrid->SetCellBackgroundColour(1, 0, wxColour(255, 128, 128));
	legendGrid->SetCellBackgroundColour(2, 0, wxColour(128, 255, 255));
	legendGrid->SetCellAlignment(0, 0, wxALIGN_CENTRE, wxALIGN_CENTRE);
	legendGrid->SetCellAlignment(1, 0, wxALIGN_CENTRE, wxALIGN_CENTRE);
	legendGrid->SetCellAlignment(2, 0, wxALIGN_CENTRE, wxALIGN_CENTRE);
	legendGrid->SetColLabelSize(0);
	legendGrid->SetRowLabelSize(0);
	legendGrid->SetColSize(0, 100);
	legendGrid->AutoSize();
			
	topSizer->Add(legendGrid, 0, wxALIGN_LEFT | wxALL, 5);

	wxStaticText* numDiffLabel = new wxStaticText ( pPanel, wxID_STATIC,
		wxString::Format("Number of Differences: %i", NumOfDifferences()), wxDefaultPosition, wxDefaultSize, 0 );

	topSizer->Add(numDiffLabel, 0, wxALIGN_LEFT | wxALL, 5);

	//Separator
	wxStaticLine* line = new wxStaticLine ( pPanel, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	topSizer->Add(line, 0, wxEXPAND | wxALL, 5);

	//End Buttons 
	wxBoxSizer* okCancelBox = new wxBoxSizer(wxHORIZONTAL);

	wxButton* pAcceptAll = new wxButton (pPanel, ID_ACCEPT_ALL, wxT("&Accept All"),
		wxDefaultPosition, wxDefaultSize, 0);
	okCancelBox->Add(pAcceptAll, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxButton* reset = new wxButton(pPanel, ID_RESET, wxT("&Reset"),
		wxDefaultPosition, wxDefaultSize, 0);
	okCancelBox->Add(reset, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	okCancelBox->AddSpacer(50);

	wxButton* ok = new wxButton (pPanel, wxID_OK, wxT("&OK"),
		wxDefaultPosition, wxDefaultSize, 0);
	okCancelBox->Add(ok, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	
	wxButton* cancel = new wxButton (pPanel, wxID_CANCEL,
		wxT("&Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	okCancelBox->Add(cancel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	topSizer->Add(okCancelBox, 0, wxALIGN_RIGHT | wxALL, 5);

	pPanel->SetAutoLayout(true);
					   	
	pPanel->SetSizer(topSizer);
	topSizer->SetSizeHints( this );
	Centre();

	//Set the icon
	wxImage toolIcon;
	toolIcon.LoadFile(wxT("./icons/script.png"));
	wxIcon frameIcon;
	wxBitmap frameIconBmp(toolIcon);
    frameIcon.CopyFromBitmap(frameIconBmp);
	SetIcon(frameIcon);
}

void C_DiffViewer::OnAcceptAll(wxCommandEvent& event)
{
	C_ComparisonTableBase* pBase =  (C_ComparisonTableBase*)m_pCompTable->GetTable();
	for(int i = 0; i < pBase->GetNumberRows(); i++)
	{
		pBase->Accept(i, true);
	}
	m_pCompTable->ForceRefresh();
}

void C_DiffViewer::OnReset(wxCommandEvent& event)
{
	C_ComparisonTableBase* pBase =  (C_ComparisonTableBase*)m_pCompTable->GetTable();
	for(int i = 0; i < pBase->GetNumberRows(); i++)
	{
		pBase->Accept(i, false);
	}
	m_pCompTable->ForceRefresh();
}

void C_DiffViewer::OnOk(wxCommandEvent& event)
{
	m_returnCode = wxID_OK;
	if(m_pModalLoop)
		m_pModalLoop->Exit();
}

void C_DiffViewer::OnCancel(wxCommandEvent& event)
{
	m_returnCode = wxID_CANCEL;
	if(m_pModalLoop)
		m_pModalLoop->Exit();
}

void C_DiffViewer::OnClose(wxCloseEvent& event)
{
	m_returnCode = wxID_CLOSE;
	if(m_pModalLoop)
		m_pModalLoop->Exit();
}

std::vector< C_AlignedData > C_DiffViewer::GetAlignedData()
{
	C_ComparisonTableBase* pBase = (C_ComparisonTableBase*)m_pCompTable->GetTable();
	return pBase->m_alignedData;
}

bool C_DiffViewer::IsDifferent()
{
	return NumOfDifferences() > 0;
}

int C_DiffViewer::NumOfDifferences()
{
	int retVal = 0;
	std::vector< C_AlignedData > ad = GetAlignedData();
	for(size_t i = 0; i < ad.size(); ++i)
	{
		if(ad[i].status != C_AlignedData::Status_NoChange)
			retVal++;
	}
	return retVal;
}

bool C_DiffViewer::AnyMatch()
{
	return GetAlignedData().size() > 0;
}

BEGIN_EVENT_TABLE(C_ComparisonTable, wxGrid)
	EVT_GRID_CELL_LEFT_CLICK(C_ComparisonTable::OnCellLeftClick)
	EVT_GRID_CELL_LEFT_DCLICK(C_ComparisonTable::OnCellLeftDClick)
END_EVENT_TABLE()

C_ComparisonTable::C_ComparisonTable(
				const std::vector< wxString >& fieldLabels,
				const std::vector< std::vector< wxString > >& leftData,
				const std::vector< std::vector< wxString > >& rightData,
				const std::vector< std::vector< wxString > >& leftDataToCompare,
				const std::vector< std::vector< wxString > >& rightDataToCompare,
				wxWindow* parent, C_Reader& reader, const wxWindowID id,
				const wxPoint& pos, const wxSize& size,
				long style) :
	wxGrid(parent, id, pos, size, style),
	m_reader(reader),
	m_prevCoords(-1, -1)
{
	SetRowLabelSize(60);
	SetCellHighlightPenWidth(0);
	DisableDragRowSize();
	
	C_ComparisonTableBase* tableBase = new C_ComparisonTableBase(
		fieldLabels, leftData, rightData, leftDataToCompare, rightDataToCompare);
	
	SetTable(tableBase, true);

	for(int i = 0; i < GetNumberRows(); i++) 
	{
		//Change color of the middle "Accept?" column
		SetCellBackgroundColour(i, fieldLabels.size(), wxColour(255, 255, 128));
		SetCellAlignment(i, fieldLabels.size(), wxALIGN_CENTRE, wxALIGN_CENTRE);

		//Do not display the checkbox on unchanged rows
		if(tableBase->m_alignedData[i].status == C_AlignedData::Status_NoChange)
		{
			SetCellEditor(i, fieldLabels.size(), new wxGridCellTextEditor());
			SetCellRenderer(i, fieldLabels.size(), new wxGridCellStringRenderer());
		} 
		else
		{
			SetCellEditor(i, fieldLabels.size(), new C_DiffCellBoolEditor());
			SetCellRenderer(i, fieldLabels.size(), new wxGridCellBoolRenderer());
		}

		//Change colors of each row depending on status
		for(int j = 0; j < GetNumberCols(); j++)
		{
			if(j == (int)fieldLabels.size()) //skip middle column
				continue;
			
			if(tableBase->m_alignedData[i].status == C_AlignedData::Status_Added)
			{
				SetCellBackgroundColour(i, j, wxColour(128, 255, 128));
			} 
			else if(tableBase->m_alignedData[i].status == C_AlignedData::Status_Deleted)
			{
				SetCellBackgroundColour(i, j, wxColour(255, 128, 128));
			} 
			else if(tableBase->m_alignedData[i].status == C_AlignedData::Status_Changed)
			{
				SetCellBackgroundColour(i, j, wxColour(128, 255, 255));

				if(j < (int)fieldLabels.size() &&
					IsColComparable(fieldLabels[j]) &&
					tableBase->GetValue(i, j) != tableBase->GetValue(i, j + fieldLabels.size() + 1))
				{
					SetCellTextColour(i, j, wxColour(255, 60, 60));
					SetCellTextColour(i, j + fieldLabels.size() + 1, wxColour(255, 60, 60));
				}

			}

		}
	}
}

bool C_ComparisonTable::IsColComparable(const wxString& colLabel)
{
	C_ColumnSet* pColSet = wxGetApp().GetDbManager().GetTypeColumnSet(m_reader.GetItemType());
	if(pColSet)
	{
		U32 numCols = pColSet->GetNumColumns();
		for(U32 i = 0; i < numCols; ++i)
		{
			C_Column* pCol = pColSet->GetColumn(i);
			if(pCol && pCol->IsComparable() && pCol->GetLabel() == colLabel)
			{
				return true;
			}
		}
	}

	return false;
}

void C_ComparisonTable::ProcessGridCellMouseEvent( wxMouseEvent& event )
{
	wxGrid::ProcessGridCellMouseEvent(event);

	int x, y;
    wxPoint pos( event.GetPosition() );
    CalcUnscrolledPosition( pos.x, pos.y, &x, &y );

    wxGridCellCoords coords;
    XYToCell( x, y, coords );

	if(coords.GetRow() >=0 && coords.GetCol() >=0 && coords != m_prevCoords)
	{
		C_ComparisonTableBase* pBase = (C_ComparisonTableBase*)GetTable();
		if(pBase)
		{
			wxString tip = pBase->GetValue(coords.GetRow(), coords.GetCol());
			wxString tipMultiline;
			wxStringTokenizer tokenizer(tip, _T("."));
            
            while(tokenizer.HasMoreTokens())
            {
				wxString token = tokenizer.GetNextToken();
				token.Trim(true).Trim(false);
				tipMultiline.Append(token);
				
				if(tokenizer.HasMoreTokens() && token.Length() > 0)
					tipMultiline.Append(".\n");
            }
			//unset the previous tooltip so it can be broken in multilines
			GetGridWindow()->SetToolTip((wxToolTip *)NULL);
			//now set the current one
			GetGridWindow()->SetToolTip(tipMultiline);
		}
		m_prevCoords = coords;
	}
	event.Skip();
}

void C_ComparisonTable::OnCellLeftClick(wxGridEvent& event)
{
	//Nasty hack to be able to check the CheckBox without first enabling the cell
	int row = event.GetRow();
	int col = event.GetCol();

	SetCurrentCell(row, col);
	
	EnableCellEditControl();
	DisableCellEditControl();
	
	event.Veto();
}

void C_ComparisonTable::OnCellLeftDClick(wxGridEvent& event)
{
	//continuation of the nasty hack
	event.Veto();
}

C_ComparisonTableBase::C_ComparisonTableBase(
		const std::vector< wxString >& fieldLabels,
		const std::vector< std::vector< wxString > >& leftData,
		const std::vector< std::vector< wxString > >& rightData,
		const std::vector< std::vector< wxString > >& leftDataToCompare,
		const std::vector< std::vector< wxString > >& rightDataToCompare) :
	m_fieldLabels(fieldLabels), m_leftData(leftData), m_rightData(rightData)
{
	m_midCol = m_fieldLabels.size();

	std::vector< wxString > leftDataToCompareConcat;
	std::vector< wxString > rightDataToCompareConcat;
	
	//Concatenate Data
	for(size_t i = 0; i < leftDataToCompare.size(); i++)
	{
		wxString concat;
		for(size_t j = 0; j < leftDataToCompare[i].size(); j++)
		{

			concat += leftDataToCompare[i][j];
		}
		leftDataToCompareConcat.push_back(concat);
	}

	for(size_t i = 0; i < rightDataToCompare.size(); i++)
	{
		wxString concat;
		for(size_t j = 0; j < rightDataToCompare[i].size(); j++)
		{
			concat += rightDataToCompare[i][j];
		}
		rightDataToCompareConcat.push_back(concat);
	}

	//Compare Concatenated Data
	C_Diff diff(leftDataToCompareConcat, rightDataToCompareConcat);
	m_matchedLines = diff.Compare();
	AlignData();
}

void C_ComparisonTableBase::AlignData()
{
	std::vector< std::pair<int, int> > matchedLines = m_matchedLines;

	//only align if we have matching lines
	if(m_matchedLines.empty())
		return;

	C_Diff::MatchedLinesIt it;
	size_t leftIndex, rightIndex, prevLeftIndex, prevRightIndex;
	size_t addedLeft, addedRight;

	prevLeftIndex = -1;
	prevRightIndex = -1;

	for(it = matchedLines.begin(); it < matchedLines.end(); it++) 
	{
		leftIndex = it->first;
		rightIndex = it->second;

		addedLeft = leftIndex - prevLeftIndex - 1;
		addedRight = rightIndex - prevRightIndex - 1;

		if(addedLeft == 0 && addedRight > 0) //added on the right
		{
			//Push the right additions
			for(size_t i = prevRightIndex + 1; i < rightIndex; i++)
			{
				C_AlignedData ad;
				std::vector< wxString > empty;
				ad.leftIndex = -1;
				ad.rightIndex = i;
				ad.leftLine = empty;
				ad.rightLine = m_rightData[i];
				ad.status = C_AlignedData::Status_Added;
				m_alignedData.push_back(ad);
			}
		}
		else if(addedLeft > 0 && addedRight == 0) //added on the left (i.e. deleted on the right)
		{
			//Push the left additions
			for(size_t i = prevLeftIndex + 1; i < leftIndex; i++)
			{
				C_AlignedData ad;
				std::vector< wxString > empty;
				ad.leftIndex = i;
				ad.rightIndex = -1;
				ad.leftLine = m_leftData[i];
				ad.rightLine = empty;
				ad.status = C_AlignedData::Status_Deleted;
				m_alignedData.push_back(ad);
			}
		}
		else if(addedLeft > 0 && addedRight > 0) //added on the left and right (i.e. changed)
		{
			if(addedLeft < addedRight) //we also have some additions on the right
			{
				//assume changes happened at the beginnig of both blocks
				//and additions happened at the end of the right block

				//for the changes
				for(size_t i = 1; i <= addedLeft; i++)
				{
					C_AlignedData ad;
					std::vector< wxString > empty;
					ad.leftIndex = ( ((prevLeftIndex + i) == leftIndex) && (prevLeftIndex + i >= 0) ) ? -1 : prevLeftIndex + i;
					ad.rightIndex = ( ((prevRightIndex + i) == rightIndex) && (prevRightIndex + i >= 0) ) ? -1 : prevRightIndex + i;
					ad.leftLine = ( ((prevLeftIndex + i) == leftIndex) && (prevLeftIndex + i >= 0) ) ? empty : m_leftData[prevLeftIndex + i];
					ad.rightLine = ( ((prevRightIndex + i) == rightIndex) && (prevRightIndex + i >= 0) ) ? empty : m_rightData[prevRightIndex + i];
					ad.status = C_AlignedData::Status_Changed;
					m_alignedData.push_back(ad);
				}

				//for the additions
				for(size_t i = prevRightIndex + addedLeft + 1; i < rightIndex; i++)
				{
					C_AlignedData ad;
					std::vector< wxString > empty;
					ad.leftIndex = -1;
					ad.rightIndex = i;
					ad.leftLine = empty;
					ad.rightLine = m_rightData[i];
					ad.status = C_AlignedData::Status_Added;
					m_alignedData.push_back(ad);
				}

			}
			else if(addedLeft > addedRight) //we also have some additions on the left
			{
				//assume changes happened at the beginnig of both blocks
				//and additions happened at the end of the left block

				//for the changes
				for(size_t i = 1; i <= addedRight; i++)
				{
					C_AlignedData ad;
					std::vector< wxString > empty;
					ad.leftIndex = ( ((prevLeftIndex + i) == leftIndex) && (prevLeftIndex + i >= 0) ) ? -1 : prevLeftIndex + i;
					ad.rightIndex = ( ((prevRightIndex + i) == rightIndex) && (prevRightIndex + i >= 0) ) ? -1 : prevRightIndex + i;
					ad.leftLine = ( ((prevLeftIndex + i) == leftIndex) && (prevLeftIndex + i >= 0) ) ? empty : m_leftData[prevLeftIndex + i];
					ad.rightLine = ( ((prevRightIndex + i) == rightIndex) && (prevRightIndex + i >= 0) ) ? empty : m_rightData[prevRightIndex + i];
					ad.status = C_AlignedData::Status_Changed;
					m_alignedData.push_back(ad);
				}

				//for the additions (deletions from DB point of view)
				for(size_t i = prevLeftIndex + addedRight + 1; i < leftIndex; i++)
				{
					C_AlignedData ad;
					std::vector< wxString > empty;
					ad.leftIndex = i;
					ad.rightIndex = -1;
					ad.leftLine = m_leftData[i];
					ad.rightLine = empty;
					ad.status = C_AlignedData::Status_Deleted;
					m_alignedData.push_back(ad);
				}
			
			}
			else //addedLeft == addedRight (only changes)
			{
				//Push the changes
				for(size_t i = 1; i <= addedLeft; i++)
				{
					C_AlignedData ad;
					std::vector< wxString > empty;
					ad.leftIndex = ( ((prevLeftIndex + i) == leftIndex) && (prevLeftIndex + i >= 0) ) ? -1 : prevLeftIndex + i;
					ad.rightIndex = ( ((prevRightIndex + i) == rightIndex) && (prevRightIndex + i >= 0) ) ? -1 : prevRightIndex + i;
					ad.leftLine = ( ((prevLeftIndex + i) == leftIndex) && (prevLeftIndex + i >= 0) ) ? empty : m_leftData[prevLeftIndex + i];
					ad.rightLine = ( ((prevRightIndex + i) == rightIndex) && (prevRightIndex + i >= 0) ) ? empty : m_rightData[prevRightIndex + i];
					ad.status = C_AlignedData::Status_Changed;
					m_alignedData.push_back(ad);
				}
			}
		}		

		//Push the current match (no change)
		C_AlignedData ad;
		ad.leftIndex = leftIndex;
		ad.rightIndex = rightIndex;
		ad.leftLine = m_leftData[leftIndex];
		ad.rightLine = m_rightData[rightIndex];
		ad.status = C_AlignedData::Status_NoChange;
		m_alignedData.push_back(ad);

		prevLeftIndex = leftIndex;
		prevRightIndex = rightIndex;
	}

	//Are there any rows added/deleted changed at the end?
	if( (leftIndex < m_leftData.size() - 1) ||
		(rightIndex < m_rightData.size() - 1) )
	{
		addedLeft = m_leftData.size() - leftIndex - 1;
		addedRight = m_rightData.size() - rightIndex - 1;

		if(addedLeft == 0 && addedRight > 0) //added on the right
		{
			//Push the right additions
			for(size_t i = rightIndex + 1; i < m_rightData.size(); i++)
			{
				C_AlignedData ad;
				std::vector< wxString > empty;
				ad.leftIndex = -1;
				ad.rightIndex = i;
				ad.leftLine = empty;
				ad.rightLine = m_rightData[i];
				ad.status = C_AlignedData::Status_Added;
				m_alignedData.push_back(ad);
			}
		}
		else if(addedLeft > 0 && addedRight == 0) //added on the left (i.e. deleted on the right)
		{
			//Push the left additions
			for(size_t i = leftIndex + 1; i < m_leftData.size(); i++)
			{
				C_AlignedData ad;
				std::vector< wxString > empty;
				ad.leftIndex = i;
				ad.rightIndex = -1;
				ad.leftLine = m_leftData[i];
				ad.rightLine = empty;
				ad.status = C_AlignedData::Status_Deleted;
				m_alignedData.push_back(ad);
			}
		}
		else if(addedLeft > 0 && addedRight > 0) //added on the left and right (i.e. changed)
		{
			if(addedLeft < addedRight) //we also have some additions on the right
			{
				//assume changes happened at the beginnig of both blocks
				//and additions happened at the end of the right block

				//for the changes
				for(size_t i = 1; i <= addedLeft; i++)
				{
					C_AlignedData ad;
					std::vector< wxString > empty;
					ad.leftIndex = ((leftIndex + i) < m_leftData.size()) ? leftIndex + i : -1;
					ad.rightIndex = ((rightIndex + i) < m_rightData.size()) ? rightIndex + i : -1;
					ad.leftLine = ((leftIndex + i) < m_leftData.size()) ? m_leftData[leftIndex + i] : empty;
					ad.rightLine = ((rightIndex + i) < m_rightData.size()) ? m_rightData[rightIndex + i] : empty;
					ad.status = C_AlignedData::Status_Changed;
					m_alignedData.push_back(ad);
				}

				//for the additions
				for(size_t i = rightIndex + addedLeft + 1; i < m_rightData.size(); i++)
				{
					C_AlignedData ad;
					std::vector< wxString > empty;
					ad.leftIndex = -1;
					ad.rightIndex = i;
					ad.leftLine = empty;
					ad.rightLine = m_rightData[i];
					ad.status = C_AlignedData::Status_Added;
					m_alignedData.push_back(ad);
				
				}
			}
			else if(addedLeft > addedRight) //we also have some additions on the left
			{
				//assume changes happened at the beginnig of both blocks
				//and additions happened at the end of the left block

				//for the changes
				for(size_t i = 1; i <= addedRight; i++)
				{
					C_AlignedData ad;
					std::vector< wxString > empty;
					ad.leftIndex = ((leftIndex + i) < m_leftData.size()) ? leftIndex + i : -1;
					ad.rightIndex = ((rightIndex + i) < m_rightData.size()) ? rightIndex + i : -1;
					ad.leftLine = ((leftIndex + i) < m_leftData.size()) ? m_leftData[leftIndex + i] : empty;
					ad.rightLine = ((rightIndex + i) < m_rightData.size()) ? m_rightData[rightIndex + i] : empty;
					ad.status = C_AlignedData::Status_Changed;
					m_alignedData.push_back(ad);
				}

				//for the additions (deletions from DB point of view)
				for(size_t i = leftIndex + addedRight + 1; i < m_leftData.size(); i++)
				{
					C_AlignedData ad;
					std::vector< wxString > empty;
					ad.leftIndex = i;
					ad.rightIndex = -1;
					ad.leftLine = m_leftData[i];
					ad.rightLine = empty;
					ad.status = C_AlignedData::Status_Deleted;
					m_alignedData.push_back(ad);
				}
			}
			else //addedLeft == addedRight (only changes)
			{
				//Push the changes
				for(size_t i = 1; i <= addedLeft; i++)
				{
					C_AlignedData ad;
					std::vector< wxString > empty;
					ad.leftIndex = ((leftIndex + i) < m_leftData.size()) ? leftIndex + i : -1;
					ad.rightIndex = ((rightIndex + i) < m_rightData.size()) ? rightIndex + i : -1;
					ad.leftLine = ((leftIndex + i) < m_leftData.size()) ? m_leftData[leftIndex + i] : empty;
					ad.rightLine = ((rightIndex + i) < m_rightData.size()) ? m_rightData[rightIndex + i] : empty;
					ad.status = C_AlignedData::Status_Changed;
					m_alignedData.push_back(ad);
				}
			}
		}
	}

	//state that no change has been accepted yet
	for(size_t i = 0; i < m_alignedData.size(); ++i)
	{
		m_alignedData[i].accepted = false;
	}
}

int C_ComparisonTableBase::GetNumberCols()
{
	int ret = m_fieldLabels.size()*2 + 1;
	return ret;
}

int C_ComparisonTableBase::GetNumberRows()
{
	int ret = m_alignedData.size();
	return ret;
}

wxString C_ComparisonTableBase::GetValue(int row, int col)
{
	if(col < m_midCol)
	{
		return (m_alignedData[row].leftLine.size() > 0) ? m_alignedData[row].leftLine[col] : wxEmptyString;
	}
	else if(col > m_midCol)
	{
		return (m_alignedData[row].rightLine.size() > 0) ? m_alignedData[row].rightLine[col - m_midCol - 1] : wxEmptyString;
	}
	else
	{
		return wxEmptyString;
	}
}

void C_ComparisonTableBase::SetValue(int row, int col, const wxString& value)
{
}

bool C_ComparisonTableBase::IsEmptyCell(int row, int col)
{
	return false;
}

wxString C_ComparisonTableBase::GetTypeName(int row, int col)
{
	if ( col == m_midCol ) //middle column
	{
		return wxGRID_VALUE_BOOL;
	}
	else
	{
		return wxString::Format(_T("%s:80"), wxGRID_VALUE_STRING);
	}
}

bool C_ComparisonTableBase::CanGetValueAs(int row, int col, const wxString& typeName)
{
	if ( typeName == wxGRID_VALUE_STRING )
	{
		return true;
	}
	else if ( typeName == wxGRID_VALUE_BOOL )
	{
		return col == m_midCol;
	}
	else
	{
		return false;
	}
}

bool C_ComparisonTableBase::CanSetValueAs(int row, int col, const wxString& typeName)
{
	return CanGetValueAs(row, col, typeName);
}

bool C_ComparisonTableBase::GetValueAsBool(int row, int col)
{
	if(col == m_midCol) //middle
	{
		return m_alignedData[row].accepted;
	}
	else
	{
		return false;
	}
}

void C_ComparisonTableBase::SetValueAsBool(int row, int col, bool value)
{
	if(col == m_midCol) //middle
	{
		m_alignedData[row].accepted = value;
	}
}

wxString C_ComparisonTableBase::GetColLabelValue(int col)
{
	if(col < m_midCol)
		return m_fieldLabels[col];
	else if(col == m_midCol)
		return wxT("Accept?");
	else
		return m_fieldLabels[col - m_midCol - 1];
}

void C_ComparisonTableBase::Accept(int row, bool value)
{
	if(value && !m_alignedData[row].accepted)
	{
		m_alignedData[row].leftLineBackup = m_alignedData[row].leftLine;
		m_alignedData[row].leftLine = m_alignedData[row].rightLine;
		m_alignedData[row].accepted = true;
		return;
	}

	if(!value && m_alignedData[row].accepted)
	{
		m_alignedData[row].leftLine = m_alignedData[row].leftLineBackup;
		m_alignedData[row].accepted = false;
		return;
	}
}

void C_DiffCellBoolEditor::BeginEdit(int row, int col, wxGrid* grid)
{
	wxGridCellBoolEditor::BeginEdit(row, col, grid);
	bool accepted = !(CBox()->GetValue());
	if(accepted)
	{
		((C_ComparisonTableBase*)grid->GetTable())->Accept(row, true);
	}
	else
	{
		((C_ComparisonTableBase*)grid->GetTable())->Accept(row, false);
	}
	EndEdit(row, col, grid);
	grid->ForceRefresh();
}

} //namespace DMan