#ifndef EDITOR_SNAP_TOOLBAR_H
#define EDITOR_SNAP_TOOLBAR_H

namespace Editor
{
	namespace ToolBars
	{
		wxToolBar* CreateSnapToolBar(wxWindow *parent, wxWindowID id,
											const wxPoint& pos = wxDefaultPosition,
											const wxSize& size = wxDefaultSize,
											long style = wxTB_FLAT|wxTB_NODIVIDER);

		void OnToggleSnappingMode(wxCommandEvent& e);
		void OnUpdateSnapToolBar(wxUpdateUIEvent& e);
	};
}

#endif