#ifndef EDITOR_VIEWPORT_MENU_BAR_H
#define EDITOR_VIEWPORT_MENU_BAR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_ViewportPanel;

	class C_ViewportShadingMenu;
	class C_ViewportRendererMenu;
	class C_ViewportCameraMenu;

	class C_ViewportMenuBar
	{
	public:
		C_ViewportMenuBar(C_ViewportPanel *pViewportPanel);
		~C_ViewportMenuBar();

		bool InitialiseUI();
		C_MenuBuilderBase* GetMenuBuilder();
		C_ViewportPanel* GetViewportPanel();
		
	private:
		C_ViewportShadingMenu	*m_pShadingMenu;
		C_ViewportRendererMenu	*m_pRendererMenu;
		C_ViewportCameraMenu	*m_pCameraMenu;

		C_ViewportPanel			*m_pViewportPanel;
		C_MenuBuilderBase		*m_pMenuBuilder;
	};
}

#endif
