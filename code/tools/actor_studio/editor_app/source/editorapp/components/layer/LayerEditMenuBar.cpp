#include "stdafx.h"
#include <wx/treectrl.h>
#include <wx/regex.h>

#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LayerManager.h>
#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtTypeDatabaseHelpers.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorApp/Components/Layer/LayerEditMenuBar.h>
#include <EditorApp/Components/Layer/LayerEditPanel.h>

#include <EditorApp/misc/EventProxy.h>
#include <EditorApp/misc/MenuBuilder.h>
#include <EditorApp/Misc/MenuHelper.h>
#include <EditorApp/Components/PanelMenu.h>
#include <EditorApp/Components/scene_graph/SceneGraphFilter.h>

#include <EditorApp/Tasks/LayerTasks.h>
#include <EditorApp/Tasks/LevelTasks.h>

namespace Editor
{

C_LayerEditMenuBar::C_LayerEditMenuBar(C_LayerEditPanel *pEditPanel)
: m_pLayerEditPanel(pEditPanel)
, m_pMenuBuilder(0)
{
}

C_LayerEditMenuBar::~C_LayerEditMenuBar()
{
	SAFE_DELETE_PTR(m_pMenuBuilder);
}

bool C_LayerEditMenuBar::InitialiseUI()
{
	C_MenuBar *pMenuBar = new C_MenuBar(m_pLayerEditPanel, wxID_ANY);
	m_pMenuBuilder = new C_PanelMenuBuilder(pMenuBar);

	if (m_pLayerEditPanel->GetSizer())
		m_pLayerEditPanel->GetSizer()->Add(pMenuBar, 0, wxRIGHT|wxLEFT, 0);

	S32 id = -1;
	id = m_pMenuBuilder->AddMenuEntry("Layer|Create Layer",
		m_pLayerEditPanel, this, &C_LayerEditMenuBar::OnCreateLayer, &C_LayerEditMenuBar::CanCreateLayer);

	id = m_pMenuBuilder->AddMenuEntry("Layer|Delete Selected Layer",
		m_pLayerEditPanel, this, &C_LayerEditMenuBar::OnDeleteLayer, &C_LayerEditMenuBar::IsLayerSelected);

	m_pMenuBuilder->AddMenuEntry("Layer|Separator0", E_MenuItem_Separator);
	
	id = m_pMenuBuilder->AddMenuEntry("Layer|Add Selection To Layer",
		m_pLayerEditPanel, this, &C_LayerEditMenuBar::OnAddSelection, &C_LayerEditMenuBar::IsLayerSelected);

	return true;
}

C_LayerEditPanel* C_LayerEditMenuBar::GetLayerEditPanel()
{
	return m_pLayerEditPanel;
}

C_MenuBuilderBase* C_LayerEditMenuBar::GetMenuBuilder()
{
	return m_pMenuBuilder;
}


void C_LayerEditMenuBar::OnCreateLayer()
{
	Tasks::CreateLayer();
}

void C_LayerEditMenuBar::CanCreateLayer(wxUpdateUIEvent& e)
{
	e.Enable(Tasks::IsLevelLoaded());
}

void C_LayerEditMenuBar::OnDeleteLayer()
{
	const EditorLib::I_Layer *pLayer = m_pLayerEditPanel->GetSelectedLayer();
	if (pLayer && pLayer->GetType() == EditorLib::E_LayerType_Actor)
		const_cast<EditorLib::I_Layer*>(pLayer)->GetLayerSet()->RemoveLayer(const_cast<EditorLib::I_Layer*>(pLayer));
	else
		LOGERROR(g_Log, "Unable to delete layer '%s'", pLayer ? pLayer->GetName().Chars() : "");
}

void C_LayerEditMenuBar::OnAddSelection()
{
	EditorLib::I_Layer *pLayer = const_cast<EditorLib::I_Layer*>(m_pLayerEditPanel->GetSelectedLayer());
	if (pLayer)
		Tasks::AddSelectionToLayer(pLayer);
}

void C_LayerEditMenuBar::IsLayerSelected(wxUpdateUIEvent& e)
{
	e.Enable(m_pLayerEditPanel->GetSelectedLayer() != 0);
}

}
