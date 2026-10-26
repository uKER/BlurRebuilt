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
#include <UserNode/Props/Rectangle/DgRectanglePropNode.h>
#include <UserNode/Props/Rectangle/DgRectanglePropNodeContexts.h>

void RegisterRectanglePropContexts()
{
	REGISTER_MANIPULATOR_CONTEXT(Editor::C_ScaleDgRectanglePropNodeContext)
}

namespace Editor
{

bool GetRectanglePropInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	if (items.Size() == 0)
		return false;

	//Matrix derived from last selected item.
	const EditorLib::I_DgNode* pPropNode = items.Back().pDgNode;
	assert(pPropNode->IsA(EditorLib::E_ObjType_DgRectanglePropNode));
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
  
C_ScaleDgRectanglePropNodeContext::C_ScaleDgRectanglePropNodeContext() : I_VectorContext()
{
}

C_ScaleDgRectanglePropNodeContext::~C_ScaleDgRectanglePropNodeContext()
{
}

bool C_ScaleDgRectanglePropNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return GetRectanglePropInitialMatrix(matrix, space);
}

void* C_ScaleDgRectanglePropNodeContext::Create()
{
	return new C_ScaleDgRectanglePropNodeContext();
}

E_ManipulatorContextType C_ScaleDgRectanglePropNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_ScaleRectangleProp;
}

bool C_ScaleDgRectanglePropNodeContext::ManipulatorContextRegister()
{
	if (!Editor::C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_DgRectanglePropNode, Editor::E_ManipulatorType_Scale, 
		Editor::C_ScaleDgRectanglePropNodeContext::GetStaticType()))
		return false;
	return true;
}

EditorLib::C_Status C_ScaleDgRectanglePropNodeContext::Update(const Maths::HVec3& scale)
{
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		const EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgRectanglePropNode))
			continue;

		const EditorLib::C_DgRectanglePropNode *pRectangleProperty = (const EditorLib::C_DgRectanglePropNode*) pProperty;

		//We should have a render node associated with our rectangle property.
		AftkRender::C_SceneGraphNode* pRenderSgNode = 
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pProperty);
		if (!pRenderSgNode)
			return EditorLib::E_Status_Failure;

		F32 width;
		pRectangleProperty->GetWidthPlug().GetValue(width);

		F32 height;
		pRectangleProperty->GetHeightPlug().GetValue(height);

		Maths::HMat44 newLocalMatrix;
		newLocalMatrix[0][0] = width * scale[0];
		newLocalMatrix[1][1] = height * scale[1];
		newLocalMatrix[2][2] = 0;
		newLocalMatrix[3][0] = 0;
		newLocalMatrix[3][1] = 0;
		newLocalMatrix[3][2] = 0;

		pRenderSgNode->SetLocalMatrix(newLocalMatrix);
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_ScaleDgRectanglePropNodeContext::Apply(const Maths::HVec3& scale)
{	
	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		EditorLib::I_DgNode* pProperty = it->pDgNode;
		if (!pProperty->IsA(EditorLib::E_ObjType_DgRectanglePropNode))
			continue;

		EditorLib::C_DgRectanglePropNode *pRectangleProperty = (EditorLib::C_DgRectanglePropNode*) pProperty;

		F32 width;
		pRectangleProperty->GetWidthPlug().GetValue(width);

		F32 height;
		pRectangleProperty->GetHeightPlug().GetValue(height);

		width *= scale[0]; 
		height *= scale[1];
		
		pRectangleProperty->GetWidthPlug().SetValue(width);
		pRectangleProperty->GetHeightPlug().SetValue(height);
	}

	return EditorLib::E_Status_Success;
}

}