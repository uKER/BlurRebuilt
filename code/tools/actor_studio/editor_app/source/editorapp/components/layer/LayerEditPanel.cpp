#include "stdafx.h"

#include <wx/treectrl.h>
#include <wx/regex.h>
#include <wx/dir.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/SceneAftkNode.h>
#include <EditorLib/Dg/SceneNodeIterator.h>
#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorApp/Components/Layer/LayerEditPanel.h>
#include <EditorApp/Components/Layer/LayerEditMenuBar.h>
#include <EditorApp/Components/Scene_Graph/LayerSceneGraphPanel.h>
#include <EditorApp/Components/PanelMenu.h>
#include <EditorApp/Components/SplitterControl.h>
#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Tasks/LayerTasks.h>
#include <EditorApp/App.h>
#include <EditorApp/Config.h>

namespace Editor
{

C_LayerEditTree::C_LayerEditTree(C_LayerEditPanel* panel)
{
	m_Panel = panel;
	m_SelectedLayer = 0;
	
	GetFlags().AllowMultipleSelection = false;
	GetFlags().HasIcons = true;
	GetFlags().HighlightSelectedRow = true;
}

C_LayerEditTreeLevelItem* C_LayerEditTree::GetLevelItem(const EditorLib::I_Level* level)
{
	C_LayerEditTreeLevelItem* levelItem = (C_LayerEditTreeLevelItem*)GetRootNode()->GetFirstChild();
	while (levelItem)
	{
		if (levelItem->GetLevel() == level)
			return levelItem;
		
		levelItem = (C_LayerEditTreeLevelItem*)levelItem->GetNextSibling();
	}

	return 0;
}

C_LayerEditTreeLayerItem* C_LayerEditTree::GetLayerItem(const EditorLib::I_Layer* layer)
{
	C_LayerEditTreeLevelItem* levelItem = (C_LayerEditTreeLevelItem*)GetRootNode()->GetFirstChild();
	while (levelItem)
	{
		C_LayerEditTreeLayerItem* layerItem = (C_LayerEditTreeLayerItem*)levelItem->GetFirstChild();

		while (layerItem)
		{
			if (layerItem->GetLayer() == layer)
				return layerItem;

			layerItem = (C_LayerEditTreeLayerItem*)layerItem->GetNextSibling();
		}
		
		levelItem = (C_LayerEditTreeLevelItem*)levelItem->GetNextSibling();
	}

	return 0;
}

EditorLib::I_Layer* C_LayerEditTree::GetSelectedLayer()
{
	return m_SelectedLayer;
}

void C_LayerEditTree::OnSelectionChanged(const XtUt::Array<TreeItem*>& selection, TcSelectionType selectionType)
{
	BIZ_ASSERT(selection.count <= 1);

	if (selection.count)
		m_SelectedLayer = ((C_LayerEditTreeLayerItem*)selection[0])->GetLayer();
	else
		m_SelectedLayer = 0;

	m_Panel->m_pNodeTree->SetLayer(m_SelectedLayer);
}

C_LayerEditTreeLevelItem::C_LayerEditTreeLevelItem(const EditorLib::I_Level* level)
{
	m_Level = level;
	UpdateName();
}

const EditorLib::I_Level* C_LayerEditTreeLevelItem::GetLevel() const
{
	return m_Level;
}

void C_LayerEditTreeLevelItem::UpdateName()
{
	SetLabel(m_Level->GetName().Chars());
}

C_LayerEditTreeLayerItem::C_LayerEditTreeLayerItem(EditorLib::I_Layer* layer)
{
	m_Layer = layer;
	UpdateName();
}

EditorLib::I_Layer* C_LayerEditTreeLayerItem::GetLayer()
{
	return m_Layer;
}

void C_LayerEditTreeLayerItem::UpdateName()
{
	SetLabel(m_Layer->GetName().Chars());
}

C_LayerEditTreeDgNodeItem::C_LayerEditTreeDgNodeItem(const EditorLib::I_DgNode* dgNode)
{
	m_DgNode = dgNode;
	UpdateName();
}

const EditorLib::I_DgNode* C_LayerEditTreeDgNodeItem::GetDgNode() const
{
	return m_DgNode;
}

void C_LayerEditTreeDgNodeItem::UpdateName()
{
	SetLabel(m_DgNode->GetName().Chars());
}

C_LayerEditPanel::C_Slots::C_Slots(C_LayerEditPanel *pLayerEdit)
: m_pLayerEditPanel(pLayerEdit)
{
}

void C_LayerEditPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelNameChanged.connect(this, &C_LayerEditPanel::C_Slots::OnLevelNameChangedSlot);
	s.levelOpening.connect(this, &C_LayerEditPanel::C_Slots::OnLevelOpeningSlot);
	s.levelOpened.connect(this, &C_LayerEditPanel::C_Slots::OnLevelOpenedSlot);
	s.levelClosing.connect(this, &C_LayerEditPanel::C_Slots::OnLevelClosingSlot);
	s.levelClosed.connect(this, &C_LayerEditPanel::C_Slots::OnLevelClosedSlot);

	s.layerEvent.connect(this, &C_LayerEditPanel::C_Slots::OnLayerEventSlot);

	s.layerNodeAdded.connect(this, &C_LayerEditPanel::C_Slots::OnLayerNodeAddedSlot);
	s.layerNodeRemoved.connect(this, &C_LayerEditPanel::C_Slots::OnLayerNodeRemovedSlot);
}

void C_LayerEditPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelNameChanged.disconnect(this);
	s.levelOpening.disconnect(this);
	s.levelOpened.disconnect(this);
	s.levelClosing.disconnect(this);
	s.levelClosed.disconnect(this);

	s.layerEvent.disconnect(this);
	
	s.layerNodeAdded.disconnect(this);
	s.layerNodeRemoved.disconnect(this);
}

void C_LayerEditPanel::C_Slots::OnLevelNameChangedSlot(const EditorLib::I_Level& level)
{
	m_pLayerEditPanel->OnLevelNameChanged(level);
}

void C_LayerEditPanel::C_Slots::OnLevelOpeningSlot(const EditorLib::I_Level& level)
{
	m_pLayerEditPanel->OnLevelOpening(level);
}

void C_LayerEditPanel::C_Slots::OnLevelOpenedSlot(const EditorLib::I_Level& level)
{
	m_pLayerEditPanel->OnLevelOpened(level);
}

void C_LayerEditPanel::C_Slots::OnLevelClosingSlot(const EditorLib::I_Level& level)
{
	m_pLayerEditPanel->OnLevelClosing(level);
}

void C_LayerEditPanel::C_Slots::OnLevelClosedSlot(const EditorLib::I_Level& level)
{
	m_pLayerEditPanel->OnLevelClosed(level);
}

void C_LayerEditPanel::C_Slots::OnLayerEventSlot(const EditorLib::I_Layer& layer, EditorLib::E_LayerEvent e)
{
	if (e == EditorLib::E_LayerEvent_Added)
		m_pLayerEditPanel->OnLayerAdded(layer);
	else if (e == EditorLib::E_LayerEvent_Removed)
		m_pLayerEditPanel->OnLayerRemoved(layer);
	else if (e == EditorLib::E_LayerEvent_Deserialised)
		m_pLayerEditPanel->OnLayerDeserialised(layer);
}

void C_LayerEditPanel::C_Slots::OnLayerNodeAddedSlot(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode)
{
	m_pLayerEditPanel->OnLayerNodeAdded(layer, dgNode);
}

void C_LayerEditPanel::C_Slots::OnLayerNodeRemovedSlot(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode)
{
	m_pLayerEditPanel->OnLayerNodeRemoved(layer, dgNode);
}

C_LayerEditPanel::C_LayerEditPanel(wxWindow *parent, wxWindowID winid, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
: wxPanel(parent, winid, pos, size, style, name)
, m_Slots(this)
{
	wxBoxSizer* pSizer;
	pSizer = new wxBoxSizer( wxVERTICAL );
	SetSizer(pSizer);
	
	m_pMenuBar = new C_LayerEditMenuBar(this);
	m_pMenuBar->InitialiseUI();

	m_pSplitterCtrl = new wxSplitterCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_NOBORDER);
	
	wxPanel* pPanel2;

	//layer panel
	{
		m_pLayerTreePanel = new wxPanel(m_pSplitterCtrl, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
		m_pLayerTreePanel->SetBackgroundColour(wxColor(255, 255, 255));

		m_pLayerTree = new C_LayerEditTree(this);
		m_pLayerTree->Create((HWND)m_pLayerTreePanel->GetHandle());

		m_pLayerTree->SetRootNode(new TreeItem());
	}

	//node panel
	{
		pPanel2 = new wxPanel(m_pSplitterCtrl, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
		wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);

		m_pNodeTree = new C_LayerSceneGraphPanel(pPanel2);
		pPanelSizer->Add(m_pNodeTree, 1, wxEXPAND, 0);

		pPanel2->SetSizer(pPanelSizer);
	}

	m_pSplitterCtrl->Split(E_SplitterLayout_2Vertical, m_pLayerTreePanel, pPanel2);
	m_pSplitterCtrl->SetSplitterProportion(5000);
	pSizer->Add(m_pSplitterCtrl, 1, wxEXPAND, 0);

	Layout();

	WinRect rcClient((HWND)m_pLayerTreePanel->GetHandle());
	SetWindowPos(*m_pLayerTree, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	
	m_Slots.Connect();

	m_pLayerTreePanel->Connect(wxEVT_SIZE, wxSizeEventHandler(C_LayerEditPanel::OnSize), NULL, this);

	m_Flags.RefreshColours = false;
	m_Flags.Connected = true;
}

C_LayerEditPanel::~C_LayerEditPanel()
{
	Shutdown();
}

void C_LayerEditPanel::Shutdown()
{
	if (m_Flags.Connected)
	{
		m_Slots.Disconnect();

		m_pLayerTreePanel->Disconnect(wxEVT_SIZE, wxSizeEventHandler(C_LayerEditPanel::OnSize));

		SAFE_DELETE_PTR(m_pLayerTree);

		m_pNodeTree->Shutdown();
		SAFE_DELETE_PTR(m_pNodeTree);

		SAFE_DELETE_PTR(m_pMenuBar);

		m_Flags.Connected = false;
	}
}

EditorLib::I_Layer* C_LayerEditPanel::GetSelectedLayer()
{
	return m_pLayerTree->GetSelectedLayer();
}

void C_LayerEditPanel::OnLevelNameChanged(const EditorLib::I_Level& level)
{
	C_LayerEditTreeLevelItem* levelItem = m_pLayerTree->GetLevelItem(&level);

	levelItem->UpdateName();
}

void C_LayerEditPanel::OnLevelOpening(const EditorLib::I_Level& level)
{
	C_LayerEditTreeLevelItem* item = new C_LayerEditTreeLevelItem(&level);
	item->SetExpanded(true);
	m_pLayerTree->GetRootNode()->AddChild(item);
}

void C_LayerEditPanel::OnLevelOpened(const EditorLib::I_Level& level)
{
	C_LayerEditTreeLevelItem* levelItem = m_pLayerTree->GetLevelItem(&level);

	levelItem->UpdateName();
}

void C_LayerEditPanel::OnLevelClosing(const EditorLib::I_Level& level)
{
	C_LayerEditTreeLevelItem* levelItem = m_pLayerTree->GetLevelItem(&level);

	if (levelItem)
		m_pLayerTree->RemoveItem(levelItem);
}

void C_LayerEditPanel::OnLevelClosed(const EditorLib::I_Level& level)
{
}

void C_LayerEditPanel::OnLayerAdded(const EditorLib::I_Layer& layer)
{
	C_LayerEditTreeLevelItem* level = m_pLayerTree->GetLevelItem(layer.GetLevel());

	if (!level)
	{
		level = new C_LayerEditTreeLevelItem(layer.GetLevel());
		level->SetExpanded(true);
		m_pLayerTree->GetRootNode()->AddChild(level);
	}

	C_LayerEditTreeLayerItem* item = new C_LayerEditTreeLayerItem(const_cast<EditorLib::I_Layer*>(&layer));
	level->AddChild(item);
}

void C_LayerEditPanel::OnLayerRemoved(const EditorLib::I_Layer& layer)
{
	C_LayerEditTreeLayerItem* item = m_pLayerTree->GetLayerItem(&layer);
	
	if (item)
		m_pLayerTree->RemoveItem(item);

	m_Flags.RefreshColours = true;
}

void C_LayerEditPanel::OnLayerDeserialised(const EditorLib::I_Layer &layer)
{
	C_LayerEditTreeLayerItem* item = m_pLayerTree->GetLayerItem(&layer);
	
	if (item)
		item->UpdateName();
}

void C_LayerEditPanel::OnLayerNodeAdded(const EditorLib::I_Layer &layer, const EditorLib::I_DgNode &dgNode)
{
	C_LayerEditTreeLayerItem* layerItem = m_pLayerTree->GetLayerItem(&layer);
	
	if (!layerItem)
		return;

	C_LayerEditTreeDgNodeItem* dgNodeItem = new C_LayerEditTreeDgNodeItem(&dgNode);
	layerItem->AddChild(dgNodeItem);
}

void C_LayerEditPanel::OnLayerNodeRemoved(const EditorLib::I_Layer &layer, const EditorLib::I_DgNode &dgNode)
{
	C_LayerEditTreeLayerItem* layerItem = m_pLayerTree->GetLayerItem(&layer);

	C_LayerEditTreeDgNodeItem* dgNodeItem = (C_LayerEditTreeDgNodeItem*)layerItem->GetFirstChild();

	while (dgNodeItem)
	{
		if (dgNodeItem->GetDgNode() == &dgNode)
		{
			m_pLayerTree->RemoveItem(dgNodeItem);
			return;
		}

		dgNodeItem = (C_LayerEditTreeDgNodeItem*)dgNodeItem->GetNextSibling();
	}
}

void C_LayerEditPanel::OnSize(wxSizeEvent& e)
{
	WinRect rcClient((HWND)m_pLayerTreePanel->GetHandle());
	SetWindowPos(*m_pLayerTree, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

}
