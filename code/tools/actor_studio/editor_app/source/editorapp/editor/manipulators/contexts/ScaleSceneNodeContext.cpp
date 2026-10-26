#include "stdafx.h"

#include <EditorApp/Editor/Manipulators/ManipulatorContextMapping.h>
#include <EditorApp/Editor/Manipulators/Contexts/IManipulatorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/IVectorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/ScaleSceneNodeContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/SceneNodeContextHelpers.h>
#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/DgNodeSelection.h>

#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Core.h>

namespace Editor
{

C_ScaleSceneNodeContext::C_ScaleSceneNodeContext() : I_VectorContext()
{
}

C_ScaleSceneNodeContext::~C_ScaleSceneNodeContext()
{
}

bool C_ScaleSceneNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return SceneNodeContextHelpers::GetInitialMatrix(matrix, space);
}

void* C_ScaleSceneNodeContext::Create()
{
	return new C_ScaleSceneNodeContext();
}

E_ManipulatorContextType C_ScaleSceneNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_ScaleSceneNode;
}

bool C_ScaleSceneNodeContext::ManipulatorContextRegister()
{
	if (!C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_SceneNode, E_ManipulatorType_Scale, 
		C_ScaleSceneNodeContext::GetStaticType())) return false;
	return true;
}

EditorLib::C_Status C_ScaleSceneNodeContext::Update(const Maths::HVec3& scale)
{
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >				Vector3;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >				Vector4;
	typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4, 4> >		Matrix44;

	const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
	for (C_DgNodeSelection::ItemArray::ConstIterator it=items.Begin(); it!=items.End(); it++)
	{
		if (it->pDgNode->IsA(EditorLib::E_ObjType_SceneNode))
		{
			const EditorLib::C_SceneNode *pSceneNode = (EditorLib::C_SceneNode*)it->pDgNode;
			if (pSceneNode->IsReadOnly())
				continue;

			AftkRender::C_SceneGraphNode *pRenderSgNode = C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pSceneNode);
			if (!pRenderSgNode )
				continue;

			Aftk::Matrix44 mSceneNodeWorld = pSceneNode->GetWorldMatrix();
			mSceneNodeWorld[3][0] = 0.f; mSceneNodeWorld[3][1] = 0.f; mSceneNodeWorld[3][2] = 0.f;
			Vector3 localTrans = (Vector3&)pSceneNode->GetLocalMatrix()[3];

			//Scaling is applied along the axes of the scene node.
			mSceneNodeWorld[0][0] *= scale[0]; 	mSceneNodeWorld[0][1] *= scale[0]; 	mSceneNodeWorld[0][2] *= scale[0];	
			mSceneNodeWorld[1][0] *= scale[1]; 	mSceneNodeWorld[1][1] *= scale[1];	mSceneNodeWorld[1][2] *= scale[1];
			mSceneNodeWorld[2][0] *= scale[2];	mSceneNodeWorld[2][1] *= scale[2]; 	mSceneNodeWorld[2][2] *= scale[2];

			//Acquire the inverse world matrix of our parent.
			Aftk::Matrix44 invExclusiveWorldMatrix;
			if (!pSceneNode->GetParent())
				UtMaths::SetIdentity(&invExclusiveWorldMatrix);
			else
			{
				Matrix44 mWorld = pSceneNode->GetParent()->GetWorldMatrix();
				//Check matrix is valid.
				F32 det = UtMaths::GetDeterminant4x4(mWorld);
				if (det == 0)
					continue;
				UtMaths::Invert4x4(&invExclusiveWorldMatrix, mWorld);
			}
			Matrix44 invExclusiveWorldMatrixFull = invExclusiveWorldMatrix;
			invExclusiveWorldMatrix[3][0] = 0.f; invExclusiveWorldMatrix[3][1] = 0.f; invExclusiveWorldMatrix[3][2] = 0.f;

			//Calculate new local matrix.
			Aftk::Matrix44 newLocalMatrix = mSceneNodeWorld * invExclusiveWorldMatrix;
			(Vector3&)newLocalMatrix[3] = localTrans;

			Maths::HMat44 localMatrix;
			Aftk::TemplateToNightmareMatrix44(localMatrix, newLocalMatrix);

			pRenderSgNode->SetLocalMatrix(localMatrix);
		}
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_ScaleSceneNodeContext::Apply(const Maths::HVec3& scale)
{
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >				Vector3;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >				Vector4;
	typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4, 4> >		Matrix44;

	Btl::String args;
	args.Printf("-s %f %f %f", scale[0], scale[1], scale[2]);

	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("xform", args);

	if (!status)
		LOGERROR(g_Log, "transform failed");

	return EditorLib::E_Status_Success;
}

}



