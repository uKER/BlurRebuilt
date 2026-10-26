#include "stdafx.h"


using namespace MayaIo;

C_Exporter::C_Exporter()
{
	m_pAftkScene = 0;

	//Callbacks
	m_pProgressNotification = 0;
}

Aftk::C_Status C_Exporter::Export(
	const Aftk::C_Scene* pAftkScene,
	Aftk::I_Progress* pProgress)
{
	m_pProgressNotification = pProgress;

	m_pAftkScene = pAftkScene;

	Aftk::C_Status status;
	MStatus	mStatus;

	status = CreateInfluencesMap();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	status = m_aftkMayaMapping.Create();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	status = ExportMaterials();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	//Import scene graph
	status = ExportSceneHierarchyTopLevel();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	//Import mesh skinning information
	status = ExportSkinClusters();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	status = ExportBindPoses();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	status = ExportDg();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	if (pProgress)
	{
		pProgress->OnProgress(0);
		pProgress->OnPrintProgress("Resolving instances....");
	}
	//Resolve instances
	status = ResolveInstances();
	AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

	Clear();
	m_aftkMayaMapping.Destroy();
	return Aftk::C_Status::E_Status_Success;
}

C_AftkMayaMapping* C_Exporter::AftkMayaMappingPtr()
{
	return &m_aftkMayaMapping;
}

const C_AftkMayaMapping* C_Exporter::AftkMayaMappingPtr() const
{
	return &m_aftkMayaMapping;
}

Aftk::C_Status C_Exporter::ExportMaterials()
{
	Aftk::C_Status status;
	MStatus mStatus;
	m_aftkMayaMaterialMappingArray.Resize(m_pAftkScene->GetNumMaterialNodes());
	m_aftkMayaMaterialShadingGroupMappingArray.Resize(m_pAftkScene->GetNumMaterialNodes());

	if (m_pProgressNotification)
	{
		SetProgress(0);
		SetProgressMax(m_pAftkScene->GetNumMaterialNodes());
	}

	for (U32 i = 0; i < m_pAftkScene->GetNumMaterialNodes(); i++)
	{
		if (m_pProgressNotification)
		{
			m_pProgressNotification->OnPrintProgress("Importing materials...");
		}

		const Aftk::C_SharedResource<Aftk::C_MaterialNode>* pAftkMaterialResource = 
			m_pAftkScene->GetMaterialResourcePtr(i);
		const Aftk::C_MaterialNode* pAftkMaterialNode = &pAftkMaterialResource->GetContent();
		
		MObject mayaMaterialObject;
		MObject mayaShadingGroupObject;

		status = C_MaterialExport::Export(
			this,
			m_pAftkScene,
			pAftkMaterialNode,
			&mayaMaterialObject,
			&mayaShadingGroupObject);
		AFTK_CHECK_ERROR_CLEAN_UP_RETURN;

		m_aftkMayaMaterialMappingMap[pAftkMaterialResource] = 
			mayaMaterialObject;
		m_aftkMayaMaterialMappingArray[pAftkMaterialResource->GetIndex()] = 
			mayaMaterialObject;
		m_aftkMayaMaterialShadingGroupMappingArray[pAftkMaterialResource->GetIndex()] =
			mayaShadingGroupObject;

		if (m_pProgressNotification)
		{
			AdvanceProgress();
			m_pProgressNotification->OnProgress(GetProgressPercent());
		}
	}

	if (m_pProgressNotification)
	{
		m_pProgressNotification->OnProgress(100.0f);
	}
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Exporter::ExportSceneHierarchyTopLevel()
{
	if (m_pProgressNotification)
	{
		SetProgress(0);
		SetProgressMax(m_pAftkScene->GetNumSceneNodes());
		m_pProgressNotification->OnPrintProgress("Importing scene hierarchy...");
	}

	const Aftk::C_SceneNode* pAftkSceneNode = m_pAftkScene->GetRootSceneNodePtr();
	if (!pAftkSceneNode)
	{
		AFTK_DO_BEGIN << "Aftk::C_Scene has no root node!" AFTK_DO_ERROR_RETURN_END_1;
	}
	
	Aftk::C_Status status;
	//Loop thru the siblings of the root node and export their children.
	pAftkSceneNode = pAftkSceneNode->GetChildPtr();
	while (pAftkSceneNode)
	{
		status = ExportSceneHierarchyChildren(pAftkSceneNode, MObject(MObject::kNullObj));
		CHECK_ERROR_RETURN;

		pAftkSceneNode = pAftkSceneNode->GetNextPtr();
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Exporter::ExportSceneHierarchyChildren(
	const Aftk::C_SceneNode* pAftkSceneNode,
	MObject& parentTransformObject)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MObject newTransformObject;
	MObject newShapeObject;

	switch (pAftkSceneNode->GetType())
	{
		case Aftk::E_ObjType_SceneMesh:
		{
			status = C_SceneMeshExport::Export(
				true,
				this,
				parentTransformObject, 
				newTransformObject,
				newShapeObject, 
				m_pAftkScene,
				pAftkSceneNode);
			CHECK_ERROR_RETURN;

			break;
		}

		case Aftk::E_ObjType_SceneInstance:
		{
			status = C_SceneInstanceExport::Export(
				true,
				this,
				parentTransformObject,
				newTransformObject,
				newShapeObject,
				m_pAftkScene,
				pAftkSceneNode);
			CHECK_ERROR_RETURN;
			break;
		}

		case Aftk::E_ObjType_SceneSpline:
		{
			status = C_SceneSplineExport::Export(
				true,
				this,
				parentTransformObject,
				newTransformObject,
				newShapeObject,
				m_pAftkScene,
				pAftkSceneNode);
			CHECK_ERROR_RETURN;
			break;
		}

		case Aftk::E_ObjType_SceneDirectionalLight:
		{
			status = C_SceneDirectionalLightExport::Export(
				true,
				this,
				parentTransformObject,
				newTransformObject,
				newShapeObject,
				m_pAftkScene,
				pAftkSceneNode);
			CHECK_ERROR_RETURN;
			break;
		}

		case Aftk::E_ObjType_SceneSpotLight:
		{
			status = C_SceneSpotLightExport::Export(
				true,
				this,
				parentTransformObject,
				newTransformObject,
				newShapeObject,
				m_pAftkScene,
				pAftkSceneNode);
			CHECK_ERROR_RETURN;
			break;
		}

		case Aftk::E_ObjType_ScenePointLight:
		{
			status = C_ScenePointLightExport::Export(
				true,
				this,
				parentTransformObject,
				newTransformObject,
				newShapeObject,
				m_pAftkScene,
				pAftkSceneNode);
			AFTK_CHECK_ERROR_RETURN;
			break;
		}

		case Aftk::E_ObjType_SceneJoint:
		{
			status = C_SceneJointExport::Export(
				true,
				this,
				parentTransformObject,
				newTransformObject,
				newShapeObject,
				m_pAftkScene,
				pAftkSceneNode);
			AFTK_CHECK_ERROR_RETURN;
			break;
		}
		default:
		{
			if (IsInfluencerNode(pAftkSceneNode))
			{
				status = C_SceneJointExport::Export(
					true,
					this,
					parentTransformObject, 
					newTransformObject,
					newShapeObject,
					m_pAftkScene,
					pAftkSceneNode);
				AFTK_CHECK_ERROR_RETURN;
			}
			else
			{
				status = C_SceneNodeExport::Export(
					true,
					this,
					parentTransformObject, 
					newTransformObject,
					newShapeObject,
					m_pAftkScene,
					pAftkSceneNode);
				AFTK_CHECK_ERROR_RETURN;
			}
			break;
		}
	}

	if (m_pProgressNotification)
	{
		AdvanceProgress();
		m_pProgressNotification->OnProgress(GetProgressPercent());
	}

	U32 numChildren = pAftkSceneNode->GetNumChildren();
	if (numChildren > 0)
	{
		for (U32 i = 0; i < numChildren; i++)
		{
			status = ExportSceneHierarchyChildren(
				pAftkSceneNode->GetChildPtr(i), 
				newTransformObject);
			AFTK_CHECK_ERROR_RETURN;
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Exporter::ExportSkinClusters()
{
	Aftk::C_Status status;

	if (m_pProgressNotification)
	{
		SetProgressMax(m_pAftkScene->GetNumSceneNodes());
		SetProgress(0);
		m_pProgressNotification->OnProgress(0);
		m_pProgressNotification->OnPrintProgress("Importing skin clusters...");
	}

	Aftk::C_SceneNodeConstIterator it(m_pAftkScene);

	for ( ; it; it++)
	{
		const Aftk::C_SceneNode* pAftkSceneNode = it;
		if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			Aftk::I_SceneMeshNode* pAftkMeshNode = (Aftk::I_SceneMeshNode*)pAftkSceneNode;
			MDagPath mayaDagPath;
			if (!m_aftkMayaMapping.GetMayaDagPathFromAftkSceneNode(pAftkMeshNode, &mayaDagPath))
			{
				//We've no Maya version of this Aftk scene node.
				continue;
			}
			status = ExportSkinCluster(mayaDagPath, pAftkMeshNode );
		}
		
		if (m_pProgressNotification)
		{
			AdvanceProgress();
			m_pProgressNotification->OnProgress(GetProgressPercent());
		}

	}

	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_Exporter::ExportSkinCluster(
	MDagPath& mayaDagPath, 
	const Aftk::I_SceneMeshNode* pAftkMeshNode)
{
	MStatus mStatus;
	Aftk::C_Status status;

	U32 nInfluenceClusters = pAftkMeshNode->GetNumInfluenceClusters();
	if(nInfluenceClusters)
	{
		MDGModifier dgMod;
		MFnSkinCluster skinCluster;

		MString command("skinCluster -tsb -ibp " + mayaDagPath.partialPathName() + " ");

		for (U32 i=0; i<nInfluenceClusters; ++i)
		{
			const Aftk::C_InfluenceClusterNode *pAftkInfluenceCluster = pAftkMeshNode->GetInfluenceClusterPtr( i );
			if( pAftkInfluenceCluster->GetUsage() != AFTK_INFLUENCE_CLUSTER_USAGE_SKIN )
				continue;

			U32 nEnvelopes = pAftkInfluenceCluster->GetNumEnvelopes();
			U32 nValidEnvelopes = 0;
			U32 nVertices = MFnMesh(mayaDagPath).numVertices();;

			MIntArray mayaInfluenceIndicesArray;
			MDoubleArray mayaWeightArray;

			MFnSingleIndexedComponent component;
			component.create(MFn::kMeshVertComponent, &mStatus);
			MCHECK_ERROR_RETURN_1
			component.setCompleteData(nVertices);

			// First pass: call command to set up DAG nodes
			for (U32 j=0; j<nEnvelopes; ++j)
			{
				const Aftk::C_WeightEnvelope pAftkEnvelope = pAftkInfluenceCluster->GetEnvelope( j );
				const Aftk::C_SceneNode *pAftkInfluenceNode = pAftkEnvelope.GetInfluenceNodePtr();

				MDagPath influenceNodeDagPath;
				if (!m_aftkMayaMapping.GetMayaDagPathFromAftkSceneNode(pAftkInfluenceNode, &influenceNodeDagPath))
				{
					AFTK_DO_BEGIN << "No influence node has been found" AFTK_DO_WARNING_END;
					continue;
				}

				MFnIkJoint influenceJoint(influenceNodeDagPath, &mStatus);
				if (!mStatus)
				{
					AFTK_DO_BEGIN << MFnDagNode(influenceNodeDagPath).name().asChar() << " is not a valid joint node" AFTK_DO_WARNING_END;
					continue;
				}
				
				command = command + " " + influenceJoint.name();
				nValidEnvelopes++;
			}

			// commit changes to DAG as we need to know where in the array the influence DAGPath lies for weighting
			mStatus = dgMod.commandToExecute(command);
			MCHECK_ERROR_RETURN_1

			mStatus = dgMod.doIt();
			MCHECK_ERROR_RETURN_1

			{
				//Get skin cluster node - check what is connected to the "inMesh" plug
				MPlugArray plugArray;
				MPlug inMeshPlug = MFnMesh(mayaDagPath).findPlug("inMesh", &mStatus);
				MCHECK_ERROR_RETURN_1

				if ( !inMeshPlug.connectedTo(plugArray,true, false) || (plugArray.length() != 1) )
					return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Maya command failed: \"inMesh\" plug not connected properly");

				mStatus = skinCluster.setObject(plugArray[0].node());
				MCHECK_ERROR_RETURN_1
			}

			mayaWeightArray.setLength(nEnvelopes*nVertices);
			for (U32 j=0; j<nValidEnvelopes*nVertices; ++j)
				mayaWeightArray.set(0.0, j);

			// Second pass: create weights
			for (U32 j=0; j<nEnvelopes; ++j)
			{
				const Aftk::C_WeightEnvelope pAftkEnvelope = pAftkInfluenceCluster->GetEnvelope( j );
				const Aftk::C_SceneNode *pAftkInfluenceNode = pAftkEnvelope.GetInfluenceNodePtr();

				MDagPath influenceNodeDagPath;
				if (!m_aftkMayaMapping.GetMayaDagPathFromAftkSceneNode(pAftkInfluenceNode, &influenceNodeDagPath))
				{
					AFTK_DO_BEGIN << "No influence node has been found" AFTK_DO_WARNING_END;
					continue;
				}

				MFnIkJoint influenceJoint(influenceNodeDagPath, &mStatus);
				if (!mStatus)
					continue;

				U32 nWeights = pAftkEnvelope.GetNumWeights();

				U32 index = skinCluster.indexForInfluenceObject(influenceNodeDagPath, &mStatus);
				MCHECK_ERROR_RETURN_1
				mStatus = mayaInfluenceIndicesArray.append(index);
				MCHECK_ERROR_RETURN_1

				for (U32 k=0; k<nWeights; ++k)
				{
					Aftk::C_Weight<F32> aftkWeight;
					pAftkEnvelope.GetWeight(k, aftkWeight);

					mayaWeightArray.set(aftkWeight.GetWeight(), index + nValidEnvelopes*aftkWeight.GetIndex());
				}
			}

			mStatus = mayaDagPath.extendToShape();
			MCHECK_ERROR_RETURN_1

			mStatus = skinCluster.setWeights(mayaDagPath, component.object(), mayaInfluenceIndicesArray, mayaWeightArray, false);
			MCHECK_ERROR_RETURN_1


			// There should only be one skin cluster associated with a mesh.
			// may as well return here.
			return Aftk::C_Status::E_Status_Success;
		}
	}
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Exporter::ExportBindPoses()
{
	Aftk::C_Status status;

	if (m_pProgressNotification)
	{
		SetProgressMax(m_pAftkScene->GetNumSceneNodes());
		SetProgress(0);
		m_pProgressNotification->OnProgress(0);
		m_pProgressNotification->OnPrintProgress("Exporting bind pose...");
	}

	Aftk::C_SceneNodeConstIterator it(m_pAftkScene);

	for ( ; it; it++)
	{
		const Aftk::C_SceneNode* pAftkSceneNode = it;

		Aftk::C_SceneJointNode* pAftkJointNode = (Aftk::C_SceneJointNode*)pAftkSceneNode;
		MDagPath mayaDagPath;
		if (!m_aftkMayaMapping.GetMayaDagPathFromAftkSceneNode(pAftkJointNode, &mayaDagPath))
		{
			//We've no Maya version of this Aftk scene node.
			continue;
		}

		MStatus mStatus;
		MFnIkJoint fnJoint(mayaDagPath, &mStatus);
		if (!mStatus)
			continue;

		status = ExportBindPose(mayaDagPath, pAftkJointNode);

		if (m_pProgressNotification)
		{
			AdvanceProgress();
			m_pProgressNotification->OnProgress(GetProgressPercent());
		}
	}

	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_Exporter::ExportBindPose(
	MDagPath &mayaDagPath,
	const Aftk::C_SceneNode *pAftkSceneNode)
{
	MStatus mStatus;
	Aftk::C_Status status;

	MFnIkJoint fnJoint(mayaDagPath, &mStatus);
	MCHECK_ERROR_RETURN_1

	{
		MPlug sscBindEnablePlug = fnJoint.findPlug("bindSegmentScaleCompensate", &mStatus);
		MCHECK_ERROR_RETURN_1;
		MPlug sscEnablePlug = fnJoint.findPlug("segmentScaleCompensate", &mStatus);
		MCHECK_ERROR_RETURN_1;

		if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneJoint)
		{
			mStatus = sscBindEnablePlug.setValue(((const Aftk::C_SceneJointNode*)pAftkSceneNode)->GetSegmentScaleCompensation());
			MCHECK_ERROR_RETURN_1;
			mStatus = sscEnablePlug.setValue(((const Aftk::C_SceneJointNode*)pAftkSceneNode)->GetSegmentScaleCompensation());
			MCHECK_ERROR_RETURN_1;
		}
		else
		{
			mStatus = sscBindEnablePlug.setValue(false);
			MCHECK_ERROR_RETURN_1;
			mStatus = sscEnablePlug.setValue(false);
			MCHECK_ERROR_RETURN_1;
		}
	}


	bool found = false;
	MPlug bindPoseXformPlug;
	{
		MPlug jointBindPosePlug = fnJoint.findPlug("bindPose", &mStatus);
		if (!mStatus)
			return Aftk::C_Status::E_Status_Success;

		MPlugArray connectedJointPlugs;
		jointBindPosePlug.connectedTo(connectedJointPlugs, false, true, &mStatus);
		MCHECK_ERROR_RETURN_1

		// Find 'worldMatrix' attribute on bindPose plug
		for (U32 i=0; i<connectedJointPlugs.length(); ++i)
		{
			MObject attrObj = connectedJointPlugs[i].attribute();
			MFnAttribute fnAttr(attrObj);
			if (fnAttr.name() == "worldMatrix")
			{
				MFnDependencyNode bindPoseNode(connectedJointPlugs[i].node(),&mStatus);
				MCHECK_ERROR_RETURN_1;

				MObject xformMatrixAttribute = bindPoseNode.attribute("xformMatrix", &mStatus);
				MCHECK_ERROR_RETURN_1

				bindPoseXformPlug = MPlug(connectedJointPlugs[i].node(), xformMatrixAttribute);

				mStatus = bindPoseXformPlug.selectAncestorLogicalIndex(connectedJointPlugs[i].logicalIndex(), xformMatrixAttribute);
				MCHECK_ERROR_RETURN_1;

				found = true;
			}
		}
	}
	if (!found)
		return Aftk::C_Status::E_Status_Success;
/*
	MPlug messageXformPlug;
	{
		MPlug jointMessagePlug = fnJoint.findPlug("message", &mStatus);
		if (!mStatus)
			return Aftk::C_Status::E_Status_Success;

		MPlugArray connectedJointPlugs;
		jointMessagePlug.connectedTo(connectedJointPlugs, false, true, &mStatus);
		MCHECK_ERROR_RETURN_1

		for (U32 i=0; i<connectedJointPlugs.length(); ++i)
		{
			MFnDependencyNode bindPoseNode(connectedJointPlugs[i].node(),&mStatus);
			MCHECK_ERROR_RETURN_1;

			MObject xformMatrixAttribute = bindPoseNode.attribute("xformMatrix", &mStatus);
			MCHECK_ERROR_RETURN_1

			messageXformPlug = MPlug(connectedJointPlugs[i].node(), xformMatrixAttribute);

			mStatus = messageXformPlug.selectAncestorLogicalIndex(connectedJointPlugs[i].logicalIndex(), xformMatrixAttribute);
			MCHECK_ERROR_RETURN_1;
		}
	}
*/
	Aftk::Matrix44 aftkBasePose = pAftkSceneNode->GetBasePose();
	Aftk::Matrix44 aftkBasePoseLocal;
	if (IsInfluencerNode(pAftkSceneNode->GetParentPtr()))
		aftkBasePoseLocal = pAftkSceneNode->GetBasePose() * pAftkSceneNode->GetParentPtr()->GetBasePoseInverse();
	else
		aftkBasePoseLocal = pAftkSceneNode->GetBasePose();

	F32 tempMatrix[4][4] = 
	{
		{ aftkBasePose[0][0], aftkBasePose[0][1], aftkBasePose[0][2], aftkBasePose[0][3] },
		{ aftkBasePose[1][0], aftkBasePose[1][1], aftkBasePose[1][2], aftkBasePose[1][3] },
		{ aftkBasePose[2][0], aftkBasePose[2][1], aftkBasePose[2][2], aftkBasePose[2][3] },
		{ aftkBasePose[3][0], aftkBasePose[3][1], aftkBasePose[3][2], aftkBasePose[3][3] }
	};

	F32 tempLocalMatrix[4][4] = 
	{
		{ aftkBasePoseLocal[0][0], aftkBasePoseLocal[0][1], aftkBasePoseLocal[0][2], aftkBasePoseLocal[0][3] },
		{ aftkBasePoseLocal[1][0], aftkBasePoseLocal[1][1], aftkBasePoseLocal[1][2], aftkBasePoseLocal[1][3] },
		{ aftkBasePoseLocal[2][0], aftkBasePoseLocal[2][1], aftkBasePoseLocal[2][2], aftkBasePoseLocal[2][3] },
		{ aftkBasePoseLocal[3][0], aftkBasePoseLocal[3][1], aftkBasePoseLocal[3][2], aftkBasePoseLocal[3][3] }
	};

	MMatrix mayaBindPose(tempMatrix);
	MMatrix mayaBindPoseLocal(tempLocalMatrix);

	MFnMatrixData fnMatrix;

	MObject mayaBindPoseObj = fnMatrix.create(mayaBindPose, &mStatus);
	MCHECK_ERROR_RETURN_1

	MObject mayaBindPoseLocalObj = fnMatrix.create(mayaBindPoseLocal, &mStatus);
	MCHECK_ERROR_RETURN_1

	MObject mayaBindPoseInvObj = fnMatrix.create(mayaBindPose.inverse(), &mStatus);
	MCHECK_ERROR_RETURN_1

    mStatus = bindPoseXformPlug.setValue(mayaBindPoseLocalObj);
	MCHECK_ERROR_RETURN_1;

    //mStatus = messageXformPlug.setValue(mayaBindPoseLocalObj);
	//MCHECK_ERROR_RETURN_1;


	//Check if there are any skin clusters bound to joint & update its bind pose
	MObject node(mayaDagPath.node());
	MItDependencyGraph it(node, MFn::kSkinClusterFilter);
	while (!it.isDone())
	{
		MPlug plug = it.thisPlug();
		unsigned int idx = plug.logicalIndex();
		MFnDependencyNode skinFn(plug.node());
		MPlug skinBindPosePlug = skinFn.findPlug( "bindPreMatrix", &mStatus);
		if( mStatus == MS::kSuccess)
		{
			// The skinCluster stores an inverse matrix
			skinBindPosePlug = skinBindPosePlug.elementByLogicalIndex(idx);
			skinBindPosePlug.setValue(mayaBindPoseInvObj);
		}
		it.next();
	}

	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_Exporter::ExportDg()
{
	Aftk::C_Status status;
	MStatus mStatus;

	//Export scene level animation information
	MTime startTime, endTime;

	//Set start- and end-frame for Maya
	MTime oneSecond( 1, MTime::kSeconds );
	F64 frameRate = oneSecond.as(MTime::uiUnit());

	F32 ourFrameRate = m_pAftkScene->GetFrameRate();

	startTime.setUnit(MTime::kSeconds);
	endTime.setUnit(MTime::kSeconds);
	
	if (ourFrameRate < AFTK_EPSILON)
	{
		startTime.setValue(m_pAftkScene->GetStartTime());
		endTime.setValue(m_pAftkScene->GetEndTime());
	}
	else
	{
		startTime.setValue(m_pAftkScene->GetStartTime() / ourFrameRate);
		endTime.setValue(m_pAftkScene->GetEndTime() / ourFrameRate);
	}

	MAnimControl::setAnimationStartTime(startTime);
	MAnimControl::setAnimationEndTime(endTime);

	MAnimControl::setMinMaxTime(startTime, endTime);
	
	MTime::setUIUnit(MTime::kNTSCField);

	//Export animated hierarchy nodes...
	Aftk::C_SceneNodeConstIterator it(m_pAftkScene);
	while (it)
	{
		MDagPath thisPath;
		Aftk::C_SceneNode* pAftkSceneNode = it;

		if (m_aftkMayaMapping.GetMayaDagPathFromAftkSceneNode(pAftkSceneNode, &thisPath))
		{
			MDagPath thisShape(thisPath);
			//Attempt to acquire shape.
			bool hasShape;

			if (pAftkSceneNode->HasType(Aftk::E_ObjType_SceneInstance) ||
				pAftkSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
				hasShape = false;
			else
			{
				mStatus = thisShape.extendToShape();
				if (mStatus)
					hasShape = true;
				else
					hasShape = false;
			}

			//Export transform and shape connections.
			if (hasShape)
			{
				status = C_ConnectedDgExport::Export(
					this,
					thisPath,
					&thisShape,
					m_pAftkScene,
					pAftkSceneNode);
				AFTK_CHECK_ERROR_RETURN;
			}
			else
			{
				status = C_ConnectedDgExport::Export(
					this,
					thisPath,
					0,
					m_pAftkScene,
					pAftkSceneNode);
				AFTK_CHECK_ERROR_RETURN;
			}
		}

		it++;
	}

	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_Exporter::ResolveInstances()
{
	Aftk::C_Status status;
	MStatus mStatus;

	if (m_pProgressNotification)
	{
		SetProgressMax(m_pAftkScene->GetNumSceneNodes());
		SetProgress(0);
		m_pProgressNotification->OnProgress(0);
		m_pProgressNotification->OnPrintProgress("Resolving Instances...");
	}

	Aftk::C_SceneNodeConstIterator it(m_pAftkScene);
	for ( ; it ; it++)
	{
		const Aftk::C_SceneNode* pAftkSceneNode = it;
		if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			const Aftk::C_SceneInstanceNode* pAftkSceneInstanceNode = 
				(const Aftk::C_SceneInstanceNode*)pAftkSceneNode;

			//We must have a source to be a valid instance
			if (!pAftkSceneInstanceNode->GetSourceNodePtr())
			{
				#ifdef _DEBUG
				AFTK_ASSERT(false);
				#endif

				AFTK_DO_BEGIN << "Given instance had no source node." AFTK_DO_WARNING_END;
				continue;
			}

			MDagPath instanceTransform;
			if (!m_aftkMayaMapping.GetMayaDagPathFromAftkSceneNode(pAftkSceneNode, &instanceTransform))
			{
				//We've no Maya version of this Aftk scene node.
				continue;
			}
			MFnDagNode instanceTransformNode(instanceTransform, &mStatus);
			MCHECK_ERROR_RETURN_1;

			#ifdef _DEBUG
			AFTK_DO_BEGIN << instanceTransformNode.name().asChar() << "\n" AFTK_DO_DEBUG_END;
			#endif

			//Resolve this instance's source.
			MDagPath sourceTransformPath;
			if (!m_aftkMayaMapping.GetMayaDagPathFromAftkSceneNode(pAftkSceneInstanceNode->GetSourceNodePtr(), &sourceTransformPath))
			{
				AFTK_DO_BEGIN << "Failed to find DAG path for instance source." << "\n" AFTK_DO_ERROR_RETURN_END_1;
			}
			#ifdef _DEBUG
			AFTK_DO_BEGIN << sourceTransformPath.fullPathName().asChar() << "\n" AFTK_DO_DEBUG_END;
			#endif
			MFnTransform sourceTransform(sourceTransformPath, &mStatus);
			MCHECK_ERROR_RETURN_1;

			MDagPath sourceShapePath(sourceTransformPath);
			mStatus = sourceShapePath.extendToShape();
			if (!mStatus)
			{
				AFTK_DO_BEGIN << "Could not extend instance source dag to shape. The targeted object has no shape!" AFTK_DO_WARNING_END;
				continue;
			}

			MFnDagNode sourceNode(sourceShapePath, &mStatus);
			MCHECK_BEGIN << "Could not acquire instance source node" MCHECK_ERROR_RETURN_END_1
			#ifdef _DEBUG
			AFTK_DO_BEGIN << sourceNode.name().asChar() << "\n" AFTK_DO_DEBUG_END;
			#endif

			MString command;
			command += "parent -add -shape ";
			command += sourceNode.name();
			command += " ";
			command += instanceTransformNode.name();
			#ifdef _DEBUG
			AFTK_DO_BEGIN << command.asChar() << "\n" AFTK_DO_DEBUG_END;
			#endif
			MStringArray results;
			mStatus = MGlobal::executeCommand(command, results, false, false);
			MCHECK_BEGIN << "Could not execute command" MCHECK_ERROR_RETURN_END_1

			if (!sourceShapePath.isInstanced())
			{
				AFTK_DO_BEGIN << "Could not validate that an instance was created." AFTK_DO_ERROR_RETURN_END_1;
				#ifdef _DEBUG
				AFTK_ASSERT(false);
				#endif
			}
		}
		if (m_pProgressNotification)
		{
			AdvanceProgress();
			m_pProgressNotification->OnProgress(GetProgressPercent());
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Exporter::AssignToShadingGroup(
	const MObject& shadingGroup,
	MDagPath& dagPath) const
{
	Aftk::C_Status status;
	MStatus	mStatus;
	MFnSet	fnSG(shadingGroup, &mStatus);
	MCHECK_ERROR_RETURN_1;

	if ( fnSG.restriction() != MFnSet::kRenderableOnly )
	{
		AFTK_DO_BEGIN << "Failed to validate MFnSet during shading group assignment." AFTK_DO_ERROR_RETURN_END_1;
	}

	//g_tcDebugMan << "SG name " << fnSG.name().asChar() << "\n";
	mStatus = fnSG.addMember(dagPath);
	MCHECK_BEGIN << "Could not add DAG node to shading group." MCHECK_ERROR_RETURN_END_1;

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Exporter::AddToDefaultShadingGroup(MDagPath& dagPath) const
{
	//g_tcDebugMan << "DAG path " << dagPath.fullPathName().asChar() << "\n";

	MSelectionList      selList;
	MObject             initialSG;

	MGlobal::getSelectionListByName(MString("initialShadingGroup"), selList);
	selList.getDependNode(0, initialSG);

	dagPath.extendToShape();

	return AssignToShadingGroup(initialSG, dagPath);
}

MObject C_Exporter::GetMayaMaterial(const Aftk::C_SharedResource<Aftk::C_MaterialNode>* pAftkMaterialResource) const
{
	Ut::UtMap<const Aftk::C_SharedResource<Aftk::C_MaterialNode>*, MObject>::ConstIterator it;

	it = m_aftkMayaMaterialMappingMap.Find(pAftkMaterialResource);
	if (it == m_aftkMayaMaterialMappingMap.End())
	{
		AFTK_ASSERT(false);
		return MObject();
	}

	return it->GetValue();
}

MObject C_Exporter::GetMayaMaterial(U32 instance) const
{
	return m_aftkMayaMaterialMappingArray[instance];
}

MObject C_Exporter::GetMayaMaterialShadingGroup(U32 instance) const
{
	return m_aftkMayaMaterialShadingGroupMappingArray[instance];
}

void C_Exporter::Clear()
{
	m_aftkMayaMaterialMappingMap.Clear();
	m_aftkMayaMaterialMappingArray.Clear();
	m_aftkMayaMaterialShadingGroupMappingArray.Clear();
}

void C_Exporter::CleanUpAfterFailure()
{
	Clear();
	m_aftkMayaMapping.Destroy();
}

Aftk::C_Status C_Exporter::CreateInfluencesMap()
{
	Aftk::C_SceneNodeConstIterator it(m_pAftkScene);
	while (it)
	{
		const Aftk::C_SceneNode* pAftkSceneNode = it;
		U32 numInfluences = pAftkSceneNode->GetNumInfluenceClusters();

		for (U32 i=0; i<numInfluences; ++i)
		{
			const Aftk::C_InfluenceClusterNode *pAftkInfluenceCluster = pAftkSceneNode->GetInfluenceClusterPtr(i);
			if (pAftkInfluenceCluster->GetUsage() == AFTK_INFLUENCE_CLUSTER_USAGE_SKIN)
			{
				U32 numEnvelopes = pAftkInfluenceCluster->GetNumEnvelopes();
				for (U32 j=0; j<numEnvelopes; ++j)
				{
					const Aftk::C_WeightEnvelope& pAftkEnvelope = pAftkInfluenceCluster->GetEnvelope(j);
					const Aftk::C_SceneNode *pInfluenceSceneNode = pAftkEnvelope.GetInfluenceNodePtr();

					m_aftkSceneNodeInfluenceMap.Insert(
						Ut::MultiMap<const Aftk::C_SceneNode*, const Aftk::C_SceneNode*>::DataType(
							pInfluenceSceneNode, pAftkSceneNode));

				}
			}
		}
		it++;
	}
	return Aftk::C_Status::E_Status_Success;
}

bool C_Exporter::IsInfluencerNode(
	const Aftk::C_SceneNode *pAftkSceneNode)
{

	Ut::MultiMap<const Aftk::C_SceneNode*, const Aftk::C_SceneNode*>::ConstIterator it
		= m_aftkSceneNodeInfluenceMap.Find(pAftkSceneNode);

	if (it != m_aftkSceneNodeInfluenceMap.End())
		return true;
	else
		return false;
}