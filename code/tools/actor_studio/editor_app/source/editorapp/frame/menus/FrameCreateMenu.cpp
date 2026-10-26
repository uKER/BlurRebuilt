#include "stdafx.h"
#include <algorithm>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Context/EditorContext.h>

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>

#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDataBase.h>
#include <EditorLib/Xt/XtTypeDatabaseHelpers.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Editor/Manipulators/ActorCreateManipulator.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Frame/Menus/FrameCreateMenu.h>
#include <EditorApp/Frame/Menus/FrameMenuBar.h>
#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>
#include <EditorApp/Misc/MenuHelper.h>

namespace Editor
{

C_FrameCreateMenu::C_FrameCreateMenu(C_FrameMenuBar* pMenuBar, wxMenuBar* pWxMenuBar) :
m_Slots(this)
{
	m_pMenuBar = pMenuBar;
	m_pWxMenuBar = pWxMenuBar;

	m_Slots.Connect();
}

C_FrameCreateMenu::~C_FrameCreateMenu()
{
	//m_Slots.Disconnect();
}

bool C_FrameCreateMenu::CreateDynamicMenu(const char* menuName, EditorLib::E_XtType xtBaseType)
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();

	wxMenu* pMenu;
	int createItemId = m_pWxMenuBar->FindMenu(menuName);
	if (createItemId == wxNOT_FOUND)
	{
		pMenu = new wxMenu();
		pWxMenuBar->Append(pMenu, _T(menuName));
	}
	else
		pMenu = m_pWxMenuBar->GetMenu(createItemId);

	typedef Ut::UtMap<Btl::String, const LtStruct*> ActorTypeMap;
	ActorTypeMap actorTypes;
	{
		Btl::Vector<const EditorLib::I_XtType*> types;
		EditorLib::C_Core::Instance().GetXtTypeDatabase().BuildTypeList(types);

		for (S32 i = 0; i < types.Size(); i++)
		{
			const EditorLib::I_XtType* pType = types[i];
			if (pType->IsA(xtBaseType))
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
		//Use friendly label.
		if (pLtStruct->attrs.GetString("SceneView", "menucategory") != "")
			menu.Printf("%s|%s|%s", menuName, pLtStruct->attrs.GetString("SceneView", "menucategory").Text(), (const char*)pLtStruct->label);
		else
			menu.Printf("%s|%s", menuName, (const char*)pLtStruct->label);

		S32 id = m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
			menu.Chars(), 
			pFrame, 
			this, 
			&C_FrameCreateMenu::OnCreateActor, 
			&C_FrameCreateMenu::OnMenuUpdate);
		m_ActorCreationMenuIds[id] = (const char*)pLtStruct->name;
	}

	return true;
}

bool C_FrameCreateMenu::InitialiseUI()
{
	wxMenu* createMenu = new wxMenu();
	m_pWxMenuBar->Append(createMenu, "Create");
	
	return true;
}

void C_FrameCreateMenu::OnDatabaseEvent(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt)
{
	int createItemId = m_pWxMenuBar->FindMenu("Create");
	if (createItemId == wxNOT_FOUND)
		return;

	wxMenu* createMenu = m_pWxMenuBar->GetMenu(createItemId);
	if (createMenu == NULL)
		return;

	if (evt == EditorLib::E_DatabaseEvent_Opened)
	{
		if (!CreateDynamicMenu("Create", EditorLib::E_XtType_SceneXtActorNode))
			return;

		MenuHelper::Sort(createMenu);

		m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Create|Separator", E_MenuItem_Separator);

		if (!CreateDynamicMenu("Create", EditorLib::E_XtType_SceneXtNode))
			return;

		C_Frame* pFrame = m_pMenuBar->GetFrame();
		
		//Append group creation item
		m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Create|Separator", E_MenuItem_Separator);
		m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
			"Create|Group", 
			pFrame, 
			this, 
			&C_FrameCreateMenu::OnCreateGroup,
			&C_FrameCreateMenu::OnMenuUpdate);

		//Append animation creation item
		m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
			"Create|Animation", 
			pFrame, 
			this, 
			&C_FrameCreateMenu::OnCreateAnimation,
			&C_FrameCreateMenu::OnMenuUpdate);
	} else if (evt == EditorLib::E_DatabaseEvent_Closing)
	{
		while (createMenu->GetMenuItemCount())
		{
			wxMenuItem* item = createMenu->FindItemByPosition(0);
			createMenu->Destroy(item);
		}
	}
	 
}

void C_FrameCreateMenu::OnCreateActor(wxCommandEvent& e)
{
	Ut::UtMap<S32, Btl::String>::ConstIterator it= m_ActorCreationMenuIds.Find(e.GetId());
	if (it != m_ActorCreationMenuIds.End())
	{
		EditorLib::C_Status status;
		EditorLib::ResultsArray results;

		EditorLib::I_XtType *pXtType = EditorLib::C_Core::Instance().GetXtTypeDatabase().FindType(it->GetValue());
		if (!pXtType)
		{
			LOGERROR(g_Log, "Unknown type %s", it->GetValue().Chars());
			return;
		}

		if (pXtType->IsA(EditorLib::E_XtType_SceneXtActorNode) ||
			pXtType->IsA(EditorLib::E_XtType_SceneXtNode))
		{
			C_ActorCreateManipulatorContext actorCreateContext;
			actorCreateContext.typeName = it->GetValue();
			EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Create, &actorCreateContext);
		}
		else
		{
			// Check if we're already in a creation context, if we are, cancel it
			if (EditorLib::C_Core::Instance().GetContext() == EditorLib::E_EditorContext_Create)
				EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);

			//not a scene actor... just create it
			status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute(
				"createXtNode", it->GetValue(), results);

			if (!status)
				LOGERROR(g_Log, "Failed to create actor of type %s", it->GetValue().Chars());

			EditorLib::C_NodeId nodeId;
			Btl::String tmp;
			results[0].GetValue(tmp);
			nodeId.FromString(tmp);

			EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(nodeId);
			if (pNode->IsA(EditorLib::E_ObjType_SceneNode))
			{
				EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetRootSceneNode()->AddChild((EditorLib::C_SceneNode*)pNode);

				//select newly created object
				EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", pNode->GetId().AsString());
			}
		}
	}
}

void C_FrameCreateMenu::OnCreateAnimation(wxCommandEvent& e)
{
	EditorLib::ResultsArray results;
	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute(
		"createAnimation", "", results);
}

void C_FrameCreateMenu::OnCreateGroup(wxCommandEvent& e)
{
	EditorLib::ResultsArray results;
	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute(
		"createGroup", "-e", results);

	if (!status)
		LOGERROR(g_Log, "Failed to create group node");

	Btl::String tmp;
	results[0].GetValue(tmp);
	EditorLib::C_NodeId nodeId;
	nodeId.FromString(tmp);

	EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(nodeId);
	//select newly created object
	if (pNode)		
		EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", pNode->GetId().AsString());
	else
		LOGERROR(g_Log, "Failed to create group node");
}

void C_FrameCreateMenu::OnMenuUpdate(wxUpdateUIEvent& e)
{
	if (EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel())
		e.Enable(true);
	else
		e.Enable(false);
}

C_FrameCreateMenu::C_Slots::C_Slots(C_FrameCreateMenu *pFrameCreateMenu)
: m_pFrameCreateMenu(pFrameCreateMenu)
{
}

void C_FrameCreateMenu::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.connect(this, &Editor::C_FrameCreateMenu::C_Slots::OnDatabaseEventSlot);
}

void C_FrameCreateMenu::C_Slots::OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt)
{
	m_pFrameCreateMenu->OnDatabaseEvent(db, evt);
}

void C_FrameCreateMenu::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.disconnect(this);
}

}
