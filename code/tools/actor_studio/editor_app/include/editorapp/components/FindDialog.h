#ifndef EDITOR_WX_XT_FIND_DIALOG_H
#define EDITOR_WX_XT_FIND_DIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <wx/combo.h>
#include <wx/listctrl.h>

namespace Editor
{
	class C_FinderDialog;
	class C_TypeChoice;
	class C_FieldChoice;
	class C_CompareChoice;
	class C_RecordCombo;

	/////////////////////////// C_FinderRow \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_FinderRow : public wxEvtHandler 
	{
	public:

		enum Type
		{
			kNone,
			kFindByActorType,
			kFindByActorName,
			kFindByActorFieldExpression
		};

		virtual Type GetType() {  return kNone; }

	public:
		static C_FinderRow* AddRow(C_FinderDialog* pFinderDlg, Type type);
		static void RemoveRows() { XtUt::Free(&rows); }
		static XtUt::Array<C_FinderRow*>& GetRows() { return rows; }

	protected:
		C_FinderRow(C_FinderDialog* pFinderDlg); //use AddRow instead
		virtual ~C_FinderRow();

	public:
		virtual void BuildQuery(Xt::Query* pQuery);

	public:
		void SetEnabled(bool enabled);
		bool IsEnabled() { return m_enabled; }
		void OnEnableBoxClicked(wxCommandEvent& event);
		virtual void OnEnableChanged();
		virtual void Reset();

	protected:
		C_FinderDialog*			m_pFinderDlg;
		wxCheckBox*				m_pEnableBox;
		bool					m_enabled;

	private:
		static XtUt::Array<C_FinderRow*>	rows;
	};

	/////////////////////////// C_FrActorsOfType \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_FrActorsOfType : public C_FinderRow
	{
	public:
		virtual Type GetType() {  return kFindByActorType; }

	public:
		C_FrActorsOfType(C_FinderDialog* pFinderDlg);
		virtual ~C_FrActorsOfType();

	public:
		virtual void BuildQuery(Xt::Query* pQuery);

	public:
		virtual void OnEnableChanged();
		virtual void Reset();

	private:
		void OnTypeSelected(wxCommandEvent& event);

	private:
		C_TypeChoice* m_pTypeChoice;
	};

	/////////////////////////// C_FrActorsNamed \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_FrActorsNamed : public C_FinderRow
	{
	public:
		virtual Type GetType() {  return kFindByActorName; }

	public:
		C_FrActorsNamed(C_FinderDialog* pFinderDlg);
		virtual ~C_FrActorsNamed();

	public:
		virtual void BuildQuery(Xt::Query* query);

	public:
		virtual void OnEnableChanged();
		virtual void Reset();

	private:
		wxTextCtrl* m_pNamedText;
	};

	/////////////////////////// C_FrActorFieldExpression \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_FrActorFieldExpression : public C_FinderRow
	{
	public:
		virtual Type GetType() {  return kFindByActorFieldExpression; }

	public:
		C_FrActorFieldExpression(C_FinderDialog* pFinderDlg);
		virtual ~C_FrActorFieldExpression();

	public:
		virtual void BuildQuery(Xt::Query* query);

	public:
		void SetActorType(LtStruct* pType);

	public:
		virtual void OnEnableChanged();
		virtual void Reset();

	private:
		void				OnFieldSelected(wxCommandEvent& event);
		void				OnCompareSelected(wxCommandEvent& event);
		void				OnRecordSelected(wxCommandEvent& event);

	private:
		C_FieldChoice*		m_pFieldChoice;
		C_CompareChoice*	m_pCompareChoice;
		C_RecordCombo*		m_pRecordCombo;
		wxTextCtrl*			m_pLowText;
		wxTextCtrl*			m_pHighText;
	};

	/////////////////////////// C_TypeChoice \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_TypeChoice : public wxChoice
	{
	public:
		C_TypeChoice(wxWindow* pParent,
             wxWindowID id,
             const wxPoint& pos,
			 const wxSize& size);

		virtual ~C_TypeChoice();

	public:
		void		SetSelectedType(LarvaType* pType);
		LarvaType*	GetSelectedType();

	private:
		void Populate();

	private:
		XtUt::Array<LarvaType*>	m_types;
	};

	/////////////////////////// C_FieldChoice \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_FieldChoice : public wxChoice
	{
	public:
		C_FieldChoice(wxWindow* pParent,
             wxWindowID id,
             const wxPoint& pos,
			 const wxSize& size);

		virtual ~C_FieldChoice();

	public:
		void				SetType(LtStruct* pType);
		LtStruct*			GetType();
		void				SetSelectedField(Xt::FieldPath* pPath);
		Xt::FieldPath*		GetSelectedField();

	public:

		struct Item
		{
			XtString		text;
			Xt::FieldPath	path;

			~Item()
			{
				Xt::Free(path.next);
			}
		};

	private:		
		void				Populate();
		Xt::FieldPath*		AddToList(XtUt::List<Item>* pList, Xt::FieldPath* pPath);
		void				BuildList(LtStruct* pType, Xt::FieldPath* pRoot, XtUt::List<Item>* pList);

		LtStruct*			m_pType;
		XtUt::List<Item>	m_fields;
	};

	/////////////////////////// C_CompareChoice \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_CompareChoice : public wxChoice
	{
	public:
		C_CompareChoice(wxWindow* pParent,
             wxWindowID id,
             const wxPoint& pos,
			 const wxSize& size);

		virtual ~C_CompareChoice();

	public:
		void					SetField(LarvaField* field);
		LarvaField*				GetField();
		void					SetSelectedCmp(Xt::FieldExpressionCmp cmp);
		Xt::FieldExpressionCmp	GetSelectedCmp();

	private:
		void								Populate();
		LarvaField*							m_pField;
		XtUt::Array<Xt::FieldExpressionCmp>	m_cmps;
	};

	/////////////////////////// C_RecordCombo \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_RecordCombo : public wxComboCtrl
	{
	public:
		C_RecordCombo(wxWindow* pParent, 
			wxWindowID id, 
			const wxString& value = "", 
			const wxPoint& pos = wxDefaultPosition, 
			const wxSize& size = wxDefaultSize,
			long style = 0);
		
		virtual ~C_RecordCombo();

	public:
		void					SetField(LarvaField* field);
		LarvaField*				GetField();
		void					SetSelectedRecordId(XtId recordId);
		XtId					GetSelectedRecordId();

	private:
		void					Populate();
		LarvaField*				m_pField;
	};

	class wxListViewComboPopup : public wxListView, public wxComboPopup
	{
	public:
		// Initialize member variables
		virtual void Init()
		{
			m_value = -1;
			m_maxItemCharLen = 0;
		}

		// Create popup control
		virtual bool Create(wxWindow* parent)
		{
			return wxListView::Create(parent, LIST_VIEW_ID, wxPoint(0,0), wxDefaultSize, 
				wxLC_LIST | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING);
		}

		// Return pointer to the created control
		virtual wxWindow *GetControl() { return this; }

		// Translate string into a list selection
		virtual void SetStringValue(const wxString& s)
		{
			int n = wxListView::FindItem(-1,s);
			if ( n >= 0 && n < wxListView::GetItemCount() )
				wxListView::Select(n);
		}

		// Get list selection as a string
		virtual wxString GetStringValue() const
		{
			if ( m_value >= 0 )
				return wxListView::GetItemText(m_value);
			return wxEmptyString;
		}

		// On mouse left up, set the value and close the popup
		void OnMouseClick(wxMouseEvent& WXUNUSED(event))
		{
			m_value = wxListView::GetFirstSelected();
			Dismiss();
		}

		void OnFocused(wxListEvent& event)
		{
			m_value = event.GetIndex();
			wxString text = event.GetText();
			wxListView::Select(m_value);
			Dismiss();
		}

		wxSize GetAdjustedSize(int minWidth, int prefHeight, int maxHeight)
		{
			int newWidth = m_maxItemCharLen * 6;
			int newHeight = GetItemCount() * 16;

			if(newWidth < minWidth)
				newWidth = minWidth;

			if(newHeight > prefHeight)
				newHeight = prefHeight;
			
			return wxSize(newWidth, newHeight);
		}

		void SetMaxItemCharLen(int maxItemCharLen)
		{
			m_maxItemCharLen = maxItemCharLen;
		}

		int GetMaxItemCharLen()
		{
			return m_maxItemCharLen;
		}

	protected:
		int m_value; // current item index
		int m_maxItemCharLen;

	private:
		static const int LIST_VIEW_ID;
		DECLARE_EVENT_TABLE()
	};

	/////////////////////////// FinderResult \\\\\\\\\\\\\\\\\\\\\\\\\\\

	struct FinderResult
	{
		XtString		path;
		LarvaField*		field;
		LtStruct*		type;
		Xt::DomMonitor	dom;

	public:
		FinderResult() :
			field(0),
			type(0)
		{
		}
	};

	/////////////////////////// C_ResultsGrid \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_ResultsGrid : public wxGrid
	{
	public:
		C_ResultsGrid(
			wxWindow* pParent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxWANTS_CHARS,
			const wxString& name = wxGridNameStr);

		virtual ~C_ResultsGrid();

	private:
		void OnCellLeftClick(wxGridEvent& event);
		void OnLabelLeftClick(wxGridEvent& event);
	};

	/////////////////////////// C_ResultsGridTableBase \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_ResultsGridTableBase : public wxGridTableBase
	{
	public:
		C_ResultsGridTableBase(XtUt::List<FinderResult>& results);
		virtual ~C_ResultsGridTableBase();

	public:
		virtual int			GetNumberRows();
		virtual int			GetNumberCols();
		virtual bool		IsEmptyCell(int row, int col);
		virtual wxString	GetValue(int row, int col);
		virtual void		SetValue(int row, int col, const wxString& value);
		virtual wxString	GetColLabelValue(int col);
		void				Sort(int col, bool isAsc);

	private:
		struct Result
		{
			wxString actorID;
			wxString actorName;
			wxString actorType;
		};

		class C_SortPredicate
		{
		public:
			C_SortPredicate(int col, bool isAsc);
			bool operator() (const Result& lhs, const Result& rhs);

		private:
			int m_col;
			bool m_isAsc;
		};

		Btl::Vector<Result> m_sortedResults;
	};

	/////////////////////////// C_FinderDialog \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_FinderDialog : public wxDialog
	{
	public:
		C_FinderDialog(
			wxWindow* pParent, 
			wxWindowID id = wxID_ANY,
			const wxString& title = wxString("Find Actors"),
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(600,400),
			long style = wxDEFAULT_DIALOG_STYLE);

		~C_FinderDialog();

	public:
		static void			OnShowDialog();
		void				OnClose(wxCloseEvent& event);

		wxFlexGridSizer*	GetFinderRowsSizer() { return m_pRowsSizer; }

	public:
		void				AddResult(const Xt::QueryResult* pResult);

	private:
		void				ExecuteQuery();
	
	private:
		void				OnFindNowClick(wxCommandEvent& event);
		void				OnNewSearchClick(wxCommandEvent& event);

	private:
		wxFlexGridSizer*			m_pRowsSizer;
		C_ResultsGrid*				m_pResultsGrid;
		wxButton*					m_pFindNowBtn;
		wxButton*					m_pNewSearchBtn;
		XtUt::List<FinderResult>	m_results;

	private:
		static C_FinderDialog*		m_pFinderDlg;

	};

}

#endif
