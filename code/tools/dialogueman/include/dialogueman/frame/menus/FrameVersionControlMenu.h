#ifndef DMAN_FRAME_VERSION_CONTROL_MENU_H
#define DMAN_FRAME_VERSION_CONTROL_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_FrameVersionControlMenu
	{
	public:
		C_FrameVersionControlMenu(C_FrameMenuBar* pMenuBar);

		bool InitialiseUI();

		static void OnUpdate(wxCommandEvent& e);
		static void OnCommit(wxCommandEvent& e);

	private:
		
		static C_FrameMenuBar* m_pMenuBar;
	};
}

#endif //DMAN_FRAME_VERSION_CONTROL_MENU_H
