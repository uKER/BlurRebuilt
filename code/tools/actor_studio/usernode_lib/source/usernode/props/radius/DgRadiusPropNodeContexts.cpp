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

#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/IActorPropertyNode.h>
#include <EditorApp/Editor/Manipulators/ManipulatorContextMapping.h>
#include <EditorApp/Editor/Manipulators/Contexts/IManipulatorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/IVectorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/IOrientationContext.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/DgNodeSelection.h>

#include <UserNode/Props/Misc/DgPropContextHelpers.h>
#include <UserNode/Props/Radius/DgRadiusPropNode.h>
#include <UserNode/Props/Radius/DgRadiusPropNodeContexts.h>
#include <UserNode/Props/Radius/RenderRadiusPropNode.h>

void RegisterRadiusPropContexts()
{
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_ScaleDgRadiusPropNodeContext)
}

namespace Editor
{

bool GetRadiusPropInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	if (items.Size() == 0)
		return false;

	//Matrix derived from last selected item.
	const EditorLib::I_DgNode* pPropNode = items.Back().pDgNode;
	assert(pPropNode->IsA(EditorLib::E_ObjType_DgRadiusPropNode));
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

	EditorLib::C_DgRadiusPropNode* pSplineProp = (EditorLib::C_DgRadiusPropNode*) pPropNode;

	// Get a list of components
	const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds = items.Back().components;

	if (!componentIds.Size())
		return false;

	//world space matrix
	propMatrix = propMatrix * nodeMatrix;

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

	return true;
}



C_ScaleDgRadiusPropNodeContext::C_ScaleDgRadiusPropNodeContext() : I_VectorContext()
{
}

C_ScaleDgRadiusPropNodeContext::~C_ScaleDgRadiusPropNodeContext()
{
}

bool C_ScaleDgRadiusPropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetRadiusPropInitialMatrix(matrix, space);
}

void* C_ScaleDgRadiusPropNodeContext::Create()
{
	return new C_ScaleDgRadiusPropNodeContext();
}

E_ManipulatorContextType C_ScaleDgRadiusPropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_ScaleRadiusProp;
}

bool C_ScaleDgRadiusPropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgRadiusPropNode, Editor::E_ManipulatorType_Scale, 
		Editor::C_ScaleDgRadiusPropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_ScaleDgRadiusPropNodeContext::Update(const Maths::HVec3& scale)
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
		if (!pProperty->IsA(EditorLib::E_ObjType_DgRadiusPropNode))
			continue;

		const EditorLib::C_DgRadiusPropNode* pRadiusProperty = 
			(const EditorLib::C_DgRadiusPropNode*)pProperty;

		const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

		//We should have a render node associated with our radius property.
		C_RenderRadiusPropNode* pRenderSgNode = (C_RenderRadiusPropNode*)
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		pRenderSgNode->ScaleRadii(radiusScale, components);
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_ScaleDgRadiusPropNodeContext::Apply(const Maths::HVec3& scale)
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
		if (!pProperty->IsA(EditorLib::E_ObjType_DgRadiusPropNode))
			continue;

		EditorLib::C_DgRadiusPropNode* pRadiusProperty = (EditorLib::C_DgRadiusPropNode*)pProperty;
		const Ut::UtVector< Ut::DynamicVector<U32> >& components = it->components;

		const U32 nRadii = pRadiusProperty->GetNumRadii();
		for (S32 i=0; i<components.Size(); ++i)
		{
			if (components[i] < nRadii)
			{
				EditorLib::C_StaticPlug<F32>& rPlug = pRadiusProperty->GetRadiusPlug(components[i]);
				const F32 radius = rPlug.GetValue();
				rPlug.SetValue(radius*radiusScale);
			}
		}
	}

	return EditorLib::E_Status_Success;
}


}
