#include "stdafx.h"
#include <string>

#include <Aftk/AftkEnum.h>
#include <Aftk/AftkTypedefs.h>
#include <Aftk/AftkBounds.h>
#include <Aftk/AftkMaths.h>
#include <Aftk/AftkEuler.h>

// aftk rendering
#include "AftkRender.h"

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>

#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>

#include <EditorApp/Editor/Manipulators/ManipulatorContextMapping.h>
#include <EditorApp/Editor/Manipulators/Contexts/IManipulatorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/IVectorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/IOrientationContext.h>
#include <EditorApp/Editor/DgNodeSelection.h>

#include <UserNode/Props/Misc/DgPropContextHelpers.h>
#include <UserNode/Props/Box/DgBoxPropNode.h>
#include <UserNode/Props/Box/DgBoxPropNodeContexts.h>

void RegisterBoxPropContexts()
{
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_ScaleDgBoxPropNodeContext)
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_RotateDgBoxPropNodeContext)
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_MoveDgBoxPropNodeContext)
}

namespace Editor
{

bool GetBoxPropInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	if (items.Size() == 0)
		return false;

	//Matrix derived from last selected item.
	const EditorLib::I_DgNode* pPropNode = items.Back().pDgNode;
	assert(pPropNode->IsA(EditorLib::E_ObjType_DgBoxPropNode));
	const EditorLib::C_DgXtCompNode* pParentComponent =
		((EditorLib::C_DgXtPropNodeBase*)pPropNode)->GetParent();
	assert(pParentComponent);
	const EditorLib::I_DgNode* pTopLevelParent = pParentComponent->GetParent();
	assert(pTopLevelParent);
	//Bail if our top level parent isn't a scene node.
	if (!pTopLevelParent->IsA(EditorLib::E_ObjType_SceneNode))
		return false;

	const EditorLib::C_SceneNode* pSceneNode = (EditorLib::C_SceneNode*)pTopLevelParent;
	const EditorLib::Matrix44& nodeMatrix = pSceneNode->GetWorldMatrix();

	EditorLib::Matrix44 propMatrix;
	UtMaths::SetIdentity(&propMatrix);

	const EditorLib::C_DgBoxPropNode *pBoxProp = (const EditorLib::C_DgBoxPropNode*) pPropNode;
	propMatrix = pBoxProp->GetLocalMatrix();

	//world space matrix
	propMatrix = propMatrix * nodeMatrix;

	if (space == E_ManipulatorSpace_Local)
	{
		//extract the rotation & translation
		EditorLib::Matrix44 tempM44;
		Maths::HMat43 tempH43;
		Aftk::TemplateToNightmareMatrix43(tempH43, propMatrix);

		Maths::HVec3 scale, shear, rotate, translate;

		Aftk::GetSShRT(scale, shear, rotate, translate, tempH43, Aftk::E_RotationOrder_XYZ);

		Aftk::EulerToMatrix(tempM44, Aftk::NightmareToTemplate(rotate), Aftk::E_RotationOrder_XYZ);
		tempM44[3][0] = propMatrix[3][0];
		tempM44[3][1] = propMatrix[3][1];
		tempM44[3][2] = propMatrix[3][2];

		Aftk::TemplateToNightmareMatrix44(matrix, tempM44);
	}
	else
	{
		//axis aligned
		matrix[3][0] = propMatrix[3][0];
		matrix[3][1] = propMatrix[3][1];
		matrix[3][2] = propMatrix[3][2];
	}

	return true;
}



C_ScaleDgBoxPropNodeContext::C_ScaleDgBoxPropNodeContext() : I_VectorContext()
{
}

C_ScaleDgBoxPropNodeContext::~C_ScaleDgBoxPropNodeContext()
{
}

bool C_ScaleDgBoxPropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetBoxPropInitialMatrix(matrix, space);
}

void* C_ScaleDgBoxPropNodeContext::Create()
{
	return new C_ScaleDgBoxPropNodeContext();
}

E_ManipulatorContextType C_ScaleDgBoxPropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_ScaleBoxProp;
}

bool C_ScaleDgBoxPropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgBoxPropNode, Editor::E_ManipulatorType_Scale, 
		Editor::C_ScaleDgBoxPropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_ScaleDgBoxPropNodeContext::Update(const Maths::HVec3& scale)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		const EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgBoxPropNode))
			continue;

		const EditorLib::C_DgBoxPropNode *pBoxProperty = (const EditorLib::C_DgBoxPropNode*) pProperty;

		//We should have a render node associated with our sphere property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		EditorLib::Vector3 size;
		EditorLib::PlugHelpers::GetCompoundValue(pBoxProperty->GetSizePlug(), size);

		EditorLib::Matrix44 boxLocal = pBoxProperty->GetLocalMatrix();
		boxLocal[0][0] *= size[0]*scale[0];
		boxLocal[0][1] *= size[0]*scale[0];
		boxLocal[0][2] *= size[0]*scale[0];
		boxLocal[1][0] *= size[1]*scale[1];
		boxLocal[1][1] *= size[1]*scale[1];
		boxLocal[1][2] *= size[1]*scale[1];		
		boxLocal[2][0] *= size[2]*scale[2];
		boxLocal[2][1] *= size[2]*scale[2];
		boxLocal[2][2] *= size[2]*scale[2];	
		boxLocal[3][0] = 0.5f * (boxLocal[0][0]+boxLocal[1][0]+boxLocal[2][0]);
		boxLocal[3][1] = 0.5f * (boxLocal[0][1]+boxLocal[1][1]+boxLocal[2][1]);
		boxLocal[3][2] = 0.5f * (boxLocal[0][2]+boxLocal[1][2]+boxLocal[2][2]);
		Maths::HMat44 localMatrix;
		Aftk::TemplateToNightmareMatrix44(localMatrix, boxLocal);

		pRenderSgNode->SetLocalMatrix(localMatrix);
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_ScaleDgBoxPropNodeContext::Apply(const Maths::HVec3& scale)
{	
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgBoxPropNode))
			continue;

		EditorLib::C_DgBoxPropNode *pBoxProperty = (EditorLib::C_DgBoxPropNode*) pProperty;

		EditorLib::Vector3 size;
		EditorLib::PlugHelpers::GetCompoundValue(pBoxProperty->GetSizePlug(), size);
		size[0] *= scale[0]; size[1] *= scale[1]; size[2] *= scale[2];
		EditorLib::PlugHelpers::SetCompoundValue(pBoxProperty->GetSizePlug(), size);
	}

	return EditorLib::E_Status_Success;
}



C_RotateDgBoxPropNodeContext::C_RotateDgBoxPropNodeContext() : I_OrientationContext()
{
}

C_RotateDgBoxPropNodeContext::~C_RotateDgBoxPropNodeContext()
{
}

bool C_RotateDgBoxPropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetBoxPropInitialMatrix(matrix, space);
}

void* C_RotateDgBoxPropNodeContext::Create()
{
	return new C_RotateDgBoxPropNodeContext();
}

E_ManipulatorContextType C_RotateDgBoxPropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_RotateBoxProp;
}

bool C_RotateDgBoxPropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgBoxPropNode, Editor::E_ManipulatorType_Rotate, 
		Editor::C_RotateDgBoxPropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_RotateDgBoxPropNodeContext::Update(const Maths::HVec3& axis, F32 angle)
{
	Maths::C_Quaternion q0(axis, angle);

	q0.Normalise();
	Maths::HMat44 rot(q0);

	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		const EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgBoxPropNode))
			continue;

		const EditorLib::C_DgBoxPropNode* pBoxProperty = 
			(const EditorLib::C_DgBoxPropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pBoxProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		//We should have a render node associated with our sphere property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		EditorLib::Matrix44 boxTransWorld = pBoxProperty->GetLocalMatrix()*pTopLevelParent->GetWorldMatrix();
		boxTransWorld[3][0] = 0.f; boxTransWorld[3][1] = 0.f; boxTransWorld[3][2] = 0.f;

		Aftk::Matrix44 mManipulatorSShRWorld;
		Aftk::NightmareToTemplateMatrix44(mManipulatorSShRWorld, rot);

		//Transform by manipulator's relative transformation in world space.
		boxTransWorld = boxTransWorld * mManipulatorSShRWorld;

		//Acquire the inverse world matrix of our parent.
		Aftk::Matrix44 invExclusiveWorldMatrix;
		{
			EditorLib::Matrix44 mWorld = pTopLevelParent->GetWorldMatrix();
			//Check matrix is valid.
			F32 det = UtMaths::GetDeterminant4x4(mWorld);
			if (det == 0)
				continue;
			UtMaths::Invert4x4(&invExclusiveWorldMatrix, mWorld);
		}
		EditorLib::Matrix44 invExclusiveWorldMatrixFull = invExclusiveWorldMatrix;
		invExclusiveWorldMatrix[3][0] = 0.f; invExclusiveWorldMatrix[3][1] = 0.f; invExclusiveWorldMatrix[3][2] = 0.f;

		//Calculate new local matrix.
		Aftk::Matrix44 newLocalMatrix = boxTransWorld * invExclusiveWorldMatrix;
		
		EditorLib::Vector3 size;
		EditorLib::PlugHelpers::GetCompoundValue(pBoxProperty->GetSizePlug(), size);
		newLocalMatrix[0][0] *= size[0];
		newLocalMatrix[0][1] *= size[0];
		newLocalMatrix[0][2] *= size[0];
		newLocalMatrix[1][0] *= size[1];
		newLocalMatrix[1][1] *= size[1];
		newLocalMatrix[1][2] *= size[1];		
		newLocalMatrix[2][0] *= size[2];
		newLocalMatrix[2][1] *= size[2];
		newLocalMatrix[2][2] *= size[2];
		newLocalMatrix[3][0] = 0.5f * (newLocalMatrix[0][0]+newLocalMatrix[1][0]+newLocalMatrix[2][0]);
		newLocalMatrix[3][1] = 0.5f * (newLocalMatrix[0][1]+newLocalMatrix[1][1]+newLocalMatrix[2][1]);
		newLocalMatrix[3][2] = 0.5f * (newLocalMatrix[0][2]+newLocalMatrix[1][2]+newLocalMatrix[2][2]);

		Maths::HMat44 localMatrix;
		Aftk::TemplateToNightmareMatrix44(localMatrix, newLocalMatrix);

		pRenderSgNode->SetLocalMatrix(localMatrix);
	}
	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_RotateDgBoxPropNodeContext::Apply(const Maths::HVec3& axis, F32 angle)
{
	Maths::C_Quaternion q0(axis, angle);

	q0.Normalise();
	Maths::HMat44 rot(q0);

	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgBoxPropNode))
			continue;

		EditorLib::C_DgBoxPropNode *pBoxProperty = (EditorLib::C_DgBoxPropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pBoxProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		Aftk::Matrix44 boxTransWorld = pBoxProperty->GetLocalMatrix() * pTopLevelParent->GetWorldMatrix();
		boxTransWorld[3][0] = 0.f; boxTransWorld[3][1] = 0.f; boxTransWorld[3][2] = 0.f;

		Aftk::Matrix44 mManipulatorSShRWorld;
		Aftk::NightmareToTemplateMatrix44(mManipulatorSShRWorld, rot);

		//Transform by manipulator's relative transformation in world space.
		boxTransWorld = boxTransWorld * mManipulatorSShRWorld;

		//Acquire the inverse world matrix of our parent.
		EditorLib::Matrix44 invExclusiveWorldMatrix;
		{
			EditorLib::Matrix44 mWorld = pTopLevelParent->GetWorldMatrix();
			//Check matrix is valid.
			F32 det = UtMaths::GetDeterminant4x4(mWorld);
			if (det == 0)
				continue;
			UtMaths::Invert4x4(&invExclusiveWorldMatrix, mWorld);
		}
		EditorLib::Matrix44 invExclusiveWorldMatrixFull = invExclusiveWorldMatrix;
		invExclusiveWorldMatrix[3][0] = 0.f; invExclusiveWorldMatrix[3][1] = 0.f; invExclusiveWorldMatrix[3][2] = 0.f;

		//Calculate new local matrix.
		EditorLib::Matrix44 newLocalMatrix = boxTransWorld * invExclusiveWorldMatrix;
		EditorLib::Vector3 newEulerRotation;
		{
			Maths::HMat44 localHMatrix;
			Aftk::TemplateToNightmareMatrix44(localHMatrix, newLocalMatrix);

			Maths::HMat43 localHMatrix43(localHMatrix);

			Maths::HVec3 scale, shear, rotate, translate;
			//Attempt to remove scaling and shear.
			if (!Aftk::GetSShRT(scale, shear, rotate, translate, localHMatrix43, Aftk::E_RotationOrder_XYZ))
				continue;

			newEulerRotation[0] = rotate[0]; newEulerRotation[1] = rotate[1]; newEulerRotation[2] = rotate[2];
		}
		EditorLib::PlugHelpers::SetCompoundValue(pBoxProperty->GetRotatePlug(), newEulerRotation);
	}
	return EditorLib::E_Status_Success;
}



C_MoveDgBoxPropNodeContext::C_MoveDgBoxPropNodeContext() : I_VectorContext()
{
}

C_MoveDgBoxPropNodeContext::~C_MoveDgBoxPropNodeContext()
{
}

bool C_MoveDgBoxPropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetBoxPropInitialMatrix(matrix, space);
}

void* C_MoveDgBoxPropNodeContext::Create()
{
	return new C_MoveDgBoxPropNodeContext();
}

E_ManipulatorContextType C_MoveDgBoxPropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_MoveBoxProp;
}

bool C_MoveDgBoxPropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgBoxPropNode, Editor::E_ManipulatorType_Move, 
		Editor::C_MoveDgBoxPropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_MoveDgBoxPropNodeContext::Update(const Maths::HVec3& translate)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		const EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgBoxPropNode))
			continue;

		const EditorLib::C_DgBoxPropNode *pBoxProperty = (EditorLib::C_DgBoxPropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pBoxProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		//We should have a render node associated with our box's parent!! - Box property does not have its own position...
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pTopLevelParent);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		EditorLib::Matrix44 propTransWorld = pTopLevelParent->GetWorldMatrix();
		(EditorLib::Vector3&)propTransWorld[3] = (EditorLib::Vector3&)propTransWorld[3] + EditorLib::Vector3(translate.X(), translate.Y(), translate.Z());

		Aftk::Matrix44 invExclusiveWorldMatrix;
		{
			EditorLib::Matrix44 nodeTransWorld = pTopLevelParent->GetParent()->GetWorldMatrix();
			//Check matrix is valid.
			F32 det = UtMaths::GetDeterminant4x4(nodeTransWorld );
			if (det == 0)
				continue;
			UtMaths::Invert4x4(&invExclusiveWorldMatrix, nodeTransWorld);
		}

		EditorLib::Matrix44 newLocalMatrix = propTransWorld * invExclusiveWorldMatrix;

		Maths::HMat44 localMatrix;
		Aftk::TemplateToNightmareMatrix44(localMatrix, newLocalMatrix);
		pRenderSgNode->SetLocalMatrix(localMatrix);
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_MoveDgBoxPropNodeContext::Apply(const Maths::HVec3& translate)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgBoxPropNode))
			continue;

		EditorLib::C_DgBoxPropNode *pBoxProperty = (EditorLib::C_DgBoxPropNode*)pProperty;

		EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pBoxProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		//We should have a render node associated with our box's parent!! - Box property does not have its own position...
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pTopLevelParent);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		EditorLib::Matrix44 propTransWorld = pTopLevelParent->GetWorldMatrix();
		(EditorLib::Vector3&)propTransWorld[3] = (EditorLib::Vector3&)propTransWorld[3] + EditorLib::Vector3(translate.X(), translate.Y(), translate.Z());

		Aftk::Matrix44 invExclusiveWorldMatrix;
		{
			EditorLib::Matrix44 nodeTransWorld = pTopLevelParent->GetParent()->GetWorldMatrix();
			//Check matrix is valid.
			F32 det = UtMaths::GetDeterminant4x4(nodeTransWorld );
			if (det == 0)
				continue;
			UtMaths::Invert4x4(&invExclusiveWorldMatrix, nodeTransWorld);
		}

		EditorLib::Matrix44 newLocalMatrix = propTransWorld * invExclusiveWorldMatrix;
		//EditorLib::SetCompoundValue(pMatrixProperty->GetTranslatePlug(), (Vector3&)newLocalMatrix[3]);

		//change the last row of the current matrix to reduce risk of floating point errors
		EditorLib::Matrix44 localMatrix = pTopLevelParent->GetLocalMatrix();
		(EditorLib::Vector3&)localMatrix[3] = (EditorLib::Vector3&)newLocalMatrix[3];
		pTopLevelParent->SetLocalMatrix(localMatrix);
	}

	return EditorLib::E_Status_Success;
}

}

