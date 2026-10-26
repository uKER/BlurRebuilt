#include "stdafx.h"

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>

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
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderScene.h>

#include <EditorLib/Context/DeleteContextMapping.h>
#include <EditorLib/Context/IDeleteContext.h>

#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/IActorPropertyNode.h>
#include <EditorApp/Editor/ActorScene.h>
#include <EditorApp/Editor/Primitives.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/SimpleRenderer.h>
#include <EditorApp/Editor/ComplexRenderer.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Misc/MathsHelpers.h>
#include <EditorApp/Editor/DgNodeSelection.h>

#include <UserNode/Props/Misc/DgPropContextHelpers.h>
#include <UserNode/Props/Spline/DgSplinePropNode.h>
#include <UserNode/Props/Spline/DgSplinePropNodeContexts.h>
#include <UserNode/Props/Spline/RenderSplinePropNode.h>

#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Core.h>

void RegisterSplinePropContexts()
{
	REGISTER_DELETE_CONTEXT(Editor::C_DeleteDgSplinePropNodeContext)
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_ScaleDgSplinePropNodeContext)
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_MoveDgSplinePropNodeContext)
}

namespace Editor
{

bool GetSplinePropInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	if (items.Size() == 0)
		return false;

	//Matrix derived from last selected item.
	const EditorLib::I_DgNode* pPropNode = items.Back().pDgNode;
	assert(pPropNode->IsA(EditorLib::E_ObjType_DgSplinePropNode));
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

	EditorLib::C_DgSplinePropNode* pSplineProp = (EditorLib::C_DgSplinePropNode*) pPropNode;

	// Get a list of components
	const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds = items.Back().components;

	if (!componentIds.Size())
		return false;

	float x = 0;
	float y = 0;
	float z = 0;
	for (int i=0; i<componentIds.Size(); i++) 
	{
		U32 idx = componentIds[i];
		if (idx >= pSplineProp->GetNumComponents())
			continue;

		Maths::HVec3 pos = pSplineProp->GetComponent(idx);
		x += pos.X();
		y += pos.Y();
		z += pos.Z();
	}
	x /= componentIds.Size();
	y /= componentIds.Size();
	z /= componentIds.Size();
	propMatrix[3][0] += x;
	propMatrix[3][1] += y;
	propMatrix[3][2] += z;

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

C_ScaleDgSplinePropNodeContext::C_ScaleDgSplinePropNodeContext() : I_VectorContext()
{
	m_PrevValue.X(1);
	m_PrevValue.Y(1);
	m_PrevValue.Z(1);
}

C_ScaleDgSplinePropNodeContext::~C_ScaleDgSplinePropNodeContext()
{
}

bool C_ScaleDgSplinePropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetSplinePropInitialMatrix(matrix, space);
}

void* C_ScaleDgSplinePropNodeContext::Create()
{
	return new C_ScaleDgSplinePropNodeContext();
}

E_ManipulatorContextType C_ScaleDgSplinePropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_ScaleSplineProp;
}

bool C_ScaleDgSplinePropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgSplinePropNode, Editor::E_ManipulatorType_Scale, 
		Editor::C_ScaleDgSplinePropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_ScaleDgSplinePropNodeContext::Update(const Maths::HVec3& scale)
{
	Maths::HVec3 newValue = scale - m_PrevValue;
	m_PrevValue = scale;

	const C_DgNodeSelection::ItemArray& items= C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgSplinePropNode))
			continue;

		const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

		EditorLib::C_DgSplinePropNode *pSplineNode = (EditorLib::C_DgSplinePropNode*)pProperty;

		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pSplineNode);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		if (newValue.X() == newValue.Y() && newValue.Y() == newValue.Z()) {
			F32 tensionScale = newValue.X()*2.0f;
			((C_RenderSplinePropNode*)pRenderSgNode)->UpdateSplineTension(components,tensionScale);
		}
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_ScaleDgSplinePropNodeContext::Apply(const Maths::HVec3& scale)
{
	Maths::HVec3 newValue = scale - m_PrevValue;
	m_PrevValue.X(1);
	m_PrevValue.Y(1);
	m_PrevValue.Z(1);

	const C_DgNodeSelection::ItemArray& items= C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgSplinePropNode))
			continue;

		const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

		EditorLib::C_DgSplinePropNode* pSplineNode = (EditorLib::C_DgSplinePropNode*)pProperty;
		pSplineNode->Apply();
	}

	return EditorLib::E_Status_Success;
}


C_MoveDgSplinePropNodeContext::C_MoveDgSplinePropNodeContext() : I_VectorContext()
{
	m_PrevValue.X(0);
	m_PrevValue.Y(0);
	m_PrevValue.Z(0);
}

C_MoveDgSplinePropNodeContext::~C_MoveDgSplinePropNodeContext()
{
}

bool C_MoveDgSplinePropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetSplinePropInitialMatrix(matrix, space);
}

void* C_MoveDgSplinePropNodeContext::Create()
{
	return new C_MoveDgSplinePropNodeContext();
}

E_ManipulatorContextType C_MoveDgSplinePropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_MoveSplineProp;
}

bool C_MoveDgSplinePropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgSplinePropNode, Editor::E_ManipulatorType_Move, 
		Editor::C_MoveDgSplinePropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_MoveDgSplinePropNodeContext::Update(const Maths::HVec3& translate)
{
	Maths::HVec3 newValue = translate - m_PrevValue;
	m_PrevValue = translate;

	const C_DgNodeSelection::ItemArray& items= C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgSplinePropNode))
			continue;

		const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

		EditorLib::C_DgSplinePropNode *pSplineNode = (EditorLib::C_DgSplinePropNode*)pProperty;

		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pSplineNode);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pSplineNode);
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

		Maths::HVec3 splineOffset;

		splineOffset.X(newLocalMatrix[3][0]);
		splineOffset.Y(newLocalMatrix[3][1]);
		splineOffset.Z(newLocalMatrix[3][2]);

		((C_RenderSplinePropNode*)pRenderSgNode)->UpdateOffset(components,splineOffset);
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_MoveDgSplinePropNodeContext::Apply(const Maths::HVec3& translate)
{
	m_PrevValue.X(0);
	m_PrevValue.Y(0);
	m_PrevValue.Z(0);

	const C_DgNodeSelection::ItemArray& items= C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgSplinePropNode))
			continue;

		const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

		EditorLib::C_DgSplinePropNode *pSplineNode = (EditorLib::C_DgSplinePropNode*)pProperty;
		pSplineNode->Apply();
	}

	return EditorLib::E_Status_Success;
}

void* C_DeleteDgSplinePropNodeContext::Create()
{
	return new C_DeleteDgSplinePropNodeContext();
}

EditorLib::E_DeleteContextType C_DeleteDgSplinePropNodeContext::GetStaticType()
{
	return EditorLib::E_DeleteContextType_SplineDelete;
}

bool C_DeleteDgSplinePropNodeContext::DeleteContextRegister()
{
	if (!EditorLib::C_DeleteContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgSplinePropNode,
		Editor::C_DeleteDgSplinePropNodeContext::GetStaticType()))
		return false;
	return true;
}

bool C_DeleteDgSplinePropNodeContext::Delete(EditorLib::I_DgNode* pNode, EditorLib::C_Selection& selectedItems)
{
	if (!pNode->IsA(EditorLib::E_ObjType_DgSplinePropNode))
		return false;

	const C_DgNodeSelection::ItemArray& items= C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		
		if (pProperty == pNode) {
			const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

			EditorLib::C_DgSplinePropNode *pSplineNode = (EditorLib::C_DgSplinePropNode*)pProperty;

			AftkRender::C_SceneGraphNode* pRenderSgNode = 
					C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pSplineNode);
			if (!pRenderSgNode)
				return false;

			pSplineNode->DeletePoints(components);
	
			Ut::Vector<U32> removableComponents;
			for (int i=0; i<components.Size(); i++) {
				removableComponents.PushBack(components[i]);
			}
			selectedItems.Remove(EditorLib::C_Selection::E_Mode_Component, pSplineNode->GetId(), removableComponents);

			((C_RenderSplinePropNode*)pRenderSgNode)->UpdateSpline();
		}
	}

	return false;
}

}
