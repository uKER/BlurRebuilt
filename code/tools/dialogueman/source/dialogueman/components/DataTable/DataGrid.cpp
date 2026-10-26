#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <DialogueMan/Components/DataTable/DataGridFilter.h>
#include <DialogueMan/Components/DataTable/DataGridBase.h>
#include <DialogueMan/Components/DataTable/DataGrid.h>

#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>

#include <DialogueMan/Actions/DeleteRows.h>

#include <DialogueMan/GlobalSignals.h>
#include <DialogueMan/App.h>
#include <wx/tokenzr.h>

namespace DMan
{

BEGIN_EVENT_TABLE(C_DataGrid, wxGrid)
	EVT_GRID_LABEL_RIGHT_CLICK(C_DataGrid::OnLabelRightClick)
	EVT_GRID_CELL_LEFT_CLICK(C_DataGrid::OnCellLeftClick)
	EVT_GRID_SELECT_CELL(C_DataGrid::OnSelectCell)
	EVT_GRID_RANGE_SELECT(C_DataGrid::OnRangeSelect)
	EVT_KEY_DOWN(C_DataGrid::OnKeyDown)
END_EVENT_TABLE()

C_DataGrid::C_DataGrid(
	wxWindow *parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
	:
wxGrid(
	parent,
	id,
	pos, 
	size,
	style,
	name),
	m_slots(this),
	m_colLabelClicked(-1),
	m_prevCoords(-1, -1),
	m_isSync(false)
{
	EnableEditing(false);
	SetRowLabelSize(60);
	SetCellHighlightPenWidth(0);
	DisableDragRowSize();
	m_flags.dirty = false;
	m_flags.enabled = true;
	m_slots.Connect();
	m_pFilter = new C_DataGridFilter();
}

C_DataGrid::~C_DataGrid()
{
	delete m_pFilter;
}

void C_DataGrid::Shutdown()
{
	C_DataGridBase* pBase = (C_DataGridBase*)GetTable();
	if (pBase)
		pBase->Shutdown();

	m_slots.Disconnect();
}

C_DataGridFilter* C_DataGrid::GetFilter()
{
	return m_pFilter;
}

void C_DataGrid::OnLabelRightClick(wxGridEvent& event)
{
    wxPoint pt = event.GetPosition();
	int col = event.GetCol();
	int row = event.GetRow();
 
    wxMenu menu;

	if(row == -1) //Right-clicked on a Col Label
	{
		m_colLabelClicked = col;
		menu.Append(ID_NO_SORT, wxString::Format(wxT("No Sorting")));
		menu.AppendSeparator();
		menu.Append(ID_SORT_ASC, wxT("Sort Ascending"));
		menu.Append(ID_SORT_DESC, wxT("Sort Descending"));	
/*		menu.AppendSeparator();
		menu.Append(ID_DISPLAY_COLUMN, wxT("Display Column..."));
		menu.Append(ID_HIDE, wxT("Hide"));
		menu.Append(wxID_ANY, wxT("Clear Content"));
		menu.AppendSeparator();
		menu.Append(wxID_ANY, wxT("Select Column"));*/
	}
	
	Connect(ID_SORT_ASC, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_DataGrid::OnSortByColumn));
	Connect(ID_SORT_DESC, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_DataGrid::OnSortByColumn));
	Connect(ID_NO_SORT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_DataGrid::OnSortByColumn));
	
    PopupMenu(&menu, pt);

	Disconnect(ID_SORT_ASC, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(ID_SORT_DESC, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(ID_NO_SORT, wxEVT_COMMAND_MENU_SELECTED);
	
    //event.Skip();
}

// For selection using the mouse
void C_DataGrid::OnCellLeftClick(wxGridEvent& event)
{
	SelectCell(event);
}
 
//For selection using the keyboard
void C_DataGrid::OnSelectCell(wxGridEvent& event)
{
	SelectCell(event);
}

void C_DataGrid::OnRangeSelect(wxGridRangeSelectEvent& event)
{
	//the event is also fired when wxGrid cleans its selection so don't continue in that case
	if(!event.Selecting() || m_isSync || event.ControlDown())
	{
		event.Skip();
		return;
	}

	wxGridCellCoords topLeft = event.GetTopLeftCoords();
	wxGridCellCoords bottomRight = event.GetBottomRightCoords();

	C_DataGridBase* pBase = (C_DataGridBase*)GetTable();
	if (!pBase) return;

	C_Selection selection;
	wxGetApp().GetSelection(selection);
	if (selection.GetItems().Size() == 0)
	{
		assert(!"Must have valid selection!");
		return;
	}

	m_selectedRows.Clear();
	Btl::Vector<U32> components;
	for(int i = topLeft.GetRow(); i <= bottomRight.GetRow(); i++)
	{
		m_selectedRows.push_back(i);
		C_Row* pRow = pBase->GetRow(i);
		components.PushBack(pRow->GetXtArrayIndex());
	}
	
	C_Selection::C_Item item = selection.GetItems().Back();
	selection.Clear();
	selection.Add(C_Selection::E_Mode_Row, item.id, components);
	wxGetApp().SetSelection(selection);
	ForceRefresh();

	event.Skip();
}

void C_DataGrid::OnKeyDown(wxKeyEvent& event)
{
	switch(event.GetKeyCode())
	{
		case WXK_DELETE:
		{
			DeleteRows::Do();
			break;
		}
		default:
			return event.Skip();
	};
}

void C_DataGrid::SyncSelection()
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetMode() != C_Selection::E_Mode_Row)
		return;

	if (selection.GetItems().Size() == 0)
		return;

	C_Selection::C_Item item = selection.GetItems().Back();

	C_Record* pRecord = 
		wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

	if (!pRecord)
		return;

	if (!item.components.Size())
		return;

	C_DataGridBase* pBase = (C_DataGridBase*)GetTable();
	if (!pBase)
		return;

	m_isSync = true;

	int lastRow = 0;

	for(int i = 0; i < item.components.Size(); i++)
	{
		for (int j = 0; j < GetNumberRows(); j++)
		{
			if (pBase->GetRow(j)->GetXtArrayIndex() == item.components[i])
			{
				if(item.components.Size() == 1)
					SelectRow(j, false);
				else
					SelectRow(j, true);
				
				lastRow = j;
			}
		}
	}

	m_isSync = false;
}

void C_DataGrid::SelectCell(wxGridEvent& event)
{	
	int row = event.GetRow();
	int col = event.GetCol();
						 
	C_DataGridBase* pBase = (C_DataGridBase*)GetTable();
	if (!pBase) return;

	//Get the current selection
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetItems().Size() == 0)
	{
		assert(!"Must have valid selection!");
		return;
	}

	C_Selection::C_Item item = selection.GetItems().Back();
	selection.Clear();

	if(!event.ControlDown())
	{
		m_selectedRows.Clear();
	}

	int foundAt = m_selectedRows.Index(row);
	if(foundAt == wxNOT_FOUND )
		m_selectedRows.push_back(row);
	else
		m_selectedRows.RemoveAt(foundAt);	

	Btl::Vector<U32> components;
	for (U32 i = 0; i < m_selectedRows.size(); ++i)
	{
		C_Row* pRow = pBase->GetRow(m_selectedRows[i]);
		components.PushBack(pRow->GetXtArrayIndex());
	}
	selection.Add(C_Selection::E_Mode_Row, item.id, components);
		
	wxGetApp().SetSelection(selection);

	ForceRefresh();
		
	event.Skip();
}

const C_DataGrid::C_Flags& C_DataGrid::GetFlags() const
{
	return m_flags;
}

C_DataGrid::C_Flags& C_DataGrid::GetFlags()
{
	return m_flags;
}

void C_DataGrid::OnDbClose()
{
	SetTable(0);
	SetDirty();
}

void C_DataGrid::OnSelectionChanged()
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetMode() != C_Selection::E_Mode_Pack && selection.GetMode() != C_Selection::E_Mode_Record)
		return;

	const C_Selection::ItemArray& arr = selection.GetItems();
	if (arr.Size() == 0)
	{
		Hide();
		SetTable(0);
		SetDirty();
		return;
	}

	const C_Selection::C_Item& item = arr.Back();

	C_Record* pRecord = 
		wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

	if (!pRecord)
	{
		Hide();
		SetTable(0);
		SetDirty();
		return;
	}

	//Create new base and assign ownership.
	C_RowSet* pRowSet = pRecord->GetRowSet(item.id.GetChildId());
	if (!pRowSet)
	{
		Hide();
		SetTable(0);
		SetDirty();
		return;
	}
	
	C_DataGridBase* pBase = new C_DataGridBase(pRowSet->GetAllColumnSet(), pRowSet, m_pFilter);
	
	SetTable(pBase, true, wxGrid::wxGridSelectRows);
	if(GetNumberRows() > 0)
		SelectRow(0, true);

	for (int i=0; i<pBase->GetNumberCols(); i++)
	{
		C_ColumnSet* columnSet = pBase->GetColumnSet();
		int width = pBase->GetColumnSet()->GetColumn(i)->GetWidth();
		if (width != -1)
			SetColSize(i, width);
	}

	Show();
	SetDirty();
}

void C_DataGrid::OnRefreshDirty()
{
	if (!m_flags.dirty || !m_flags.enabled)
		return;

	if (GetTable() && ((C_DataGridBase*)GetTable())->GetFlags().dirty)
	{
		((C_DataGridBase*)GetTable())->OnRefreshDirty();
	}

	SyncSelection();

	ForceRefresh();

	m_flags.dirty = false;
}

void C_DataGrid::OnViewsEnabled(bool enable)
{
	m_flags.enabled = enable;
}

void C_DataGrid::SetDirty()
{
	m_flags.dirty = true;
	if (GetTable())
		((C_DataGridBase*)GetTable())->GetFlags().dirty = true;
}

void C_DataGrid::OnSortByColumn(wxCommandEvent& event)
{
	if (event.GetId() == ID_SORT_ASC || event.GetId() == ID_NO_SORT)
		m_pFilter->SetSortOrder(C_DataGridFilter::E_SortOrder_AlphaAsc);
	else if (event.GetId() == ID_SORT_DESC)
		m_pFilter->SetSortOrder(C_DataGridFilter::E_SortOrder_AlphaDesc);

	if (GetTable() && event.GetId() != ID_NO_SORT)
	{
		C_Column* column = ((C_DataGridBase*)GetTable())->GetColumnSet()->GetColumn(m_colLabelClicked);
		m_pFilter->SetSortColumn(column);
	}
	else
	{
		m_pFilter->SetSortColumn(0);
	}

	SetDirty();
}

//Overriding to show tooltip
void C_DataGrid::ProcessGridCellMouseEvent( wxMouseEvent& event )
{
	wxGrid::ProcessGridCellMouseEvent(event);

	int x, y;
    wxPoint pos( event.GetPosition() );
    CalcUnscrolledPosition( pos.x, pos.y, &x, &y );

    wxGridCellCoords coords;
    XYToCell( x, y, coords );

	if(coords.GetRow() >=0 && coords.GetCol() >=0 && coords != m_prevCoords)
	{
		C_DataGridBase* pBase = (C_DataGridBase*)GetTable();
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

C_DataGrid::C_Slots::C_Slots(C_DataGrid* pDataGrid)
:
m_pDataGrid(pDataGrid)
{
}
	
void C_DataGrid::C_Slots::Connect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.appSelectionChanged.connect(this, &C_DataGrid::C_Slots::OnSelectionChangedSlot);
	s.appRefreshDirty.connect(this, &C_DataGrid::C_Slots::OnAppRefreshDirtySlot);
	s.appDbClose.connect(this, &C_DataGrid::C_Slots::OnDbCloseSlot);

	s.rowDeserialised.connect(this, &C_DataGrid::C_Slots::OnRowDeserialisedSlot);

	s.rowSetRowAppended.connect(this, &C_DataGrid::C_Slots::OnRowSetRowAppendedSlot);
	s.rowSetRowReordered.connect(this, &C_DataGrid::C_Slots::OnRowSetRowReorderedSlot);
	s.rowSetRowDelete.connect(this, &C_DataGrid::C_Slots::OnRowSetRowDeleteSlot);

	s.viewsEnabled.connect(this, &C_DataGrid::C_Slots::OnViewsEnabledSlot);
}
		
void C_DataGrid::C_Slots::Disconnect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.appSelectionChanged.disconnect(this);
	s.appRefreshDirty.disconnect(this);
	s.appDbClose.disconnect(this);

	s.rowDeserialised.disconnect(this);

	s.rowSetRowAppended.disconnect(this);
	s.rowSetRowReordered.disconnect(this);
	s.rowSetRowDelete.disconnect(this);
}

void C_DataGrid::C_Slots::OnDbCloseSlot()
{
	m_pDataGrid->OnDbClose();
}

void C_DataGrid::C_Slots::OnSelectionChangedSlot()
{
	m_pDataGrid->OnSelectionChanged();
}

void C_DataGrid::C_Slots::OnRowDeserialisedSlot(C_Row* pRow)
{
	m_pDataGrid->SetDirty();
}

void C_DataGrid::C_Slots::OnRowSetRowAppendedSlot(C_RowSet* pRowSet, C_Row* pRow)
{
	m_pDataGrid->SetDirty();
}

void C_DataGrid::C_Slots::OnRowSetRowReorderedSlot(C_RowSet* pRowSet, C_Row* pRow)
{
	m_pDataGrid->SetDirty();
}

void C_DataGrid::C_Slots::OnRowSetRowDeleteSlot(C_RowSet* pRowSet, C_Row* pRow)
{
	m_pDataGrid->SetDirty();
}

void C_DataGrid::C_Slots::OnAppRefreshDirtySlot()
{
	m_pDataGrid->OnRefreshDirty();
}

void C_DataGrid::C_Slots::OnViewsEnabledSlot(bool enable)
{
	m_pDataGrid->OnViewsEnabled(enable);
}

}
