#include "stdafx.h"
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/DgXtCompNode.h>

#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Misc/SelectionHelpers.h>

#include <EditorLib/Core.h>

#include <algorithm>

namespace EditorLib
{
namespace SelectionHelpers
{

bool _PassIsAFilter(const I_DgNode* pNode, E_ObjType objType)
{
	return pNode->IsA(objType);
}

bool _PassFilter(const I_DgNode* pNode, E_ObjType objType)
{
	return pNode->GetType() == objType;
}

void _FilterSelection(
	C_Selection::ItemArray& result, const C_Selection::ItemArray& selection, E_ObjType objType,
	bool (*passFilter)(const I_DgNode*, E_ObjType))
{
	result.Clear();
	C_Selection::ItemMap tmp;
	for (S32 i = 0; i < selection.Size(); ++i)
	{
		I_DgNode* pNode = C_Core::Instance().GetDgManager().FindNodeById(selection[i].id);
		if (!pNode || pNode->GetType() != objType)
			continue;

		tmp[selection[i].id] = i;
	}

	for (C_Selection::ItemMap::Iterator it = tmp.Begin(); it != tmp.End(); ++it)
	{
		result.PushBack();
		result.Back().id = it->GetKey();
		result.Back().components = selection[it->GetValue()].components;
	}
}

void _FilterPropertySelection(
	C_Selection::ItemArray& result, const C_Selection::ItemArray& selection, E_ObjType objType,
	bool (*passFilter)(const I_DgNode*, E_ObjType))
{
	result.Clear();
	C_Selection::ItemMap tmp;
	for (S32 i = 0; i < selection.Size(); ++i)
	{
		I_DgNode* pNode = C_Core::Instance().GetDgManager().FindNodeById(selection[i].id);
		if (!pNode)
			continue;

		if (passFilter(pNode, objType))
		{
			tmp[selection[i].id] = i;
		}
		else if (pNode->IsA(E_ObjType_SceneXtActorNode))
		{
			const C_SceneXtActorNode* pActor = (C_SceneXtActorNode*)pNode;
			const ConstDgActorComponentArray& comps = pActor->GetComponents();
			for (ConstDgActorComponentArray::ConstIterator it = comps.Begin(); it != comps.End(); ++it)
			{
				const C_DgXtCompNode& comp = *it;
				const ConstDgPropArray& props = comp.GetProperties();
				for (ConstDgPropArray::ConstIterator it2 = props.Begin(); it2 != props.End(); ++it2)
				{
					const EditorLib::C_DgXtPropNodeBase& prop = *it2;
					if (passFilter(&prop, objType))
					{
						tmp[prop.GetId()] = i;
					}
				}
			}
		}
	}

	for (C_Selection::ItemMap::Iterator it = tmp.Begin(); it != tmp.End(); ++it)
	{
		result.PushBack();
		result.Back().id = it->GetKey();
		result.Back().components = selection[it->GetValue()].components;
	}
}

void FilterSelection(C_Selection::ItemArray& result, const C_Selection::ItemArray& selection, E_ObjType objType)
{
	return _FilterSelection(result, selection, objType, _PassFilter);
}

void FilterSelectionIsA(C_Selection::ItemArray& result, const C_Selection::ItemArray& selection, E_ObjType objType)
{
	return _FilterSelection(result, selection, objType, _PassIsAFilter);
}

void FilterPropertySelection(C_Selection::ItemArray& result, const C_Selection::ItemArray& selection, E_ObjType objType)
{
	return _FilterPropertySelection(result, selection, objType, _PassFilter);
}

void FilterPropertySelectionIsA(C_Selection::ItemArray& result, const C_Selection::ItemArray& selection, E_ObjType objType)
{
	return _FilterPropertySelection(result, selection, objType, _PassIsAFilter);
}

}
}