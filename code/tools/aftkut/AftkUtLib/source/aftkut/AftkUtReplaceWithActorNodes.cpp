#include "stdafx.h"

using namespace AftkUt;

Aftk::C_Status C_ReplaceWithActorNodes::ReplaceWithActorNodes(
	Aftk::C_Scene* pAftkScene)
{
	Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> > nodesToReplace;

	for (Aftk::C_SceneNodeIterator it(pAftkScene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode = it;
		//TODO: need to revisit this we will eventually want to export meshes with actor proporties.
		//What we really ought to do is have an actor param to determine if mesh is to be exported.
		if (AftkUt::C_NodeFlagParser::IsFlagged(*pSceneNode))
		{
			bool status;
			AftkUt::C_NodeFlagParser parser(*pSceneNode, &status);
			if (status)
			{
				std::string value;
				parser.GetParameterValue("ignoreParentObject", value);
				if (value == "true")
					nodesToReplace.PushBack(pSceneNode);
			}
		}
	}

	//Replace nodes.
	for (U32 i = 0; i < (U32)nodesToReplace.Size(); i++)
	{
		Aftk::C_SceneNode* pNodeToReplace = nodesToReplace[i];
		Aftk::C_SceneNode* pNewSceneNode = new Aftk::C_SceneNode(pAftkScene);

		//Copy node proporties.
		*pNewSceneNode = *pNodeToReplace;

		//Make a list of nodes instancing the one we're to replace.
		Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> > instancesOfTheNodeWereReplacing;
		for (U32 j = 1; j < (U32)pNodeToReplace->GetNumInstancesOfThisNode(); j++)
		{
			instancesOfTheNodeWereReplacing.PushBack(pNodeToReplace->GetInstanceOfThisNode(j));
		}

		//Replace node.
		pNewSceneNode->Replace(pNodeToReplace, true);

		//Repoint instances of the replaced node
		for (U32 j = 0; j < (U32)instancesOfTheNodeWereReplacing.Size(); j++)
		{
			Aftk::C_SceneNode* pInstance = instancesOfTheNodeWereReplacing[j];
			Aftk::C_Status status = C_Utility::ReparentInstance(pInstance, pNewSceneNode);
			if (!status) return status;
		}
	}

	return Aftk::C_Status::E_Status_Success;
}
