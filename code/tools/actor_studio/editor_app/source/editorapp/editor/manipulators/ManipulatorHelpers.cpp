#include "stdafx.h"

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Core.h>

#include <EditorApp/Editor/Manipulators/ManipulatorHelpers.h>

#include "aftk/AftkEuler.h"

namespace Editor
{
namespace ManipulatorHelpers
{

bool SelectedNodesAreA(
	EditorLib::E_ObjType isAType,
	const EditorLib::C_Selection& selection)
{
	const EditorLib::C_Selection::ItemArray& items = selection.GetItems();

	for (EditorLib::C_Selection::ItemArray::ConstIterator it = items.Begin(); 
		it != items.End(); 
		++it)
	{
		const EditorLib::C_Selection::C_Item& item = *it;
		EditorLib::I_DgNode* pNode = 
			EditorLib::C_Core::Instance().GetDgManager().FindNodeById(item.id);
		if (!pNode)
			return false;

		if (!pNode->IsA(isAType))
			return false;
	}

	return true;
}

EditorLib::E_ObjType SelectedNodesGetType(const EditorLib::C_Selection& selection)
{
	const EditorLib::C_Selection::ItemArray& items = selection.GetItems();

	EditorLib::E_ObjType type = EditorLib::E_ObjType_Unknown;
	bool ambiguous = false;
	bool foundSceneNode = false;
	for (EditorLib::C_Selection::ItemArray::ConstIterator it = items.Begin(); 
		it != items.End();
		++it)
	{
		const EditorLib::C_Selection::C_Item& item = *it;
		EditorLib::I_DgNode* pNode = 
			EditorLib::C_Core::Instance().GetDgManager().FindNodeById(item.id);
		if (!pNode)
			continue;

		if (type == EditorLib::E_ObjType_Unknown)
			type = pNode->GetType();

		if (pNode->GetType() != type)
			ambiguous = true;

		if (pNode->IsA(EditorLib::E_ObjType_SceneNode))
			foundSceneNode = true;
	}

	if (!ambiguous)
		return type;

	if (foundSceneNode)
		return EditorLib::E_ObjType_SceneNode;
	
	return EditorLib::E_ObjType_Unknown;
}

EditorLib::E_ObjType DetermineSelectionType()
{
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	if (selection.GetItems().Size() == 0)
		return EditorLib::E_ObjType_Unknown;


	if (SelectedNodesAreA(EditorLib::E_ObjType_SceneNode, selection))
	{
		//Filter and manipulate scene nodes.
		return EditorLib::E_ObjType_SceneNode;
	}

	//All nodes must be of same type if node selecting at scene node level
	return SelectedNodesGetType(selection);
}

}
}
