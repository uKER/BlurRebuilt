#ifndef EDITOR_DISPLAY_TOOLBAR_H
#define EDITOR_DISPLAY_TOOLBAR_H

namespace Editor
{
	namespace ToolBars
	{
		wxToolBar* CreateDisplayToolBar(wxWindow *parent, wxWindowID id,
											const wxPoint& pos = wxDefaultPosition,
											const wxSize& size = wxDefaultSize,
											long style = wxTB_FLAT|wxTB_NODIVIDER);

		void OnToggleDisplayMode(wxCommandEvent& e);
		void OnUpdateDisplayToolBar(wxUpdateUIEvent& e);
	};
}

#endif