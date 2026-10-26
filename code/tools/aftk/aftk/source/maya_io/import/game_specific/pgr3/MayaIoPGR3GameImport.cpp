#include "stdafx.h"

#include "../MayaIoIGameImport.h"
#include "MayaIoPGR3GameImport.h"
//#pragma message("TODO:REMOVE OLD STRING PROC")
#include <core/utility/stringproc.h>

#define PGR3_THRESHOLD_INCREMENT 20.0f
#define TRANS_PGR3_TO_OVERRIDE_FIND_FAIL "Cannot find specified material to override."
#define TRANS_PGR3_MATERIAL_FIND_FAIL "Cannot find specified material [" << searchName << "]. Try looking for material in scene, and making sure its name has not got any extra text"

using namespace MayaIo;

C_PGR3GameImport::~C_PGR3GameImport()
{
}

Aftk::C_Status C_PGR3GameImport::PreProcessScene(
	C_Importer* pImporter,
	const C_ImportOptions* pOptions)
{
	if (pOptions->deformedInstancesAsMesh)
	{
		return Aftk::C_Status::E_Status_Success;
	}

	Aftk::C_Status status;
	return Aftk::C_Status::E_Status_Success;
}

bool C_PGR3GameImport::IsReferenceInstance(const MDagPath& path) const
{
	MStatus mStatus;
	MFnDagNode fnTransform(path, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;

	MPlug lodPlugArray = fnTransform.findPlug("normalInstanceParent", &mStatus);
	if (mStatus)
	{
		return true;
	}

	return false;
}

Aftk::C_Status C_PGR3GameImport::PostProcessScene(
	C_Importer* pImporter,
	const C_ImportOptions* pOptions)
{
	if (pOptions->deformedInstancesAsMesh)
	{
		return Aftk::C_Status::E_Status_Success;
	}

	Aftk::C_Status status;
	MStatus mStatus;
	Aftk::C_Scene* pAftkScene = pImporter->GetAftkScenePtr();

	MSelectionList list;
	MGlobal::getActiveSelectionList(list);

	MItDag itDag(MItDag::kBreadthFirst, MFn::kTransform, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_IT_FAIL MCHECK_ERROR_RETURN_END_1
	for (; !itDag.isDone() && itDag.depth() <= 1; itDag.next()) 
	{
		MDagPath transformDagPath;
		mStatus = itDag.getPath(transformDagPath);
		MCHECK_BEGIN << TRANS_ACQUIRE_DAG_PATH_FAIL MCHECK_ERROR_RETURN_END_1
		status = PostProcessDagNode(
			pImporter,
			pOptions,
			transformDagPath,
			list);
		CHECK_ERROR_RETURN;
	}

	//Remove unused scene nodes if we've performed lod level mesh merging.
	if (pOptions->mergeLodMeshes)
	{
		status = MergeBonedInstanceLodMeshes(
			pImporter,
			pOptions);
		CHECK_ERROR_RETURN;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR3GameImport::PostProcessDagNode(
	C_Importer* pImporter,
	const C_ImportOptions* pOptions,
	const MDagPath& transformDagPath,
	const MSelectionList& list)
{
	Aftk::C_Status status;
	MStatus mStatus;
	bool exportCurrentNode = true;

	if (pOptions->importSelection)
	{
		if (!list.hasItem(transformDagPath, MObject::kNullObj, &mStatus))
		{
			exportCurrentNode = false;
		}
		MCHECK_BEGIN << TRANS_QUERY_SELECTION_LIST_FAIL MCHECK_ERROR_RETURN_END_1;
	}
	
	//Only process nodes within our set of nodes to be exported.
	if (exportCurrentNode)
	{
		if (pImporter->IsBonedInstance(transformDagPath))
		{
			status = ImportBonedInstance(
				pImporter, 
				pOptions, 
				transformDagPath);
			CHECK_ERROR_RETURN;
		}
		else if (IsReferenceInstance(transformDagPath))
		{
			status = ImportReferenceInstance(
				pImporter,
				pOptions,
				transformDagPath);
		}
	}

	for (U32 iChild = 0; iChild < transformDagPath.childCount(); iChild++)
	{
		MObject childObj = transformDagPath.child(iChild, &mStatus);
		if (!childObj.hasFn(MFn::kTransform))
			continue;
		MCHECK_ERROR_RETURN_1;
		MFnDagNode childNode(childObj, &mStatus);
		MCHECK_ERROR_RETURN_1;
		MDagPath childPath;
		mStatus = childNode.getPath(childPath);
		MCHECK_ERROR_RETURN_1;

		PostProcessDagNode(
			pImporter,
			pOptions,
			childPath,
			list);
		CHECK_ERROR_RETURN;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR3GameImport::OverrideMaterials(	
	C_Importer* pImporter,
	const C_ImportOptions* pOptions,
	const MDagPath& instanceSourcePath,
	const MDagPath& instancePath,
	Aftk::C_SceneMeshRemappableNode* pRemappedInstance)
{
	MStatus mStatus;
	MFnDagNode fnInstanceSource(instanceSourcePath, &mStatus);
	MFnDagNode fnInstance(instancePath, &mStatus);

	//Get remapped material pointer.
	S32 materialReplaceIndex = -1;
	const Aftk::C_MaterialResource* pOverrideMaterialRes = 0;
	S32 endCapMaterialReplaceIndex = -1;
	const Aftk::C_MaterialResource* pOverrideEndCapMaterialRes = 0;

	//Find the material to replace.
	MPlug originalMaterialName = fnInstanceSource.findPlug("materialToOverride", &mStatus);
	if (mStatus)
	{
		{
			MString name;
			originalMaterialName.getValue(name);
			std::string searchName(name.asChar());
			const Aftk::C_MaterialResource* pOriginalMaterialRes = 
				pImporter->GetAftkScenePtr()->GetMaterialResourcePtrBySubString(searchName);
			if (pOriginalMaterialRes)
			{
				materialReplaceIndex = 
					pRemappedInstance->GetPolygonMaterialMappingIndex(pOriginalMaterialRes);
			}
			if (!pOriginalMaterialRes || materialReplaceIndex == -1)
			{
				AFTK_DO_BEGIN << TRANS_PGR3_TO_OVERRIDE_FIND_FAIL AFTK_DO_ERROR_END_1;
			}
		}
		{
			MPlug materialName = fnInstance.findPlug("overrideMaterial", &mStatus);
			if (mStatus)
			{
				MString name;
				materialName.getValue(name);
				std::string searchName(name.asChar());
				pOverrideMaterialRes = pImporter->GetAftkScenePtr()->GetMaterialResourcePtrBySubString(searchName);
	
				if (!pOverrideMaterialRes)
				{
					AFTK_DO_BEGIN << TRANS_PGR3_MATERIAL_FIND_FAIL AFTK_DO_ERROR_END_1;
				}
			}
		}
	}

	//Find the material to replace.
	MPlug endCapOriginalMaterialName = fnInstanceSource.findPlug("endcapMaterialsToOverride", &mStatus);
	if (mStatus)
	{
		{
			MString name;
			endCapOriginalMaterialName.getValue(name);
			std::string searchName(name.asChar());
			const Aftk::C_MaterialResource* pOriginalEndCapMaterialRes =
				pImporter->GetAftkScenePtr()->GetMaterialResourcePtrBySubString(searchName);
			if (pOriginalEndCapMaterialRes)
			{
				endCapMaterialReplaceIndex = 
					pRemappedInstance->GetPolygonMaterialMappingIndex(pOriginalEndCapMaterialRes);
			}
	
			if (!pOriginalEndCapMaterialRes || endCapMaterialReplaceIndex == -1)
			{
				AFTK_DO_BEGIN << TRANS_PGR3_TO_OVERRIDE_FIND_FAIL AFTK_DO_ERROR_END_1;
			}
		}
		{
			MPlug endCapMaterialName = fnInstance.findPlug("overrideEndcapMaterial", &mStatus);
			if (mStatus)
			{
				MString name;
				endCapMaterialName.getValue(name);
				std::string searchName(name.asChar());
				pOverrideEndCapMaterialRes = pImporter->GetAftkScenePtr()->GetMaterialResourcePtrBySubString(searchName);
				if (!pOverrideEndCapMaterialRes)
				{
					AFTK_DO_BEGIN << TRANS_PGR3_MATERIAL_FIND_FAIL AFTK_DO_ERROR_END_1;
				}
			}
		}
	}

	//Override material proporties.
	if ((materialReplaceIndex != -1) && pOverrideMaterialRes)
	{
		pRemappedInstance->SetPolygonMaterialMapping(
			materialReplaceIndex, pOverrideMaterialRes);
	}

	//Override material proporties.
	if ((endCapMaterialReplaceIndex != -1) && pOverrideEndCapMaterialRes)
	{
		pRemappedInstance->SetPolygonMaterialMapping(
			endCapMaterialReplaceIndex, pOverrideEndCapMaterialRes);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR3GameImport::ImportReferenceInstance(
	C_Importer* pImporter,
	const C_ImportOptions* pOptions,
	const MDagPath& path)
{
	if (m_pathsProcessed.Find(path.fullPathName().asChar()) != m_pathsProcessed.End())
	{
		//We've already processed this path - abort.
		return Aftk::C_Status::E_Status_Success;
	}
	Aftk::C_Status status;
	MStatus mStatus;
	Aftk::C_Scene* pAftkScene = pImporter->GetAftkScenePtr();

	MFnDagNode fnTransform(path, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;

	MPlug normalInstanceParent = fnTransform.findPlug("normalInstanceParent", &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;

	MString attrString;
	normalInstanceParent.getValue(attrString);

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "normalInstanceParent String dyn attr. " << attrString.asChar() << "\n" AFTK_DO_DEBUG_END;
	#endif
	std::string toFind = attrString.asChar();
	MDagPath instanceSourceDagPath;
	if (!pImporter->GetAftkMayaMapping().GetMayaSceneNodeFromNameString(toFind, instanceSourceDagPath))
	{
		AFTK_DO_BEGIN << TRANS_DAG_NODE_FROM_DAG_STRING_FAIL << "[" << std::string(fnTransform.name().asChar()) << "]" << " Finding [" << toFind << "]" AFTK_DO_ERROR_RETURN_END_1;
	}

	//Note that for the moment, LODs of 'normal' instances are not supported.
	MFnDagNode fnInstanceParent(instanceSourceDagPath, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;
	Aftk::C_SceneMeshNode* pAftkOriginalNormalInstance = 0;
	if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(
		instanceSourceDagPath,
		(Aftk::C_SceneNode**)&pAftkOriginalNormalInstance))
	{
		AFTK_DO_BEGIN << TRANS_DAG_NODE_FROM_DAG_STRING_FAIL << "[" <<  std::string(fnTransform.name().asChar()) << "]" << " Finding [" << toFind << "]" AFTK_DO_ERROR_RETURN_END_1;
	}

	//Find the mesh to replace with an instance.
	Aftk::C_SceneNode* pAftkMeshToReplace = 0;
	if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(
		path,
		(Aftk::C_SceneNode**)&pAftkMeshToReplace))
	{
		AFTK_DO_BEGIN << TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL AFTK_DO_ERROR_RETURN_END_1;
	}

	//Don't process non-mesh nodes.
	if (pAftkMeshToReplace->GetType() != Aftk::E_ObjType_SceneMesh)
		return Aftk::C_Status::E_Status_Success;

	//Don't process corrupt or invalid nodes.
	if (pImporter->IsDebris(pAftkMeshToReplace))
		return Aftk::C_Status::E_Status_Success;

	//Create new remappabed mesh node
	Aftk::C_SceneMeshRemappedInstanceNode* pRemappedInstance = 
		new Aftk::C_SceneMeshRemappedInstanceNode(pImporter->GetAftkScenePtr());

	pRemappedInstance->SetName(
			pAftkOriginalNormalInstance->GetName() + std::string("_inst"));
	pRemappedInstance->SetLocalMatrix(pAftkMeshToReplace->GetLocalMatrix());
	pRemappedInstance->SetSourceNodePtr(pAftkOriginalNormalInstance);

	status = OverrideMaterials(
		pImporter,
		pOptions,
		instanceSourceDagPath, 
		path,
		pRemappedInstance);
	CHECK_ERROR_RETURN;

	//Remap the instance source pointers of any nodes that were instancing the node we're about to replace.
	{
		U32 numInstances = pAftkMeshToReplace->GetNumInstancesOfThisNode();
		for (U32 iInstance = 0; iInstance < numInstances; iInstance++)
		{
			Aftk::C_SceneNode* pNode = pAftkMeshToReplace->GetInstanceOfThisNode(0);
			if (pNode->GetType() == Aftk::E_ObjType_SceneInstance)
			{
				Aftk::C_SceneInstanceNode* pInstance = (Aftk::C_SceneInstanceNode*)pNode;
				pInstance->SetSourceNodePtr(pAftkOriginalNormalInstance);
			}
			if (pNode->GetType() == Aftk::E_ObjType_SceneMeshRemappedInstance)
			{
				Aftk::C_SceneMeshRemappedInstanceNode* pInstance = (Aftk::C_SceneMeshRemappedInstanceNode*)pNode;
				pInstance->SetSourceNodePtr(pAftkOriginalNormalInstance);
			}
		}
	}
	//Add the processed node path to our list of paths that we've processed. This is to avoid processing the
	//same path more than once.
	m_pathsProcessed.Insert(std::string(path.fullPathName().asChar()));
	pRemappedInstance->Replace(pAftkMeshToReplace);

	//Re-associate mappings with correct node.
	pImporter->GetAftkMayaMapping().SetDagPathStringMayaSceneNodeMapping(path.fullPathName().asChar(), path);
	pImporter->GetAftkMayaMapping().SetMayaAftkSceneNodeMapping(path, pRemappedInstance);
	pImporter->GetAftkMayaMapping().SetNameStringMayaSceneNodeMapping(pRemappedInstance->GetName(), path);
	pImporter->GetAftkMayaMapping().SetAftkMayaSceneNodeMapping(pRemappedInstance, path);
	return Aftk::C_Status::E_Status_Success;
}

bool C_PGR3GameImport::IsBonedInstanceSource(const MFnDagNode& fnTransform) const
{
	MStatus status;
	MPlug bonedInstanceParent = fnTransform.findPlug("barrierLods", &status);
	if (!status) 
		return false;
	return true;
}

bool C_PGR3GameImport::PreserveChildJointTransforms(
	C_Importer* pImporter,
	const MDagPath& path) const
{
	if (pImporter->GetImportOptionsPtr()->deformedInstancesAsMesh)
		return false;

	if (pImporter->IsBonedInstance(path) || IsBonedInstanceSource(path))
		return true;

	return false;
}

Aftk::C_Status C_PGR3GameImport::FindBones(
	C_Importer* pImporter,
	bool findingInstanceParentBones,
	const MDagPath& bonedInstancePath,
	const MFnDagNode& fnBonedInstance, 
	Aftk::C_SceneNode** pBones)
{
	Aftk::C_Status status;
	MStatus mStatus;

#ifdef BUILD_CONFIG_DEBUG
	std::string fullPath = bonedInstancePath.fullPathName().asChar();
#endif

	//Find the child indices for our two bones.
	U32 childBoneCount = 0;
	U32 childBoneIndices[2];
	for (U32 i = 0; i < bonedInstancePath.childCount(); i++)
	{
		MObject childObj = bonedInstancePath.child(i, &mStatus);
		MCHECK_BEGIN << TRANS_ACQUIRE_OBJECT_FAIL MCHECK_ERROR_RETURN_END_1;
		MFnDagNode childNode(childObj, &mStatus);
		MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;
		MDagPath childPath;
		mStatus = childNode.getPath(childPath);
		MCHECK_BEGIN << TRANS_ACQUIRE_DAG_PATH_FAIL MCHECK_ERROR_RETURN_END_1;
		if (childPath.hasFn(MFn::kJoint))
		{
			if (childBoneCount > 1)
			{
				childBoneCount++;

				continue;
			}
			else if (childBoneCount > 2)
			{
				break;
			}
			else
			{
				childBoneIndices[childBoneCount] = i;
				childBoneCount++;
			}
		}

	}

	if (childBoneCount < 2)
	{
		AFTK_DO_BEGIN << TRANS_INSUFFICIENT_BONES_FOR_BONED_INSTANCE AFTK_DO_ERROR_RETURN_END_1;
	}
	else if (childBoneCount > 2)
	{
		AFTK_DO_BEGIN << TRANS_TOO_MANY_BONES_FOR_BONED_INSTANCE AFTK_DO_ERROR_RETURN_END_1;
	}

	//Grab pointers to our two bones and attach them to our boned instance.
	{
		for (U32 iBone = 0; iBone < 2; iBone++)
		{
			MObject mayaBoneObj = bonedInstancePath.child(childBoneIndices[iBone], &mStatus);
			MCHECK_BEGIN << TRANS_ACQUIRE_OBJECT_FAIL MCHECK_ERROR_RETURN_END_1;
			MFnDagNode mayaBoneNode(mayaBoneObj, &mStatus);
			MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;
			MDagPath bonePath;
			mStatus = mayaBoneNode.getPath(bonePath);
			MCHECK_BEGIN << TRANS_ACQUIRE_DAG_PATH_FAIL MCHECK_ERROR_RETURN_END_1;
			if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(bonePath, &pBones[iBone]))
			{
				AFTK_DO_BEGIN << TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL AFTK_DO_ERROR_RETURN_END_1;
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR3GameImport::ImportBones(
	C_Importer* pImporter,
	Aftk::C_SceneMeshBonedInstanceNode* pAftkBonedInstance,
	const MDagPath& bonedInstancePath,
	const MFnDagNode& fnBonedInstance,
	const Aftk::C_SceneNode* pAftkBonedInstanceParent,
	const MDagPath& bonedInstanceParentPath,
	const MFnDagNode& fnBonedInstanceParent)
{
	Aftk::C_Status status;
	MStatus mStatus;

	Aftk::C_SceneNode* bones[2];
	status = FindBones(
		pImporter,
		false,
		bonedInstancePath,
		fnBonedInstance,
		bones);
	AFTK_CHECK_ERROR_RETURN;

	//Now that we've got the boned instance parent, find the parents bones.
	//Find the child indices for our two bones.
	Aftk::C_SceneNode* pParentBones[2];
	status = FindBones(
		pImporter,
		true,
		bonedInstanceParentPath,
		fnBonedInstanceParent,
		pParentBones);
	AFTK_CHECK_ERROR_RETURN;

	//The base pose is the transformation that takes us from the bone's original position to its deformed
	//position. During animation we use the inverse base pose to transform our vertices to the correct 
	//pivot point. We then multiply by the current world matrix of the bone to correctly position the
	//vertices.

	//Calculate the base pose for our bones.
	for (U32 iBone = 0; iBone < 2; iBone++)
	{
		Aftk::Matrix44 parentBoneWorldMatrix = pParentBones[iBone]->GetWorldMatrix();
		bones[iBone]->SetBasePose(parentBoneWorldMatrix);
	}

	pAftkBonedInstance->SetBonePtr(0, bones[0]);
	pAftkBonedInstance->SetBonePtr(1, bones[1]);

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR3GameImport::ImportBonedInstance(
	C_Importer* pImporter,
	const C_ImportOptions* pOptions,
	const MDagPath& path)
{
	Aftk::C_Status status;
	MStatus mStatus;
	Aftk::C_Scene* pAftkScene = pImporter->GetAftkScenePtr();

	MFnDagNode fnTransform(path, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;

	MPlug bonedInstanceParent = fnTransform.findPlug("bonedInstanceParent", &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;

	{
		MString attrString;
		bonedInstanceParent.getValue(attrString);

		#ifdef _DEBUG
		AFTK_DO_BEGIN << "bonedInstanceParent String dyn attr. " << attrString.asChar() << "\n" AFTK_DO_DEBUG_END;
		#endif
		std::string toFind = attrString.asChar();
		MDagPath bonedInstanceParentDagPath;
		if (!pImporter->GetAftkMayaMapping().GetMayaSceneNodeFromNameString(toFind, bonedInstanceParentDagPath))
		{
			AFTK_DO_BEGIN << TRANS_FIND_BONED_INSTANCE_PARENT_FAIL AFTK_DO_ERROR_RETURN_END_1;
		}
		MFnDagNode fnBonedInstanceParent(bonedInstanceParentDagPath, &mStatus);
		MCHECK_ERROR_RETURN_1;

		//Acquire our boned instance parent.
		Aftk::C_SceneNode* pAftkBonedInstanceParent;
		if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(bonedInstanceParentDagPath, (Aftk::C_SceneNode**)&pAftkBonedInstanceParent))
		{
			AFTK_DO_BEGIN << "Failed to find boned instance parent." AFTK_DO_ERROR_RETURN_END_1;
		}

		//Our original boned instance - we don't yet know which Lod this is from.
		Aftk::C_SceneMeshBonedInstanceNode* pAftkOriginalBonedInstanceNode;
		if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(path, (Aftk::C_SceneNode**)&pAftkOriginalBonedInstanceNode))
		{
			AFTK_DO_BEGIN << "Failed to find boned instance." AFTK_DO_ERROR_RETURN_END_1;
		}

		status = ImportBones(
			pImporter,
			pAftkOriginalBonedInstanceNode,
			path, 
			fnTransform,
			pAftkBonedInstanceParent,
			bonedInstanceParentDagPath,
			fnBonedInstanceParent);
		AFTK_CHECK_ERROR_RETURN;

		Aftk::C_SceneLodGroupNode* pAftkLodGroupNode;
		status = GenerateBonedInstanceLodGroup(
			pImporter, 
			pOptions,
			bonedInstanceParentDagPath,
			path,
			&pAftkLodGroupNode);
		AFTK_CHECK_ERROR_RETURN;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR3GameImport::MergeBonedInstanceLodMeshes(
	C_Importer* pImporter,
	const C_ImportOptions* pOptions)
{
	//Delete the lod meshes that we've used.
	Ut::UtSet< Aftk::C_SceneNode* >::Iterator it;
	//Pass 1 - delete instances
	for (it = m_lodLevelsToDelete.Begin(); it != m_lodLevelsToDelete.End(); it++)
	{
		if (it->GetKey()->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			//Delete an original lod level mesh instance. This function will preserve the hierarchy.
			pImporter->GetAftkScenePtr()->DeleteSceneNodeAndReparentChildren(*it, true);
		}
	}
	//Pass 2 - delete meshes
	for (it = m_lodLevelsToDelete.Begin(); it != m_lodLevelsToDelete.End(); it++)
	{
		if (it->GetKey()->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			if (it->GetKey()->GetNumInstancesOfThisNode() != 0)
			{
				Aftk::C_SceneNode* pThisNode = (*it);
				if (pThisNode->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
				{
					continue;
				}
				//We cannot delete a lod level mesh if there are any instances of it. 
				//Lod level meshes should not be instanced.
				AFTK_DO_BEGIN << "The lod level [" << pThisNode->GetName() << "] is instanced by the following nodes." AFTK_DO_INFO_END;
				for (U32 iNode = 0; iNode < pThisNode->GetNumInstancesOfThisNode(); iNode++)
				{
					Aftk::C_SceneNode* pInstance = pThisNode->GetInstanceOfThisNode(iNode);
					if (pInstance)
					{
						AFTK_DO_BEGIN << "[" << pInstance->GetName() << "] type [" << pInstance->GetType() << "]" AFTK_DO_INFO_END;
					}
					else
					{
						AFTK_DO_BEGIN << "NULL NODE" AFTK_DO_INFO_END;
					}
				}
				AFTK_DO_BEGIN << TRANS_LOD_INSTANCED_ERROR AFTK_DO_ERROR_RETURN_END_1;
			}

			//Delete an original lod level mesh. This function will preserve the hierarchy.
			pImporter->GetAftkScenePtr()->DeleteSceneNodeAndReparentChildren(*it, true);
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR3GameImport::GenerateBonedInstanceLodGroup(
	C_Importer* pImporter, 
	const C_ImportOptions* pOptions,
	const MDagPath& instanceSourceDagPath,
	const MDagPath& instanceDagPath,
	Aftk::C_SceneLodGroupNode** ppAftkLodGroupNode)
{
	Aftk::C_Status status;
	MStatus mStatus;
	//If this boned instance references a barrier, then it have an array attribute present which will 
	//provide us with the paths to the higher levels of detail of that model.

	MFnDagNode fnInstanceParent(instanceSourceDagPath, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;
	Aftk::C_SceneMeshNode* pAftkOriginalBonedInstanceLod = 0;
	if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(
		instanceSourceDagPath,
		(Aftk::C_SceneNode**)&pAftkOriginalBonedInstanceLod))
	{
		AFTK_DO_BEGIN << TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL AFTK_DO_ERROR_RETURN_END_1;
	}

	MPlug lodPlugArray = fnInstanceParent.findPlug("barrierLods", &mStatus);
	if (!mStatus)
	{
		AFTK_DO_BEGIN << TRANS_ACQUIRE_PLUG_FAIL AFTK_DO_ERROR_RETURN_END_1;
	}

	if (!lodPlugArray.isArray())
	{
		AFTK_DO_BEGIN << "barrierLods plug should be an array type." AFTK_DO_ERROR_RETURN_END_1;
		AFTK_ASSERT(lodPlugArray.isArray());
	}

	//Our original boned instance - we don't yet know which Lod this is from.
	Aftk::C_SceneMeshBonedInstanceNode* pAftkOriginalBonedInstanceNode;
	if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(instanceDagPath, (Aftk::C_SceneNode**)&pAftkOriginalBonedInstanceNode))
	{
		AFTK_DO_BEGIN << "Failed to find boned instance." AFTK_DO_ERROR_RETURN_END_1;
	}

	//Create new Lod GROUP node.
	Aftk::C_SceneLodGroupNode* pAftkLodGroupNode;
	{
		pAftkLodGroupNode = new Aftk::C_SceneLodGroupNode(pImporter->GetAftkScenePtr());
		pAftkLodGroupNode->SetName(
			pAftkOriginalBonedInstanceNode->GetName() + std::string("_barrierLodGroup"));
		//Add our new group node as a child of the original boned instance parent. Note that this is
		//different to the mesh lod approach.
		if (!pAftkOriginalBonedInstanceNode->GetParentPtr())
		{
			AFTK_DO_BEGIN << "Boned instance parent node has no parent." AFTK_DO_ERROR_RETURN_END_1;
		}
		pAftkOriginalBonedInstanceNode->GetParentPtr()->AddChildPtr(pAftkLodGroupNode);

		for (U32 iLayer = 0; iLayer < pAftkOriginalBonedInstanceNode->GetNumLayers(); iLayer++)
		{
			pAftkOriginalBonedInstanceNode->GetLayerPtr(iLayer)->AddMember(pAftkLodGroupNode);
		}

		#ifdef _DEBUG
		AFTK_DO_BEGIN << "*** Building lod levels for Lod group [" 
			<< pAftkLodGroupNode->GetName() << "]. ***\n" AFTK_DO_DEBUG_END;
		#endif
	}

	//Now we need to loop through all of the levels of detail, creating boned instances for each Lod
	//level, excluding the boned instance that we've already created however.
	U32 nLodLevels = lodPlugArray.numElements();
	for (U32 iLodLevel = 0; iLodLevel < nLodLevels; iLodLevel++)
	{
		S32 logicalElementIndex = (nLodLevels-1)-iLodLevel;
		AFTK_ASSERT(logicalElementIndex >= 0);
		MPlug lodPlug = lodPlugArray.elementByLogicalIndex(logicalElementIndex, &mStatus);
		MCHECK_BEGIN << TRANS_QUERY_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;

		MString mLodPathString;
		mStatus = lodPlug.getValue(mLodPathString);
		MCHECK_BEGIN << TRANS_QUERY_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;

		#ifdef _DEBUG
		AFTK_DO_BEGIN << "Barrier Lod path string dyn attr. " << mLodPathString.asChar() << "\n" AFTK_DO_DEBUG_END;
		#endif
		
		std::string lodPathString = mLodPathString.asChar();
		
		Aftk::C_SceneMeshNode* pAftkLodLevelMesh = 0;
		MDagPath lodPath;
		if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromDagPathString(lodPathString, (Aftk::C_SceneNode**)&pAftkLodLevelMesh))
		{
			Tu::SearchAndReplace(lodPathString, "|", "|barrierModels:");
			if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromDagPathString(lodPathString, (Aftk::C_SceneNode**)&pAftkLodLevelMesh))
			{
				AFTK_DO_BEGIN << "Failed to acquire barrier lod level [" << iLodLevel << "]." AFTK_DO_ERROR_RETURN_END_1;
			}
		}

		U32 insertionPoint = pAftkLodGroupNode->GetNumLodLevels();

		//Set the name and threshold for this lod level
		Aftk::C_GroupLodLevelInfo info;
		if (iLodLevel == lodPlugArray.numElements()-1)
		{
			info.SetThreshold(0);
		}
		else
		{
			info.SetThreshold(PGR3_THRESHOLD_INCREMENT*(iLodLevel+1));
		}

		std::string instanceSourceDagString = instanceSourceDagPath.fullPathName().asChar();
		if (instanceSourceDagString == lodPathString)
		{
			//Use our original boned instance for the current lod level.
			info.SetSceneNodePtr(pAftkOriginalBonedInstanceNode);

			//Add original lod to the group.
			pImporter->GetAftkScenePtr()->ExtractSceneNodeAndChildren(pAftkOriginalBonedInstanceNode);
			pAftkLodGroupNode->AddChildPtr(pAftkOriginalBonedInstanceNode);

			//Override materials for each lod.
			status = OverrideMaterials(
				pImporter,
				pOptions,
				instanceSourceDagPath, 
				instanceDagPath,
				pAftkOriginalBonedInstanceNode);
			AFTK_CHECK_ERROR_RETURN;
		}
		else
		{
			//Create new boned instance.
			Aftk::C_SceneMeshBonedInstanceNode* pNewBonedInstance = new Aftk::C_SceneMeshBonedInstanceNode(
				pImporter->GetAftkScenePtr());

			status = ImportLodLevel(
				pImporter->GetAftkScenePtr(),
				pNewBonedInstance,
				pAftkOriginalBonedInstanceNode,
				pAftkLodLevelMesh);
			AFTK_CHECK_ERROR_RETURN;

			pAftkLodGroupNode->AddChildPtr(pNewBonedInstance);
			info.SetSceneNodePtr(pNewBonedInstance);

			//Override materials for each lod.
			status = OverrideMaterials(
				pImporter,
				pOptions,
				instanceSourceDagPath, 
				instanceDagPath,
				pNewBonedInstance);
			AFTK_CHECK_ERROR_RETURN;
		}

		//Insert after existing lod levels.
		pAftkLodGroupNode->InsertLodLevelInfo(insertionPoint, info);
		if (pAftkLodGroupNode->GetNumLayers() == 0)
		{
			//Add the lod group to the first layer we encounter during lod level insertion.
			if (info.GetSceneNodePtr())
				for (U32 iLayer = 0; iLayer < info.GetSceneNodePtr()->GetNumLayers(); iLayer++)
					info.GetSceneNodePtr()->GetLayerPtr(iLayer)->AddMember(pAftkLodGroupNode);
		}
	}

	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_PGR3GameImport::GenerateBonedInstanceLodMesh(
	C_Importer* pImporter, 
	const C_ImportOptions* pOptions,
	const MDagPath& instanceSourceDagPath,
	const MDagPath& instanceDagPath,
	Aftk::C_SceneMeshNode** ppAftkLodGroupMeshNode)
{
	Aftk::C_Status status;
	MStatus mStatus;
	//If this boned instance references a barrier, then it have an array attribute present which will 
	//provide us with the paths to the higher levels of detail of that model.

	MFnDagNode fnInstanceParent(instanceSourceDagPath, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;
	Aftk::C_SceneMeshNode* pAftkBonedInstanceParentNode = 0;
	if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(
		instanceSourceDagPath,
		(Aftk::C_SceneNode**)&pAftkBonedInstanceParentNode))
	{
		AFTK_DO_BEGIN << TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL AFTK_DO_ERROR_RETURN_END_1;
	}

	//Abort if we've already created the lods for this node previously.
	{
		Ut::UtSet< Aftk::C_SceneMeshNode* >::Iterator it;
		it = m_lodGroupsProcessed.Find(pAftkBonedInstanceParentNode);
		if (it != m_lodGroupsProcessed.End())
		{
			//We've already acquired the Lod levels for lod group.
			Ut::UtMap< Aftk::C_SceneMeshNode*, Aftk::C_SceneMeshNode*>::Iterator mapIt;
			mapIt = m_lodGroupMapping.Find(pAftkBonedInstanceParentNode);
			if (mapIt == m_lodGroupMapping.End())
			{
				AFTK_DO_BEGIN << "Failed to find processed lod group." AFTK_DO_ERROR_RETURN_END_1;
			}
			else
			{
				*ppAftkLodGroupMeshNode = mapIt->GetValue();
			}
		}
	}

	if (!pAftkBonedInstanceParentNode)
	{
		AFTK_DO_BEGIN << "Failed to acquire boned instance parent." AFTK_DO_ERROR_RETURN_END_1;
	}

	MPlug lodPlugArray = fnInstanceParent.findPlug("barrierLods", &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;
	if (!lodPlugArray.isArray())
	{
		AFTK_CHECK_BEGIN << "barrierLods plug should be an array type." AFTK_DO_ERROR_RETURN_END_1;
		AFTK_ASSERT(lodPlugArray.isArray());
	}
	
	//Loop thru lod level dag paths

	//Create new Lod group MESH
	Aftk::C_SceneMeshNode* pAftkLodGroupMeshNode;
	{
		pAftkLodGroupMeshNode = new Aftk::C_SceneMeshNode(pImporter->GetAftkScenePtr());
		pAftkLodGroupMeshNode->SetName(
			pAftkBonedInstanceParentNode->GetName() + std::string("_barrierLodGroup"));
		pAftkLodGroupMeshNode->SetLocalMatrix(
			pAftkBonedInstanceParentNode->GetLocalMatrix());
		//Add our new group node as a child of the instance parent's parent.
		if (!pAftkBonedInstanceParentNode->GetParentPtr())
		{
			AFTK_DO_BEGIN << "Boned instance parent node has no parent." AFTK_DO_ERROR_RETURN_END_1;
		}
		pAftkBonedInstanceParentNode->GetParentPtr()->AddChildPtr(pAftkLodGroupMeshNode);

		//Override materials for master lod level.
		status = OverrideMaterials(
			pImporter,
			pOptions,
			instanceSourceDagPath,
			instanceDagPath,
			pAftkLodGroupMeshNode);
		AFTK_CHECK_ERROR_RETURN;

		#ifdef _DEBUG
		AFTK_DO_BEGIN << "*** Building lod levels for Lod group [" 
			<< pAftkLodGroupMeshNode->GetName() << "]. ***\n" AFTK_DO_DEBUG_END;
		#endif
	}

	U32 nLodLevels = lodPlugArray.numElements();
	for (U32 iLodLevel = 0; iLodLevel < nLodLevels; iLodLevel++)
	{
		S32 logicalElementIndex = (nLodLevels-1)-iLodLevel;
		AFTK_ASSERT(logicalElementIndex >= 0);
		MPlug lodPlug = lodPlugArray.elementByLogicalIndex(logicalElementIndex, &mStatus);
		MCHECK_BEGIN << TRANS_QUERY_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;

		MString lodPathString;
		mStatus = lodPlug.getValue(lodPathString);
		MCHECK_BEGIN << TRANS_QUERY_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;

		#ifdef _DEBUG
		AFTK_DO_BEGIN << "Barrier Lod path string dyn attr. " << lodPathString.asChar() << "\n" AFTK_DO_DEBUG_END;
		#endif

		std::string toFind = lodPathString.asChar();
		Tu::SearchAndReplace(toFind, "|", "|barrierModels:");
		
		Aftk::C_SceneMeshNode* pAftkLodLevelMesh = 0;
		MDagPath lodPath;
		if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromDagPathString(toFind, (Aftk::C_SceneNode**)&pAftkLodLevelMesh))
		{
			AFTK_DO_BEGIN << "Failed to acquire barrier lod level [" << iLodLevel << "]." AFTK_DO_ERROR_RETURN_END_1;
		}

		//Set the name and threshold for this lod level
		Aftk::C_MeshLodLevelInfo info;
		info.SetName(pAftkLodLevelMesh->GetName());
		if (iLodLevel == lodPlugArray.numElements()-1)
		{
			info.SetThreshold(0);
		}
		else
		{
			info.SetThreshold(PGR3_THRESHOLD_INCREMENT*(iLodLevel+1));
		}

		U32 insertionPoint = pAftkLodGroupMeshNode->GetNumLodLevels();
		//Insert after existing lod levels.
		pAftkLodGroupMeshNode->SetLodLevelInfo(insertionPoint, info);

		//We now add this mesh lod level to our lod mesh.
		status = pAftkLodGroupMeshNode->SetLodLevelMesh(
			insertionPoint, pAftkLodLevelMesh, true);
		AFTK_CHECK_BEGIN << "Aftk reported an error during Lod level creation." AFTK_CHECK_ERROR_RETURN_END_1;

		m_lodLevelsToDelete.Insert(pAftkLodLevelMesh);
	}

#ifdef _DEBUG
	for (U32 iLodLevel = 0; iLodLevel < pAftkLodGroupMeshNode->GetNumLodLevels(); iLodLevel++)
	{
		AFTK_DO_BEGIN 
			<< pAftkLodGroupMeshNode->GetLodLevelInfo(iLodLevel).GetName() 
			<< " T = " << pAftkLodGroupMeshNode->GetLodLevelInfo(iLodLevel).GetThreshold() 
			<< " poly count " << pAftkLodGroupMeshNode->GetNumPolygons(iLodLevel) 
			<< "\n"
		AFTK_DO_DEBUG_END;
	}
#endif

	//Mark this Lod group as having been processed.
	m_lodGroupsProcessed.Insert(pAftkLodGroupMeshNode);

	//Add to lod group mapping
	m_lodGroupMapping[pAftkBonedInstanceParentNode] = pAftkLodGroupMeshNode;

	//Return new group node.
	*ppAftkLodGroupMeshNode = pAftkLodGroupMeshNode;

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR3GameImport::ImportLodLevel(
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const Aftk::C_SceneMeshBonedInstanceNode* pOriginalBonedInstance,
	Aftk::C_SceneMeshNode* pAftkLodLevelMesh)
{
	MStatus status;

	//Copy across proporties from our original boned instance.
	pAftkSceneNode->SetName(pAftkLodLevelMesh->GetName());
	pAftkSceneNode->SetLocalMatrix(pOriginalBonedInstance->GetLocalMatrix());
	for (U32 iLayer = 0; iLayer < pOriginalBonedInstance->GetNumLayers(); iLayer++) 
	{
		Aftk::C_LayerNode* pLayer = const_cast<Aftk::C_LayerNode*>(pOriginalBonedInstance->GetLayerPtr(iLayer));
		pLayer->AddMember(pAftkSceneNode);
	}
   
	Aftk::C_SceneMeshBonedInstanceNode* pAftkSceneMeshBonedInstanceNode = 
		(Aftk::C_SceneMeshBonedInstanceNode*)pAftkSceneNode;

	//These are the bones we are to use for the new boned instance. Simply use the bones from the existing
	//boned instance that we were given.
	{
		Aftk::C_SceneNode* pBone0 = const_cast<Aftk::C_SceneNode*>(pOriginalBonedInstance->GetBonePtr(0));
		Aftk::C_SceneNode* pBone1 = const_cast<Aftk::C_SceneNode*>(pOriginalBonedInstance->GetBonePtr(1));
		pAftkSceneMeshBonedInstanceNode->SetBonePtr(0, pBone0);
		pAftkSceneMeshBonedInstanceNode->SetBonePtr(1, pBone1);
	}

	//Find the bones for the boned instance geometry. In other words, the mesh node that is the source
	//for our new boned instance.
	{
		//pAftkLodLevelMesh is the aftk pointer to our source geometry.
		pAftkSceneMeshBonedInstanceNode->SetBasePose(
			pOriginalBonedInstance->GetBasePose());

		pAftkSceneMeshBonedInstanceNode->SetSourceNodePtr(pAftkLodLevelMesh);
	}

	return Aftk::C_Status::E_Status_Success;
}
