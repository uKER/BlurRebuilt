#ifndef DMAN_FRAME_TOOLS_MENU_H
#define DMAN_FRAME_TOOLS_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

class AsyncOp;

namespace DMan
{
	class C_FrameToolsMenu
	{
	public:
		C_FrameToolsMenu(C_FrameMenuBar* pMenuBar);

		bool InitialiseUI();

		static void OnBuild(wxCommandEvent& e);
		static bool OnBuildSub(AsyncOp* op, C_App* sep);
		static void OnGenerateTTS(wxCommandEvent& event);
		static void OnGenerateNames(wxCommandEvent& event);
		static void OnGenerateHashes(wxCommandEvent& event);
		static void OnGenerateSplitGroups(wxCommandEvent& event);
		static void OnExecuteDML(wxCommandEvent& event);
		static void OnAudioXML(wxCommandEvent& event);
		static void OnDisplayTTSFilesInFolder(wxCommandEvent& event);
		static void OnDisplayNonTTSFilesInFolder(wxCommandEvent& event);
		static void OnBatchGenerate(wxCommandEvent& event);

	private:
		
		static C_FrameMenuBar* m_pMenuBar;
	};
}

#endif //DMAN_FRAME_TOOLS_MENU_H
