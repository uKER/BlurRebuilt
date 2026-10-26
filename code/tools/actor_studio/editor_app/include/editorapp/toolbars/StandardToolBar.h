#ifndef EDITOR_STANDARD_TOOLBAR_H
#define EDITOR_STANDARD_TOOLBAR_H

namespace Editor
{
	namespace ToolBars
	{
		wxToolBar* CreateStandardToolBar(wxWindow *parent, wxWindowID id,
											const wxPoint& pos = wxDefaultPosition,
											const wxSize& size = wxDefaultSize,
											long style = wxTB_FLAT|wxTB_NODIVIDER);
	};
}

#endif