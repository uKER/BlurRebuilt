#include "stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_SceneAimConstraintImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;
	//Import the base AFTK scene node
	status = C_SceneNodeImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		true,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	Aftk::C_SceneAimConstraintNode* pAftkSceneAimConstraintNode = 
		(Aftk::C_SceneAimConstraintNode*)pAftkSceneNode;

	status = C_LimitMixinImport::Import(
		pImporter,
		dagPath,
		(Aftk::C_LimitMixin*)pAftkSceneAimConstraintNode,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	MFnDagNode fnDagNode(dagPath, &mStatus);
	if (!mStatus)
	{
		if (pOptions->strict)
		{
			AFTK_DO_BEGIN << TRANS_ACQUIRE_FN_FAIL AFTK_DO_ERROR_RETURN_END_0;
		}
		else
		{
			AFTK_DO_BEGIN << TRANS_ACQUIRE_FN_WARN AFTK_DO_WARNING_END;
			pImporter->AddDebris(pAftkSceneNode);
			return Aftk::C_Status::E_Status_Success;
		}
	}

	{
		MPlug plug = fnDagNode.findPlug("aimVectorX", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetAimVectorPlug().GetChildPtr(0))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("aimVectorY", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetAimVectorPlug().GetChildPtr(1))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("aimVectorZ", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetAimVectorPlug().GetChildPtr(2))->SetValue(value);
	}

	{
		MPlug plug = fnDagNode.findPlug("upVectorX", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetUpVectorPlug().GetChildPtr(0))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("upVectorY", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetUpVectorPlug().GetChildPtr(1))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("upVectorZ", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetUpVectorPlug().GetChildPtr(2))->SetValue(value);
	}

	{
		MPlug plug = fnDagNode.findPlug("offsetX", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetRotationalOffsetPlug().GetChildPtr(0))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("offsetY", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetRotationalOffsetPlug().GetChildPtr(1))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("offsetZ", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetRotationalOffsetPlug().GetChildPtr(2))->SetValue(value);
	}

	{
		MPlug plug = fnDagNode.findPlug("worldUpType", &mStatus);
		AFTK_ASSERT(mStatus);
		S32 value;
		plug.getValue(value);
		switch (value)
		{
		case 0:
			pAftkSceneAimConstraintNode->GetWorldUpVectorTypePlug().SetValue(
				(U32)Aftk::E_UpVectorType_SceneUp);
			break;

		case 1:
			pAftkSceneAimConstraintNode->GetWorldUpVectorTypePlug().SetValue(
				(U32)Aftk::E_UpVectorType_ObjectUp);
			break;

		case 2:
			pAftkSceneAimConstraintNode->GetWorldUpVectorTypePlug().SetValue(
				(U32)Aftk::E_UpVectorType_ObjectRotationUp);
			break;

		case 3:
			pAftkSceneAimConstraintNode->GetWorldUpVectorTypePlug().SetValue(
				(U32)Aftk::E_UpVectorType_Vector);
			break;

		case 4:
		default:
			pAftkSceneAimConstraintNode->GetWorldUpVectorTypePlug().SetValue(
				(U32)Aftk::E_UpVectorType_None);
			break;
		}
	}


	{
		MPlug plug = fnDagNode.findPlug("worldUpVectorX", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetWorldUpVectorPlug().GetChildPtr(0))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("worldUpVectorY", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetWorldUpVectorPlug().GetChildPtr(1))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("worldUpVectorZ", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkSceneAimConstraintNode->GetWorldUpVectorPlug().GetChildPtr(2))->SetValue(value);
	}

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Aim constraint " << fnDagNode.name().asChar() << " id " << fnDagNode.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

Aftk::I_Plug* C_SceneAimConstraintImport::FindPlug(
	Aftk::C_SceneNode* pAftkNode,
	const char* pPlugName)
{
	Aftk::C_SceneAimConstraintNode* pAftkAimConstraint = 
		(Aftk::C_SceneAimConstraintNode*)pAftkNode;
	Aftk::I_Plug* pPlug = C_SceneNodeImport::FindPlug(
		pAftkAimConstraint, 
		pPlugName);
	if (pPlug)
		return pPlug; 

	if (!strcmp(pPlugName, "constraintTranslate"))
		return &pAftkAimConstraint->GetConstraintTranslatePlug();
	if (!strcmp(pPlugName, "constraintTranslateX"))
		return pAftkAimConstraint->GetConstraintTranslatePlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "constraintTranslateY"))
		return pAftkAimConstraint->GetConstraintTranslatePlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "constraintTranslateZ"))
		return pAftkAimConstraint->GetConstraintTranslatePlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "target[0].targetTranslate"))
		return &pAftkAimConstraint->GetTargetTranslatePlug();
	if (!strcmp(pPlugName, "target[0].targetTranslateX"))
		return pAftkAimConstraint->GetTargetTranslatePlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "target[0].targetTranslateY"))
		return pAftkAimConstraint->GetTargetTranslatePlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "target[0].targetTranslateZ"))
		return pAftkAimConstraint->GetTargetTranslatePlug().GetChildPtr(2);
	if (!strcmp(pPlugName, "target[0].targetParentMatrix"))
		return &pAftkAimConstraint->GetTargetParentPlug();

	if (!strcmp(pPlugName, "constraintParentInverseMatrix"))
		return &pAftkAimConstraint->GetConstraintParentPlug();

	if (!strcmp(pPlugName, "aimVector"))
		return &pAftkAimConstraint->GetAimVectorPlug();
	if (!strcmp(pPlugName, "aimVectorX"))
		return pAftkAimConstraint->GetAimVectorPlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "aimVectorY"))
		return pAftkAimConstraint->GetAimVectorPlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "aimVectorZ"))
		return pAftkAimConstraint->GetAimVectorPlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "upVector"))
		return &pAftkAimConstraint->GetUpVectorPlug();
	if (!strcmp(pPlugName, "upVectorX"))
		return pAftkAimConstraint->GetUpVectorPlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "upVectorY"))
		return pAftkAimConstraint->GetUpVectorPlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "upVectorZ"))
		return pAftkAimConstraint->GetUpVectorPlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "offset"))
		return &pAftkAimConstraint->GetRotationalOffsetPlug();
	if (!strcmp(pPlugName, "offsetX"))
		return pAftkAimConstraint->GetRotationalOffsetPlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "offsetY"))
		return pAftkAimConstraint->GetRotationalOffsetPlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "offsetZ"))
		return pAftkAimConstraint->GetRotationalOffsetPlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "worldUpType"))
		return &pAftkAimConstraint->GetWorldUpVectorTypePlug();

	if (!strcmp(pPlugName, "worldUpVector"))
		return &pAftkAimConstraint->GetWorldUpVectorPlug();
	if (!strcmp(pPlugName, "worldUpVectorX"))
		return pAftkAimConstraint->GetWorldUpVectorPlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "worldUpVectorY"))
		return pAftkAimConstraint->GetWorldUpVectorPlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "worldUpVectorZ"))
		return pAftkAimConstraint->GetWorldUpVectorPlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "constraintRotateOrder"))
		return &pAftkAimConstraint->GetConstraintRotateOrderPlug();

	if (!strcmp(pPlugName, "constraintRotate"))
		return &pAftkAimConstraint->GetConstraintRotatePlug();
	if (!strcmp(pPlugName, "constraintRotateX"))
		return pAftkAimConstraint->GetConstraintRotatePlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "constraintRotateY"))
		return pAftkAimConstraint->GetConstraintRotatePlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "constraintRotateZ"))
		return pAftkAimConstraint->GetConstraintRotatePlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "worldUpMatrix"))
		return &pAftkAimConstraint->GetWorldUpMatrixPlug();

	return 0;
}
