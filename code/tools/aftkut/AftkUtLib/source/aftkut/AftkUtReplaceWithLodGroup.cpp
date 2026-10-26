#include "stdafx.h"

using namespace AftkUt;

void StoreBaseInstanceFlagsAsInstanceFlags(
	Aftk::C_SceneNode* pTarget,
	const Aftk::C_SceneNode* pPrimarySource,
	const Aftk::C_SceneNode* pSecondarySource = 0)
{
	const Aftk::C_SceneNode* pSource = 0;

	if (pSecondarySource && C_NodeFlagParser::IsFlagged(*pSecondarySource))
		pSource = pSecondarySource;
	if (pPrimarySource && C_NodeFlagParser::IsFlagged(*pPrimarySource))
		pSource = pPrimarySource;

	//Bail if nothing to copy, or if instance has its own flag data.
	if (C_NodeFlagParser::IsFlagged(*pTarget) || !pSource)
		return;

	{
		C_NodeFlagParser parser(*pSource);
		parser.StoreFlags(*pTarget);
	}
	//Ensure instances don't get flagged as being "base instances".
	{
		C_NodeFlagParser parser(*pTarget);
		if (!stricmp(parser.GetFlagGroupCategory().c_str(), "system") &&
			!stricmp(parser.GetFlagGroupName().c_str(), "BaseInstance"))
			parser.SetFlagGroupName("Instance");
	}
}

Aftk::C_SceneLodGroupNode* C_ReplaceWithLodGroup::ReplaceWithLodGroup(
	Aftk::C_SceneNode* pToReplace, 
	const Aftk::C_SceneLodGroupNode* pLodGroup)
{
	//Both nodes mustn't be null.
	if (!pToReplace || !pLodGroup || !pToReplace->GetParentPtr())
		return 0;

	//Both nodes must be within the same scene.
	if (pToReplace->GetScenePtr() != pLodGroup->GetScenePtr())
		return 0;

	Aftk::C_Scene* pScene = pToReplace->GetScenePtr();

	//Create a new Lod group.
	Aftk::C_SceneLodGroupNode* pNewLodGroup = new Aftk::C_SceneLodGroupNode(pScene);
	StoreBaseInstanceFlagsAsInstanceFlags(pNewLodGroup, pToReplace, pLodGroup);
	pToReplace->GetParentPtr()->AddChildPtr(pNewLodGroup);

	//Set a name to indicate the processing that's been done/
	pNewLodGroup->SetName(pToReplace->GetName() + "_lod");
	//Set our transformation in world-space, also preserving the world-space transformations of our
	//children.
	
	pNewLodGroup->SetWorldMatrix(pToReplace->GetWorldMatrix(), true);

	pScene->ExtractSceneNodeAndChildren(pNewLodGroup);
	
	//Replace original node and assume its unique id.
	if (!pNewLodGroup->Replace(pToReplace, true))
		return 0;

	//Create instances of all of our source lod groups lods.
	for (U32 i = 0; i < pLodGroup->GetNumLodLevels(); i++)
	{
		Aftk::C_GroupLodLevelInfo info = pLodGroup->GetLodLevelInfo(i);

		Aftk::C_SceneNode* pLod = info.GetSceneNodePtr();
		//Resolve any indirect instancing.
		if (pLod->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			Aftk::C_SceneInstanceNode* pLodInst = (Aftk::C_SceneInstanceNode*)pLod;
			if (pLodInst->GetSourceNodePtr() && 
				pLodInst->GetSourceNodePtr()->GetType() == Aftk::E_ObjType_SceneInstance)
			{
				pLodInst->SetSourceNodePtr(pLodInst->GetTopLevelSourceNodePtr());
			}
		}

		//Create and initialise instance.
		Aftk::C_SceneInstanceNode* pInst = new Aftk::C_SceneInstanceNode(pScene);

		//inherit layer membership
		for (U32 i=0; i<pNewLodGroup->GetNumLayers(); ++i)
		{
			Aftk::C_LayerNode *pLayerNode = pNewLodGroup->GetLayerPtr(i);
			pLayerNode->AddMember(pInst);
		}

		pInst->SetName(pLod->GetName() + "_inst");
		pInst->SetSourceNodePtr(pLod);
		Aftk::Matrix44 id;
		UtMaths::SetIdentity(&id);
		pInst->SetLocalMatrix(id);
		//Instance matrix will be identity.

		//Add as child of our new lod group.
		pNewLodGroup->AddChildPtr(pInst);

		//Associate with lod info.
		info.SetSceneNodePtr(pInst);
		//Set lod info to new lod group.
		pNewLodGroup->AddLodLevelInfo(info);

		StoreBaseInstanceFlagsAsInstanceFlags(pInst, pLod);
	}

	return pNewLodGroup;
}
