#include "stdafx.h"

#include <Btl/Utility/String.h>
#include <Btl/Utility/hashops.h>
#include <Btl/Containers/OpenHashSet.h>
#include <Btl/Containers/OpenHashMap.h>

#include <AftkUt/AftkUtIrmOptimise.h>

namespace AftkUt
{


bool IsTopLevelBaseInstance(const Aftk::C_SceneNode* pSceneNode)
{
	Btl::String sceneNodePath = pSceneNode->GetPath().c_str();
	if (sceneNodePath.Search("AftkRoot|InstanceReferences") != -1 && pSceneNode->GetName() != "InstanceReferences")
		return true;

	return false;
}

std::string GetOriginalFilePath(const Aftk::C_SceneNode* pNode)
{
	while(pNode)
	{
		const Aftk::I_Plug* pPlug = pNode->GetPlugPtr("originalFilePath");
		if (pPlug)
		{
			std::string fp;
			pPlug->GetValue(fp);
			return fp;
		}
		pNode = pNode->GetParentPtr();
	}

	return "";
}

void C_IrmOptimise::WriteOriginDebug(const Aftk::C_Scene* pScene, const char* outputFilePath)
{
	Aftk::C_Scene junkScene;

	Aftk::C_SceneNode* pRoot = new Aftk::C_SceneNode(&junkScene);
	pRoot->SetName("AftkRoot");
	Aftk::Matrix44 rootM;
	UtMaths::SetIdentity(&rootM);
	rootM[2][2] = -1.f;
	pRoot->SetLocalMatrix(rootM);
	junkScene.SetRootSceneNodePtr(pRoot);

	for (Aftk::C_SceneNodeConstIterator it(pScene); it; ++it)
	{
		const Aftk::C_SceneNode* pNode = it;
		if (!pNode->HasType(Aftk::E_ObjType_SceneMesh) ||
			AftkUt::Query::IsBaseInstance(pNode) ||
			((Aftk::C_SceneMeshNode*)pNode)->GetNumPolygons(0) == 0)
			continue;

		Aftk::Position3 centre = pNode->GetWorldBounds().GetCentre();
		//Is XZ @ 0?
		centre[1] = 0;
		if (Aftk::EqualityTestFVector(Aftk::Vector3F32(0,0,0), (Aftk::Vector3F32&)centre))
		{
			std::string originalFp = GetOriginalFilePath(pNode);
			LOGWARN(g_Log, "Origin clutter - Scene %s mesh %s centred at origin on XZ plane. Most likely a placement / IRM error.", originalFp.c_str(), pNode->GetPath().c_str());
			Aftk::C_SceneNode* pClone = (Aftk::C_SceneNode*)pNode->Clone(&junkScene);
			
			pClone->SetName(originalFp + ":" + pNode->GetPath());
			pRoot->AddChildPtr(pClone);
		}
	}

	if (outputFilePath && junkScene.GetNumDgNodes() > 1)
	{
		LOGINFO(g_Log, "Writing clutter debug file %s", outputFilePath);
		junkScene.Write(outputFilePath);
	}
}

Aftk::C_Status C_IrmOptimise::RemoveDuplicates(Aftk::C_Scene* pScene)
{
	//Build a unique mapping between base instance paths and the base instance itself.
	//Nodes we're interested in will be prefixed AftkRoot/InstanceReferences.
	typedef Btl::OpenHashMap<Btl::String, Aftk::C_SceneNode*, Btl::HashKeyOpBtlString> BaseInstanceMap;
	BaseInstanceMap baseInstanceMapping;

	U32 initialNodeCount = pScene->GetNumDgNodes();

	//Store ids of nodes to remove.
	typedef Btl::OpenHashSet<S32> ToRemoveSet;
	ToRemoveSet toRemove;
	for (Aftk::C_SceneNodeIterator it(pScene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode = it;

		if (IsTopLevelBaseInstance(pSceneNode))
		{
			Btl::String sceneNodePath = pSceneNode->GetPath().c_str();
			BaseInstanceMap::Iterator mapIt = baseInstanceMapping.Find(sceneNodePath);
			if (mapIt == baseInstanceMapping.End())
				baseInstanceMapping.Insert(sceneNodePath, pSceneNode);
			else
			{
				//Remap to existing instance.
				Aftk::C_SceneNode* pExistingBaseInstance = mapIt->GetValue();

				//Make a list of nodes instancing the one we're to replace.
				Btl::Vector<Aftk::C_SceneNode*> instancesToRemap;
				for (U32 i = 0; i < pSceneNode->GetNumInstancesOfThisNode(); ++i)
					instancesToRemap.PushBack(pSceneNode->GetInstanceOfThisNode(i));

				//Repoint instances of the replaced node
				for (S32 i = 0; i < instancesToRemap.Size(); ++i)
				{
					Aftk::C_Status status = C_Utility::ReparentInstance(instancesToRemap[i], pExistingBaseInstance);
					if (!status) return status;
				}

				toRemove.Insert(pSceneNode->GetUniqueId());
			}
		}
	}

	for (ToRemoveSet::Iterator toRemoveIt = toRemove.Begin(); toRemoveIt != toRemove.End(); ++toRemoveIt)
	{
		Aftk::C_SceneNode* pNodeToRemove = pScene->GetSceneNodePtrByUniqueID(toRemoveIt->GetKey());
		if (pNodeToRemove)
			pScene->DeleteSceneNodeAndChildren(pNodeToRemove);
	}

	U32 finalNodeCount = pScene->GetNumDgNodes();

	U32 nodesRemoved = initialNodeCount - finalNodeCount;

	LOGDEBUG(g_Log, "IRM Optimise: Initial node count : %u", initialNodeCount);
	LOGDEBUG(g_Log, "IRM Optimise: Final node count   : %u", finalNodeCount);
	LOGDEBUG(g_Log, "IRM Optimise: Nodes removed      : %u", nodesRemoved);

	return Aftk::C_Status::E_Status_Success;
}

}

