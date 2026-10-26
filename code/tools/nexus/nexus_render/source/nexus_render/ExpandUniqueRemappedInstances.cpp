#include <nexus_render/stdafx.h>
#include <nexus_render/ExpandUniqueRemappedInstances.h>

using namespace NexusRender;

bool ExpandUniqueRemappedInstances::KeysEqual(
	const Ut::UtVector<Ut::DynamicVector<U32> >& key1,
	const Ut::UtVector<Ut::DynamicVector<U32> >& key2)
{
	if (key1.Size() != key2.Size())
		return false;
	if (memcmp(&key1[0], &key2[0], key1.Size()*sizeof(U32)) != 0)
		return false;

	return true;
}

void ExpandUniqueRemappedInstances::GetUniqueKey(
	Ut::UtVector<Ut::DynamicVector<U32> >& uniqueMappingKey,
	const Aftk::C_SceneMeshRemappableNode* pRemappableInstance)
{
	U32 numMappings = 
		pRemappableInstance->GetNumPolygonMaterialMappings() +
		pRemappableInstance->GetNumPolygonBlindDataMappings();

	//Add material indices.
	for (U16 i = 0; i < pRemappableInstance->GetNumPolygonMaterialMappings(); i++)
	{
		const Aftk::C_MaterialResource* pRes =
			pRemappableInstance->GetPolygonMaterialMappingPtr(i);
		uniqueMappingKey.PushBack(pRes->GetIndex());
	}
	//Add blind data indices.
	for (U16 i = 0; i < pRemappableInstance->GetNumPolygonBlindDataMappings(); i++)
	{
		const Aftk::C_BlindDataResource* pRes =
			pRemappableInstance->GetPolygonBlindDataMappingPtr(i);
		uniqueMappingKey.PushBack(pRes->GetIndex());
	}
}


void ExpandUniqueRemappedInstances::GetUniqueKey(
	Ut::UtVector<Ut::DynamicVector<U32> >& uniqueMappingKey,
	const Aftk::C_SceneMeshRemappedInstanceNode* pRemappedInstance)
{
	//Add source instance.
	uniqueMappingKey.PushBack(pRemappedInstance->GetSourceNodePtr()->GetUniqueId());
	GetUniqueKey(uniqueMappingKey, (Aftk::C_SceneMeshRemappableNode*)pRemappedInstance);
}

void ExpandUniqueRemappedInstances::GetUniqueKey(
	Ut::UtVector<Ut::DynamicVector<U32> >& uniqueMappingKey,
	const Aftk::C_SceneMeshNode* pMesh)
{
	//Add instance.
	uniqueMappingKey.PushBack(pMesh->GetUniqueId());
	GetUniqueKey(uniqueMappingKey, (Aftk::C_SceneMeshRemappableNode*)pMesh);
}

Tu::Status ExpandUniqueRemappedInstances::Expand(
	SceneRenderer* pSceneRenderer,
	Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderNodes,
	Nexus::Progress* pProgress)
{
	if (pProgress)
	{
		pProgress->OnPrintProgress("Expanding unique remapped instances...\n");
		pProgress->OnProgress(0);
	}

	Aftk::C_Scene* pAftkScene = pSceneRenderer->GetAftkScenePtr();

	Ut::UtVector<Ut::DynamicVector<
		Ut::UtVector< Ut::DynamicVector<U32> > > > uniqueRemappings;

	Ut::UtVector< Ut::DynamicVector< 
		Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneMeshRemappedInstanceNode*> > > > instanceBuckets;

	//Note that we need to split on both blind data and material mappings.

	//Build a list of nodes to process.
	Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneMeshRemappedInstanceNode*> > nodesToProcess;
	nodesToProcess.Reserve(pAftkScene->GetNumSceneNodes());
	{
		Aftk::C_SceneNodeIterator it(pAftkScene);
		while(it)
		{
			Aftk::C_SceneNode* pSceneNode = it;
			if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
				nodesToProcess.PushBack((Aftk::C_SceneMeshRemappedInstanceNode*)pSceneNode);
			it++;
		}
	}
	Ut::UtVector<Ut::DynamicVector<U32> > uniqueMappingKey;
	//Reserve the absolute worst-case required ammount of memory to avoid dynamic allocs per node.
	uniqueMappingKey.Reserve(pAftkScene->GetNumMaterialNodes() + pAftkScene->GetNumBlindDataNodes() + 1);
	for (U32 iNode = 0; iNode < (U32)nodesToProcess.Size(); iNode++)
	{
		Aftk::C_SceneMeshRemappedInstanceNode* pRemappedInstance = nodesToProcess[iNode];

		//Create a block of memory representing the unique combination of materials on this object.
		//Note that we use the unique 32-bit material indices, not the mesh relative 16-bit indices.
		Ut::UtVector<Ut::DynamicVector<U32> > uniqueMappingKey;
		GetUniqueKey(uniqueMappingKey, pRemappedInstance);

        //Determine if we've encountered this key before.
		S32 existingBucketIndex = -1;
		for (U32 i = 0; i < (U32)uniqueRemappings.Size(); i++)
		{
			if (KeysEqual(uniqueMappingKey, uniqueRemappings[i]))
			{
				//Identical key found.
				existingBucketIndex = i;
				break;
			}
		}

		if (existingBucketIndex == -1)
		{
			existingBucketIndex = instanceBuckets.Size();
			//No bucket found, create one.
			instanceBuckets.PushBack();
			//Add new mapping
			uniqueRemappings.PushBack(uniqueMappingKey);
		}

		//Insert in to bucket.
		instanceBuckets[existingBucketIndex].PushBack(pRemappedInstance);
		uniqueMappingKey.Clear();
	}

	//Create meshes from the first entry in each bucket array.
	for (U32 i = 0; i < (U32)instanceBuckets.Size(); i++)
	{
		Aftk::C_SceneMeshRemappedInstanceNode* pFirstRemappedInstance = instanceBuckets[i][0];

		RenderMeshInstanceNode* pRenderFirstRemappedInstance;
		{
			RenderNode* pRenderNode = 
				pSceneRenderer->GetRenderNodeFromAftkNode(pFirstRemappedInstance);

			RenderNode::ERenderNodeType type = pRenderNode->GetType();

			if (!pRenderNode->HasType(RenderNode::RENDER_NODE_TYPE_MESH_REMAPPABLE))
				continue;

			pRenderFirstRemappedInstance = (RenderMeshInstanceNode*)pRenderNode;
		}

		Aftk::C_SceneMeshNode* pInstanceParent = pFirstRemappedInstance->GetSourceNodePtr();
		Ut::UtVector<Ut::DynamicVector<U32> > instanceParentKey;
		GetUniqueKey(instanceParentKey, pInstanceParent);
		
		RenderMeshNode* pRenderMesh;
		//Decide whether to use an existing mesh as the instance parent, or use a new mesh.
		if (KeysEqual(instanceParentKey, uniqueRemappings[i]))
		{
			//Reuse existing mesh.
			pRenderMesh = (RenderMeshNode*)
				pSceneRenderer->GetRenderNodeFromAftkNode(pInstanceParent);
			if (!pRenderMesh)
				return Tu::Status::FAILURE;
		}
		else
		{
			pRenderMesh = new RenderMeshNode(
				pSceneRenderer,
				pInstanceParent,
				pFirstRemappedInstance);
			//Hide the new instance parent node.
			pRenderMesh->GetNodeFlags().isHiddenExpandedSource = true;

			renderNodes.PushBack(pRenderMesh);
		}

		//Add a child to the parent of the instance's source if needed.
		if (pRenderFirstRemappedInstance->GetSourcePtr()->GetParentPtr() != pRenderMesh->GetParentPtr())
			pRenderFirstRemappedInstance->GetSourcePtr()->GetParentPtr()->AddChild(pRenderMesh);

		for (U32 j = 0; j < (U32)instanceBuckets[i].Size(); j++)
		{
			Aftk::C_SceneMeshRemappedInstanceNode* pRemappedInstance = instanceBuckets[i][j];
			if (pRemappedInstance->GetType() == Aftk::E_ObjType_SceneMeshRemappedInstance)
			{
				RenderMeshInstanceNode* pRenderNode = (RenderMeshInstanceNode*)
					pSceneRenderer->GetRenderNodeFromAftkNode(pRemappedInstance);
				if (!pRenderNode)
					return Tu::Status::FAILURE;
				
				pRenderNode->SetSourcePtr(pRenderMesh);
			}
			else if (pRemappedInstance->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
			{
				RenderMeshBonedInstanceNode* pRenderNode = (RenderMeshBonedInstanceNode*)
					pSceneRenderer->GetRenderNodeFromAftkNode(pRemappedInstance);
				if (!pRenderNode)
					return Tu::Status::FAILURE;

				pRenderNode->SetSourcePtr(pRenderMesh);
			}
		}
	}

	if (pProgress)
		pProgress->OnProgress(100.f);

	return Tu::Status::SUCCESS;
};