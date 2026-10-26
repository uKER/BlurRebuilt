#ifndef EDITOR_FRAME_MENU_BAR_H
#define EDITOR_FRAME_MENU_BAR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_FrameCreateMenu;
	class C_FrameDisplayMenu;
	class C_FrameEditMenu;
	class C_FrameFileMenu;
	class C_FrameVersionControlMenu;
	class C_FrameToolsMenu;
	class C_FrameViewMenu;
	class C_FrameHelpMenu;

	class C_FrameMenuBar
	{
	public:
		C_FrameMenuBar(C_Frame* pFrame);
		~C_FrameMenuBar();

		bool InitialiseUI(const Util::C_String& projectName);

		C_MenuBuilder* GetMenuBuilder();

		C_Frame* GetFrame();

	private:
		C_FrameCreateMenu*		m_pCreateMenu;
		C_FrameDisplayMenu*		m_pDisplayMenu;
		C_FrameEditMenu*		m_pEditMenu;
		C_FrameFileMenu*		m_pFileMenu;
		C_FrameVersionControlMenu*	m_pVersionControlMenu;
		C_FrameToolsMenu*		m_pToolsMenu;
		C_FrameViewMenu*		m_pViewMenu;
		C_FrameHelpMenu*		m_pHelpMenu;

		C_Frame*				m_pFrame;
		C_MenuBuilder*			m_pMenuBuilder;

		wxMenuBar*				m_pMenuBar;
	};
}

#endif
