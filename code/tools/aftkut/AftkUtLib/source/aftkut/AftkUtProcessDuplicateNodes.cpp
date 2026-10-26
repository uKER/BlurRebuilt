#include "stdafx.h"

namespace AftkUt
{
	Aftk::C_Status C_ProcessDuplicateNodes::SortNodesIntoBuckets(Aftk::C_Scene* pScene, 
				Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >** ppNodeBuckets,
				U32& nNodeBuckets, U32 flags)
	{
		Aftk::C_Status status = Aftk::C_Status::E_Status_Success;

		if (!(pScene && ppNodeBuckets))
		{
			return Aftk::C_Status::E_Status_Failure;
		}

		//We need to do equality checks on everything against everything
		//bucket sort to avoid n^2

		//Find number of buckets by taking sqrt of nNodes

		U32 nNodes = 0;
			
		if (flags&AFTKUT_EQUALITY_MESHES_ONLY)
		{
			nNodes = pScene->GetNumDgNodes(Aftk::E_ObjType_SceneMesh);
		}
		else
		{
			nNodes = pScene->GetNumSceneNodes();
		}

		const U32 nBuckets = (U32)sqrt((float)nNodes);
		const U32 nNodesPerBucket = nBuckets;

		//preallocate buckets as an array of preallocated vectors
		Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >* nodeBuckets = 
						new Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >[nBuckets];

		for (U32 bucket = 0; bucket < nBuckets; bucket++)
		{
			nodeBuckets[bucket].Reserve(nNodesPerBucket);
		}

		if ((flags & AFTKUT_EQUALITY_MESHES_ONLY) || 
			(flags & AFTKUT_EQUALITY_SCENENODES))
		{
			AddSceneNodesToBuckets(pScene, nodeBuckets, nBuckets, flags);
		}

		if ((flags & AFTKUT_EQUALITY_MATERIALNODES))
		{
			AddMaterialNodesToBuckets(pScene, nodeBuckets, nBuckets, flags);
		}

		if ((flags & AFTKUT_EQUALITY_BLINDDATANODES))
		{
			AddBlindDataNodesToBuckets(pScene, nodeBuckets, nBuckets, flags);
		}

		(*ppNodeBuckets) = nodeBuckets;
		nNodeBuckets = nBuckets;

		return Aftk::C_Status::E_Status_Success;
	}
	
	Aftk::C_Status C_ProcessDuplicateNodes::GetEqualNodeSetsFromBuckets(Aftk::C_Scene* pScene, 
		const Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >* nodeBuckets,
		const U32 nBuckets,
		Ut::UtVector<Ut::DynamicVector< std::set<Aftk::C_Node*>* > >& equalNodes,
		U32 flags,
		Aftk::E_ObjType classToAbortTest)
	{
		//we need a vector of sets to store equal node pointers. Each member of the vector
		//is a set of pointers to nodes which are exactly the same. We can arbitrarily pick one to keep, 
		//and remove the others. 

		if (!(pScene && &nodeBuckets && &equalNodes))
		{
			return Aftk::C_Status::E_Status_Failure;
		}

		for (U32 bucket = 0; bucket < nBuckets; bucket++)
		{
			const U32 nNodesInBucket = (U32)(nodeBuckets[bucket].Size());

			if (!nNodesInBucket) 
			{
				continue;
			}

			//compare meshes equality

			for (U32 node = 0; node < nNodesInBucket-1; node++)
			{
				for (U32 other = node+1; other < nNodesInBucket; other++)
				{
					Aftk::C_Node* pNode = nodeBuckets[bucket][node];
					Aftk::C_Node* pOther = nodeBuckets[bucket][other];
					
					if (pNode->GetType() != pOther->GetType())
					{
						continue;
					}

					if (
						((flags & AFTKUT_EQUALITY_TEST_LOCAL_CLASS_ONLY) && pNode->IsEqualToThis(*pOther)) ||
						((flags & AFTKUT_EQUALITY_MESHES_ONLY) && pNode->IsEqualTo(*pOther, classToAbortTest, Aftk::E_Comparison_WorldTransform)))
					{				
						//These 2 nodes are the same:

						//Is the mesh node the same as any in the equal list?

						bool bInList = false;

						for (U32 keepIndex = 0; keepIndex < (U32)equalNodes.Size(); keepIndex++)
						{
							std::set<Aftk::C_Node*>::iterator iter;
							iter = equalNodes[keepIndex]->begin();

							//As all nodes in the set are the same, we only need test against the first
							if (pNode->GetType() != (*iter)->GetType())
							{
								continue;
							}

							if (
								((flags & AFTKUT_EQUALITY_TEST_LOCAL_CLASS_ONLY) && pNode->IsEqualToThis(**iter)) ||
								((flags & AFTKUT_EQUALITY_MESHES_ONLY) && pNode->IsEqualTo(**iter, Aftk::E_ObjType_SceneNode)))
							{
								//add to the set at this keepIndex
								equalNodes[keepIndex]->insert(pNode);
								equalNodes[keepIndex]->insert(pOther);
								bInList = true;
								break;
							}
						}
						
						//If the meshes aren't in the equal list, add them both to a new set
						//and add this to the list.

						if (!bInList)
						{
							//add to list
							std::set<Aftk::C_Node*>* pSet = new std::set<Aftk::C_Node*>;
							pSet->insert(pNode);
							pSet->insert(pOther);
							equalNodes.PushBack(pSet);
						}
					}
				}
			}
		}

		return Aftk::C_Status::E_Status_Success;
	}

	Aftk::C_Status C_ProcessDuplicateNodes::RemoveDuplicateNodes(
		Aftk::C_Scene* pScene,
		U32 flags,
		void (*pMessageCallback)(const std::string& text),
		void (*pWarningCallback)(const std::string& text),
		void (*pErrorCallback)(const std::string& text),
        Aftk::I_Progress* pProgressCallback)
	{
		return Process(
			pScene,
			flags,
			Aftk::E_ObjType_Unknown,
			RemoveDuplicateNodesTask,
			pMessageCallback,
			pWarningCallback,
			pErrorCallback);
	}

	Aftk::C_Status C_ProcessDuplicateNodes::CreateInstances(
		Aftk::C_Scene* pScene,
		void (*pMessageCallback)(const std::string& text),
		void (*pWarningCallback)(const std::string& text),
		void (*pErrorCallback)(const std::string& text),
		Aftk::I_Progress* pProgress)
	{
		return Process(
			pScene,
			AFTKUT_EQUALITY_MESHES_ONLY,
			Aftk::E_ObjType_SceneNode,
			ConvertDuplicateNodesToInstancesTask,
			pMessageCallback,
			pWarningCallback,
			pErrorCallback);
	}

Aftk::C_Status C_ProcessDuplicateNodes::AddSceneNodesToBuckets(
	Aftk::C_Scene* pScene,
	Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >* pBuckets, 
	U32 nBuckets, 
	U32 flags)
{
	Aftk::C_SceneNodeIterator aftkIt(pScene->GetRootSceneNodePtr());

	while (aftkIt)
	{
		Aftk::C_SceneNode* pAftkSceneNode = aftkIt;

		if ((flags & AFTKUT_EQUALITY_MESHES_ONLY) && (pAftkSceneNode->GetType() != Aftk::E_ObjType_SceneMesh))
		{
			aftkIt++;
			continue;
		}


		U32 hash = 0;
		
		if (flags & AFTKUT_EQUALITY_TEST_LOCAL_CLASS_ONLY)
		{
			hash = pAftkSceneNode->GetHashOfThis();
		}
		else
		{
			hash = pAftkSceneNode->GetHash();
		}

		hash = hash%nBuckets;

		//add this node to the bucket indexed by its hash. If we have a good hash, 
		//these buckets should end up with similar numbers of members.

		//Any duplicate nodes will ALWAYS end up in the same bucket, as the hash
		//will be the same.

		pBuckets[hash].PushBack((Aftk::C_Node*)pAftkSceneNode);
		
		aftkIt++;
	}	
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_ProcessDuplicateNodes::AddMaterialNodesToBuckets(Aftk::C_Scene* pScene,
							Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >* pBuckets, U32 nBuckets, U32 flags)
{
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_ProcessDuplicateNodes::AddBlindDataNodesToBuckets(Aftk::C_Scene* pScene,
							Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >* pBuckets, U32 nBuckets, U32 flags)
{
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_ProcessDuplicateNodes::FindDuplicateNodes(
	Ut::UtVector<Ut::DynamicVector< std::set<Aftk::C_Node*>* > >& equalNodes,
	Aftk::C_Scene* pScene,
	U32 flags,
	Aftk::E_ObjType classToAbortTest,
	void (*pMessageCallbackFunc)(const std::string& text),
	void (*pWarningCallback)(const std::string& text),
	void (*pErrorCallback)(const std::string& text),
	Aftk::I_Progress* pProgressCallback)
{
	if (!(flags&AFTKUT_EQUALITY_MESHES_ONLY))
		return Aftk::C_Status::E_Status_NotImplemented;

	Aftk::C_Status status;

	if (!pScene)
		return Aftk::C_Status::E_Status_Failure;

	if (pProgressCallback)
		if (pProgressCallback->OnProgress(0))
			//User cancelled operation.
			return Aftk::C_Status::E_Status_Cancelled;

	U32 numProcessingStagesCompleted = 0;

	Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >* pNodeBuckets = 0;
	U32 nBuckets = 0;

	if (!SortNodesIntoBuckets(pScene, &pNodeBuckets, nBuckets, flags))
	{
		delete[] pNodeBuckets;
		return Aftk::C_Status::E_Status_Failure;
	}
	if (pProgressCallback)
		if (pProgressCallback->OnProgress(15.f))
			//User cancelled operation.
			return Aftk::C_Status::E_Status_Cancelled;

	if (!GetEqualNodeSetsFromBuckets(
		pScene, 
		pNodeBuckets,
		nBuckets, 
		equalNodes, 
		flags,
		classToAbortTest))
	{
		delete[] pNodeBuckets;
		return Aftk::C_Status::E_Status_Failure;
	}

	const U32 nUniqueEqualNodes = (U32)equalNodes.Size();

	if (pProgressCallback)
		if (pProgressCallback->OnProgress(30.f))
			return Aftk::C_Status::E_Status_Cancelled;

	delete[] pNodeBuckets;
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_ProcessDuplicateNodes::RemoveDuplicateNodesTask(
	std::set<Aftk::C_Node*>& equalNodes,
	Aftk::C_Scene* pScene,
	U32 flags,
	void (*pMessageCallback)(const std::string& text),
	void (*pWarningCallback)(const std::string& text),
	void (*pErrorCallback)(const std::string& text))
{
	std::set<Aftk::C_Node*>::iterator iter;
	iter = equalNodes.begin();		

	//Get the first member of the set, and assign it as the pKeepNode

	Aftk::C_Node* pKeepNode = *iter;

	iter++;

	for (; iter != equalNodes.end(); iter++)
	{
		Aftk::C_Node* pDeleteNode = *iter;


		if (pDeleteNode->HasType(Aftk::E_ObjType_SceneNode))
		{
			//reparent any instances to the keepmesh

			U32 nInstances = ((Aftk::C_SceneNode*)pDeleteNode)->GetNumInstancesOfThisNode();

			for (U32 instanceIndex = 0; instanceIndex < nInstances; instanceIndex++)
			{
				//Always grab the first index, as we are removing it later.

				Aftk::C_SceneNode* node = ((Aftk::C_SceneNode*)pDeleteNode)->GetInstanceOfThisNode(0);

				if (!node)
				{
					//printf("empty node referenced\n");
					continue;
				}

				//reparent the instances to the keepmesh

				if (node->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance) &&
					(pKeepNode->HasType(Aftk::E_ObjType_SceneMesh)))
				{
					((Aftk::C_SceneMeshRemappedInstanceNode*)node)->SetSourceNodePtr(((Aftk::C_SceneMeshNode*)pKeepNode), false);
				}
				else if (node->HasType(Aftk::E_ObjType_SceneInstance))
				{
					((Aftk::C_SceneInstanceNode*)node)->SetSourceNodePtr(((Aftk::C_SceneNode*)pKeepNode));
				}
				else
				{
					//printf("Unknown instance type");
				}

/*				if (pMessageCallback)
				{
					std::stringstream ss;
					ss << "Reparented " << node->GetName() << " to " << pKeepNode->GetName() << " successfully.";
					pMessageCallback(ss.str());
				}
*/
			}
/*
			if (pMessageCallback)
			{
				std::stringstream ss;
				ss << "Deleting duplicate node [" << pDeleteNode->GetName() << "].";
				pMessageCallback(ss.str());						
			}
*/
			//delete from scene
			pScene->DeleteSceneNodeAndReparentChildren(((Aftk::C_SceneNode*)pDeleteNode), true);
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_ProcessDuplicateNodes::ConvertDuplicateNodesToInstancesTask(
	std::set<Aftk::C_Node*>& equalNodes,
	Aftk::C_Scene* pScene,
	U32 flags,
	void (*pMessageCallback)(const std::string& text),
	void (*pWarningCallback)(const std::string& text),
	void (*pErrorCallback)(const std::string& text))
{
	std::set<Aftk::C_Node*>::iterator iter;
	iter = equalNodes.begin();		

	//Get the first member of the set, and assign it as the pKeepNode
	Aftk::C_Node* pKeepNode = *iter;
	iter++;

	for (; iter != equalNodes.end(); iter++)
	{
		Aftk::C_Node* pDeleteNode = *iter;

		if (pDeleteNode->HasType(Aftk::E_ObjType_SceneNode))
		{
			//Create a new instance.
			Aftk::C_SceneInstanceNode* pNewInst = new Aftk::C_SceneInstanceNode(pScene);
			pNewInst->SetSourceNodePtr((Aftk::C_SceneNode*)pKeepNode);

			//reparent any instances to the keepmesh
			U32 nInstances = ((Aftk::C_SceneNode*)pDeleteNode)->GetNumInstancesOfThisNode();
			for (U32 instanceIndex = 0; instanceIndex < nInstances; instanceIndex++)
			{
				//Always grab the first index, as we are removing it later.

				Aftk::C_SceneNode* node = ((Aftk::C_SceneNode*)pDeleteNode)->GetInstanceOfThisNode(0);

				if (!node)
				{
					//printf("empty node referenced\n");
					continue;
				}

				//reparent the instances to the keepmesh
				if (node->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance) &&
					(pKeepNode->HasType(Aftk::E_ObjType_SceneMesh)))
				{
					((Aftk::C_SceneMeshRemappedInstanceNode*)node)->SetSourceNodePtr(((Aftk::C_SceneMeshNode*)pKeepNode));
				}
				else if (node->HasType(Aftk::E_ObjType_SceneInstance))
				{
					((Aftk::C_SceneInstanceNode*)node)->SetSourceNodePtr(((Aftk::C_SceneNode*)pKeepNode));
				}
				else
				{
					return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Unknown instance type");
				}

/*				if (pMessageCallback)
				{
					std::stringstream ss;
					ss << "Reparented " << node->GetName() << " to " << pKeepNode->GetName() << " successfully.";
					pMessageCallback(ss.str());
				}
*/
			}

			//Replace old mesh.
			pNewInst->SetName(((Aftk::C_SceneNode*)pDeleteNode)->GetName());
			pNewInst->SetBlindDataResourcePtr(((Aftk::C_SceneNode*)pDeleteNode)->GetBlindDataResourcePtr());
			pNewInst->SetLocalMatrix(((Aftk::C_SceneNode*)pDeleteNode)->GetLocalMatrix());
/*
			if (pMessageCallback)
			{
				std::stringstream ss;
				ss << "Replaced " << pDeleteNode->GetName() << " with an instance of " << pNewInst->GetSourceNodePtr()->GetName() << ".";
				pMessageCallback(ss.str());						
			}
*/
			pNewInst->Replace((Aftk::C_SceneNode*)pDeleteNode);
			//printf("Deleted %s \n", pDeleteNode->GetName().c_str());
		}
		else
		{
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_ProcessDuplicateNodes::Process(
	Aftk::C_Scene* pScene,
	U32 flags,
	Aftk::E_ObjType classToAbortTest,
	ProcessDuplicateTaskCallback pTaskCallback,
	void (*pMessageCallback)(const std::string& text),
	void (*pWarningCallback)(const std::string& text),
	void (*pErrorCallback)(const std::string& text),
	Aftk::I_Progress* pProgressCallback)
{
	if (!pScene) 
		return Aftk::C_Status::E_Status_Failure;

	if (pProgressCallback)
		if (pProgressCallback->OnProgress(0))
			//User cancelled operation.
			return Aftk::C_Status::E_Status_Cancelled;

	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Processing...");
		pProgressCallback->OnProgress(0);
	}

	Aftk::C_Status status;
	U32 numProcessingStagesCompleted = 0;

	Ut::UtVector<Ut::DynamicVector< std::set<Aftk::C_Node*>* > > equalNodes;
	status = FindDuplicateNodes(
		equalNodes,
		pScene,
		flags,
		classToAbortTest,
		pMessageCallback,
		pWarningCallback,
		pErrorCallback,
		pProgressCallback);
	if (!status) return status;

	//Delete the std::set contained at equalNodes[keepIndex] 
	for (U32 i = 0; i < (U32)equalNodes.Size(); i++)
	{
		status = pTaskCallback(
			*equalNodes[i],
			pScene,
			flags,
			pMessageCallback,
			pWarningCallback,
			pErrorCallback);
		delete equalNodes[i];

		numProcessingStagesCompleted++;
		F32 progress = 30.f + (((F32)numProcessingStagesCompleted / (F32)equalNodes.Size()) * 70.0f);
		if (pProgressCallback)
			if (pProgressCallback->OnProgress(progress))
				return Aftk::C_Status::E_Status_Cancelled;
	}
	
	return status;
}


}
