#ifndef EDITOR_SELECTION_TOOLPANEL_H
#define EDITOR_SELECTION_TOOLPANEL_H


#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_SelectionToolPanel : public wxPanel
	{
	public:
		C_SelectionToolPanel(wxWindow *parent, wxWindowID id = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxDefaultSize,
							long style = wxTAB_TRAVERSAL|wxNO_BORDER);

		~C_SelectionToolPanel();

		void OnSelectionMode(wxCommandEvent& e);
		void OnSelectionFilter(wxCommandEvent& e);

		void OnUpdateSelectionMode(wxUpdateUIEvent& e);
		void OnUpdateSelectionFilter(wxUpdateUIEvent& e);

		virtual wxSize DoGetBestSize();

	private:

		wxToolBar*					m_pObjectSelectFilter;
		wxToolBar*					m_pComponentSelectFilter;

		wxToolBar*					m_pMainToolBar;
		Ut::UtMap<S32, wxToolBar*>	m_pToolBarSizerMap;
		Ut::UtMap<S32, S32>			m_ToolBarItemIdMap;
		


	};

}

#endif