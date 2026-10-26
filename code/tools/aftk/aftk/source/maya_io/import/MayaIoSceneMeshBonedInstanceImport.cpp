#include "stdafx.h"


using namespace MayaIo;

Aftk::C_Status SceneMeshBonedInstanceImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	status = C_SceneNodeImport::Import(
		pImporter,
		dagPath,
		0,
		pAftkScene,
		pAftkSceneNode,
		false,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	Aftk::C_SceneMeshBonedInstanceNode* pAftkSceneMeshBonedInstanceNode = 
		(Aftk::C_SceneMeshBonedInstanceNode*)pAftkSceneNode;

	MFnDagNode fnTransformDagNode(dagPath, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;

	MDagPath fnShapeDagPath = dagPath;
	if (!fnShapeDagPath.extendToShape())
	{
		AFTK_DO_BEGIN << TRANS_EXTEND_TO_SHAPE_FAIL AFTK_DO_ERROR_RETURN_END_1;
	}
	// Create the appropriate shape node type
	MFnDagNode fnShapeDagNode(fnShapeDagPath, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;

	status = ImportMeshMaterials(
		pImporter,
		fnShapeDagPath, 
		pAftkScene,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	//Get the parent of this boned instance. The boned instance parent is always the lowest LOD of the 
	//boned instance geometry.
	MDagPath bonedInstancePath;
	Aftk::C_SceneNode* pAftkInstanceParent = 0;
	{
		MPlug bonedInstanceParent = fnTransformDagNode.findPlug("bonedInstanceParent", &mStatus);
		MCHECK_BEGIN << TRANS_ACQUIRE_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;
		{
			MString attrString;
			bonedInstanceParent.getValue(attrString);

			#ifdef _DEBUG
			AFTK_DO_BEGIN << "bonedInstanceParent String dyn attr. " << attrString.asChar() << "\n" AFTK_DO_DEBUG_END;
			#endif
			std::string toFind = attrString.asChar();
			if (!pImporter->GetAftkMayaMapping().GetMayaSceneNodeFromNameString(toFind, bonedInstancePath))
			{
				AFTK_DO_BEGIN << TRANS_FIND_BONED_INSTANCE_PARENT_FAIL AFTK_DO_ERROR_RETURN_END_1;
			}
		}

		if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(bonedInstancePath, &pAftkInstanceParent))
		{
			AFTK_DO_BEGIN << TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL AFTK_DO_ERROR_RETURN_END_1;
		}
		if (!pAftkInstanceParent)
		{
			#ifdef _DEBUG
			AFTK_ASSERT(pAftkInstanceParent);
			#endif
			AFTK_DO_BEGIN << TRANS_BONED_INSTANCE_PARENT_INVALID AFTK_DO_ERROR_RETURN_END_1;
		}
		#ifdef _DEBUG
		AFTK_DO_BEGIN << "bonedInstanceParent name : " << pAftkInstanceParent->GetName() << "\n" AFTK_DO_DEBUG_END;
		#endif
		if (pAftkInstanceParent->GetType() != Aftk::E_ObjType_SceneMesh)
		{
			AFTK_DO_BEGIN << TRANS_BONED_INSTANCE_PARENT_INVALID_TYPE AFTK_DO_ERROR_RETURN_END_1;
		}
		pAftkSceneMeshBonedInstanceNode->SetSourceNodePtr((Aftk::C_SceneMeshNode*)pAftkInstanceParent);
	}

	pAftkSceneNode->SetBasePose(pAftkInstanceParent->GetWorldMatrix());

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status SceneMeshBonedInstanceImport::ImportMeshMaterials(	
	C_Importer* pImporter,
	const MDagPath &mayaDagPath,			// source
	Aftk::C_Scene* pAftkScene,					// destination
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MObject cmp;
	MSelectionList slist;

	// If the shape is instanced then we need to determine which
	// instance this path refers to.
	//
	int instanceNum = 0;
	if (mayaDagPath.isInstanced()) 
	{
		instanceNum = mayaDagPath.instanceNumber();
	}

	// Get a list of all sets pertaining to the selected shape and the
	// members of those sets.
	//
	MFnMesh fnMesh(mayaDagPath);
	MObjectArray fPolygonSets;
	MObjectArray fPolygonComponents;
	mStatus = fnMesh.getConnectedSetsAndMembers(instanceNum, fPolygonSets, fPolygonComponents, true);
	MCHECK_BEGIN << "Failed to get material proporties for one of the mesh objects." MCHECK_ERROR_RETURN_END_1

	// Loop through all the sets.  If the set is a polygonal set, find the
	// shader attached to the and print out the texture file name for the
	// set along with the polygons in the set.
	//
	bool skipThisSet = false;
	bool materialsCorrupt = false;

	for (U32 i = 0; i < fPolygonSets.length(); i++) 
	{
		MObject set = fPolygonSets[i];
		MObject comp = fPolygonComponents[i];

		MFnSet fnSet(set, &mStatus);
		MCHECK_BEGIN << "Error aquiring MFnSet" MCHECK_ERROR_RETURN_END_1

		MObject shaderNode = C_MaterialHelper::FindShader(set, status);
		AFTK_CHECK_ERROR_RETURN;

		if (shaderNode == MObject::kNullObj)
		{
			// Skip this material shader.
			continue;
		}
	
		// Source material shader
		MFnDependencyNode shaderDependencyNode = MFnDependencyNode(shaderNode);

		// Destination material
		C_MaterialImport::Import(
			pImporter,
			shaderDependencyNode,
			pAftkScene,
			0,
			status);
		AFTK_CHECK_ERROR_RETURN;
	}

	return Aftk::C_Status::E_Status_Success;
}

