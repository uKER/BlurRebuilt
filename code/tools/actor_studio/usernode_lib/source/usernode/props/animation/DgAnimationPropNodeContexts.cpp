#include "stdafx.h"
#include <string>

#include <Aftk/AftkEnum.h>
#include <Aftk/AftkTypedefs.h>
#include <Aftk/AftkBounds.h>
#include <Aftk/AftkMaths.h>
#include <Aftk/AftkEuler.h>

// aftk rendering
#include "AftkRender.h"

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>
#include <Misc_Shared/NewCurve/MayaHermiteSpline.h>

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

#include <EditorLib/Context/DeleteContextMapping.h>
#include <EditorLib/Context/IDeleteContext.h>

#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/IActorPropertyNode.h>
#include <EditorApp/Misc/MathsHelpers.h>
#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/ComplexRenderer.h>

#include <UserNode/Props/Misc/DgPropContextHelpers.h>
#include <UserNode/Props/Animation/DgAnimationPropNode.h>
#include <UserNode/Props/Animation/DgAnimationPropNodeContexts.h>
#include <UserNode/Props/Animation/RenderAnimationPropNode.h>

#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Core.h>

void RegisterAnimationPropContexts()
{
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_RotateDgAnimationPropNodeContext)
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_MoveDgAnimationPropNodeContext)
	REGISTER_DELETE_CONTEXT(Editor::C_DeleteDgAnimationPropNodeContext)
}


namespace Editor
{

bool GetAnimationPropInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	const C_DgNodeSelection& selection = C_RenderCore::Instance().GetRenderScene().GetSelection();
	const C_DgNodeSelection::ItemArray& items = selection.GetItems();
	if (items.Size() == 0)
		return false;

	//Matrix derived from last selected item.
	const EditorLib::I_DgNode* pPropNode = items.Back().pDgNode;
	assert(pPropNode->IsA(EditorLib::E_ObjType_DgAnimationPropNode));
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

	EditorLib::C_DgAnimationPropNode *pAnimationProp = (EditorLib::C_DgAnimationPropNode*) pPropNode;

	AftkRender::C_SceneGraphNode* pRenderSgNode = 
		C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pAnimationProp);

	if (!pRenderSgNode)
			return false;

	const C_DgNodeSelection::C_Item *pItem = selection.Find(pPropNode);
	const Ut::UtVector< Ut::DynamicVector<U32> >* pComponents = 0;
	if (pItem)
		pComponents = &pItem->components;

	if (!pComponents || !pComponents->Size())
		return false;

	U32 componentId = pComponents->Back();

	if (componentId >= (U32)pAnimationProp->GetObjects().Size())
		return false;

	Maths::HMat44& m = ((C_RenderAnimationPropNode*)pRenderSgNode)->GetLocalTransform(componentId);

	EditorLib::Matrix44 propMatrix;

	Aftk::NightmareToTemplateMatrix44(propMatrix, m);
	
	//world space matrix
	propMatrix = propMatrix * nodeMatrix;

	if (space == E_ManipulatorSpace_Local)
	{
		//extract the rotation & translation
		EditorLib::Matrix44 tempM44;
		Maths::HMat43 tempH43;
		Aftk::TemplateToNightmareMatrix43(tempH43, propMatrix);

		Maths::HVec3 scale, shear, rotate, translate;

		Aftk::GetSShRT(scale, shear, rotate, translate, tempH43, Aftk::E_RotationOrder_ZXY);

		Aftk::EulerToMatrix(tempM44, Aftk::NightmareToTemplate(rotate), Aftk::E_RotationOrder_ZXY);
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

C_RotateDgAnimationPropNodeContext::C_RotateDgAnimationPropNodeContext() : I_OrientationContext()
{
}

C_RotateDgAnimationPropNodeContext::~C_RotateDgAnimationPropNodeContext()
{
}

bool C_RotateDgAnimationPropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetAnimationPropInitialMatrix(matrix, space);
}

void* C_RotateDgAnimationPropNodeContext::Create()
{
	return new C_RotateDgAnimationPropNodeContext();
}

E_ManipulatorContextType C_RotateDgAnimationPropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_RotateAnimationProp;
}

bool C_RotateDgAnimationPropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgAnimationPropNode, Editor::E_ManipulatorType_Rotate, 
		Editor::C_RotateDgAnimationPropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_RotateDgAnimationPropNodeContext::Update(const Maths::HVec3& axis, F32 angle)
{
	Maths::C_Quaternion rotate(axis, angle);
	rotate.Normalise();

	if (!m_PrevValue.Finite())
		m_PrevValue = rotate;	
	
	Maths::C_Quaternion newValue = rotate * !m_PrevValue;
	m_PrevValue = rotate;

	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		const EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgAnimationPropNode))
			continue;

		EditorLib::C_DgAnimationPropNode *pAnimationNode = (EditorLib::C_DgAnimationPropNode*)pProperty;

		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pAnimationNode);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		const Ut::UtVector< Ut::DynamicVector<U32> >* pComponents = 0;
		pComponents = &it->components;

		if (pComponents && pComponents->Size())
			((C_RenderAnimationPropNode*)pRenderSgNode)->ApplyRotate(pComponents, newValue);
	}
	
	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_RotateDgAnimationPropNodeContext::Apply(const Maths::HVec3& axis, F32 angle)
{
	m_PrevValue = Maths::C_Quaternion();

	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgAnimationPropNode))
			continue;

		EditorLib::C_DgAnimationPropNode *pAnimationNode = (EditorLib::C_DgAnimationPropNode*)pProperty;

		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pAnimationNode);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		// TODO: Commit rotation to xt
	}

	return EditorLib::E_Status_Success;
}

C_MoveDgAnimationPropNodeContext::C_MoveDgAnimationPropNodeContext() : I_VectorContext()
{
	m_PrevValue.X(0);
	m_PrevValue.Y(0);
	m_PrevValue.Z(0);
}

C_MoveDgAnimationPropNodeContext::~C_MoveDgAnimationPropNodeContext()
{
}

bool C_MoveDgAnimationPropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetAnimationPropInitialMatrix(matrix, space);
}

void* C_MoveDgAnimationPropNodeContext::Create()
{
	return new C_MoveDgAnimationPropNodeContext();
}

E_ManipulatorContextType C_MoveDgAnimationPropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_MoveAnimationProp;
}

bool C_MoveDgAnimationPropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgAnimationPropNode, Editor::E_ManipulatorType_Move, 
		Editor::C_MoveDgAnimationPropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_MoveDgAnimationPropNodeContext::Update(const Maths::HVec3& translate)
{
	Maths::HVec3 newValue = translate - m_PrevValue;
	m_PrevValue = translate;

	const C_DgNodeSelection::ItemArray& items= C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgAnimationPropNode))
			continue;

		const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

		EditorLib::C_DgAnimationPropNode *pAnimationNode = (EditorLib::C_DgAnimationPropNode*)pProperty;

		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pAnimationNode);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pAnimationNode);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		EditorLib::Matrix44 transWorld(pTopLevelParent->GetWorldMatrix());
		(EditorLib::Vector3&)transWorld[3] = (EditorLib::Vector3&)transWorld[3] + EditorLib::Vector3(newValue.X(), newValue.Y(), newValue.Z());

		Aftk::Matrix44 invExclusiveWorldMatrix;
		{
			EditorLib::Matrix44 nodeTransWorld = pTopLevelParent->GetWorldMatrix();
			//Check matrix is valid.
			F32 det = UtMaths::GetDeterminant4x4(nodeTransWorld );
			if (det == 0)
				continue;
			UtMaths::Invert4x4(&invExclusiveWorldMatrix, nodeTransWorld);
		}

		EditorLib::Matrix44 newLocalMatrix = transWorld * invExclusiveWorldMatrix;

		Maths::HVec3 animationOffset;

		animationOffset.X(newLocalMatrix[3][0]);
		animationOffset.Y(newLocalMatrix[3][1]);
		animationOffset.Z(newLocalMatrix[3][2]);

		const Ut::UtVector< Ut::DynamicVector<U32> >* pComponents = 0;
		pComponents = &it->components;

		if (pComponents && pComponents->Size())
			((C_RenderAnimationPropNode*)pRenderSgNode)->ApplyTranslate(pComponents, animationOffset);
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_MoveDgAnimationPropNodeContext::Apply(const Maths::HVec3& translate)
{
	Maths::HVec3 newValue = translate - m_PrevValue;
	m_PrevValue.X(0);
	m_PrevValue.Y(0);
	m_PrevValue.Z(0);

	const C_DgNodeSelection::ItemArray& items= C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgAnimationPropNode))
			continue;

		const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

		EditorLib::C_DgAnimationPropNode *pAnimationNode = (EditorLib::C_DgAnimationPropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pAnimationNode);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		EditorLib::Matrix44 transWorld(pTopLevelParent->GetWorldMatrix());

		(EditorLib::Vector3&)transWorld[3] = (EditorLib::Vector3&)transWorld[3] + EditorLib::Vector3(newValue.X(), newValue.Y(), newValue.Z());

		Aftk::Matrix44 invExclusiveWorldMatrix;
		{
			EditorLib::Matrix44 nodeTransWorld = pTopLevelParent->GetWorldMatrix();
			//Check matrix is valid.
			F32 det = UtMaths::GetDeterminant4x4(nodeTransWorld );
			if (det == 0)
				continue;
			UtMaths::Invert4x4(&invExclusiveWorldMatrix, nodeTransWorld);
		}

		EditorLib::Matrix44 newLocalMatrix = transWorld * invExclusiveWorldMatrix;

		Maths::HVec3 animationOffset;

		animationOffset.X(newLocalMatrix[3][0]);
		animationOffset.Y(newLocalMatrix[3][1]);
		animationOffset.Z(newLocalMatrix[3][2]);

		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pAnimationNode);

		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

//		((C_RenderAnimationPropNode*)pRenderSgNode)->ApplyTranslate(animationOffset);
	}

	return EditorLib::E_Status_Success;
}

void* C_DeleteDgAnimationPropNodeContext::Create()
{
	return new C_DeleteDgAnimationPropNodeContext();
}

EditorLib::E_DeleteContextType C_DeleteDgAnimationPropNodeContext::GetStaticType()
{
	return EditorLib::E_DeleteContextType_AnimationDelete;
}

bool C_DeleteDgAnimationPropNodeContext::DeleteContextRegister()
{
	if (!EditorLib::C_DeleteContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgAnimationPropNode,
		Editor::C_DeleteDgAnimationPropNodeContext::GetStaticType()))
		return false;
	return true;
}

bool C_DeleteDgAnimationPropNodeContext::Delete(EditorLib::I_DgNode* pNode, EditorLib::C_Selection& selectedItems)
{
	if (!pNode->IsA(EditorLib::E_ObjType_DgAnimationPropNode))
		return false;

	const C_DgNodeSelection::ItemArray& items= C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		
		if (pProperty == pNode) {
			const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

			EditorLib::C_DgAnimationPropNode *pAnimationNode = (EditorLib::C_DgAnimationPropNode*)pProperty;

			AftkRender::C_SceneGraphNode* pRenderSgNode = 
					C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pAnimationNode);
			if (!pRenderSgNode)
				return false;

//			pAnimationNode->DeletePoints(components);
	
			Ut::Vector<U32> removableComponents;
			for (int i=0; i<components.Size(); i++) {
				removableComponents.PushBack(components[i]);
			}
			selectedItems.Remove(EditorLib::C_Selection::E_Mode_Component, pAnimationNode->GetId(), removableComponents);

			((C_RenderAnimationPropNode*)pRenderSgNode)->UpdateAnimation();
		}
	}

	return false;
}

}
