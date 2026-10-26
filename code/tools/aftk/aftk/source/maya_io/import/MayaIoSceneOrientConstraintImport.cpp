#include "stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_SceneOrientConstraintImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
//	if (pAftkSceneNode->GetUniqueId() == 281)
//		int x = 1;
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

	Aftk::C_SceneOrientConstraintNode* pAftkSceneOrientConstraintNode = 
		(Aftk::C_SceneOrientConstraintNode*)pAftkSceneNode;

	status = C_LimitMixinImport::Import(
		pImporter,
		dagPath,
		(Aftk::C_LimitMixin*)pAftkSceneOrientConstraintNode,
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

		U32 highestLogicalIndex(0);
		for (U32 i = 0; i < mTargetArray.numConnectedElements(); i++)
		{
			MPlug element = mTargetArray.connectionByPhysicalIndex(i, &mStatus);
			if (element.logicalIndex() > highestLogicalIndex)
				highestLogicalIndex = element.logicalIndex();
		}

		pAftkSceneOrientConstraintNode->SetNumInputs(highestLogicalIndex+1);
		for (U32 i = 0; i < mTargetArray.numConnectedElements(); i++)
		{
			MPlug element = mTargetArray.connectionByPhysicalIndex(i, &mStatus);
			MCHECK_ERROR_RETURN_1;

			U32 logicalIndex = element.logicalIndex();

			for (U32 j = 0; j < element.numChildren(); j++)
			{
				MPlug child = element.child(j, &mStatus);
				MCHECK_ERROR_RETURN_1;
				std::string strippedPlugName;
				StripNodeNameAndIndexFromPlugName(child.name().asChar(), strippedPlugName);
		
				if (strippedPlugName == "targetRotate")
				{
					{
						MPlug x = child.child(0);
						F32 value;
						mStatus = x.getValue(value);
						MCHECK_ERROR_RETURN_1;
						((Aftk::C_StaticPlug<F32>*)pAftkSceneOrientConstraintNode->GetTargetRotatePlug(logicalIndex).GetChildPtr(0))->SetValue(value);
					}
					{
						MPlug y = child.child(1);
						F32 value;
						mStatus = y.getValue(value);
						MCHECK_ERROR_RETURN_1;
						((Aftk::C_StaticPlug<F32>*)pAftkSceneOrientConstraintNode->GetTargetRotatePlug(logicalIndex).GetChildPtr(1))->SetValue(value);
					}
					{
						MPlug z = child.child(2);
						F32 value;
						mStatus = z.getValue(value);
						MCHECK_ERROR_RETURN_1;
						((Aftk::C_StaticPlug<F32>*)pAftkSceneOrientConstraintNode->GetTargetRotatePlug(logicalIndex).GetChildPtr(2))->SetValue(value);
					}
				}
				else if (strippedPlugName == "targetWeight")
				{
					F32 value;
					mStatus = child.getValue(value);
					MCHECK_ERROR_RETURN_1;
					pAftkSceneOrientConstraintNode->GetTargetWeightPlug(logicalIndex).SetValue(value);
				}
			}
		}
	}

	{
		MPlug plug = fnDagNode.findPlug("offsetX", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		mStatus = plug.getValue(value);
		MCHECK_ERROR_RETURN_1;
		((Aftk::C_StaticPlug<F32>*)pAftkSceneOrientConstraintNode->GetRotateOffsetPlug().GetChildPtr(0))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("offsetY", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		mStatus = plug.getValue(value);
		MCHECK_ERROR_RETURN_1;
		((Aftk::C_StaticPlug<F32>*)pAftkSceneOrientConstraintNode->GetRotateOffsetPlug().GetChildPtr(1))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("offsetZ", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		mStatus = plug.getValue(value);
		MCHECK_ERROR_RETURN_1;
		((Aftk::C_StaticPlug<F32>*)pAftkSceneOrientConstraintNode->GetRotateOffsetPlug().GetChildPtr(2))->SetValue(value);
	}

	{
		MPlug plug = fnDagNode.findPlug("restRotateX", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		mStatus = plug.getValue(value);
		MCHECK_ERROR_RETURN_1;
		((Aftk::C_StaticPlug<F32>*)pAftkSceneOrientConstraintNode->GetRestRotatePlug().GetChildPtr(0))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("restRotateY", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		mStatus = plug.getValue(value);
		MCHECK_ERROR_RETURN_1;
		((Aftk::C_StaticPlug<F32>*)pAftkSceneOrientConstraintNode->GetRestRotatePlug().GetChildPtr(1))->SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("restRotateZ", &mStatus);
		MCHECK_ERROR_RETURN_1;
		F32 value;
		mStatus = plug.getValue(value);
		MCHECK_ERROR_RETURN_1;
		((Aftk::C_StaticPlug<F32>*)pAftkSceneOrientConstraintNode->GetRestRotatePlug().GetChildPtr(2))->SetValue(value);
	}

	{
		MPlug plug = fnDagNode.findPlug("enableRestPosition", &mStatus);
		MCHECK_ERROR_RETURN_1;
		bool value;
		mStatus = plug.getValue(value);
		MCHECK_ERROR_RETURN_1;
	
		pAftkSceneOrientConstraintNode->EnableRestRotate(value);
	}

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Orient constraint " << fnDagNode.name().asChar() << " id " << fnDagNode.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

Aftk::I_Plug* C_SceneOrientConstraintImport::FindPlug(
	Aftk::C_SceneNode* pAftkNode,
	const char* pPlugName)
{
	Aftk::C_SceneOrientConstraintNode* pAftkOrientConstraint = 
		(Aftk::C_SceneOrientConstraintNode*)pAftkNode;
	Aftk::I_Plug* pPlug = C_SceneNodeImport::FindPlug(
		pAftkOrientConstraint, 
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
		
		if (strippedPlugName == "targetRotate")
			return &pAftkOrientConstraint->GetTargetRotatePlug(index);
		if (strippedPlugName == "targetRotateX")
			return pAftkOrientConstraint->GetTargetRotatePlug(index).GetChildPtr(0);
		if (strippedPlugName == "targetRotateY")
			return pAftkOrientConstraint->GetTargetRotatePlug(index).GetChildPtr(1);
		if (strippedPlugName == "targetRotateZ")
			return pAftkOrientConstraint->GetTargetRotatePlug(index).GetChildPtr(2);

		if (strippedPlugName == "targetWeight")
			return &pAftkOrientConstraint->GetTargetWeightPlug(index);
	}
	if (!strcmp(pPlugName, "offset"))
		return &pAftkOrientConstraint->GetRotateOffsetPlug();
	if (!strcmp(pPlugName, "offsetX"))
		return pAftkOrientConstraint->GetRotateOffsetPlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "offsetY"))
		return pAftkOrientConstraint->GetRotateOffsetPlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "offsetZ"))
		return pAftkOrientConstraint->GetRotateOffsetPlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "constraintRotate"))
		return &pAftkOrientConstraint->GetConstraintRotatePlug();
	if (!strcmp(pPlugName, "constraintRotateX"))
		return pAftkOrientConstraint->GetConstraintRotatePlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "constraintRotateY"))
		return pAftkOrientConstraint->GetConstraintRotatePlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "constraintRotateZ"))
		return pAftkOrientConstraint->GetConstraintRotatePlug().GetChildPtr(2);

	return 0;
}
