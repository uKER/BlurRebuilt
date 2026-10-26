#include "stdafx.h"
#include <DialogueMan/Components/DataTable/ColumnEditor.h>

namespace DMan
{

BEGIN_EVENT_TABLE(C_ColumnEditor, wxDialog)
	EVT_GRID_EDITOR_SHOWN(C_ColumnEditor::OnEditorShown)
	EVT_GRID_CELL_CHANGE(C_ColumnEditor::OnCellValueChanged)
END_EVENT_TABLE()

C_ColumnEditor::C_ColumnEditor(wxWindow *parent)
:
wxDialog(
	parent,
	wxID_ANY,
	wxString(wxT("Column Editor")))
{
	/*
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Select the column(s) to display")),
				   0,
				   wxALL,
				   10);

	m_pGrid = new wxGrid(this,
                       wxID_ANY);

	int numDataTableCols = C_ColumnSet::I(m_type).GetNumberCols();
	for(int i = 0; i < numDataTableCols; ++i)
	{
		m_allAndHideMap[i] = i;
	}
	

	m_pGrid->CreateGrid(numDataTableCols, 2);
	m_pGrid->SetColLabelValue(0, wxT("Column Label"));
	m_pGrid->SetColLabelValue(1, wxT("Display?"));
	m_pGrid->SetRowLabelSize(0);

	//set the grid Col 0 values
	for(int row = 0; row < numDataTableCols; ++row)
	{
		C_Column* pCol = C_ColumnSet::I(m_type).GetColumn(row);
		if(pCol)
		{
			m_pGrid->SetCellValue(row, 0, pCol->GetLabel());	
			m_pGrid->SetReadOnly(row, 0, true);
		}
	}

	//set the grid Col 1 values
	for(int row = 0; row < numDataTableCols; ++row)
	{
		C_Column* pCol = C_ColumnSet::I(m_type).GetColumn(row);
		if(pCol)
		{
			m_pGrid->SetCellRenderer(row, 1, new wxGridCellBoolRenderer);
			m_pGrid->SetCellEditor(row, 1, new wxGridCellBoolEditor);
			m_pGrid->SetCellValue(row, 1, pCol->IsHidden() ? wxT("0") : wxT("1"));
			m_pGrid->SetCellAlignment(row, 1, wxALIGN_CENTRE, wxALIGN_CENTRE);
		}
	}

	m_pGrid->Fit();

	mainSizer->Add(m_pGrid,
				   0,
				   wxALL,
				   10);

	wxButton* pOkBtn = new wxButton(this, wxID_CLOSE, wxT("Close"));

	mainSizer->Add(pOkBtn,
				   0,
				   wxALL | wxALIGN_CENTER,
				   10);

    SetSizer(mainSizer);
    mainSizer->SetSizeHints(this);
    mainSizer->Fit(this);
	*/
}

void C_ColumnEditor::OnEditorShown(wxGridEvent& event)
{
	int row = event.GetRow();
	int col = event.GetCol();
}

void C_ColumnEditor::OnCellValueChanged(wxGridEvent& event)
{
	int row = event.GetRow();
	int col = event.GetCol();
	
/*
	C_Column* pCol = C_ColumnSet::I(m_type).GetColumn(row);
	bool display = m_pGrid->GetCellValue(row, col) == wxT("1") ? true : false;	
	wxGrid* pParent = (wxGrid*)GetParent();

	if(display)
	{
		pCol->SetHidden(false);

		//reduce the index after the displayed column
		for(size_t i = 0; i < m_allAndHideMap.size(); ++i)
		{
			
		}

		wxGridTableMessage msg( pParent->GetTable(),
							wxGRIDTABLE_NOTIFY_COLS_INSERTED,
                            row, 1 );
		pParent->ProcessTableMessage(msg);
	}
	else
	{
		pCol->SetHidden(true);
		wxGridTableMessage msg( pParent->GetTable(),
                            wxGRIDTABLE_NOTIFY_COLS_DELETED,
                            row, 1 );
		pParent->ProcessTableMessage(msg);
	}
   
	pParent->Fit();
	wxGetApp().GetFrame()->GetAuiManager().Update();
	*/
}

}
