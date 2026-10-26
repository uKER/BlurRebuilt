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

#include <EditorApp/Editor/Manipulators/ManipulatorContextMapping.h>
#include <EditorApp/Editor/Manipulators/Contexts/IManipulatorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/IVectorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/IOrientationContext.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/DgNodeSelection.h>

#include <UserNode/Props/Misc/DgPropContextHelpers.h>
#include <UserNode/Props/Matrix/DgMatrixPropNode.h>
#include <UserNode/Props/Matrix/DgMatrixPropNodeContexts.h>

void RegisterMatrixPropContexts()
{
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_ScaleDgMatrixPropNodeContext)
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_RotateDgMatrixPropNodeContext)
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_MoveDgMatrixPropNodeContext)
}

namespace Editor
{

bool GetMatrixPropInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	if (items.Size() == 0)
		return false;

	//Matrix derived from last selected item.
	const EditorLib::I_DgNode* pPropNode = items.Back().pDgNode;
	assert(pPropNode->IsA(EditorLib::E_ObjType_DgMatrixPropNode));
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

	const EditorLib::C_DgMatrixPropNode *pMatrixProp = (const EditorLib::C_DgMatrixPropNode*) pPropNode;
	propMatrix = pMatrixProp->GetLocalMatrix();

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



C_ScaleDgMatrixPropNodeContext::C_ScaleDgMatrixPropNodeContext() : I_VectorContext()
{
}

C_ScaleDgMatrixPropNodeContext::~C_ScaleDgMatrixPropNodeContext()
{
}

bool C_ScaleDgMatrixPropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetMatrixPropInitialMatrix(matrix, space);
}

void* C_ScaleDgMatrixPropNodeContext::Create()
{
	return new C_ScaleDgMatrixPropNodeContext();
}

E_ManipulatorContextType C_ScaleDgMatrixPropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_ScaleMatrixProp;
}

bool C_ScaleDgMatrixPropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgMatrixPropNode, Editor::E_ManipulatorType_Scale, 
		Editor::C_ScaleDgMatrixPropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_ScaleDgMatrixPropNodeContext::Update(const Maths::HVec3& scale)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgMatrixPropNode))
			continue;

		const EditorLib::C_DgMatrixPropNode* pMatrixProperty = 
			(const EditorLib::C_DgMatrixPropNode*)pProperty;

		//We should have a render node associated with our matrix property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pMatrixProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		Aftk::Matrix44 propTransWorld = pMatrixProperty->GetLocalMatrix() * pTopLevelParent->GetWorldMatrix();
		propTransWorld[3][0] = 0.f; propTransWorld[3][1] = 0.f; propTransWorld[3][2] = 0.f;

		Aftk::Matrix44 mManipulatorSShRWorld;
		UtMaths::SetIdentity(&mManipulatorSShRWorld);
		mManipulatorSShRWorld[0][0] = scale[0];
		mManipulatorSShRWorld[1][1] = scale[1];
		mManipulatorSShRWorld[2][2] = scale[2];

		propTransWorld = mManipulatorSShRWorld * propTransWorld;

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
		EditorLib::Matrix44 newLocalMatrix = propTransWorld * invExclusiveWorldMatrix;
		(EditorLib::Vector3&)newLocalMatrix[3] = (const EditorLib::Vector3&)pMatrixProperty->GetLocalMatrix()[3];

		Maths::HMat44 localMatrix;
		Aftk::TemplateToNightmareMatrix44(localMatrix, newLocalMatrix);

		pRenderSgNode->SetLocalMatrix(localMatrix);
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_ScaleDgMatrixPropNodeContext::Apply(const Maths::HVec3& scale)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgMatrixPropNode))
			continue;

		EditorLib::C_DgMatrixPropNode* pMatrixProperty = (EditorLib::C_DgMatrixPropNode*)pProperty;

		//We should have a render node associated with our property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pMatrixProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		Aftk::Matrix44 propTransWorld = pMatrixProperty->GetLocalMatrix() * pTopLevelParent->GetWorldMatrix();
		propTransWorld[3][0] = 0.f; propTransWorld[3][1] = 0.f; propTransWorld[3][2] = 0.f;

		Aftk::Matrix44 mManipulatorSShRWorld;
		UtMaths::SetIdentity(&mManipulatorSShRWorld);
		mManipulatorSShRWorld[0][0] = scale[0];
		mManipulatorSShRWorld[1][1] = scale[1];
		mManipulatorSShRWorld[2][2] = scale[2];

		propTransWorld = mManipulatorSShRWorld * propTransWorld;

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
		EditorLib::Matrix44 newLocalMatrix = propTransWorld * invExclusiveWorldMatrix;
		(EditorLib::Vector3&)newLocalMatrix[3] = 
			(const EditorLib::Vector3&)pMatrixProperty->GetLocalMatrix()[3];

		pMatrixProperty->SetLocalMatrix(newLocalMatrix);
		//pMatrixProperty->OnTransformPlugsChanged();
	}

	return EditorLib::E_Status_Success;
}



C_RotateDgMatrixPropNodeContext::C_RotateDgMatrixPropNodeContext() : I_OrientationContext()
{
}

C_RotateDgMatrixPropNodeContext::~C_RotateDgMatrixPropNodeContext()
{
}

bool C_RotateDgMatrixPropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetMatrixPropInitialMatrix(matrix, space);
}

void* C_RotateDgMatrixPropNodeContext::Create()
{
	return new C_RotateDgMatrixPropNodeContext();
}

E_ManipulatorContextType C_RotateDgMatrixPropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_RotateMatrixProp;
}

bool C_RotateDgMatrixPropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgMatrixPropNode, Editor::E_ManipulatorType_Rotate, 
		Editor::C_RotateDgMatrixPropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_RotateDgMatrixPropNodeContext::Update(const Maths::HVec3& axis, F32 angle)
{
	Maths::C_Quaternion q0(axis, angle);

	q0.Normalise();
	Maths::HMat44 rot(q0);

	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		const EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgMatrixPropNode))
			continue;

		const EditorLib::C_DgMatrixPropNode *pMatrixProperty = 
			(const EditorLib::C_DgMatrixPropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pMatrixProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		//We should have a render node associated with our property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		Aftk::Matrix44 propTransWorld = 
			pMatrixProperty->GetLocalMatrix()*pTopLevelParent->GetWorldMatrix();
		propTransWorld[3][0] = 0.f; propTransWorld[3][1] = 0.f; propTransWorld[3][2] = 0.f;

		Aftk::Matrix44 mManipulatorSShRWorld;
		Aftk::NightmareToTemplateMatrix44(mManipulatorSShRWorld, rot);

		//Transform by manipulator's relative transformation in world space.
		propTransWorld = propTransWorld * mManipulatorSShRWorld;

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
		Aftk::Matrix44 newLocalMatrix = propTransWorld * invExclusiveWorldMatrix;
		(EditorLib::Vector3&)newLocalMatrix[3] = (const EditorLib::Vector3&)pMatrixProperty->GetLocalMatrix()[3];

		Maths::HMat44 localMatrix;
		Aftk::TemplateToNightmareMatrix44(localMatrix, newLocalMatrix);

		pRenderSgNode->SetLocalMatrix(localMatrix);
	}
	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_RotateDgMatrixPropNodeContext::Apply(const Maths::HVec3& axis, F32 angle)
{
	Maths::C_Quaternion q0(axis, angle);

	q0.Normalise();
	Maths::HMat44 rot(q0);

	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgMatrixPropNode))
			continue;

		EditorLib::C_DgMatrixPropNode *pMatrixProperty = (EditorLib::C_DgMatrixPropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pMatrixProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		Aftk::Matrix44 propTransWorld = pMatrixProperty->GetLocalMatrix() * pTopLevelParent->GetWorldMatrix();
		propTransWorld[3][0] = 0.f; propTransWorld[3][1] = 0.f; propTransWorld[3][2] = 0.f;

		Aftk::Matrix44 mManipulatorSShRWorld;
		Aftk::NightmareToTemplateMatrix44(mManipulatorSShRWorld, rot);

		//Transform by manipulator's relative transformation in world space.
		propTransWorld = propTransWorld * mManipulatorSShRWorld;

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
		EditorLib::Matrix44 newLocalMatrix = propTransWorld * invExclusiveWorldMatrix;
		(EditorLib::Vector3&)newLocalMatrix[3] = (const EditorLib::Vector3&)pMatrixProperty->GetLocalMatrix()[3];
		pMatrixProperty->SetLocalMatrix(newLocalMatrix);
		//pMatrixProperty->OnTransformPlugsChanged();
	}
	return EditorLib::E_Status_Success;
}



C_MoveDgMatrixPropNodeContext::C_MoveDgMatrixPropNodeContext() : I_VectorContext()
{
}

C_MoveDgMatrixPropNodeContext::~C_MoveDgMatrixPropNodeContext()
{
}

bool C_MoveDgMatrixPropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetMatrixPropInitialMatrix(matrix, space);
}

void* C_MoveDgMatrixPropNodeContext::Create()
{
	return new C_MoveDgMatrixPropNodeContext();
}

E_ManipulatorContextType C_MoveDgMatrixPropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_MoveMatrixProp;
}

bool C_MoveDgMatrixPropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgMatrixPropNode, Editor::E_ManipulatorType_Move, 
		Editor::C_MoveDgMatrixPropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_MoveDgMatrixPropNodeContext::Update(const Maths::HVec3& translate)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		const EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgMatrixPropNode))
			continue;

		const EditorLib::C_DgMatrixPropNode *pMatrixProperty = (EditorLib::C_DgMatrixPropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pMatrixProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		//We should have a render node associated with our matrix property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		EditorLib::Matrix44 propTransWorld = pMatrixProperty->GetLocalMatrix() * pTopLevelParent->GetWorldMatrix();
		(EditorLib::Vector3&)propTransWorld[3] = (EditorLib::Vector3&)propTransWorld[3] + EditorLib::Vector3(translate.X(), translate.Y(), translate.Z());

		Aftk::Matrix44 invExclusiveWorldMatrix;
		{
			EditorLib::Matrix44 nodeTransWorld = pTopLevelParent->GetWorldMatrix();
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

EditorLib::C_Status C_MoveDgMatrixPropNodeContext::Apply(const Maths::HVec3& translate)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgMatrixPropNode))
			continue;

		EditorLib::C_DgMatrixPropNode *pMatrixProperty = (EditorLib::C_DgMatrixPropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pMatrixProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		//We should have a render node associated with our matrix property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		EditorLib::Matrix44 propTransWorld = pMatrixProperty->GetLocalMatrix() * pTopLevelParent->GetWorldMatrix();
		(EditorLib::Vector3&)propTransWorld[3] = (EditorLib::Vector3&)propTransWorld[3] + EditorLib::Vector3(translate.X(), translate.Y(), translate.Z());

		Aftk::Matrix44 invExclusiveWorldMatrix;
		{
			EditorLib::Matrix44 nodeTransWorld = pTopLevelParent->GetWorldMatrix();
			//Check matrix is valid.
			F32 det = UtMaths::GetDeterminant4x4(nodeTransWorld );
			if (det == 0)
				continue;
			UtMaths::Invert4x4(&invExclusiveWorldMatrix, nodeTransWorld);
		}

		EditorLib::Matrix44 newLocalMatrix = propTransWorld * invExclusiveWorldMatrix;
		//EditorLib::SetCompoundValue(pMatrixProperty->GetTranslatePlug(), (Vector3&)newLocalMatrix[3]);

		//change the last row of the current matrix to reduce risk of floating point errors
		EditorLib::Matrix44 localMatrix = pMatrixProperty->GetLocalMatrix();
		(EditorLib::Vector3&)localMatrix[3] = (EditorLib::Vector3&)newLocalMatrix[3];
		pMatrixProperty->SetLocalMatrix(localMatrix);
		//pMatrixProperty->OnTransformPlugsChanged();
	}

	return EditorLib::E_Status_Success;
}

}
