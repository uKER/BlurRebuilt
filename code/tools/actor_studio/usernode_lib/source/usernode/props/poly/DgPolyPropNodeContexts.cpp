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
#include <UserNode/Props/Poly/DgPolyPropNode.h>
#include <UserNode/Props/Poly/DgPolyPropNodeContexts.h>
#include <UserNode/Props/Poly/RenderPolyPropNode.h>

#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Core.h>

void RegisterPolyPropContexts()
{
	REGISTER_DELETE_CONTEXT(Editor::C_DeleteDgPolyPropNodeContext)
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_MoveDgPolyPropNodeContext)
}

namespace Editor
{

bool GetPolyPropInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	if (items.Size() == 0)
		return false;

	//Matrix derived from last selected item.
	const EditorLib::I_DgNode* pPropNode = items.Back().pDgNode;
	assert(pPropNode->IsA(EditorLib::E_ObjType_DgPolyPropNode));
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

	EditorLib::C_DgPolyPropNode* pPolyProp = (EditorLib::C_DgPolyPropNode*) pPropNode;

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
		if (idx >= pPolyProp->GetNumComponents())
			continue;

		Maths::HVec3 pos = pPolyProp->GetComponent(idx);
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

C_MoveDgPolyPropNodeContext::C_MoveDgPolyPropNodeContext() : I_VectorContext()
{
	m_PrevValue.X(0);
	m_PrevValue.Y(0);
	m_PrevValue.Z(0);
}

C_MoveDgPolyPropNodeContext::~C_MoveDgPolyPropNodeContext()
{
}

bool C_MoveDgPolyPropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetPolyPropInitialMatrix(matrix, space);
}

void* C_MoveDgPolyPropNodeContext::Create()
{
	return new C_MoveDgPolyPropNodeContext();
}

E_ManipulatorContextType C_MoveDgPolyPropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_MovePolyProp;
}

bool C_MoveDgPolyPropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgPolyPropNode, Editor::E_ManipulatorType_Move, 
		Editor::C_MoveDgPolyPropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_MoveDgPolyPropNodeContext::Update(const Maths::HVec3& translate)
{
	Maths::HVec3 newValue = translate - m_PrevValue;
	m_PrevValue = translate;

	const C_DgNodeSelection::ItemArray& items= C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgPolyPropNode))
			continue;

		const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

		EditorLib::C_DgPolyPropNode *pPolyNode = (EditorLib::C_DgPolyPropNode*)pProperty;

		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pPolyNode);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pPolyNode);
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

		Maths::HVec3 PolyOffset;

		PolyOffset.X(newLocalMatrix[3][0]);
		PolyOffset.Y(newLocalMatrix[3][1]);
		PolyOffset.Z(newLocalMatrix[3][2]);

		((C_RenderPolyPropNode*)pRenderSgNode)->UpdateOffset(components,PolyOffset);
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_MoveDgPolyPropNodeContext::Apply(const Maths::HVec3& translate)
{
	m_PrevValue.X(0);
	m_PrevValue.Y(0);
	m_PrevValue.Z(0);

	const C_DgNodeSelection::ItemArray& items= C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgPolyPropNode))
			continue;

		const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

		EditorLib::C_DgPolyPropNode *pPolyNode = (EditorLib::C_DgPolyPropNode*)pProperty;
		pPolyNode->Apply();
	}

	return EditorLib::E_Status_Success;
}

void* C_DeleteDgPolyPropNodeContext::Create()
{
	return new C_DeleteDgPolyPropNodeContext();
}

EditorLib::E_DeleteContextType C_DeleteDgPolyPropNodeContext::GetStaticType()
{
	return EditorLib::E_DeleteContextType_PolyDelete;
}

bool C_DeleteDgPolyPropNodeContext::DeleteContextRegister()
{
	if (!EditorLib::C_DeleteContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgPolyPropNode,
		Editor::C_DeleteDgPolyPropNodeContext::GetStaticType()))
		return false;
	return true;
}

bool C_DeleteDgPolyPropNodeContext::Delete(EditorLib::I_DgNode* pNode, EditorLib::C_Selection& selectedItems)
{
	if (!pNode->IsA(EditorLib::E_ObjType_DgPolyPropNode))
		return false;

	const C_DgNodeSelection::ItemArray& items= C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		
		if (pProperty == pNode) {
			const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

			EditorLib::C_DgPolyPropNode *pPolyNode = (EditorLib::C_DgPolyPropNode*)pProperty;

			AftkRender::C_SceneGraphNode* pRenderSgNode = 
					C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pPolyNode);
			if (!pRenderSgNode)
				return false;

			pPolyNode->DeletePoints(components);
	
			Ut::Vector<U32> removableComponents;
			for (int i=0; i<components.Size(); i++) {
				removableComponents.PushBack(components[i]);
			}
			selectedItems.Remove(EditorLib::C_Selection::E_Mode_Component, pPolyNode->GetId(), removableComponents);

			((C_RenderPolyPropNode*)pRenderSgNode)->UpdatePoly();
		}
	}

	return false;
}

}
