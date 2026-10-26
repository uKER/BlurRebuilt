#include "stdafx.h"

#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Editor/Manipulators/ManipulatorContextMapping.h>
#include <EditorApp/Editor/Manipulators/Contexts/IManipulatorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/IVectorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/MoveSceneNodeContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/SceneNodeContextHelpers.h>

#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/DgNodeSelection.h>

namespace Editor
{

C_MoveSceneNodeContext::C_MoveSceneNodeContext() : I_VectorContext()
{
}

C_MoveSceneNodeContext::~C_MoveSceneNodeContext()
{
}

bool C_MoveSceneNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return SceneNodeContextHelpers::GetInitialMatrix(matrix, space);
}

void* C_MoveSceneNodeContext::Create()
{
	return new C_MoveSceneNodeContext();
}

E_ManipulatorContextType C_MoveSceneNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_MoveSceneNode;
}

bool C_MoveSceneNodeContext::ManipulatorContextRegister()
{
	if (!C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_SceneNode, E_ManipulatorType_Move, 
		C_MoveSceneNodeContext::GetStaticType())) return false;
	return true;
}

EditorLib::C_Status C_MoveSceneNodeContext::Update(const Maths::HVec3& translate)
{
//	LOGDEBUG(g_Log, "UPDTE %.2f %.2f %.2f", (F32)translate.X(), (F32)translate.Y(), (F32)translate.Z());

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
			if (pRenderSgNode )
			{
				Matrix44 mSceneNodeTransWorld;
				UtMaths::SetIdentity(&mSceneNodeTransWorld);
				(Vector3&)mSceneNodeTransWorld[3] = (Vector3&)pSceneNode->GetWorldMatrix()[3];
				(Vector3&)mSceneNodeTransWorld[3] = (Vector3&)mSceneNodeTransWorld[3] + Vector3(translate.X(), translate.Y(), translate.Z());

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

				Matrix44 mNewLocalTrans = mSceneNodeTransWorld * invExclusiveWorldMatrixFull;
				Matrix44 newLocalMatrix = pSceneNode->GetLocalMatrix();
				(Vector3&)newLocalMatrix[3] = (Vector3&)mNewLocalTrans[3];

				Maths::HMat44 localMatrix;
				Aftk::TemplateToNightmareMatrix44(localMatrix, newLocalMatrix);
				pRenderSgNode->SetLocalMatrix(localMatrix);

				EditorLib::C_Core::Instance().GetGlobalSignals().dgNodeTransform(
					*pSceneNode, EditorLib::E_DgNodeTransform_Move);
			}
		}
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_MoveSceneNodeContext::Apply(const Maths::HVec3& translate)
{
//	LOGDEBUG(g_Log, "Apply %.2f %.2f %.2f", (F32)translate.X(), (F32)translate.Y(), (F32)translate.Z());

	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >				Vector3;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >				Vector4;
	typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4, 4> >		Matrix44;

	Btl::String args;
	args.Printf("-t %f %f %f", translate[0], translate[1], translate[2]);

	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("xform", args);
	if (!status)
	{
		LOGERROR(g_Log, "transform failed");
	}
	else
	{
		const C_DgNodeSelection::ItemArray& items = C_RenderCore::Instance().GetRenderScene().GetSelection().GetItems();
		for (C_DgNodeSelection::ItemArray::ConstIterator it=items.Begin(); it!=items.End(); it++)
		{
			if (it->pDgNode->IsA(EditorLib::E_ObjType_SceneNode))
			{
				const EditorLib::C_SceneNode *pSceneNode = (EditorLib::C_SceneNode*)it->pDgNode;
				if (pSceneNode->IsReadOnly())
					continue;

				EditorLib::C_Core::Instance().GetGlobalSignals().dgNodeTransform(
					*pSceneNode, EditorLib::E_DgNodeTransform_EndMove);
			}
		}
	}

	return status;
}

}
