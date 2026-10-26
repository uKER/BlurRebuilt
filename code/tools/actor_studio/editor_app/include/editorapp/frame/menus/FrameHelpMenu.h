#ifndef EDITOR_FRAME_HELP_MENU_H
#define EDITOR_FRAME_HELP_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_FrameHelpMenu
	{
	public:
		C_FrameHelpMenu(C_FrameMenuBar* pMenuBar);

		bool InitialiseUI();

		static void OnAbout(wxCommandEvent& e);

	private:
		C_FrameMenuBar* m_pMenuBar;
	};
}

#endif
