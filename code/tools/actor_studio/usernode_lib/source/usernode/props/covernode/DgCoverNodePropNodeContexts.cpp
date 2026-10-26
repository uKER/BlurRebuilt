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
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>

#include <EditorApp/Editor/Manipulators/ManipulatorContextMapping.h>
#include <EditorApp/Editor/Manipulators/Contexts/IManipulatorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/IVectorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/IOrientationContext.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/DgNodeSelection.h>

#include <UserNode/Props/Misc/DgPropContextHelpers.h>
#include <UserNode/Props/CoverNode/DgCoverNodePropNode.h>
#include <UserNode/Props/CoverNode/DgCoverNodePropNodeContexts.h>

void RegisterCoverNodePropContexts()
{
}

namespace Editor
{

bool GetCoverNodePropInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	if (items.Size() == 0)
		return false;

	//Matrix derived from last selected item.
	const EditorLib::I_DgNode* pPropNode = items.Back().pDgNode;
	assert(pPropNode->IsA(EditorLib::E_ObjType_DgCoverNodePropNode));
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

//	const EditorLib::C_DgCoverNodePropNode *pCoverNodeProp = (const EditorLib::C_DgCoverNodePropNode*) pPropNode;
//	propMatrix = pCoverNodeProp->GetLocalMatrix();

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


/*
C_ScaleDgCoverNodePropNodeContext::C_ScaleDgCoverNodePropNodeContext() : I_VectorContext()
{
}

C_ScaleDgCoverNodePropNodeContext::~C_ScaleDgCoverNodePropNodeContext()
{
}

bool C_ScaleDgCoverNodePropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetCoverNodePropInitialMatrix(matrix, space);
}

void* C_ScaleDgCoverNodePropNodeContext::Create()
{
	return new C_ScaleDgCoverNodePropNodeContext();
}

E_ManipulatorContextType C_ScaleDgCoverNodePropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_ScaleCoverNodeProp;
}

bool C_ScaleDgCoverNodePropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgCoverNodePropNode, Editor::E_ManipulatorType_Scale, 
		Editor::C_ScaleDgCoverNodePropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_ScaleDgCoverNodePropNodeContext::Update(const Maths::HVec3& scale)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		const EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgCoverNodePropNode))
			continue;

		const EditorLib::C_DgCoverNodePropNode *pCoverNodeProperty = (const EditorLib::C_DgCoverNodePropNode*) pProperty;

		//We should have a render node associated with our sphere property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		EditorLib::Vector3 size;
		pCoverNodeProperty->GetSizePlug().GetCompoundValue(size);

		EditorLib::Matrix44 CoverNodeLocal = pCoverNodeProperty->GetLocalMatrix();
		CoverNodeLocal[0][0] *= size[0]*scale[0];
		CoverNodeLocal[0][1] *= size[0]*scale[0];
		CoverNodeLocal[0][2] *= size[0]*scale[0];
		CoverNodeLocal[1][0] *= size[1]*scale[1];
		CoverNodeLocal[1][1] *= size[1]*scale[1];
		CoverNodeLocal[1][2] *= size[1]*scale[1];		
		CoverNodeLocal[2][0] *= size[2]*scale[2];
		CoverNodeLocal[2][1] *= size[2]*scale[2];
		CoverNodeLocal[2][2] *= size[2]*scale[2];	
		CoverNodeLocal[3][0] = 0.5f * (CoverNodeLocal[0][0]+CoverNodeLocal[1][0]+CoverNodeLocal[2][0]);
		CoverNodeLocal[3][1] = 0.5f * (CoverNodeLocal[0][1]+CoverNodeLocal[1][1]+CoverNodeLocal[2][1]);
		CoverNodeLocal[3][2] = 0.5f * (CoverNodeLocal[0][2]+CoverNodeLocal[1][2]+CoverNodeLocal[2][2]);
		Maths::HMat44 localMatrix;
		Aftk::TemplateToNightmareMatrix44(localMatrix, CoverNodeLocal);

		pRenderSgNode->SetLocalMatrix(localMatrix);
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_ScaleDgCoverNodePropNodeContext::Apply(const Maths::HVec3& scale)
{	
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgCoverNodePropNode))
			continue;

		EditorLib::C_DgCoverNodePropNode *pCoverNodeProperty = (EditorLib::C_DgCoverNodePropNode*) pProperty;

		EditorLib::Vector3 size;
		pCoverNodeProperty->GetSizePlug().GetCompoundValue(size);
		size[0] *= scale[0]; size[1] *= scale[1]; size[2] *= scale[2];
		EditorLib::SetCompoundValue(pCoverNodeProperty->GetSizePlug(), size);
	}

	return EditorLib::E_Status_Success;
}



C_RotateDgCoverNodePropNodeContext::C_RotateDgCoverNodePropNodeContext() : I_OrientationContext()
{
}

C_RotateDgCoverNodePropNodeContext::~C_RotateDgCoverNodePropNodeContext()
{
}

bool C_RotateDgCoverNodePropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetCoverNodePropInitialMatrix(matrix, space);
}

void* C_RotateDgCoverNodePropNodeContext::Create()
{
	return new C_RotateDgCoverNodePropNodeContext();
}

E_ManipulatorContextType C_RotateDgCoverNodePropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_RotateCoverNodeProp;
}

bool C_RotateDgCoverNodePropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgCoverNodePropNode, Editor::E_ManipulatorType_Rotate, 
		Editor::C_RotateDgCoverNodePropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_RotateDgCoverNodePropNodeContext::Update(const Maths::HVec3& axis, F32 angle)
{
	Maths::C_Quaternion q0(axis, angle);

	q0.Normalise();
	Maths::HMat44 rot(q0);

	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		const EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgCoverNodePropNode))
			continue;

		const EditorLib::C_DgCoverNodePropNode* pCoverNodeProperty = 
			(const EditorLib::C_DgCoverNodePropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pCoverNodeProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		//We should have a render node associated with our sphere property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		EditorLib::Matrix44 CoverNodeTransWorld = pCoverNodeProperty->GetLocalMatrix()*pTopLevelParent->GetWorldMatrix();
		CoverNodeTransWorld[3][0] = 0.f; CoverNodeTransWorld[3][1] = 0.f; CoverNodeTransWorld[3][2] = 0.f;

		Aftk::Matrix44 mManipulatorSShRWorld;
		Aftk::NightmareToTemplateMatrix44(mManipulatorSShRWorld, rot);

		//Transform by manipulator's relative transformation in world space.
		CoverNodeTransWorld = CoverNodeTransWorld * mManipulatorSShRWorld;

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
		Aftk::Matrix44 newLocalMatrix = CoverNodeTransWorld * invExclusiveWorldMatrix;
		
		EditorLib::Vector3 size;
		pCoverNodeProperty->GetSizePlug().GetCompoundValue(size);
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

EditorLib::C_Status C_RotateDgCoverNodePropNodeContext::Apply(const Maths::HVec3& axis, F32 angle)
{
	Maths::C_Quaternion q0(axis, angle);

	q0.Normalise();
	Maths::HMat44 rot(q0);

	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgCoverNodePropNode))
			continue;

		EditorLib::C_DgCoverNodePropNode *pCoverNodeProperty = (EditorLib::C_DgCoverNodePropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pCoverNodeProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		Aftk::Matrix44 CoverNodeTransWorld = pCoverNodeProperty->GetLocalMatrix() * pTopLevelParent->GetWorldMatrix();
		CoverNodeTransWorld[3][0] = 0.f; CoverNodeTransWorld[3][1] = 0.f; CoverNodeTransWorld[3][2] = 0.f;

		Aftk::Matrix44 mManipulatorSShRWorld;
		Aftk::NightmareToTemplateMatrix44(mManipulatorSShRWorld, rot);

		//Transform by manipulator's relative transformation in world space.
		CoverNodeTransWorld = CoverNodeTransWorld * mManipulatorSShRWorld;

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
		EditorLib::Matrix44 newLocalMatrix = CoverNodeTransWorld * invExclusiveWorldMatrix;
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
		EditorLib::SetCompoundValue(pCoverNodeProperty->GetRotatePlug(), newEulerRotation);
	}
	return EditorLib::E_Status_Success;
}



C_MoveDgCoverNodePropNodeContext::C_MoveDgCoverNodePropNodeContext() : I_VectorContext()
{
}

C_MoveDgCoverNodePropNodeContext::~C_MoveDgCoverNodePropNodeContext()
{
}

bool C_MoveDgCoverNodePropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetCoverNodePropInitialMatrix(matrix, space);
}

void* C_MoveDgCoverNodePropNodeContext::Create()
{
	return new C_MoveDgCoverNodePropNodeContext();
}

E_ManipulatorContextType C_MoveDgCoverNodePropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_MoveCoverNodeProp;
}

bool C_MoveDgCoverNodePropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgCoverNodePropNode, Editor::E_ManipulatorType_Move, 
		Editor::C_MoveDgCoverNodePropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_MoveDgCoverNodePropNodeContext::Update(const Maths::HVec3& translate)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		const EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgCoverNodePropNode))
			continue;

		const EditorLib::C_DgCoverNodePropNode *pCoverNodeProperty = (EditorLib::C_DgCoverNodePropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pCoverNodeProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		//We should have a render node associated with our CoverNode's parent!! - CoverNode property does not have its own position...
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

EditorLib::C_Status C_MoveDgCoverNodePropNodeContext::Apply(const Maths::HVec3& translate)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgCoverNodePropNode))
			continue;

		EditorLib::C_DgCoverNodePropNode *pCoverNodeProperty = (EditorLib::C_DgCoverNodePropNode*)pProperty;

		EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pCoverNodeProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		//We should have a render node associated with our CoverNode's parent!! - CoverNode property does not have its own position...
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
*/
}
