#ifndef DMAN_XT_DATA_TABLE_BASE_H
#define DMAN_XT_DATA_TABLE_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_DataGridFilter;

	class C_DataGridBase : public wxGridTableBase
	{
	public:
		struct C_Flags
		{
			bool enabled	: 1;
			bool shutdown	: 1;
			bool dirty		: 1;
		};

		C_DataGridBase(C_ColumnSet* pColumnSet, C_RowSet* pRowSet, C_DataGridFilter* pFilter);
		virtual ~C_DataGridBase();

		// You must override these functions in a derived table class
		virtual int GetNumberRows();
		virtual int GetNumberCols();

		virtual bool IsEmptyCell(int row, int col);

		virtual wxString GetValue(int row, int col);
		virtual void SetValue(int row, int col, const wxString& value);

		// Data type determination and value access
		virtual wxString GetTypeName(int row, int col);

		virtual bool CanGetValueAs(int row, int col, const wxString& typeName);
		virtual bool CanSetValueAs(int row, int col, const wxString& typeName);

		virtual bool   GetValueAsBool(int row, int col);
		virtual long   GetValueAsLong(int row, int col);
		virtual double GetValueAsDouble(int row, int col);

		virtual void SetValueAsBool(int row, int col, bool value);
		virtual void SetValueAsLong(int row, int col, long value);
		virtual void SetValueAsDouble(int row, int col, double value);

		const C_Flags& GetFlags() const;
		C_Flags& GetFlags();

		void OnRefreshDirty();

		// Overriding these is optional
		virtual void Clear();

		virtual wxString GetColLabelValue( int col );

		C_ColumnSet*	GetColumnSet();

		C_Row*			GetRow(S32 row);

		void Shutdown();

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_DataGridBase* pDataGridBase);
			void Connect();
			void Disconnect();

			void OnSelectionChangedSlot();

			void OnRowDeserialisedSlot(C_Row* pRow);
			
			void OnRowSetRowInsertedSlot(C_RowSet* pRowSet, C_Row* pRow, int pos);
			void OnRowSetRowAppendedSlot(C_RowSet* pRowSet, C_Row* pRow);
			void OnRowSetRowReorderedSlot(C_RowSet* pRowSet, C_Row* pRow);
			void OnRowSetRowDeleteSlot(C_RowSet* pRowSet, C_Row* pRow);

			void OnViewsEnabledSlot(bool enabled);

			void OnAppRefreshDirtySlot();

			FRIEND_CLASS C_DataGridBase;

		private:
			C_DataGridBase* m_pDataGridBase;
		};

		class C_SortFilterPredicate
		{
		public:
			C_SortFilterPredicate(C_DataGridFilter* pDataGridFilter);
			bool operator() (C_Row* pLhs, C_Row* pRhs);

		private:
			C_DataGridFilter* m_pDataGridFilter;
		};

		void OnRowDeserialised(C_Row* pRow);

		void OnRowSetRowInserted(C_RowSet* pRowSet, C_Row* pRow, int pos);
		void OnRowSetRowAppended(C_RowSet* pRowSet, C_Row* pRow);
		void OnRowSetRowReordered(C_RowSet* pRowSet, C_Row* pRow);
		void OnRowSetRowDelete(C_RowSet* pRowSet, C_Row* pRow);

		void OnViewsEnabled(bool enable);

		void FilterRows();
		bool FilterRow(C_Row* pRow);
		void SortFilteredRows();

		C_Slots				m_slots;
		C_Flags				m_Flags;
		C_ColumnSet*		m_pColumnSet;
		C_RowSet*			m_pRowSet;
		C_DataGridFilter*	m_pFilter;
		Btl::Vector<C_Row*>	m_filteredRows;	
	};
}

#endif
