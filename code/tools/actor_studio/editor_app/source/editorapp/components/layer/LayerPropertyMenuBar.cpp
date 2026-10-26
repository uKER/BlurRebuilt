#include "stdafx.h"

#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LayerManager.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Core.h>

#include <EditorApp/components/Layer/LayerPropertyMenuBar.h>
#include <EditorApp/components/Layer/LayerPropertyPanel.h>
#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>
#include <EditorApp/Components/PanelMenu.h>
#include <EditorApp/Tasks/LayerTasks.h>
#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Tasks/UITasks.h>

namespace Editor
{


C_LayerPropertyMenuBar::C_LayerPropertyMenuBar(Editor::C_LayerPropertyPanel *pPanel)
: m_pLayerPropertyPanel(pPanel)
, m_pMenuBuilder(0)
{
}

C_LayerPropertyMenuBar::~C_LayerPropertyMenuBar()
{
	delete m_pMenuBuilder;
}

bool C_LayerPropertyMenuBar::InitialiseUI()
{
	C_MenuBar *pMenuBar = new C_MenuBar(m_pLayerPropertyPanel, wxID_ANY);
	m_pMenuBuilder = new C_PanelMenuBuilder(pMenuBar);

	if (m_pLayerPropertyPanel->GetSizer())
		m_pLayerPropertyPanel->GetSizer()->Add(pMenuBar, 0, wxRIGHT|wxLEFT, 0);

	m_pMenuBuilder->AddMenuEntry("Layer|Create New Layer", m_pLayerPropertyPanel, this, &C_LayerPropertyMenuBar::OnCreateLayer, &C_LayerPropertyMenuBar::OnUpdateLevelLoaded, E_MenuItem_Normal);	
	m_pMenuBuilder->AddMenuEntry("Layer|Select Objects in Layers", m_pLayerPropertyPanel, this, &C_LayerPropertyMenuBar::OnSelectLayerObjects, &C_LayerPropertyMenuBar::OnUpdateLayerSelected, E_MenuItem_Normal);

	m_pMenuBuilder->AddMenuEntry("Layer|Separator0", E_MenuItem_Separator);

	m_pMenuBuilder->AddMenuEntry("Layer|Add Selected Objects to Layer", m_pLayerPropertyPanel, this, &C_LayerPropertyMenuBar::OnAddObjectsToLayer, &C_LayerPropertyMenuBar::OnUpdateSingleLayerSelected, E_MenuItem_Normal);
	m_pMenuBuilder->AddMenuEntry("Layer|Remove Selected Objects from Layers", m_pLayerPropertyPanel, this, &C_LayerPropertyMenuBar::OnRemoveObjectsFromLayers, &C_LayerPropertyMenuBar::OnUpdateLayerSelected, E_MenuItem_Normal);

	m_pMenuBuilder->AddMenuEntry("Layer|Separator1", E_MenuItem_Separator);

	m_pMenuBuilder->AddMenuEntry("Layer|Delete Selected Layers", m_pLayerPropertyPanel, this, &C_LayerPropertyMenuBar::OnDeleteSelectedLayers, &C_LayerPropertyMenuBar::OnUpdateLayerSelected, E_MenuItem_Normal);
	m_pMenuBuilder->AddMenuEntry("Layer|Delete All Unused Layers", m_pLayerPropertyPanel, this, &C_LayerPropertyMenuBar::OnDeleteUnusedLayers, &C_LayerPropertyMenuBar::OnUpdateLevelLoaded, E_MenuItem_Normal);

	m_pMenuBuilder->AddMenuEntry("Layer|Separator2", E_MenuItem_Separator);

	m_pMenuBuilder->AddMenuEntry("Layer|Membership...", m_pLayerPropertyPanel, this, &C_LayerPropertyMenuBar::OnShowLayerEditor, &C_LayerPropertyMenuBar::OnUpdateLevelLoaded, E_MenuItem_Normal);
	return true;
}

C_LayerPropertyPanel* C_LayerPropertyMenuBar::GetLayerPropertyPanel()
{
	return m_pLayerPropertyPanel;
}


void C_LayerPropertyMenuBar::OnUpdateLevelLoaded(wxUpdateUIEvent& e)
{
	e.Enable(Tasks::IsLevelLoaded());
}

void C_LayerPropertyMenuBar::OnUpdateLayerSelected(wxUpdateUIEvent& e)
{
	bool selected = false;
	const C_LayerPropertyCtrl *pLayerPropCtrl = m_pLayerPropertyPanel->GetLayerPropertyCtrl();
	const C_LayerPropertyCtrl::LayerDataArray& layerData = pLayerPropCtrl->GetLayerData();

	for (S32 i=0; i<layerData.Size() && !selected; ++i)
		selected = layerData[i].selected;

	e.Enable(selected);
}

void C_LayerPropertyMenuBar::OnUpdateSingleLayerSelected(wxUpdateUIEvent& e)
{
	U32 selected = 0;
	const C_LayerPropertyCtrl *pLayerPropCtrl = m_pLayerPropertyPanel->GetLayerPropertyCtrl();
	const C_LayerPropertyCtrl::LayerDataArray& layerData = pLayerPropCtrl->GetLayerData();

	for (S32 i=0; i<layerData.Size(); ++i)
		if (layerData[i].selected)
			selected++;

	e.Enable(selected == 1);
}

void C_LayerPropertyMenuBar::OnCreateLayer()
{
	Tasks::CreateLayer();
	C_LayerPropertyCtrl *pLayerPropCtrl = m_pLayerPropertyPanel->GetLayerPropertyCtrl();
	pLayerPropCtrl->EditLayerName();
}

void C_LayerPropertyMenuBar::OnSelectLayerObjects()
{
	Ut::Vector<const EditorLib::I_Layer*> layers;

	const C_LayerPropertyCtrl *pLayerPropCtrl = m_pLayerPropertyPanel->GetLayerPropertyCtrl();
	const C_LayerPropertyCtrl::LayerDataArray& layerData = pLayerPropCtrl->GetLayerData();
	for (S32 i=0; i<layerData.Size(); ++i)
		if(layerData[i].selected)
			layers.PushBack(layerData[i].pLayer);

	assert(layers.Size());
	Tasks::SetSelectionFromLayers(layers, wxGetKeyState(WXK_SHIFT) ? "-a" : wxGetKeyState(WXK_CONTROL) ? "-r" : 0);
}

void C_LayerPropertyMenuBar::OnAddObjectsToLayer()
{
	Ut::Vector<EditorLib::I_Layer*> layers;

	const C_LayerPropertyCtrl *pLayerPropCtrl = m_pLayerPropertyPanel->GetLayerPropertyCtrl();
	const C_LayerPropertyCtrl::LayerDataArray& layerData = pLayerPropCtrl->GetLayerData();
	for (S32 i=0; i<layerData.Size(); ++i)
		if(layerData[i].selected)
			layers.PushBack(layerData[i].pLayer);

	assert(layers.Size() == 1);
	Tasks::AddSelectionToLayer(layers[0]);
}

void C_LayerPropertyMenuBar::OnRemoveObjectsFromLayers()
{
	Ut::Vector<EditorLib::I_Layer*> layers;

	const C_LayerPropertyCtrl *pLayerPropCtrl = m_pLayerPropertyPanel->GetLayerPropertyCtrl();
	const C_LayerPropertyCtrl::LayerDataArray& layerData = pLayerPropCtrl->GetLayerData();
	for (S32 i=0; i<layerData.Size(); ++i)
		if(layerData[i].selected)
			layers.PushBack(layerData[i].pLayer);

	assert(layers.Size());
	Tasks::RemoveSelectionFromLayers(layers);
}

void C_LayerPropertyMenuBar::OnDeleteSelectedLayers()
{
	Ut::Vector<EditorLib::I_Layer*> layers;

	const C_LayerPropertyCtrl *pLayerPropCtrl = m_pLayerPropertyPanel->GetLayerPropertyCtrl();
	const C_LayerPropertyCtrl::LayerDataArray& layerData = pLayerPropCtrl->GetLayerData();
	for (S32 i=0; i<layerData.Size(); ++i)
		if(layerData[i].selected)
			layers.PushBack(layerData[i].pLayer);

	assert(layers.Size());
	for (S32 i=0; i<layers.Size(); ++i)
		layers[i]->GetLayerSet()->RemoveLayer(layers[i]);
}

void C_LayerPropertyMenuBar::OnDeleteUnusedLayers()
{
	Tasks::RemoveUnusedLayers();
}

void C_LayerPropertyMenuBar::OnShowLayerEditor()
{
	Tasks::ShowLayerEditWindow();
}






}