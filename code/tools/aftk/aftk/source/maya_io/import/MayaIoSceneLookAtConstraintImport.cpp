#include "stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_SceneLookAtConstraintImport::Import(
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
	status = C_SceneAimConstraintImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	Aftk::C_SceneLookAtConstraintNode* pAftkSceneLookAtConstraintNode = 
		(Aftk::C_SceneLookAtConstraintNode*)pAftkSceneNode;

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
		MPlug plug = fnDagNode.findPlug("distanceBetween", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		pAftkSceneLookAtConstraintNode->GetDistanceBetweenPlug().SetValue(value);
	}
	{
		MPlug plug = fnDagNode.findPlug("twist", &mStatus);
		AFTK_ASSERT(mStatus);
		F32 value;
		plug.getValue(value);
		pAftkSceneLookAtConstraintNode->GetRollPlug().SetValue(value);
	}

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Look at node " << fnDagNode.name().asChar() << " id " << fnDagNode.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	pAftkSceneLookAtConstraintNode->GetUpVectorPlug().OnChanged();
	return Aftk::C_Status::E_Status_Success;
}

Aftk::I_Plug* C_SceneLookAtConstraintImport::FindPlug(
	Aftk::C_SceneNode* pAftkNode,
	const char* pPlugName)
{
	Aftk::C_SceneLookAtConstraintNode* pAftkLookAtConstraint = 
		(Aftk::C_SceneLookAtConstraintNode*)pAftkNode;
	Aftk::I_Plug* pPlug = C_SceneAimConstraintImport::FindPlug(
		pAftkLookAtConstraint, 
		pPlugName);
	if (pPlug)
		return pPlug; 

	if (!strcmp(pPlugName, "twist"))
		return &pAftkLookAtConstraint->GetRollPlug();
	if (!strcmp(pPlugName, "twistX"))
		return pAftkLookAtConstraint->GetRollPlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "twistY"))
		return pAftkLookAtConstraint->GetRollPlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "twistZ"))
		return pAftkLookAtConstraint->GetRollPlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "distanceBetween"))
		return &pAftkLookAtConstraint->GetDistanceBetweenPlug();

	return 0;
}
