#ifndef DMAN_DATA_GRID_H
#define DMAN_DATA_GRID_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_DataGridFilter;

	class C_DataGrid : public wxGrid
	{
	public:
		enum {
			ID_NO_SORT = 1,
			ID_SORT_ASC,
			ID_SORT_DESC,
		};

		struct C_Flags
		{
			bool enabled	 : 1;
			bool dirty		 : 1;
		};

		C_DataGrid(
			wxWindow *parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxWANTS_CHARS,
			const wxString& name = wxGridNameStr);

		virtual ~C_DataGrid();

		void OnLabelRightClick(wxGridEvent& event);
		void OnCellLeftClick(wxGridEvent& event);
		void OnSelectCell(wxGridEvent& event);
		void OnRangeSelect(wxGridRangeSelectEvent& event);
		void OnSortByColumn(wxCommandEvent& event);
		void OnKeyDown(wxKeyEvent& event);
		
		virtual void ProcessGridCellMouseEvent( wxMouseEvent& event );

		const C_Flags& GetFlags() const;
		C_Flags& GetFlags();
		
		C_DataGridFilter* GetFilter();
		void Shutdown();
		void SetDirty();

		DECLARE_EVENT_TABLE()

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_DataGrid* pDataGrid);
			void Connect();
			void Disconnect();

			void OnSelectionChangedSlot();
			void OnDbCloseSlot();

			void OnRowDeserialisedSlot(C_Row* pRow);
			
			void OnRowSetRowAppendedSlot(C_RowSet* pRowSet, C_Row* pRow);
			void OnRowSetRowReorderedSlot(C_RowSet* pRowSet, C_Row* pRow);
			void OnRowSetRowDeleteSlot(C_RowSet* pRowSet, C_Row* pRow);

			void OnViewsEnabledSlot(bool enabled);

			void OnAppRefreshDirtySlot();

			FRIEND_CLASS C_DataGrid;

		private:
			C_DataGrid* m_pDataGrid;
		};

		void SelectCell(wxGridEvent& event);

		void OnDbClose();
		void OnSelectionChanged();
		
		void OnRefreshDirty();
		void OnViewsEnabled(bool enable);
		void SyncSelection();

		C_DataGridFilter*	m_pFilter;
		C_Flags				m_flags;
		C_Slots				m_slots;
		S32					m_colLabelClicked;
		wxGridCellCoords	m_prevCoords;
		bool				m_isSync;
		wxArrayInt			m_selectedRows;
	};
}

#endif
