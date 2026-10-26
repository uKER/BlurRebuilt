#ifndef DMAN_ADD_ROWS_H
#define DMAN_ADD_ROWS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace AddRows
{

	class C_Options : public wxDialog
	{
	public:

		enum {
			ID_ROWTYPE = 1,
			ID_LOCTABLE,
			ID_POSITION
		};

		C_Options(wxWindow *parent);

		LarvaType*		GetRowType() const { return m_pRowType; }
		Xt::DomRecord*	GetLocTable() const { return m_pLocTable; }
		int				GetPosition() const { return m_pPositionRadio->GetSelection(); }	

	private:
		void PopulateRowTypeChoice();
		void PopulateLocTableChoice();
		void DisablePositionRadioIf();

		void OnRowTypeSelected(wxCommandEvent& event);
		void OnLocTableSelected(wxCommandEvent& event);
		void OnPositionRadioSelected(wxCommandEvent& event);

		LarvaType*		m_pRowType;
		Xt::DomRecord*	m_pLocTable;

		wxChoice*		m_pRowTypeChoice;
		wxChoice*		m_pLocTableChoice;
		wxRadioBox*		m_pPositionRadio;

		static int		defaultRowTypeSel;
		static int		defaultLocTableSel;
		static int		defaultPositionSel;

		DECLARE_EVENT_TABLE()
	};

	bool Do();
	bool Add(LarvaType* pRowType, Xt::DomArray* pDialogueArray, Xt::DomRecord* pLocTable, Btl::Map<wxString, C_MutantValue>& row, int index = -1);
}
}

#endif // DMAN_ADD_ROWS_H
