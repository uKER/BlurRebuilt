#include "stdafx.h"

#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/DgXtCompNode.h>

#include <EditorApp/Editor/PropertyVisibilityManager.h>
#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>

#include <EditorApp/Frame/Menus/FrameDisplayMenu.h>

namespace Editor
{

C_PropertyVisibilityManager::C_PropertyVisibilityManager()
{
	m_pMenu = 0;
}

C_PropertyVisibilityManager::~C_PropertyVisibilityManager()
{
}

void C_PropertyVisibilityManager::UpdateViews()
{
	C_RenderCore::Instance().DirtyAllViewports();
}

bool C_PropertyVisibilityManager::IsVisible(EditorLib::C_DgXtPropNodeBase* pPropNode)
{
	Btl::String typeName = pPropNode->GetTypeName();

	Btl::OpenHashMap<Btl::String,C_PropertyVisibilityData, Btl::HashKeyOpBtlString>::Iterator it = m_RegisteredProperties.Find(typeName);
	if (it == m_RegisteredProperties.End())
		return true;

	if (!it->GetValue().IsVisible())
		return false;

	switch (it->GetValue().GetVisibilityType())
	{
	case E_PropertyVisibilityType_All:
		{
			return true;
		}
	case E_PropertyVisibilityType_Selection:
		{
			const C_DgNodeSelection& selection = C_RenderCore::Instance().GetRenderScene().GetSelection();

			if (selection.Find(pPropNode->GetParent()->GetParent()) || selection.Find(pPropNode))
				return true;
			else
				return false;
		}
	case E_PropertyVisibilityType_None:
	default:
		return false;
	}
}

C_PropertyVisibilityData* C_PropertyVisibilityManager::GetVisibilityData(const Btl::String& typeName)
{
	Btl::OpenHashMap<Btl::String,C_PropertyVisibilityData, Btl::HashKeyOpBtlString>::Iterator it = m_RegisteredProperties.Find(typeName);
	if (it == m_RegisteredProperties.End())
		return 0;

	return &it->GetValue();
}

void C_PropertyVisibilityManager::SetMenu(C_FrameDisplayMenu* menu)
{
	m_pMenu = menu;
}

}