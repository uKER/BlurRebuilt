#include "stdafx.h"
#include <AftkSpeedTree/AftkSpeedTreeCleanup.h>
#include <AftkSpeedTree/AftkSpeedTreeData.h>

using namespace AftkSpeedTree;

bool IsAncestorOf(const Aftk::C_SceneNode *pNode, const Aftk::C_SceneNode *pRoot)
{
	while (pNode)
	{
		if (pNode == pRoot)
			return true;

		pNode = pNode->GetParentPtr();
	}
	return false;
}

void Cleanup::PreProcess(Aftk::C_Scene& scene)
{
	for (Aftk::C_SceneNodeIterator it(&scene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode = it;
		if (!pSceneNode->GetParentPtr())
			continue;

		if (pSceneNode->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			Aftk::C_SceneInstanceNode* pInst = (Aftk::C_SceneInstanceNode*)pSceneNode;
			//Look at our source for SPT data.
			if (!HasSPTData(pInst->GetTopLevelSourceNodePtr()))
				//Skip
				continue;
		}
		else if (!HasSPTData(it))
			continue;
		
		if (pSceneNode->GetNumChildren() != 0)
		{
			LOGINFO( g_Log, "Preforming cleanup speed tree parent node %s.\n",
				pSceneNode->GetPath().c_str());


			//increment iterator to skip subtree ...
			{
				Aftk::C_SceneNodeIterator it2 = it;
				it2++;
				while (IsAncestorOf(it2, pSceneNode))
				{
					it = it2;
					++it2;
				}
			}

			//Reparent children and recurse.
			while (pSceneNode->GetNumChildren())
			{
				Aftk::C_SceneNode* pChild = pSceneNode->GetChildPtr(0);
				//Reparent child.
				pSceneNode->GetParentPtr()->AddChildPtr(pChild);
				//Compensate transform.
				pChild->SetLocalMatrix(pChild->GetLocalMatrix() * pSceneNode->GetLocalMatrix());
			}
			assert(pSceneNode->GetNumChildren() == 0);
		}
	}
}

void CleanSimpleTreeInstances(Aftk::C_Scene& scene)
{
	//Pass 1
	for (Aftk::C_SceneNodeIterator it(&scene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode(it);

		//Ignore lod groups
		if (pSceneNode->GetParentPtr() && pSceneNode->GetParentPtr()->GetType() == Aftk::E_ObjType_SceneLodGroup)
			continue;

		//We're only interested in instances.
		if (pSceneNode->GetType() != Aftk::E_ObjType_SceneInstance)
			continue;

		//Hacky look to see if we're orphaned from a deleted lod group.
		Aftk::C_SceneInstanceNode* pInst = (Aftk::C_SceneInstanceNode*)pSceneNode;
		Util::C_String name = pInst->GetName().c_str();
		if (name.ISearch("SpeedTreeMesh:") == -1)
			continue;
			
		//Set a more appropriate name,
		name = name.IReplace("SpeedTreeMesh:", "SpeedTreeInstance:");
		pInst->SetName(name.Chars());
	}

	//Allow instance node flags...
	/*
	for (Aftk::C_SceneNodeIterator it(&scene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode(it);

		//Ignore lod groups
		if (pSceneNode->GetParentPtr() && pSceneNode->GetParentPtr()->GetType() == Aftk::E_ObjType_SceneLodGroup)
			continue;

		//We're only interested in instances.
		if (pSceneNode->GetType() != Aftk::E_ObjType_SceneInstance)
			continue;

		//Are we a simple instance?
		Aftk::C_SceneInstanceNode* pInst = (Aftk::C_SceneInstanceNode*)pSceneNode;
		Util::C_String name = pInst->GetName().c_str();
		if (name.ISearch("SpeedTreeInstance:") == -1)
			continue;

		//Remove any actor proporties, we're driven by our base instance.
		AftkUt::C_NodeFlagParser::DeleteFlags(*pSceneNode);
	}
	*/
}

//What we're doing here is looking for base instances that are instanced by nodes that are part
//of a LOD group AND normal instances.
void Cleanup::PostProcess(Aftk::C_Scene& scene)
{
	//Instances can only have a source that's in a LOD group if they themselves are also in
	//a lod group. Where this is not the case we need to duplicate the source data and ensure
	//that this is outside of any LOD group.
	Ut::Vector<Aftk::C_SceneInstanceNode*> instancesToRetarget;
	
	//Map from mesh source -> cloned version
	Ut::UtMap<Aftk::C_SceneMeshNode*, Aftk::C_SceneMeshNode*> cloneMapping;
	for (Aftk::C_SceneNodeIterator it(&scene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode(it);

		//Traverse for nodes that are NOT LOD group children.
		if (pSceneNode->GetParentPtr() && pSceneNode->GetParentPtr()->GetType() == Aftk::E_ObjType_SceneLodGroup)
			continue;

		if (pSceneNode->GetType() != Aftk::E_ObjType_SceneInstance)
			continue;

		Aftk::C_SceneInstanceNode* pInst = 
			(Aftk::C_SceneInstanceNode*)pSceneNode;
		
		Aftk::C_SceneNode* pSource = pInst->GetTopLevelSourceNodePtr();
		if (!pSource)
			continue;

		//Only interested in mesh.
		if (pSource->GetType() != Aftk::E_ObjType_SceneMesh)
			continue;

		Aftk::C_SceneMeshNode* pSourceMesh = (Aftk::C_SceneMeshNode*)pSource;

		//Our source isn't allowed to be within a LOD group.
		if (pSourceMesh->GetParentPtr() && 
			pSourceMesh->GetParentPtr()->GetType() == Aftk::E_ObjType_SceneLodGroup)
		{
			//We need to clone our soure
			Aftk::C_SceneMeshNode* pClonedSourceMesh;
			Ut::UtMap<Aftk::C_SceneMeshNode*, Aftk::C_SceneMeshNode*>::Iterator it = 
				cloneMapping.Find(pSourceMesh);
			if (it != cloneMapping.End())
			{
//				LOGINFO( g_Log, "Reusing base-instance clone %s.\n",
//					pSourceMesh->GetName().c_str());

				//We've previously cloned this mesh.
				pClonedSourceMesh = it->GetValue();
			}
			else
			{
				Aftk::C_SceneLodGroupNode* pLodGroup = 
					(Aftk::C_SceneLodGroupNode*)pSourceMesh->GetParentPtr();

				//Clone mesh and add to our mapping.
//				LOGINFO( g_Log, "Cloning base-instance %s.\n",
//					pSourceMesh->GetName().c_str());
		
				//Copy our mesh.
				pClonedSourceMesh =
					(Aftk::C_SceneMeshNode*)pSourceMesh->Clone(pSourceMesh->GetScenePtr());

				//Nobble the name.
				pClonedSourceMesh->SetName(pClonedSourceMesh->GetName() + "_expanded_tree_base");
				pClonedSourceMesh->SetLocalMatrix(
					pSourceMesh->GetLocalMatrix() * pLodGroup->GetLocalMatrix());			

				//Add our new mesh to the scene graph.
				pLodGroup->GetParentPtr()->AddChildPtr(pClonedSourceMesh);

				//Copy any actor data.
				CleanupActorForLightingTool(pLodGroup->GetParentPtr());

				cloneMapping[pSourceMesh] = pClonedSourceMesh;
			}

			//Reparent our instance.
			pInst->SetSourceNodePtr(pClonedSourceMesh);
		}
	}

	CleanSimpleTreeInstances(scene);
}
