#ifndef EDITOR_LAYER_PROPERTY_MENU_BAR_H
#define EDITOR_LAYER_PROPERTY_MENU_BAR_H


#ifdef _MSC_VER
#pragma once
#endif


namespace Editor
{
	class C_LayerPropertyPanel;

	class C_LayerPropertyMenuBar
	{
	public:
		C_LayerPropertyMenuBar(C_LayerPropertyPanel *pPanel);
		~C_LayerPropertyMenuBar();

		bool InitialiseUI();
		C_LayerPropertyPanel* GetLayerPropertyPanel();

	private:
		
		void OnUpdateLevelLoaded(wxUpdateUIEvent& e);
		void OnUpdateLayerSelected(wxUpdateUIEvent& e);
		void OnUpdateSingleLayerSelected(wxUpdateUIEvent& e);
		void OnCreateLayer();
		void OnSelectLayerObjects();
		void OnAddObjectsToLayer();
		void OnRemoveObjectsFromLayers();
		void OnDeleteSelectedLayers();
		void OnDeleteUnusedLayers();
		void OnShowLayerEditor();


		C_LayerPropertyPanel	*m_pLayerPropertyPanel;
		C_MenuBuilderBase		*m_pMenuBuilder;

	};
}

#endif