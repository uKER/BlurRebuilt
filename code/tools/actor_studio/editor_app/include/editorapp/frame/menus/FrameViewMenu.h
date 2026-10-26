#ifndef EDITOR_FRAME_VIEW_MENU_H
#define EDITOR_FRAME_VIEW_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_FrameViewMenu
	{
	public:
		C_FrameViewMenu(C_FrameMenuBar* pMenuBar);

		bool InitialiseUI();

	private:
		C_FrameMenuBar* m_pMenuBar;
	};
}

#endif
