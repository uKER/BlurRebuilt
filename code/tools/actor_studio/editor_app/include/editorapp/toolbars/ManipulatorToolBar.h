#ifndef EDITOR_MANIPULATOR_TOOLBAR_H
#define EDITOR_MANIPULATOR_TOOLBAR_H

namespace Editor
{
	namespace ToolBars
	{
		wxToolBar* CreateManipulatorToolBar(wxWindow *parent, wxWindowID id,
											const wxPoint& pos = wxDefaultPosition,
											const wxSize& size = wxDefaultSize,
											long style = wxTB_FLAT|wxTB_NODIVIDER);

		void OnSetActiveManipulator(wxCommandEvent& e);
		void OnUpdateManipulatorToolBar(wxUpdateUIEvent& e);
	};
}

#endif