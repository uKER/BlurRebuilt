#ifndef DMAN_FRAME_EDIT_MENU_H
#define DMAN_FRAME_EDIT_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_FrameEditMenu
	{
	public:
		C_FrameEditMenu(C_FrameMenuBar* pMenuBar);

		bool InitialiseUI();

		static void OnAdd(wxCommandEvent& e);
		static void OnDelete(wxCommandEvent& e);

	private:
		
		static C_FrameMenuBar* m_pMenuBar;
	};
}

#endif   //DMAN_FRAME_EDIT_MENU_H
