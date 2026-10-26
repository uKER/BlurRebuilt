#ifndef DMAN_FRAME_FILE_MENU_H
#define DMAN_FRAME_FILE_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_FrameFileMenu
	{
	public:
		C_FrameFileMenu(C_FrameMenuBar* pMenuBar);

		bool InitialiseUI();

		static void OnExit(wxCommandEvent& event);
		static void OnImport(wxCommandEvent& event);
		static void OnExport(wxCommandEvent& event);
		static void OnSave(wxCommandEvent& event);
		static void OnReport(wxCommandEvent& event);

	private:
		
		static C_FrameMenuBar* m_pMenuBar;
	};
}

#endif //DMAN_FRAME_FILE_MENU_H
