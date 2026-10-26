#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <DialogueMan/Components/DataTable/DataGrid.h>
#include <DialogueMan/Components/DataTable/DataGridFilter.h>
#include <DialogueMan/Components/DataTable/DataGridBase.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/GlobalSignals.h>
#include <DialogueMan/Frame/Frame.h>
#include <DialogueMan/App.h>

namespace DMan
{

C_DataGridBase::C_SortFilterPredicate::C_SortFilterPredicate(C_DataGridFilter* pDataGridFilter)
{
	m_pDataGridFilter = pDataGridFilter;
}

bool C_DataGridBase::C_SortFilterPredicate::operator()(C_Row* pLhs, C_Row* pRhs)
{
	if (m_pDataGridFilter->GetSortColumn() == 0)
	{
		if (m_pDataGridFilter->GetSortOrder() == C_DataGridFilter::E_SortOrder_AlphaAsc)
			return pLhs->GetXtArrayIndex() < pRhs->GetXtArrayIndex();
		else
			return pLhs->GetXtArrayIndex() > pRhs->GetXtArrayIndex();
	} else {
		wxString columnKey = m_pDataGridFilter->GetSortColumn()->GetKey();
		if (m_pDataGridFilter->GetSortOrder() == C_DataGridFilter::E_SortOrder_AlphaAsc)
			return (wxString)pLhs->GetColumns()[columnKey] < (wxString)pRhs->GetColumns()[columnKey];
		else
			return (wxString)pLhs->GetColumns()[columnKey] > (wxString)pRhs->GetColumns()[columnKey];
	}
}

template<typename T>
T GetValueAs(
	C_ColumnSet* pColumnSet, 
	Btl::Vector<C_Row*> rows,
	int row, 
	int col, 
	const T& defaultValue)
{
	C_Column* pColumn = pColumnSet->GetColumn(col);
	if (!pColumn) return defaultValue;

	if (row >= rows.Size())
		return defaultValue;
	
	C_Row* pRow = rows[row];
	assert(pRow);
	C_Row::ColumnMap::ConstIterator it = pRow->GetColumns().Find(pColumn->GetKey());
	if (it == pRow->GetColumns().End()) return defaultValue;

	return it->GetValue();
}

C_DataGridBase::C_DataGridBase(C_ColumnSet* pColumnSet, C_RowSet* pRowSet, C_DataGridFilter* pFilter)
:
m_slots(this)
{
	m_pColumnSet = pColumnSet;
	m_pRowSet = pRowSet;
	m_pFilter = pFilter;

	SetAttrProvider(new wxGridCellAttrProvider); 
	m_slots.Connect();

	//Populate filter
	m_filteredRows.Resize(pRowSet->GetNumRows());
	for (S32 i = 0; i < pRowSet->GetNumRows(); ++i)
		m_filteredRows[i] = pRowSet->GetRow(i);
	SortFilteredRows();

	m_Flags.shutdown = false;
	m_Flags.dirty = false;
	m_Flags.enabled = true;
}

C_DataGridBase::~C_DataGridBase()
{
	Shutdown();
}

void C_DataGridBase::Shutdown()
{
	if (m_Flags.shutdown)
		return;

	m_slots.Disconnect();
	m_Flags.shutdown = true;
}

C_DataGridBase::C_Flags& C_DataGridBase::GetFlags()
{
	return m_Flags;
}

const C_DataGridBase::C_Flags& C_DataGridBase::GetFlags() const
{
	return m_Flags;
}

void C_DataGridBase::OnRefreshDirty()
{
	if (!m_Flags.dirty || !m_Flags.enabled)
		return;

	FilterRows();
	SortFilteredRows();

	m_Flags.dirty = false;
}

int C_DataGridBase::GetNumberRows()
{
	return m_filteredRows.Size();
}

int C_DataGridBase::GetNumberCols()
{
	return (int)m_pColumnSet->GetNumColumns();
}

bool C_DataGridBase::IsEmptyCell(int row, int col)
{
	return false;
}

wxString C_DataGridBase::GetValue(int row, int col)
{
	C_Column* pColumn = m_pColumnSet->GetColumn(col);
	if(pColumn)
	{
		if (row >= m_filteredRows.Size())
			return wxEmptyString;

		const C_Row::ColumnMap& columnMap = m_filteredRows[row]->GetColumns();
		C_Row::ColumnMap::ConstIterator it = columnMap.Find(pColumn->GetKey());
		if (it != columnMap.End())
			return it->GetValue();
	}

	return wxEmptyString;
}

wxString C_DataGridBase::GetTypeName(int row, int col)
{
	C_Column* pColumn = m_pColumnSet->GetColumn(col);
	if(pColumn)
	{
		switch(pColumn->GetDataType())
		{
		case E_DataType_Bool:
			return wxGRID_VALUE_BOOL;
			//return wxString::Format(_T("%s:all,MSW,GTK,other"), wxGRID_VALUE_CHOICE);
		case E_DataType_Int:
		case E_DataType_Long:
			return wxGRID_VALUE_LONG;
		case E_DataType_Float:
		case E_DataType_Double:
			return wxGRID_VALUE_FLOAT;
		case E_DataType_UInt: //wxGrid cannot display big numbers like UINT correctly, so display as text
		case E_DataType_String:
			return wxString::Format(_T("%s:80"), wxGRID_VALUE_STRING);
		}
	}

	return wxEmptyString;
}

bool C_DataGridBase::CanGetValueAs(int row, int col, const wxString& typeName)
{
	C_Column* pColumn = m_pColumnSet->GetColumn(col);
	
	if ( typeName == wxGRID_VALUE_STRING )
    {
        return true;
    }
    else if ( typeName == wxGRID_VALUE_BOOL )
    {
		return pColumn->GetDataType() == E_DataType_Bool;
    }
    else if ( typeName == wxGRID_VALUE_LONG )
    {
		return pColumn->GetDataType() == E_DataType_Int || pColumn->GetDataType() == E_DataType_Long;	
    }
	else if ( typeName == wxGRID_VALUE_FLOAT )
    {
		return pColumn->GetDataType() == E_DataType_Float || pColumn->GetDataType() == E_DataType_Double;	
    }

	return false;
}

bool C_DataGridBase::CanSetValueAs(int row, int col, const wxString& typeName)
{
	return CanGetValueAs(row, col, typeName);
}

bool C_DataGridBase::GetValueAsBool(int row, int col)
{
	return GetValueAs(m_pColumnSet, m_filteredRows, row, col, false);
}

long C_DataGridBase::GetValueAsLong(int row, int col)
{
	return GetValueAs(m_pColumnSet, m_filteredRows, row, col, long(0));
}

double C_DataGridBase::GetValueAsDouble(int row, int col)
{
	return GetValueAs(m_pColumnSet, m_filteredRows, row, col, double(0));
}

void C_DataGridBase::SetValue(int row, int col, const wxString& value)
{
}

void C_DataGridBase::SetValueAsBool(int row, int col, bool value)
{
}

void C_DataGridBase::SetValueAsLong(int row, int col, long value)
{
}

void C_DataGridBase::SetValueAsDouble(int row, int col, double value)
{
}

void C_DataGridBase::Clear()
{
	
}
/*
void C_XtDataTableBase::OnRecordAdded(int index)
{
	if(index != -1) 
	{
		NotifyRowsInserted(index, 1);
	}
	else
	{
		NotifyRowsAppended(1);
	}
	GetView()->Fit();
	((C_DataTable*)GetView())->SelectCell(0, 0);
	wxGetApp().GetFrame()->GetAuiManager().Update();
}

void C_XtDataTableBase::OnRecordModified(int index)
{
	GetView()->Refresh();
}

void C_XtDataTableBase::OnRecordDeleted(int index)
{
	NotifyRowsDeleted(index);

	GetView()->Fit();
	((C_DataTable*)GetView())->SelectCell(0, 0);
	wxGetApp().GetFrame()->GetAuiManager().Update();
}

void C_XtDataTableBase::OnRecordsSorted()
{
	GetView()->Refresh();
	((C_DataTable*)GetView())->SelectCell(0, 0);
}
*/
/*
void C_DataGridBase::NotifyRowsAppended(size_t numRows)
{
    wxGridTableMessage msg( this,
                            wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                            numRows );

    GetView()->ProcessTableMessage( msg );
}

void C_DataGridBase::NotifyRowsInserted(size_t pos, size_t numRows)
{
    wxGridTableMessage msg( this,
                            wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                            pos, numRows );

    GetView()->ProcessTableMessage( msg );
}

void C_DataGridBase::NotifyRowsDeleted(size_t pos, size_t numRows)
{
    wxGridTableMessage msg( this,
                            wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                            pos, numRows );

    GetView()->ProcessTableMessage( msg );
}
*/
wxString C_DataGridBase::GetColLabelValue(int col)
{
	C_Column* column = m_pColumnSet->GetColumn(col);
	if(column)
	{
		return column->GetLabel() == wxT("") ? column->GetKey() : column->GetLabel();
	}

	return wxEmptyString;
}

C_ColumnSet* C_DataGridBase::GetColumnSet()
{
	return m_pColumnSet;
}

C_Row* C_DataGridBase::GetRow(S32 row)
{
	return m_filteredRows[row];
}

void C_DataGridBase::OnRowDeserialised(C_Row* pRow)
{
}

void C_DataGridBase::OnRowSetRowInserted(C_RowSet* pRowSet, C_Row* pRow, int pos)
{
	m_Flags.dirty = true;

	/*m_filteredRows.InsertBefore(pRow, m_filteredRows.Begin() + pos);

    wxGridTableMessage msg( this,
                            wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                            pos, 1 );

    GetView()->ProcessTableMessage( msg );*/
}

void C_DataGridBase::OnRowSetRowAppended(C_RowSet* pRowSet, C_Row* pRow)
{
	m_Flags.dirty = true;

	/*m_filteredRows.PushBack(pRow);
	
    wxGridTableMessage msg(	this,
                            wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                            1);

    GetView()->ProcessTableMessage( msg );*/
}

void C_DataGridBase::OnRowSetRowReordered(C_RowSet* pRowSet, C_Row* pRow)
{
	m_Flags.dirty = true;
}

void C_DataGridBase::OnRowSetRowDelete(C_RowSet* pRowSet, C_Row* pRow)
{
	S32 idx = Btl::FindIndex(m_filteredRows, pRow);
	if (idx == m_filteredRows.Size())
		return;

	m_filteredRows.EraseByIndex(idx);

    wxGridTableMessage msg( this,
                            wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                            idx, 
							1);

    GetView()->ProcessTableMessage( msg );
}

void C_DataGridBase::OnViewsEnabled(bool enable)
{
	m_Flags.enabled = enable;
}

void C_DataGridBase::FilterRows()
{
	int oldRowCount = m_filteredRows.Size();
	
	m_filteredRows.Clear();
	for (S32 i=0; i<m_pRowSet->GetNumRows(); i++)
	{
		C_Row* currentRow = m_pRowSet->GetRow(i);
		if (FilterRow(currentRow))
			m_filteredRows.PushBack(currentRow);
	}

	if (m_filteredRows.Size() > oldRowCount)
	{
		wxGridTableMessage msg( this,
                            wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
							0,
							m_filteredRows.Size() - oldRowCount);

		GetView()->ProcessTableMessage( msg );
	}
	else
	{
		wxGridTableMessage msg( this,
                            wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                            0, 
							oldRowCount - m_filteredRows.Size());

		GetView()->ProcessTableMessage( msg );
	}
}

bool C_DataGridBase::FilterRow(C_Row* pRow)
{
	C_Column* filterColumn = m_pFilter->GetFilterColumn();
	if (filterColumn)
	{
		wxString string = pRow->GetColumns()[filterColumn->GetKey()];
		
		if (m_pFilter->MatchText(string.ToAscii()))
			return true;
	}
	else
	{
		C_Row::ColumnMap& columns = pRow->GetColumns();
		
		for (C_Row::ColumnMap::Iterator it = columns.Begin(); it != columns.End(); ++it)
		{
			wxString string = it->GetValue();
			if (m_pFilter->MatchText(string.ToAscii()))
				return true;
		}
	}
	
	return false;
}

void C_DataGridBase::SortFilteredRows()
{
	if (m_filteredRows.Size() > 0)
	{
		C_SortFilterPredicate predicate(m_pFilter);

		std::sort(
			&m_filteredRows[0],
			(&m_filteredRows[m_filteredRows.Size()-1])+1,
			predicate);
	}
}


C_DataGridBase::C_Slots::C_Slots(C_DataGridBase* pDataGridBase)
:
m_pDataGridBase(pDataGridBase)
{
}
	
void C_DataGridBase::C_Slots::Connect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.rowDeserialised.connect(this, &C_DataGridBase::C_Slots::OnRowDeserialisedSlot);
	
	s.rowSetRowInserted.connect(this, &C_DataGridBase::C_Slots::OnRowSetRowInsertedSlot);
	s.rowSetRowAppended.connect(this, &C_DataGridBase::C_Slots::OnRowSetRowAppendedSlot);
	s.rowSetRowReordered.connect(this, &C_DataGridBase::C_Slots::OnRowSetRowReorderedSlot);
	s.rowSetRowDelete.connect(this, &C_DataGridBase::C_Slots::OnRowSetRowDeleteSlot);
	
	s.viewsEnabled.connect(this, &C_DataGridBase::C_Slots::OnViewsEnabledSlot);
}
		
void C_DataGridBase::C_Slots::Disconnect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.rowDeserialised.disconnect(this);

	s.rowSetRowInserted.disconnect(this);
	s.rowSetRowAppended.disconnect(this);
	s.rowSetRowReordered.disconnect(this);
	s.rowSetRowDelete.disconnect(this);

	s.viewsEnabled.disconnect(this);
}

void C_DataGridBase::C_Slots::OnRowDeserialisedSlot(C_Row* pRow)
{
	m_pDataGridBase->OnRowDeserialised(pRow);
}

void C_DataGridBase::C_Slots::OnRowSetRowInsertedSlot(C_RowSet* pRowSet, C_Row* pRow, int pos)
{
	m_pDataGridBase->OnRowSetRowInserted(pRowSet, pRow, pos);
}

void C_DataGridBase::C_Slots::OnRowSetRowAppendedSlot(C_RowSet* pRowSet, C_Row* pRow)
{
	m_pDataGridBase->OnRowSetRowAppended(pRowSet, pRow);
}

void C_DataGridBase::C_Slots::OnRowSetRowReorderedSlot(C_RowSet* pRowSet, C_Row* pRow)
{
	m_pDataGridBase->OnRowSetRowReordered(pRowSet, pRow);
}

void C_DataGridBase::C_Slots::OnRowSetRowDeleteSlot(C_RowSet* pRowSet, C_Row* pRow)
{
	m_pDataGridBase->OnRowSetRowDelete(pRowSet, pRow);
}

void C_DataGridBase::C_Slots::OnViewsEnabledSlot(bool enable)
{
	m_pDataGridBase->OnViewsEnabled(enable);
}

}
