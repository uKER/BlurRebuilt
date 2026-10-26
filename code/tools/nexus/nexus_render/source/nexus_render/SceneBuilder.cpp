#include <nexus_render/stdafx.h>
#include <nexus_render/SceneBuilder.h>
#include <nexus_render/ExpandUniqueRemappedInstances.h>
#include <nexus_render/AftkUtility.h>

using namespace NexusRender;

Tu::Status SceneBuilder::Build(
	RenderNode** ppRootRenderNode,
	SceneRenderer* pSceneRenderer,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderInstanceNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderDirectionalLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderPointLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSpotLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderAreaLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSplineNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderMeshNodes,
	Ut::OpenHashMap<
		const void*, 
		RenderNode*, 
		Ut::HashKeyOpPtr >& aftkNodeToRenderNodeMapping,
		Aftk::C_Scene* pAftkScene,
	Nexus::Progress* pProgress)
{
	//Replicate the Aftk's scene graph using RenderNodes
	Tu::Status status;

	Aftk::C_SceneNode* pAftkThisNode = pAftkScene->GetRootSceneNodePtr();
	if (!pAftkThisNode) return Tu::Status::SUCCESS;

	renderNodes.Reserve(pAftkScene->GetNumSceneNodes());

	//Traverse hierachy
	*ppRootRenderNode = BuildNode(
		pAftkScene->GetRootSceneNodePtr(),
		pSceneRenderer,
		renderNodes,
		renderInstanceNodes,
		renderDirectionalLightNodes,
		renderPointLightNodes,
		renderSpotLightNodes,
		renderAreaLightNodes,
		renderSplineNodes,
		renderMeshNodes,
		aftkNodeToRenderNodeMapping,
		status, 
		pProgress);
	if (!status) return status;

	//Create instance associations
	status = BuildInstances(
		renderNodes,
		aftkNodeToRenderNodeMapping,
		pProgress);
	if (!status) return status;

	BuildSpecificNodeLists(
		pAftkScene,
		renderDirectionalLightNodes,
		renderPointLightNodes,
		renderSpotLightNodes,
		renderAreaLightNodes,
		renderSplineNodes,
		renderMeshNodes,
		aftkNodeToRenderNodeMapping);

	status = ExpandUniqueRemappedInstances::Expand(
		pSceneRenderer,
		renderNodes,
		pProgress);
	if (!status) return status;

	return Tu::Status::SUCCESS;
}

void SceneBuilder::BuildSpecificNodeLists(
	Aftk::C_Scene* pAftkScene,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderDirectionalLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderPointLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSpotLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderAreaLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSplineNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderMeshNodes,
	Ut::OpenHashMap<
		const void*, 
		RenderNode*, 
		Ut::HashKeyOpPtr >& aftkNodeToRenderNodeMapping)
{
	Aftk::C_SceneNodeIterator it(pAftkScene);
	for (; it; it++)
	{
		Aftk::C_SceneNode* pAftkSourceNode = AftkUtility::GetIndirectSourcePtr(it);
		Ut::OpenHashMap<
			const void*, 
			RenderNode*, 
			Ut::HashKeyOpPtr >::Iterator hashIt;
		if (!pAftkSourceNode)
			continue;

		//Store seperate lists for specified node types and their instances.
		switch (pAftkSourceNode->GetType())
		{
		case Aftk::E_ObjType_ScenePointLight:
			hashIt = aftkNodeToRenderNodeMapping.Find(it);
			if (hashIt != aftkNodeToRenderNodeMapping.End())
				renderPointLightNodes.PushBack(hashIt->GetValue());
			break;

		case Aftk::E_ObjType_SceneDirectionalLight:
			hashIt = aftkNodeToRenderNodeMapping.Find(it);
			if (hashIt != aftkNodeToRenderNodeMapping.End())
				renderDirectionalLightNodes.PushBack(hashIt->GetValue());
			break;

		case Aftk::E_ObjType_SceneSpotLight:
			hashIt = aftkNodeToRenderNodeMapping.Find(it);
			if (hashIt != aftkNodeToRenderNodeMapping.End())
				renderSpotLightNodes.PushBack(hashIt->GetValue());
			break;

		default:
			break;
		};
	}
}

RenderNode* SceneBuilder::BuildNode(
	Aftk::C_SceneNode* pAftkThisNode, 
	SceneRenderer* pSceneRenderer,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderInstanceNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderDirectionalLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderPointLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSpotLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderAreaLightNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSplineNodes,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderMeshNodes,
	Ut::OpenHashMap<
		const void*, 
		RenderNode*, 
		Ut::HashKeyOpPtr >& aftkNodeToRenderNodeMapping,
	Tu::Status& status, 
	Nexus::Progress* pProgress)
{
	RenderNode* pNewRenderNode;

	switch (pAftkThisNode->GetType())
	{
	case Aftk::E_ObjType_SceneNode:
		pNewRenderNode = new RenderNode(pSceneRenderer, pAftkThisNode);
		if (!pNewRenderNode)
		{
			status.Set(Tu::Status::OUT_OF_MEMORY);
			return 0;
		}
		break;

	case Aftk::E_ObjType_SceneMesh:
		{
			Aftk::C_SceneMeshNode* pAftkMesh = (Aftk::C_SceneMeshNode*)pAftkThisNode;
			if (pAftkMesh->GetNumPolygons(0) > 0)
			{
				pNewRenderNode = new RenderMeshNode(
					pSceneRenderer, 
					pAftkThisNode, 
					(Aftk::C_SceneMeshRemappableNode*)pAftkThisNode);
				renderMeshNodes.PushBack(pNewRenderNode);
			}
			else
			{
				//Create a null node if the mesh has no polygons.
				pNewRenderNode = new RenderNode(pSceneRenderer, pAftkThisNode);
			}

			if (!pNewRenderNode)
			{
				status.Set(Tu::Status::OUT_OF_MEMORY);
				return 0;
			}
		}
		break;

	case Aftk::E_ObjType_SceneMeshRemappedInstance:
		{
			Aftk::C_SceneMeshRemappedInstanceNode* pAftkInstance = 
				(Aftk::C_SceneMeshRemappedInstanceNode*)pAftkThisNode;
			if (pAftkInstance->GetSourceNodePtr())
			{
				Aftk::C_SceneMeshNode* pAftkMesh = (Aftk::C_SceneMeshNode*)pAftkInstance->GetSourceNodePtr();
				if (pAftkMesh->GetNumPolygons(0) == 0)
				{
					//Create a null node if the mesh has no polygons.
					pNewRenderNode = new RenderNode(pSceneRenderer, pAftkThisNode);
				}
				else
				{
					pNewRenderNode = new RenderMeshInstanceNode(
						pSceneRenderer, 
						pAftkThisNode,
						(Aftk::C_SceneMeshRemappableNode*)pAftkThisNode);
					renderMeshNodes.PushBack(pNewRenderNode);
				}
			}
			else
			{
				//Create a null node if the instance source is invalid.
				pNewRenderNode = new RenderNode(pSceneRenderer, pAftkThisNode);
			}
			if (!pNewRenderNode)
			{
				status.Set(Tu::Status::OUT_OF_MEMORY);
				return 0;
			}
		}
		break;

	case Aftk::E_ObjType_SceneMeshBonedInstance:
		{
			Aftk::C_SceneMeshBonedInstanceNode* pAftkInstance = 
				(Aftk::C_SceneMeshBonedInstanceNode*)pAftkThisNode;
			if (pAftkInstance->GetSourceNodePtr())
			{
				Aftk::C_SceneMeshNode* pAftkMesh = (Aftk::C_SceneMeshNode*)pAftkInstance->GetSourceNodePtr();
				if (pAftkMesh->GetNumPolygons(0) == 0)
				{
					//Create a null node if the mesh has no polygons.
					pNewRenderNode = new RenderNode(pSceneRenderer, pAftkThisNode);
				}
				else
				{
					pNewRenderNode = new RenderMeshBonedInstanceNode(
						pSceneRenderer, 
						pAftkThisNode,
						(Aftk::C_SceneMeshRemappableNode*)pAftkThisNode);
					renderMeshNodes.PushBack(pNewRenderNode);
				}
			}
			else
			{
				//Create a null node if the instance source is invalid.
				pNewRenderNode = new RenderNode(pSceneRenderer, pAftkThisNode);
			}
			if (!pNewRenderNode)
			{
				status.Set(Tu::Status::OUT_OF_MEMORY);
				return 0;
			}
		}
		break;

	case Aftk::E_ObjType_SceneSpline:
		{
			Aftk::C_SceneSplineNode* pSpline = 
				(Aftk::C_SceneSplineNode*)pAftkThisNode;
			if (pSpline->GetNumCVs() > 0)
			{
				pNewRenderNode = new RenderSplineNode(pSceneRenderer, pAftkThisNode);
			}
			else
			{
				//Create a null node if the spline has no CVs.
				pNewRenderNode = new RenderNode(pSceneRenderer, pAftkThisNode);
			}
			if (!pNewRenderNode)
			{
				status.Set(Tu::Status::OUT_OF_MEMORY);
				return 0;
			}
			renderSplineNodes.PushBack(pNewRenderNode);
		}
		break;

	case Aftk::E_ObjType_ScenePointLight:
		pNewRenderNode = new RenderPointLightNode(pSceneRenderer, pAftkThisNode);
		if (!pNewRenderNode)
		{
			status.Set(Tu::Status::OUT_OF_MEMORY);
			return 0;
		}
		renderPointLightNodes.PushBack(pNewRenderNode);
		break;

	case Aftk::E_ObjType_SceneInstance:
		{
			Aftk::C_SceneInstanceNode* pAftkInstance = 
				(Aftk::C_SceneInstanceNode*)pAftkThisNode;
			if (pAftkInstance->GetSourceNodePtr())
			{
				//Create mesh instance if needed.
				if (pAftkInstance->GetSourceNodePtr()->HasType(Aftk::E_ObjType_ISceneMesh))
				{
					pNewRenderNode = new RenderMeshInstanceNode(
						pSceneRenderer,
						pAftkThisNode, 
						(Aftk::C_SceneMeshRemappableNode*)pAftkThisNode);
					renderMeshNodes.PushBack(pNewRenderNode);
				}
				else
					pNewRenderNode = new RenderInstanceNode(pSceneRenderer, pAftkThisNode);
			}
			else
			{
				//Create a null node if the instance source is invalid.
				pNewRenderNode = new RenderNode(pSceneRenderer, pAftkThisNode);
				renderInstanceNodes.PushBack((RenderInstanceNode*)pNewRenderNode);
			}
			if (!pNewRenderNode)
			{
				status.Set(Tu::Status::OUT_OF_MEMORY);
				return 0;
			}
		}	
		break;

	case Aftk::E_ObjType_SceneDirectionalLight:
		pNewRenderNode = new RenderDirectionalLightNode(pSceneRenderer, pAftkThisNode);
		if (!pNewRenderNode)
		{
			status.Set(Tu::Status::OUT_OF_MEMORY);
			return 0;
		}
		renderDirectionalLightNodes.PushBack(pNewRenderNode);
		break;

	case Aftk::E_ObjType_SceneSpotLight:
		pNewRenderNode = new RenderSpotLightNode(pSceneRenderer, pAftkThisNode);
		if (!pNewRenderNode)
		{
			status.Set(Tu::Status::OUT_OF_MEMORY);
			return 0;
		}
		renderSpotLightNodes.PushBack(pNewRenderNode);
		break;

	case Aftk::E_ObjType_SceneAreaLight:
		pNewRenderNode = new RenderAreaLightNode(pSceneRenderer, pAftkThisNode);
		if (!pNewRenderNode)
		{
			status.Set(Tu::Status::OUT_OF_MEMORY);
			return 0;
		}
		renderAreaLightNodes.PushBack(pNewRenderNode);
		break;
		
	default:
		pNewRenderNode = new RenderNode(pSceneRenderer, pAftkThisNode);
		if (!pNewRenderNode)
		{
			status.Set(Tu::Status::OUT_OF_MEMORY);
			return 0;
		}
		break;
	};

	//All nodes are added to this master list.
	renderNodes.PushBack(pNewRenderNode);

	//Store mapping.
	aftkNodeToRenderNodeMapping[pAftkThisNode] = pNewRenderNode;

	//Traverse children
	Aftk::C_SceneNode* pChild = pAftkThisNode->GetChildPtr(0);
	while(pChild)
	{
		pNewRenderNode->AddChild(
			BuildNode(
				pChild,
				pSceneRenderer,
				renderNodes,
				renderInstanceNodes,
				renderDirectionalLightNodes,
				renderPointLightNodes,
				renderSpotLightNodes,
				renderAreaLightNodes,
				renderSplineNodes,
				renderMeshNodes,
				aftkNodeToRenderNodeMapping,
				status, 
				pProgress));
		if (!status) 
			return 0;
		pChild = pChild->GetNextPtr();
	}

	return pNewRenderNode;
}

Tu::Status SceneBuilder::BuildInstances(
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderNodes,
	Ut::OpenHashMap<
		const void*, 
		RenderNode*, 
		Ut::HashKeyOpPtr >& aftkNodeToRenderNodeMapping,
	Nexus::Progress* pProgress)
{
	Tu::Status status;

	for (U32 i = 0; i < (U32)renderNodes.Size(); i++)
	{
		RenderNode* pRenderNode = renderNodes[i];

		if (pRenderNode->GetType() == RenderNode::RENDER_NODE_TYPE_INSTANCE)
		{
			RenderInstanceNode* pGInstance = (RenderInstanceNode*)pRenderNode;
			const Aftk::C_SceneInstanceNode* pAftkInstance = 
				(const Aftk::C_SceneInstanceNode*)pGInstance->GetAftkSceneNodePtr();

			const Aftk::C_SceneNode* pAftkSource = pAftkInstance->GetSourceNodePtr();
			if (!pAftkSource)
				continue;

			Ut::OpenHashMap<
				const void*, 
				RenderNode*, 
				Ut::HashKeyOpPtr >::Iterator it;

			it = aftkNodeToRenderNodeMapping.Find(pAftkSource);
			ASSERT(it != aftkNodeToRenderNodeMapping.End());
			pGInstance->SetSourcePtr(it->GetValue());
			it->GetValue()->AddInstance(pGInstance);
		}
		else if (pRenderNode->HasType(RenderNode::RENDER_NODE_TYPE_MESH_INSTANCE))
		{
			RenderMeshInstanceNode* pMeshInstance = (RenderMeshInstanceNode*)pRenderNode;

			const Aftk::C_SceneNode* pAftkInstance = (const Aftk::C_SceneNode*)pMeshInstance->GetAftkSceneNodePtr();
			const Aftk::C_SceneNode* pAftkSource = 0;
			if (pAftkInstance->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
			{
				pAftkSource = ((Aftk::C_SceneMeshRemappedInstanceNode*)pAftkInstance)->GetSourceNodePtr();
				if (!pAftkSource)
					continue;
			}
			else if (pAftkInstance->GetType() == Aftk::E_ObjType_SceneInstance)
			{
				pAftkSource = ((Aftk::C_SceneInstanceNode*)pAftkInstance)->GetSourceNodePtr();
				if (!pAftkSource)
					continue;
			}
			else
			{
				ASSERT(false);
				continue;
			}
			Ut::OpenHashMap<
				const void*, 
				RenderNode*, 
				Ut::HashKeyOpPtr >::Iterator it;

			it = aftkNodeToRenderNodeMapping.Find(pAftkSource);
			ASSERT(it != aftkNodeToRenderNodeMapping.End());
			ASSERT(it->GetValue()->GetType() == RenderNode::RENDER_NODE_TYPE_MESH);
			pMeshInstance->SetSourcePtr((RenderMeshNode*)it->GetValue());
		}
	}

	return Tu::Status::SUCCESS;
}

