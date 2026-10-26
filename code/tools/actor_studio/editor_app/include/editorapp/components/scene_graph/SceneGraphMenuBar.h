#ifndef EDITOR_SCENE_GRAPH_MENU_BAR_H
#define EDITOR_SCENE_GRAPH_MENU_BAR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_SceneGraphPanel;

	class C_SceneGraphFilterMenu;

	class C_SceneGraphFilter;

	class C_MenuBar;

	class C_SceneGraphMenuBar
	{
	public:
		C_SceneGraphMenuBar(C_SceneGraphPanel *pSceneGraphPanel, C_SceneGraphFilter* pSceneFilter);
		~C_SceneGraphMenuBar();

		bool InitialiseUI();
		C_MenuBuilderBase* GetMenuBuilder();
		C_SceneGraphPanel* GetSceneGraphPanel();

		C_SceneGraphFilterMenu* GetFilterMenu();

		C_MenuBar* GetMenuBar();
		
	private:
		C_SceneGraphFilterMenu*	m_pFilterMenu;

		C_SceneGraphPanel*		m_pSceneGraphPanel;
		C_MenuBuilderBase*		m_pMenuBuilder;

		C_SceneGraphFilter*		m_pSceneFilter;

		C_MenuBar*				m_pMenuBar;
	};
}

#endif
