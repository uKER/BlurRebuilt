#include "stdafx.h"

namespace AftkUt
{

bool C_ExpandUniqueRemappedInstances::KeysEqual(
	const Ut::UtVector<Ut::DynamicVector<U32> >& key1,
	const Ut::UtVector<Ut::DynamicVector<U32> >& key2)
{
	if (key1.Size() != key2.Size())
		return false;
	if (memcmp(&key1[0], &key2[0], key1.Size()*sizeof(U32)) != 0)
		return false;

	return true;
}

void C_ExpandUniqueRemappedInstances::GetUniqueKey(
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


void C_ExpandUniqueRemappedInstances::GetUniqueKey(
	Ut::UtVector<Ut::DynamicVector<U32> >& uniqueMappingKey,
	const Aftk::C_SceneMeshRemappedInstanceNode* pRemappedInstance)
{
	//Add source instance.
	uniqueMappingKey.PushBack(pRemappedInstance->GetSourceNodePtr()->GetUniqueId());

	GetUniqueKey(uniqueMappingKey, (Aftk::C_SceneMeshRemappableNode*)pRemappedInstance);
/*	
	std::cout << pRemappedInstance->GetName() << " unique key [";
	for (U32 i = 0; i < (U32)uniqueMappingKey.Size(); i++)
		std::cout << uniqueMappingKey[i];
	std::cout << "] instance parent name [" << pRemappedInstance->GetSourceNodePtr()->GetName() << "]\n";
*/
}

void C_ExpandUniqueRemappedInstances::GetUniqueKey(
	Ut::UtVector<Ut::DynamicVector<U32> >& uniqueMappingKey,
	const Aftk::C_SceneMeshNode* pMesh)
{
	//Add instance.
	uniqueMappingKey.PushBack(pMesh->GetUniqueId());

	GetUniqueKey(uniqueMappingKey, (Aftk::C_SceneMeshRemappableNode*)pMesh);
/*
	std::cout << pMesh->GetName() << " unique mesh key [";
	for (U32 i = 0; i < (U32)uniqueMappingKey.Size(); i++)
		std::cout << uniqueMappingKey[i];
	std::cout << "]\n";
*/
}

Aftk::C_Status C_ExpandUniqueRemappedInstances::Expand(
	Aftk::C_Scene* pAftkScene,
	void (*pMessageCallbackFunc)(const std::string& text),
	void (*pWarningCallback)(const std::string& text),
	void (*pErrorCallback)(const std::string& text),
	Aftk::I_Progress* pProgress)
{
	if (pProgress)
	{
		pProgress->OnPrintProgress("Expanding unique remapped instances...");
		pProgress->OnProgress(0);
	}

//#define AFTK_UT_EXPAND_UNIQUE_REMAPPED_INSTANCES_DEBUG

#ifdef AFTK_UT_EXPAND_UNIQUE_REMAPPED_INSTANCES_DEBUG
	F32 hack = 3.f;
#endif
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
		Aftk::C_SceneMeshNode* pInstanceParent = pFirstRemappedInstance->GetSourceNodePtr();
		Ut::UtVector<Ut::DynamicVector<U32> > instanceParentKey;
		GetUniqueKey(instanceParentKey, pInstanceParent);
		Aftk::C_SceneMeshNode* pMesh;
		//Decide whether to use an existing mesh as the instance parent, or use a new mesh.
		if (KeysEqual(instanceParentKey, uniqueRemappings[i]))
		{
			pMesh = pInstanceParent;
//			std::cout << "reusing existing mesh" << std::endl;
		}
		else
		{
			pMesh = new Aftk::C_SceneMeshNode(*pInstanceParent);
			std::string name = pMesh->GetName();
			name = name + "___expanded_copy";
//			std::cout << "Created " << name << std::endl;
			pMesh->SetName(name);
		}
		//Add a child to the parent of the instance's source if needed.
		if (pFirstRemappedInstance->GetSourceNodePtr()->GetParentPtr() != pMesh->GetParentPtr())
		{
			pFirstRemappedInstance->GetSourceNodePtr()->GetParentPtr()->AddChildPtr(pMesh);
		}
		for (U32 j = 0; j < (U32)pFirstRemappedInstance->GetNumPolygonMaterialMappings(); j++)
		{
			pMesh->SetPolygonMaterialMapping(j, pFirstRemappedInstance->GetPolygonMaterialMappingPtr(j));
		}
		for (U32 j = 0; j < (U32)pFirstRemappedInstance->GetNumPolygonBlindDataMappings(); j++)
		{
			pMesh->SetPolygonBlindDataMapping(j, pFirstRemappedInstance->GetPolygonBlindDataMappingPtr(j));
		}

#ifdef AFTK_UT_EXPAND_UNIQUE_REMAPPED_INSTANCES_DEBUG
		Aftk::Matrix44 m = pMesh->GetLocalMatrix();
		m[3][1] += hack;
		hack+=3.f;
		pMesh->SetLocalMatrix(m);
#endif

		for (U32 j = 0; j < (U32)instanceBuckets[i].Size(); j++)
		{
			Aftk::C_SceneMeshRemappedInstanceNode* pRemappedInstance = instanceBuckets[i][j];
			if (pRemappedInstance->GetType() == Aftk::E_ObjType_SceneMeshRemappedInstance)
			{
				Aftk::C_SceneInstanceNode* pInstance = new Aftk::C_SceneInstanceNode(pAftkScene);
				pInstance->SetSourceNodePtr(pMesh);
				//Set common node proporties.
				std::string name = pRemappedInstance->GetName() + "___expanded_instance";
				pInstance->SetName(name);
				pInstance->SetBlindDataResourcePtr(pRemappedInstance->GetBlindDataResourcePtr());
				pInstance->SetLocalMatrix(pRemappedInstance->GetLocalMatrix());

				//Overwrite instance node.
				pInstance->Replace(pRemappedInstance);
			}
			else if (pRemappedInstance->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
			{
				pRemappedInstance->SetSourceNodePtr(pMesh, true);
			}
		}
	}

	if (pProgress)
	{
		pProgress->OnProgress(100.f);
	}

	return Aftk::C_Status::E_Status_Success;
};

}
