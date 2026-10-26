#include "stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_ScenePointConstraintImport::Import(
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

	Aftk::C_ScenePointConstraintNode* pAftkScenePointConstraintNode = 
		(Aftk::C_ScenePointConstraintNode*)pAftkSceneNode;

	status = C_LimitMixinImport::Import(
		pImporter,
		dagPath,
		(Aftk::C_LimitMixin*)pAftkScenePointConstraintNode,
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
		MPlug mTargetArray = fnDagNode.findPlug("target", &mStatus);
		MCHECK_ERROR_RETURN_1;
		AFTK_ASSERT(mTargetArray.isArray());

		pAftkScenePointConstraintNode->SetNumInputs(mTargetArray.numConnectedElements());
		for (U32 i = 0; i < mTargetArray.numConnectedElements(); i++)
		{
			MPlug element = mTargetArray.connectionByPhysicalIndex(i, &mStatus);
			MCHECK_ERROR_RETURN_1;

			for (U32 j = 0; j < element.numChildren(); j++)
			{
				MPlug child = element.child(j, &mStatus);
				MCHECK_ERROR_RETURN_1;
				std::string strippedPlugName;
				StripNodeNameAndIndexFromPlugName(child.name().asChar(), strippedPlugName);

				if (strippedPlugName == "targetTranslate")
				{
					{
						MPlug x = child.child(0);
						F32 value;
						mStatus = x.getValue(value);
						MCHECK_ERROR_RETURN_1;
						((Aftk::C_StaticPlug<F32>*)pAftkScenePointConstraintNode->GetTargetTranslatePlug(i).GetChildPtr(0))->SetValue(value);
					}
					{
						MPlug y = child.child(1);
						F32 value;
						mStatus = y.getValue(value);
						MCHECK_ERROR_RETURN_1;
						((Aftk::C_StaticPlug<F32>*)pAftkScenePointConstraintNode->GetTargetTranslatePlug(i).GetChildPtr(1))->SetValue(value);
					}
					{
						MPlug z = child.child(2);
						F32 value;
						mStatus = z.getValue(value);
						MCHECK_ERROR_RETURN_1;
						((Aftk::C_StaticPlug<F32>*)pAftkScenePointConstraintNode->GetTargetTranslatePlug(i).GetChildPtr(2))->SetValue(value);
					}
				}
				else if (strippedPlugName == "targetWeight")
				{
					F32 value;
					mStatus = child.getValue(value);
					MCHECK_ERROR_RETURN_1;
					pAftkScenePointConstraintNode->GetTargetWeightPlug(i).SetValue(value);
				}
			}
		}
	}

	{
		MPlug plug = fnDagNode.findPlug("offsetX", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkScenePointConstraintNode->GetTranslateOffsetPlug().GetChildPtr(0))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("offsetY", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkScenePointConstraintNode->GetTranslateOffsetPlug().GetChildPtr(1))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("offsetZ", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkScenePointConstraintNode->GetTranslateOffsetPlug().GetChildPtr(2))->SetValue(value);
	}

	{
		MPlug mTargetWeight = fnDagNode.findPlug("constraintOffsetPolarity", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		mTargetWeight.getValue(value);
		pAftkScenePointConstraintNode->GetOffsetPolarityPlug().SetValue(value);
	}

	{
		MPlug plug = fnDagNode.findPlug("restTranslateX", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkScenePointConstraintNode->GetRestTranslatePlug().GetChildPtr(0))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("restTranslateY", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkScenePointConstraintNode->GetRestTranslatePlug().GetChildPtr(1))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("restTranslateZ", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		plug.getValue(value);
		((Aftk::C_StaticPlug<F32>*)pAftkScenePointConstraintNode->GetRestTranslatePlug().GetChildPtr(2))->SetValue(value);
	}

	{
		MPlug plug = fnDagNode.findPlug("enableRestPosition", &mStatus);
		MCHECK_ERROR_RETURN_1;
		bool value;
		plug.getValue(value);
	
		pAftkScenePointConstraintNode->EnableRestTranslate(value);
	}

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Point constraint " << fnDagNode.name().asChar() << " id " << fnDagNode.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

Aftk::I_Plug* C_ScenePointConstraintImport::FindPlug(
	Aftk::C_SceneNode* pAftkNode,
	const char* pPlugName)
{
	Aftk::C_ScenePointConstraintNode* pAftkPointConstraint = 
		(Aftk::C_ScenePointConstraintNode*)pAftkNode;
	Aftk::I_Plug* pPlug = C_SceneNodeImport::FindPlug(
		pAftkPointConstraint, 
		pPlugName);
	if (pPlug)
		return pPlug; 

	const char* p = strstr(pPlugName, "[");
	if (p)
	{
		Util::C_String plugName(pPlugName);
		S32 begin = plugName.Search("[");
		S32 end = plugName.Search("]");
		Util::C_String indexStr = plugName(begin+1, end);
		S32 index = atoi(indexStr.Chars());

		Util::C_String strippedPlugName = plugName(end+2, plugName.Length());
		
		if (strippedPlugName == "targetTranslate")
			return &pAftkPointConstraint->GetTargetTranslatePlug(index);
		if (strippedPlugName == "targetTranslateX")
			return pAftkPointConstraint->GetTargetTranslatePlug(index).GetChildPtr(0);
		if (strippedPlugName == "targetTranslateY")
			return pAftkPointConstraint->GetTargetTranslatePlug(index).GetChildPtr(1);
		if (strippedPlugName == "targetTranslateZ")
			return pAftkPointConstraint->GetTargetTranslatePlug(index).GetChildPtr(2);

		if (strippedPlugName == "targetWeight")
			return &pAftkPointConstraint->GetTargetWeightPlug(index);

		if (strippedPlugName == "targetParentMatrix")
			return &pAftkPointConstraint->GetTargetParentPlug(index);
	}

	if (!strcmp(pPlugName, "offset"))
		return &pAftkPointConstraint->GetTranslateOffsetPlug();
	if (!strcmp(pPlugName, "offsetX"))
		return pAftkPointConstraint->GetTranslateOffsetPlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "offsetY"))
		return pAftkPointConstraint->GetTranslateOffsetPlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "offsetZ"))
		return pAftkPointConstraint->GetTranslateOffsetPlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "constraintOffsetPolarity"))
		return &pAftkPointConstraint->GetOffsetPolarityPlug();

	if (!strcmp(pPlugName, "constraintTranslate"))
		return &pAftkPointConstraint->GetConstraintTranslatePlug();
	if (!strcmp(pPlugName, "constraintTranslateX"))
		return pAftkPointConstraint->GetConstraintTranslatePlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "constraintTranslateY"))
		return pAftkPointConstraint->GetConstraintTranslatePlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "constraintTranslateZ"))
		return pAftkPointConstraint->GetConstraintTranslatePlug().GetChildPtr(2);

	return 0;
}
