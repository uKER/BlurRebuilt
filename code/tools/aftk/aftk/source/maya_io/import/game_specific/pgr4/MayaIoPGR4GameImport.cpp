#include "stdafx.h"

#include "../MayaIoIGameImport.h"
#include "MayaIoPGR4GameImport.h"
//#pragma message("TODO:REMOVE OLD STRING PROC")
#include <core/utility/stringproc.h>
#include <maya/MFnStringArrayData.h>
#include "../../../MayaIoPolyFlagsHelper.h"

#define PGR4_THRESHOLD_INCREMENT 20.0f
#define TRANS_PGR4_TO_OVERRIDE_FIND_FAIL "Cannot find specified material to override."
#define TRANS_PGR4_MATERIAL_FIND_FAIL "Cannot find specified material [" << searchName << "]. Try looking for material in scene, and making sure its name has not got any extra text"

using namespace MayaIo;

#define MAYA_IO_NUM_BONES 2
#define MAYA_IO_EXPORT_BARRIER_LODS

C_PGR4GameImport::~C_PGR4GameImport()
{
}

Aftk::C_Status C_PGR4GameImport::PreProcessScene(
	C_Importer* pImporter,
	const C_ImportOptions* pOptions)
{
//	PolyFlagsHelper::Validate("z:\\tools\\art_tools\\config\\PolyFlags_gotham.txt");
	
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR4GameImport::PostProcessSceneStage1(
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
		status = PostProcessDagNodeStage1(
			pImporter,
			pOptions,
			transformDagPath,
			list);
		CHECK_ERROR_RETURN;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR4GameImport::PostProcessDagNodeStage1(
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
	if (exportCurrentNode && pImporter->IsBonedInstanceParent(transformDagPath))
	{
		MFnDagNode fnTransform(transformDagPath, &mStatus);
		MCHECK_ERROR_RETURN_1;		
		//Find the material to replace.
		MPlug originalMaterialName = fnTransform.findPlug("scratchMaterialToOverride", &mStatus);
		if (mStatus)
		{
			MString name;
			originalMaterialName.getValue(name);
			std::string searchName(name.asChar());
			searchName = "barrierModels:" + searchName;
			const Aftk::C_MaterialResource* pOriginalMaterialRes = 
				pImporter->GetAftkScenePtr()->GetMaterialResourcePtrByName(searchName);
			if( !pOriginalMaterialRes)
			{
				// material name without "barrierModels:" prefix
				pOriginalMaterialRes = pImporter->GetAftkScenePtr()->GetMaterialResourcePtrByName(searchName.substr(13));
			}
			if (pOriginalMaterialRes)
			{
				Aftk::C_SceneMeshNode* pAftkBaseInstance;
				if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(
					transformDagPath,
					(Aftk::C_SceneNode**)&pAftkBaseInstance))
				{
					AFTK_DO_BEGIN << TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL AFTK_DO_ERROR_RETURN_END_1;
				}

				S32 materialReplaceIndex = 
					pAftkBaseInstance->GetPolygonMaterialMappingIndex(pOriginalMaterialRes);
				if (materialReplaceIndex == -1)
				{
					//We couldn't find the expected scratch texture on our base instance, however
					//we did find the material in the scene. Under these conditions we should add
					//our scratch texture to our base instance.

					//Add our material for the sake of being able to replace it.
					pAftkBaseInstance->AddPolygonMaterialMapping(pOriginalMaterialRes);
				}
			}
			else
			{
				AFTK_DO_BEGIN << TRANS_PGR4_TO_OVERRIDE_FIND_FAIL AFTK_DO_ERROR_END_1;
			}
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

		status = PostProcessDagNodeStage1(
			pImporter,
			pOptions,
			childPath,
			list);
		CHECK_ERROR_RETURN;
	}

	return Aftk::C_Status::E_Status_Success;
}

bool C_PGR4GameImport::IsReferenceInstance(const MDagPath& path) const
{
	MStatus mStatus;
	MFnDagNode fnTransform(path, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;

	MPlug lodPlugArray = fnTransform.findPlug("normalInstanceParent", &mStatus);
	if (mStatus)
		return true;

	return false;
}

Aftk::C_Status C_PGR4GameImport::PostProcessScene(
	C_Importer* pImporter,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;

	status = PostProcessSceneStage1(pImporter, pOptions);
	CHECK_ERROR_RETURN;

	status = PostProcessSceneStage2(pImporter, pOptions);
	CHECK_ERROR_RETURN;

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR4GameImport::PostProcessSceneStage2(
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
		status = PostProcessDagNodeStage2(
			pImporter,
			pOptions,
			transformDagPath,
			list);
		CHECK_ERROR_RETURN;
	}


	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR4GameImport::PostProcessDagNodeStage2(
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

		status = PostProcessDagNodeStage2(
			pImporter,
			pOptions,
			childPath,
			list);
		CHECK_ERROR_RETURN;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR4GameImport::OverrideMaterials(	
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

	//Find the material to replace.
	MPlug originalMaterialName = fnInstanceSource.findPlug("materialToOverride", &mStatus);
	if (mStatus)
	{
		{
			MString name;
			originalMaterialName.getValue(name);
			std::string searchName(name.asChar());
			searchName = "barrierModels:" + searchName;
			const Aftk::C_MaterialResource* pOriginalMaterialRes = 
				pImporter->GetAftkScenePtr()->GetMaterialResourcePtrByName(searchName);
			if( !pOriginalMaterialRes)
			{
				// material name without "barrierModels:" prefix
				pOriginalMaterialRes = pImporter->GetAftkScenePtr()->GetMaterialResourcePtrByName(searchName.substr(13));
			}
			if (pOriginalMaterialRes)
			{
				materialReplaceIndex = 
					pRemappedInstance->GetPolygonMaterialMappingIndex(pOriginalMaterialRes);
			}
			if (!pOriginalMaterialRes || materialReplaceIndex == -1)
			{
				AFTK_DO_BEGIN << TRANS_PGR4_TO_OVERRIDE_FIND_FAIL AFTK_DO_ERROR_END_1;
			}
		}
		{
			MPlug materialName = fnInstance.findPlug("overrideMaterial", &mStatus);
			if (mStatus)
			{
				MString name;
				materialName.getValue(name);
				std::string searchName(name.asChar());
				pOverrideMaterialRes = pImporter->GetAftkScenePtr()->GetMaterialResourcePtrByName(searchName);
	
				if (!pOverrideMaterialRes)
				{
					AFTK_DO_BEGIN << TRANS_PGR4_MATERIAL_FIND_FAIL AFTK_DO_ERROR_END_1;
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

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR4GameImport::OverrideScratchMaterials(	
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

	//Find the material to replace.
	MPlug originalMaterialName = fnInstanceSource.findPlug("scratchMaterialToOverride", &mStatus);
	if (mStatus)
	{
		{
			MString name;
			originalMaterialName.getValue(name);
			std::string searchName(name.asChar());
			searchName = "barrierModels:" + searchName;
			const Aftk::C_MaterialResource* pOriginalMaterialRes = 
				pImporter->GetAftkScenePtr()->GetMaterialResourcePtrByName(searchName);
			if( !pOriginalMaterialRes)
			{
				// material name without "barrierModels:" prefix
				pOriginalMaterialRes = pImporter->GetAftkScenePtr()->GetMaterialResourcePtrByName(searchName.substr(13));
			}
			if (pOriginalMaterialRes)
			{
				materialReplaceIndex = 
					pRemappedInstance->GetPolygonMaterialMappingIndex(pOriginalMaterialRes);
			}
			if (!pOriginalMaterialRes || materialReplaceIndex == -1)
			{
				AFTK_DO_BEGIN << TRANS_PGR4_TO_OVERRIDE_FIND_FAIL AFTK_DO_ERROR_END_1;
			}
		}
		{
			MPlug materialName = fnInstance.findPlug("scratchOverrideMaterial", &mStatus);
			if (mStatus)
			{
				MString name;
				materialName.getValue(name);
				std::string searchName(name.asChar());
				pOverrideMaterialRes = pImporter->GetAftkScenePtr()->GetMaterialResourcePtrByName(searchName);
	
				if (!pOverrideMaterialRes)
				{
					AFTK_DO_BEGIN << TRANS_PGR4_MATERIAL_FIND_FAIL AFTK_DO_ERROR_END_1;
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

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR4GameImport::ImportReferenceInstance(
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
	MDagPath instanceParentDagPath;
	if (!pImporter->GetAftkMayaMapping().GetMayaSceneNodeFromNameString(toFind, instanceParentDagPath))
	{
		AFTK_DO_BEGIN << TRANS_DAG_NODE_FROM_DAG_STRING_FAIL AFTK_DO_ERROR_RETURN_END_1;
	}

	//Note that for the moment, LODs of 'normal' instances are not supported.
	MFnDagNode fnInstanceParent(instanceParentDagPath, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;
	Aftk::C_SceneMeshNode* pAftkOriginalNormalInstance = 0;
	if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(
		instanceParentDagPath,
		(Aftk::C_SceneNode**)&pAftkOriginalNormalInstance))
	{
		AFTK_DO_BEGIN << TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL AFTK_DO_ERROR_RETURN_END_1;
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
		instanceParentDagPath, 
		path,
		pRemappedInstance);
	AFTK_CHECK_ERROR_RETURN;

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

	//Delete old mapping table entry.
	pImporter->GetAftkMayaMapping().DeleteAftkMayaSceneNodeMapping(pAftkMeshToReplace);

	pRemappedInstance->Replace(pAftkMeshToReplace);

	//Re-associate mappings with correct node.
	pImporter->GetAftkMayaMapping().SetDagPathStringMayaSceneNodeMapping(path.fullPathName().asChar(), path);
	pImporter->GetAftkMayaMapping().SetMayaAftkSceneNodeMapping(path, pRemappedInstance);
	pImporter->GetAftkMayaMapping().SetNameStringMayaSceneNodeMapping(pRemappedInstance->GetName(), path);
	pImporter->GetAftkMayaMapping().SetAftkMayaSceneNodeMapping(pRemappedInstance, path);

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR4GameImport::ImportBonedInstance(
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

	Aftk::C_SceneNode* bones[MAYA_IO_NUM_BONES];
	status = FindBones(
		pImporter,
		false,
		path,
		bones);
	AFTK_CHECK_ERROR_RETURN;

	MFnDagNode fnInstanceParent(bonedInstanceParentDagPath, &mStatus);
	MPlug lodPlugArray = fnInstanceParent.findPlug("barrierLods", &mStatus);
	//If we've got lod levels...
	if (mStatus)
	{
#ifdef MAYA_IO_EXPORT_BARRIER_LODS
		//Generate barrier lod group, creating additional boned instance nodes
		//for each lod level. All of the boned instances are then grouped under a new lod level.
		Aftk::C_SceneLodGroupNode* pAftkLodGroupNode;
		status = GenerateBonedInstanceLodGroup(
			pImporter,
			pOptions,
			bonedInstanceParentDagPath,
			path,
			bones,
			pAftkOriginalBonedInstanceNode,
			&pAftkLodGroupNode);
		AFTK_CHECK_ERROR_RETURN;
#else
		status = ReplaceBonedInstanceWithHighRes(
			pImporter,
			bones,
			pOptions,
			bonedInstanceParentDagPath,
			path,
			pAftkOriginalBonedInstanceNode);
		AFTK_CHECK_ERROR_RETURN;
#endif
	}
	else
	{
		status = ImportBonesAndCloneIfNeeded(
			pImporter,
			bones,
			pAftkOriginalBonedInstanceNode,
			path,
			pAftkBonedInstanceParent,
			bonedInstanceParentDagPath);
		AFTK_CHECK_ERROR_RETURN;

		status = OverrideMaterials(
			pImporter,
			pOptions,
			bonedInstanceParentDagPath, 
			path,
			pAftkOriginalBonedInstanceNode);
		AFTK_CHECK_ERROR_RETURN;

		status = OverrideScratchMaterials(
			pImporter,
			pOptions,
			bonedInstanceParentDagPath, 
			path,
			pAftkOriginalBonedInstanceNode);
		AFTK_CHECK_ERROR_RETURN;
	}

	//Set the base pose for our boned instance.
	pAftkOriginalBonedInstanceNode->SetBasePose(pAftkBonedInstanceParent->GetWorldMatrix());

	return Aftk::C_Status::E_Status_Success;
}

bool C_PGR4GameImport::PreserveChildJointTransforms(
	C_Importer* pImporter,
	const MDagPath& path) const
{
	MFnDagNode node(path);
	if (std::string(node.name().asChar()).find("barrierJoints") == std::string::npos)
		return false;

	return true;
}

Aftk::C_Status C_PGR4GameImport::FindBones(
	C_Importer* pImporter,
	bool findingInstanceParentBones,
	const MDagPath& bonedInstancePath,
	Aftk::C_SceneNode** pBones)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MFnDagNode fnBonedInstance(bonedInstancePath, &mStatus);
	MCHECK_ERROR_RETURN_1;

	MPlug instanceBone0 = fnBonedInstance.findPlug("instanceBone0", &mStatus);
	MCHECK_BEGIN << "Failed to find boned instance bone0 for " << fnBonedInstance.name().asChar() MCHECK_ERROR_RETURN_END_1;

	MPlug instanceBone1 = fnBonedInstance.findPlug("instanceBone1", &mStatus);
	MCHECK_BEGIN << "Failed to find boned instance bone1 for " << fnBonedInstance.name().asChar() MCHECK_ERROR_RETURN_END_1;

	{
		std::string pathString;
		{
			MString tmp;
			mStatus = instanceBone0.getValue(tmp);
			MCHECK_BEGIN << TRANS_QUERY_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;
			pathString = tmp.asChar();
			if (findingInstanceParentBones)
				Tu::SearchAndReplace(pathString, "|", "|barrierModels:");
		}
		if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromDagPathString(pathString, &pBones[0]))
		{
			AFTK_DO_BEGIN << "Failed to find boned instance [" << pathString << "] [" << pathString << "] joint0." AFTK_DO_ERROR_RETURN_END_1;
		}
		AFTK_ASSERT(pBones[0]);

		{
			MString tmp;
			mStatus = instanceBone1.getValue(tmp);
			MCHECK_BEGIN << TRANS_QUERY_PLUG_FAIL MCHECK_ERROR_RETURN_END_1;
			pathString = tmp.asChar();
			if (findingInstanceParentBones)
				Tu::SearchAndReplace(pathString, "|", "|barrierModels:");
		}
		if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromDagPathString(pathString, &pBones[1]))
		{
			AFTK_DO_BEGIN << "Failed to find boned instance [" << pathString << "] [" << pathString << "] joint1." AFTK_DO_ERROR_RETURN_END_1;
		}
		AFTK_ASSERT(pBones[1]);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR4GameImport::ImportBonesAndCloneIfNeeded(
	C_Importer* pImporter,
	Aftk::C_SceneNode** bones,
	Aftk::C_SceneMeshBonedInstanceNode* pAftkBonedInstance,
	const MDagPath& bonedInstancePath,
	const Aftk::C_SceneNode* pAftkBonedInstanceParent,
	const MDagPath& bonedInstanceParentPath)
{
	Aftk::C_Status status;
	MStatus mStatus;

	//Clone bones as required. We can't allow bones to be used across multiple boned instances
	//as unique base poses would be required, unless the parent bones of all boned instances
	//had the same transform.
	for (U32 i = 0; i < MAYA_IO_NUM_BONES; i++)
	{
		AFTK_ASSERT(bones[i]);
		if (bones[i]->GetNumInstancesOfThisNode() > 0)
		{
			Aftk::C_SceneNode* pCloneBone = 
				(Aftk::C_SceneNode*)bones[i]->Clone(bones[i]->GetScenePtr());
			//Add cloned bone as a sibling of our original bone.
			bones[i]->GetParentPtr()->AddChildPtr(pCloneBone);
			//Use our cloned bone instead of our original.
			bones[i] = pCloneBone;
		}
	}

	//Now that we've got the boned instance parent, find the parents bones.
	//Find the child indices for our two bones.
	Aftk::C_SceneNode* pParentBones[MAYA_IO_NUM_BONES];
	status = FindBones(
		pImporter,
		true,
		bonedInstanceParentPath,
		pParentBones);
	AFTK_CHECK_ERROR_RETURN;

	//The base pose is the transformation that takes us from the bone's original position to its deformed
	//position. During animation we use the inverse base pose to transform our vertices to the correct 
	//pivot point. We then multiply by the current world matrix of the bone to correctly position the
	//vertices.

	//Calculate the base pose for our bones.
	for (U32 iBone = 0; iBone < MAYA_IO_NUM_BONES; iBone++)
	{
		bones[iBone]->SetBasePose(pParentBones[iBone]->GetWorldMatrix());
		pAftkBonedInstance->SetBonePtr(iBone, bones[iBone]);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR4GameImport::ReplaceBonedInstanceWithHighRes(
	C_Importer* pImporter,
	Aftk::C_SceneNode** bones,
	const C_ImportOptions* pOptions,
	const MDagPath& instanceParentDagPath,
	const MDagPath& instanceDagPath,
	Aftk::C_SceneMeshBonedInstanceNode* pAftkOriginalBonedInstanceNode)
{
	Aftk::C_Status status;
	MStatus mStatus;
	//If this boned instance references a barrier, then it have an array attribute present which will 
	//provide us with the paths to the higher levels of detail of that model.

	MFnDagNode fnInstanceParent(instanceParentDagPath, &mStatus);
	MPlug lodPlugArray = fnInstanceParent.findPlug("barrierLods", &mStatus);
	if (!mStatus)
	{
		AFTK_DO_BEGIN << TRANS_ACQUIRE_PLUG_FAIL AFTK_DO_ERROR_RETURN_END_1;
	}
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;
	Aftk::C_SceneMeshNode* pAftkInstanceParent = 0;
	if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(
		instanceParentDagPath,
		(Aftk::C_SceneNode**)&pAftkInstanceParent))
	{
		AFTK_DO_BEGIN << TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL AFTK_DO_ERROR_RETURN_END_1;
	}

	//Now we need to loop through all of the levels of detail, creating boned instances for each Lod
	//level, excluding the boned instance that we've already created however.
	MObject obj;
	mStatus = lodPlugArray.getValue(obj);
	MCHECK_ERROR_RETURN_1;
	MFnStringArrayData lodArray(obj, &mStatus);
	MCHECK_ERROR_RETURN_1;

	for (U32 iLodLevel = 0; iLodLevel < lodArray.length(); iLodLevel++)
	{
		MString mLodPathString = lodArray[iLodLevel];

		#ifdef _DEBUG
		AFTK_DO_BEGIN << "Barrier Lod path string dyn attr. " << mLodPathString.asChar() << "\n" AFTK_DO_DEBUG_END;
		#endif
		
		std::string lodPathString = mLodPathString.asChar();
		Tu::SearchAndReplace(lodPathString, "|", "|barrierModels:");

		Aftk::C_SceneMeshNode* pAftkLodLevelMesh = 0;

		if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromDagPathString(lodPathString, (Aftk::C_SceneNode**)&pAftkLodLevelMesh))
			AFTK_DO_BEGIN << "Failed to acquire barrier lod level [" << iLodLevel << "]. GetAftkSceneNodeFromDagPathString failed to find Aftk mesh from Maya path [" << lodPathString << "] on mesh [" << pAftkOriginalBonedInstanceNode->GetName() << "]" AFTK_DO_ERROR_RETURN_END_1;

		MDagPath lodInstanceParent;
		if (!pImporter->GetAftkMayaMapping().GetMayaDagPathFromAftkSceneNode(pAftkLodLevelMesh, &lodInstanceParent))
			AFTK_DO_BEGIN << "Failed to acquire barrier lod level [" << iLodLevel << "]. GetMayaDagPathFromAftkSceneNode failed to find Maya path for Aftk scene node [" << pAftkLodLevelMesh->GetName() << "] on mesh [" << pAftkOriginalBonedInstanceNode->GetName() << "]" AFTK_DO_ERROR_RETURN_END_1;

		std::string instanceSourceDagString = instanceParentDagPath.fullPathName().asChar();
		if (iLodLevel == lodArray.length()-1 && instanceSourceDagString != lodPathString)
		{
			//Let's use our high-res version.
			pAftkOriginalBonedInstanceNode->SetName(pAftkLodLevelMesh->GetName());
			pAftkOriginalBonedInstanceNode->SetSourceNodePtr(pAftkLodLevelMesh);

			status = ImportBonesAndCloneIfNeeded(
				pImporter,
				bones,
				pAftkOriginalBonedInstanceNode,
				instanceDagPath,
				pAftkInstanceParent,
				lodInstanceParent);
			AFTK_CHECK_ERROR_RETURN;

			//Override materials for each lod.
			status = OverrideMaterials(
				pImporter,
				pOptions,
				lodInstanceParent, 
				instanceDagPath,
				pAftkOriginalBonedInstanceNode);
			AFTK_CHECK_ERROR_RETURN;

			status = OverrideScratchMaterials(
				pImporter,
				pOptions,
				instanceParentDagPath, 
				instanceDagPath,
				pAftkOriginalBonedInstanceNode);
			AFTK_CHECK_ERROR_RETURN;
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_PGR4GameImport::GenerateBonedInstanceLodGroup(
	C_Importer* pImporter,
	const C_ImportOptions* pOptions,
	const MDagPath& instanceParentDagPath,
	const MDagPath& instanceDagPath,
	Aftk::C_SceneNode** bones,
	Aftk::C_SceneMeshBonedInstanceNode* pAftkOriginalBonedInstanceNode,
	Aftk::C_SceneLodGroupNode** ppAftkLodGroupNode)
{
	Aftk::C_Status status;
	MStatus mStatus;
	//If this boned instance references a barrier, then it have an array attribute present which will 
	//provide us with the paths to the higher levels of detail of that model.

	MFnDagNode fnInstanceParent(instanceParentDagPath, &mStatus);
	MPlug lodPlugArray = fnInstanceParent.findPlug("barrierLods", &mStatus);
	if (!mStatus)
	{
		AFTK_DO_BEGIN << TRANS_ACQUIRE_PLUG_FAIL AFTK_DO_ERROR_RETURN_END_1;
	}
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;
	Aftk::C_SceneMeshNode* pAftkInstanceParent = 0;
	if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(
		instanceParentDagPath,
		(Aftk::C_SceneNode**)&pAftkInstanceParent))
	{
		AFTK_DO_BEGIN << TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL AFTK_DO_ERROR_RETURN_END_1;
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
	MObject obj;
	mStatus = lodPlugArray.getValue(obj);
	MCHECK_ERROR_RETURN_1;
	MFnStringArrayData lodArray(obj, &mStatus);
	MCHECK_ERROR_RETURN_1;

	for (U32 iLodLevel = 0; iLodLevel < lodArray.length(); iLodLevel++)
	{
		MString mLodPathString = lodArray[iLodLevel];

		#ifdef _DEBUG
		AFTK_DO_BEGIN << "Barrier Lod path string dyn attr. " << mLodPathString.asChar() << "\n" AFTK_DO_DEBUG_END;
		#endif
		
		std::string lodPathString = mLodPathString.asChar();
		Tu::SearchAndReplace(lodPathString, "|", "|barrierModels:");

		Aftk::C_SceneMeshNode* pAftkLodLevelMesh = 0;


		if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromDagPathString(lodPathString, (Aftk::C_SceneNode**)&pAftkLodLevelMesh))
			AFTK_DO_BEGIN << "Failed to acquire barrier lod level [" << iLodLevel << "]. GetAftkSceneNodeFromDagPathString failed to find Aftk mesh from Maya path [" << lodPathString << "] on mesh [" << pAftkOriginalBonedInstanceNode->GetName() << "]" AFTK_DO_ERROR_RETURN_END_1;

		MDagPath lodInstanceParent;
		if (!pImporter->GetAftkMayaMapping().GetMayaDagPathFromAftkSceneNode(pAftkLodLevelMesh, &lodInstanceParent))
			AFTK_DO_BEGIN << "Failed to acquire barrier lod level [" << iLodLevel << "]. GetMayaDagPathFromAftkSceneNode failed to find Maya path for Aftk scene node [" << pAftkLodLevelMesh->GetName() << "] on mesh [" << pAftkOriginalBonedInstanceNode->GetName() << "]" AFTK_DO_ERROR_RETURN_END_1;


		U32 insertionPoint = 0;

		//Set the name and threshold for this lod level
		Aftk::C_GroupLodLevelInfo info;
		if (iLodLevel == lodPlugArray.numElements()-1)
		{
			info.SetThreshold(0);
		}
		else
		{
			info.SetThreshold(PGR4_THRESHOLD_INCREMENT*(iLodLevel+1));
		}

		Aftk::C_SceneMeshBonedInstanceNode* pThisLodLevel;
		std::string instanceSourceDagString = instanceParentDagPath.fullPathName().asChar();
		if (instanceSourceDagString == lodPathString)
		{
			//Add original lod to the group.
			pImporter->GetAftkScenePtr()->ExtractSceneNodeAndChildren(pAftkOriginalBonedInstanceNode);
			pAftkLodGroupNode->AddChildPtr(pAftkOriginalBonedInstanceNode);

			//Use our original boned instance for the current lod level.
			pThisLodLevel = pAftkOriginalBonedInstanceNode;
		}
		else
		{
			//Create new boned instance.
			Aftk::C_SceneMeshBonedInstanceNode* pNewBonedInstance = new Aftk::C_SceneMeshBonedInstanceNode(
				pImporter->GetAftkScenePtr());

			pAftkLodGroupNode->AddChildPtr(pNewBonedInstance);

			pThisLodLevel = pNewBonedInstance;

			status = ImportLodLevel(
				pImporter->GetAftkScenePtr(),
				pThisLodLevel,
				pAftkOriginalBonedInstanceNode,
				pAftkLodLevelMesh);
			AFTK_CHECK_ERROR_RETURN;
		}
		//Associate with lod info.
		info.SetSceneNodePtr(pThisLodLevel);

		status = ImportBonesAndCloneIfNeeded(
			pImporter,
			bones,
			pThisLodLevel,
			instanceDagPath,
			pAftkInstanceParent,
			lodInstanceParent);
		AFTK_CHECK_ERROR_RETURN;

		//Override materials for each lod.
		status = OverrideMaterials(
			pImporter,
			pOptions,
			lodInstanceParent, 
			instanceDagPath,
			pThisLodLevel);
		AFTK_CHECK_ERROR_RETURN;

		status = OverrideScratchMaterials(
			pImporter,
			pOptions,
			instanceParentDagPath, 
			instanceDagPath,
			pThisLodLevel);
		AFTK_CHECK_ERROR_RETURN;

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

Aftk::C_Status C_PGR4GameImport::ImportLodLevel(
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
