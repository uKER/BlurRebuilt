#ifndef EDITOR_EXPORT_DIALOG
#define EDITOR_EXPORT_DIALOG

#ifdef _MSC_VER
#pragma once
#endif

class wxButton;
class wxListCtrl;
class wxPropertyGrid;
class wxPGProperty;
class wxListEvent;

namespace Editor
{
	class C_DbPropertyDialog : public wxDialog
	{
	public:
		static void OnShowDialog();

		C_DbPropertyDialog (wxWindow *parent, wxWindowID id=wxID_ANY,
							const wxString& title=wxString("Autosave Recovery"),
							const wxPoint& pos=wxDefaultPosition,
							const wxSize& size=wxSize(850,500),
							long style=wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

		~C_DbPropertyDialog();

	private:

		struct C_XtPatch
		{
			Xt::DomBlock	*pOriginal;
			Xt::DomBlock	*pModified;
			bool			deleteOnRevert;
		};

		void InitialisePropertyGrid();
		void PopulatePackList();
		void PopulatePackProperties();
		void InitialisePackProperties();

		void OnPackListColumnClick(wxListEvent& e);
		void OnPackListItemSelect(wxListEvent& e);
		void OnPropertySelected(wxPropertyGridEvent& e);
		void OnPropertyChanged(wxPropertyGridEvent& e);
		void OnButtonClick(wxCommandEvent& e);
		void OnUpdateUI(wxUpdateUIEvent& e);

		wxListCtrl		*m_pListCtrl;
		wxPropertyGrid	*m_pPropGrid;
		wxStaticText	*m_pHelpText;

		wxButton		*m_pInitButton;
		wxButton		*m_pOkButton;
		wxButton		*m_pCancelButton;

		typedef Btl::OpenHashMap<wxPGProperty*, LarvaField*,Btl::HashKeyOpPtr> PropertyHashMap;
		PropertyHashMap	m_propertyHashMap;

		typedef Btl::OpenHashMap<XtId,C_XtPatch> PatchHashMap;
		PatchHashMap	m_xtPatches;

		XtId			m_currentPackId;




	};
}

#endif