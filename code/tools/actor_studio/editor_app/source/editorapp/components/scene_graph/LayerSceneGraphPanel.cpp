#include "stdafx.h"

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LayerManager.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Components/Scene_Graph/LayerSceneGraphPanel.h>
#include <EditorApp/Components/SplitterControl.h>

using namespace Editor;

C_LayerSceneGraphPanel::C_LayerSceneGraphPanel(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
:
C_SelectionSceneGraphPanel(parent, id, pos, size, style),
m_Slots(this)
{
	m_pNodeTreeCtrl->GetFlags().ToggleSelection = true;
	m_pNodeTreeCtrl->GetFlags().HighlightSelectedRow = true;
	m_pSplitter->SetSplitterProportion(10000);

	m_IgnoreEvents = false;

	m_Slots.Connect();
}

C_LayerSceneGraphPanel::~C_LayerSceneGraphPanel()
{
	m_Slots.Disconnect();
}

void C_LayerSceneGraphPanel::SetLayer(EditorLib::I_Layer* layer)
{
	m_Layer = layer;

	m_IgnoreEvents = true;
	m_pNodeTreeCtrl->ClearSelection(true);

	if (layer)
	{
		for (EditorLib::I_Layer::DgNodeSet::ConstIterator it = layer->GetNodes().Begin(); it != layer->GetNodes().End(); ++it)
		{
			EditorLib::I_DgNode* dgNode = it->GetValue();

			SceneNodeDataMap::Iterator treeNode = m_NodeDataMap.Find(dgNode->GetId());

			if (treeNode != m_NodeDataMap.End())
				treeNode->GetValue()->SetSelected(true);
		}
	}

	m_IgnoreEvents = false;
}

bool C_LayerSceneGraphPanel::OnSelectionChanging(TreeItem* item, bool selected)
{
	if (!C_SelectionSceneGraphPanel::OnSelectionChanging(item, selected))
		return false;

	if (!m_Layer ||
		((C_SceneTreeItem*)item)->GetType() != E_SceneGraphItemType_DgNode ||
		((C_SceneTreeItemDgNode*)item)->GetDataType() == E_SceneGraphDataType_Level ||
		m_Layer->GetLevel() != EditorLib::C_Core::Instance().GetDgManager().LevelOf(((C_SceneTreeItemDgNode*)item)->GetDgNode()->GetId()))
	{
		return false;
	}

	if (!m_IgnoreEvents)
	{
		if (m_Layer->IsReadOnly())
			return false;

		if (selected)
			EditorLib::C_Core::Instance().GetLayerManager().AddNodeToLayer(m_Layer, const_cast<EditorLib::I_DgNode*>(((C_SceneTreeItemDgNode*)item)->GetDgNode()));
		else
			EditorLib::C_Core::Instance().GetLayerManager().RemoveLayerNode(((C_SceneTreeItemDgNode*)item)->GetDgNode());
	}

	return true;
}

void C_LayerSceneGraphPanel::OnLayerEvent(const EditorLib::I_Layer& layer, EditorLib::E_LayerEvent evt)
{
	if (evt == EditorLib::E_LayerEvent_Removed)
	{
		if (&layer == m_Layer)
		{
			SetLayer(0);
		}
	}
}

void C_LayerSceneGraphPanel::OnLayerNodeAdded(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode)
{
	if (&layer != m_Layer)
		return;

	SceneNodeDataMap::Iterator treeNode = m_NodeDataMap.Find(dgNode.GetId());

	if (treeNode != m_NodeDataMap.End())
	{
		m_IgnoreEvents = true;
		treeNode->GetValue()->SetSelected(true);
		m_IgnoreEvents = false;
	}
}

void C_LayerSceneGraphPanel::OnLayerNodeRemoved(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode)
{
	if (&layer != m_Layer)
		return;

	SceneNodeDataMap::Iterator treeNode = m_NodeDataMap.Find(dgNode.GetId());

	if (treeNode != m_NodeDataMap.End())
	{
		m_IgnoreEvents = true;
		treeNode->GetValue()->SetSelected(false);
		m_IgnoreEvents = false;
	}
}

C_LayerSceneGraphPanel::C_Slots::C_Slots(C_LayerSceneGraphPanel* pLayerSceneGraph)
{
	m_pLayerSceneGraph = pLayerSceneGraph;
}

void C_LayerSceneGraphPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.layerEvent.connect(this, &C_LayerSceneGraphPanel::C_Slots::OnLayerEventSlot);
	s.layerNodeAdded.connect(this, &C_LayerSceneGraphPanel::C_Slots::OnLayerNodeAddedSlot);
	s.layerNodeRemoved.connect(this, &C_LayerSceneGraphPanel::C_Slots::OnLayerNodeRemovedSlot);
}

void C_LayerSceneGraphPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.layerEvent.disconnect(this);
	s.layerNodeAdded.disconnect(this);
	s.layerNodeRemoved.disconnect(this);
}

void C_LayerSceneGraphPanel::C_Slots::OnLayerEventSlot(const EditorLib::I_Layer& layer, EditorLib::E_LayerEvent evt)
{
	m_pLayerSceneGraph->OnLayerEvent(layer, evt);
}

void C_LayerSceneGraphPanel::C_Slots::OnLayerNodeAddedSlot(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode)
{
	m_pLayerSceneGraph->OnLayerNodeAdded(layer, dgNode);
}

void C_LayerSceneGraphPanel::C_Slots::OnLayerNodeRemovedSlot(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode)
{
	m_pLayerSceneGraph->OnLayerNodeRemoved(layer, dgNode);
}