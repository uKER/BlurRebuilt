#ifndef EDITOR_NEW_LEVEL_DIALOG
#define EDITOR_NEW_LEVEL_DIALOG

#ifdef _MSC_VER
#pragma once
#endif

namespace XtWin
{
	class C_Explorer;
}

namespace Editor
{
	class C_NewLevelDialog : public wxDialog
	{
	public:
		C_NewLevelDialog(wxWindow *parent, wxWindowID id=wxID_ANY,
							const wxString& title = wxString("New Level"),
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxSize(600,400),
							long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

		~C_NewLevelDialog();

		void UpdateUI();

		void OnButtonClick(wxCommandEvent& e);
		void OnSize(wxSizeEvent& e);

	private:
		Ut::UtVector<Ut::DynamicVector<XtId> > m_XtPackIds;

		wxTextCtrl*			m_pLevelName;
		wxPanel*			m_pExplorerPanel;
		Explorer*			m_pXtTreeView;
		wxButton*			m_pButtonOK;
		wxButton*			m_pButtonCancel;
	};
}

#endif
