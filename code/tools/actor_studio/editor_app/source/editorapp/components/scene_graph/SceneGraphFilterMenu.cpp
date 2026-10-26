#include "stdafx.h"

#include <wx/treectrl.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>

#include <xtwin/TreeCtrl.h>

#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtTypeDatabaseHelpers.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/misc/EventProxy.h>
#include <EditorApp/misc/MenuBuilder.h>
#include <EditorApp/Misc/MenuHelper.h>

#include <EditorApp/Components/Scene_Graph/SceneGraphFilter.h>
#include <EditorApp/Components/Scene_Graph/SceneGraphFilterMenu.h>
#include <EditorApp/Components/Scene_Graph/SceneGraphMenuBar.h>
#include <EditorApp/Components/Scene_Graph/SceneGraphPanel.h>
#include <EditorApp/Components/PanelMenu.h>

using namespace Editor;

C_SceneGraphFilterMenu::C_SceneGraphFilterMenu(C_SceneGraphMenuBar *pMenuBar, C_SceneGraphFilter* pSceneFilter)
: m_pMenuBar(pMenuBar),
m_Slots(this)
{
	m_pSceneFilter = pSceneFilter;
	m_Slots.Connect();
}

C_SceneGraphFilterMenu::~C_SceneGraphFilterMenu()
{
	//m_Slots.Disconnect();
}

bool C_SceneGraphFilterMenu::InitialiseUI()
{
	C_MenuBuilderBase *pMenuBuilder = m_pMenuBar->GetMenuBuilder();
	S32 id = -1;

	id = pMenuBuilder->AddMenuEntry("Sort|Placement",
		m_pMenuBar->GetSceneGraphPanel(), this, 
		&C_SceneGraphFilterMenu::OnSetSortType, &C_SceneGraphFilterMenu::OnUpdateSortFilter,
		E_MenuItem_Radio);
	m_SortPlacementId = id;

	id = pMenuBuilder->AddMenuEntry("Sort|Alphabetical Ascending",
		m_pMenuBar->GetSceneGraphPanel(), this, 
		&C_SceneGraphFilterMenu::OnSetSortType, &C_SceneGraphFilterMenu::OnUpdateSortFilter,
		E_MenuItem_Radio);
	m_SortAlphaAscId = id;

	id = pMenuBuilder->AddMenuEntry("Sort|Alphabetical Descending",
		m_pMenuBar->GetSceneGraphPanel(), this, 
		&C_SceneGraphFilterMenu::OnSetSortType, &C_SceneGraphFilterMenu::OnUpdateSortFilter,
		E_MenuItem_Radio);
	m_SortAlphaDescId = id;

	id = pMenuBuilder->AddMenuEntry("Sort|By Type Ascending",
		m_pMenuBar->GetSceneGraphPanel(), this, 
		&C_SceneGraphFilterMenu::OnSetSortType, &C_SceneGraphFilterMenu::OnUpdateSortFilter,
		E_MenuItem_Radio);
	m_SortTypeAscId = id;

	id = pMenuBuilder->AddMenuEntry("Sort|By Type Descending",
		m_pMenuBar->GetSceneGraphPanel(), this, 
		&C_SceneGraphFilterMenu::OnSetSortType, &C_SceneGraphFilterMenu::OnUpdateSortFilter,
		E_MenuItem_Radio);
	m_SortTypeDescId = id;

	id = pMenuBuilder->AddMenuEntry("Global|Clear All", 
		m_pMenuBar->GetSceneGraphPanel(), this, 
		&C_SceneGraphFilterMenu::OnSetGlobalFilter, &C_SceneGraphFilterMenu::OnUpdateNull,
		E_MenuItem_Normal);
	m_ClearGlobalId = id;

	id = pMenuBuilder->AddMenuEntry("Global|Separator", 
		m_pMenuBar->GetSceneGraphPanel(), this, 
		&C_SceneGraphFilterMenu::OnSetGlobalFilter, &C_SceneGraphFilterMenu::OnUpdateNull,
		E_MenuItem_Separator);	

	id = pMenuBuilder->AddMenuEntry("Global|Actors", 
		m_pMenuBar->GetSceneGraphPanel(), this, 
		&C_SceneGraphFilterMenu::OnSetGlobalFilter, &C_SceneGraphFilterMenu::OnUpdateGlobalFilter,
		E_MenuItem_Check);
	m_GlobalFilterMenuIds[id] = E_SceneGraphDataType_Actor;
	m_GlobalFilterTypes[E_SceneGraphDataType_Actor] = false;

	id = pMenuBuilder->AddMenuEntry("Global|References", 
		m_pMenuBar->GetSceneGraphPanel(), this, 
		&C_SceneGraphFilterMenu::OnSetGlobalFilter, &C_SceneGraphFilterMenu::OnUpdateGlobalFilter,
		E_MenuItem_Check);
	m_GlobalFilterMenuIds[id] = E_SceneGraphDataType_Reference;
	m_GlobalFilterTypes[E_SceneGraphDataType_Reference] = false;

	id = pMenuBuilder->AddMenuEntry("Global|Groups", 
		m_pMenuBar->GetSceneGraphPanel(), this, 
		&C_SceneGraphFilterMenu::OnSetGlobalFilter, &C_SceneGraphFilterMenu::OnUpdateGlobalFilter,
		E_MenuItem_Check);
	m_GlobalFilterMenuIds[id] = E_SceneGraphDataType_SceneNode;
	m_GlobalFilterTypes[E_SceneGraphDataType_SceneNode] = false;

	wxMenu* filterMenu = new wxMenu();
	m_pMenuBar->GetMenuBar()->Append(filterMenu, "Filter");
	wxMenu* actorFilterMenu = new wxMenu();
	filterMenu->AppendSubMenu(actorFilterMenu, "Actors");

	return true;
}

void C_SceneGraphFilterMenu::OnDatabaseEvent(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt)
{
	int filterItemId = m_pMenuBar->GetMenuBar()->FindMenu("Filter");
	if (filterItemId == wxNOT_FOUND)
		return;

	wxMenu* filterMenu = m_pMenuBar->GetMenuBar()->GetMenu(filterItemId);
	if (filterMenu == NULL)
		return;

	int actorItemId = filterMenu->FindItem("Actors");
	if (actorItemId == wxNOT_FOUND)
		return;

	wxMenuItem* actorMenuItem = filterMenu->FindItem(actorItemId);
	if (actorMenuItem == NULL)
		return;

	if (actorMenuItem->IsSubMenu() == false)
		return;

	wxMenu* actorMenu = (wxMenu*)actorMenuItem->GetSubMenu();

	if (evt == EditorLib::E_DatabaseEvent_Opened)
	{
		C_MenuBuilderBase *pMenuBuilder = m_pMenuBar->GetMenuBuilder();
		S32 id = -1;

		id = pMenuBuilder->AddMenuEntry("Filter|Actors|Clear All", 
			m_pMenuBar->GetSceneGraphPanel(), this, 
			&C_SceneGraphFilterMenu::OnSetActorFilter, &C_SceneGraphFilterMenu::OnUpdateNull,
			E_MenuItem_Normal);
		m_ClearActorsId = id;

		pMenuBuilder->AddMenuEntry("Filter|Actors|Separator", E_MenuItem_Separator);

		typedef Ut::UtMap<Btl::String, const LtStruct*> ActorTypeMap;
		ActorTypeMap actorTypes;
		{
			Btl::Vector<const EditorLib::I_XtType*> types;
			EditorLib::C_Core::Instance().GetXtTypeDatabase().BuildTypeList(types);

			for (S32 i = 0; i < types.Size(); i++)
			{
				const EditorLib::I_XtType* pType = types[i];
				if (pType->IsA(EditorLib::E_XtType_SceneXtActorNode))
				{
					assert(pType->GetXtType()->kind == kLtStruct);
					const LtStruct* pLtStruct = (LtStruct*)pType->GetXtType();
					//Ignore types that are hidden.
					if (pLtStruct->hidden)
						continue;
					actorTypes.Insert(
						ActorTypeMap::DataType(
						Btl::String((const char*)pLtStruct->label), pLtStruct));
				}
			}
		}

		for (ActorTypeMap::Iterator it = actorTypes.Begin(); 
			it != actorTypes.End(); ++it)
		{
			LtStruct* pLtStruct = const_cast<LtStruct*>(it->GetValue());
			Btl::String menu;

			if (pLtStruct->attrs.GetString("SceneView", "menucategory") != "")
				menu.Printf("Filter|Actors|%s|%s", pLtStruct->attrs.GetString("SceneView", "menucategory").Text(), (const char*)pLtStruct->label);
			else
				menu.Printf("Filter|Actors|%s", (const char*)pLtStruct->label);

			S32 id = m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
				menu.Chars(), 
				m_pMenuBar->GetSceneGraphPanel(), 
				this, 
				&C_SceneGraphFilterMenu::OnSetActorFilter,
				&C_SceneGraphFilterMenu::OnUpdateActorFilter,
				E_MenuItem_Check);

			m_ActorFilterMenuIds[id] = (const char*)pLtStruct->name;
			m_ActorFilterTypes[(const char*)pLtStruct->name] = false;
		}

		MenuHelper::Sort(actorMenu, 2);
	} else if (evt == EditorLib::E_DatabaseEvent_Closing)
	{
		while (actorMenu->GetMenuItemCount())
		{
			wxMenuItem* item = actorMenu->FindItemByPosition(0);
			actorMenu->Destroy(item);
		}
	}
}

void C_SceneGraphFilterMenu::OnUpdateNull(wxUpdateUIEvent& e)
{
}

void C_SceneGraphFilterMenu::OnSetSortType(wxCommandEvent& e)
{
	if (e.GetId() == m_SortPlacementId) {
		m_pSceneFilter->SetSortFilter(C_SceneGraphFilter::E_SortFilter_Placement);
	} else if (e.GetId() == m_SortAlphaAscId) {
		m_pSceneFilter->SetSortFilter(C_SceneGraphFilter::E_SortFilter_AlphaAsc);
	} else if (e.GetId() == m_SortAlphaDescId) {
		m_pSceneFilter->SetSortFilter(C_SceneGraphFilter::E_SortFilter_AlphaDesc);
	} else if (e.GetId() == m_SortTypeAscId) {
		m_pSceneFilter->SetSortFilter(C_SceneGraphFilter::E_SortFilter_TypeAsc);
	} else if (e.GetId() == m_SortTypeDescId) {
		m_pSceneFilter->SetSortFilter(C_SceneGraphFilter::E_SortFilter_TypeDesc);
	}

	m_pMenuBar->GetSceneGraphPanel()->OnFilterMenuUpdate();
}

void C_SceneGraphFilterMenu::OnSetGlobalFilter(wxCommandEvent& e)
{
	if (e.GetId() == m_ClearGlobalId) {
		for (Ut::UtMap<S32,E_SceneGraphDataType>::Iterator i = m_GlobalFilterMenuIds.Begin(); i!=m_GlobalFilterMenuIds.End(); ++i) {
			m_GlobalFilterTypes[i->GetValue()] =  false;
			m_pSceneFilter->SetGlobalFilter(i->GetValue(),false);
		}
	} else {
		m_GlobalFilterTypes[m_GlobalFilterMenuIds[e.GetId()]] =
			!m_GlobalFilterTypes[m_GlobalFilterMenuIds[e.GetId()]];

		m_pSceneFilter->SetGlobalFilter(m_GlobalFilterMenuIds[e.GetId()],m_GlobalFilterTypes[m_GlobalFilterMenuIds[e.GetId()]]);
	}

	m_pMenuBar->GetSceneGraphPanel()->OnFilterMenuUpdate();
}

void C_SceneGraphFilterMenu::OnSetActorFilter(wxCommandEvent& e)
{
	if (e.GetId() == m_ClearActorsId) {
		for (Ut::UtMap<S32,Btl::String>::Iterator i = m_ActorFilterMenuIds.Begin(); i!=m_ActorFilterMenuIds.End(); ++i) {
			m_ActorFilterTypes[i->GetValue()] =  false;
			m_pSceneFilter->SetActorFilter(i->GetValue(),false);
		}
	} else {
		m_ActorFilterTypes[m_ActorFilterMenuIds[e.GetId()]] =
			!m_ActorFilterTypes[m_ActorFilterMenuIds[e.GetId()]];

		m_pSceneFilter->SetActorFilter(m_ActorFilterMenuIds[e.GetId()],m_ActorFilterTypes[m_ActorFilterMenuIds[e.GetId()]]);
	}

	m_pMenuBar->GetSceneGraphPanel()->OnFilterMenuUpdate();
}

void C_SceneGraphFilterMenu::OnUpdateGlobalFilter(wxUpdateUIEvent& e)
{
	if (m_GlobalFilterMenuIds.Find(e.GetId()) != m_GlobalFilterMenuIds.End()) {
		e.Check(m_GlobalFilterTypes[m_GlobalFilterMenuIds[e.GetId()]]);
	}
}

void C_SceneGraphFilterMenu::OnUpdateActorFilter(wxUpdateUIEvent& e)
{
	if (m_ActorFilterMenuIds.Find(e.GetId()) != m_ActorFilterMenuIds.End()) {
		e.Check(m_ActorFilterTypes[m_ActorFilterMenuIds[e.GetId()]]);
	}
}

void C_SceneGraphFilterMenu::OnUpdateSortFilter(wxUpdateUIEvent& e)
{
	if (e.GetId() == m_SortAlphaAscId) {
		if (m_pSceneFilter->GetSortFilter() == C_SceneGraphFilter::E_SortFilter_AlphaAsc)
			e.Check(true);
		else
			e.Check(false);
	}

	if (e.GetId() == m_SortAlphaDescId) {
		if (m_pSceneFilter->GetSortFilter() == C_SceneGraphFilter::E_SortFilter_AlphaDesc)
			e.Check(true);
		else
			e.Check(false);
	}

	if (e.GetId() == m_SortPlacementId) {
		if (m_pSceneFilter->GetSortFilter() == C_SceneGraphFilter::E_SortFilter_Placement)
			e.Check(true);
		else
			e.Check(false);
	}

	if (e.GetId() == m_SortTypeAscId) {
		if (m_pSceneFilter->GetSortFilter() == C_SceneGraphFilter::E_SortFilter_TypeAsc)
			e.Check(true);
		else
			e.Check(false);
	}

	if (e.GetId() == m_SortTypeDescId) {
		if (m_pSceneFilter->GetSortFilter() == C_SceneGraphFilter::E_SortFilter_TypeDesc)
			e.Check(true);
		else
			e.Check(false);
	}
}

C_SceneGraphFilterMenu::C_Slots::C_Slots(C_SceneGraphFilterMenu *pSceneGraphFilterMenu)
: m_pSceneGraphFilterMenu(pSceneGraphFilterMenu)
{
}

void C_SceneGraphFilterMenu::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.connect(this, &Editor::C_SceneGraphFilterMenu::C_Slots::OnDatabaseEventSlot);
}

void C_SceneGraphFilterMenu::C_Slots::OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt)
{
	m_pSceneGraphFilterMenu->OnDatabaseEvent(db, evt);
}

void C_SceneGraphFilterMenu::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.disconnect(this);
}
