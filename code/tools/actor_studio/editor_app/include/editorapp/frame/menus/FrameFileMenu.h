#ifndef EDITOR_FRAME_FILE_MENU_H
#define EDITOR_FRAME_FILE_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	template <class T>
	class C_FileMenuItem
	{
	public:
		C_FileMenuItem(int id, wxEventConnection event, T args, Btl::String name) : id(id), event(event), args(args), name(name) {};

		int id;
		wxEventConnection event;
		T args;
		Btl::String name;
	};

	class C_FrameFileMenu
	{
	public:
		C_FrameFileMenu(C_FrameMenuBar* pMenuBar);

		bool InitialiseUI();

		static void OnNewLevel(wxCommandEvent& e);
		static void OnOpenLevel(wxCommandEvent& e);
		static void OnAddLevel(wxCommandEvent& e);
		static void OnClosePack(wxCommandEvent& e);
		static void OnCloseAllPacks(wxCommandEvent& e);
		static void OnSaveLevel(wxCommandEvent& e);
		static void OnExportToLevel(wxCommandEvent& e);
		static void OnShowReferencesDialog(wxCommandEvent& e);
		static void UpdateClosePacks();
		static void UpdateRecentFiles();
		static void OnQuit(wxCommandEvent& e);
		static void OnRecentFiles(wxCommandEvent& e);
		static void OnFileMenu(wxMenuEvent& e);

	private:
		
		static C_FrameMenuBar* m_pMenuBar;
		static wxMenu* m_pFileMenu;
		static Ut::Vector<C_FileMenuItem<int> > m_ClosePackMenuItems;
		static Ut::Vector<C_FileMenuItem<Btl::String> > m_RecentFileMenuItems;
	};
}

#endif
