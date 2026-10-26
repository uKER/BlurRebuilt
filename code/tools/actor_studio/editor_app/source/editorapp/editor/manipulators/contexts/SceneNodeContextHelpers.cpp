#include "stdafx.h"

#include <EditorLib/Dg/SceneNode.h>

#include <EditorApp/editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/editor/manipulators/Manipulator.h>
#include <EditorApp/editor/manipulators/ManipulatorHelpers.h>
#include <EditorApp/editor/RenderViewport.h>
#include <EditorApp/editor/RenderCore.h>
#include <EditorApp/editor/RenderScene.h>
#include <EditorApp/Editor/DgNodeSelection.h>

#include <EditorApp/Editor/Manipulators/Contexts/SceneNodeContextHelpers.h>

#include <aftk/AftkEuler.h>

namespace Editor
{
namespace SceneNodeContextHelpers
{

bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	matrix.SetIdentity();

	C_DgNodeSelection::ItemArray& items = const_cast<C_DgNodeSelection::ItemArray&>
		(C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems());
	if (items.Size() == 0)
		return false;

	//Matrix derived from last selected item. TODO, this is a sorted container. Fix stupidity.
	const EditorLib::C_SceneNode* pSceneNode = 0;
	{
		//Search for a scene node.
		for (C_DgNodeSelection::ItemArray::ConstReverseIterator it=items.ReverseBegin(); it!=items.ReverseEnd(); it++)
		{
			const EditorLib::I_DgNode* pNode = it->pDgNode;
			if (pNode->IsA(EditorLib::E_ObjType_SceneNode))
			{
				pSceneNode = (EditorLib::C_SceneNode*)pNode;
				break;
			}
		}
	}
	//Return if no scene nodes.
	if (!pSceneNode)
		return false;

	const EditorLib::Matrix44& nodeMatrix = pSceneNode->GetWorldMatrix();
	if (space == E_ManipulatorSpace_Local)
	{
		//extract the rotation & translation
		EditorLib::Matrix44 tempM44;
		Maths::HMat43 tempH43;
		Aftk::TemplateToNightmareMatrix43(tempH43, nodeMatrix);

		Maths::HVec3 scale, shear, rotate, translate;

		Aftk::GetSShRT(scale, shear, rotate, translate, tempH43, Aftk::E_RotationOrder_XYZ);

		Aftk::EulerToMatrix(tempM44, Aftk::NightmareToTemplate(rotate), Aftk::E_RotationOrder_XYZ);
		tempM44[3][0] = nodeMatrix[3][0];
		tempM44[3][1] = nodeMatrix[3][1];
		tempM44[3][2] = nodeMatrix[3][2];

		Aftk::TemplateToNightmareMatrix44(matrix, tempM44);
	}
	else
	{
		//axis aligned
		matrix[3][0] = nodeMatrix[3][0];
		matrix[3][1] = nodeMatrix[3][1];
		matrix[3][2] = nodeMatrix[3][2];
	}

	return true;
}

}
}
