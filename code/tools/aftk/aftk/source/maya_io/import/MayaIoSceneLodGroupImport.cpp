#include "stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_SceneLodGroupImport::Import(	
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	//Export the base AFTK scene node, this includes the bind pose.
	status = C_SceneNodeImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		true,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	Aftk::C_SceneLodGroupNode* pAftkSceneLodGroupNode = (Aftk::C_SceneLodGroupNode*)pAftkSceneNode;

	MFnDagNode fnLodGroupDagNode(dagPath, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Lod Group C_Node " << fnLodGroupDagNode.name().asChar() << " id " << fnLodGroupDagNode.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	//Grab the threshold value array from the Maya group node.
	MPlug lodThresholdArray = fnLodGroupDagNode.findPlug("threshold", &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;
	AFTK_ASSERT(lodThresholdArray.isArray());

	//Loop thru lod level dag paths
	for (U32 iLodLevel = 0; iLodLevel < lodThresholdArray.numElements(); iLodLevel++)
	{
		MPlug thresholdPlug = lodThresholdArray.elementByLogicalIndex(iLodLevel, &mStatus);
		MCHECK_BEGIN << TRANS_ACQUIRE_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;

		F32 threshold;
		thresholdPlug.getValue(threshold);
		Aftk::C_GroupLodLevelInfo info;
		info.SetThreshold(threshold);
		//We'll fill in the scene node pointer after first exporting the rest of the scene.
		info.SetSceneNodePtr(0);
		pAftkSceneLodGroupNode->AddLodLevelInfo(info);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneLodGroupImport::ResolveChildrenSceneWide(	
	C_Importer* pImporter,
	Aftk::C_Scene* pAftkScene,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	Aftk::C_SceneNodeIterator it(pAftkScene);

	for (; it; it++)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;
		if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneLodGroup)
		{
			Aftk::C_SceneLodGroupNode* pAftkSceneLodGroupNode = (Aftk::C_SceneLodGroupNode*)pAftkSceneNode;
			U32 iChild = 0;
			while (true)
			{
				if (iChild >= pAftkSceneLodGroupNode->GetNumLodLevels())
				{
					//Add extra lod level info - one extra always required for last lod level.
					Aftk::C_GroupLodLevelInfo info;
					info.SetThreshold(0);
					info.SetSceneNodePtr(pAftkSceneLodGroupNode->GetChildPtr(iChild));
					pAftkSceneLodGroupNode->AddLodLevelInfo(info);
					break;
				}

				Aftk::C_GroupLodLevelInfo* pInfo = &pAftkSceneLodGroupNode->GetLodLevelInfo(iChild);
				pInfo->SetSceneNodePtr(pAftkSceneLodGroupNode->GetChildPtr(iChild));
				//Let's iterate...
				iChild++;
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

