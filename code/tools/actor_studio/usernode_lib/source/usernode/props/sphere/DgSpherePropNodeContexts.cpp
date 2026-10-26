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
#include <UserNode/Props/Sphere/DgSpherePropNode.h>
#include <UserNode/Props/Sphere/DgSpherePropNodeContexts.h>

void RegisterSpherePropContexts()
{
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_ScaleDgSpherePropNodeContext)
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_MoveDgSpherePropNodeContext)
}

namespace Editor
{

bool GetSpherePropInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	if (items.Size() == 0)
		return false;

	//Matrix derived from last selected item.
	const EditorLib::I_DgNode* pPropNode = items.Back().pDgNode;
	assert(pPropNode->IsA(EditorLib::E_ObjType_DgSpherePropNode));
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

	const EditorLib::C_DgSpherePropNode* pSphereProp = 
		(const EditorLib::C_DgSpherePropNode*)pPropNode;
	EditorLib::Vector3 position;
	EditorLib::PlugHelpers::GetCompoundValue(pSphereProp->GetPositionPlug(), position);
	(EditorLib::Vector3&)propMatrix[3] = position;

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



C_ScaleDgSpherePropNodeContext::C_ScaleDgSpherePropNodeContext() : I_VectorContext()
{
}

C_ScaleDgSpherePropNodeContext::~C_ScaleDgSpherePropNodeContext()
{
}

bool C_ScaleDgSpherePropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetSpherePropInitialMatrix(matrix, space);
}

void* C_ScaleDgSpherePropNodeContext::Create()
{
	return new C_ScaleDgSpherePropNodeContext();
}

E_ManipulatorContextType C_ScaleDgSpherePropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_ScaleSphereProp;
}

bool C_ScaleDgSpherePropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgSpherePropNode, Editor::E_ManipulatorType_Scale, 
		Editor::C_ScaleDgSpherePropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_ScaleDgSpherePropNodeContext::Update(const Maths::HVec3& scale)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();

	//convert the scaling vector into a single radius scaling factor
	F32 radiusScale = 1.f;
	{
		//we don't know which axis the of the manipulator is active...
		Maths::HVec3 tmp = scale;

		radiusScale = 0.f;
		U32 axis = 0;
		for (U32 i=1; i<3; ++i)
			if (Maths::Abs(scale[i]-1.f)>Maths::Abs(scale[axis]-1.f))
				axis = i;

		radiusScale = Maths::Abs(scale[axis]);
	}
	
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		const EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgSpherePropNode))
			continue;

		const EditorLib::C_DgSpherePropNode* pSphereProperty = 
			(const EditorLib::C_DgSpherePropNode*)pProperty;

		//We should have a render node associated with our sphere property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		EditorLib::Vector3 sphereTranslate;
		EditorLib::PlugHelpers::GetCompoundValue(pSphereProperty->GetPositionPlug(), sphereTranslate);

		F32 radius;
		pSphereProperty->GetRadiusPlug().GetValue(radius);

		Maths::HMat44 newLocalMatrix;
		newLocalMatrix[0][0] = newLocalMatrix[1][1] = newLocalMatrix[2][2] = radius * radiusScale;
		newLocalMatrix[3][0] = sphereTranslate[0];
		newLocalMatrix[3][1] = sphereTranslate[1];
		newLocalMatrix[3][2] = sphereTranslate[2];

		pRenderSgNode->SetLocalMatrix(newLocalMatrix);
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_ScaleDgSpherePropNodeContext::Apply(const Maths::HVec3& scale)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();

	//convert the scaling vector into a single radius scaling factor
	F32 radiusScale = 1.f;
	{
		//we don't know which axis the of the manipulator is active...
		Maths::HVec3 tmp = scale;

		radiusScale = 0.f;
		U32 axis = 0;
		for (U32 i=1; i<3; ++i)
			if (Maths::Abs(scale[i]-1.f)>Maths::Abs(scale[axis]-1.f))
				axis = i;

		radiusScale = Maths::Abs(scale[axis]);
	}
	

	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgSpherePropNode))
			continue;

		EditorLib::C_DgSpherePropNode* pSphereProperty = 
			(EditorLib::C_DgSpherePropNode*)pProperty;

		EditorLib::Vector3 sphereTranslate;
		EditorLib::PlugHelpers::GetCompoundValue(pSphereProperty->GetPositionPlug(), sphereTranslate);

		F32 radius;
		pSphereProperty->GetRadiusPlug().GetValue(radius);
		pSphereProperty->GetRadiusPlug().SetValue(radius*radiusScale);
	}

	return EditorLib::E_Status_Success;
}



C_MoveDgSpherePropNodeContext::C_MoveDgSpherePropNodeContext() : I_VectorContext()
{
}

C_MoveDgSpherePropNodeContext::~C_MoveDgSpherePropNodeContext()
{
}

bool C_MoveDgSpherePropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetSpherePropInitialMatrix(matrix, space);
}

void* C_MoveDgSpherePropNodeContext::Create()
{
	return new C_MoveDgSpherePropNodeContext();
}

E_ManipulatorContextType C_MoveDgSpherePropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_MoveSphereProp;
}

bool C_MoveDgSpherePropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgSpherePropNode, Editor::E_ManipulatorType_Move, 
		Editor::C_MoveDgSpherePropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_MoveDgSpherePropNodeContext::Update(const Maths::HVec3& translate)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();

	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgSpherePropNode))
			continue;

		EditorLib::C_DgSpherePropNode* pSphereProperty = 
			(EditorLib::C_DgSpherePropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pSphereProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		//We should have a render node associated with our sphere property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		EditorLib::Matrix44 sphereTransWorld;
		{
			UtMaths::SetIdentity(&sphereTransWorld);
			F32 radius;
			EditorLib::Vector3 sphereTranslate;
			pSphereProperty->GetRadiusPlug().GetValue(radius);
			EditorLib::PlugHelpers::GetCompoundValue(pSphereProperty->GetPositionPlug(), sphereTranslate);
			sphereTransWorld[0][0] = sphereTransWorld[1][1] = sphereTransWorld[2][2] = radius;
			(EditorLib::Vector3&)sphereTransWorld[3] = sphereTranslate;

			sphereTransWorld = 
				sphereTransWorld * pTopLevelParent->GetWorldMatrix();
		}

		(EditorLib::Vector3&)sphereTransWorld[3] = 
			(EditorLib::Vector3&)sphereTransWorld[3] + EditorLib::Vector3(translate.X(), translate.Y(), translate.Z());

		Aftk::Matrix44 invExclusiveWorldMatrix;
		{
			EditorLib::Matrix44 nodeTransWorld = pTopLevelParent->GetWorldMatrix();
			//Check matrix is valid.
			F32 det = UtMaths::GetDeterminant4x4(nodeTransWorld );
			if (det == 0)
				continue;
			UtMaths::Invert4x4(&invExclusiveWorldMatrix, nodeTransWorld);
		}

		EditorLib::Matrix44 newLocalMatrix = sphereTransWorld * invExclusiveWorldMatrix;

		Maths::HMat44 localMatrix;
		Aftk::TemplateToNightmareMatrix44(localMatrix, newLocalMatrix);
		pRenderSgNode->SetLocalMatrix(localMatrix);
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_MoveDgSpherePropNodeContext::Apply(const Maths::HVec3& translate)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();

	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgSpherePropNode))
			continue;

		EditorLib::C_DgSpherePropNode* pSphereProperty = 
			(EditorLib::C_DgSpherePropNode*)pProperty;

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pSphereProperty);
		if (!pTopLevelParent) return EditorLib::E_Status_Failure;

		EditorLib::Matrix44 sphereTransWorld;
		{
			UtMaths::SetIdentity(&sphereTransWorld);

			EditorLib::Vector3 sphereTranslate;
			EditorLib::PlugHelpers::GetCompoundValue(pSphereProperty->GetPositionPlug(), sphereTranslate);
			(EditorLib::Vector3&)sphereTransWorld[3] = sphereTranslate;

			sphereTransWorld = sphereTransWorld * pTopLevelParent->GetWorldMatrix();
		}

		(EditorLib::Vector3&)sphereTransWorld[3] = 
			(EditorLib::Vector3&)sphereTransWorld[3] + EditorLib::Vector3(translate.X(), translate.Y(), translate.Z());

		Aftk::Matrix44 invExclusiveWorldMatrix;
		{
			EditorLib::Matrix44 nodeTransWorld = pTopLevelParent->GetWorldMatrix();
			//Check matrix is valid.
			F32 det = UtMaths::GetDeterminant4x4(nodeTransWorld );
			if (det == 0)
				continue;
			UtMaths::Invert4x4(&invExclusiveWorldMatrix, nodeTransWorld);
		}

		EditorLib::Matrix44 mNewLocalTrans = sphereTransWorld * invExclusiveWorldMatrix;
		EditorLib::PlugHelpers::SetCompoundValue(pSphereProperty->GetPositionPlug(), (EditorLib::Vector3&)mNewLocalTrans[3]);	
	}

	return EditorLib::E_Status_Success;
}

}
