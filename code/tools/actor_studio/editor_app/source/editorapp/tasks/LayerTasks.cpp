#include "stdafx.h"

#include <EditorApp/Tasks/LayerTasks.h>

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>

#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LayerManager.h>
#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/Selection.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

namespace Editor
{
namespace Tasks
{

EditorLib::I_Layer* CreateLayer()
{
	EditorLib::I_Level* pLevel = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel();
	EditorLib::I_LayerSet* pLayerSet = EditorLib::C_Core::Instance().GetLayerManager().GetLayerSet(pLevel);
	EditorLib::C_XtActorLayer* pLayer = new EditorLib::C_XtActorLayer(pLayerSet, pLevel);
	pLayerSet->AddLayer(pLayer);
//	if (pLayer)
//		pLayer->SetName("New Layer");

	return pLayer;
}

void SetSelectionFromLayers(const Ut::Vector<const EditorLib::I_Layer*>& layers, const char* cmd)
{
	EditorLib::C_Selection selection;
	Btl::String args;
	for (S32 i=0; i<layers.Size(); ++i)
	{
		const EditorLib::I_Layer::DgNodeSet& nodeSet = layers[i]->GetNodes();
		EditorLib::I_Layer::DgNodeSet::ConstIterator it = nodeSet.Begin();

		while (it != nodeSet.End())
		{
			Btl::String tmp;
			tmp.Printf("%s,", it->GetValue()->GetId().AsString().Chars());
			args += tmp;
			it++;
		}
	}

	if (args.Length())
	{
		args = args(0, args.Length()-1);
		if (cmd)
			args = args + " " + cmd;
			

		EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", args);
	}
}

void AddSelectionToLayer(EditorLib::I_Layer* pLayer)
{
	if (!pLayer || pLayer->GetType() != EditorLib::E_LayerType_Actor)
	{
		LOGERROR(g_Log, "Layer '%s' is read only", pLayer ? pLayer->GetName().Chars() : "");
		return;
	}

	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	const EditorLib::C_Selection::ItemArray& items = selection.GetItems();

	EditorLib::C_ViewDisabler viewDisabler;
	EditorLib::C_Selection::ItemArray::ConstIterator selectionIt = items.Begin();
	while (selectionIt != items.End())
	{
		const EditorLib::C_Selection::C_Item& item = *selectionIt;
		EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(item.id);

		if (pNode)
			EditorLib::C_Core::Instance().GetLayerManager().AddNodeToLayer(pLayer, pNode);

		selectionIt++;
	}
}

void RemoveSelectionFromLayers(const Ut::Vector<EditorLib::I_Layer*>& layers)
{
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	EditorLib::C_ViewDisabler viewDisabler;
	const EditorLib::C_Selection::ItemArray& items = selection.GetItems();

	EditorLib::C_Selection::ItemArray::ConstIterator selectionIt = items.Begin();
	while (selectionIt != items.End())
	{
		const EditorLib::C_Selection::C_Item& item = *selectionIt;
		const EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(item.id);

		if (pNode)
			EditorLib::C_Core::Instance().GetLayerManager().RemoveLayerNode(pNode);

		selectionIt++;
	}
}

void RemoveUnusedLayers()
{
	for (EditorLib::LayerSetMap::ConstIterator layerSet = EditorLib::C_Core::Instance().GetLayerManager().LayerSetIteratorBegin(); layerSet != EditorLib::C_Core::Instance().GetLayerManager().LayerSetIteratorEnd(); ++layerSet)
	{
		Ut::Vector<EditorLib::I_Layer*> layers;

		EditorLib::LayerList::ConstIterator layer = layerSet->GetValue()->GetLayerIterator();
		while (layer.Pointer())
		{
			if ((*layer)->GetNodes().Size() == 0)
				layers.PushBack((*layer));
			++layer;
		}

		for (S32 i=0; i<layers.Size(); ++i)
			layerSet->GetValue()->RemoveLayer(layers[i]);
	}
}

void AddNodesToLayer(EditorLib::I_Layer* pLayer, const Ut::UtVector< Ut::DynamicVector<EditorLib::I_DgNode*> >& nodes)
{
	if (!pLayer || pLayer->GetType() != EditorLib::E_LayerType_Actor)
	{
		LOGERROR(g_Log, "Layer '%s' is read only", pLayer ? pLayer->GetName().Chars() : "");
		return;
	}

	for (S32 i=0; i<nodes.Size(); ++i)
		EditorLib::C_Core::Instance().GetLayerManager().AddNodeToLayer(pLayer, nodes[i]);
}

}
}
