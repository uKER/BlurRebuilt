#include <nexus_app/stdafx.h>
#include <nexus_app/Nexus.h>

#include <nexus_app/DockingPanelRight.h>
#include <nexus_app/MainFrm.h>

#include "../../resource.h"
#include <nexus_app/NexusSystem.h>
#include <nexus_app/NexusDoc.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <nexus_app/SceneGraphTree.h>
#include <nexus_app/LayerGrid.h>

using namespace Nexus;

bool DockingPanelRight::Create(CWnd* pMainFrame)
{
	m_pSceneGraphTreeCtrl = new CExtWFF< SceneGraphTree >;
	if (!m_pSceneGraphTreeCtrl)
		return false;

	m_pLayerGridCtrl = new LayerGrid;
	if (!m_pLayerGridCtrl)
		return false;

	if(	!CExtControlBar::Create(
			_T("Display Panel"),
			pMainFrame,
			ID_VIEW_RIGHT_TOOLBAR))
		return false;

	if( !m_tabPageContainerCtrl.Create(
			this,
			CRect(0,0,0,0),
			this->GetDlgCtrlID()))
		return false;

	if (!m_pSceneGraphTreeCtrl->Create(&m_tabPageContainerCtrl))
		return false;

	if (!m_pLayerGridCtrl->Create(&m_tabPageContainerCtrl))
		return false;
	if(	!m_tabPageContainerCtrl.PageInsert(
		m_pSceneGraphTreeCtrl,
		"Scene Graph"))
		return false;

	if(	!m_tabPageContainerCtrl.PageInsert(
		m_pLayerGridCtrl,
		"Layers"))
		return false;

	return true;
}

void DockingPanelRight::Destroy()
{
	if (m_pSceneGraphTreeCtrl)
		m_pSceneGraphTreeCtrl->Destroy();
	delete m_pSceneGraphTreeCtrl;

	if (m_pLayerGridCtrl)
		m_pLayerGridCtrl->Destroy();
	delete m_pLayerGridCtrl;
}

void DockingPanelRight::Update()
{
	m_pSceneGraphTreeCtrl->Update();
	m_pLayerGridCtrl->Update();
}

void DockingPanelRight::Clear()
{
	m_pSceneGraphTreeCtrl->Clear();
	m_pLayerGridCtrl->Clear();
}

SceneGraphTree* DockingPanelRight::GetSceneGraphTreePtr()
{
	return m_pSceneGraphTreeCtrl;
}

const SceneGraphTree* DockingPanelRight::GetSceneGraphTreePtr() const
{
	return m_pSceneGraphTreeCtrl;
}

LayerGrid* DockingPanelRight::GetLayerGridPtr()
{
	return m_pLayerGridCtrl;
}

const LayerGrid* DockingPanelRight::GetLayerGridPtr() const
{
	return m_pLayerGridCtrl;
}
