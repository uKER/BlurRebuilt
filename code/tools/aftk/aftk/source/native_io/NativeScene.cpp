#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeScene.h"
#include "NativeMaterialNode.h"
#include "NativeBlindDataNode.h"
#include "NativeSceneNode.h"
#include "NativeSceneMeshRemappableNode.h"
#include "NativeSceneMeshNode.h"
#include "NativeSceneMeshBonedInstanceNode.h"
#include "NativeSceneMeshRemappedInstanceNode.h"
#include "NativeSceneSplineNode.h"
#include "NativeSceneInstanceNode.h"
#include "NativeUtility.h"
#include "NativeLayerNode.h"
#include "NativeSceneLightNode.h"
#include "NativeSceneNonAmbientLightNode.h"
#include "NativeScenePointLightNode.h"
#include "NativeSceneAreaLightNode.h"
#include "NativeSceneDirectionalLightNode.h"
#include "NativeSceneSpotLightNode.h"
#include "NativeSceneHemisphereLightNode.h"
#include "NativeInfluenceClusterNode.h"
#include "NativeSceneJointNode.h"
#include "NativeSceneLodGroupNode.h"
#include "NativeSceneCameraNode.h"
#include "NativeSceneXRefNode.h"
#include "NativeXRefScene.h"
#include "NativeAnimationCurve.h"
#include "NativeBlendWeightedNode.h"
#include "NativeIPlug.h"
#include "NativeWeightEnvelope.h"
#include "NativeUnitConversionNode.h"
#include "NativeSceneConstraintNode.h"
#include "NativeSceneAimConstraintNode.h"
#include "NativeSceneLookAtConstraintNode.h"
#include "NativeDistanceDimNode.h"
#include "NativeScenePointConstraintNode.h"
#include "NativeSceneOrientConstraintNode.h"

using namespace Aftk;

U32 C_NativeScene::m_numProcessingStagesCompleted;
U32 C_NativeScene::m_numProcessingStages;

C_Status C_NativeScene::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk,
	const C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;
	m_numProcessingStagesCompleted = 0;
	m_numProcessingStages = 0;

	status = WriteScene(pDiskWriter, pNativeFtk, pScene, pProgress);
	if (!status) return status;

	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk,
	C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;
	m_numProcessingStagesCompleted = 0;
	m_numProcessingStages = 0;

	Matrix44 sceneTransform;
	{
		status = ReadScene(
			pDiskReader, 
			pNativeFtk,
			pScene,
			sceneTransform,
			pProgress);
	}
	//Copy back next unique id.
	pScene->m_nextSceneNodeUniqueId = pNativeFtk->m_nextSceneNodeUniqueId;
	if (!status) return status;

	if (!ResolveObjectPointers(
		pDiskReader->GetVersion(), 
		pNativeFtk, 
		pScene))
		return C_Status(C_Status::E_Status_Failure, "Failed to resolve object pointers.");

	//Trigger re-evaluation of plug dependent proporties.
	if (!EvaluateUnmanagedPlugs(
		pDiskReader->GetVersion(), 
		pNativeFtk, 
		pScene))
		return C_Status(C_Status::E_Status_Failure, "Failed to evaluate unmanaged plugs.");

	if (pDiskReader->GetVersion() < 202)
	{
		if (pScene->GetRootSceneNodePtr())
		{
			pScene->GetRootSceneNodePtr()->SetLocalMatrix(
				sceneTransform * pScene->GetRootSceneNodePtr()->GetLocalMatrix(false));

		}
	}

	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::WriteScenePrototype(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk,
	const C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;
	C_SceneNodeConstIterator it(pScene);

	m_numProcessingStagesCompleted = 0;
	m_numProcessingStages = 0;

	//////////////////// BEGIN WRITING DATA TO DISK ////////////////////
	
	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_PROTOTYPE);
	{
		//C_Scene proporties
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_SCENE);
		{
			*pDiskWriter << pScene->GetNumMaterialNodes();
			*pDiskWriter << pScene->GetNumBlindDataNodes();
			*pDiskWriter << (U32)pScene->m_unmanagedDgNodes.Size();
		}
		pDiskWriter->EndBlock();

		//Write out animation timing information.
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_ANIMATION_TIMING);
		{
			//U32 timing = (U32)pScene->GetTiming();
			U32 timing = 0;//(U32)pScene->GetTiming();
			*pDiskWriter << timing;

			*pDiskWriter << pScene->GetStartTime();

			*pDiskWriter << pScene->GetEndTime();

			*pDiskWriter << pScene->GetFrameRate();
		}
		pDiskWriter->EndBlock();

		//Write out memory manager cues
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_MEMORY_MANAGER_CUES);
		{
			status = WriteMemoryManagerCues(
				pDiskWriter,
				pNativeFtk,
				pScene,
				pProgress);
			if (!status) return status;
		}
		pDiskWriter->EndBlock();
	}
	// End BLOCK_ID_PROTOTYPE
	pDiskWriter->EndBlock();

	//////////////////// END WRITING DATA TO DISK ////////////////////

	// Let's work out how much work we have to do to load this scene based upon its prototype
	m_numProcessingStagesCompleted = 0;
	m_numProcessingStages;

	m_numProcessingStages = pScene->GetNumBlindDataNodes();
	m_numProcessingStages += pScene->GetNumMaterialNodes();
	m_numProcessingStages += (U32)pScene->m_unmanagedDgNodes.Size();
	
	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::WriteMaterials(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk,
	const C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;

	U32 numMaterialNodes = pScene->GetNumMaterialNodes();

	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_MATERIAL_LIST);
	{
		//Write out the number of materials
		*pDiskWriter << numMaterialNodes;
		//Loop thru material list
		for (U32 i = 0 ; i < numMaterialNodes; i++) 
		{
			// source
			const C_MaterialResource* pRes = 
				pScene->GetMaterialResourcePtr(i);
			//Write out material
			status = C_NativeMaterialNode::Serialise(pDiskWriter, pNativeFtk, pRes->GetIndex(), &pRes->GetContent(), true);
			if (!status) return status;

			m_numProcessingStagesCompleted++;
			F32 progress = ((F32)m_numProcessingStagesCompleted / (F32)m_numProcessingStages) * 100.0f;
			if (pProgress)
			{
				if (pProgress->OnProgress(progress))
				{
					return C_Status(C_Status::E_Status_Failure, "Scene writing aborted.");
				}
			}
		}
	}
	pDiskWriter->EndBlock();
	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::WriteBlindData(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk,
	const C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;
	U32 numBlindDataNodes = pScene->GetNumBlindDataNodes();

	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_BLIND_DATA_LIST);
	{
		//Write out the number of blind data nodes
		*pDiskWriter << numBlindDataNodes;
		//Loop thru blind data nodes list

		if (numBlindDataNodes > 0)
		{
			Ut::UtVector<Ut::DynamicVector<C_SharedResource<C_BlindDataNode>* > >::ConstIterator it =
				pScene->m_blindDataNodeManager.m_array.Begin();
			for (U32 i = 0 ; i < numBlindDataNodes; i++) 
			{
				const C_BlindDataResource* pRes = (*it);
				//Store ith mapping for speed later on.
				(*pNativeFtk->m_pBlindDataResToIthIndexMapping)[pRes] = i;

				//Write out blind data nodes
				status = C_NativeBlindDataNode::Serialise(pDiskWriter, pNativeFtk, &pRes->GetContent(), true);
				if (!status) return status;

				m_numProcessingStagesCompleted++;
				F32 progress = ((F32)m_numProcessingStagesCompleted / (F32)m_numProcessingStages) * 100.0f;
				if (pProgress)
				{
	//				pProgress->OnPrintProgress(std::string("Read node ") + pSceneNode->GetName());
					if (pProgress->OnProgress(progress))
					{
						return C_Status(C_Status::E_Status_Failure, "Scene writing aborted.");
					}
				}

				it++;
			}
		}
	}
	pDiskWriter->EndBlock();
	
	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::WriteUnmanagedDgNodes(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk,
	const C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;
	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_UNMANAGED_DG_NODE_LIST);
	{
		Ut::UtSet<C_Node*>::ConstIterator it = pScene->m_unmanagedDgNodes.Begin();
		Ut::UtSet<C_Node*>::ConstIterator end = pScene->m_unmanagedDgNodes.End();
		for (; it!= end; it++)
		{
			const C_Node* pNode = *it;

			switch (pNode->GetType())
			{
				//////////////// UNMANAGED DG NODES ////////////////
				case E_ObjType_AnimationCurve:
				{
					status = C_NativeAnimationCurveNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_AnimationCurveNode*)pNode, 
						true);
					if (!status) return status;
				}
				break;

				case E_ObjType_BlendWeighted:
				{
					status = C_NativeBlendWeightedNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_BlendWeightedNode*)pNode, 
						true);
					if (!status) return status;
				}
				break;

				case E_ObjType_DistanceDim:
				{
					status = C_NativeDistanceDimNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_DistanceDimNode*)pNode, 
						true);
					if (!status) return status;
				}
				break;

				case E_ObjType_UnitConversion:
				{
					status = C_NativeUnitConversionNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_UnitConversionNode*)pNode, 
						true);
					if (!status) return status;
				}
				break;

				case E_ObjType_InfluenceCluster:
				{
					status = C_NativeInfluenceClusterNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_InfluenceClusterNode*)pNode, 
						true);
					if (!status) return status;
				}
				break;

				case E_ObjType_Layer:
				{
					status = C_NativeLayerNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_LayerNode*)pNode, 
						true);
					if (!status) return status;
				}
				break;

				//////////////// SCENE GRAPH NODES ////////////////
				case E_ObjType_SceneNode:
				{
					status = C_NativeSceneNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneNode*)pNode, 
						true);
					if (!status) return status;
				}
				break;

				case E_ObjType_SceneJoint:
				{
					status = C_NativeSceneJointNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(C_SceneJointNode*)pNode, 
						true);
					if (!status) return status;
				}
				break;

				case E_ObjType_SceneLodGroup:
				{
					status = C_NativeSceneLodGroupNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneLodGroupNode*)pNode, 
						true);
					if (!status) return status;
				}
				break;

				case E_ObjType_SceneCamera:
				{
					status = C_NativeSceneCameraNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneCameraNode*)pNode,
						true);
					if (!status) return status;
				}
				break;
				case E_ObjType_SceneMesh:
				{
					status = C_NativeSceneMeshNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneMeshNode*)pNode,
						true);
					if (!status) return status;

					break;
				}
				case E_ObjType_SceneMeshBonedInstance:
				{
					status = C_NativeSceneMeshBonedInstanceNode::Serialise(
						pDiskWriter,
						pNativeFtk,
						(const C_SceneMeshBonedInstanceNode*)pNode,
						true);
					if (!status) return status;
				}
				break;
				case E_ObjType_SceneMeshRemappedInstance:
				{
					status = C_NativeSceneMeshRemappedInstanceNode::Serialise(
						pDiskWriter,
						pNativeFtk,
						(const C_SceneMeshRemappedInstanceNode*)pNode,
						true);
					if (!status) return status;
				}
				break;
				case E_ObjType_SceneSpline:
				{
					status = C_NativeSceneSplineNode::Serialise(
						pDiskWriter,
						pNativeFtk, 
						(const C_SceneSplineNode*)pNode,
						true);
					if (!status) return status;
					break;
				}
				case E_ObjType_ScenePointLight:
				{
					status = C_NativeScenePointLightNode::Serialise(
						pDiskWriter, 
						pNativeFtk,
						(const C_ScenePointLightNode*)pNode,
						true);
					if (!status) return status;
					break;
				}
				case E_ObjType_SceneAreaLight:
				{
					status = C_NativeSceneAreaLightNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneAreaLightNode*)pNode, 
						true);
					if (!status) return status;
					break;
				}
				case E_ObjType_SceneDirectionalLight:
				{
					status = C_NativeSceneDirectionalLightNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneDirectionalLightNode*)pNode, 
						true);
					if (!status) return status;
					break;
				}
				case E_ObjType_SceneSpotLight:
				{
					status = C_NativeSceneSpotLightNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneSpotLightNode*)pNode,
						true);
					if (!status) return status;
					break;
				}
				case E_ObjType_SceneHemisphereLight:
				{
					status = C_NativeSceneHemisphereLightNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneHemisphereLightNode*)pNode,
						true);
					if (!status) return status;
					break;
				}
				case E_ObjType_SceneXRef:
				{
					status = C_NativeSceneXRefNode::Serialise(
						pDiskWriter,
						pNativeFtk, 
						(const C_SceneXRefNode*)pNode,
						true);
					if (!status) return status;
					break;
				}
				case E_ObjType_SceneInstance:
				{
					status = C_NativeSceneInstanceNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneInstanceNode*)pNode, 
						true);
					if (!status) return status;
					break;
				}
				case E_ObjType_SceneConstraint:
				{
					status = C_NativeSceneConstraintNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneConstraintNode*)pNode);
					if (!status) return status;
					break;
				}
				case E_ObjType_SceneAimConstraint:
				{
					status = C_NativeSceneAimConstraintNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneAimConstraintNode*)pNode);
					if (!status) return status;
					break;
				}
				case E_ObjType_SceneLookAtConstraint:
				{
					status = C_NativeSceneLookAtConstraintNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneLookAtConstraintNode*)pNode);
					if (!status) return status;
					break;
				}
				case E_ObjType_ScenePointConstraint:
				{
					status = C_NativeScenePointConstraintNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_ScenePointConstraintNode*)pNode);
					if (!status) return status;
					break;
				}
				case E_ObjType_SceneOrientConstraint:
				{
					status = C_NativeSceneOrientConstraintNode::Serialise(
						pDiskWriter, 
						pNativeFtk, 
						(const C_SceneOrientConstraintNode*)pNode);
					if (!status) return status;
					break;
				}
			}
			m_numProcessingStagesCompleted++;
			F32 progress = ((F32)m_numProcessingStagesCompleted / (F32)m_numProcessingStages) * 100.0f;
			if (pProgress && pProgress->OnProgress(progress))
				return C_Status(C_Status::E_Status_Failure, "Scene writing aborted.");
		}
	}
	pDiskWriter->EndBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::WriteScene(
	C_NativeDiskWriter* pDiskWriter,
	C_NativeFtk* pNativeFtk,
	const C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;

	//Write out scene data
	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_SCENE);
	{
		status = WriteScenePrototype(pDiskWriter, pNativeFtk, pScene, pProgress);
		if (!status) return status;

		*pDiskWriter <<	pNativeFtk->GetIdFromPtr(pScene->m_pRootSceneNode);
		*pDiskWriter << pScene->m_nextSceneNodeUniqueId;

		*pDiskWriter << std::string(pScene->m_origin.c_str());

		//Write out the material nodes
		status = WriteMaterials(pDiskWriter, pNativeFtk, pScene, pProgress);
		if (!status) return status;

		//Write out the blind data nodes
		status = WriteBlindData(pDiskWriter, pNativeFtk, pScene, pProgress);
		if (!status) return status;

		status = WriteUnmanagedDgNodes(pDiskWriter, pNativeFtk, pScene, pProgress);
		if (!status) return status;

		status = WriteXRefs(pDiskWriter, pNativeFtk, pScene, pProgress);
		if (!status) return status;

		//Write out blind data.
		{
			if (pScene->m_pSceneBlindDataResource == 0)
			{
				*pDiskWriter << S32(-1);
			}
			else
			{
				//store the instance of the blind data resource, not its shared resource pointer
				Ut::UtMap<const C_BlindDataResource*, U32>::Iterator bdIt;
				bdIt = pNativeFtk->m_pBlindDataResToIthIndexMapping->Find(pScene->m_pSceneBlindDataResource);
				if (bdIt == pNativeFtk->m_pBlindDataResToIthIndexMapping->End())
				{
					LOGERROR(g_aftkMainLogger, "Corrupt blind-data iterator.\n");
					AFTK_ASSERT(bdIt != pNativeFtk->m_pBlindDataResToIthIndexMapping->End());
					return C_Status::E_Status_Failure;
				}
				S32 ithIndex = bdIt->GetValue();
				*pDiskWriter << ithIndex;
			}
		}
	}
	// End BLOCK_ID_SCENE
	pDiskWriter->EndBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::ReadScenePrototype(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk,
	C_Scene* pScene,
	I_Progress* pProgress)
{
	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_PROTOTYPE))
		return C_Status(C_Status::E_Status_Failure, "Could not find prototype block.");

	C_Status status;
	U32 numMaterialNodes = 0;
	U32 numBlindDataNodes = 0;
	U32 numUnmanagedDgNodes = 0;
	//NATIVE_BLOCK_ID_PROTOTYPE
	pDiskReader->EnterBlock();
	{
		S32 blockId;
		while (blockId = pDiskReader->GetNextBlockInfo())
		{
			if (blockId == NATIVE_BLOCK_ID_SCENE)
			{
				//C_Scene proporties
				pDiskReader->EnterBlock();
				{
					*pDiskReader >> numMaterialNodes;
					*pDiskReader >> numBlindDataNodes;
					*pDiskReader >> numUnmanagedDgNodes;
				}
				pDiskReader->ExitBlock();
			}
			//Read in animation timing information.
			else if (blockId == NATIVE_BLOCK_ID_ANIMATION_TIMING)
			{
				pDiskReader->EnterBlock();
				{
					U32 timing;
					*pDiskReader >> timing;

					F32 startTime;
					*pDiskReader >> startTime;
					pScene->SetStartTime(startTime);

					F32 endTime;
					*pDiskReader >> endTime;
					pScene->SetEndTime(endTime);

					F32 frameRate;
					*pDiskReader >> frameRate;
					pScene->SetFrameRate(frameRate);
				}
				pDiskReader->ExitBlock();
			}
			//Read in memory manager cues
			else if (blockId == NATIVE_BLOCK_ID_MEMORY_MANAGER_CUES)
			{
				pDiskReader->EnterBlock();
				{
					status = ReadMemoryManagerCues(
						pDiskReader,
						pNativeFtk,
						pScene,
						pProgress);
					if (!status) return status;
				}
				pDiskReader->ExitBlock();
			}
			else
			{
				// We've encountered an unknown block, skip it.
				pDiskReader->SkipBlock();
			}
		}
	}
	// End BLOCK_ID_PROTOTYPE
	pDiskReader->ExitBlock();

	m_numProcessingStages = numBlindDataNodes;
	m_numProcessingStages += numMaterialNodes;
	m_numProcessingStages += numUnmanagedDgNodes;
	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::ReadScenePrototype176(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk,
	C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;

	//C_Node counts
	U32 numNulls		= 0;
	U32 numMeshes		= 0;
	U32 numSplines		= 0;
	U32 numGroups		= 0;
	U32 numLights		= 0;
	U32 numInstances	= 0;
	U32 numXRefs		= 0;
	U32 numActors		= 0;
	U32 numCameras		= 0;
	U32 numSceneNodes	= 0;

	//Mesh counts
	U32 numMeshVertexPositions	= 0;
	U32 numMeshVertexNormals	= 0;
	U32 numMeshVertexColours	= 0;
	U32 numMeshPolygonVertices	= 0;
	U32 numMeshPolygons			= 0;

	//Spline counts
	U32 numSplineCVs = 0;

	U32 numMaterialNodes;
	U32 numBlindDataNodes;
	U32 numMaps;

	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_PROTOTYPE))
	{
		return C_Status(C_Status::E_Status_Failure, "Could not find prototype block.");
	}

	//NATIVE_BLOCK_ID_PROTOTYPE
	pDiskReader->EnterBlock();
	{
		U32 blockId;
		while (blockId = pDiskReader->GetNextBlockInfo())
		{
			if (blockId == NATIVE_BLOCK_ID_SCENE)
			{
				//C_Scene proporties
				pDiskReader->EnterBlock();
				{
					*pDiskReader >> numMaterialNodes;
					*pDiskReader >> numBlindDataNodes;
					*pDiskReader >> numMaps;
				}
				pDiskReader->ExitBlock();
			}
			else if (blockId == NATIVE_BLOCK_ID_SCENE_NODE)
			{
				pDiskReader->EnterBlock();
				{
					//Get the number of null scene nodes
					*pDiskReader >> numNulls;
				}
				pDiskReader->ExitBlock();
			}
			else if (blockId == NATIVE_BLOCK_ID_MESH_NODE)
			{
				pDiskReader->EnterBlock();
				{
					//Get the number of meshes
					*pDiskReader >> numMeshes;
					//Get mesh data counts
					*pDiskReader >> numMeshVertexPositions;
					*pDiskReader >> numMeshVertexNormals;
					*pDiskReader >> numMeshVertexColours;
					*pDiskReader >> numMeshPolygonVertices;
					*pDiskReader >> numMeshPolygons;
				}
				pDiskReader->ExitBlock();
			}
			else if (blockId == NATIVE_BLOCK_ID_SPLINE_NODE)
			{
				pDiskReader->EnterBlock();
				{
					//Get the number of splines
					*pDiskReader >> numSplines;
					//Get spline data counts
					*pDiskReader >> numSplineCVs;
				}
				pDiskReader->ExitBlock();
			}
			else if (blockId == NATIVE_BLOCK_ID_GROUP_NODE)
			{
				pDiskReader->EnterBlock();
				{
					//Get the number of groups
					*pDiskReader >> numGroups;
				}
				pDiskReader->ExitBlock();
			}
			else if (blockId == NATIVE_BLOCK_ID_LIGHT_NODE)
			{
				pDiskReader->EnterBlock();
				{
					//Get the number of lights
					*pDiskReader >> numLights;
				}
				pDiskReader->ExitBlock();
			}
			else if (blockId == NATIVE_BLOCK_ID_INSTANCE_NODE)
			{
				pDiskReader->EnterBlock();
				{
					//Get the number of instances
					*pDiskReader >> numInstances;
				}
				pDiskReader->ExitBlock();
			}
			else if (blockId == NATIVE_BLOCK_ID_XREF_NODE)
			{
				pDiskReader->EnterBlock();
				{
					//Get the number of X-refs
					*pDiskReader >> numXRefs;
				}
				pDiskReader->ExitBlock();
			}
			else if (blockId == NATIVE_BLOCK_ID_ACTOR_NODE)
			{
				pDiskReader->EnterBlock();
				{
					//Get the number of actors
					*pDiskReader >> numActors;
				}
				pDiskReader->ExitBlock();
			}
			else if (blockId == NATIVE_BLOCK_ID_CAMERA_NODE)
			{
				pDiskReader->EnterBlock();
				{
					*pDiskReader >> numCameras;
				}
				pDiskReader->ExitBlock();
			}
			//Read in animation timing information.
			else if (
				(blockId == NATIVE_BLOCK_ID_ANIMATION_TIMING)
				&&
				(pDiskReader->GetVersion() >= 144))
			{
				pDiskReader->EnterBlock();
				{
					U32 timing;
					*pDiskReader >> timing;

					F32 startTime;
					*pDiskReader >> startTime;
					pScene->SetStartTime(startTime);

					F32 endTime;
					*pDiskReader >> endTime;
					pScene->SetEndTime(endTime);

					F32 frameRate;
					*pDiskReader >> frameRate;
					pScene->SetFrameRate(frameRate);
				}
				pDiskReader->ExitBlock();
			}
			//Read in memory manager cues
			else if (blockId == NATIVE_BLOCK_ID_MEMORY_MANAGER_CUES)
			{
				pDiskReader->EnterBlock();
				{
					status = ReadMemoryManagerCues(
						pDiskReader,
						pNativeFtk,
						pScene,
						pProgress);
					if (!status) return status;
				}
				pDiskReader->ExitBlock();
				break;
			}
			else
			{
				// We've encountered an unknown block, skip it.
				pDiskReader->SkipBlock();
				break;
			}
		}
	}

	// End BLOCK_ID_PROTOTYPE
	pDiskReader->ExitBlock();

	// Let's work out how much work we have to do to load this scene based upon its prototype
	m_numProcessingStagesCompleted = 0;

	m_numProcessingStages = numBlindDataNodes;
	m_numProcessingStages += numMaterialNodes;
	m_numProcessingStages += numSceneNodes;

	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::ReadScene(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk,
	C_Scene* pScene,
	Matrix44& sceneTransform,
	I_Progress* pProgress)
{
	C_Status status;

	//Read in scene data
	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_SCENE))
		return C_Status(C_Status::E_Status_Failure, "Could not find scene block.");

	pDiskReader->EnterBlock();
	{
		status = ReadScenePrototype(pDiskReader, pNativeFtk, pScene, pProgress);
		if (!status) return status;

		S32 rootNodeId;
		*pDiskReader >> rootNodeId;
		#pragma warning(push)
		#pragma warning(disable : 4312)
		pScene->m_pRootSceneNode = reinterpret_cast<C_SceneNode*>(rootNodeId);
		#pragma warning(pop)

		*pDiskReader >> pNativeFtk->m_nextSceneNodeUniqueId;

		if (pDiskReader->GetVersion() < 202)
		{
			sceneTransform;
			*pDiskReader >> sceneTransform;
		}
		else
		{
			UtMaths::SetIdentity(&sceneTransform);
		}

		std::string blindDataOrigin;
		*pDiskReader >> blindDataOrigin;
		pScene->SetOrigin(blindDataOrigin);

		//Read in the material nodes
		status = ReadMaterials(
			pDiskReader, 
			pNativeFtk, 
			pScene, 
			pProgress);
		if (!status) return status;

		//Read in the blind data nodes
		status = ReadBlindData(
			pDiskReader, 
			pNativeFtk, 
			pScene, 
			pProgress);
		if (!status) return status;

		//Read in the unmanaged dg contents.
		status = ReadUnmanagedDgNodes(
			pDiskReader, 
			pNativeFtk, 
			pScene, 
			pProgress);
		if (!status) return status;

		status = ReadXRefs(pDiskReader, pNativeFtk, pScene, pProgress);
		if (!status) return status;

		//Fetch the instance of the blind data resource, not its shared resource pointer
		S32 blindDataIndex;
		*pDiskReader >> blindDataIndex;
		
		if (blindDataIndex != -1)
		{
			//Fetch the blind data corresponding to this instance
			pScene->m_pSceneBlindDataResource = 
				pNativeFtk->m_blindDataMappingArray[blindDataIndex];
			if (pScene->m_pSceneBlindDataResource == 0)
			{
				//We couldn't fetch the blind data instance. Perhaps the file is corrupt. Abort.
				return C_Status(C_Status::E_Status_Failure, "Failed to fetch blind data instance.");
			}
		}
	}
	pDiskReader->ExitBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::ReadMaterials(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk,
	C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;

	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_MATERIAL_LIST))
	{
		return C_Status(C_Status::E_Status_Failure, "Failed to find material list block.");
	}
	//NATIVE_BLOCK_ID_MATERIAL_LIST
	AFTK_ASSERT(pScene->GetNumMaterialNodes() == 0);
	pDiskReader->EnterBlock();
	{
		U32 numMaterialNodes;
		//Read in the number of materials
		*pDiskReader >> numMaterialNodes;
		//Loop thru material list
		for (U32 i = 0 ; i < numMaterialNodes; i++) 
		{
			//Destination material
			C_MaterialResource* pRes = new C_MaterialResource();
			
			status = C_NativeMaterialNode::Deserialise(
				pDiskReader, 
				pNativeFtk, 
				&pRes->GetContent(), 
				true);
			if (!status) return status;
			pScene->AddMaterialNodeNoTest(pRes);

			m_numProcessingStagesCompleted++;
			F32 progress = ((F32)m_numProcessingStagesCompleted / (F32)m_numProcessingStages) * 100.0f;
			if (pProgress && pProgress->OnProgress(progress))
				return C_Status(C_Status::E_Status_Failure, "Scene reading aborted.");
		}
		AFTK_ASSERT(numMaterialNodes == pScene->GetNumMaterialNodes());
	}
	pDiskReader->ExitBlock();
	
	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::ReadBlindData(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;
	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_BLIND_DATA_LIST))
	{
		return C_Status(C_Status::E_Status_Failure, "Failed to find blind data list.");
	}
	//NATIVE_BLOCK_ID_BLIND_DATA_LIST
	pDiskReader->EnterBlock();
	{
		U32 numBlindDataNodes;
		//Read in the number of blind data nodes
		*pDiskReader >> numBlindDataNodes;
		//Loop thru blind data list

		pNativeFtk->m_blindDataMappingArray.Resize(numBlindDataNodes);
		for (U32 i = 0 ; i < numBlindDataNodes; i++) 
		{
			//Read in blind data
			C_BlindDataResource* pRes = new C_BlindDataResource();

			//This mapping table will be used to rebuild the blind data resource manager.
			pNativeFtk->m_blindDataMappingArray[i] = pRes;
			
			status = C_NativeBlindDataNode::Deserialise(
				pDiskReader, 
				pNativeFtk, 
				&pRes->GetContent(),
				true);
			if (!status) return status;

			pScene->AddBlindDataNodeNoTest(pRes);

			m_numProcessingStagesCompleted++;
			F32 progress = ((F32)m_numProcessingStagesCompleted / (F32)m_numProcessingStages) * 100.0f;
			if (pProgress)
			{
				//pProgress->OnPrintProgress(std::string("Read node ") + pSceneNode->GetName());
				if (pProgress->OnProgress(progress))
				{
					return C_Status(C_Status::E_Status_Failure, "Scene reading aborted.");
				}
			}
		}
		AFTK_ASSERT(numBlindDataNodes == pScene->GetNumBlindDataNodes());
	}
	pDiskReader->ExitBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::ReadXRefs(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;

	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_XREF_LIST))
	{
		return C_Status(C_Status::E_Status_Failure, "Failed to find xref list block.");
	}

	pDiskReader->EnterBlock();
	{
		U32 numXRefScenes;
		*pDiskReader >> numXRefScenes;
		for (U32 iScene = 0; iScene < numXRefScenes; iScene++)
		{
			status = C_NativeXRefScene::Deserialise(
				pDiskReader,
				pNativeFtk,
				pScene);
			if (!status) return status;
		}
	}
	pDiskReader->ExitBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::WriteXRefs(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;

	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_XREF_LIST);
	{
		*pDiskWriter << pScene->GetNumXRefScenes();
		for (U32 iScene = 0; iScene < pScene->GetNumXRefScenes(); iScene++)
		{
			status = C_NativeXRefScene::Serialise(
				pDiskWriter,
				pNativeFtk,
				pScene->GetXRefScenePtr(iScene));
			if (!status) return status;
		}
	}
	pDiskWriter->EndBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::ReadUnmanagedDgNodes(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk,
	C_Scene* pScene,
	I_Progress* pProgress)
{
	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_UNMANAGED_DG_NODE_LIST))
		return C_Status::E_Status_Success;
	C_Status status;

	pDiskReader->EnterBlock();
	{
		U32 blockId;
		while (blockId = pDiskReader->GetNextBlockInfo())
		{
			//////////////// UNMANAGED DG NODES ////////////////
			if (blockId == NATIVE_BLOCK_ID_ANIMATION_CURVE_NODE)
			{
				C_AnimationCurveNode* pNode = new C_AnimationCurveNode(pScene);
				if (!pNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create node.");

				status = C_NativeAnimationCurveNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pNode, 
					true);
				if (!status) return status;
			}

			else if (blockId == NATIVE_BLOCK_ID_BLEND_WEIGHTED_NODE)
			{
				C_BlendWeightedNode* pNode = new C_BlendWeightedNode(pScene);
				if (!pNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create node.");

				status = C_NativeBlendWeightedNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pNode, 
					true);
				if (!status) return status;
			}

			else if (blockId == NATIVE_BLOCK_ID_DISTANCE_DIM_NODE)
			{
				C_DistanceDimNode* pNode = new C_DistanceDimNode(pScene);
				if (!pNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create node.");

				status = C_NativeDistanceDimNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pNode, 
					true);
				if (!status) return status;
			}

			else if (blockId == NATIVE_BLOCK_ID_UNIT_CONVERSION_NODE)
			{
				C_UnitConversionNode* pNode = new C_UnitConversionNode(pScene);
				if (!pNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create node.");

				status = C_NativeUnitConversionNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pNode, 
					true);
				if (!status) return status;
			}

			else if (blockId == NATIVE_BLOCK_ID_INFLUENCE_CLUSTER_NODE)
			{
				C_InfluenceClusterNode* pNode = new C_InfluenceClusterNode(pScene);
				if (!pNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create node.");

				status = C_NativeInfluenceClusterNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pNode, 
					true);
				if (!status) return status;
			}

			else if (blockId == NATIVE_BLOCK_ID_LAYER_NODE)
			{
				C_LayerNode* pNode = new C_LayerNode(pScene);
				if (!pNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create node.");

				status = C_NativeLayerNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pNode, 
					true);
				if (!status) return status;
			}

			//////////////// SCENE GRAPH NODES ////////////////
			else if (blockId == NATIVE_BLOCK_ID_SCENE_NODE)
			{
				C_SceneNode* pSceneNode = new C_SceneNode(pScene);
				if (!pSceneNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create scene node.");

				status = C_NativeSceneNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pSceneNode, 
					true);
				if (!status) return status;
			}
			else if (blockId == NATIVE_BLOCK_ID_CAMERA_NODE)
			{
				//Destination
				C_SceneCameraNode* pSceneCameraNode = new C_SceneCameraNode(pScene);
				if (!pSceneCameraNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create camera node.");
				//Read in
				status = C_NativeSceneCameraNode::Deserialise(
					pDiskReader,
					pNativeFtk, 
					pSceneCameraNode,
					true);
				if (!status)
				{
					delete pSceneCameraNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_SCENE_JOINT_NODE)
			{
				//Destination
				C_SceneJointNode* pSceneJointNode = new C_SceneJointNode(pScene);
				if (!pSceneJointNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create joint node.");

				//Read in
				status = C_NativeSceneJointNode::Deserialise(
					pDiskReader,
					pNativeFtk, 
					pSceneJointNode, 
					true);
				if (!status) 
				{
					delete pSceneJointNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_SCENE_LOD_GROUP_NODE)
			{
				//Destination
				C_SceneLodGroupNode* pSceneLodGroupNode = new C_SceneLodGroupNode(pScene);
				if (!pSceneLodGroupNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create lod group node.");

				//Read in
				status = C_NativeSceneLodGroupNode::Deserialise(
					pDiskReader, 
					pNativeFtk,
					pSceneLodGroupNode, true);
				if (!status)
				{
					delete pSceneLodGroupNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_MESH_NODE)
			{
				//Destination
				C_SceneMeshNode* pSceneMeshNode = new C_SceneMeshNode(pScene);
				if (!pSceneMeshNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create mesh node.");

				//Read in
				status = C_NativeSceneMeshNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pSceneMeshNode, true);
				if (!status)
				{
					delete pSceneMeshNode;
					return status;
				}

			}
			else if (blockId == NATIVE_BLOCK_ID_MESH_BONED_INSTANCE_NODE)
			{
				//Destination
				C_SceneMeshBonedInstanceNode* pSceneMeshBonedInstanceNode = 
					new C_SceneMeshBonedInstanceNode(pScene);
				if (!pSceneMeshBonedInstanceNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create mesh boned instance node.");

				//Read in
				status = C_NativeSceneMeshBonedInstanceNode::Deserialise(
					pDiskReader, 
					pNativeFtk,
					pSceneMeshBonedInstanceNode,
					true);
				if (!status)
				{
					delete pSceneMeshBonedInstanceNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_MESH_REMAPPED_INSTANCE_NODE)
			{
				//Destination
				C_SceneMeshRemappedInstanceNode* pSceneMeshRemappedInstanceNode = new C_SceneMeshRemappedInstanceNode(pScene);
				if (!pSceneMeshRemappedInstanceNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create mesh remapped instance node.");

				//Read in
				status = C_NativeSceneMeshRemappedInstanceNode::Deserialise(
					pDiskReader, 
					pNativeFtk,
					pSceneMeshRemappedInstanceNode,
					true);
				if (!status)
				{
					delete pSceneMeshRemappedInstanceNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_SPLINE_NODE)
			{
				//Destination
				C_SceneSplineNode* pSceneSplineNode = new C_SceneSplineNode(pScene);
				if (!pSceneSplineNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create spline node.");

				//Read in
				status = C_NativeSceneSplineNode::Deserialise(
					pDiskReader,
					pNativeFtk,
					pSceneSplineNode,
					true);
				if (!status)
				{
					delete pSceneSplineNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_POINT_LIGHT_NODE)
			{
				C_ScenePointLightNode* pScenePointLightNode = new C_ScenePointLightNode(pScene);
				if (!pScenePointLightNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create point light node.");

				status = C_NativeScenePointLightNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pScenePointLightNode, 
					true);
				if (!status)
				{
					delete pScenePointLightNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_AREA_LIGHT_NODE)
			{
				C_SceneAreaLightNode* pSceneAreaLightNode = new C_SceneAreaLightNode(pScene);
				if (!pSceneAreaLightNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create area light node.");

				status = C_NativeSceneAreaLightNode::Deserialise(
					pDiskReader, 
					pNativeFtk,
					pSceneAreaLightNode, 
					true);
				if (!status)
				{
					delete pSceneAreaLightNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_DIRECTIONAL_LIGHT_NODE)
			{
				C_SceneDirectionalLightNode* pSceneDirectionalLightNode = new C_SceneDirectionalLightNode(pScene);
				if (!pSceneDirectionalLightNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create directional light node.");

				status = C_NativeSceneDirectionalLightNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pSceneDirectionalLightNode, 
					true);
				if (!status)
				{
					delete pSceneDirectionalLightNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_SPOT_LIGHT_NODE)
			{
				C_SceneSpotLightNode* pSceneSpotLightNode = new C_SceneSpotLightNode(pScene);
				if (!pSceneSpotLightNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create spot light node.");

				status = C_NativeSceneSpotLightNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pSceneSpotLightNode, 
					true);
				if (!status)
				{
					delete pSceneSpotLightNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_HEMISPHERE_LIGHT_NODE)
			{
				C_SceneHemisphereLightNode* pSceneHemisphereLightNode = new C_SceneHemisphereLightNode(pScene);
				if (!pSceneHemisphereLightNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create hemisphere light node.");

				status = C_NativeSceneHemisphereLightNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pSceneHemisphereLightNode,
					true);
				if (!status)
				{
					delete pSceneHemisphereLightNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_XREF_NODE)
			{
				C_SceneXRefNode* pSceneXRefNode = new C_SceneXRefNode(pScene);
				if (!pSceneXRefNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create xref node.");

				status = C_NativeSceneXRefNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pSceneXRefNode, 
					true);
				if (!status)
				{
					delete pSceneXRefNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_INSTANCE_NODE)
			{
				//Destination
				C_SceneInstanceNode* pSceneInstanceNode = new C_SceneInstanceNode(pScene);
				if (!pSceneInstanceNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create instance node.");

				//Read in
				status = C_NativeSceneInstanceNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pSceneInstanceNode, 
					true);
				if (!status)
				{
					delete pSceneInstanceNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_CONSTRAINT_NODE)
			{
				//Destination
				C_SceneConstraintNode* pSceneConstraintNode = new C_SceneConstraintNode(pScene);
				if (!pSceneConstraintNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create constraint node.");

				//Read in
				status = C_NativeSceneConstraintNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pSceneConstraintNode);
				if (!status)
				{
					delete pSceneConstraintNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_AIM_CONSTRAINT_NODE)
			{
				//Destination
				C_SceneAimConstraintNode* pSceneAimConstraintNode = new C_SceneAimConstraintNode(pScene);
				if (!pSceneAimConstraintNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create aim constraint node.");

				//Read in
				status = C_NativeSceneAimConstraintNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pSceneAimConstraintNode);
				if (!status)
				{
					delete pSceneAimConstraintNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_LOOK_AT_CONSTRAINT_NODE)
			{
				//Destination
				C_SceneLookAtConstraintNode* pSceneLookAtConstraintNode = new C_SceneLookAtConstraintNode(pScene);
				if (!pSceneLookAtConstraintNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create look at constraint node.");

				//Read in
				status = C_NativeSceneLookAtConstraintNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pSceneLookAtConstraintNode);
				if (!status)
				{
					delete pSceneLookAtConstraintNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_POINT_CONSTRAINT_NODE)
			{
				//Destination
				C_ScenePointConstraintNode* pScenePointConstraintNode = new C_ScenePointConstraintNode(pScene);
				if (!pScenePointConstraintNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create point constraint node.");

				//Read in
				status = C_NativeScenePointConstraintNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pScenePointConstraintNode);
				if (!status)
				{
					delete pScenePointConstraintNode;
					return status;
				}
			}
			else if (blockId == NATIVE_BLOCK_ID_ORIENT_CONSTRAINT_NODE)
			{
				//Destination
				C_SceneOrientConstraintNode* pSceneOrientConstraintNode = new C_SceneOrientConstraintNode(pScene);
				if (!pSceneOrientConstraintNode)
					return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create orient constraint node.");

				//Read in
				status = C_NativeSceneOrientConstraintNode::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pSceneOrientConstraintNode);
				if (!status)
				{
					delete pSceneOrientConstraintNode;
					return status;
				}
			}
			else 
			{
				/*
				//Unknown block type. We assume that it's some new scene node type that this version
				//of the file reader is unaware of. As all scene nodes derive from C_Node and C_SceneNode
				//all is not lost. We can read this as if it were an C_SceneNode, or NULL node.
				pDiskReader->EnterBlock();
				status = ReadSceneNode(pDiskReader, pNativeFtk, pScene, pProgress);
				if (!status) return status;
				pDiskReader->ExitBlock();
				*/
				return C_Status(C_Status::E_Status_Failure, "Unknown node type encountered.");
			}

			m_numProcessingStagesCompleted++;
			F32 progress = ((F32)m_numProcessingStagesCompleted / (F32)m_numProcessingStages) * 100.0f;

			if (pProgress && pProgress->OnProgress(progress))
				return C_Status(C_Status::E_Status_Failure, "Scene reading aborted.");
		}
	}
	pDiskReader->ExitBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::ReadUnknownUnmanagedDgNode(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk,
	C_Scene* pScene,
	I_Progress* pProgress)
{
	C_Status status;
/*
	//Destination
	C_SceneNode* pSceneNode = new C_SceneNode(pScene);
	if (pSceneNode == 0)
	{
		return C_Status(C_Status::E_Status_OutOfMemory, "Failed to create scene node.");
	}

	//Read in
	status = C_NativeSceneNode::Deserialise(pDiskReader, pNativeFtk, pSceneNode, true);
	if (!status)
	{
		return status;
	}
*/
	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::WriteMemoryManagerCues(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_Scene* pScene,
	I_Progress* pProgress)
{
	*pDiskWriter << (U32)C_MemoryManager::GetNumPools();
	for (U32 iPool = 0; iPool < C_MemoryManager::GetNumPools(); iPool++)
	{
		Ut::SSSMemoryPool* pPool = C_MemoryManager::GetPoolPtr(iPool);
		U32 numBytes;
		if (!pPool)
		{
			numBytes = 0;
		}
		else
		{
			numBytes = pPool->SizeBytes();
		}
		*pDiskWriter << numBytes;
	}

	return C_Status::E_Status_Success;
}

C_Status C_NativeScene::ReadMemoryManagerCues(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_Scene* pScene, 
	I_Progress* pProgress)
{
	U32 numPools;
	*pDiskReader >> numPools;
	for (U32 iPool = 0; iPool < numPools; iPool++)
	{
		U32 numBytes;
		*pDiskReader >> numBytes;
#ifdef AFTK_USE_MEMORY_POOL
		//C_MemoryManager::PoolReserve(iPool, numBytes);
#endif
	}

	return C_Status::E_Status_Success;
}

bool C_NativeScene::EvaluateUnmanagedPlugs(U32 version, C_NativeFtk* pNativeFtk, C_Scene* pScene)
{
	const Ut::UtSet<C_Node*>& dgSet = pScene->GetDgNodeSet();
	for (Ut::UtSet<C_Node*>::ConstIterator it = dgSet.Begin(); it != dgSet.End(); ++it)
	{
		C_Node* pNode = it->GetKey();
		for (U32 iPlug = 0; iPlug < pNode->GetNumPlugs(); iPlug++)
		{
			//Trigger initial evaluation of plugs.
			I_Plug* pPlug = pNode->GetPlugPtr(iPlug);
			if (pPlug) 
				pPlug->OnChanged();
		}
	}

	return true;
}

bool C_NativeScene::ResolveObjectPointers(U32 version, C_NativeFtk* pNativeFtk, C_Scene* pScene)
{
	const Ut::UtVector<Ut::DynamicVector<S32> >& objectsToResolve = pNativeFtk->GetObjectsToResolve();
	for (U32 i = 0; i < (U32)objectsToResolve.Size(); i++)
	{
		S32 id = objectsToResolve[i];
		I_Object* pObject = (I_Object*)pNativeFtk->GetPtrFromId(id);
		if (!ResolveObjectPointer(version, pNativeFtk, id, pObject))
			return false;
	}

	#pragma warning(push)
		#pragma warning(disable: 4311)
		//Finally, lets get our scene root node.
		S32 id = reinterpret_cast<S32>(pScene->GetRootSceneNodePtr());
		pScene->SetRootSceneNodePtr((C_SceneNode*)pNativeFtk->GetPtrFromId(id));
	#pragma warning(pop)
	return true;
}

bool C_NativeScene::ResolveObjectPointer(
	U32 version,
	C_NativeFtk* pNativeFtk, 
	S32 id, 
	I_Object* pObject)
{
	if (pObject->HasType(E_ObjType_IPlug))
	{
		if (!C_NativeIPlug::ResolveUnmanagedDgNodePointers(
			version,
			pNativeFtk, 
			(I_Plug*)pObject))
			return false;
	}

	if (pObject->HasType(E_ObjType_Layer))
	{
		if (!C_NativeLayerNode::ResolveUnmanagedDgNodePointers(
			version,
			pNativeFtk, 
			(C_LayerNode*)pObject))
			return false;
	}

	if (pObject->HasType(E_ObjType_InfluenceCluster))
	{
		if (!C_NativeInfluenceClusterNode::ResolveUnmanagedDgNodePointers(
			version,
			pNativeFtk, 
			(C_InfluenceClusterNode*)pObject))
			return false;
	}

	if (pObject->HasType(E_ObjType_SceneNode))
	{
		if (!C_NativeSceneNode::ResolveUnmanagedDgNodePointers(
			version,
			pNativeFtk, 
			(C_SceneNode*)pObject))
			return false;
	}

	if (pObject->HasType(E_ObjType_SceneInstance))
	{
		if (!C_NativeSceneInstanceNode::ResolveUnmanagedDgNodePointers(
			version,
			pNativeFtk, 
			(C_SceneInstanceNode*)pObject))
			return false;
	}

	if (pObject->HasType(E_ObjType_SceneMeshRemappedInstance))
	{
		if (!C_NativeSceneMeshRemappedInstanceNode::ResolveUnmanagedDgNodePointers(
			version,
			pNativeFtk, 
			(C_SceneMeshBonedInstanceNode*)pObject))
			return false;
	}

	if (pObject->HasType(E_ObjType_SceneMeshBonedInstance))
	{
		if (!C_NativeSceneMeshBonedInstanceNode::ResolveUnmanagedDgNodePointers(
			version,
			pNativeFtk, 
			(C_SceneMeshBonedInstanceNode*)pObject))
			return false;
	}

	if (pObject->HasType(E_ObjType_SceneLodGroup))
	{
		if (!C_NativeSceneLodGroupNode::ResolveUnmanagedDgNodePointers(
			version,
			pNativeFtk, 
			(C_SceneLodGroupNode*)pObject))
			return false;
	}

	return true;
}
