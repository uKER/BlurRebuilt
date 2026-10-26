#ifndef DMAN_FRAME_FRAMEMENUBAR_H
#define DMAN_FRAME_FRAMEMENUBAR_H

namespace Tools
{
	class C_MenuBuilder;
}

namespace DMan
{
	class C_FrameFileMenu;
	class C_FrameEditMenu;
	class C_FrameVersionControlMenu;
	class C_FrameToolsMenu;
	class C_FrameHelpMenu;

	class C_FrameMenuBar
	{
	public:
		C_FrameMenuBar(C_Frame* pFrame);
		~C_FrameMenuBar();

		bool InitialiseUI(const Util::C_String& projectName);

		Tools::C_MenuBuilder* GetMenuBuilder();

		C_Frame* GetFrame();

	private:
		C_FrameFileMenu*		m_pFileMenu;
		C_FrameEditMenu*		m_pEditMenu;
		C_FrameVersionControlMenu*	m_pVersionControlMenu;
		C_FrameToolsMenu*		m_pToolsMenu;
		C_FrameHelpMenu*		m_pHelpMenu;

		C_Frame*				m_pFrame;
		Tools::C_MenuBuilder*			m_pMenuBuilder;

		wxMenuBar*				m_pMenuBar;
	};
}

#endif //DMAN_FRAME_FRAMEMENUBAR_H