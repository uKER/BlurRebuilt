#ifndef EDITOR_LEVEL_SELECT_DIALOG_H
#define EDITOR_LEVEL_SELECT_DIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

class Explorer;

namespace Editor
{
	class C_LevelSelectDialog : public wxDialog
	{
	public:
		C_LevelSelectDialog(
			wxWindow *parent, wxWindowID id=wxID_ANY,
			const wxString& title = wxString("Load Level"),
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(500,500),
			long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

		~C_LevelSelectDialog();

		void OnSize(wxSizeEvent& e);
		void OnButtonClick(wxCommandEvent& e);

		void UpdateUI();
		void OpenPack(Xt::DomPack* pPack);
		void OpenRecord(Xt::DomRecord* pRecord);

		Xt::DomElement* GetSelectedItem() const;

	private:
		wxPanel*		m_pExplorerPanel;
		Explorer*		m_pXtTreeView;
		wxButton*		m_pButtonOK;
		wxButton*		m_pButtonCancel;

		Xt::DomElement*	m_pSelectedItem;
	};
}

#endif
