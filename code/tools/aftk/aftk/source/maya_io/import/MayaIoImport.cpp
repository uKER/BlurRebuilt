#include "stdafx.h"

#include "game_specific/MayaIoIGameImport.h"
#include "game_specific/pgr3/MayaIoPGR3GameImport.h"
#include "game_specific/pgr4/MayaIoPGR4GameImport.h"
#include "game_specific/shitstorm/MayaIoShitstormGameImport.h"
#include "../MayaIoMatrixHelper.h"
#include "MayaIoIrmPostProcess.h"
#include "MayaIoSceneConstraintImport.h"

#include <maya/MItSelectionList.h>
#include <maya/MCommandResult.h>
#include <core/utility/stringproc.h>

using namespace MayaIo;

C_Importer::C_Importer()
{
	m_bindPoseHelper.SetImporterPtr(this);
	m_interestingNodeCount = 0;
	m_interestingNodeImportedCount = 1;

	m_pAftkScene = 0;

	//Callbacks
	m_pProgressNotification = 0;
	m_pIGameImport = 0;
}

C_Importer::~C_Importer()
{
	Clear();
}

void C_Importer::Clear()
{
	m_debrisList.Clear();
	// Stores all of the dag paths processed, along with pointers to the AftkSceneNodes which were added
	m_instanceTracker.Clear();
	// We need to store a stack for the transforms that we are processing. The reason for this is that in cases
	// where multiple transforms are connected to each other (and have no shapes), we want to to collapse these
	// transforms instead to adding null nodes.
	m_shapelessTransformStack.Clear();

	m_layerMap.Clear();

	m_materialMap.Clear();

	m_aftkMayaMapping.Destroy();

	if (m_pIGameImport)
	{
		//Delete the game specific importer.
		delete m_pIGameImport;
		m_pIGameImport = 0;
	}
}

void C_Importer::CleanUpAfterFailure()
{
	m_bindPoseHelper.UndoGoIntoBindPose();
	Clear();
}

void C_Importer::AddDebris(Aftk::C_SceneNode* pSceneNode)
{
	for (S32 i = 0; i < m_debrisList.Size(); i++)
		if (m_debrisList[i] == pSceneNode)
			return;

	m_debrisList.PushBack(pSceneNode);
}

bool C_Importer::IsDebris(const Aftk::C_SceneNode* pSceneNode) const
{
	for (U32 i = 0; i < (U32)m_debrisList.Size(); i++)
	{
		if (pSceneNode == m_debrisList[i])
			return true;
	}
	return false;
}

const C_ImportOptions* C_Importer::GetImportOptionsPtr() const
{
	return &m_options;
}

Aftk::C_Status C_Importer::PreProcessScene()
{
	Aftk::C_Status status;
	MStatus mStatus;
	MSelectionList list;
	MGlobal::getActiveSelectionList(list);

	m_interestingNodeCount = 0;
	Ut::UtSet<std::string> bonedInstanceJoints;
	{
		MItDag itDag(MItDag::kBreadthFirst, MFn::kTransform, &mStatus);
		MCHECK_BEGIN << TRANS_ACQUIRE_DAG_IT_FAIL MCHECK_ERROR_RETURN_END_1
		for (; !itDag.isDone() && itDag.depth() <= 1; itDag.next()) 
		{
			MDagPath transformDagPath;
			mStatus = itDag.getPath(transformDagPath);
			MCHECK_BEGIN << TRANS_ACQUIRE_DAG_PATH_FAIL MCHECK_ERROR_RETURN_END_1
			status = PreProcessDagNode(
				bonedInstanceJoints,
				transformDagPath,
				list);
			CHECK_ERROR_RETURN;
		}
	}
	{
		MItDag itDag(MItDag::kBreadthFirst, MFn::kTransform, &mStatus);
		MCHECK_BEGIN << TRANS_ACQUIRE_DAG_IT_FAIL MCHECK_ERROR_RETURN_END_1;
		for (; !itDag.isDone(); itDag.next()) 
		{
			MDagPath transformDagPath;
			mStatus = itDag.getPath(transformDagPath);
			MCHECK_ERROR_RETURN_1;
			if (bonedInstanceJoints.Find(transformDagPath.fullPathName().asChar()) 
				!= bonedInstanceJoints.End())
				//We've found a boned instance joint.
				m_bonedInstanceJointList.Add(transformDagPath);
		}
	}

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Found " << m_interestingNodeCount << " interesting nodes\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::PreProcessDagNode(
	Ut::UtSet<std::string>& bonedInstanceJoints,
	const MDagPath& transformDagPath,
	const MSelectionList& list)
{
	Aftk::C_Status status;
	MStatus mStatus;
	bool doWeImportThisNode;
	bool doWeImportItsChildren;
	DoWeImportThisNode(
		transformDagPath, 
		doWeImportThisNode,
		doWeImportItsChildren,
		status);
	CHECK_ERROR_RETURN;
			
	MFnDagNode fnTransform(transformDagPath, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;

	if (m_options.importSelection && (!list.hasItem(transformDagPath, MObject::kNullObj, &mStatus)))
		doWeImportThisNode = false;

	//Only pre-process nodes within our set of nodes to be imported.
	if (doWeImportThisNode)
	{
		if (m_pIGameImport && m_pIGameImport->PreserveChildJointTransforms(
				this,
				transformDagPath))
		{
			for (U32 i = 0; i < transformDagPath.childCount(); i++)
			{
				MObject childObj = transformDagPath.child(i, &mStatus);
				MCHECK_ERROR_RETURN_1;
				if (!childObj.hasFn(MFn::kJoint))
					continue;

				MFnDagNode childNode(childObj, &mStatus);
				MCHECK_ERROR_RETURN_1;
				MDagPath childPath;
				mStatus = childNode.getPath(childPath);
				MCHECK_ERROR_RETURN_1;

				bonedInstanceJoints.Insert(std::string(childPath.fullPathName().asChar()));
			}
		}

		//Store these mappings for all nodes.
		m_aftkMayaMapping.SetNameStringMayaSceneNodeMapping(fnTransform.name().asChar(), transformDagPath);
		m_aftkMayaMapping.SetDagPathStringMayaSceneNodeMapping(transformDagPath.fullPathName().asChar(), transformDagPath);
		m_interestingNodeCount++;
	}

	if (doWeImportItsChildren)
	{
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

			PreProcessDagNode(
				bonedInstanceJoints, 
				childPath,
				list);
			CHECK_ERROR_RETURN;
		}
	}
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::DGDirty()
{
	Aftk::C_Status status;
	MStatus mStatus = MGlobal::executeCommand("dgdirty", true, false);
	MCHECK_ERROR_RETURN_1;
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::ReloadIRM()
{
	Aftk::C_Status status;
	// To prevent any race conditions, run the cleanup and reload separately
	MStatus mStatus = MGlobal::executeCommand("eval(\"biz_DO_CleanupReferences();biz_DO_RestoreLostInstances(0);biz_DO_ReloadReferences(1);\")", true, false);
	if (!mStatus)
	{
		Tu::StringBuilder b;
		b << "Failed to reload Instance References. Please resolve this problem using the Instance Reference Manager before re-exporting";
		Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());
		status.SetErrorString(b.Text());
		status.SetErrorCode(Aftk::C_Status::E_Status_Failure);
		status.SetSeverity(1);
		return status;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::Import(
	Aftk::C_Scene* pAftkScene,
	const char* pOptions,
	Aftk::I_Progress* pProgress)
{
	Aftk::C_Status status;
	MStatus mStatus;

	m_options.Clear();

	//If we've got an option string then parse it.
	if (pOptions)
	{
		std::string importOptionsString = pOptions;
		status = GetParameters(m_options, importOptionsString);
		if (!status) return status;
	}

	m_pProgressNotification = pProgress;

	if (m_options.project.length() == 0)
	{
		MCommandResult result;
		mStatus = MGlobal::executeCommand("optionVar -q \"biz_TM_currentProject\"", result, true, false);
		if (mStatus)
		{
			MString projectName;
			mStatus = result.getResult(projectName);
			if (mStatus)
				m_options.project = projectName.asChar();
		}
	}

	//Instantiate game specific import code.
	if (m_options.project == "project_gotham")
	{
		MCommandResult result;
		mStatus = MGlobal::executeCommand("biz_mb_findBarrierType()", result, true, false);
		MCHECK_ERROR_RETURN_1;

		int gameVersion;
		mStatus = result.getResult(gameVersion);
		MCHECK_ERROR_RETURN_1;

		switch(gameVersion)
		{
		case 3:
			AFTK_DO_BEGIN << "Performing PGR3 Export." AFTK_DO_DEBUG_END;
			m_pIGameImport = (I_GameImport*)new C_PGR3GameImport();
			break;

		case 4:
		default:
			AFTK_DO_BEGIN << "Performing PGR4 Export." AFTK_DO_DEBUG_END;
			m_pIGameImport = (I_GameImport*)new C_PGR4GameImport();
			break;
		};
	}
	else if (m_options.project == "project_shooter")
	{
		AFTK_DO_BEGIN << "Performing Shooter Export." AFTK_DO_DEBUG_END;
		m_pIGameImport = (I_GameImport*)new C_ShitstormGameImport();
	}
	else
		m_pIGameImport = 0;

	//Hopefully this will fix blind-data oddness.
	status = DGDirty();
	CHECK_ERROR_CLEAN_UP_RETURN;

	//Reload IRM data on full exports (Not export selection)
	if (!m_options.importSelection) {

		// Only run this function if in command line mode
		if (MGlobal::mayaState() == MGlobal::kBatch ||
			MGlobal::mayaState() == MGlobal::kLibraryApp) 
		{
			status = ReloadIRM();
			CHECK_ERROR_CLEAN_UP_RETURN;
		}
	}

	status = m_aftkMayaMapping.Create();
	CHECK_ERROR_CLEAN_UP_RETURN;

	if (m_options.validateMaterials)
	{
		status = C_MaterialHelper::ValidateTextures(
			this,
			&m_options,
			pProgress);
		CHECK_ERROR_CLEAN_UP_RETURN;
	}

	m_pAftkScene = pAftkScene;

	//Clear our list of joints to be excluded from the import process.
	m_bonedInstanceJointList.Clear();

	//Pre-process scene in order to:
	//1) Count the number of nodes to import.
	//2) Flag any boned-instance joints as being non-importable.
	status = PreProcessScene();
	CHECK_ERROR_CLEAN_UP_RETURN;

	//////////////////// GAME SPECIFIC IMPORT CODE ////////////////////
	//TODO: Move game specific code in to a seperate DLL loaded at run-time.

	//If we have a game specific importer, perform game specific scene
	//post processing.
	if (m_pIGameImport)
	{
		AFTK_DO_BEGIN << "Game specific pre-process..." AFTK_DO_DEBUG_END;
		status = m_pIGameImport->PreProcessScene(this, &m_options);
		AFTK_CHECK_ERROR_CLEAN_UP_RETURN;
	}
	else
	{
		AFTK_DO_BEGIN << "No game specific pre-process." AFTK_DO_DEBUG_END;
	}
	//////////////////// GAME SPECIFIC IMPORT CODE ////////////////////

	status = m_bindPoseHelper.GoIntoBindPose();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	SetProgressMax(m_interestingNodeCount);
	SetProgress(0);

	//Build poly-set information.
	status = BuildSetInfo();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	//Import scene graph
	status = TraverseDagTopLevel(m_pAftkScene);
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	AFTK_DO_BEGIN << "Importing bind poses..." AFTK_DO_DEBUG_END;
	//Import bind poses
	status = m_bindPoseHelper.ImportBindPoses();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	AFTK_DO_BEGIN << "Importing boned instances..." AFTK_DO_DEBUG_END;
	//We can only resolve the instance objects for boned instances after we've first 
	//imported the scene graph.
	status = ImportBonedInstances();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	AFTK_DO_BEGIN << "Importing skin clusters..." AFTK_DO_DEBUG_END;
	//Import mesh skinning information
	status = ImportSkinClusters();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	AFTK_DO_BEGIN << "Importing connected dg nodes..." AFTK_DO_DEBUG_END;
	status = ImportDg();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	AFTK_DO_BEGIN << "Importing LODs..." AFTK_DO_DEBUG_END;
	//Combine the meshes in a lod into a single lod mesh.
	if (m_options.mergeLodMeshes)
	{
		status = C_SceneLodMeshImport::MergeLodMeshesSceneWide(
			this,
			m_pAftkScene,
			&m_options);
		AFTK_CHECK_ERROR_CLEAN_UP_RETURN;
	}
	//Import lod groups instead.
	else
	{
		status = C_SceneLodGroupImport::ResolveChildrenSceneWide(
			this,
			m_pAftkScene,
			&m_options);
		AFTK_CHECK_ERROR_CLEAN_UP_RETURN;
	}

	status = C_SceneConstraintImport::ResolveConnectionsSceneWide(
		this,
		m_pAftkScene,
		&m_options);
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	status = C_SceneCameraImport::PostProcess(
		this,
		m_pAftkScene,
		&m_options);
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;


	//////////////////// GAME SPECIFIC IMPORT CODE ////////////////////
	//TODO: Move game specific code in to a seperate DLL loaded at run-time.

	//If we have a game specific importer, perform game specific scene
	//post processing.
	if (m_pIGameImport)
	{
		AFTK_DO_BEGIN << "Performing game specific import..." AFTK_DO_DEBUG_END;
		status = m_pIGameImport->PostProcessScene(this, &m_options);
		AFTK_CHECK_ERROR_CLEAN_UP_RETURN;
	}
	else
	{
		AFTK_DO_BEGIN << "No game specific post-process." AFTK_DO_DEBUG_END;
	}
	//////////////////// GAME SPECIFIC IMPORT CODE ////////////////////

	pAftkScene->SetOrigin("maya");

	AFTK_DO_BEGIN << "Re-sourcing indirect instances..." AFTK_DO_DEBUG_END;
	U32 indirectInstanceCount = m_pAftkScene->ResourceIndirectInstances();
	AFTK_DO_BEGIN << indirectInstanceCount << " indirect instance(s) re-sourced." AFTK_DO_DEBUG_END;

	//Flip scene about z-axis to move from Maya's right handed coordinate system into a left handed one.
	if (m_options.zflip && m_pAftkScene->GetRootSceneNodePtr())
	{
		Aftk::C_StaticPlug<F32>& zScalePlug = 
			(Aftk::C_StaticPlug<F32>&)*m_pAftkScene->GetRootSceneNodePtr()->GetScalePlug().GetChildPtr(2);

		F32 zScale;
		zScalePlug.GetValue(zScale, false);
		zScale *= -1.f;
		zScalePlug.SetValue(zScale);

		//Invert base poses as these are in world space.
		for (Aftk::C_SceneNodeIterator it(m_pAftkScene); it; ++it)
		{
			Aftk::C_SceneNode* pNode = it;
			pNode->SetBasePose(pNode->GetBasePose() * m_pAftkScene->GetRootSceneNodePtr()->GetLocalMatrix(false));
		}

		//Disable segment scale compensation on root joints. Otherwise would compensate for -1 in z.
		Aftk::C_SceneNode* pChild = m_pAftkScene->GetRootSceneNodePtr()->GetChildPtr();
		while(pChild)
		{
			if (pChild->HasType(Aftk::E_ObjType_SceneJoint))
			{
				Aftk::C_SceneJointNode* pJoint = (Aftk::C_SceneJointNode*)pChild;
				pJoint->SetSegmentScaleCompensation(false);
			}

			pChild = pChild->GetNextPtr();
		}
	}

#ifndef BUILD_CONFIG_DEBUG
	if (m_pAftkScene != 0 && m_options.meshWeld)
	{
		AFTK_DO_BEGIN << "Welding meshes..." AFTK_DO_DEBUG_END;
		//The class below contains memory pools we'll need.
		Aftk::C_MeshWeldMemoryManager weldMemoryManager
			(AFTK_MESH_WELD_MEMORY_MANAGER_LARGE_CAPACITY);

		//This will be visiting our nodes.
		Aftk::C_MeshWeld::C_WeldVisitor visitor;
		//Set tolerances.
		visitor.positionThreshold	= Aftk::kOneMM;
		visitor.normalThreshold		= Aftk::kSlackTolerence;
		visitor.uvThreshold			= Aftk::kTightTolerence;
		visitor.tangentThreshold	= Aftk::kSlackTolerence;

		//Perform work.
		status = Aftk::C_SceneNodeVisit::Traverse(
			m_pAftkScene, 
			visitor,
			weldMemoryManager, 
			pProgress);

		AFTK_DO_BEGIN << "Validating scene..." AFTK_DO_DEBUG_END;
		if (m_pAftkScene->Validate(pProgress) != 0)
		{
			AFTK_DO_BEGIN << "Could not validate scene..." AFTK_DO_ERROR_CLEAN_UP_RETURN_END_1
		}
	}
#endif

	AFTK_DO_BEGIN << "IRM post-process..." AFTK_DO_DEBUG_END;
	status = C_IrmPostProcess::PostProcessScene(*m_pAftkScene);
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	m_pAftkScene->Validate();

	AFTK_DO_BEGIN << "Restoring original node positions..." AFTK_DO_DEBUG_END;
	status = m_bindPoseHelper.UndoGoIntoBindPose();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	AFTK_DO_BEGIN << "Removing debris..." AFTK_DO_DEBUG_END;
	status = ReplaceDebris();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	AFTK_DO_BEGIN << "Performing cleanup..."		 AFTK_DO_DEBUG_END;
	Clear();
	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_Importer::TraverseDagTopLevel(Aftk::C_Scene *pAftkScene)
{
	Aftk::C_Status status;
	MStatus mStatus;

	// Use DAG iterator to traverse level 0
	MFnDagNode dagNode;
	Ut::UtVector< Ut::DynamicVector<Aftk::Matrix44> > matrixStack;
	MItDag itDag(MItDag::kBreadthFirst, MFn::kTransform, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_IT_FAIL MCHECK_ERROR_RETURN_END_1

	// Add root node
	Aftk::C_SceneNode* pAftkRootNode = new Aftk::C_SceneNode(m_pAftkScene);
	pAftkRootNode->SetName("AftkRoot");
	m_pAftkScene->SetRootSceneNodePtr(pAftkRootNode);

	for (; !itDag.isDone() && itDag.depth() <= 1; itDag.next()) 
	{
		MDagPath transformDagPath;
		mStatus = itDag.getPath(transformDagPath);
		MCHECK_BEGIN << TRANS_ACQUIRE_DAG_PATH_FAIL MCHECK_ERROR_RETURN_END_1

		//Recurse...
		status = ImportTransformAndShapeIfPresent(
			transformDagPath, 
			matrixStack,
			0,
			pAftkScene);
		CHECK_ERROR_RETURN;
	}

	return Aftk::C_Status::E_Status_Success;
}

void C_Importer::DoWeImportThisNode(
	const MDagPath& dagPath,
	bool& doWeImportThisNode,
	bool& doWeImportItsChildren,
	Aftk::C_Status& status)
{
    MStatus mStatus;
	if (m_options.importSelection == false)
	{
		if (IsExcludedByImportFilters(dagPath, status))
		{
			//Don't import this nodes children if filtered out
			doWeImportItsChildren = false;
			doWeImportThisNode = false;
			return;
		}
		//We're importing everything!
		doWeImportItsChildren = true;
		doWeImportThisNode = true;
		return;
	}

	MSelectionList list;
	MGlobal::getActiveSelectionList(list);

	if (list.isEmpty(&mStatus) && !m_options.importSelection)
	{
		if (IsExcludedByImportFilters(dagPath, status))
		{
			doWeImportItsChildren = false;
			doWeImportThisNode = false;
			return;
		}

		doWeImportItsChildren = true;
		doWeImportThisNode = true;
		return;
	}
	if (!mStatus)
	{
		AFTK_DO_BEGIN << TRANS_ACQUIRE_SELECTION_LIST_FAIL AFTK_DO_ERROR_END_1;
		doWeImportItsChildren = false;
		doWeImportThisNode = false;
		return;
	}

	//Only query for complete objects, partial (component) selections are treated as object selections.
	bool hasItem = list.hasItem(dagPath, MObject::kNullObj, &mStatus);
	if (!mStatus)
	{
		AFTK_DO_BEGIN << TRANS_QUERY_SELECTION_LIST_FAIL AFTK_DO_ERROR_END_1;
		doWeImportItsChildren = false;
		doWeImportThisNode = false;
		return;
	}

	if (hasItem)
	{
		doWeImportItsChildren = true;
		doWeImportThisNode = true;
		return;
	}

	//This node was not part of the current selection. Do not import this node, but continue attempting
	//to import its children.
	doWeImportItsChildren = true;
	doWeImportThisNode = false;
	status = Aftk::C_Status::E_Status_Success;
}

bool C_Importer::IsExcludedByImportFilters(
	const MDagPath& dagPath, 
	Aftk::C_Status& status)
{
	MStatus mStatus;
	MFnDagNode dagNode(dagPath, &mStatus);
	if (!status)
	{
		AFTK_DO_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL AFTK_DO_ERROR_END_1;
		return false;
	}

	if (m_sceneNodeFilter.Find(dagNode.name().asChar()))
	{
		status = Aftk::C_Status::E_Status_Success;
		return true;
	}

	MString layerName;
	bool	layerMember;
	status = GetLayerName(dagPath, layerMember, layerName);
	if (m_layerFilter.Find(layerName.asChar()))
	{
		status = Aftk::C_Status::E_Status_Success;
		return true;
	}

	status = Aftk::C_Status::E_Status_Success;
	return false;
}

Aftk::C_Status C_Importer::ImportTransformAndShapeIfPresent(
	const MDagPath&	transformDagPath, 
	Ut::UtVector< Ut::DynamicVector<Aftk::Matrix44> >& matrixStack,
	Aftk::C_SceneNode*	pLastGoodAncestor,
	Aftk::C_Scene*		pAftkScene
	)
{
	Aftk::C_Status status;
	MStatus mStatus;	

	bool hasTransform = true;

	//MTransformationMatrix transMatrix;
	MMatrix mayaLocalTransformMatrix;
	mayaLocalTransformMatrix = mayaLocalTransformMatrix.identity;

	MFnDagNode fnTransform(transformDagPath);

	// DAG iterator
	MItDag itDag;	
	// AFTK node to be created
	Aftk::C_SceneNode* pAftkNewSceneNode = 0; 

	if (!transformDagPath.hasFn(MFn::kTransform)) 
	{
		AFTK_DO_BEGIN << "Object at DAG path was not a transform." AFTK_DO_ERROR_RETURN_END_1
	}

	Ut::UtVector< Ut::DynamicVector<Aftk::Matrix44> > newMatrixStack;
	//Build the ancestral transformation matrix from the current stack.
	Aftk::Matrix44 exclusiveMatrixNotToUseDirectly;
	UtMaths::SetIdentity(&exclusiveMatrixNotToUseDirectly);
	Aftk::Matrix44* pExclusiveMatrix;
	for (U32 i = 0; i < (U32)matrixStack.Size(); i++)
		exclusiveMatrixNotToUseDirectly = matrixStack[i] * exclusiveMatrixNotToUseDirectly;
	if (matrixStack.Size() > 0)
		pExclusiveMatrix = &exclusiveMatrixNotToUseDirectly;
	else
		pExclusiveMatrix = 0;

	//C_Node: Before we recurse any children, we need to find an appropriate ancestor for them.
	Aftk::C_SceneNode* pChildAncestor;
	
	bool doWeImportThisNode;
	bool doWeImportItsChildren;
	DoWeImportThisNode(
		transformDagPath, 
		doWeImportThisNode,
		doWeImportItsChildren,
		status);
	AFTK_CHECK_ERROR_RETURN;

	if (doWeImportThisNode)
	{
		//Update progress
		if (m_pProgressNotification)
		{
			std::string title;
			std::string count;
			{
				char buffer[20];
				sprintf(buffer, "%i", m_interestingNodeImportedCount);
				count = buffer;
			}
			title += "Node [";
			title += count;
			title += "/";
			{
				char buffer[20];
				sprintf(buffer, "%i", m_interestingNodeCount);
				count = buffer;
			}
			title += count;
			title += "] : ";
			title += fnTransform.name().asChar();

			m_pProgressNotification->OnPrintProgress(title);
			m_pProgressNotification->OnProgress(((F32)m_interestingNodeImportedCount / (F32)m_interestingNodeCount)*100.f);

			//Update progress bar
			AdvanceProgress();
		}
		//Increase the imported node count for next time.
		m_interestingNodeImportedCount++;
		{
			//Perform additional processing if our transform is instanced.
			if (transformDagPath.isInstanced())
			{
				bool foundInstance = false;
				U32 instanceIndex;

				for (instanceIndex = 0; instanceIndex < (U32)m_instanceTracker.Size(); instanceIndex++)
				{
					if (m_instanceTracker[instanceIndex].m_dagPath.node() == transformDagPath.node()) 
					{
						// decrease the number of references to this node that we have yet to process
						m_instanceTracker[instanceIndex].m_remainingInstances--;
						if (m_instanceTracker[instanceIndex].m_remainingInstances == 0) 
						{
							m_instanceTracker.EraseByIndex(instanceIndex);
						}
						foundInstance = true;
						break;
					}
				}
				if (!foundInstance) 
				{
					C_InstanceTracker newInstance;
					newInstance.m_dagPath = transformDagPath;
					// Get the number of direct and indirect instances of this node
					newInstance.m_remainingInstances = fnTransform.instanceCount(true);

					MDagPath shapeDagPath(transformDagPath);
					if (!shapeDagPath.extendToShape())
					{
						pAftkNewSceneNode = 
							ImportTransformWithoutShape(
							transformDagPath, 
							pExclusiveMatrix,
							pAftkScene, 
							status);
						//Check for any errors, returning if need be.
						if (!status || pAftkNewSceneNode == 0)
						{
							if (status)	
							{
								AFTK_DO_BEGIN << "Unknown failure when importing transform without shape." AFTK_DO_ERROR_END_1;
								pAftkNewSceneNode = 0;
							}
							return status;
						}
					}
					//Import the data (the shape) for the shape associated with this transform node.
					else
					{
						pAftkNewSceneNode = ResolveAndImportShapeOrShapeInstance(
							transformDagPath,
							pExclusiveMatrix,
							pAftkScene, 
							status);
						//Check for any errors, returning if need be.
						if (!status || pAftkNewSceneNode == 0)
						{
							if (status)	
							{
								AFTK_DO_BEGIN << "Unknown error when resolving shape or shape instance." AFTK_DO_ERROR_END_1;
								pAftkNewSceneNode = 0;
							}
							return status;
						}
					}
					newInstance.m_pAftkSceneNode = pAftkNewSceneNode;
					m_instanceTracker.PushBack(newInstance);

					//Add our new node to the aftk <--> maya node mapping tables. Note that we _only_
					//do this for our base-instance transform.
					m_aftkMayaMapping.SetAftkMayaSceneNodeMapping(pAftkNewSceneNode, transformDagPath);
					m_aftkMayaMapping.SetMayaAftkSceneNodeMapping(transformDagPath, pAftkNewSceneNode);
				}
				else
				{
					// This shape is an instance, perform appropriate import
					C_InstanceTracker* pInstance = &m_instanceTracker[instanceIndex];
					Aftk::C_SceneNode* pAftkSourceNode = pInstance->m_pAftkSceneNode;

					pAftkNewSceneNode = new Aftk::C_SceneInstanceNode(pAftkScene);
					if (!pAftkNewSceneNode)
						AFTK_DO_BEGIN << "Failed to allocate memory for new instance." AFTK_DO_ERROR_RETURN_END_1;

						if (!C_SceneInstanceImport::Import(
							this,
							transformDagPath, 
							pExclusiveMatrix,
							pAftkScene, 
							pAftkNewSceneNode,
							&m_options,
							pAftkSourceNode))
					{
						AFTK_DO_BEGIN << "Could not import instance" AFTK_DO_ERROR_RETURN_END_1;
					} 
				}
			}
			//The transform is not instanced, and is not instancing any other transform.
			else
			{
				MDagPath shapeDagPath(transformDagPath);
				if (!shapeDagPath.extendToShape())
				{
					pAftkNewSceneNode = 
						ImportTransformWithoutShape(
						transformDagPath, 
						pExclusiveMatrix,
						pAftkScene, 
						status);
					//Check for any errors, returning if need be.
					if (!status || pAftkNewSceneNode == 0)
					{
						if (status)	
						{
							AFTK_DO_BEGIN << "Unknown error when importing transform without shape." AFTK_DO_ERROR_END_1;
							pAftkNewSceneNode = 0;
						}
						return status;
					}
				}
				//Import the data (the shape) for the shape associated with this transform node.
				else
				{
					pAftkNewSceneNode = ResolveAndImportShapeOrShapeInstance(
						transformDagPath,
						pExclusiveMatrix,
						pAftkScene, 
						status);
					//Check for any errors, returning if need be.
					if (!status || pAftkNewSceneNode == 0)
					{
						if (status)	
						{
							AFTK_DO_BEGIN << "Unknown error when importing transform and shape." AFTK_DO_ERROR_END_1;
							pAftkNewSceneNode = 0;
						}
						return status;
					}
				}

				//Add our new node to the aftk <--> maya node mapping tables
				m_aftkMayaMapping.SetAftkMayaSceneNodeMapping(pAftkNewSceneNode, transformDagPath);
				m_aftkMayaMapping.SetMayaAftkSceneNodeMapping(transformDagPath, pAftkNewSceneNode);
			}
		}
		//We're using this node as the ancestor for any children
		pChildAncestor = pAftkNewSceneNode;
	}
	//Else we're not importing this node.
	else
	{
		//We're using the previous ancestor of this node for any children it might have
		pChildAncestor = pLastGoodAncestor;
	}

	if (doWeImportThisNode)
	{
		//Attach this node to our last good ancestor
		if (!pLastGoodAncestor)
		{
			if (pAftkNewSceneNode != pAftkScene->GetRootSceneNodePtr())
				if (!pAftkScene->GetRootSceneNodePtr())
					pAftkScene->SetRootSceneNodePtr(pAftkNewSceneNode);
				else
					pAftkScene->GetRootSceneNodePtr()->AddChildPtr(pAftkNewSceneNode);
			else
				AFTK_ASSERT(false);
		}
		else
			pLastGoodAncestor->AddChildPtr(pAftkNewSceneNode);
	}

	// add children
	if (doWeImportItsChildren)
	{
		for (U32 iChild = 0; iChild < transformDagPath.childCount(); iChild++)
		{
			MObject childObj = transformDagPath.child(iChild, &mStatus);
			MCHECK_ERROR_RETURN_1;

			if (!childObj.hasFn(MFn::kTransform))
				continue;
			MCHECK_ERROR_RETURN_1;
			MFnDagNode childNode(childObj, &mStatus);
			MCHECK_ERROR_RETURN_1;
			MDagPath childPath;
			mStatus = childNode.getPath(childPath);
			MCHECK_ERROR_RETURN_1;

			Aftk::Matrix44 aftkMatrix;
			{
				MMatrix currentMatrix;
				Aftk::E_RotationOrder rotationOrder;
				if (!MatrixHelper::GetMatrix(
					currentMatrix,
					rotationOrder,
					transformDagPath, 
					m_options.absNumericRange))
				{
//					if (m_options.strict)
//						AFTK_DO_BEGIN << TRANS_INFINITE_MATRIX_FAIL AFTK_DO_ERROR_RETURN_END_0;
//					else
//						AFTK_DO_BEGIN << TRANS_INFINITE_MATRIX_WARN AFTK_DO_WARNING_END;
				}
				if (!MatrixHelper::MayaToAftkMatrix44(
					aftkMatrix,
					currentMatrix,
					m_options.absNumericRange))
				{
					AFTK_DO_BEGIN << "Could not convert Maya matrix for " << pAftkNewSceneNode->GetName() << " as it contains invalid." AFTK_DO_WARNING_END;
				}
			}
			if (doWeImportThisNode)
			{
#ifdef BUILD_CONFIG_DEBUG
//				AFTK_DO_BEGIN << "Using fresh matrix stack." << "\n" AFTK_DO_INFO_END;
#endif
				// Import child
				status = ImportTransformAndShapeIfPresent(
					childPath,
					newMatrixStack, //Use empty stack.
					pChildAncestor,
					pAftkScene);
				AFTK_CHECK_ERROR_RETURN;
			}
			else
			{
#ifdef BUILD_CONFIG_DEBUG
//				AFTK_DO_BEGIN << "pushing matrix " << fnTransform.name().asChar() << "\n" AFTK_DO_INFO_END;
#endif
				matrixStack.PushBack(aftkMatrix);
			
				// Import child
				status = ImportTransformAndShapeIfPresent(
					childPath,
					matrixStack, //Use existing stack.
					pChildAncestor,
					pAftkScene);
				AFTK_CHECK_ERROR_RETURN;

#ifdef BUILD_CONFIG_DEBUG
//				AFTK_DO_BEGIN << "popping matrix " << fnTransform.name().asChar() << "\n" AFTK_DO_INFO_END;
#endif
				matrixStack.PopBack();
			}
		}
	}

	//If we're importing this node, attach to layer and apply blind data.
	if (doWeImportThisNode)
	{
		// do we have a root node? if not, add one.
		if (pAftkScene->GetRootSceneNodePtr() == 0) 
			pAftkScene->SetRootSceneNodePtr(pAftkNewSceneNode);

		status = AddToLayerIfNeeded(fnTransform, pAftkNewSceneNode);
		AFTK_CHECK_ERROR_RETURN;
	}

	return Aftk::C_Status::E_Status_Success;
}

bool C_Importer::IsBonedInstance(const MFnDagNode& fnTransform)
{
	if (m_options.deformedInstancesAsMesh)
		return false;

	MStatus status;
	MPlug bonedInstanceParent = fnTransform.findPlug("bonedInstanceParent", &status);
	if (!status)
		return false;
	return true;
}

bool C_Importer::IsBonedInstanceParent(const MFnDagNode& fnTransform)
{
	if (m_options.deformedInstancesAsMesh)
		return false;
	
	if (IsBonedInstance(fnTransform))
		return false;

	MStatus status;
	MPlug bonedInstanceParent = fnTransform.findPlug("instanceBone0", &status);
	if (!status)
		return false;
	return true;
}

bool C_Importer::IsBonedInstanceJoint(const MDagPath& path)
{
	if (m_options.deformedInstancesAsMesh)
		return false;

	if (m_bonedInstanceJointList.Find(path))
		return true;

	return false;
}

C_AftkMayaMapping& C_Importer::GetAftkMayaMapping()
{
	return m_aftkMayaMapping;
}

const C_AftkMayaMapping& C_Importer::GetAftkMayaMapping() const
{
	return m_aftkMayaMapping;
}
bool C_Importer::HasSpeedTreeNode(MFnDagNode& mFnShape, MObject& speedTreeNode) const
{
	MStatus mStatus;

	MStringArray result;
	std::string command = "biz_mb_findSpeedTreeNode(\"" + std::string(mFnShape.fullPathName().asChar()) + "\")";
	mStatus = MGlobal::executeCommand(command.c_str(), result, false, false);
	if (!mStatus)
	{
#pragma message("Get rid of the MEL warnings issued here.")
//		DO_BEGIN << TRANS_FIND_SPEED_TREE_NODE_WARN DO_WARNING_END;
		return false;
	}
	if (result.length() > 0)
	{
		MSelectionList selectionList;
		mStatus = MGlobal::getSelectionListByName(result[0], selectionList);
		if (mStatus)
		{
			if (selectionList.length() > 0)
			{
				mStatus = selectionList.getDependNode(0, speedTreeNode);
				if (mStatus)
				{
					MFnDependencyNode fnNode(speedTreeNode);
					MTypeId typeId = fnNode.typeId(&mStatus);
					if (typeId.id() == MAYA_IO_SPEED_TREE_NODE_ID)
						return true;
					else
						return false;
				}
			}
		}
		AFTK_DO_BEGIN << TRANS_FIND_SPEED_TREE_NODE_WARN AFTK_DO_WARNING_END;
	}

	return false;
}

Aftk::C_SceneNode* C_Importer::ImportTransformWithoutShape(
	const MDagPath &transformDagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_Status& status)
{
	MStatus mStatus;
	Aftk::C_SceneNode* pAftkNewSceneNode = 0;

	MDagPath shapeDagPath(transformDagPath);
	shapeDagPath.extendToShape();

	// Create the appropriate transform node type
	MFnDagNode fnTransform(transformDagPath, &mStatus);
	if (!mStatus)
	{
		AFTK_DO_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL AFTK_DO_ERROR_END_1;
		return 0;
	}
#ifdef _DEBUG
	std::string transformName = fnTransform.name().asChar();
#endif

	//If we're not merging lod meshes, import lod groups.
	if (!m_options.mergeLodMeshes)
	{
		if (transformDagPath.hasFn(MFn::kLodGroup))
		{
			#ifdef DEBUG_PRINT
			AFTK_DO_BEGIN << "Lod Group " << transformName << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
			#endif

			//Note that we use the scene node import code for the joint, as aside from their ID enum, they
			//are identical in every way.

			//Create a new scene node
			pAftkNewSceneNode = new Aftk::C_SceneLodGroupNode(pAftkScene);
			if (!pAftkNewSceneNode)
			{
				AFTK_DO_BEGIN << "Failed to allocate memory for new lod group." AFTK_DO_ERROR_END_1;
				return 0;
			}

			status = C_SceneLodGroupImport::Import(
				this,
				transformDagPath, 
				pExclusiveMatrix,
				pAftkScene, 
				pAftkNewSceneNode,
				&m_options);
			if (!status) return 0;

			return pAftkNewSceneNode;
		}
	}

	if (transformDagPath.hasFn(MFn::kJoint))
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "Joint/Joint instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		//Note that we use the scene node import code for the joint, as aside from their ID enum, they
		//are identical in every way.

		//Create a new scene node
		pAftkNewSceneNode = new Aftk::C_SceneJointNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new joint." AFTK_DO_ERROR_END_1;
			return 0;
		}

		status = C_SceneJointImport::Import(
			this,
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
		if (!status) return 0;
	}
	else if (transformDagPath.hasFn(MFn::kLookAt))
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "AftkImport::ImportShape(): LookAt/LookAt instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		//Create a new scene node
		pAftkNewSceneNode = new Aftk::C_SceneLookAtConstraintNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new look at constraint." AFTK_DO_ERROR_END_1
			return 0;
		}

		status = C_SceneLookAtConstraintImport::Import(
			this,
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
		if (!status) return 0;
	}
	else if (transformDagPath.hasFn(MFn::kAimConstraint))
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "AftkImport::ImportShape(): Aim constraint/Aim constraint instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		//Create a new scene node
		pAftkNewSceneNode = new Aftk::C_SceneAimConstraintNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new aim constaint." AFTK_DO_ERROR_END_1
			return 0;
		}

		status = C_SceneAimConstraintImport::Import(
			this,
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
		if (!status) return 0;
	}
	else if (transformDagPath.hasFn(MFn::kPointConstraint))
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "AftkImport::ImportShape(): Point constraint/Point constraint instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		//Create a new scene node
		pAftkNewSceneNode = new Aftk::C_ScenePointConstraintNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new Point constaint." AFTK_DO_ERROR_END_1
			return 0;
		}

		status = C_ScenePointConstraintImport::Import(
			this,
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
		if (!status) return 0;
	}
	else if (transformDagPath.hasFn(MFn::kOrientConstraint))
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "AftkImport::ImportShape(): Orient constraint/Orient constraint instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		//Create a new scene node
		pAftkNewSceneNode = new Aftk::C_SceneOrientConstraintNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new Orient constaint." AFTK_DO_ERROR_END_1
			return 0;
		}

		status = C_SceneOrientConstraintImport::Import(
			this,
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
		if (!status) return 0;
	}
	else
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "Unknown node type " << name << " encountered!\n" AFTK_DO_DEBUG_END;
		#endif

		// Create a new null node
		pAftkNewSceneNode = new Aftk::C_SceneNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new null node." AFTK_DO_ERROR_END_1;
			return 0;
		}
		
		status = C_SceneNodeImport::Import(
			this,
			transformDagPath,
			pExclusiveMatrix,
			pAftkScene,
			pAftkNewSceneNode,
			true,
			&m_options);

		if (!status) return 0;
	}

	return pAftkNewSceneNode;
}

Aftk::C_SceneNode* C_Importer::ImportTransformWithShape(
	const MDagPath &transformDagPath, 
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_Status& status)
{
	MStatus mStatus;
	Aftk::C_SceneNode* pAftkNewSceneNode = 0;

	MDagPath shapeDagPath = transformDagPath;
	mStatus = shapeDagPath.extendToShape();
	if (!mStatus)
	{
		AFTK_DO_BEGIN << TRANS_EXTEND_TO_SHAPE_FAIL AFTK_DO_ERROR_END_1;
		return 0;
	}

	// Create the appropriate transform node type
	MFnDagNode fnTransform(transformDagPath, &mStatus);

	// Create the appropriate shape node type
	MFnDagNode mFnShapeDagNode(shapeDagPath, &mStatus);

	MTypeId typeId = mFnShapeDagNode.typeId(&mStatus);
#ifdef _DEBUG
	std::string transformName = fnTransform.name().asChar();
	std::string shapeName = mFnShapeDagNode.name().asChar();
	AFTK_DO_BEGIN << "Importing shape [" << shapeName << "] id " << typeId.id() << "\n" AFTK_DO_DEBUG_END;
#endif
	if (!status)
	{
		AFTK_DO_BEGIN << "Failed to query node type." AFTK_DO_ERROR_END_1;
		return 0;
	}
	if (typeId.id() == MAYA_IO_CUSTOM_CONSTRAINT_ID)
	{
		pAftkNewSceneNode = new Aftk::C_SceneConstraintNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new actor." AFTK_DO_ERROR_END_1;
			return 0;
		}

		status = C_SceneConstraintImport::Import(
			this, 
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
	}
	//Check for custom light id.
	else if (typeId.id() == MAYA_IO_CUSTOM_LIGHT_NODE_ID)
	{
		//Import custom light
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "Custom Light/Custom Light instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		pAftkNewSceneNode = C_SceneCustomLightImport::CreateSceneNode(
			transformDagPath, 
			pAftkScene);

		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new point light." AFTK_DO_ERROR_END_1;
			return 0;
		}
	
		status = C_SceneCustomLightImport::Import(
			this, 
			transformDagPath,
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);

		if (!status) return 0;
	}
	// Check for actor nodes
	else if (typeId.id() == MAYA_IO_ACTOR_NODE_ID) {

		pAftkNewSceneNode = new Aftk::C_SceneNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new actor." AFTK_DO_ERROR_END_1;
			return 0;
		}

		status = C_SceneCustomActorImport::Import(
			this, 
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);

	}
	//Switch on node type
	else if (transformDagPath.hasFn(MFn::kAreaLight))
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "AftkImport::ImportShape(): Area Light/Point Light instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		pAftkNewSceneNode = new Aftk::C_SceneAreaLightNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new area light." AFTK_DO_ERROR_END_1;
			return 0;
		}

		status = C_SceneAreaLightImport::Import(
			this, 
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
		if (!status) return 0;
	} 
	else if (transformDagPath.hasFn(MFn::kPointLight))
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "Point Light/Point Light instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		pAftkNewSceneNode = new Aftk::C_ScenePointLightNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new point light." AFTK_DO_ERROR_END_1;
			return 0;
		}

		status = C_ScenePointLightImport::Import(		
			this, 
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
		if (!status) return 0;
	} 
	else if (transformDagPath.hasFn(MFn::kDirectionalLight) )
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "Directional Light/Directional Light instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		pAftkNewSceneNode = new Aftk::C_SceneDirectionalLightNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for directional light." AFTK_DO_ERROR_END_1
			return 0;
		}

		status = C_SceneDirectionalLightImport::Import(
			this, 
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
		if (!status) return 0;
	} 
	else if (transformDagPath.hasFn(MFn::kSpotLight))
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "Spot Light/Spot Light instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		pAftkNewSceneNode = new Aftk::C_SceneSpotLightNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for spot light." AFTK_DO_ERROR_END_1
			return 0;
		}
		status = SceneSpotLightImport::Import(
			this, 
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
		if (!status) return 0;
	} 
	else if (transformDagPath.hasFn(MFn::kCurve)) 
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "Curve/Curve instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		//Create a new scene node
		pAftkNewSceneNode = new Aftk::C_SceneSplineNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for spline." AFTK_DO_ERROR_END_1;
			return 0;
		}

		status = C_SceneSplineImport::Import(	
			this, 
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
		if (!status) return 0;
	} 
	else if (transformDagPath.hasFn(MFn::kMesh))
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "Mesh/Mesh instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		MObject speedTreeNode;
		if (IsBonedInstance(fnTransform))
		{
			//Attach a new boned instance to our scene.
			pAftkNewSceneNode = new Aftk::C_SceneMeshBonedInstanceNode(pAftkScene);
			if (!pAftkNewSceneNode)
			{
				AFTK_DO_BEGIN << TRANS_NEW_BONED_INSTANCE_OUT_OF_MEMORY AFTK_DO_ERROR_END_1;
				return 0;
			}
			
			//The contents of the new boned instance will actually be imported AFTER the remainder of the
			//scene graph.
		}
		else if (HasSpeedTreeNode(mFnShapeDagNode, speedTreeNode))
		{
			pAftkNewSceneNode = new Aftk::C_SceneNode(pAftkScene);
			if (!pAftkNewSceneNode)
			{
				AFTK_DO_BEGIN << TRANS_NEW_SPEED_TREE_OUT_OF_MEMORY AFTK_DO_ERROR_END_1;
				return 0;
			}

			status = C_SceneSpeedTreeImport::Import(	
				this,
				transformDagPath,
				speedTreeNode,
				pExclusiveMatrix,
				pAftkScene, 
				pAftkNewSceneNode,
				&m_options);
			if (!status) return 0;
		}
		else
		{
			//Create a new scene node
			pAftkNewSceneNode = new Aftk::C_SceneMeshNode(pAftkScene);
			if (!pAftkNewSceneNode)
			{
				AFTK_DO_BEGIN << "Failed to allocate memory for new mesh." AFTK_DO_ERROR_END_1;
				return 0;
			}

			status = C_SceneMeshImport::Import(
				this,
				transformDagPath, 
				pExclusiveMatrix,
				pAftkScene, 
				pAftkNewSceneNode,
				&m_options);
			if (!status) return 0;
		}
	}
	else if (transformDagPath.hasFn(MFn::kCamera))
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "AftkImport::ImportShape(): Camera/Camera instance " << name << " encountered. Resolving...\n" AFTK_DO_DEBUG_END;
		#endif

		//Create a new scene node
		pAftkNewSceneNode = new Aftk::C_SceneCameraNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new camera." AFTK_DO_ERROR_END_1
			return 0;
		}

		status = C_SceneCameraImport::Import(
			this,
			transformDagPath, 
			pExclusiveMatrix,
			pAftkScene, 
			pAftkNewSceneNode,
			&m_options);
		if (!status) return 0;
	}
	else 
	{
		#ifdef DEBUG_PRINT
		AFTK_DO_BEGIN << "AftkImport::ImportShape(): Unknown node type " << name << " encountered!\n" AFTK_DO_DEBUG_END;
		#endif

		// Create a new 0 node
		pAftkNewSceneNode = new Aftk::C_SceneNode(pAftkScene);
		if (!pAftkNewSceneNode)
		{
			AFTK_DO_BEGIN << "Failed to allocate memory for new null node." AFTK_DO_ERROR_END_1
			return 0;
		}
		
		status = C_SceneNodeImport::Import(	
			this,
			transformDagPath,
			pExclusiveMatrix,
			pAftkScene,
			pAftkNewSceneNode,
			true,
			&m_options);
		if (!status) return 0;
	}

	return pAftkNewSceneNode;
}

Aftk::C_Status C_Importer::GetLayerIndices(
    const MFnDagNode& dagNodeFn,
	bool& gameLayerMember,
	MIntArray& layerIndices)
{

	MStatus mStatus;

	MPlug layerConnectPlug = dagNodeFn.findPlug("layerConnect", &mStatus);
	if (!mStatus) {
		// No game layer connection found
		gameLayerMember = false;
		return Aftk::C_Status::E_Status_Success;
	}

	MPlugArray connectedPlugs;
	bool asSrc = true;
	bool asDst = false;
	bool connected = layerConnectPlug.connectedTo( connectedPlugs, asDst, asSrc, &mStatus);
	MCHECK_BEGIN << "Cannot get connected plugs" MCHECK_ERROR_RETURN_END_1

	if (connected) gameLayerMember = true;
	else		   gameLayerMember = false;

	for (U32 i = 0; i < connectedPlugs.length(); i ++) {

		MPlug currentPlug = connectedPlugs[i];
		if (currentPlug.isElement()) {
			
			MPlug arrayPlug = currentPlug.array();
			if (arrayPlug.isChild()) {

				MPlug layersPlug = arrayPlug.parent();
				layerIndices.append( layersPlug.logicalIndex() );

			}
		}
	}

    return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::GetGameLayerName(
	int layerIndex,
	MString &layerName)
{

	MStatus mStatus;
	MObject gameLayerDatabase;

	MStringArray resultArray;
	MGlobal::executeCommand("ls -type gameLayerDatabase", resultArray);

	MSelectionList list;
	mStatus = MGlobal::getSelectionListByName(resultArray[0], list);
	MCHECK_BEGIN << "Cannot get selection list" MCHECK_ERROR_RETURN_END_1;

	MItSelectionList iter(list);
	mStatus = iter.getDependNode(gameLayerDatabase);
	MCHECK_BEGIN << "Cannot get depend node" MCHECK_ERROR_RETURN_END_1;

	MFnDependencyNode databaseFn(gameLayerDatabase);

	MPlug layersPlug = databaseFn.findPlug("layers", &mStatus);
	MCHECK_BEGIN << "Cannot find plug" MCHECK_ERROR_RETURN_END_1;

	MPlug activeLayerPlug = layersPlug.elementByPhysicalIndex(layerIndex, &mStatus);
	MCHECK_BEGIN << "Cannot get element plug at index" MCHECK_ERROR_RETURN_END_1;

	MPlug layerNamePlug = activeLayerPlug.child(3, &mStatus);
	MCHECK_BEGIN << "Cannot get child plug" MCHECK_ERROR_RETURN_END_1;

	// Because the MPlug function .getValue causes a failure (Still don't know
	// why) I've put a mel call to getAttr here to get the name of the current
	// layer....seems to work ok :)

	MString cmdString =  "getAttr ";
			cmdString += layerNamePlug.name();

	std::string j = cmdString.asChar();

	MCommandResult result;
	mStatus = MGlobal::executeCommand(cmdString, result, true, false);

	//MCommandResult::Type r = result.resultType();

	mStatus = result.getResult(layerName);
	MCHECK_BEGIN << "Cannot get layer Name" MCHECK_ERROR_RETURN_END_1;

	//std::string f = layerName.asChar();

	return Aftk::C_Status::E_Status_Success;

}

Aftk::C_Status C_Importer::GetLayerName(
	const MFnDagNode& shapeDagNode, 
	bool& layerMember,
	MString& layerName)
{
	Aftk::C_Status status;
	MStatus mStatus;

	//If this node is on a layer then we have some additional processing to do.
	//We use a map to track the names of the layers we encounter.
	//When we encounter a new name we add that layer, and then add this node.
	//If the layer already exists we simply add this node to it.

	MPlug layerPlug = shapeDagNode.findPlug("drawOverride", &mStatus);
	MCHECK_BEGIN << "[" << shapeDagNode.name().asChar() << "] Could not find layer drawOverride plug." MCHECK_ERROR_RETURN_END_1

	if (!layerPlug.isNull()) 
	{
		MPlugArray connectedPlugs;
		bool asSrc = false;
		bool asDst = true;
		layerPlug.connectedTo( connectedPlugs, asDst, asSrc );

		//Okay, now according to Alias we should now check the first plug in this array
		//in order to determine whether it is our layer.

		if (connectedPlugs.length() == 0)
		{
			#ifdef DEBUG_PRINT
			g_tcDebugMan << "This node is not a member of any layer.\n";
			#endif
			layerMember = false;
			return Aftk::C_Status::E_Status_Success;
		}

		MObject nodeObject = connectedPlugs[0].node();
		//Is this object a display layer?
		if (nodeObject.hasFn(MFn::kDisplayLayer))
		{
			//Wohoo, the given DAG node was attached to a layer after all.
			MFnDependencyNode dgNode(nodeObject);

			layerName = dgNode.name();
			layerMember = true;
			return Aftk::C_Status::E_Status_Success;
		}
	}

	layerMember = false;
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::AddToMayaLayerIfNeeded(
	const MFnDagNode& fnTransform, 
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;

	bool	layerMember;
	MString layerName;

	status = GetLayerName(
		fnTransform,
		layerMember,
		layerName);
	AFTK_CHECK_ERROR_RETURN;

	if (layerMember)
	{
		//Wohoo, the given DAG node was attached to a layer after all.
		MFnDependencyNode dgNode(fnTransform);

		std::string layerNameStd = layerName.asChar();

		//This is the layer we intend to add our scene node to.
		Aftk::C_LayerNode* pAftkLayer;

		//Okay, now attempt to add this layer name to our layer map
		//Does this layer already exist?
		Ut::UtMap<std::string, Aftk::C_LayerNode*>::Iterator it;
		it = m_layerMap.Find(layerNameStd);

		if (it == m_layerMap.End())
		{
			//We need to add this layer
			pAftkLayer = new Aftk::C_LayerNode(pAftkSceneNode->GetScenePtr());
			
			//Set the layer's name.
			pAftkLayer->SetName(layerNameStd);

			//Add this layer to our map.
			m_layerMap[layerNameStd] = pAftkLayer;
		}
		else
		{
			//We need to add this scene node to an existing layer.
			pAftkLayer = m_layerMap[layerNameStd];
		}

		if (pAftkLayer == 0)
		{
			//What the...? For some reason the AFTK was unable to find the layer
			//we tried to fetch.
			AFTK_DO_BEGIN << "Unable to find layer in AFTK." AFTK_DO_ERROR_RETURN_END_1
		}

		//////ADD THE SCENE NODE TO THE LAYER//////
		pAftkLayer->AddMember(pAftkSceneNode);
	}
	
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::AddToGameLayerIfNeeded(
	const MFnDagNode& fnTransform, 
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;

	bool	  layerMember;
	MIntArray layerIndices;

	status = GetLayerIndices(
		fnTransform,
		layerMember,
		layerIndices);
	CHECK_ERROR_RETURN;

	if (layerMember)
	{
        MFnDependencyNode nodeFn(fnTransform);

		for (U32 i = 0; i < layerIndices.length(); i++) {

			//This is the layer we intend to add our scene node to.
			Aftk::C_LayerNode* pAftkLayer;

			// Get the unique name of the current layer
			MString layerName;
			status = GetGameLayerName(layerIndices[i], layerName);
			std::string layerNameStd = layerName.asChar();

			//Okay, now attempt to add this layer name to our layer map
			//Does this layer already exist?
			Ut::UtMap<std::string, Aftk::C_LayerNode*>::Iterator it;
			it = m_layerMap.Find(layerNameStd);

			if (it == m_layerMap.End())
			{
				//We need to add this layer
				pAftkLayer = new Aftk::C_LayerNode(pAftkSceneNode->GetScenePtr());
				
				//Set the layer's name.
				pAftkLayer->SetName(layerNameStd);

				//Add this layer to our map.
				m_layerMap[layerNameStd] = pAftkLayer;
			}
			else
			{
				//We need to add this scene node to an existing layer.
				pAftkLayer = m_layerMap[layerNameStd];
			}

			if (pAftkLayer == 0)
			{
				//What the...? For some reason the AFTK was unable to find the layer
				//we tried to fetch.
				AFTK_DO_BEGIN << "Unable to find layer in AFTK." AFTK_DO_ERROR_RETURN_END_1;
			}

			//////ADD THE SCENE NODE TO THE LAYER//////
			pAftkLayer->AddMember(pAftkSceneNode);

		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::AddToLayerIfNeeded(
	const MFnDagNode& fnTransform,
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;

	MCommandResult commandResult;
	MGlobal::executeCommand("isTrue gameLayerDatabaseExists", commandResult);

	int gameLayerExists;
	commandResult.getResult(gameLayerExists);

	if (gameLayerExists)
		return AddToGameLayerIfNeeded(fnTransform, pAftkSceneNode);

	return AddToMayaLayerIfNeeded(fnTransform, pAftkSceneNode);
}

Aftk::C_SceneNode* C_Importer::ResolveAndImportShapeOrShapeInstance(
	const MDagPath& transformDagPath, 
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_Status& status)
{
	MStatus mStatus;

	MDagPath shapeDagPath(transformDagPath);
	shapeDagPath.extendToShape();
	MFnDagNode shapeNode(shapeDagPath, &mStatus);
	if (!mStatus)
	{
		AFTK_DO_BEGIN << "Failed to get shape node." AFTK_DO_ERROR_END_1;
		return 0;
	}

	// Perform shape import
	if (shapeNode.isInstanced())
	{
		bool foundInstance = false;
		U32 instanceIndex;
		for (instanceIndex = 0; instanceIndex < (U32)m_instanceTracker.Size(); instanceIndex++)
		{
			if (m_instanceTracker[instanceIndex].m_dagPath.node() == shapeDagPath.node()) 
			{
				// decrease the number of references to this node that we have yet to process
				m_instanceTracker[instanceIndex].m_remainingInstances--;
				if (m_instanceTracker[instanceIndex].m_remainingInstances == 0) 
				{
					m_instanceTracker.EraseByIndex(instanceIndex);
				}
				foundInstance = true;
				break;
			}
		}
		if (!foundInstance) 
		{
			C_InstanceTracker newInstance;
			newInstance.m_dagPath = shapeDagPath;
			// Get the number of direct and indirect instances of this node
			newInstance.m_remainingInstances = shapeNode.instanceCount(true);
			Aftk::C_SceneNode* pAftkNewSceneNode = 
				ImportTransformWithShape(
				transformDagPath,
				pExclusiveMatrix,
				pAftkScene, 
				status);

			if (!status || !pAftkNewSceneNode)
			{
				return 0;
			}
			newInstance.m_pAftkSceneNode = pAftkNewSceneNode;
			m_instanceTracker.PushBack(newInstance);

			status = Aftk::C_Status::E_Status_Success;
			return pAftkNewSceneNode;
		}
		else
		{
			// This shape is an instance, perform appropriate import
			C_InstanceTracker* pInstance = &m_instanceTracker[instanceIndex];
			Aftk::C_SceneNode* pAftkSourceNode = pInstance->m_pAftkSceneNode;

			Aftk::C_SceneNode* pAftkNewSceneNode = new Aftk::C_SceneInstanceNode(pAftkScene);
			if (!pAftkNewSceneNode)
			{
				AFTK_DO_BEGIN << "Failed to allocate memory for new instance." AFTK_DO_ERROR_END_1;
				return 0;
			}

			if (!C_SceneInstanceImport::Import(
				this,
				transformDagPath, 
				pExclusiveMatrix,
				pAftkScene, 
				pAftkNewSceneNode,
				&m_options, 
				pAftkSourceNode))
			{
				AFTK_DO_BEGIN << "Could not import instance" AFTK_DO_ERROR_END_1;
				return 0;
			} 

			status = Aftk::C_Status::E_Status_Success;
			return pAftkNewSceneNode;
		}
	}
	
	//If we get here then this shape is not instanced
	Aftk::C_SceneNode* pAftkNewSceneNode = 
		ImportTransformWithShape(
		transformDagPath, 
		pExclusiveMatrix,
		pAftkScene, 
		status);
	if (!status || pAftkNewSceneNode == 0)
	{
		if (status)	
		{
			AFTK_DO_BEGIN << "Failed to import transform with shape." AFTK_DO_ERROR_END_1;
		}
		return 0;
	} 
	status = Aftk::C_Status::E_Status_Success;
	return pAftkNewSceneNode;
}

Aftk::C_Status C_Importer::AddToImportedMaterialMap(
	const Aftk::C_SharedResource<Aftk::C_MaterialNode>* pMaterialResource)
{
	Ut::UtMap<std::string, const Aftk::C_SharedResource<Aftk::C_MaterialNode>*>::Iterator it;

	std::string name = pMaterialResource->GetContent().GetName();

	it = m_materialMap.Find(name);

    if (it == m_materialMap.End())
	{
		//Failed to find a material with the given name
		m_materialMap[name] = pMaterialResource;
	}
	else
	{
		//We already have an instance of the given material
		AFTK_DO_BEGIN << "Found an existing material - this should not happen!" AFTK_DO_WARNING_END;
	}

	return Aftk::C_Status::E_Status_Success;
}

const Aftk::C_SharedResource<Aftk::C_MaterialNode>* C_Importer::GetImportedMaterialByName(
	const std::string& name)
{
	Ut::UtMap<std::string, const Aftk::C_SharedResource<Aftk::C_MaterialNode>*>::Iterator it;
	it = m_materialMap.Find(name);

    if (it == m_materialMap.End())
	{
		//Failed to find a material with the given name
		return 0;
	}
	else
	{
		//We already have an instance of the given material
		return it->GetValue();
	}
}

bool C_Importer::IsVisible(MFnDagNode &fnDag, Aftk::C_Status& status) 
{
	MStatus mStatus;
	if(fnDag.isIntermediateObject())
	{
		return false;
	}

	MPlug visPlug = fnDag.findPlug("visibility", &mStatus);
	if (!status)
	{
		AFTK_DO_BEGIN << "MPlug::findPlug." AFTK_DO_ERROR_END_1;
		return false;
	} 
	else
	{
		bool visible;
		mStatus = visPlug.getValue(visible);
		if (mStatus)
		{
			AFTK_DO_BEGIN << "MPlug::getValue" AFTK_DO_ERROR_END_1;
			return false;
		}

		return visible;
	}
}

bool C_Importer::IsActor(const MDagPath& transformDagPath) 
{
	MStatus mStatus;

	MDagPath shapeDagPath = transformDagPath;
	mStatus = shapeDagPath.extendToShape();
	if (!mStatus)
		return false;

	MFnDagNode mFnShapeDagNode(shapeDagPath);
	MTypeId typeId = mFnShapeDagNode.typeId(&mStatus);
	if (!mStatus)
		return false;

	if (typeId.id() == MAYA_IO_ACTOR_NODE_ID)
		return true;
	else
		return false;

}

const C_MaterialImportFilter& C_Importer::GetMaterialFilter() const
{
	return m_materialFilter;
}

const C_PlugImportFilter& C_Importer::GetPlugFilter() const
{
	return m_plugFilter;
}

Aftk::C_Status C_Importer::ImportSkinClusters()
{
	Aftk::C_Status status;
	Aftk::C_SceneNodeIterator it(m_pAftkScene);

	while (it)
	{
		Aftk::C_SceneNode* pNode = it;
		if (pNode->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			Aftk::I_SceneMeshNode* pMesh = (Aftk::I_SceneMeshNode*)pNode;
			MDagPath mayaDagPath;
			if (!m_aftkMayaMapping.GetMayaDagPathFromAftkSceneNode(pMesh, &mayaDagPath))
			{
				AFTK_DO_BEGIN << "Could not find mesh for skin cluster import." AFTK_DO_ERROR_END_1;
			}
			status = ImportSkinCluster(mayaDagPath, pMesh);
			if (!status) return status;
		}
		it++;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::GetClusterMembership(MObject &obCluster, MSelectionList &obMembers)
{
	Aftk::C_Status status;
	MStatus mStatus;

	obMembers.clear();

	MItDependencyGraph itGraph( obCluster, MFn::kSet );

	if (itGraph.isDone())
	{
		AFTK_DO_BEGIN << "No set available for GetClusterMembership." AFTK_DO_ERROR_RETURN_END_1;
	}

	MFnSet fnSet(itGraph.thisNode(), &mStatus);
	MCHECK_BEGIN << "Could no acquire function set from graph node." MCHECK_ERROR_RETURN_END_1;

	// Get membership of set.
	mStatus = fnSet.getMembers(obMembers, true);
	MCHECK_BEGIN << "Could not get set members." MCHECK_ERROR_RETURN_END_1;

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::ImportSkinCluster(
	const MDagPath& mayaDagPath,
	Aftk::I_SceneMeshNode* pAftkMeshNode)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MFnMesh m_mayaMesh(mayaDagPath, &mStatus);
	if (!mStatus)
	{
		//If we can't create a function set for this mesh then it should have already been flagged as debris
		//and will be deleted
		return Aftk::C_Status::E_Status_Success;
	}
//	MCHECK_BEGIN << "Could not create mesh function set for dag path " << mayaDagPath.fullPathName().asChar() << "]" MCHECK_ERROR_RETURN_END_1;

	MItDependencyGraph dgIt(m_mayaMesh.object(),
		MFn::kInvalid,
		MItDependencyGraph::kUpstream, 
		MItDependencyGraph::kBreadthFirst,
		MItDependencyGraph::kNodeLevel, 
		&mStatus);
	MCHECK_BEGIN << "Could not create dependency graph iterator" MCHECK_ERROR_RETURN_END_1;

	// Disable dependency graph node filtering.
	dgIt.disablePruningOnFilter();

	//Iterate thru shader nodes
	while (!dgIt.isDone())
	{
		MObject dgNodeObject = dgIt.thisNode();

		if (dgNodeObject.apiType() == MFn::kSkinClusterFilter)
		{
			// For each skinCluster node, get the list of influence objects
			//
			MFnSkinCluster skinCluster(dgNodeObject);
			#ifdef _DEBUG
			AFTK_DO_BEGIN << "SKIN CLUSTER NAME " << skinCluster.name().asChar() AFTK_DO_DEBUG_END;
			#endif
			MDagPathArray influenceObjects;
			U32 numInfluenceObj = skinCluster.influenceObjects(influenceObjects, &mStatus);
			MCHECK_BEGIN << "Error getting influence objects." MCHECK_ERROR_RETURN_END_1

			#ifdef _DEBUG
			AFTK_DO_BEGIN << " influence count [" << numInfluenceObj << "]\n" AFTK_DO_DEBUG_END;
			#endif

			// Cache Cluster membership as MSelectionList
			// for rejection of invalid weight results below.
			MSelectionList obMembers;
			GetClusterMembership( dgNodeObject, obMembers );

			if (numInfluenceObj == 0)
			{
				AFTK_CHECK_BEGIN << "No influence objects found." AFTK_CHECK_ERROR_RETURN_END_1;
			}

			// Create an AftkSkinCluster to fill.
			Aftk::C_InfluenceClusterNode* pAftkSkinCluster = new Aftk::C_InfluenceClusterNode(m_pAftkScene);
			if (!pAftkSkinCluster)
			{
				AFTK_DO_BEGIN << "Failed to create AFTK influence cluster node." AFTK_DO_ERROR_RETURN_END_1;
			}

			//IMPORTANT - indicate the purpose of the new generic influence cluster.
			pAftkSkinCluster->SetUsage(AFTK_INFLUENCE_CLUSTER_USAGE_SKIN);

			//Add to the cluster's mesh list
			pAftkSkinCluster->SetSceneNodePtr(pAftkMeshNode);

			// Create one envelope per influence object
			pAftkSkinCluster->SetEnvelopeInitialCapacity(numInfluenceObj);
			for (U32 i = 0; i < numInfluenceObj; i++)
			{
				bool doWeImportThisNode;
				bool doWeImportItsChildren;
				DoWeImportThisNode(influenceObjects[i], doWeImportThisNode, doWeImportItsChildren, status);
				AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END_1;

				U32 envelopeIndex = pAftkSkinCluster->AddEnvelope();
				BIZ_ASSERT(i == envelopeIndex);

				if (doWeImportThisNode)
				{
					Aftk::C_WeightEnvelope* pAftkEnvelope = &pAftkSkinCluster->GetEnvelope(i);
					//Set the scene node that influences this envelope
					Aftk::C_SceneNode* pAftkSceneNode;
					if (!m_aftkMayaMapping.GetAftkSceneNodeFromMayaDagPath(influenceObjects[i], &pAftkSceneNode))
					{
						AFTK_DO_BEGIN << "Failed to map from dag path [" << influenceObjects[i].fullPathName().asChar() << "]-> AFTK object. No AFTK object created for search target node." AFTK_DO_ERROR_RETURN_END_1;
					}
					pAftkEnvelope->SetInfluenceNodePtr(pAftkSceneNode);
				}
			}
            			
			// loop through the geometries affected by this cluster
			U32 nGeoms = skinCluster.numOutputConnections();
			//Yes this is awful, I know. Needs a better solution.
			std::set<std::string> warningSet;
			for (U32 i = 0; i < nGeoms; i++) 
			{
				U32 index = skinCluster.indexForOutputConnection(i, &mStatus);
				MCHECK_BEGIN << "Error getting geometry index." MCHECK_ERROR_RETURN_END_1;

				// get the dag path of the i'th geometry
				MDagPath skinPath;
				mStatus = skinCluster.getPathAtIndex(index,skinPath);
				MCHECK_BEGIN << "Error getting geometry path." MCHECK_ERROR_RETURN_END_1;

				#ifdef _DEBUG
				AFTK_DO_BEGIN << "Skin geometry path: " << skinPath.fullPathName().asChar() << "\n" AFTK_DO_DEBUG_END;
				#endif

				// iterate through the components of this geometry
				MItGeometry gIter(skinPath);

				for ( ; !gIter.isDone(); gIter.next()) 
				{
					MObject comp = gIter.component(&mStatus);
					MCHECK_BEGIN << "Error getting component." MCHECK_ERROR_RETURN_END_1;

					// Get the weights for this vertex (one per influence object)
					MFloatArray wts;
					U32 infCount;
					mStatus = skinCluster.getWeights(skinPath,comp,wts,infCount);
					MCHECK_BEGIN << "Error getting weights." MCHECK_ERROR_RETURN_END_1;
					if (0 == infCount) 
					{
						AFTK_DO_BEGIN << "Error: 0 influence objects." AFTK_DO_ERROR_RETURN_END_1;
					}

					//We assume that there's a vertex weight entry for each influence object of this
					//skin cluster
					AFTK_ASSERT(wts.length() == numInfluenceObj);

					// MFnGeometryFilter::getWeights() returns a weight of 1.0 if
					// a component is NOT a member of the Cluster!  Use membership of
					// MSelectionList to reject invalid weights.
					if (!obMembers.hasItem(skinPath, comp))
					{
						// The weighting reported for this component does not represent this Cluster.
						continue;
					}

					for (U32 influenceIndex = 0; influenceIndex < wts.length(); influenceIndex++)
					{
						if (influenceIndex >= pAftkSkinCluster->GetNumEnvelopes())
						{
							std::stringstream ss;
							ss << "Influence index [" << influenceIndex << "] >= than skin cluster envelope count [" << pAftkSkinCluster->GetNumEnvelopes() << "] for node [" << pAftkMeshNode->GetName() << "]\n";
							warningSet.insert(ss.str());
							continue;
						}

						Aftk::C_WeightEnvelope* pAftkSkinEnvelope = &pAftkSkinCluster->GetEnvelope(influenceIndex);

						//If our weight is > EPSILON then add it to our weight array
						if (wts[influenceIndex] > 1e-10f)
						{
							Aftk::C_Weight<F32> aftkVertexWeight;
							aftkVertexWeight.SetIndex(gIter.index());
							aftkVertexWeight.SetWeight(wts[influenceIndex]);
							
							pAftkSkinEnvelope->AddWeight(aftkVertexWeight);

							#ifdef DEBUG_PRINT
							AFTK_DO_BEGIN << "Envelope [" << influenceIndex << "] vertex index [" << gIter.index() << "] vertex weight [" << wts[influenceIndex] << "]\n" AFTK_DO_DEBUG_END;
							#endif
						}
					}
				}
			}

			for (std::set<std::string>::iterator it = warningSet.begin(); it != warningSet.end(); ++it)
			{
				AFTK_DO_BEGIN << (*it) AFTK_DO_WARNING_END;
			}
		}

		dgIt.next();
	}

	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_Importer::ImportDg()
{
	Aftk::C_Status status;
	MStatus mStatus;

	//Import scene level animation information

	//Get start- and end-frame from Maya
	MTime oneSecond( 1, MTime::kSeconds );
	F32 frameRate = (F32)oneSecond.as(MTime::uiUnit());
	m_pAftkScene->SetFrameRate((F32)frameRate);

	F32 startTime = (F32)MAnimControl::minTime().as(MTime::kSeconds);
	F32 endTime = (F32)MAnimControl::maxTime().as(MTime::kSeconds);
	m_pAftkScene->SetStartTime(startTime * frameRate);
	m_pAftkScene->SetEndTime(endTime * frameRate);

	//Import animated hierarchy nodes...
	for (Aftk::C_SceneNodeIterator it(m_pAftkScene); it; ++it)
	{
		MDagPath thisPath;
		Aftk::C_SceneNode* pAftkSceneNode = it;

		if (!m_aftkMayaMapping.GetMayaDagPathFromAftkSceneNode(pAftkSceneNode, &thisPath))
			continue;

		MDagPath thisShape(thisPath);
		mStatus = thisShape.extendToShape();

		MDagPath* pShape;
		if (mStatus)
			pShape = &thisShape;
		else
			pShape = 0;

		status = C_SceneNodeImport::ImportConnectivity(	
			this,
			thisPath,
			pShape,
			pAftkSceneNode,
			&m_options);
		AFTK_CHECK_ERROR_RETURN;
	}
		
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Scene* C_Importer::GetAftkScenePtr()
{
	return (Aftk::C_Scene*)m_pAftkScene;
}

const Aftk::C_Scene* C_Importer::GetAftkScenePtr() const
{
	return (const Aftk::C_Scene*)m_pAftkScene;
}

Aftk::C_Status C_Importer::ImportBonedInstances()
{
	Aftk::C_Status status;

	Aftk::C_SceneNodeIterator it(m_pAftkScene);
	while (it)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;
		if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
		{
			MDagPath transformDagPath;
			if (m_aftkMayaMapping.GetMayaDagPathFromAftkSceneNode(pAftkSceneNode, &transformDagPath))
			{
				//If we've got a Maya version of this Aftk scene node.
				status = SceneMeshBonedInstanceImport::Import(
					this,
					transformDagPath,
					m_pAftkScene, 
					pAftkSceneNode,
					&m_options);
				AFTK_CHECK_ERROR_RETURN;
			}
		}
		it++;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::ReplaceDebris()
{
	Aftk::C_Status status;

	for (U32 i = 0; i < (U32)m_debrisList.Size(); i++)
	{
		Aftk::C_SceneNode* pDebrisNode = m_debrisList[i];
		//Remove from scene mapping.
		m_aftkMayaMapping.DeleteAftkMayaSceneNodeMapping(pDebrisNode);
		//Create a new null node from the 'scene node' fragment of the debris.
		Aftk::C_SceneNode* pNullNode = new Aftk::C_SceneNode(*pDebrisNode); 
		//Replace the debris in the hierarchy.
		pNullNode->Replace(pDebrisNode);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::CheckForInvertedMatrices()
{
	Aftk::C_Status status;
	bool failed = false;
	Aftk::C_SceneNodeIterator it(m_pAftkScene);
	while (it)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;
		if (pAftkSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		{
			
			Aftk::C_SceneMeshRemappedInstanceNode* pInstance = (Aftk::C_SceneMeshRemappedInstanceNode*)pAftkSceneNode;
			if (pInstance->GetSourceNodePtr())
			{
				if (Aftk::DoesMatrixHaveNegativeScaling(pInstance->GetWorldMatrix()) != 
					Aftk::DoesMatrixHaveNegativeScaling(pInstance->GetSourceNodePtr()->GetWorldMatrix()))
				{
					if (m_options.strict)
					{
						AFTK_DO_BEGIN << TRANS_INVERTED_MATRIX_FAIL AFTK_DO_ERROR_END_0;
						failed = true;
					}
					else
					{
						AFTK_DO_BEGIN << TRANS_INVERTED_MATRIX_WARN AFTK_DO_WARNING_END;
					}
				}
			}
		}
		else if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			Aftk::C_SceneInstanceNode* pInstance = (Aftk::C_SceneInstanceNode*)pAftkSceneNode;
			if (pInstance->GetSourceNodePtr())
			{
				if (Aftk::DoesMatrixHaveNegativeScaling(pInstance->GetWorldMatrix()) != 
					Aftk::DoesMatrixHaveNegativeScaling(pInstance->GetSourceNodePtr()->GetWorldMatrix()))
				{
					if (m_options.strict)
					{
						AFTK_DO_BEGIN << TRANS_INVERTED_MATRIX_FAIL AFTK_DO_ERROR_END_0;
						failed = true;
					}
					else
					{
						AFTK_DO_BEGIN << TRANS_INVERTED_MATRIX_WARN AFTK_DO_WARNING_END;
					}
				}
				}
		}
		it++;
	}

	if (failed)
		AFTK_DO_BEGIN << "Inverted matrix check failed." AFTK_DO_ERROR_RETURN_END_1;

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::GetParameters(C_ImportOptions& options, const std::string& _arguments)
{
	std::string arguments(_arguments);
	Tu::SearchAndReplace(arguments, "|", ";");
	if (arguments.size() > 0)
	{
		char delimiter[] = ";";
		char* token;
		
		//Get the first token:
		char* buffer = new char[arguments.length()+1];
		strcpy(buffer, arguments.c_str());
		token = strtok(buffer, delimiter);
		while( token != NULL )
		{
			size_t tokenLength = strlen(token);
			if (tokenLength > 1)
			{
				std::string tokenStd(token);
				if (tokenStd.find("exportSelection=") != std::string::npos)
				{
					if (!GetOptionValueBool(token, options.importSelection))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
				if (tokenStd.find("project=") != std::string::npos)
				{
					if (!GetOptionValueString(token, options.project))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
				else if (tokenStd.find("triangulate=") != std::string::npos)
				{
					if (!GetOptionValueBool(token, options.triangulate))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
				else if (tokenStd.find("tri=") != std::string::npos)
				{
					if (!GetOptionValueBool(token, options.triangulate))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
				else if (tokenStd.find("exporttangentspace=") != std::string::npos)
				{
					if (!GetOptionValueBool(token, options.importTangentSpace))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
				else if (tokenStd.find("mergelodmeshes=") != std::string::npos)
				{
					if (!GetOptionValueBool(token, options.mergeLodMeshes))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
				else if (tokenStd.find("validatematerials=") != std::string::npos)
				{
					if (!GetOptionValueBool(token, options.validateMaterials))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
				else if (tokenStd.find("deformedinstancesasmesh=") != std::string::npos)
				{
					if (!GetOptionValueBool(token, options.deformedInstancesAsMesh))
							return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
				else if (tokenStd.find("strict=") != std::string::npos)
				{
					if (!GetOptionValueBool(token, options.strict))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
				else if (tokenStd.find("absNumericRange=") != std::string::npos)
				{
					if (!GetOptionValueF32(token, options.absNumericRange))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
				else if (tokenStd.find("meshweld=") != std::string::npos)
				{
					if (!GetOptionValueBool(token, options.meshWeld))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
				else if (tokenStd.find("zflip=") != std::string::npos)
				{
					if (!GetOptionValueBool(token, options.zflip))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse open task argument.");
				}
			}

			//Get next token:
			token = strtok(NULL, delimiter);
		}
		delete[] buffer;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Importer::BuildSetInfo()
{
	Aftk::C_Status status;
	MStatus mStatus;

	m_polySetArray.Clear();
	m_polySetCombinationTable.Clear();

	// create an iterator to go through all nodes
	for (MItDependencyNodes it(MFn::kInvalid); !it.isDone(); it.next())
	{
		//Ignore everything but sets.
		if (!it.item().hasFn(MFn::kSet))
			continue;

		MFnSet fnSet(it.thisNode(), &mStatus);
		MCHECK_BEGIN << "Could no acquire function set from graph node." MCHECK_ERROR_RETURN_END_1;

		//Store a list of sets which are restricted to operating on faces only.
		if (fnSet.restriction() == MFnSet::kFacetsOnly)
		{
			m_polySetArray.PushBack(fnSet.object());
			#ifdef _DEBUG
			AFTK_DO_BEGIN << m_polySetArray.Size()-1 << " : Add poly set [" << fnSet.name().asChar() << "].\n" AFTK_DO_DEBUG_END;
			#endif
		}
	}
	AFTK_DO_BEGIN << "Found " << m_polySetArray.Size() << " poly sets.\n" AFTK_DO_DEBUG_END;

	return Aftk::C_Status::E_Status_Success;
}

const Ut::UtVector<Ut::DynamicVector<MObject> >& 
	C_Importer::GetPolySetArray() const
{
	return m_polySetArray;
}

const Ut::OpenHashMap<Ut::UtSet<U32>, const Aftk::C_BlindDataResource*, JenkinsHashKeyOpSetU32 >& 
	C_Importer::GetPolySetCombinationTable() const
{
	return m_polySetCombinationTable;
}

const Aftk::C_BlindDataResource* C_Importer::GetPolySetCombinationBlindData(
	Ut::UtSet<U32>& polySetCombinationIndices)
{
	if (polySetCombinationIndices.Size() == 0)
		return 0;

	MStatus mStatus;
	Ut::OpenHashMap<Ut::UtSet<U32>, const Aftk::C_BlindDataResource*, JenkinsHashKeyOpSetU32>::Iterator it;

	it = m_polySetCombinationTable.Find(polySetCombinationIndices);
	if (it == m_polySetCombinationTable.End())
	{
		Ut::OpenHashMap<Ut::UtSet<U32>, const Aftk::C_BlindDataResource*, JenkinsHashKeyOpSetU32 >::InsertionReturnType result =
		m_polySetCombinationTable.Insert(polySetCombinationIndices);
		//Update hash-table iterator.
		it = result.first;

		//Build blind data.
		Aftk::C_BlindDataNode bd;

		for (
			Ut::UtSet<U32>::ConstIterator setIt = polySetCombinationIndices.Begin(); 
			setIt != polySetCombinationIndices.End(); ++setIt)
		{
			Aftk::C_GenericAttribute attr;
			//Get the current poly set fn
			MFnSet fnSet(m_polySetArray[setIt->GetValue()], &mStatus);
			std::string originalName = fnSet.name().asChar();
			std::string setName = originalName;
			//Remove namespacing for references.
			std::string::size_type pos = setName.find("polySet");
			if (pos != std::string::npos)
				setName = setName.substr(pos, setName.size()-pos);
			else
			{
				//Ignore this set as it's nothing to do with polyflags.
				continue;
			}

			MPlug plug = fnSet.findPlug("enumIndex", &mStatus);
			if (mStatus == MStatus::kSuccess)
			{
				std::string::size_type pos = setName.rfind("_");
				//In the case where we're dealing with an enum, strip the last fragment from the set
				//name.
				std::string enumString = setName.substr(pos+1, setName.length());
				attr.SetValue(enumString);
				attr.SetName(setName.substr(0, pos));
			}
			else
				attr.SetName(setName);

			//Add attribute to blind data.
			bd.AddAttribute(attr);
		}

		if (bd.GetNumAttributes() == 0)
		{
			it->GetValue() = 0;
			return 0;
		}

		//Add blind data to scene.
		it->GetValue() = m_pAftkScene->AddBlindDataNode(bd);
	}
	return it->GetValue();
}
