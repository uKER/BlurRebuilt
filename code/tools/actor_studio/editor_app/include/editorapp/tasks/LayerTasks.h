#ifndef EDITOR_LAYER_TASKS_H
#define EDITOR_LAYER_TASKS_H


#ifdef _MSC_VER
#pragma once
#endif


namespace EditorLib
{
	class C_Layer;
}

namespace Editor
{
	namespace Tasks
	{
		EditorLib::I_Layer* CreateLayer();

		void SetSelectionFromLayers(const Ut::Vector<const EditorLib::I_Layer*>& layers, const char* cmd);
		void AddSelectionToLayer(EditorLib::I_Layer* layer);
		void RemoveSelectionFromLayers(const Ut::Vector<EditorLib::I_Layer*>& layers);
		void RemoveUnusedLayers();

		void AddNodesToLayer(EditorLib::I_Layer* pLayer, const Ut::UtVector< Ut::DynamicVector<EditorLib::I_DgNode*> >& nodes);
	}
}

#endif