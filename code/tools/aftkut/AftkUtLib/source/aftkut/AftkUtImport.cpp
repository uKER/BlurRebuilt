#include <stdafx.h>

#include <AftkUt/AftkUt.h>

using namespace AftkUt;


Aftk::C_Status C_Import::ImportPlugConnections(
	const Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>& nodeMappingTable,
	const Aftk::I_Plug& plug)
{
	Aftk::C_PlugLocationInfo sourceLocation;
	plug.GetLocationInfo(sourceLocation);
	
	Aftk::C_Status status = MirrorConnectivity(
		nodeMappingTable,
		sourceLocation);
	if (!status) return status;

	for (U32 i = 0; i < plug.GetNumChildren(); i++)
	{
		status = ImportPlugConnections(nodeMappingTable, *plug.GetChildPtr(i));
		if (!status) return status;
	}

	return Aftk::C_Status::E_Status_Success;
}

//Given a C_LocationInfo structure and a node mapping table we can resolve source connections between plugs.
Aftk::C_Status C_Import::MirrorConnectivity(
	const Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>& nodeMappingTable,
	const Aftk::C_PlugLocationInfo& rhsLocationInfo)
{
	const Aftk::C_Node* pSourceNode = rhsLocationInfo.pTopLevelParent;
	Aftk::C_Node* pTargetNode;
	{
		Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>::ConstIterator findIt;
		findIt = nodeMappingTable.Find(pSourceNode);
		if (findIt == nodeMappingTable.End())
		{
			AFTK_ASSERT(false);
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to find node.");
		}
		pTargetNode = findIt->GetValue();
	}
	
	const Aftk::I_Plug* pSourcePlug = 
		pSourceNode->GetPlugPtr(rhsLocationInfo);
	const Aftk::I_Object* pSourceConnection = pSourcePlug->GetSourceConnectionPtr();
	Aftk::I_Object* pTargetConnection;
	if (pSourceConnection)
	{
		//Source connections can only be made to nodes or plugs.

		if (pSourceConnection->HasType(Aftk::E_ObjType_Node))
		{
			Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>::ConstIterator findIt;
			findIt = nodeMappingTable.Find((Aftk::C_Node*)pSourceConnection);
			if (findIt == nodeMappingTable.End())
			{
				AFTK_ASSERT(false);
				return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to find node.");
			}
			pTargetConnection = findIt->GetValue();
			
		}
		else if (pSourceConnection->HasType(Aftk::E_ObjType_IPlug))
		{
			Aftk::I_Plug* pSourceConnectionPlug = (Aftk::I_Plug*)pSourceConnection;
			Aftk::C_PlugLocationInfo sourceConnectionPlugInfo;
			pSourceConnectionPlug->GetLocationInfo(sourceConnectionPlugInfo);

			const Aftk::C_Node* pSourceConnectionNode = sourceConnectionPlugInfo.pTopLevelParent;
			Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>::ConstIterator findIt;
			findIt = nodeMappingTable.Find(pSourceConnectionNode);
			if (findIt == nodeMappingTable.End())
			{
				AFTK_ASSERT(false);
				return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to find anim node.");
			}

			//We've found the node our source connection plug resides on.
			Aftk::C_Node* pTargetConnectionNode = findIt->GetValue();

            //Get the source connection for our target plug.
			pTargetConnection = 
				pTargetConnectionNode->GetPlugPtr(sourceConnectionPlugInfo);

			if (!pTargetConnection)
				return Aftk::C_Status::E_Status_Failure;
		}
		else
			pTargetConnection = 0;
	}
	else
		pTargetConnection = 0;


	Aftk::I_Plug* pTargetPlug = 
		pTargetNode->GetPlugPtr(rhsLocationInfo);
	if (!pTargetPlug)
		return Aftk::C_Status::E_Status_Failure;

	if (pTargetConnection)
		//Set our resolved target connection.
		pTargetPlug->SetSourceConnectionPtr(pTargetConnection);

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Import::RebuildInfluenceCluster(
	const Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>& nodeMappingTable,
	Aftk::C_InfluenceClusterNode* pTargetInfluenceCluster,
	const Aftk::C_InfluenceClusterNode* pSourceInfluenceCluster)
{
	//Ensure that target cluster envelopes are pointing to the correct scene nodes.
	for (U32 jEnvelope = 0; jEnvelope < pSourceInfluenceCluster->GetNumEnvelopes(); jEnvelope++)
	{
		const Aftk::C_WeightEnvelope* pSourceWeightEnvelope = 
			&pSourceInfluenceCluster->GetEnvelope(jEnvelope);
		const Aftk::C_SceneNode* pSourceInfluenceNode =
			pSourceWeightEnvelope->GetInfluenceNodePtr();
		if (pSourceInfluenceNode)
		{
			Aftk::C_SceneNode* pTargetInfluenceNode;
			//Find target scene node for envelope.
			{
				Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>::ConstIterator mappingIt;
				mappingIt = nodeMappingTable.Find(pSourceInfluenceNode);
				if (mappingIt == nodeMappingTable.End())
				{
					return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, AFTK_E_IMPORT_REMAPPING_FAIL);
				}
				pTargetInfluenceNode = (Aftk::C_SceneNode*)mappingIt->GetValue();
			}

			Aftk::C_WeightEnvelope* pTargetWeightEnvelope = 
				&pTargetInfluenceCluster->GetEnvelope(jEnvelope);
			AFTK_ASSERT(pTargetInfluenceNode);
			pTargetWeightEnvelope->SetInfluenceNodePtr(pTargetInfluenceNode);
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Import::RebuildNodes(
	const Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>& nodeMappingTable)
{
	Aftk::C_Status status;
	//Rebuild nodes that require it.
	for (Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>::ConstIterator it = nodeMappingTable.Begin(); 
		it != nodeMappingTable.End(); 
		++it)
	{
		const Aftk::C_Node* pSourceNode = it->GetKey();
		Aftk::C_Node* pTargetNode = it->GetValue();

		if (pSourceNode->HasType(Aftk::E_ObjType_SceneNode))
		{
			Aftk::C_SceneNode* pSourceSceneNode = (Aftk::C_SceneNode*)pSourceNode;
			Aftk::C_SceneNode* pTargetSceneNode = (Aftk::C_SceneNode*)pTargetNode;

			//Resolve layer connections for new node.
			for (U32 iLayer = 0; iLayer < pSourceSceneNode->GetNumLayers(); iLayer++)
			{
				const Aftk::C_LayerNode* pSourceLayer = pSourceSceneNode->GetLayerPtr(iLayer);
				if (pSourceLayer)
				{
					Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>::ConstIterator mappingIt;
					mappingIt = nodeMappingTable.Find(pSourceLayer);
					if (mappingIt == nodeMappingTable.End())
					{
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, AFTK_E_IMPORT_REMAPPING_FAIL);
					}
					
					//Add new scene node to target layer.
					Aftk::C_LayerNode* pTargetLayer = (Aftk::C_LayerNode*)mappingIt->GetValue();
					pTargetLayer->AddMember(pTargetSceneNode);
				}
			}

			//Resolve influence cluster connections for new node.
			for (U32 iInfluenceCluster = 0; iInfluenceCluster < pSourceSceneNode->GetNumInfluenceClusters(); iInfluenceCluster++)
			{
				const Aftk::C_InfluenceClusterNode* pSourceInfluenceCluster = 
					pSourceSceneNode->GetInfluenceClusterPtr(iInfluenceCluster);
				if (pSourceInfluenceCluster)
				{
					Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>::ConstIterator mappingIt;
					mappingIt = nodeMappingTable.Find(pSourceInfluenceCluster);
					if (mappingIt == nodeMappingTable.End())
					{
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, AFTK_E_IMPORT_REMAPPING_FAIL);
					}
					
					//Associate target influence cluster with our target scene node.
					Aftk::C_InfluenceClusterNode* pTargetInfluence = (Aftk::C_InfluenceClusterNode*)mappingIt->GetValue();
					pTargetInfluence->SetSceneNodePtr(pTargetSceneNode);
				}
			}
		}
			
		//Yep, the code below is repeated twice. However I can't currently think of a way of having all 
		//instances derive from an 'instance' class without using multiple inheritence. So, I've not 
		//done it! In any case, instances need not be mesh, but 'boned instances' and 'remapped instances'
		//can _only_ be of mesh. So perhaps this distinction is fine in actual fact.
		if (pSourceNode->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			const Aftk::C_SceneInstanceNode* pOldInstance = 
				(const Aftk::C_SceneInstanceNode*)pSourceNode;
			const Aftk::C_SceneNode* pOldSource = pOldInstance->GetSourceNodePtr();
			if (pOldSource)
			{
				Aftk::C_SceneNode* pNewSource;
				//Find the new source that corresponds to the one from the incoming scene.
				{
					Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>::ConstIterator mappingIt;
					mappingIt = nodeMappingTable.Find(pOldSource);
					if (mappingIt == nodeMappingTable.End())
					{
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, AFTK_E_IMPORT_REMAPPING_FAIL);
					}
					pNewSource = (Aftk::C_SceneNode*)mappingIt->GetValue();
				}
				//Set the source pointer for the new node.
				Aftk::C_SceneInstanceNode* pNewInstance = 
					(Aftk::C_SceneInstanceNode*)pTargetNode;
				pNewInstance->SetSourceNodePtr(pNewSource);
			}
		}
		else if (pSourceNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		{
			const Aftk::C_SceneMeshRemappedInstanceNode* pOldInstance = 
				(const Aftk::C_SceneMeshRemappedInstanceNode*)pSourceNode;
			const Aftk::C_SceneNode* pOldSource = pOldInstance->GetSourceNodePtr();
			if (pOldSource)
			{
				Aftk::C_SceneNode* pNewSource;
				//Find the new source that corresponds to the one from the incoming scene.
				{
					Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>::ConstIterator mappingIt;
					mappingIt = nodeMappingTable.Find(pOldSource);
					if (mappingIt == nodeMappingTable.End())
					{
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, AFTK_E_IMPORT_REMAPPING_FAIL);
					}
					pNewSource = (Aftk::C_SceneNode*)mappingIt->GetValue();
				}
				//Set the source pointer for the new node.
				Aftk::C_SceneMeshRemappedInstanceNode* pNewInstance = 
					(Aftk::C_SceneMeshRemappedInstanceNode*)pTargetNode;

				//Use setonly function to avoid blatting our material mapping table.
				pNewInstance->SetSourceNodePtr((Aftk::C_SceneMeshNode*)pNewSource, false);
			}

			if (pSourceNode->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
			{
				const Aftk::C_SceneMeshBonedInstanceNode* pOldBonedInstance = 
					(const Aftk::C_SceneMeshBonedInstanceNode*)pSourceNode;
				for (U32 iBone = 0; iBone < 2; iBone++)
				{
					const Aftk::C_SceneNode* pOldBone = pOldBonedInstance->GetBonePtr(iBone);
					if (pOldBone)
					{
						Aftk::C_SceneNode* pNewBone;
						//Find the new source that corresponds to the one from the incoming scene.
						{
							Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>::ConstIterator mappingIt;
							mappingIt = nodeMappingTable.Find(pOldBone);
							if (mappingIt == nodeMappingTable.End())
							{
								return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, AFTK_E_IMPORT_REMAPPING_FAIL);
							}
							pNewBone = (Aftk::C_SceneNode*)mappingIt->GetValue();
						}
						//Set the source pointer for the new boned instance node.
						Aftk::C_SceneMeshBonedInstanceNode* pNewBonedInstance = 
							(Aftk::C_SceneMeshBonedInstanceNode*)pTargetNode;
						
						pNewBonedInstance->SetBonePtr(iBone, pNewBone);
					}
				}
			}
		}
	
		if (pSourceNode->GetType() == Aftk::E_ObjType_SceneLodGroup)
		{
			const Aftk::C_SceneLodGroupNode* pOldLodGroup =
				(const Aftk::C_SceneLodGroupNode*)pSourceNode;

			Aftk::C_SceneLodGroupNode* pNewLodGroup = 
				(Aftk::C_SceneLodGroupNode*)pTargetNode;

			//Create new lod group entries for our new log group. The trick here is that we
			//need to make sure we copy across the correct scene node pointers for each LOD level.
			
			//Loop thru each LOD level entry...
			for (U32 iLodLevel = 0; iLodLevel < pOldLodGroup->GetNumLodLevels(); iLodLevel++)
			{
				const Aftk::C_GroupLodLevelInfo& oldInfo = pOldLodGroup->GetLodLevelInfo(iLodLevel);
				const Aftk::C_SceneNode* pOldLod = oldInfo.GetSceneNodePtr();
				if (pOldLod)
				{
					Aftk::C_SceneNode* pNewLod;
					//Find the new lod that corresponds to the one from the incoming scene.
					{
						Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>::ConstIterator mappingIt;
						mappingIt = nodeMappingTable.Find(pOldLod);
						if (mappingIt == nodeMappingTable.End())
						{
							return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, AFTK_E_IMPORT_REMAPPING_FAIL);
						}
						pNewLod = (Aftk::C_SceneNode*)mappingIt->GetValue();
					}
					//Set the lod pointer for the new node.
					Aftk::C_GroupLodLevelInfo newInfo;
					newInfo.SetThreshold(oldInfo.GetThreshold());
					newInfo.SetSceneNodePtr(pNewLod);
					pNewLodGroup->InsertLodLevelInfo(iLodLevel, newInfo);
				}
			}
		}

		//Dg nodes...

		//Ignore everything apart from influence clusters.
		if (pSourceNode->GetType() == Aftk::E_ObjType_InfluenceCluster)
		{
			status = RebuildInfluenceCluster(
				nodeMappingTable,
				(Aftk::C_InfluenceClusterNode*)pTargetNode,
				(const Aftk::C_InfluenceClusterNode*)pSourceNode);
			if (!status) return status;
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Import::ImportAnimation(
	Aftk::C_Scene* pTargetScene,
	const std::string &fileName, 
	Aftk::E_FileType format)
{
	Aftk::C_Status status;

	Aftk::C_Scene importScene;
	status = importScene.Read(fileName, format);
	if (!status) return status;

	status = ImportAnimation(pTargetScene, &importScene);
	if (!status) return status;

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Import::ImportAnimation(
	Aftk::C_Scene* pTargetScene,
	const Aftk::C_Scene* pScene)
{
	if (pScene == pTargetScene)
	{
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Cannot copy animation data to and from the same location.");
	}

	Aftk::C_Status status;

	Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*> animNodes;
	//Build a list of all of the animated nodes in the source scene.
	Ut::UtVector<Ut::DynamicVector<const Aftk::C_SceneNode*> > sourceSceneNodes;
	Aftk::C_SceneNodeConstIterator itSource(pScene);
	while(itSource)
	{
		const Aftk::C_SceneNode* pSceneNode = itSource;
		if (pSceneNode->IsTransformDirectlyAnimated())
		{
			sourceSceneNodes.PushBack(pSceneNode);
		}

		itSource++;
	}

	//Abort if the import scene has no animation!
	if (sourceSceneNodes.Size() == 0)
		return Aftk::C_Status::E_Status_Success;

	//Check no duplicate names exist in the incomming scene.
	{
		Ut::UtSet<std::string> uniqueNames;
		for (U32 iSourceSceneNode = 0; iSourceSceneNode < (U32)sourceSceneNodes.Size(); iSourceSceneNode++)
		{
			const Aftk::C_SceneNode* pSourceSceneNode = sourceSceneNodes[iSourceSceneNode];
			Ut::UtSet<std::string>::Iterator it = uniqueNames.Find(pSourceSceneNode->GetName());
			if (it != uniqueNames.End())
			{
				return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Duplicate node name found in input scene."); 
			}
			uniqueNames.Insert(pSourceSceneNode->GetName());
		}
	}

	//Build a list of the animated nodes in the target scene that have names matching nodes in
	//the target scene.

	Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> > targetSceneNodes;

	//O(n^2) search - I'm sure we could do much better, but n is likely to be quite small.
	for (U32 iSourceSceneNode = 0; iSourceSceneNode < (U32)sourceSceneNodes.Size(); iSourceSceneNode++)
	{
		Aftk::C_SceneNodeIterator itTarget(pTargetScene);
		while(itTarget)
		{
			Aftk::C_SceneNode* pSceneNode = itTarget;
			if (pSceneNode->GetName() == sourceSceneNodes[iSourceSceneNode]->GetName())
			{
				targetSceneNodes.PushBack(pSceneNode);
				//Store mapping between source and target scene nodes.
				animNodes[sourceSceneNodes[iSourceSceneNode]] = pSceneNode;
				break;
			}
			itTarget++;
		}
	}

	//Abort if we have no matches!
	if (targetSceneNodes.Size() == 0)
	{
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Source scene contains no compatible input nodes.");
	}

	//Remove existing animation data from targeted scene nodes.
	for (U32 iSceneNode = 0; iSceneNode < (U32)targetSceneNodes.Size(); iSceneNode++)
	{
		Aftk::C_SceneNode* pTargetSceneNode = targetSceneNodes[iSceneNode];

		//The source and target scene nodes _must_ have equal plug counts.
		if (pTargetSceneNode->GetNumPlugs() != sourceSceneNodes[iSceneNode]->GetNumPlugs())
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Scene nodes must have identical numbers of plugs.");

		//Static plugs 0..3 are transform plugs.
		for (U32 iPlug = 0; iPlug < pTargetSceneNode->GetNumPlugs(); iPlug++)
		{
			Aftk::I_Plug* pPlug = pTargetSceneNode->GetPlugPtr(iPlug);
			Aftk::I_Object* pSourceConnection = pPlug->GetSourceConnectionPtr();
			if (!pSourceConnection)
				continue;
			
			if (pSourceConnection->HasType(Aftk::E_ObjType_AnimationCurve))
				//Clear animation connection.
				pPlug->SetSourceConnectionPtr(0);
			
			for (U32 jPlugChild = 0; jPlugChild < pPlug->GetNumChildren(); jPlugChild++)
			{
				Aftk::I_Plug* pChildPlug = pPlug->GetChildPtr(jPlugChild);
				pSourceConnection = pChildPlug->GetSourceConnectionPtr();
				if (pSourceConnection->HasType(Aftk::E_ObjType_AnimationCurve))
					//Clear animation connection.
					pChildPlug->SetSourceConnectionPtr(0);
			}
		}
	}

	//Copy animation nodes and build a mapping table from the old node ptrs->new node ptrs.
	for (
		Ut::UtSet<Aftk::C_Node*>::ConstIterator it = pScene->GetDgNodeSet().Begin(); 
		it != pScene->GetDgNodeSet().End();
		++it)
	{
		Aftk::C_Node* pNode = it->GetKey();
		if (IsAnimNode(pNode))
		{
			Aftk::C_Node* pNewObj = pNode->Clone(pTargetScene);
			if (!pNewObj)
				return Aftk::C_Status::E_Status_OutOfMemory;
			animNodes[pNode] = pNewObj;
		}
	}

	status = RebuildNodes(animNodes);
	if (!status) return status;

	//Apply scene-node animation connections.
	for (
		U32 iSourceSceneNode = 0; 
		iSourceSceneNode < (U32)sourceSceneNodes.Size(); 
		iSourceSceneNode++)
	{
		const Aftk::C_SceneNode* pSourceSceneNode = 
			sourceSceneNodes[iSourceSceneNode];
		Aftk::C_SceneNode* pTargetSceneNode = 
			pTargetScene->GetSceneNodePtr(pSourceSceneNode->GetName());
		if (!pTargetSceneNode)
			return Aftk::C_Status::E_Status_Failure;

		//Mirror plug connectivity between scenes.
		for (U32 iPlug = 0; iPlug < pSourceSceneNode->GetNumPlugs(); iPlug++)
		{
			Aftk::C_PlugLocationInfo sourcePlugLocationInfo;

			//Get plug location information.
			pSourceSceneNode->GetPlugPtr(iPlug)->GetLocationInfo(
				sourcePlugLocationInfo);

			//Set plug connectivity information.
			status = MirrorConnectivity(animNodes, sourcePlugLocationInfo);
			if (!status) return status;
		}
	}

	//We're done!
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Import::Import(
	Aftk::C_Scene* pTargetScene,
	const std::string &fileName, 
	Aftk::E_FileType format,
	Aftk::I_Progress* pProgress)
{
	Aftk::C_Status status;

	static Debug::Logger &log = Debug::GetLogger("tools.aftkut");

	LOGINFO( log, "Exporting Scene '%s'.", fileName.c_str());

	Aftk::C_Scene importScene;
	status = importScene.Read(fileName, format, 0, pProgress);
	if (!status) return status;

	status = Import(
		pTargetScene, 
		&importScene,
		pProgress);
	if (!status) return status;

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Import::Import(
	Aftk::C_Scene* pTargetScene,
	const Aftk::C_Scene* pSourceScene,
	Aftk::I_Progress* pProgress)
{
	Aftk::C_Status status;

	if (pProgress)
	{
		if (pProgress->OnProgress(0))
		{
			return Aftk::C_Status(Aftk::C_Status::E_Status_Cancelled, "Scene import aborted.");
		}
		pProgress->OnPrintProgress("Importing...");
	}

	Aftk::C_SceneNode* pRoot = pTargetScene->GetRootSceneNodePtr();
	if (!pRoot || pRoot->GetName() != "AftkUtImportRoot")
	{
		Aftk::C_SceneNode* pNewRoot = new Aftk::C_SceneNode(pTargetScene);
		pNewRoot->SetName("AftkUtImportRoot");
		if (pRoot)
			pNewRoot->SetChildPtr(pRoot);
		pTargetScene->SetRootSceneNodePtr(pNewRoot);
	}

	U32 workToDo = pSourceScene->GetNumSceneNodes();
	U32 workDone = 0;

	//Note that the scene transform is ignored.
	//Note that the blind data origin is ignored.

	//Copy layers. Merge layers by name.
	//Maintain a mapping table of [old layer ptr][new layer ptr]
	Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*> nodeMappingTable;
	//Loop thru layers on incoming scene.
	for (U32 iLayer = 0; iLayer < pSourceScene->GetNumLayers(); iLayer++)
	{
		Aftk::C_LayerNode* pTargetLayer = 0;
		for (U32 jLayer = 0; jLayer < pTargetScene->GetNumLayers(); jLayer++)
		{
			//Try to find a layer to merge into.
			const Aftk::C_LayerNode* pSourceLayer = pSourceScene->GetLayerPtr(iLayer);
			if (pTargetScene->GetLayerPtr(jLayer)->GetName() == 
				pSourceLayer->GetName())
			{
				pTargetLayer = pTargetScene->GetLayerPtr(jLayer);
				nodeMappingTable[pSourceLayer] = pTargetLayer;
			}
		}
		if (!pTargetLayer)
		{
			const Aftk::C_LayerNode* pSourceLayer = 
				pSourceScene->GetLayerPtr(iLayer);
			pTargetLayer = (Aftk::C_LayerNode*)pSourceLayer->Clone(pTargetScene);
			nodeMappingTable[pSourceLayer] = pTargetLayer;

			//Copy between layers.
			*pTargetLayer = *pSourceScene->GetLayerPtr(iLayer);
		}

		//Layer membership will be resolved later.
	}

	//Copy all DG set members, except the following types:
	//	- layers which were treated above.
	//	- scene nodes
	{
		const Ut::UtSet<Aftk::C_Node*>& sourceDgSet = pSourceScene->GetDgNodeSet();
		for (
			Ut::UtSet<Aftk::C_Node*>::ConstIterator dgIt = sourceDgSet.Begin(); 
			dgIt != sourceDgSet.End(); 
			++dgIt)
		{
			const Aftk::C_Node* pSourceNode = dgIt->GetKey();

			//Skip filtered nodes.
			if (
				(pSourceNode->HasType(Aftk::E_ObjType_SceneNode)) ||
				(pSourceNode->GetType() == Aftk::E_ObjType_Layer))
			{
				continue;			
			}

			if (!pSourceNode)
				return Aftk::C_Status::E_Status_OutOfMemory;

			nodeMappingTable[pSourceNode] = pSourceNode->Clone(pTargetScene);
		}
	}

	//Import materials.
	U32 oldMtlCount = pTargetScene->GetNumMaterialNodes();
	Aftk::MaterialRemappingHashTable materialResourceMappingTable;
	for (U32 i = 0; i < pSourceScene->GetNumMaterialNodes(); i++)
	{
		const Aftk::C_MaterialResource* pSourceRes = 
			pSourceScene->GetMaterialResourcePtr(i);

		const Aftk::C_MaterialResource* pTargetRes;
		if (pSourceRes == 0)
		{
			pTargetRes = 0;
		}
		else
		{
			pTargetRes = pTargetScene->AddMaterialNode(pSourceRes->GetContent());
		}

		materialResourceMappingTable[pSourceRes] = pTargetRes;
	}

	//Import scene graph.
	if (!pSourceScene->GetRootSceneNodePtr())
	{
		return Aftk::C_Status::E_Status_Success;
	}

	Aftk::C_SceneNode* pIncomingRoot = Import(
		pTargetScene,
		pSourceScene->GetRootSceneNodePtr(),
		nodeMappingTable,
		materialResourceMappingTable,
		status, 
		workDone,
		workToDo,
		pProgress);
	//If nothing went wrong, add to our main scene graph.
	if (pIncomingRoot && status)
	{
		if (pTargetScene->GetRootSceneNodePtr())
			pTargetScene->GetRootSceneNodePtr()->AddChildPtr(pIncomingRoot);
		else
			pTargetScene->SetRootSceneNodePtr(pIncomingRoot);
	}
	else
		return status;

	status = RebuildNodes(nodeMappingTable);
	if (!status) return status;

	//Resolve DG plug connections.
	{
		const Ut::UtSet<Aftk::C_Node*>& sourceDgNodeSet = pSourceScene->GetDgNodeSet();
		for (
			Ut::UtSet<Aftk::C_Node*>::ConstIterator sourceDgIt = sourceDgNodeSet.Begin();
			sourceDgIt != sourceDgNodeSet.End();
			sourceDgIt++)
		{
			const Aftk::C_Node* pSourceNode = sourceDgIt->GetKey();
			for (U32 iPlug = 0; iPlug < pSourceNode->GetNumPlugs(); iPlug++)
			{
				const Aftk::I_Plug* pSourcePlug = pSourceNode->GetPlugPtr(iPlug);
				status = ImportPlugConnections(nodeMappingTable, *pSourcePlug);
				if (!status) return status;
			}
		}
	}

	if (pSourceScene->GetEndTime() > pTargetScene->GetEndTime())
		pTargetScene->SetEndTime(pSourceScene->GetEndTime());

	if (pSourceScene->GetStartTime() < pTargetScene->GetStartTime())
		pTargetScene->SetStartTime(pSourceScene->GetStartTime());

	if (pTargetScene->GetFrameRate() == 0)
		pTargetScene->SetFrameRate(pSourceScene->GetFrameRate());

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_SceneNode* C_Import::Import(
	Aftk::C_Scene* pTargetScene,
	const Aftk::C_SceneNode* pSourceSceneNode, 
	Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>& nodeMappingTable,
	const Aftk::MaterialRemappingHashTable& materialResourceMappingTable,
	Aftk::C_Status& status,
	U32& workDone,
	U32& workToDo,
	Aftk::I_Progress* pProgress)
{
	//Import this node.
	Aftk::C_SceneNode* pTargetSceneNode;
	if (pSourceSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
	{
		pTargetSceneNode = 
			(Aftk::C_SceneNode*)(((Aftk::C_SceneMeshRemappableNode*)pSourceSceneNode)->Clone(pTargetScene, materialResourceMappingTable));
	}
	else
	{
		pTargetSceneNode = 
			(Aftk::C_SceneNode*)pSourceSceneNode->Clone(pTargetScene);
	}

	AFTK_ASSERT(pTargetSceneNode->GetNumLayers() == 0);
	AFTK_ASSERT(pTargetSceneNode->GetNumInfluenceClusters() == 0);
	//Add cloned node to our node mapping table.
	nodeMappingTable[pSourceSceneNode] = pTargetSceneNode;

	workDone++;

	if (pProgress)
	{
		F32 percent = (F32)workDone/(F32)workToDo;
		percent*=100.f;
		if (pProgress->OnProgress(percent))
		{
			status = Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Scene import aborted.");
			return 0;
		}
	}

	//Import our children.
	{
		const Aftk::C_SceneNode* pChild = pSourceSceneNode->GetChildPtr();
		while (pChild)
		{
			Aftk::C_SceneNode* pNewChild = Import(
				pTargetScene,
				pChild,
				nodeMappingTable,
				materialResourceMappingTable,
				status,
				workDone,
				workToDo,
				pProgress);
			//If nothing went wrong, add new child.
			if (pNewChild && status)
				pTargetSceneNode->AddChildPtr(pNewChild);
			else
				return 0;

			//Let's iterate...
			pChild = pChild->GetNextPtr();
		}
	}

	return pTargetSceneNode;
}

bool C_Import::IsAnimNode(const Aftk::I_Object* pObj)
{
	if ((pObj->GetType() == Aftk::E_ObjType_AnimationCurve) ||
		(pObj->GetType() == Aftk::E_ObjType_BlendWeighted))
		return true;

	return false;
}
