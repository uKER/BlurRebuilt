#include "stdafx.h"

#include <sigslot/sigslot.h>

#include <DialogueMan/Components/DataTable/DataGrid.h>
#include <DialogueMan/Components/DataTable/DataGridPanel.h>
#include <DialogueMan/Components/DataTable/DataGridFilter.h>

#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/ColumnSet.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/GlobalSignals.h>

namespace DMan
{

IMPLEMENT_CLASS(C_DataGridPanel, wxPanel);

BEGIN_EVENT_TABLE(C_DataGridPanel,wxPanel)
END_EVENT_TABLE()

C_DataGridPanel::C_DataGridPanel(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
:
wxPanel(parent, id, pos, size, style),
m_slots(this)
{
	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* pSizer1;
	pSizer1 = new wxBoxSizer( wxHORIZONTAL );

	m_pFilterText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	pSizer1->Add( m_pFilterText, 1, wxALL|wxEXPAND, 0 );

	m_pFilterColumn = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_DROPDOWN|wxCB_READONLY);

	pSizer1->Add( m_pFilterColumn, 1, wxALL|wxEXPAND, 0 );

	pSizer0->Add( pSizer1, 0, wxALL|wxEXPAND, 0 );

	m_pGrid = new C_DataGrid(
			this,
			wxID_ANY,
			wxDefaultPosition,
			wxDefaultSize,
			wxHSCROLL|wxVSCROLL);
	
	pSizer0->Add( m_pGrid, 1, wxALL|wxEXPAND, 0 );

	SetSizer(pSizer0);
	Layout();

	Connect(
		m_pFilterText->GetId(),
		wxEVT_COMMAND_TEXT_UPDATED,
		wxTextEventHandler(C_DataGridPanel::OnFilterTextUpdate));

	Connect(
		m_pFilterColumn->GetId(),
		wxEVT_COMMAND_TEXT_UPDATED,
		wxTextEventHandler(C_DataGridPanel::OnFilterColumnUpdate));

	m_slots.Connect();
}

void C_DataGridPanel::Shutdown()
{
	m_slots.Disconnect();
	m_pGrid->Shutdown();
}

void C_DataGridPanel::OnSelectionChanged()
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetMode() != C_Selection::E_Mode_Pack && selection.GetMode() != C_Selection::E_Mode_Record)
		return;

	const C_Selection::ItemArray& arr = selection.GetItems();
	if (arr.Size() == 0)
	{
		UpdateColumnFilters(0);
		return;
	}

	const C_Selection::C_Item& item = arr.Back();

	C_Record* pRecord = 
		wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

	if (!pRecord)
	{
		UpdateColumnFilters(0);
		return;
	}

	//Create new base and assign ownership.
	C_RowSet* pRowSet = pRecord->GetRowSet(item.id.GetChildId());
	if (!pRowSet)
	{
		UpdateColumnFilters(0);
		return;
	}

	UpdateColumnFilters(pRowSet->GetAllColumnSet());
}

void C_DataGridPanel::UpdateColumnFilters(C_ColumnSet* columnSet)
{
	m_pColumnSet = columnSet;

	m_pFilterColumn->Clear();
	m_pFilterColumn->Append("All Columns");

	if (!columnSet)
		return;

	for (U32 i=0; i<columnSet->GetNumColumns(); i++)
	{
		C_Column* column = columnSet->GetColumn(i);
		m_pFilterColumn->Append(column->GetLabel());
	}
	m_pFilterColumn->SetSelection(0);
}

void C_DataGridPanel::OnFilterColumnUpdate(wxCommandEvent& e)
{
	if (m_pColumnSet)
	{
		int columnId = m_pFilterColumn->GetCurrentSelection();
		C_Column* column = 0;
		if (columnId > 0)
			column = m_pColumnSet->GetColumn(columnId-1);
		m_pGrid->GetFilter()->SetFilterColumn(column);
	}
	m_pGrid->SetDirty();
}

void C_DataGridPanel::OnFilterTextUpdate(wxCommandEvent& e)
{
	wxString filterValue = m_pFilterText->GetValue();
	const wxCharBuffer charBuffer = filterValue.ToUTF8();
	const char* filterText = charBuffer;
	m_pGrid->GetFilter()->SetTextFilter(filterText);
	m_pGrid->SetDirty();
}

C_DataGridPanel::C_Slots::C_Slots(C_DataGridPanel* pDataGridPanel)
:
m_pDataGridPanel(pDataGridPanel)
{
}
	
void C_DataGridPanel::C_Slots::Connect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.appSelectionChanged.connect(this, &C_DataGridPanel::C_Slots::OnSelectionChangedSlot);
}
		
void C_DataGridPanel::C_Slots::Disconnect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.appSelectionChanged.disconnect(this);
}

void C_DataGridPanel::C_Slots::OnSelectionChangedSlot()
{
	m_pDataGridPanel->OnSelectionChanged();
}

}