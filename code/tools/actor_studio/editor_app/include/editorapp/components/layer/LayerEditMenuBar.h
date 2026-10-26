#ifndef EDITOR_LAYER_EDIT_MENU_BAR_H
#define EDITOR_LAYER_EDIT_MENU_BAR_H


#ifdef _MSC_VER
#pragma once
#endif

#include <EditorApp/Components/scene_graph/SceneGraphEnums.h>

namespace Editor
{
	class C_LayerEditPanel;
	class C_SceneGraphFilter;
	class C_MenuBar;

	class C_LayerEditMenuBar
	{
	public:
		C_LayerEditMenuBar(C_LayerEditPanel *pEditPanel);
		~C_LayerEditMenuBar();

		bool InitialiseUI();

		C_LayerEditPanel* GetLayerEditPanel();
		C_MenuBuilderBase* GetMenuBuilder();

	private:
		void OnCreateLayer();
		void CanCreateLayer(wxUpdateUIEvent &e);

		void OnDeleteLayer();
		void OnAddSelection();

		void IsLayerSelected(wxUpdateUIEvent &e);

		C_LayerEditPanel	*m_pLayerEditPanel;
		C_MenuBuilderBase	*m_pMenuBuilder;
	};
}

#endif