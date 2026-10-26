#include "stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_SceneLodMeshImport::MergeLodMeshesSceneWide(
	C_Importer* pImporter,
	Aftk::C_Scene* pAftkScene,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MSelectionList list;
	MGlobal::getActiveSelectionList(list);

	//Mesh and mesh instance data to delete.
	Ut::UtSet< Aftk::C_SceneNode*> lodLevelsToDelete;

	//Find any kLodGroup nodes.
	{
		MItDag itDag(MItDag::kBreadthFirst, MFn::kLodGroup, &mStatus);
		MCHECK_BEGIN << TRANS_ACQUIRE_DAG_IT_FAIL MCHECK_ERROR_RETURN_END_1;

		for (; !itDag.isDone(); itDag.next()) 
		{
			MDagPath path;
			mStatus = itDag.getPath(path);
			MCHECK_BEGIN << TRANS_ACQUIRE_DAG_PATH_FAIL MCHECK_ERROR_RETURN_END_1;

			if (pOptions->importSelection)
			{
				if (!list.hasItem(path, MObject::kNullObj, &mStatus))
				{
					continue;
				}
				MCHECK_BEGIN << TRANS_QUERY_SELECTION_LIST_FAIL MCHECK_ERROR_RETURN_END_1;
			}

			if (path.childCount() == 0)
			{
				continue;
			}

			status = ImportLodGroup(
				pImporter,
				pAftkScene,
				path,
				lodLevelsToDelete);
			AFTK_CHECK_ERROR_RETURN;
		}
	}

	{
		//Delete the lod meshes that we've used.
		Ut::UtSet< Aftk::C_SceneNode* >::Iterator it;
		for (it = lodLevelsToDelete.Begin(); it != lodLevelsToDelete.End(); it++)
		{
			//Delete the original lod level mesh or mesh instance. This function will preserve the hierarchy.
			pAftkScene->DeleteSceneNodeAndReparentChildren(*it, true);
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneLodMeshImport::CopyLodGroupTransformProporties(
	Aftk::C_SceneNode* pSource,
	Aftk::C_SceneNode* pDestination)
{
	pDestination->SetName(			pSource->GetName());
	pDestination->SetLocalMatrix(	pSource->GetLocalMatrix());

	for (U32 iLayer = 0; iLayer < pSource->GetNumLayers(); iLayer++)
		pSource->GetLayerPtr(iLayer)->AddMember(pDestination);

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneLodMeshImport::ImportLodGroup(
	C_Importer* pImporter,
	Aftk::C_Scene* pAftkScene,
	const MDagPath& lodGroupPath,
	Ut::UtSet< Aftk::C_SceneNode*>& lodLevelsToDelete)
{
	Aftk::C_Status status;
	MStatus mStatus;

	//C_Track unique combinations. For a combination of meshes - fetch a group node we've created.
	//We can then instance this group node.
	Ut::UtMap< LodGroupMeshes, Aftk::C_SceneMeshNode*> groupNodeTracker;

	MFnDagNode fnLodGroupDagNode(lodGroupPath, &mStatus);
	AFTK_CHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL AFTK_CHECK_ERROR_RETURN_END_1;

	//Get the Aftk placeholder node for this lod group
	Aftk::C_SceneNode* pAftkLodGroupPlaceholder = 0;
	if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(lodGroupPath, &pAftkLodGroupPlaceholder))
	{
		AFTK_DO_BEGIN << TRANS_EXPORT_AFTK_MAYA_TO_MAYA_DAG_PATH_FAIL AFTK_DO_ERROR_RETURN_END_1;
	}
	if (pImporter->IsDebris(pAftkLodGroupPlaceholder))
		//Don't process debris
		return Aftk::C_Status::E_Status_Success;

	//Count the number of meshes beneath this LOD node.
	U32 nLodLevels = 0;
	LodGroupMeshes lodGroupMeshes;

	MItDag itDag(MItDag::kDepthFirst, MFn::kTransform, &mStatus);
	AFTK_CHECK_BEGIN << TRANS_ACQUIRE_DAG_IT_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
	itDag.reset(fnLodGroupDagNode.object());

	for (; !itDag.isDone(); itDag.next()) 
	{
		if (itDag.depth() == 0)
		{
			continue;
		}
		MDagPath childPath;
		mStatus = itDag.getPath(childPath);
		AFTK_CHECK_BEGIN << TRANS_ACQUIRE_DAG_PATH_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		if (childPath.apiType() != MFn::kTransform)
		{
			continue;
		}

		std::string path = childPath.fullPathName().asChar();
		if (!childPath.hasFn(MFn::kMesh))
		{
			continue;
		}

		//Find the mesh data
		Aftk::C_SceneNode* pAftkLodLevelMesh = 0;
		Aftk::C_SceneNode* pAftkLodLevel = 0;
		if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(childPath, &pAftkLodLevel))
		{
			AFTK_DO_BEGIN << TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		}
		AFTK_ASSERT(pAftkLodLevel);
		if (pAftkLodLevel->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			Aftk::C_SceneInstanceNode* pAftkInstance = (Aftk::C_SceneInstanceNode*)(pAftkLodLevel);
			pAftkLodLevelMesh = (Aftk::C_SceneMeshNode*)pAftkInstance->GetSourceNodePtr();
			//Delete this lod level later on.
			lodLevelsToDelete.Insert(pAftkInstance);
		}
		else
		{
			pAftkLodLevelMesh = (Aftk::C_SceneMeshNode*)pAftkLodLevel;
			//Delete this lod level later on.
			lodLevelsToDelete.Insert(pAftkLodLevelMesh);
		}
		AFTK_ASSERT(pAftkLodLevelMesh);
		lodGroupMeshes.meshes.PushBack(pAftkLodLevelMesh);

		nLodLevels++;
	}

	//Abort if we have no meshes.
	if (nLodLevels == 0)
	{
		return Aftk::C_Status::E_Status_Success;
	}

	//Check if we've already exported this combination of meshes before.
	{
		Ut::UtMap<LodGroupMeshes, Aftk::C_SceneMeshNode*>::Iterator it;
		it = groupNodeTracker.Find(lodGroupMeshes);
		if (it != groupNodeTracker.End())
		{
			Aftk::C_SceneMeshNode* pAftkLodGroupMeshNode = it->GetValue();
			AFTK_ASSERT(pAftkLodGroupMeshNode);
			//We've already exported this LOD group combination.
			Aftk::C_SceneInstanceNode* pAftkSceneInstanceNode = new Aftk::C_SceneInstanceNode(pAftkScene);
			if (!pAftkLodGroupMeshNode)
			{
				AFTK_DO_BEGIN << TRANS_NEW_INSTANCE_OUT_OF_MEMORY AFTK_DO_ERROR_RETURN_END_1;
			}
			//Copy LOD group transform attributes to our new node.
			status = CopyLodGroupTransformProporties(pAftkLodGroupPlaceholder, pAftkSceneInstanceNode);
			AFTK_CHECK_BEGIN << TRANS_COPY_LOD_GROUP_PROPORTIES_FAIL AFTK_CHECK_ERROR_RETURN_END_1;

			//Attach our LOD group instance to the lod group mesh.
			pAftkSceneInstanceNode->SetSourceNodePtr(pAftkLodGroupMeshNode);

			//Replace the lod group placeholder with our instance node.
			pAftkSceneInstanceNode->Replace(pAftkLodGroupPlaceholder);

			return Aftk::C_Status::E_Status_Success;
		}
	}
	//If we get here then we're going to be exporting a lod, lets create a lod mesh.
	Aftk::C_SceneMeshNode* pAftkLodGroupMeshNode = new Aftk::C_SceneMeshNode(pAftkScene);
	if (!pAftkLodGroupMeshNode)
	{
		AFTK_DO_BEGIN << TRANS_NEW_MESH_OUT_OF_MEMORY AFTK_DO_ERROR_RETURN_END_1;
	}
	//Copy LOD group transform attributes to our new node.
	status = CopyLodGroupTransformProporties(pAftkLodGroupPlaceholder, pAftkLodGroupMeshNode);
	AFTK_CHECK_BEGIN << TRANS_COPY_LOD_GROUP_PROPORTIES_FAIL AFTK_CHECK_ERROR_RETURN_END_1;

	//We've not seen this mesh combination before, add it to our tracker.
	groupNodeTracker[lodGroupMeshes] = pAftkLodGroupMeshNode;
	
	//Grab the threshold value array from the Maya group node.
	MPlug lodThresholdArray = fnLodGroupDagNode.findPlug("threshold", &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;

	AFTK_ASSERT(lodThresholdArray.isArray());
	if (nLodLevels != lodThresholdArray.numElements()+1)
	{
		AFTK_DO_BEGIN << TRANS_LOD_THRESHOLD_LOD_LEVEL_MISMATCH AFTK_DO_ERROR_RETURN_END_1;
	}

	F32* pThresholds = new F32[nLodLevels];
	//Loop thru lod level dag paths
	for (U32 iLodLevel = 0; iLodLevel < lodThresholdArray.numElements(); iLodLevel++)
	{
		MPlug thresholdPlug = lodThresholdArray.elementByLogicalIndex(iLodLevel, &mStatus);
		MCHECK_BEGIN << TRANS_ACQUIRE_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;

		F32 threshold;
		thresholdPlug.getValue(threshold);

		pThresholds[iLodLevel] = threshold;
	}
	pThresholds[lodThresholdArray.numElements()] = 0;


	//Add lods to layer in order. In Maya, the nodes are stored in the order
	//of highest lod -> lowest lod.
	U32 lodLevel = 0;
	itDag.reset(fnLodGroupDagNode.object());

	for (; !itDag.isDone(); itDag.next()) 
	{
		if (itDag.depth() == 0)
		{
			continue;
		}
		MDagPath childPath;
		mStatus = itDag.getPath(childPath);
		MCHECK_BEGIN << TRANS_ACQUIRE_DAG_PATH_FAIL MCHECK_ERROR_RETURN_END_1;
		std::string path = childPath.fullPathName().asChar();
		if (!childPath.hasFn(MFn::kMesh))
		{
			continue;
		}
		if (childPath.apiType() != MFn::kTransform)
		{
			continue;
		}

		//Find the mesh data
		Aftk::C_SceneMeshNode* pAftkLodLevelMesh = 0;
		Aftk::C_SceneNode* pAftkLodLevel = 0;
		if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(childPath, &pAftkLodLevel))
		{
			AFTK_DO_BEGIN << TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL AFTK_DO_ERROR_RETURN_END_1;
		}
		AFTK_ASSERT(pAftkLodLevel);
		if (pAftkLodLevel->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			Aftk::C_SceneInstanceNode* pAftkInstance = (Aftk::C_SceneInstanceNode*)(pAftkLodLevel);
			pAftkLodLevelMesh = (Aftk::C_SceneMeshNode*)pAftkInstance->GetSourceNodePtr();
		}
		else
		{
			pAftkLodLevelMesh = (Aftk::C_SceneMeshNode*)pAftkLodLevel;
		}
	
		//Set the name and threshold for this lod group
		Aftk::C_MeshLodLevelInfo info;
		info.SetName(pAftkLodLevelMesh->GetName());
		info.SetThreshold(pThresholds[lodLevel]);

		pAftkLodGroupMeshNode->SetLodLevelInfo(lodLevel, info);

		//We now add this mesh lod level to our lod mesh.
		pAftkLodGroupMeshNode->SetLodLevelMesh(
			lodLevel, 
			(Aftk::C_SceneMeshNode*)pAftkLodLevelMesh,
			false);

		lodLevel++;
	}
	delete[] pThresholds;

	pAftkLodGroupMeshNode->Replace(pAftkLodGroupPlaceholder);

	return Aftk::C_Status::E_Status_Success;
}
		
