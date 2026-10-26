#include "stdafx.h"

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Dg_Cmd/XFormCmd.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Core.h>

#include <Aftk/AftkTypedefs.h>
#include <Aftk/AftkEnum.h>
#include <Aftk/AftkBounds.h>
#include <Aftk/AftkMaths.h>
#include <Aftk/AftkEuler.h>

namespace EditorLib
{

C_XFormCmd::C_XFormCmd(U32 type) : I_Command(type)
{
}

C_XFormCmd::~C_XFormCmd()
{
}

C_Status C_XFormCmd::Do()
{
	C_Status status;
	C_Selection selection;
	C_Core::Instance().GetSelection(selection);
	const C_Selection::ItemArray& items = selection.GetItems();

	E_Action action;
	if (m_args.IsSet('s'))
		action = E_Action_Scale;
	else if (m_args.IsSet('r'))
		action = E_Action_Rotate;
	else if (m_args.IsSet('h'))
		action = E_Action_Shear;
	else if (m_args.IsSet('t'))
		action = E_Action_Translate;
	else if (m_args.IsSet('m'))
		action = E_Action_Matrix;
	else
		return E_Status_Failure;

	//Manipulator transform in world coords.
	Matrix44 mManipulatorSShRWorld;
	Vector3 manipulatorTransWorld(0,0,0);
	Vector3 scale(1,1,1);
	if (action == E_Action_Matrix)
	{
		const ArgumentDataArray* pData = m_args.GetArgData('m');
		(*pData)[0].GetValue(mManipulatorSShRWorld[0][0]);
		(*pData)[1].GetValue(mManipulatorSShRWorld[0][1]);
		(*pData)[2].GetValue(mManipulatorSShRWorld[0][2]);
		(*pData)[3].GetValue(mManipulatorSShRWorld[0][3]);

		(*pData)[4].GetValue(mManipulatorSShRWorld[1][0]);
		(*pData)[5].GetValue(mManipulatorSShRWorld[1][1]);
		(*pData)[6].GetValue(mManipulatorSShRWorld[1][2]);
		(*pData)[7].GetValue(mManipulatorSShRWorld[1][3]);

		(*pData)[8].GetValue(mManipulatorSShRWorld[2][0]);
		(*pData)[9].GetValue(mManipulatorSShRWorld[2][1]);
		(*pData)[10].GetValue(mManipulatorSShRWorld[2][2]);
		(*pData)[11].GetValue(mManipulatorSShRWorld[2][3]);

		mManipulatorSShRWorld[3][0] = 0;
		mManipulatorSShRWorld[3][1] = 0;
		mManipulatorSShRWorld[3][2] = 0;
		mManipulatorSShRWorld[3][3] = 1.f;
		(*pData)[12].GetValue(manipulatorTransWorld[0]);
		(*pData)[13].GetValue(manipulatorTransWorld[1]);
		(*pData)[14].GetValue(manipulatorTransWorld[2]);
		(*pData)[15].GetValue(manipulatorTransWorld[3]);
	}
	else
	{
		Vector3 translate(0,0,0);
		Vector3 shear(0,0,0);
		Vector3 rotateEuler(0,0,0);
		
		switch(action)
		{
		case E_Action_Scale:
			{
			const ArgumentDataArray* pData = m_args.GetArgData('s');
			(*pData)[0].GetValue(scale[0]);
			(*pData)[1].GetValue(scale[1]);
			(*pData)[2].GetValue(scale[2]);
			break;
			}
		case E_Action_Rotate:
			{
			const ArgumentDataArray* pData = m_args.GetArgData('r');
			(*pData)[0].GetValue(rotateEuler[0]);
			(*pData)[1].GetValue(rotateEuler[1]);
			(*pData)[2].GetValue(rotateEuler[2]);
			break;
			}
		case E_Action_Shear:
			{
			const ArgumentDataArray* pData = m_args.GetArgData('h');
			(*pData)[0].GetValue(shear[0]);
			(*pData)[1].GetValue(shear[1]);
			(*pData)[2].GetValue(shear[2]);
			break;
			}
		case E_Action_Translate:
			{
			const ArgumentDataArray* pData = m_args.GetArgData('t');
			(*pData)[0].GetValue(manipulatorTransWorld[0]);
			(*pData)[1].GetValue(manipulatorTransWorld[1]);
			(*pData)[2].GetValue(manipulatorTransWorld[2]);
			break;
			}
		default:
			break;
		};

		Matrix44 rotate;
		Aftk::EulerToMatrix(rotate, rotateEuler, Aftk::E_RotationOrder_XYZ);

		Aftk::Vector3F32 scalePivot(0,0,0);
		Aftk::Vector3F32 scalePivotTranslation(0,0,0);
		Aftk::Vector3F32 rotatePivot(0,0,0);
		Aftk::Vector3F32 rotatePivotTranslation(0,0,0);
		Aftk::Matrix44 rotateOrientation;
		UtMaths::SetIdentity(&rotateOrientation);
		Aftk::Vector3F32 translation(0,0,0);
		Aftk::BuildMatrix(
			mManipulatorSShRWorld,
			scalePivot,
			Aftk::Vector3F32(1,1,1),
			shear,
			scalePivotTranslation,
			rotatePivot,
			rotateOrientation,
			rotate, 
			rotatePivotTranslation,
			translation);
	}
	{
		//Check matrix is valid.
		F32 det = UtMaths::GetDeterminant4x4(mManipulatorSShRWorld);
		if (det == 0)
			return E_Status_Failure;
	}

	{
		C_CheckpointHelper c(&m_pChange, GetName());
		for (C_Selection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
		{
			C_NodeId id = it->id;
			I_DgNode* pNode = C_Core::Instance().GetDgManager().FindNodeById(id);
			if (!IsInteresting(pNode))
				continue;

			C_SceneNode* pSceneNode = (C_SceneNode*)pNode;

			Aftk::Matrix44 mSceneNodeWorld = pSceneNode->GetWorldMatrix();

			Matrix44 mSceneNodeTransWorld;
			UtMaths::SetIdentity(&mSceneNodeTransWorld);
			(Vector3&)mSceneNodeTransWorld[3] = (Vector3&)mSceneNodeWorld[3];
			mSceneNodeWorld[3][0] = 0.f; mSceneNodeWorld[3][1] = 0.f; mSceneNodeWorld[3][2] = 0.f;

			//Transform by manipulator's relative transformation in world space.
			mSceneNodeWorld = mSceneNodeWorld * mManipulatorSShRWorld;
			mSceneNodeWorld[0][0] *= scale[0]; 	mSceneNodeWorld[0][1] *= scale[0]; 	mSceneNodeWorld[0][2] *= scale[0];	
			mSceneNodeWorld[1][0] *= scale[1]; 	mSceneNodeWorld[1][1] *= scale[1];	mSceneNodeWorld[1][2] *= scale[1];
			mSceneNodeWorld[2][0] *= scale[2];	mSceneNodeWorld[2][1] *= scale[2]; 	mSceneNodeWorld[2][2] *= scale[2];

			(Vector3&)mSceneNodeTransWorld[3] = (Vector3&)mSceneNodeTransWorld[3] + manipulatorTransWorld;

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

			Matrix44 mNewLocalTrans = mSceneNodeTransWorld * invExclusiveWorldMatrixFull;
			(Vector3&)newLocalMatrix[3] = (Vector3&)mNewLocalTrans[3];
			pSceneNode->SetLocalMatrix(newLocalMatrix);
		}
	}

	return E_Status_Success;
}

bool C_XFormCmd::IsUndoable() const
{
	return true;
}

void C_XFormCmd::GetResults(ResultsArray& results) const
{
}

I_Command* C_XFormCmd::Create(U32 type)
{
	return new C_XFormCmd(type);
}

C_Syntax* C_XFormCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		{
			C_Argument arg(
				's', 
				"scale",
				"x y z scale values",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'r', 
				"rotate",
				"x y z rotate values",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'h', 
				"shear",
				"x y z shear values",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				't', 
				"translate",
				"x y z translate values",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'm', 
				"matrix",
				"apply matrix",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();

			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);

			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);

			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);

			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);

			pSyntax->AddArg(arg);
		}
	}

	return pSyntax;
}

const char* C_XFormCmd::GetName()
{
	static char* p = "xform";
	return p;
}

bool C_XFormCmd::IsInteresting(I_DgNode* pNode)
{
	if (pNode && pNode->IsA(E_ObjType_SceneNode))
		return !pNode->IsReadOnly();

	return false;
}

}
