#ifndef DMAN_FRAME_HELP_MENU_H
#define DMAN_FRAME_HELP_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_FrameHelpMenu
	{
	public:
		C_FrameHelpMenu(C_FrameMenuBar* pMenuBar);

		bool InitialiseUI();

		static void OnAbout(wxCommandEvent& e);

	private:
		static C_FrameMenuBar* m_pMenuBar;
	};
}

#endif //DMAN_FRAME_HELP_MENU_H
