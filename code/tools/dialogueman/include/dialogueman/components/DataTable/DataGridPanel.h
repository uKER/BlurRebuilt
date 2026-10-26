#ifndef DMAN_DATA_GRID_PANEL_H
#define DMAN_DATA_GRID_PANEL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_DataGridPanel : public wxPanel
{
public:
	C_DataGridPanel(wxWindow *parent,
			wxWindowID = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200,400),
			long style = wxTAB_TRAVERSAL|wxNO_BORDER);

	void Shutdown();

private:
	class C_Slots : public sigslot::has_slots<>
	{
	public:
		C_Slots(C_DataGridPanel* pDataGridPanel);
		void Connect();
		void Disconnect();

		void OnSelectionChangedSlot();

		FRIEND_CLASS C_DataGridPanel;

	private:
		C_DataGridPanel* m_pDataGridPanel;
	};

	void OnSelectionChanged();
	void UpdateColumnFilters(C_ColumnSet* columnSet);

	void OnFilterTextUpdate(wxCommandEvent& e);
	void OnFilterColumnUpdate(wxCommandEvent& e);

	DECLARE_CLASS(C_DataGridPanel);
	DECLARE_EVENT_TABLE()

	C_Slots			m_slots;
	C_DataGrid*		m_pGrid;
	C_ColumnSet*	m_pColumnSet;
	wxTextCtrl*		m_pFilterText;
	wxComboBox*		m_pFilterColumn;
};

}

#endif