#include "stdafx.h"

#include <EditorApp/Editor/Manipulators/ManipulatorContextMapping.h>
#include <EditorApp/Editor/Manipulators/Contexts/IManipulatorContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/IOrientationContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/RotateSceneNodeContext.h>
#include <EditorApp/Editor/Manipulators/Contexts/SceneNodeContextHelpers.h>
#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/DgNodeSelection.h>

#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <Aftk/AftkEuler.h>

#include <core/maths/hvec/new/IncludeGeneral.inl>

namespace Editor
{

C_RotateSceneNodeContext::C_RotateSceneNodeContext() : I_OrientationContext()
{
}

C_RotateSceneNodeContext::~C_RotateSceneNodeContext()
{
}

bool C_RotateSceneNodeContext::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	return SceneNodeContextHelpers::GetInitialMatrix(matrix, space);
}

void* C_RotateSceneNodeContext::Create()
{
	return new C_RotateSceneNodeContext();
}

E_ManipulatorContextType C_RotateSceneNodeContext::GetStaticType()
{
	return E_ManipulatorContextType_RotateSceneNode;
}

bool C_RotateSceneNodeContext::ManipulatorContextRegister()
{
	if (!C_ManipulatorContextMapping::Instance().Register(
		EditorLib::E_ObjType_SceneNode, E_ManipulatorType_Rotate, 
		C_RotateSceneNodeContext::GetStaticType())) return false;
	return true;
}

EditorLib::C_Status C_RotateSceneNodeContext::Update(const Maths::HVec3& axis, F32 angle)
{
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >				Vector3;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >				Vector4;
	typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4, 4> >		Matrix44;


	Maths::C_Quaternion q0(axis, angle);

	q0.Normalise();
	Maths::HMat44 rot(q0);

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

			Aftk::Matrix44 mManipulatorSShRWorld;
			Aftk::NightmareToTemplateMatrix44(mManipulatorSShRWorld, rot);

			//Transform by manipulator's relative transformation in world space.
			mSceneNodeWorld = mSceneNodeWorld * mManipulatorSShRWorld;

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

			EditorLib::C_Core::Instance().GetGlobalSignals().dgNodeTransform(
					*pSceneNode, EditorLib::E_DgNodeTransform_Rotate);
		}
	}

	return EditorLib::E_Status_Success;
}

EditorLib::C_Status C_RotateSceneNodeContext::Apply(const Maths::HVec3& axis, F32 angle)
{
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >				Vector3;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >				Vector4;
	typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4, 4> >		Matrix44;

	Maths::C_Quaternion q0(axis, angle);

	q0.Normalise();
	Maths::HMat33 rot(q0);

	Maths::EulerXYZ euler = Maths::MakeEulerXYZ( rot );
	Btl::String args;
	args.Printf("-r %f %f %f", Maths::X(euler).m_V, Maths::Y(euler).m_V, Maths::Z(euler).m_V);

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
					*pSceneNode, EditorLib::E_DgNodeTransform_EndRotate);
			}
		}
	}

	return EditorLib::E_Status_Success;
}

}
