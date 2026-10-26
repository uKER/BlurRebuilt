#ifndef EDITOR_GROUP_EDIT_PANEL_H
#define EDITOR_GROUP_EDIT_PANEL_H

#ifdef _MSC_VER
#pragma once
#endif

class wxSplitterCtrl;

namespace Editor
{

class C_SelectionSceneGraphPanel;

class C_GroupEditPanel : public wxPanel
{
public:
	C_GroupEditPanel(wxWindow *parent, wxWindowID winid = wxID_ANY,
					 const wxPoint& pos = wxDefaultPosition,
					 const wxSize& size = wxSize(300, 500),
					 long style = wxTAB_TRAVERSAL | wxNO_BORDER,
					 const wxString& name = wxPanelNameStr);

	~C_GroupEditPanel();

	void Shutdown();

private:
	void OnButton(wxCommandEvent& e);

	wxSplitterCtrl* m_SplitterCtrl;
	C_SelectionSceneGraphPanel* m_LeftPanel;
	C_SelectionSceneGraphPanel* m_RightPanel;
	wxButton* m_LeftButton;
	wxButton* m_RightButton;
};

}

#endif