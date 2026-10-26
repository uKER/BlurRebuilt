#include "stdafx.h"

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Frame/Menus/FrameDisplayMenu.h>
#include <EditorApp/Frame/Menus/FrameMenuBar.h>
#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>

#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Tasks/UITasks.h>

#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/PropertyVisibilityManager.h>

namespace Editor
{
Ut::UtMap<S32,Btl::String> C_FrameDisplayMenu::m_ActorFilterMenuIds;

static void ToggleSelectedNodes()
{
	C_RenderPipeline::GlobalRenderState& state = C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState();
	Tasks::SetShowHUDSelectedActors(!Tasks::GetShowHUDSelectedActors());
	state.hudSelectionCount = Tasks::GetShowHUDSelectedActors();
	C_RenderCore::Instance().DirtyAllViewports();
}

static void HideSelected()
{
	C_RenderCore::Instance().GetRenderScene().SetSelectionVisibility(false);
	C_RenderCore::Instance().DirtyAllViewports();
}

static void HideUnselected()
{
	C_RenderCore::Instance().GetRenderScene().SetInverseSelectionVisibility(false);
	C_RenderCore::Instance().DirtyAllViewports();
}

static void HideAll()
{
	C_RenderCore::Instance().GetRenderScene().SetNodeVisibility(false);
	C_RenderCore::Instance().DirtyAllViewports();
}

static void ShowSelected()
{
	C_RenderCore::Instance().GetRenderScene().SetSelectionVisibility(true);
	C_RenderCore::Instance().DirtyAllViewports();
}

static void ShowLastHidden()
{
	C_RenderCore::Instance().GetRenderScene().ShowPreviouslyHiddenNodes();
	C_RenderCore::Instance().DirtyAllViewports();
}

static void ShowAll()
{
	C_RenderCore::Instance().GetRenderScene().SetNodeVisibility(true);
	C_RenderCore::Instance().DirtyAllViewports();
}

static void ToggleLocator()
{
	C_RenderPipeline::GlobalRenderState& state = C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState();
	state.locatorVisible = !state.locatorVisible;
	C_RenderCore::Instance().DirtyAllViewports();
}

static void ToggleSprite()
{
	C_RenderPipeline::GlobalRenderState& state = C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState();
	state.spriteVisible = !state.spriteVisible;
	C_RenderCore::Instance().DirtyAllViewports();
}

static void ToggleMesh()
{
	C_RenderPipeline::GlobalRenderState& state = C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState();
	state.meshVisible = !state.meshVisible;
	C_RenderCore::Instance().DirtyAllViewports();
}

static void OnSelectedNodesUpdate(wxUpdateUIEvent& e)
{
	e.Check(C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState().hudSelectionCount);
	e.Enable(Tasks::IsLevelLoaded());
}

static void OnMenuUpdate(wxUpdateUIEvent& e)
{
	e.Enable(Tasks::IsLevelLoaded());
}

static void OnLocatorUpdate(wxUpdateUIEvent& e)
{
	e.Check(C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState().locatorVisible);
	e.Enable(Tasks::IsLevelLoaded());
}

static void OnSpriteUpdate(wxUpdateUIEvent& e)
{
	e.Check(C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState().spriteVisible);
	e.Enable(Tasks::IsLevelLoaded());
}

static void OnMeshUpdate(wxUpdateUIEvent& e)
{
	e.Check(C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState().meshVisible);
	e.Enable(Tasks::IsLevelLoaded());
}

void C_FrameDisplayMenu::HideProperties()
{
	for (Ut::UtMap<S32,Btl::String>::Iterator it = m_ActorFilterMenuIds.Begin(); it != m_ActorFilterMenuIds.End(); ++it)
	{
		C_PropertyVisibilityData* visibilityData = 
			Editor::C_PropertyVisibilityManager::Instance().GetVisibilityData(it->GetValue());
		visibilityData->SetVisibilityType(Editor::E_PropertyVisibilityType_None);
	}
	C_RenderCore::Instance().DirtyAllViewports();
}

void C_FrameDisplayMenu::ShowPropertiesAll()
{
	for (Ut::UtMap<S32,Btl::String>::Iterator it = m_ActorFilterMenuIds.Begin(); it != m_ActorFilterMenuIds.End(); ++it)
	{
		C_PropertyVisibilityData* visibilityData = 
			Editor::C_PropertyVisibilityManager::Instance().GetVisibilityData(it->GetValue());
		visibilityData->SetVisibilityType(Editor::E_PropertyVisibilityType_All);
	}
	C_RenderCore::Instance().DirtyAllViewports();
}

void C_FrameDisplayMenu::ShowPropertiesSelected()
{
	for (Ut::UtMap<S32,Btl::String>::Iterator it = m_ActorFilterMenuIds.Begin(); it != m_ActorFilterMenuIds.End(); ++it)
	{
		C_PropertyVisibilityData* visibilityData = 
			Editor::C_PropertyVisibilityManager::Instance().GetVisibilityData(it->GetValue());
		visibilityData->SetVisibilityType(Editor::E_PropertyVisibilityType_Selection);
	}
	C_RenderCore::Instance().DirtyAllViewports();
}


void C_FrameDisplayMenu::UpdatePropertyAll(wxUpdateUIEvent& e)
{
	C_PropertyVisibilityData* visibilityData = Editor::C_PropertyVisibilityManager::Instance().GetVisibilityData(m_ActorFilterMenuIds[e.GetId()]);
	e.Check(visibilityData? visibilityData->GetVisibilityType()==Editor::E_PropertyVisibilityType_All : false);
}

void C_FrameDisplayMenu::UpdatePropertySelected(wxUpdateUIEvent& e)
{
	C_PropertyVisibilityData* visibilityData = Editor::C_PropertyVisibilityManager::Instance().GetVisibilityData(m_ActorFilterMenuIds[e.GetId()]);
	e.Check(visibilityData? visibilityData->GetVisibilityType()==Editor::E_PropertyVisibilityType_Selection : false);
}

void C_FrameDisplayMenu::UpdatePropertyNone(wxUpdateUIEvent& e)
{
	C_PropertyVisibilityData* visibilityData = Editor::C_PropertyVisibilityManager::Instance().GetVisibilityData(m_ActorFilterMenuIds[e.GetId()]);
	e.Check(visibilityData? visibilityData->GetVisibilityType()==Editor::E_PropertyVisibilityType_None : false);
}

void C_FrameDisplayMenu::TogglePropertyAll(wxCommandEvent& e)
{
	C_PropertyVisibilityData* visibilityData = Editor::C_PropertyVisibilityManager::Instance().GetVisibilityData(m_ActorFilterMenuIds[e.GetId()]);
	visibilityData->SetVisibilityType(E_PropertyVisibilityType_All);
	C_RenderCore::Instance().DirtyAllViewports();
}

void C_FrameDisplayMenu::TogglePropertySelected(wxCommandEvent& e)
{
	C_PropertyVisibilityData* visibilityData = Editor::C_PropertyVisibilityManager::Instance().GetVisibilityData(m_ActorFilterMenuIds[e.GetId()]);
	visibilityData->SetVisibilityType(E_PropertyVisibilityType_Selection);
	C_RenderCore::Instance().DirtyAllViewports();
}

void C_FrameDisplayMenu::TogglePropertyNone(wxCommandEvent& e)
{
	C_PropertyVisibilityData* visibilityData = Editor::C_PropertyVisibilityManager::Instance().GetVisibilityData(m_ActorFilterMenuIds[e.GetId()]);
	visibilityData->SetVisibilityType(E_PropertyVisibilityType_None);
	C_RenderCore::Instance().DirtyAllViewports();
}

C_FrameDisplayMenu::C_FrameDisplayMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

C_FrameDisplayMenu::~C_FrameDisplayMenu()
{
}

bool C_FrameDisplayMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();
	wxMenu *pMenu = new wxMenu();
	pWxMenuBar->Append(pMenu, _T("Display"));

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Display|Heads Up Display|Selected Actors", pFrame, &ToggleSelectedNodes, &Editor::OnSelectedNodesUpdate, Editor::E_MenuItem_Check);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Display|Separator1", Editor::E_MenuItem_Separator);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Display|Hide Selected Nodes\tCtrl-H", pFrame, &HideSelected, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Display|Hide Unselected Nodes\tAlt-H", pFrame, &HideUnselected, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Display|Hide All Nodes", pFrame, &HideAll, &Editor::OnMenuUpdate);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Display|Separator2", Editor::E_MenuItem_Separator);
	
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Display|Show Selected Nodes\tShift-H", pFrame, &ShowSelected, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Display|Show Last Hidden Nodes\tShift-Ctrl-H", pFrame, &ShowLastHidden, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Display|Show All Nodes", pFrame, &ShowAll, &Editor::OnMenuUpdate);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Display|Separator3", Editor::E_MenuItem_Separator);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Display|Visualisation|Locator", pFrame, &ToggleLocator, &Editor::OnLocatorUpdate, Editor::E_MenuItem_Check);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Display|Visualisation|Sprite", pFrame, &ToggleSprite, &Editor::OnSpriteUpdate, Editor::E_MenuItem_Check);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Display|Visualisation|Mesh", pFrame, &ToggleMesh, &Editor::OnMeshUpdate, Editor::E_MenuItem_Check);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Display|Properties|Hide All", pFrame, &HideProperties, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Display|Properties|Show All", pFrame, &ShowPropertiesAll, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Display|Properties|Show Selected", pFrame, &ShowPropertiesSelected, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Display|Properties|Separator1", Editor::E_MenuItem_Separator);

	Editor::C_PropertyVisibilityManager::Instance().SetMenu(this);

	return true;
}

bool C_FrameDisplayMenu::AddProperty(const Btl::String& displayName, Editor::C_PropertyVisibilityData* pVisibilityData)
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();

	S32 id;
	
	id = m_pMenuBar->GetMenuBuilder()->AddMenuEntry((Btl::String("Display|Properties|")+displayName+Btl::String("|All")).Chars(), 
			pFrame, &TogglePropertyAll, &UpdatePropertyAll, E_MenuItem_Check);
	m_ActorFilterMenuIds[id] = pVisibilityData->GetTypeName();

	id = m_pMenuBar->GetMenuBuilder()->AddMenuEntry((Btl::String("Display|Properties|")+displayName+Btl::String("|Selected")).Chars(), 
			pFrame, &TogglePropertySelected, &UpdatePropertySelected, E_MenuItem_Check);
	m_ActorFilterMenuIds[id] = pVisibilityData->GetTypeName();

	id = m_pMenuBar->GetMenuBuilder()->AddMenuEntry((Btl::String("Display|Properties|")+displayName+Btl::String("|None")).Chars(), 
			pFrame, &TogglePropertyNone, &UpdatePropertyNone, E_MenuItem_Check);
	m_ActorFilterMenuIds[id] = pVisibilityData->GetTypeName();

	return true;
}


}
