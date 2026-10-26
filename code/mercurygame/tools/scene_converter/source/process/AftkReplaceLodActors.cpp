#include "stdafx.h"
#include "AftkReplaceLodActors.h"
#include <AftkUt/AftkUt.h>

#define LOD_NAME_PARAM "LODObjectName"

bool IsAncestorOfNonExportedBaseInstance(const Aftk::C_SceneNode& node)
{
	const Aftk::C_SceneNode* currentNode = &node;

	bool isNonExportedBaseInstance = false;
	while (currentNode != NULL && !isNonExportedBaseInstance)
	{
		if (currentNode->GetName() == "InstanceReferences")
		{
			isNonExportedBaseInstance = true;
			break;
		}

		// Parse actor properties, but don't query base instances.
		AftkUt::C_NodeFlagParser actorParser(*currentNode, NULL, false);

		if (!AftkUt::Query::IsInstance(&node)
			//&& node.GetNumInstancesOfThisNode() == 0
			&& actorParser.GetFlagGroupCategory() == "System"
			&& actorParser.GetFlagGroupName() == "BaseInstance")
		{
			bool exportBase = false;
			bool status = actorParser.GetParameterValue("ExportBase", exportBase);

			if (status && !exportBase)
			{
				isNonExportedBaseInstance = true;
				break;
			}
		}
		currentNode = currentNode->GetParentPtr();
	}

	return isNonExportedBaseInstance;
}

bool AftkReplaceLodActors::Process(Aftk::C_Scene& scene)
{
	Ut::Vector<Ut::Pair<
		Aftk::C_SceneNode*, const Aftk::C_SceneLodGroupNode*> > toReplace;

	for (Aftk::C_SceneNodeIterator it(&scene); it; ++it)
	{
		bool status;

		Aftk::C_SceneNode* pSceneNode = it;

		AftkUt::C_NodeFlagParser parser(*pSceneNode, &status);

		//Ignore nodes which aren't actors.
		if (!status)
			continue;

		std::string sourceNode;
		status = parser.GetParameterValue(LOD_NAME_PARAM, sourceNode);
		if (!status)
			//Ignore this node if it doesn't contain a lod source.
			continue;

		//Try and find our source.
		Aftk::C_SceneNode* pSourceNode = scene.GetSceneNodePtr(sourceNode);

		//Our source must exist and be of the correct type.
		if (!pSourceNode || pSourceNode->GetType() != Aftk::E_ObjType_SceneLodGroup)
			continue;

		//If we've dropped down this far we've found a node that we need to replace with a lod group.
		if (!IsAncestorOfNonExportedBaseInstance(*pSceneNode))
		{
			toReplace.PushBack(
				Ut::Pair<Aftk::C_SceneNode*, const Aftk::C_SceneLodGroupNode*>(
					pSceneNode, (const Aftk::C_SceneLodGroupNode*)pSourceNode));
		}

		if (!parser.DeleteParameter(LOD_NAME_PARAM))
			LOGDEBUG( Tools::g_log, "Failed to delete LOD param.");

		//Let's also add any instances of the scene node pointing to our LOD group.
		LOGDEBUG( Tools::g_log, "Replacing base instance %s with LOD group %s.", 
			pSceneNode->GetName().c_str(), pSourceNode->GetName().c_str());

		U32 i = 0;
		for (; i < pSceneNode->GetNumInstancesOfThisNode(); i++)
		{
			Aftk::C_SceneNode* pInstance = pSceneNode->GetInstanceOfThisNode(i);
			if (!pInstance)
				continue;

			toReplace.PushBack(
				Ut::Pair<Aftk::C_SceneNode*, const Aftk::C_SceneLodGroupNode*>(
					pInstance, (const Aftk::C_SceneLodGroupNode*)pSourceNode));
		}
		if (i)
		{
			LOGDEBUG( Tools::g_log, "Replaced %i instances of %s with LOD groups.", 
				i, pSceneNode->GetName().c_str());
		}

		if (IsAncestorOfNonExportedBaseInstance(*pSceneNode))
			continue;

	}
	LOGDEBUG( Tools::g_log, "Replaced %i nodes with LOD group instances.", toReplace.Size());

	//Replace our nodes.
	for (U32 i = 0; i < (U32)toReplace.Size(); ++i)
	{
		Aftk::C_SceneNode* pSceneNode = toReplace[i].first;
		const Aftk::C_SceneLodGroupNode* pSourceNode = toReplace[i].second;
		if (!AftkUt::C_ReplaceWithLodGroup::ReplaceWithLodGroup(
			pSceneNode,
			pSourceNode))
		{
			LOGERROR( Tools::g_log, "Failed to replace actor with LOD group.");
		}
	}

	return true;
}
