#include "stdafx.h"

#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/SceneNode.h>

#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/RenderHelpers.h>
#include <EditorApp/Editor/DgNodeSelection.h>

namespace Editor
{
namespace RenderHelpers
{

Maths::RGBA GetHighlightColour(const EditorLib::I_DgNode* pDgNode, Maths::RGBA defaultCol)
{
	const C_DgNodeSelection& selection = C_RenderCore::Instance().GetRenderScene().GetSelection();

	if (selection.Find(pDgNode))
	{
		if (pDgNode->IsReadOnly())
			return Maths::RGBA(255,255,255,255);

		defaultCol = Maths::RGBA(0, 255, 128, 255);
		//is the node the last selected item?
		C_DgNodeSelection::ItemArray& items = const_cast<C_DgNodeSelection::ItemArray&>(selection.GetItems());
		for (C_DgNodeSelection::ItemArray::ConstReverseIterator it=items.ReverseBegin(); it!=items.ReverseEnd(); it++)
		{
			const EditorLib::I_DgNode* pNode = it->pDgNode;
			if (pNode->IsA(EditorLib::E_ObjType_SceneNode))
			{
				if (pNode == pDgNode)
					defaultCol = Maths::RGBA(0, 255, 128, 255);
				else
					defaultCol = Maths::RGBA(255, 255, 255, 255);
				break;
			}
		}
	}
	else
	{
		if (pDgNode->IsReadOnly())
			return Maths::RGBA(64,64,64,255);

		//are any of the 'parents' selected?
		const EditorLib::I_DgNode* pParent = pDgNode;
		do
		{
			if (pParent->IsA(EditorLib::E_ObjType_SceneNode))
				pParent = static_cast<const EditorLib::C_SceneNode*>(pParent)->GetParent();
			else if (pParent->IsA(EditorLib::E_ObjType_DgXtPropNodeBase))
				pParent = static_cast<const EditorLib::C_DgXtPropNodeBase*>(pParent)->GetParent();
			else if (pParent->IsA(EditorLib::E_ObjType_DgXtCompNode))
				pParent = static_cast<const EditorLib::C_DgXtCompNode*>(pParent)->GetParent();
			else
				pParent = 0;

			if (pParent && selection.Find(pParent))
			{
				defaultCol = Maths::RGBA(0, 255, 128, 255);
				break;
			}
		} while (pParent);
	}

	return defaultCol;

}

}
}
